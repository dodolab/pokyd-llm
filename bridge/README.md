# Pokyd Bridge - Protocol & Setup

The bridge is a Node.js TCP server that sits between the Pokyd DOS client and the
OpenAI API. Pokyd speaks a minimal line-oriented text protocol over TCP; the bridge
translates it into an OpenAI agentic loop and returns a plain-ASCII reply.

---

## Wire protocol

### Transport

Plain TCP (no TLS). Default port **8765**. Suitable for localhost or a trusted LAN.
For wider exposure use a VPN or terminate TLS in a reverse proxy - do **not** implement
TLS on the DOS side.

### Framing

Each message is a single line terminated by `\n` (LF). No embedded newlines in the
payload. The server closes the connection on protocol error.

### Handshake (optional, for future extensions)

Not required in v1. The bridge accepts the first `USER` line without a handshake.
A future v2 could add:

```
CLIENT HELLO v1\n
SERVER OK\n
```

### Request - DOS -> Node

```
USER <text>\n
```

- `<text>` is plain ASCII, at most 79 characters (fits in `retezec1`).
- Czech diacritics are already stripped by Pokyd's `NAPIS()` input routine (by
  design of the original application); only 7-bit ASCII arrives here.

```
INITIATIVE <kind> [<seconds>]\n
```

- Proactive Pokyd line when the legacy engine would speak without user input
  (idle timer in `NAPIS()`, startup joke/weather, `REALTIMEKEC()`, welcome).
- `<kind>`: `idle`, `joke`, `weather`, `welcome`, `banter`, `samomluva`, `insult`,
  `resume`, `second_pc`, `second_pc_rules`, `solo_remark`, `joke_fallback`,
  `weather_fallback`, `goodbye` (unknown kinds fall back to `banter` on the bridge).
- Optional number after `idle` is the silence threshold from `POKYD.CFG`
  (`pocetvterin`); after `resume` it is the conversation visit/file index.
- Response is the same `REPLY` / `ERROR` format as `USER`. Does not count as a
  user turn on the DOS side.

### Response - Node -> DOS (success)

```
REPLY <text>\n
```

- `<text>` is plain ASCII, at most 3 980 characters (fits in Pokyd's `dlouhe[4001]`
  buffer, leaving room for the null terminator and the `REPLY ` prefix itself).
- OpenAI responses in UTF-8 are transliterated to ASCII by the bridge before
  sending (Czech and other diacritics replaced with their base letter, e.g. `c-caron -> c`).
- Newlines in the OpenAI response are replaced with a single space, so the entire
  reply fits on one protocol line. Pokyd's `ODPOVED()` handles word-wrap at column 80.

### Response - Node -> DOS (error)

```
ERROR <reason>\n
```

- `<reason>` is a short ASCII string shown to the user via Pokyd's `HLASKA()`.
- On `ERROR`, Pokyd falls back to the legacy rule-based engine for that turn.

### Session state

Session history is kept **per TCP connection**. Disconnecting resets the conversation
context. For reconnect support without losing history, a future `SESSION <id>` handshake
can be added without changing the per-message format.

---

## One-shot: build, bridge, and DOSBox-X (macOS / Linux host)

From the repository root, with `bridge/.env` configured and Open Watcom + DOSBox-X installed:

```bash
./build-and-run-llm.sh
```

This compiles `pokyd.exe` with Watt-32, starts `node bridge/server.js` in the background (log: `bridge/pokyd-bridge.log`), and launches DOSBox-X with `-llm=<host>:<port>` for slirp networking (default host `10.0.2.2`, port from `BRIDGE_PORT` / `bridge/.env`). Use `./build-and-run-llm.sh --exit-after-pokyd` for a non-interactive smoke run.

Environment variables (same names on Windows and macOS/Linux; see repo `README.md`):

| Variable | Role |
|----------|------|
| `BRIDGE_PORT` | Node bridge listen port |
| `POKYD_LLM_IP` | Bridge address as seen from the DOS guest (default `10.0.2.2` with slirp) |
| `POKYD_LLM_PORT` | Port for `pokyd.exe -llm=` (defaults to `BRIDGE_PORT`) |
| `POKYD_LLM_HOST` | Full `host:port` for `-llm=` (overrides IP + port) |

---

## Node.js bridge

### Requirements

- Node.js >= 18
- An OpenAI API key

### Install & run

```bash
cd bridge
npm install
cp .env.example .env
# Edit .env - set OPENAI_API_KEY at minimum
node server.js
```

The server prints its listening address and waits for DOS clients.

### Configuration (`.env`)

