/* Tento zdrojovy kod je pod licenci GNU/GPL. Muzete ho pouzit k vlastni
   potrebe, ale nesmite jej ani programy zalozene na tomto kodu vyuzit komercne!

   Jedna se o zdrojovy kod programu Pokyd (http://iqpokyd.kyblsoft.cz)
   od Alese Jandy, aktivne vyvijeneho 1999 - 2002
*/

/*
 * pokyd_llm.c - remote LLM mode: TCP client connecting to the Node.js bridge.
 *
 * Compile-time gate: define POKYD_LLM_WATT (build.sh sets it when WATT_ROOT
 * is found) to compile the real Watt-32 networking code.  Without that flag
 * all public entry points compile as no-op stubs so the rest of the build is
 * unaffected.
 *
 * Watt-32 requirements:
 *   - Headers:   $WATT_ROOT/inc/tcp.h
 *   - Library:   $WATT_ROOT/lib/wattcplf.lib or wattcpwl.lib (Open Watcom large model)
 *   - Runtime:   packet driver loaded before pokyd.exe; WATTCP.CFG present
 *
 * Protocol (see bridge/README.md for the full spec):
 *   DOS -> Node:  optional "CONFIG_START\n" ... lines ... "CONFIG_END\n" (POKYD.CFG)
 *   Node -> DOS:  "OK CONFIG\n" after CONFIG block (optional; failure is non-fatal)
 *   DOS -> Node:  "ASSISTANT <text>\n" ? Pokyd rule-engine line (conversation context)
 *   DOS -> Node:  "USER <text>\n"   (ASCII, <=79 chars)
 *   DOS -> Node:  "INITIATIVE <kind> [<n>]\n"  proactive line (idle/joke/weather/...)
 *   Node -> DOS:  "REPLY <text>\n"  (ASCII, <=3980 chars, no embedded newlines)
 *                 "ERROR <msg>\n"   (bridge-side failure)
 *
 * Encoding:
 *   Pokyd's NAPIS() strips Czech diacritics from user input to base ASCII
 *   letters before storing in retezec1 -- so outgoing text is always 7-bit.
 *   The Node bridge transliterates OpenAI's UTF-8 response to 7-bit ASCII
 *   (see bridge/server.js toAscii()) so incoming bytes are always safe to
 *   display via BIOS INT 10h in any VGA text mode without font remapping.
 *   (Proper CP852 display of diacritics via the VRATDIAKRITIKU() font slots
 *   is left as a future v2 enhancement.)
 *
 * Globals used from pokyd_pr.c: llm_enabled, llm_connected, llm_host, llm_port,
 *   retezec1, dlouhe, barvapocitac1, pocetuzivvet, pozodp.
 * Functions used from pokyd.c / fragments: HLASKA, NAPISRETEZEC, BARVA, STRANA,
 *   DBGLOG, DBGLOGF (all visible because this file is #include-d into pokyd.c).
 */

#ifdef POKYD_LLM_WATT

/*
 * pokyd_pr.c defines BYTE, WORD, DWORD as macros.  Watt-32's inc/sys/wtypes.h
 * uses those names as typedef keywords ? the preprocessor would expand them and
 * break typedef lines ("typedef unsigned char unsigned char").  Undefine for the
 * Watt headers only, then restore the Pokyd macros (same underlying sizes).
 */
#undef BYTE
#undef WORD
#undef DWORD
#include <tcp.h>      /* Watt-32 main header: tcp_Socket, sock_init, etc. */
#define BYTE unsigned char
#define WORD unsigned short
#define DWORD unsigned long

/* TCP socket on the far heap (keeps ~200B out of tight DGROUP). */
static tcp_Socket far *llm_sock     = NULL;
static BYTE            llm_watt_up    = 0;
static BYTE            llm_timer_isr  = 0;

/* Watt sock_* APIs take sock_type* (union); tcp_Socket layout matches .tcp member. */
#define LLM_SK ((sock_type *)llm_sock)

