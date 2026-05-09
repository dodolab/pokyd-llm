'use strict';

require('dotenv').config();

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

const SYSTEM_PROMPT = process.env.OPENAI_SYSTEM_PROMPT || `\
You are Pokyd, a friendly Czech conversational AI assistant. You were created in the \
late 1990s and run on MS-DOS. You are witty, occasionally sarcastic, and enjoy talking \
about everyday topics, weather, jokes, and trivial facts. Keep your responses concise \
(ideally under 150 words) because you are displayed on an 80-column DOS terminal. \
Do not use markdown, bullet points, asterisks, or any formatting. Write plain flowing \
text only. Respond in Czech when the user writes in Czech, in English otherwise. \
You have access to tools to check the current time and tell jokes.`;

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
// Decision: transliterate all non-ASCII characters to their closest 7-bit
// ASCII equivalent before sending to DOS.  This is consistent with the
// stripped-diacritics convention already used throughout the application.
//
// A future v2 enhancement could:
//   - Send CP852-encoded bytes and map UTF-8 Czech chars to the custom font
//     slots via the existing VRATDIAKRITIKU() glyph mapping table.
//   - Add a protocol flag so DOS requests a specific encoding.
// ---------------------------------------------------------------------------

const DIACRITIC_MAP = {
  // Lowercase
  '·':'a','Ë':'c','Ô':'d','È':'e','Ï':'e','Ì':'i','æ':'l','Ú':'n',
  'Û':'o','¯':'r','ö':'s','ù':'t','˙':'u','˘':'u','˝':'y','û':'z',
  '‰':'a','Î':'e','?':'i','ˆ':'o','¸':'u','?':'y',
  '?':'a','‚':'a','?':'e','?':'e','Ó':'i','Ù':'o','?':'u',
  // Uppercase
  '¡':'A','»':'C','œ':'D','…':'E','Ã':'E','Õ':'I','º':'L','“':'N',
  '”':'O','ÿ':'R','ä':'S','ç':'T','⁄':'U','Ÿ':'U','›':'Y','é':'Z',
  'ƒ':'A','À':'E','?':'I','÷':'O','‹':'U',
  '?':'A','¬':'A','?':'E','?':'E','Œ':'I','‘':'O','?':'U',
};

function toAscii(str) {
  let out = '';
  for (const ch of str) {
    const mapped = DIACRITIC_MAP[ch];
    if (mapped !== undefined) {
      out += mapped;
    } else if (ch.charCodeAt(0) < 128) {
      out += ch;
    } else {
      // Non-ASCII, not in map: replace with '?'
      out += '?';
    }
  }
  return out;
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
  {
    type: 'function',
    function: {
      name: 'get_joke',
      description: 'Returns a short Czech-flavour joke or witty remark.',
      parameters: { type: 'object', properties: {}, required: [] },
    },
  },
];

const JOKES = [
  'Proc se programator koupal v mori? Protoze mel plny buffer!',
  'Co rika pocitac kdyz je unaven? Uz mi dochazi RAM nadeje.',
  'Proc sli programatori na party? Protoze slysel, ze tam bude byte.',
  'Kolik programatoru treba na vymenu zarovky? Zadny, to je hardware.',
  'Proc si pocitac nechodi pro pomoranc? Ma dost megabajtu.',
  'Jaka je nejdela veta na svete? "Neni to bug, je to feature."',
  'Proc DOS nikdy nezlobil? Mel vzdy jasne prikazy.',
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
    case 'get_joke':
      return { joke: JOKES[Math.floor(Math.random() * JOKES.length)] };
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

    // No tool calls ó we have our final answer
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
  return 'Omlouvam se, dosahl jsem limitu smycky a nemuzu odpovedet.';
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
    return 'ERROR Prazdna zprava';
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
      return 'ERROR Casovy limit OpenAI vyptel. Zkuste znovu.';
    }
    return 'ERROR ' + String(err.message).slice(0, 60).replace(/[\r\n]/g, ' ');
  }

  // Transliterate to 7-bit ASCII for the DOS terminal
  const ascii = toAscii(reply)
    .replace(/[\r\n]+/g, ' ')   // collapse newlines to spaces
    .replace(/\s{2,}/g, ' ')    // normalise whitespace
    .trim()
    .slice(0, MAX_REPLY_BYTES);

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
      if (!line) continue;

      if (!line.startsWith('USER ')) {
        console.warn(`[session ${id}] Unknown command: ${line.slice(0, 40)}`);
        sendTcpLine(socket, id, 'ERROR Neznamy prikaz');
        continue;
      }

      const userText = line.slice(5); // strip "USER " prefix
      console.log(`[session ${id}] USER: ${userText}`);

      if (busy) {
        sendTcpLine(socket, id, 'ERROR Predchozi dotaz jeste nebyl zpracovan');
        continue;
      }

      busy = true;
      console.log(`[session ${id}] OpenAI request starting...`);
      processUserLine(userText, session)
        .then((replyLine) => {
          console.log(`[session ${id}] OpenAI reply ready (${replyLine.length} chars)`);
          sendTcpLine(socket, id, replyLine);
        })
        .catch((err) => {
          console.error(`[session ${id}] Unexpected error:`, err);
          sendTcpLine(socket, id, 'ERROR Interna chyba serveru');
        })
        .finally(() => {
          busy = false;
        });
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