| Variable | Default | Description |
|---|---|---|
| `OPENAI_API_KEY` | (required) | OpenAI secret key |
| `OPENAI_MODEL` | `gpt-4o-mini` | Chat model name |
| `OPENAI_MAX_TOKENS` | `512` | Max tokens per API call |
| `BRIDGE_PORT` | `8765` | TCP port to listen on |
| `BRIDGE_BIND` | `0.0.0.0` | Bind address (`127.0.0.1` for localhost-only) |
| `BRIDGE_TIMEOUT_MS` | `30000` | Max ms to wait for an OpenAI response |

Host-side `BRIDGE_PORT` is independent of the address Pokyd uses inside DOS. For DOSBox-X slirp, set `POKYD_LLM_IP=10.0.2.2` (or `POKYD_LLM_HOST=10.0.2.2:<port>`) when launching Pokyd; see `scripts/pokyd-llm-env.sh` / `pokyd-llm-env.ps1`.

The Pokyd persona **system prompt** is read from `bridge/system_prompt.txt` at startup (UTF-8). Edit that file to change behavior; it must be non-empty.

When Pokyd sends `POKYD.CFG` after connect, the bridge parses **mood** (`vyborna` … `hrozna`) and **character** (`stroj`, `naivni`, `klidny`, `prumerny`, `neduverivy`, `naladovy`, `vybusny`) and injects detailed behavioral instructions into the system message. If character is **`stroj`**, the base prompt is replaced with **`system_prompt_terminator.txt`** (Terminator movie persona, emotionless, short cold replies).

Quick check (no OpenAI): `node persona-test.js` from the `bridge/` directory.

### Agentic tools

The bridge runs a full agentic loop: after each user message it calls the OpenAI
Chat Completions API and, if the model requests tool calls, executes them and loops
until a final text response is produced. Tools run **on the Node host only** - no
DOS-side tool execution in v1.

Built-in tools:

| Tool | Description |
|---|---|
| `get_current_datetime` | Returns current date and time on the host |
---

## DOS client (Pokyd)

### Build requirements

- **Bundled Watt-32:** this repo ships `vendor/watt32-dos/` (`inc/` + `wattcpwl.lib`).
  `build.sh` sets `WATT_ROOT` to that path automatically when present - no manual Watt
  build is required on macOS/Linux **unless** you want to regenerate the library.
- Optional: set `WATT_ROOT` yourself to override the bundled tree.
- A packet driver loaded in DOS before running Pokyd (see `assets/NE2000.COM`).

### Enable LLM mode

Pass `-llm=<host>:<port>` on the command line:

```
pokyd.exe -pokyd -llm=10.0.2.2:8765
```

If the bridge cannot be reached, Pokyd prints a message and falls back to the
legacy rule-based engine automatically. LLM mode is completely opt-in.

### WATTCP.CFG

Watt-32 reads `WATTCP.CFG` from the current directory (or the path in the `WATTCP`
environment variable). See `../assets/WATTCP.CFG` for a template.

For **DOSBox-X with slirp backend**:

```ini
my_ip   = 10.0.2.15
netmask = 255.255.255.0
gateway = 10.0.2.2
nameserver = 8.8.8.8
hostname = pokyd
```

The Node bridge then runs on the **host** machine (not inside DOSBox). The `gateway`
address `10.0.2.2` is the host as seen from the slirp guest - run the bridge on the
host and point Pokyd at `10.0.2.2:8765`.

For **real hardware**, set `my_ip` to the machine's static IP and `gateway` to your
router. The bridge must be reachable at the given address on the given port (open the
firewall if necessary).

### Regenerating Watt-32 (optional)

The canonical way on the host (especially **macOS Apple Silicon**) is Docker:

```bash
./scripts/bootstrap-watt32-docker.sh
```

That clones Watt-32 under `vendor/watt-32/` (gitignored), runs Open Watcom **linux/amd64**
inside Ubuntu, and refreshes `vendor/watt32-dos/`.

Manual builds on native DOS or Linux are described upstream in Watt-32 `INSTALL`.

Then:

```bash
./build.sh
```

### DOSBox-X NE2000 configuration

Add to your DOSBox-X config (or let `build-and-run.sh` / `run-dosbox.ps1` do it when
`POKYD_LLM_HOST` or `POKYD_LLM_IP` / `POKYD_LLM_PORT` is set):

```ini
[ne2000]
ne2000=true
nicbase=0x300
nicirq=10
backend=slirp
```

Load the packet driver in the autoexec section before `pokyd.exe`:

```
NE2000.COM 0x60 10 0x300
```

`NE2000.COM` is included under **`assets/NE2000.COM`** (from the Crynwr `pktd11.zip`
collection via Internet Archive). To refresh it:

```bash
./scripts/download-ne2000.sh
```
