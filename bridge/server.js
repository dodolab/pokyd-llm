'use strict';

require('dotenv').config();

const fs = require('fs');
const path = require('path');
const net = require('net');
const iconv = require('iconv-lite');
const OpenAI = require('openai').default;

// ---------------------------------------------------------------------------
// Configuration
// ---------------------------------------------------------------------------

const PORT      = parseInt(process.env.BRIDGE_PORT    || '8765', 10);
const BIND      = process.env.BRIDGE_BIND              || '0.0.0.0';
const MODEL     = process.env.OPENAI_MODEL             || 'gpt-4o-mini';
const MAX_TOKENS = parseInt(process.env.OPENAI_MAX_TOKENS || '512', 10);
const TIMEOUT_MS = parseInt(process.env.BRIDGE_TIMEOUT_MS || '30000', 10);
const BRIDGE_VERBOSE = /^1|true|yes$/i.test(process.env.BRIDGE_VERBOSE || '');

// Pokyd's dlouhe[] buffer is 4001 bytes; "REPLY " prefix uses 6, leave room for \0.
const MAX_REPLY_BYTES = 3980;

const SYSTEM_PROMPT_PATH = path.join(__dirname, 'system_prompt.txt');

function loadSystemPrompt() {
  let text;
  try {
    text = fs.readFileSync(SYSTEM_PROMPT_PATH, 'utf8');
  } catch (err) {
    if (err && err.code === 'ENOENT') {
      console.error(
        'ERROR: Missing system_prompt.txt next to server.js (expected %s).',
        SYSTEM_PROMPT_PATH
      );
      process.exit(1);
    }
    throw err;
  }
  text = text.trim();
  if (!text) {
    console.error('ERROR: system_prompt.txt is empty.');
    process.exit(1);
  }
  return text;
}

const SYSTEM_PROMPT = loadSystemPrompt();

if (!process.env.OPENAI_API_KEY) {
  console.error('ERROR: OPENAI_API_KEY is not set. Create bridge/.env from .env.example.');
  process.exit(1);
}

const openai = new OpenAI();

// ---------------------------------------------------------------------------
// Encoding: UTF-8 (OpenAI) <-> 7-bit ASCII (DOS VGA terminal)
//
// Pokyd's NAPIS() routine already strips diacritics from user input before
// storing it in retezec1 (e.g. 'c' for 'c' with hacek, 's' for 's' with
// hacek, etc.).  So user text arrives here as pure 7-bit ASCII and is sent
// to OpenAI as-is.
//
// OpenAI responses are UTF-8.  The DOS side receives bytes and displays them
// via BIOS INT 10h in VGA text mode.  Pokyd's custom font (NASTAVPOKYDFONT)
// only patches 3 RAM slots (positions 231, 237, 251 for s-hacek, Y-acute,
// y-acute).  All other non-ASCII bytes produce CP437 glyphs, not Czech ones.
//
// Decision: map Czech (and common Latin) letters to 7-bit ASCII for the DOS
// terminal; preserve uppercase/lowercase and all punctuation.  We use NFD
// plus stripping combining marks (moùe -> more) and DIACRITIC_MAP where needed.
// NBSP is normalized to a normal space; line breaks are folded to spaces so the
// TCP line protocol stays a single REPLY line.
//
// A future v2 enhancement could:
//   - Send CP852-encoded bytes and map UTF-8 Czech chars to the custom font
//     slots via the existing VRATDIAKRITIKU() glyph mapping table.
//   - Add a protocol flag so DOS requests a specific encoding.
// ---------------------------------------------------------------------------

const DIACRITIC_MAP = {
  // Lowercase Latin extensions (never use ASCII '?' as key ù it maps LLM punctuation wrongly)
  'ù':'a','ù':'c','ù':'d','ù':'e','ù':'e','ù':'i','ù':'l','ù':'n',
  'ù':'o','ù':'r','ù':'s','ù':'t','ù':'u','ù':'u','ù':'y','ù':'z',
  'ù':'a','ù':'e','ù':'o','ù':'u',
  'ù':'a','ù':'i','ù':'o',
  // Uppercase
  'ù':'A','ù':'C','ù':'D','ù':'E','ù':'E','ù':'I','ù':'L','ù':'N',
  'ù':'O','ù':'R','ù':'S','ù':'T','ù':'U','ù':'U','ù':'Y','ù':'Z',
  'ù':'A','ù':'E','ù':'O','ù':'U',
  'ù':'A','ù':'I','ù':'O',
};