/* Start Watt-32 + timer ISR once, after INTRO has loaded POKYD.CFG / SLOVNIK.DAT. */
static BYTE llm_boot_watt(void) {
  int rc;

  if (llm_watt_up)
    return 1;
  if (!llm_enabled)
    return 0;

  if (llm_sock == NULL) {
    llm_sock = (tcp_Socket far *)_fmalloc(sizeof(tcp_Socket));
    if (llm_sock == NULL) {
      DBGLOG("LLM boot: _fmalloc socket failed");
      return 0;
    }
  }

  rc = sock_init();
  if (rc != 0) {
    DBGLOGF("LLM boot: sock_init failed rc=%d", rc);
    return 0;
  }

  init_timer_isr();
  llm_timer_isr = 1;
  init_userSuppliedTimerTick();
  llm_watt_up = 1;
  DBGLOG("LLM boot: Watt-32 stack OK");
  return 1;
}

/* Try one TCP open + poll until established or timeout. Returns 1 when connected. */
static BYTE llm_connect_to_ip(longword ip) {
  unsigned wait;
  int      rc;

  DBGLOGF("LLM_CONNECT: connecting to %08lx:%u", (unsigned long)ip, (unsigned)llm_port);
  memset(llm_sock, 0, sizeof(*llm_sock));
  rc = tcp_open(llm_sock, 0, ip, llm_port, NULL);
  if (rc == 0) {
    DBGLOGF("LLM_CONNECT: tcp_open failed rc=%d", rc);
    return 0;
  }

  for (wait = 0; wait < 80; wait++) {
    if (tcp_established(llm_sock)) {
      DBGLOG("LLM_CONNECT: connected");
      llm_connected = 1;
      return 1;
    }
    userTimerTick(20);
    tcp_tick(llm_sock);
    tcp_tick(llm_sock);
    tcp_tick(llm_sock);
    tcp_tick(llm_sock);
    CEKEJ(20);
  }

  DBGLOG("LLM_CONNECT: timeout waiting for established");
  sock_close(LLM_SK);
  llm_connected = 0;
  return 0;
}

/* ---------------------------------------------------------------------------
 * LLM_INIT_WATT
 * Parse "host:port" into llm_host/llm_port.  Watt-32 starts lazily on connect
 * so INTRO can read POKYD.CFG / SLOVNIK.DAT before the packet driver hooks run.
 * Returns 1 when host/port parsed, 0 on failure.
 * ------------------------------------------------------------------------- */
static BYTE llm_init_watt(BYTE *hostport) {
  char *colon;

  if (hostport == NULL || hostport[0] == 0)
    return 0;

  /* Copy hostport into llm_host; isolate host and port parts. */
  strncpy((char *)llm_host, (char *)hostport, (size_t)(sizeof(llm_host) - 1));
  llm_host[sizeof(llm_host) - 1] = 0;

  colon = strrchr((char *)llm_host, ':');
  if (colon != NULL) {
    *colon   = 0;
    llm_port = (WORD)atoi(colon + 1);
  }
  if (llm_port == 0) llm_port = 8765;

  llm_enabled = 1;
  DBGLOGF("LLM_INIT: host=%s port=%u (Watt-32 deferred until connect)", (char *)llm_host, (unsigned)llm_port);
  return 1;
}

/* ---------------------------------------------------------------------------
 * LLM_CONNECT_WATT
 * Resolve the bridge host and open a TCP connection (up to 10 s timeout).
 * Returns 1 when connected, 0 on failure.
 * ------------------------------------------------------------------------- */
static BYTE llm_connect_watt(void) {
  longword ip;

  if (!llm_enabled)
    return 0;
  if (!llm_boot_watt())
    return 0;

  DBGLOGF("LLM_CONNECT: resolving %s", (char *)llm_host);
  ip = resolve((char *)llm_host);
  if (ip != 0L && llm_connect_to_ip(ip))
    return 1;

  if (strcmp((char *)llm_host, "127.0.0.1") == 0 ||
      strcmp((char *)llm_host, "10.0.2.2") == 0) {
    /* Slirp NAT gateway (10.0.2.2) is the documented host address inside the guest. */
    if (strcmp((char *)llm_host, "127.0.0.1") == 0) {
      DBGLOG("LLM_CONNECT: retry via slirp gateway 10.0.2.2");
    }
    ip = resolve("10.0.2.2");
    if (ip != 0L && llm_connect_to_ip(ip))
      return 1;
  }

  if (strcmp((char *)llm_host, "10.0.2.2") == 0) {
    DBGLOG("LLM_CONNECT: retry via host loopback 127.0.0.1");
    ip = resolve("127.0.0.1");
    if (ip != 0L && llm_connect_to_ip(ip))
      return 1;
  }

  DBGLOG("LLM_CONNECT: failed (caller may fall back to classic Pokyd)");
  return 0;
}

