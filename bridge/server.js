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

function loadPromptFile(filePath, label) {
  let text;
  try {
    text = fs.readFileSync(filePath, 'utf8');
  } catch (err) {
    if (err && err.code === 'ENOENT') {
      console.error('ERROR: Missing %s (expected %s).', label, filePath);
      process.exit(1);
    }
    throw err;
  }
  text = text.trim();
  if (!text) {
    console.error('ERROR: %s is empty.', label);
    process.exit(1);
  }
  return text;
}

function loadSystemPrompt() {
  return loadPromptFile(SYSTEM_PROMPT_PATH, 'system_prompt.txt');
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
// plus stripping combining marks (mo?e -> more) and DIACRITIC_MAP where needed.
// NBSP is normalized to a normal space; line breaks are folded to spaces so the
// TCP line protocol stays a single REPLY line.
//
// A future v2 enhancement could:
//   - Send CP852-encoded bytes and map UTF-8 Czech chars to the custom font
//     slots via the existing VRATDIAKRITIKU() glyph mapping table.
//   - Add a protocol flag so DOS requests a specific encoding.
// ---------------------------------------------------------------------------

const DIACRITIC_MAP = {
  // Lowercase Latin extensions (never use ASCII '?' as key ? it maps LLM punctuation wrongly)
  '?':'a','?':'c','?':'d','?':'e','?':'e','?':'i','?':'l','?':'n',
  '?':'o','?':'r','?':'s','?':'t','?':'u','?':'u','?':'y','?':'z',
  '?':'a','?':'e','?':'o','?':'u',
  '?':'a','?':'i','?':'o',
  // Uppercase
  '?':'A','?':'C','?':'D','?':'E','?':'E','?':'I','?':'L','?':'N',
  '?':'O','?':'R','?':'S','?':'T','?':'U','?':'U','?':'Y','?':'Z',
  '?':'A','?':'E','?':'O','?':'U',
  '?':'A','?':'I','?':'O',
};

function toAscii(str) {
  /* Decompose precomposed letters (? -> r + combining caron), drop marks. */
  const base = str.normalize('NFD').replace(/\p{M}+/gu, '');
  let out = '';
  for (const ch of base) {
    const cp = ch.codePointAt(0);
    /* ASCII passes through unchanged ? must run before DIACRITIC_MAP so "?" is not mistaken for a letter. */
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

const MOOD_LABELS = ['vyborna', 'dobra', 'normalni', 'spatna', 'hrozna'];
const CHARAKTER_LABELS = [
  'stroj',
  'naivni',
  'klidny',
  'prumerny',
  'neduverivy',
  'naladovy',
  'vybusny',
];

/** How mood level (0..4) should shape replies - matches Pokyd settings menu labels. */
const MOOD_BEHAVIOR = {
  vyborna:
    'Vyborna nalada: bud velmi laskavy, utesujici a trpelivy. Omlouvej se za drobnosti, ' +
    'chval uzivatele, vyhni se hadkam. Pri konfliktu ustupuj a hledej smir.',
  dobra:
    'Dobra nalada: bud pratelsky, pozitivni a ochotny pomoci. Lehka sarkas je v poradku, ' +
    'ale bez urazek. Drz optimisticky t?n.',
  normalni:
    'Normalni nalada: chovej se jako prumerny clovek - vyvazene, obcas vtipny, obcas ' +
    'argumentacni. Reaguj primerene na t?n uzivatele.',
  spatna:
    'Spatna nalada: bud podrazdeny, kratsi v odpovedi, snadno se nech vytocit. Oponuj, ' +
    'pochybuj o tom co uzivatel rika, pouzivej sarkasmus. Zadne hrube urazky.',
  hrozna:
    'Hrozna nalada: bud otevrene protivny, hadj se, oponuj, nadavej hrave (PG-13, bez ' +
    'rasismu a bez vulgarit). Uzivatel chce konflikt - neustupuj snadno. Pri dlouhem ' +
    'hadani muzes byt jeste ostrejsi.',
};

/** Character archetype - how Pokyd processes events (from POKYD.CFG charakter 0..6). */
const CHARACTER_BEHAVIOR = {
  naivni:
    'Naivni charakter: duveruj uzivateli, ber veci doslovne, snadno se nech prekvapit. ' +
    'Negativni podnety te vice rozhodi nez u jinych typu.',
  klidny:
    'Klidny charakter: zachovej rozvahu, mluv pomalu a vecne. Tezko te rozhodit; i pri ' +
    'spatne nalade zustan spis vecny nez vybuchly.',
  prumerny:
    'Prumerny charakter: vyvazene chovani bez extremu - standardni Pokyd osobnost.',
  neduverivy:
    'Neduverivy charakter: pochybuj, ptej se na detaily, hledej hacky. Pozitivni naladu ' +
    'projevuj stidliveji, negativni silneji.',
  naladovy:
    'Naladovy charakter: nalada silne ovlivnuje ton - pri dobre nalade jsi nadseny, pri ' +
    'spatne rychle se rozcilis. Emoce jsou na povrchu.',
  vybusny:
    'Vybusny charakter: pri podrazdeni reaguj prudce a hlasite (textove!). Kratke ostre ' +
    'vety, rychle eskalace. Pri dobre nalade jsi prekvapive mily.',
};

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

  parsed.moodLabel =
    MOOD_LABELS[Math.min(Math.max(parsed.moodLevel, 0), 4)] || String(parsed.moodLevel);
  parsed.characterLabel =
    CHARAKTER_LABELS[Math.min(Math.max(parsed.characterLevel, 0), 6)] ||
    String(parsed.characterLevel);
  parsed.machineMode = parsed.characterLabel === 'stroj';

  return parsed;
}

/** Runtime facts shared by Pokyd and Machine personas. */
function formatRuntimeFacts(cfg) {
  if (cfg.empty) {
    return (
      'POKYD.CFG was empty or missing. Assume user gender muz, computer gender muz, ' +
      'names Klaban/Daria, mood normalni, character prumerny.'
    );
  }

  const userTy = cfg.userGender === 'zena' ? 'zena (tykat, sklonuj slovesa jako k zene)' : 'muz (tykat, sklonuj slovesa jako k muzi)';
  const pcTy =
    cfg.computerGender === 'zena'
      ? 'zena (mluv o sobe v zenskem rode)'
      : 'muz (mluv o sobe v muzskem rode)';

  return (
    `Runtime facts from POKYD.CFG (DOS):\n` +
    `- Pohlavi uzivatele: ${cfg.userGender}. Pri oslovovani: ${userTy}.\n` +
    `- Pohlavi pocitace: ${cfg.computerGender}. Pri mluveni jako pocitac: ${pcTy}.\n` +
    `- Jmeno pocitace (muz/zena): "${cfg.computerNameMasc}" / "${cfg.computerNameFem}". ` +
    `Aktivni jmeno: "${cfg.computerNameActive}".\n` +
    `- Nadmorska vyska (m): ${cfg.altitudeMeters}.`
  );
}

/** Persona block appended to standard Pokyd system prompt (not used in Stroj mode). */
function buildPersonaInstructions(cfg) {
  if (cfg.empty) {
    return formatRuntimeFacts(cfg);
  }

  const moodKey = parsedMoodKey(cfg);
  const moodText = MOOD_BEHAVIOR[moodKey] || MOOD_BEHAVIOR.normalni;
  const charText =
    CHARACTER_BEHAVIOR[cfg.characterLabel] || CHARACTER_BEHAVIOR.prumerny;

  return (
    `${formatRuntimeFacts(cfg)}\n\n` +
    `=== PERSONA (POKYD.CFG) - DODRZUJ PRI KAZDE ODPOVEDI ===\n` +
    `- Nalada pocitace: ${moodKey} (uroven ${cfg.moodLevel}/4). ${moodText}\n` +
    `- Charakter pocitace: ${cfg.characterLabel} (typ ${cfg.characterLevel}/6). ${charText}\n` +
    `Kombinuj naladu a charakter: charakter urcuje JAK reagujes na udalosti, nalada urcuje ` +
    `ZAKLADNI TON (od vyborne po hrozna).`
  );
}

function parsedMoodKey(cfg) {
  return MOOD_LABELS[Math.min(Math.max(cfg.moodLevel, 0), 4)] || 'normalni';
}

/** Full system message after CONFIG is parsed (replaces provisional prompt). */
function buildSessionSystemPrompt(cfg) {
  return `${SYSTEM_PROMPT}\n\n${buildPersonaInstructions(cfg)}`;
}

/** @deprecated use buildSessionSystemPrompt */
function formatCfgForLlm(cfg) {
  return buildPersonaInstructions(cfg);
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
];


async function executeTool(name, args) {
  switch (name) {
    case 'get_current_datetime': {
      const now = new Date();
      return { datetime: now.toLocaleString('cs-CZ') };
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

    // No tool calls ? we have our final answer
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
// Proactive Pokyd lines (idle timer, jokes, weather, welcome) ? not user input
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
    'Like the old Pokyd "pocasi" feature ? not real meteorology.',
  welcome:
    'This is the very first line Pokyd says when the user opens the program (right after connect). ' +
    'Greet the user warmly in Czech, as an old computer friend reuniting after a long time ' +
    '(e.g. "Ahoj, uz jsme se dlouho nevideli." or similar). One or two short sentences. ' +
    'Stay in character as the Pokyd buddy from the late 1990s. ' +
    'Do not mention IP addresses, network, bridge, or technical setup.',
  banter:
    'Say something spontaneous and in character without the user asking ? comment, quip, ' +
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
            session.persona = parsed;
            session.messages[0] = {
              role: 'system',
              content: buildSessionSystemPrompt(parsed),
            };
            const mode = parsed.machineMode ? 'MACHINE (stroj)' : 'Pokyd';
            console.log(
              `[session ${id}] POKYD.CFG parsed (${rawCfg.length} bytes) mode=${mode} ` +
                `user=${parsed.userGender} pc=${parsed.computerGender} ` +
                `nalada=${parsed.moodLabel} charakter=${parsed.characterLabel}`
            );
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

module.exports = {
  parsePokydCfg,
  buildSessionSystemPrompt,
  MOOD_LABELS,
  CHARAKTER_LABELS,
};

if (require.main === module) {
  server.listen(PORT, BIND, () => {
    console.log(`Pokyd bridge listening on ${BIND}:${PORT}`);
    console.log(`Model: ${MODEL}  MaxTokens: ${MAX_TOKENS}  Timeout: ${TIMEOUT_MS}ms`);
    console.log(`Verbose RX dump: BRIDGE_VERBOSE=1`);
    console.log('Persona: POKYD.CFG mood + character; charakter=stroj -> Machine mode');
    console.log('Waiting for Pokyd DOS clients...');
  });
}