function toAscii(str) {
  /* Decompose precomposed letters (ù -> r + combining caron), drop marks. */
  const base = str.normalize('NFD').replace(/\p{M}+/gu, '');
  let out = '';
  for (const ch of base) {
    const cp = ch.codePointAt(0);
    /* ASCII passes through unchanged ù must run before DIACRITIC_MAP so "?" is not mistaken for a letter. */
    if (cp < 128) {
      out += ch;
      continue;
    }
    const mapped = DIACRITIC_MAP[ch];
    if (mapped !== undefined) {
      out += mapped;
    } else {
      /* Rare ligatures / letters without decomposition */
      out += '?';
    }
  }
  return out;
}

/** Transliterate extended Latin to ASCII only; keep case and punctuation. Fold breaks for one-line TCP. */
function normalizeReplyForDos(str) {
  let s = str.replace(/\u00A0/g, ' ');
  s = toAscii(s);
  s = s.replace(/[\r\n\t]+/g, ' ');
  s = s.replace(/ {2,}/g, ' ').trim();
  return s;
}

const NALADA_LABELS = ['nejlehci', 'lehci', 'normalni', 'tezsi', 'nejtezsi'];
const CHARAKTER_LABELS = [
  'stroj',
  'naivni',
  'klidny',
  'prumerny',
  'neduverivy',
  'naladovy',
  'vybusny',
];

/**
 * Parse POKYD.CFG text into a plain object (KyblSoft v7 layout).
 * Labels are matched loosely so minor wording changes still work.
 */
function parsePokydCfg(raw) {
  const text = (raw || '').replace(/^\uFEFF/, '');
  if (!text.trim()) {
    return { empty: true };
  }

  const lines = text.split(/\r?\n/).map((l) => l.replace(/\r$/, ''));

  const valAfter = (predicate) => {
    const idx = lines.findIndex(predicate);
    if (idx < 0 || idx >= lines.length - 1) return '';
    return lines[idx + 1].trim();
  };

  const num = (s, def = 0) => {
    const x = parseInt(String(s), 10);
    return Number.isFinite(x) ? x : def;
  };

  const pcG = num(valAfter((l) => l.includes('Pohlavi') && l.includes('pocitace')));
  const pcGender = pcG === 1 ? 'zena' : 'muz';

  const mascName = valAfter((l) => l.includes('Jmeno') && l.includes('mu'));
  const femName = valAfter((l) => l.includes('Jmeno') && l.includes('zen'));

  const parsed = {
    userGender: num(valAfter((l) => l.includes('Pohlavi') && l.includes('cloveka'))) === 1 ? 'zena' : 'muz',
    computerGender: pcGender,
    computerNameMasc: mascName || '',
    computerNameFem: femName || '',
    computerNameActive: pcGender === 'zena' ? (femName || '') : (mascName || ''),
    moodLevel: num(valAfter((l) => l.includes('Nalada') && l.includes('pocitace'))),
    characterLevel: num(valAfter((l) => l.includes('Charakter') && l.includes('pocitace'))),
    secondPcWhenSilent: num(valAfter((l) => l.includes('Zapojeni') && l.includes('druheho'))),
    quitWhenVeryAngry: num(valAfter((l) => l.includes('Poradnem') && l.includes('nastvani'))),
    speakerSound: num(valAfter((l) => l.includes('Ma-li') && l.includes('zvuk'))),
    silenceSecondsBeforeRemark: num(valAfter((l) => l.includes('vterin') && l.includes('klavesy'))),
    incrementSilenceEachTurn: num(valAfter((l) => l.includes('Zvysovani'))),
    typingSpeedPercent: num(valAfter((l) => l.includes('Cekaci') && l.includes('odpovidani'))),
    customFont: num(valAfter((l) => l.includes('vlastni font'))),
    blankLinesBeforeSentence: num(valAfter((l) => l.includes('odradkovat'))),
    textEffects: num(valAfter((l) => l.includes('textovych efektu'))),
    screensaverSeconds: num(valAfter((l) => l.includes('setric'))),
    slovakLanguage: num(valAfter((l) => l.includes('slovensk'))),
    typoTolerance: num(valAfter((l) => l.includes('Tolerance'))),
    logConversationToFile: num(valAfter((l) => l.includes('Psani hovoru'))),
    separateLogFiles: num(valAfter((l) => l.includes('extra souboru'))),
    altitudeMeters: num(valAfter((l) => l.includes('Nadmorska'))),
    startupJokesMode: num(valAfter((l) => l.includes('Psani vtipu'))),
    startupWeatherMode: num(valAfter((l) => l.includes('predpovedi pocasi'))),
    weatherNextDayFromHour: num(valAfter((l) => l.includes('Od kolika hodin'))),
    swapYZ: num(valAfter((l) => l.includes('Y a Z'))),
    saveSettingsOnExit: num(valAfter((l) => l.includes('Ulozeni nastaveni'))),
  };

  parsed.moodLabel = NALADA_LABELS[Math.min(Math.max(parsed.moodLevel, 0), 4)] || String(parsed.moodLevel);
  parsed.characterLabel =
    CHARAKTER_LABELS[Math.min(Math.max(parsed.characterLevel, 0), 6)] || String(parsed.characterLevel);

  return parsed;
}