/* ---------------------------------------------------------------------------
 * llm_recv_line  (internal helper)
 * Read one CRLF-terminated line using Watt sock_gets().  Our old path used
 * sock_dataready()+sock_read() with &llm_sock; sock_* expect sock_type* and
 * sock_dataready() can return 0 in ASCII mode until a full line exists, so
 * partial TCP segments never looked "ready".  sock_gets assembles lines from
 * the rx buffer and matches POP3/SMTP-style text protocols.
 * Returns line length (excluding NUL), or 0 on timeout / disconnect.
 * ------------------------------------------------------------------------- */
static WORD llm_recv_line(BYTE *buf, WORD maxlen, int timeout_sec) {
  unsigned ticks;
  unsigned limit;
  WORD     n;

  if (timeout_sec < 1) timeout_sec = 1;
  limit = (unsigned)timeout_sec * 50;

  for (ticks = 0; ticks < limit; ticks++) {
    userTimerTick(20);
    tcp_tick(LLM_SK);

    if (!tcp_established(llm_sock)) {
      DBGLOG("llm_recv_line: connection closed by remote");
      llm_connected = 0;
      return 0;
    }

    n = sock_gets(LLM_SK, buf, (int)maxlen);
    if (n > 0)
      return n;

    CEKEJ(20);
  }

  DBGLOG("llm_recv_line: timeout");
  return 0;
}

/* ---------------------------------------------------------------------------
 * LLM_SEND_CONFIG_WATT
 * After connect, send POKYD.CFG so the bridge can feed structured settings to the LLM.
 * Non-fatal if the file is missing or OK CONFIG is not received.
 * ------------------------------------------------------------------------- */
static BYTE llm_send_config_watt(void) {
  FILE *f;
  char  line[256];
  int   n;
  WORD  rlen;
  static const char start[] = "CONFIG_START\n";
  static const char end[]   = "CONFIG_END\n";

  if (!llm_connected)
    return 0;

  if (sock_write(LLM_SK, start, (int)sizeof(start) - 1) != (int)sizeof(start) - 1) {
    DBGLOG("LLM_SEND_CONFIG: sock_write CONFIG_START failed");
    llm_connected = 0;
    return 0;
  }

  f = fopen("POKYD.CFG", "rt");
  if (f != NULL) {
    while (fgets(line, (int)sizeof(line), f) != NULL) {
      n = (int)strlen(line);
      while (n > 0 && (line[n - 1] == '\n' || line[n - 1] == '\r'))
        line[--n] = 0;
      line[n]     = '\n';
      line[n + 1] = 0;
      if (sock_write(LLM_SK, line, n + 1) != n + 1) {
        DBGLOG("LLM_SEND_CONFIG: sock_write body failed");
        fclose(f);
        llm_connected = 0;
        return 0;
      }
    }
    fclose(f);
  }

  if (sock_write(LLM_SK, end, (int)sizeof(end) - 1) != (int)sizeof(end) - 1) {
    DBGLOG("LLM_SEND_CONFIG: sock_write CONFIG_END failed");
    llm_connected = 0;
    return 0;
  }

  memset(dlouhe, 0, sizeof(dlouhe));
  rlen = llm_recv_line(dlouhe, (WORD)sizeof(dlouhe), 15);
  if (rlen > 0 && strncmp((char *)dlouhe, "OK CONFIG", 9) == 0)
    DBGLOG("LLM_SEND_CONFIG: bridge acknowledged config");
  else if (rlen > 0)
    DBGLOGF("LLM_SEND_CONFIG: unexpected ack: %.60s", (char *)dlouhe);
  return 1;
}

/* ---------------------------------------------------------------------------
 * llm_send_line_recv  (internal)
 * Send one CRLF-terminated command, receive REPLY/ERROR into dlouhe[].
 * Sets pozodp=100 and llm_odpoved_z_bridge on REPLY.
 * count_user_turn: if 1, increment pocetuzivvet (USER turns only).
 * Returns 1 on REPLY, 0 on failure.
 * ------------------------------------------------------------------------- */