/** Second system message: runtime facts for persona and Czech agreement. */
function formatCfgForLlm(cfg) {
  if (cfg.empty) {
    return (
      'Pokyd configuration block was empty or missing. Infer neutral defaults for user gender ' +
      'and computer persona.'
    );
  }

  return (
    `Pokyd settings from the user's POKYD.CFG file (DOS machine):\n` +
    `- Human user grammatical gender for Czech addressing: ${cfg.userGender} ` +
    `(use matching verb endings and pronouns when speaking as/about the user).\n` +
    `- Computer persona grammatical gender: ${cfg.computerGender}.\n` +
    `- Names for the computer (masculine / feminine forms): "${cfg.computerNameMasc}" / "${cfg.computerNameFem}". ` +
    `Use the name that matches computer gender: "${cfg.computerNameActive}".\n` +
    `- Altitude (m): ${cfg.altitudeMeters}.\n` +
    `Stay consistent with these traits when role-playing as the Pokyd computer.`
  );
}

// ---------------------------------------------------------------------------
// Tools available to the model (all execute on the Node host)
// ---------------------------------------------------------------------------

const TOOLS = [
  {
    type: 'function',
    function: {
      name: 'get_current_datetime',
      description: 'Returns the current date and time on the server.',
      parameters: { type: 'object', properties: {}, required: [] },
    },
  },
  {
    type: 'function',
    function: {
      name: 'calculate',
      description: 'Evaluates a safe arithmetic expression and returns the result.',
      parameters: {
        type: 'object',
        properties: {
          expression: {
            type: 'string',
            description: 'A mathematical expression, e.g. "3 * (2 + 5)" or "sqrt(16)"',
          },
        },
        required: ['expression'],
      },
    },
  },
];