static BYTE llm_send_line_recv(BYTE *line, WORD line_len, BYTE count_user_turn) {
  WORD rlen;

  if (!llm_connected) return 0;

  STRANA(1);
  BARVA(barvapocitac1);
  NAPISRETEZEC("...", barvapocitac1);

  if (sock_write(LLM_SK, line, (int)line_len) != (int)line_len) {
    DBGLOG("llm_send_line_recv: sock_write failed");
    llm_connected = 0;
    return 0;
  }
  DBGLOGF("llm_send_line_recv: sent %u bytes", (unsigned)line_len);

  memset(dlouhe, 0, sizeof(dlouhe));
  rlen = llm_recv_line(dlouhe, (WORD)sizeof(dlouhe), 120);

  if (rlen == 0) {
    DBGLOG("llm_send_line_recv: no reply received");
    HLASKA("LLM: Zadna odpoved z bridge serveru.", 4);
    return 0;
  }

  DBGLOGF("llm_send_line_recv: received %u bytes: %.60s", (unsigned)rlen, (char *)dlouhe);

  if (strncmp((char *)dlouhe, "REPLY ", 6) == 0) {
    memmove(dlouhe, dlouhe + 6, (size_t)(rlen - 6 + 1));
    pozodp = 100;
    if (count_user_turn != 0) pocetuzivvet++;
    llm_odpoved_z_bridge = 1;
    return 1;
  }

  if (strncmp((char *)dlouhe, "ERROR ", 6) == 0) {
    HLASKA(dlouhe + 6, 4);
    return 0;
  }

  DBGLOGF("llm_send_line_recv: unexpected reply prefix: %.40s", (char *)dlouhe);
  return 0;
}

/* ---------------------------------------------------------------------------
 * LLM_SEND_RECV_WATT  (internal)
 * Send "USER <retezec1>\n", receive reply, fill dlouhe[], set pozodp=100.
 * Returns 1 for success (caller should goto OD), 0 to fall back to legacy.
 * ------------------------------------------------------------------------- */
static BYTE llm_send_recv_watt(void) {
  BYTE  msg[90];
  WORD  dlen;

  /* Build "USER <text>\n". */
  msg[0] = 'U'; msg[1] = 'S'; msg[2] = 'E'; msg[3] = 'R'; msg[4] = ' ';
  dlen = (WORD)strlen((char *)retezec1);
  if (dlen > 79) dlen = 79;
  memcpy(msg + 5, retezec1, (size_t)dlen);
  msg[5 + dlen] = '\n';
  dlen = (WORD)(6 + dlen);

  return llm_send_line_recv(msg, dlen, 1);
}

/* ---------------------------------------------------------------------------
 * LLM_SEND_INITIATIVE_WATT  (internal)
 * Proactive Pokyd line (idle poke, joke, weather, ...) without a user sentence.
 * kind: ASCII token (idle, joke, weather, welcome, banter, ...).
 * idle_seconds: optional; appended for kind "idle" (silence timer from NAPIS).
 * ------------------------------------------------------------------------- */
static BYTE llm_send_initiative_watt(BYTE *kind, WORD idle_seconds) {
  BYTE  msg[96];
  WORD  dlen;
  WORD  klen;
  char  numbuf[8];

  if (!llm_connected) {
    if (!llm_connect_watt()) return 0;
    llm_send_config_watt();
  }

  klen = (WORD)strlen((char *)kind);
  if (klen > 24) klen = 24;

  memcpy(msg, "INITIATIVE ", 11);
  memcpy(msg + 11, kind, (size_t)klen);
  dlen = (WORD)(11 + klen);

  if (idle_seconds > 0) {
    if (strncmp((char *)kind, "idle", 4) == 0 ||
        strncmp((char *)kind, "resume", 6) == 0) {
      sprintf(numbuf, " %u", (unsigned)idle_seconds);
      {
        WORD nlen = (WORD)strlen(numbuf);
        memcpy(msg + dlen, numbuf, (size_t)nlen);
        dlen = (WORD)(dlen + nlen);
      }
    }
  }

  msg[dlen] = '\n';
  dlen = (WORD)(dlen + 1);

  DBGLOGF("LLM_SEND_INITIATIVE: kind=%s idle_sec=%u", (char *)kind, (unsigned)idle_seconds);
  return llm_send_line_recv(msg, dlen, 0);
}

/* Paint status into the consplit shell strip (physical rows above Pokyd).
 * Replaces the echoed "-llm host port" command line with a short label. */
static void llm_zapis_shell_stav(BYTE *text, BYTE attr) {
  BYTE  r, puvx, puvy;
  WORD  len, pos, c;
  BYTE *p;

  if (pokyd_shell_rows == 0 || text == NULL || text[0] == 0)
    return;

  puvx = (BYTE)wherex();
  puvy = (BYTE)wherey();
  len  = (WORD)strlen((char *)text);
  if (len > 80) len = 80;
  pos = (WORD)((80 - len) / 2);
  if (pos < 1) pos = 1;

  for (r = 1; r <= pokyd_shell_rows; r++) {
    memset(&pokyd_regs, 0, sizeof(pokyd_regs));
    _AH = 0x02;
    _BH = 0;
    _DH = (BYTE)(r - 1);
    _DL = 0;
    geninterrupt(0x10);
    for (c = 0; c < 80; c++)
      NAPISZNAK(' ', 7);
  }

  memset(&pokyd_regs, 0, sizeof(pokyd_regs));
  _AH = 0x02;
  _BH = 0;
  _DH = (BYTE)(pokyd_shell_rows - 1);
  _DL = (BYTE)(pos - 1);
  geninterrupt(0x10);
  for (p = text; *p != 0; p++)
    NAPISZNAK(*p, attr);

  gotoxy(puvx, puvy);
}

#endif /* POKYD_LLM_WATT */

/* ===========================================================================
 * Public interface ? always compiled.  Stubs when POKYD_LLM_WATT is absent.
 * ========================================================================= */

/* LLM_INIT - parse host:port string and initialise Watt-32 stack.
 * Returns 1 on success, 0 on failure or when networking is not compiled in. */
BYTE LLM_INIT(BYTE *hostport) {
#ifdef POKYD_LLM_WATT
  return llm_init_watt(hostport);
#else
  DBGLOG("LLM_INIT: no Watt-32 in this build (rebuild with vendor/watt32-dos)");
  if (hostport != NULL && hostport[0] != 0) {
    HLASKA("LLM: tato pokyd.exe nema Watt-32. Spustte build-and-run-llm.bat znovu.", 4);
  }
  (void)hostport;
  return 0;
#endif
}

/* LLM_CONNECT - establish TCP connection to the bridge server.
 * Returns 1 when connected, 0 on failure. */
BYTE LLM_CONNECT(void) {
#ifdef POKYD_LLM_WATT
  return llm_connect_watt();
#else
  return 0;
#endif
}

/* Max payload bytes after "ASSISTANT " on one line (dlouhe may be very long). */
#define LLM_ASS_MAXPAY 3800

/* LLM_APPEND_ASSISTANT - send rule-engine / native Pokyd reply so the bridge history matches the screen. */
void LLM_APPEND_ASSISTANT(BYTE *text) {
#ifdef POKYD_LLM_WATT
  size_t tlen;
  WORD   paylen;

  if (!llm_enabled || !llm_connected || text == NULL || text[0] == 0)
    return;
  tlen = strlen((char *)text);
  if (tlen > (size_t)LLM_ASS_MAXPAY)
    tlen = (size_t)LLM_ASS_MAXPAY;
  /* Reuse dlouhe[] (already in DGROUP) instead of a second 4KiB stack buffer. */
  memcpy(dlouhe, "ASSISTANT ", 10);
  memcpy(dlouhe + 10, text, tlen);
  /* One TCP line only: embedded CR/LF would split the write into extra lines and the
   * bridge would respond ERROR Neznamy prikaz for each continuation (breaks next USER). */
  {
    size_t i;
    for (i = 0; i < tlen; i++) {
      if (dlouhe[10 + i] == '\n' || dlouhe[10 + i] == '\r')
        dlouhe[10 + i] = ' ';
    }
  }
  dlouhe[10 + tlen] = '\n';
  paylen = (WORD)(11 + tlen);
  if (sock_write(LLM_SK, dlouhe, (int)paylen) != (int)paylen) {
    DBGLOG("LLM_APPEND_ASSISTANT: sock_write failed");
    llm_connected = 0;
  }
#else
  (void)text;
#endif
}