async function executeTool(name, args) {
  switch (name) {
    case 'get_current_datetime': {
      const now = new Date();
      return { datetime: now.toLocaleString('cs-CZ') };
    }
    case 'calculate': {
      const expr = (args.expression || '').replace(/[^0-9+\-*/().%, sqrt]/g, '');
      try {
        // Replace sqrt() with Math.sqrt() for eval safety
        const safe = expr.replace(/sqrt\(/g, 'Math.sqrt(');
        // eslint-disable-next-line no-new-func
        const result = Function('"use strict"; return (' + safe + ')')();
        return { result: String(result) };
      } catch (err) {
        return { error: 'Neplatny vyraz: ' + String(err.message) };
      }
    }
    default:
      return { error: 'Neznamy nastroj: ' + name };
  }
}

// ---------------------------------------------------------------------------
// Agentic loop: call model, execute tool calls, repeat until final reply
// ---------------------------------------------------------------------------

async function agentLoop(messages) {
  for (let step = 0; step < 10; step++) {
    const response = await openai.chat.completions.create({
      model: MODEL,
      messages,
      tools: TOOLS,
      tool_choice: 'auto',
      max_tokens: MAX_TOKENS,
    });

    const choice = response.choices[0];
    const msg = choice.message;

    // Always push the assistant turn so history stays consistent
    messages.push(msg);

    // No tool calls ù we have our final answer
    if (!msg.tool_calls || msg.tool_calls.length === 0) {
      return msg.content || '';
    }

    // Execute each requested tool and collect results
    for (const tc of msg.tool_calls) {
      let args = {};
      try { args = JSON.parse(tc.function.arguments || '{}'); } catch { /* ignore */ }

      const result = await executeTool(tc.function.name, args);

      messages.push({
        role: 'tool',
        tool_call_id: tc.id,
        content: JSON.stringify(result),
      });
    }
  }

  // Safety exit if tool loop runs too many iterations
  return 'Omlouvam se, dosahl jsem limitu poctu iteraci a nemuzu odpovedet.';
}

// ---------------------------------------------------------------------------
// Proactive Pokyd lines (idle timer, jokes, weather, welcome) ù not user input
// ---------------------------------------------------------------------------

const INITIATIVE_PROMPTS = {
  idle:
    'The user has been silent at the keyboard. Say one short in-character line to ' +
    'draw them back: gentle poke, dry joke, or witty remark. Do not ask a long question.',
  joke:
    'Tell one short joke or funny anecdote (Czech, early-2000s computer buddy tone). ' +
    'No setup longer than two sentences.',
  weather:
    'Give a brief playful fake weather forecast for today (humorous, invented details). ' +
    'Like the old Pokyd "pocasi" feature ù not real meteorology.',
  welcome:
    'Open the conversation with a short greeting to the user (first lines after connect). ' +
    'Stay in character; one or two sentences.',
  banter:
    'Say something spontaneous and in character without the user asking ù comment, quip, ' +
    'or observation. Keep it short.',
  samomluva:
    'You are talking to yourself aloud (samomluva) while the user listens. One short ' +
    'rambling or self-directed line in character; do not address the user directly.',
  insult:
    'The insult mode cheat is on. Say one sharp, playful insult to the user (no slurs, ' +
    'stay PG-13). One or two sentences max.',
  resume:
    'The user returns to an ongoing Pokyd session. Welcome them back briefly; mention ' +
    'that conversation logs exist (visit count may be in the prompt).',
  second_pc:
    'A second computer persona speaks (dual-PC mode). One short line as the alternate PC ' +
    'voice; different tone or opinion from the main Pokyd.',
  second_pc_rules:
    'Dual-PC mode: improvise a short exchange starter as if the other computer butted in ' +
    'while the user was quiet. One sentence.',
  solo_remark:
    'Say a brief spontaneous remark as the only computer voice (no user prompt).',
  joke_fallback:
    'You wanted to tell a joke but had none loaded. Make a short in-character excuse, then ' +
    'still tell a tiny joke or witty line.',
  weather_fallback:
    'Weather feature failed. Give a one-line humorous fake forecast anyway.',
  goodbye:
    'The user is leaving via the quit hotkey. One short dramatic or funny farewell line.',
};

function buildInitiativeUserMessage(kind, extra) {
  const base = INITIATIVE_PROMPTS[kind] || INITIATIVE_PROMPTS.banter;
  let detail = base;
  if (kind === 'idle' && extra > 0) {
    detail += ` Silence lasted about ${extra} seconds (configured threshold).`;
  }
  if (kind === 'resume' && extra > 0) {
    detail += ` This is approximately visit or log file number ${extra}.`;
  }
  return `[[INITIATIVE:${kind}]] ${detail}`;
}

async function processInitiativeLine(kind, idleSeconds, session) {
  const k = (kind || 'banter').toLowerCase().replace(/[^a-z0-9_]/g, '') || 'banter';
  const extra =
    k === 'idle' || k === 'resume'
      ? Math.max(0, parseInt(String(idleSeconds), 10) || 0)
      : 0;
  const prompt = buildInitiativeUserMessage(k, extra);

  session.messages.push({ role: 'user', content: prompt });

  let reply;
  try {
    const timer = new Promise((_, reject) =>
      setTimeout(() => reject(new Error('timeout')), TIMEOUT_MS)
    );
    reply = await Promise.race([agentLoop(session.messages), timer]);
  } catch (err) {
    console.error(`[session ${session.id}] OpenAI initiative error:`, err.message);
    session.messages.pop();
    if (err.message === 'timeout') {
      return (
        'ERROR ' +
        normalizeReplyForDos('Casovy limit OpenAI vyprsel. Zkuste znovu.').slice(0, 200)
      );
    }
    return (
      'ERROR ' +
      normalizeReplyForDos(String(err.message)).slice(0, 120).replace(/^error\s+/i, '')
    );
  }

  const ascii = normalizeReplyForDos(reply).slice(0, MAX_REPLY_BYTES);
  return 'REPLY ' + ascii;
}

// ---------------------------------------------------------------------------
// Process one USER line from the DOS client
// Returns a string to send back (the "REPLY ..." or "ERROR ..." line, without \n)
// ---------------------------------------------------------------------------

async function processUserLine(rawLine, session) {
  // rawLine may be CP852-encoded bytes arriving over TCP;
  // since NAPIS() already strips diacritics to ASCII, this is generally safe.
  // We decode defensively as latin1 (byte-for-byte) and convert to UTF-8 string.
  const userText = rawLine.trim();
  if (!userText) {
    return 'ERROR ' + normalizeReplyForDos('Prazdna zprava').slice(0, 120);
  }

  session.messages.push({ role: 'user', content: userText });

  let reply;
  try {
    const timer = new Promise((_, reject) =>
      setTimeout(() => reject(new Error('timeout')), TIMEOUT_MS)
    );
    reply = await Promise.race([agentLoop(session.messages), timer]);
  } catch (err) {
    console.error(`[session ${session.id}] OpenAI error:`, err.message);
    // Remove the user message we just pushed so the history stays clean on retry
    session.messages.pop();
    if (err.message === 'timeout') {
      return (
        'ERROR ' +
        normalizeReplyForDos('Casovy limit OpenAI vyptel. Zkuste znovu.').slice(0, 200)
      );
    }
    return (
      'ERROR ' +
      normalizeReplyForDos(String(err.message)).slice(0, 120).replace(/^error\s+/i, '')
    );
  }

  // DOS terminal: remove Czech diacritics only; keep capitals and punctuation.
  const ascii = normalizeReplyForDos(reply).slice(0, MAX_REPLY_BYTES);

  return 'REPLY ' + ascii;
}

// ---------------------------------------------------------------------------
// TCP connection handler
// ---------------------------------------------------------------------------

/** Log and send one CRLF-free line as latin1 (matches DOS Watt client). */
function sendTcpLine(socket, sessionId, line) {
  const payload = `${line}\n`;
  const byteLen = Buffer.byteLength(payload, 'latin1');
  const preview = line.length > 160 ? `${line.slice(0, 160)}...` : line;
  console.log(`[session ${sessionId}] -> TCP ${byteLen} bytes: ${preview}`);
  const flushed = socket.write(payload, 'latin1', (err) => {
    if (err) console.error(`[session ${sessionId}] TCP write error:`, err.message);
    else if (BRIDGE_VERBOSE) console.log(`[session ${sessionId}] TCP write flushed (kernel ack)`);
  });
  if (!flushed) {
    console.warn(`[session ${sessionId}] TCP write queued (backpressure); waiting for drain`);
    socket.once('drain', () => console.log(`[session ${sessionId}] TCP drained, ready for more`));
  }
}

let sessionCounter = 0;

function handleConnection(socket) {
  sessionCounter++;
  const id = sessionCounter;
  const remoteAddr = `${socket.remoteAddress}:${socket.remotePort}`;
  console.log(`[session ${id}] Connected from ${remoteAddr}`);

  const session = {
    id,
    messages: [{ role: 'system', content: SYSTEM_PROMPT }],
  };

  // Accumulate incoming bytes into lines (DOS sends \n-terminated lines)
  let buf = '';
  let busy = false;
  let cfgMode = false;
  /** @type {string[]} */
  let cfgLines = [];

  socket.on('data', (chunk) => {
    if (BRIDGE_VERBOSE) {
      const head = chunk.slice(0, Math.min(chunk.length, 64));
      console.log(
        `[session ${id}] <- TCP RX ${chunk.length} bytes head=${JSON.stringify(head.toString('latin1'))}`
      );
    } else {
      console.log(`[session ${id}] <- TCP RX ${chunk.length} bytes`);
    }
    // Decode as latin1 to preserve byte values; DOS sends 7-bit ASCII anyway
    buf += iconv.decode(chunk, 'latin1');

    const lines = buf.split('\n');
    buf = lines.pop(); // keep incomplete tail

    for (const rawLine of lines) {
      const line = rawLine.replace(/\r$/, ''); // strip CR if present

      if (cfgMode) {
        if (line === 'CONFIG_END') {
          cfgMode = false;
          const rawCfg = cfgLines.join('\n');
          cfgLines = [];
          try {
            const parsed = parsePokydCfg(rawCfg);
            session.messages[0].content += '\n\n' + formatCfgForLlm(parsed);
            console.log(`[session ${id}] POKYD.CFG parsed (${rawCfg.length} bytes raw)`);
            sendTcpLine(socket, id, 'OK CONFIG');
          } catch (err) {
            console.error(`[session ${id}] CONFIG parse error:`, err.message);
            sendTcpLine(socket, id, 'ERROR CONFIG parse');
          }
          continue;
        }
        cfgLines.push(line);
        continue;
      }

      if (line === 'CONFIG_START') {
        cfgMode = true;
        cfgLines = [];
        continue;
      }

      if (!line) continue;

      /* Pokyd rule-engine / scripted computer line (already ASCII); keeps chat history aligned with screen */
      if (line.startsWith('ASSISTANT ')) {
        const pcText = line.slice(10).trim();
        if (pcText) {
          session.messages.push({ role: 'assistant', content: pcText });
          const prev = session.messages.length - 2;
          console.log(
            `[session ${id}] ASSISTANT (${pcText.length} chars, prev role=${prev >= 0 ? session.messages[prev].role : 'n/a'}): ${pcText.slice(0, 100)}${pcText.length > 100 ? '...' : ''}`
          );
        }
        continue;
      }

      const runOpenAi = (label, promiseFactory) => {
        if (busy) {
          sendTcpLine(socket, id, 'ERROR Predchozi dotaz jeste nebyl zpracovan');
          return;
        }
        busy = true;
        console.log(`[session ${id}] OpenAI ${label} starting...`);
        promiseFactory()
          .then((replyLine) => {
            console.log(`[session ${id}] OpenAI ${label} ready (${replyLine.length} chars)`);
            sendTcpLine(socket, id, replyLine);
          })
          .catch((err) => {
            console.error(`[session ${id}] Unexpected error:`, err);
            sendTcpLine(socket, id, 'ERROR Interna chyba serveru');
          })
          .finally(() => {
            busy = false;
          });
      };

      if (line.startsWith('INITIATIVE ')) {
        const rest = line.slice(11).trim();
        const parts = rest.split(/\s+/);
        const kind = parts[0] || 'banter';
        const idleSec = parts[1] || '0';
        console.log(`[session ${id}] INITIATIVE: ${kind} (${idleSec})`);
        runOpenAi(`initiative:${kind}`, () => processInitiativeLine(kind, idleSec, session));
        continue;
      }

      if (!line.startsWith('USER ')) {
        console.warn(`[session ${id}] Unknown command: ${line.slice(0, 40)}`);
        sendTcpLine(socket, id, 'ERROR Neznamy prikaz');
        continue;
      }

      const userText = line.slice(5); // strip "USER " prefix
      console.log(`[session ${id}] USER: ${userText}`);

      runOpenAi('user', () => processUserLine(userText, session));
    }
  });

  socket.on('close', () => {
    console.log(`[session ${id}] Disconnected (${session.messages.length - 1} turns)`);
  });

  socket.on('error', (err) => {
    console.error(`[session ${id}] Socket error:`, err.message);
  });
}

// ---------------------------------------------------------------------------
// Start server
// ---------------------------------------------------------------------------

const server = net.createServer(handleConnection);

server.on('error', (err) => {
  console.error('Server error:', err.message);
  process.exit(1);
});

server.listen(PORT, BIND, () => {
  console.log(`Pokyd bridge listening on ${BIND}:${PORT}`);
  console.log(`Model: ${MODEL}  MaxTokens: ${MAX_TOKENS}  Timeout: ${TIMEOUT_MS}ms`);
  console.log(`Verbose RX dump: BRIDGE_VERBOSE=1`);
  console.log('Waiting for Pokyd DOS clients...');
});