/* LLM_SEND_CONFIG - push POKYD.CFG to the bridge (once per connection). */
BYTE LLM_SEND_CONFIG(void) {
#ifdef POKYD_LLM_WATT
  return llm_send_config_watt();
#else
  return 0;
#endif
}

/* LLM_SEND_RECV - send current user input (retezec1) to the bridge,
 * fill dlouhe[] with the ASCII reply, and set pozodp=100.
 * Returns 1 for success (caller should "goto OD"), 0 to use legacy engine. */
BYTE LLM_SEND_RECV(void) {
#ifdef POKYD_LLM_WATT
  if (!llm_connected) {
    /* Attempt a lazy reconnect before giving up. */
    DBGLOG("LLM_SEND_RECV: not connected, attempting reconnect");
    if (!llm_connect_watt()) return 0;
    llm_send_config_watt();
  }
  return llm_send_recv_watt();
#else
  return 0;
#endif
}

/* LLM_SEND_INITIATIVE - proactive line (jokes, idle remarks, weather, ...).
 * Does not count as a user turn. Returns 1 when dlouhe[] is ready (pozodp=100). */
BYTE LLM_SEND_INITIATIVE(BYTE *kind, WORD idle_seconds) {
#ifdef POKYD_LLM_WATT
  if (!llm_enabled) return 0;
  return llm_send_initiative_watt(kind, idle_seconds);
#else
  (void)kind;
  (void)idle_seconds;
  return 0;
#endif
}

/* LLM_INITIATIVE_SHOW - send INITIATIVE and display via ODPOVED (long-message path).
 * Returns 1 on success. odpo_mode is passed to ODPOVED (0 or 1). */
BYTE LLM_INITIATIVE_SHOW(BYTE *kind, WORD idle_seconds, BYTE odpo_mode) {
  if (LLM_SEND_INITIATIVE(kind, idle_seconds) == 0) return 0;
  ODPOVED(odpo_mode);
  return 1;
}

/* Show "LLM Pripojeno" once on the first conversation row (yellow status label). */
static BYTE llm_stav_zobrazen = 0;

static void llm_zapis_stav_radek(void) {
  STRANA(1);
  NAPISRETEZEC("LLM Pripojeno", 14);
  pokyd_emit_nl();
}

/* LLM_ZOBRAZ_PRIPOJENO - replace consplit command echo with a short OK label. */
void LLM_ZOBRAZ_PRIPOJENO(void) {
#ifdef POKYD_LLM_WATT
  if (llm_stav_zobrazen != 0)
    return;
  llm_stav_zobrazen = 1;
  DBGLOG("LLM_ZOBRAZ_PRIPOJENO: status label");
  if (pokyd_shell_rows != 0)
    llm_zapis_shell_stav((BYTE *)"LLM Pripojeno", 14);
  llm_zapis_stav_radek();
#else
  (void)0;
#endif
}

/* LLM_ZOBRAZ_CHYBU - connection / bridge failure in the standard HLASKA bar. */
void LLM_ZOBRAZ_CHYBU(BYTE *text) {
  if (text != NULL && text[0] != 0)
    HLASKA(text, 4);
}

/* LLM_CLOSE - close the TCP socket and shut down the Watt-32 stack.
 * Called from PREDKONEC() / KONEC() on program exit. */
void LLM_CLOSE(void) {
#ifdef POKYD_LLM_WATT
  DBGLOGF("LLM_CLOSE e=%u c=%u", (unsigned)llm_enabled, (unsigned)llm_connected);
  if (llm_connected) {
    sock_close(LLM_SK);
    llm_connected = 0;
  }
  if (llm_watt_up) {
    if (llm_timer_isr) {
      exit_timer_isr();
      llm_timer_isr = 0;
    }
    sock_exit();
    llm_watt_up = 0;
  }
  if (llm_sock != NULL) {
    _ffree(llm_sock);
    llm_sock = NULL;
  }
  DBGLOG("LLM_CLOSE ok");
#endif
}
