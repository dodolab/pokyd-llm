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
 *   DOS -> Node:  "ASSISTANT <text>\n" — Pokyd rule-engine line (conversation context)
 *   DOS -> Node:  "USER <text>\n"   (ASCII, <=79 chars)
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
 * uses those names as typedef keywords — the preprocessor would expand them and
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

/* Single persistent TCP socket for the bridge connection. */
static tcp_Socket llm_sock;

/* Watt sock_* APIs take sock_type* (union); tcp_Socket layout matches .tcp member. */
#define LLM_SK ((sock_type *)&llm_sock)

/* ---------------------------------------------------------------------------
 * LLM_INIT_WATT
 * Parse "host:port" string into llm_host/llm_port and initialise Watt-32.
 * Called once from main() before INTRO.
 * Returns 1 on success, 0 on failure.
 * ------------------------------------------------------------------------- */
static BYTE llm_init_watt(BYTE *hostport) {
  char *colon;
  int   rc;

  /* Copy hostport into llm_host; isolate host and port parts. */
  strncpy((char *)llm_host, (char *)hostport, (size_t)(sizeof(llm_host) - 1));
  llm_host[sizeof(llm_host) - 1] = 0;

  colon = strrchr((char *)llm_host, ':');
  if (colon != NULL) {
    *colon   = 0;
    llm_port = (WORD)atoi(colon + 1);
  }
  if (llm_port == 0) llm_port = 8765;

  DBGLOGF("LLM_INIT: host=%s port=%u", (char *)llm_host, (unsigned)llm_port);

  rc = sock_init();
  if (rc != 0) {
    DBGLOGF("LLM_INIT: sock_init failed rc=%d", rc);
    HLASKA("LLM: Watt-32 sock_init selhal. Skontrolujte WATTCP.CFG a packet driver.", 4);
    return 0;
  }

  llm_enabled = 1;
  DBGLOG("LLM_INIT: Watt-32 stack OK");
  return 1;
}

/* ---------------------------------------------------------------------------
 * LLM_CONNECT_WATT
 * Resolve the bridge host and open a TCP connection (up to 10 s timeout).
 * Returns 1 when connected, 0 on failure.
 * ------------------------------------------------------------------------- */
static BYTE llm_connect_watt(void) {
  longword ip;
  int      status;
  int      rc;

  DBGLOGF("LLM_CONNECT: resolving %s", (char *)llm_host);
  ip = resolve((char *)llm_host);
  if (ip == 0L) {
    DBGLOGF("LLM_CONNECT: resolve failed for %s", (char *)llm_host);
    HLASKA("LLM: Nelze prelozit adresu bridge serveru.", 4);
    return 0;
  }

  DBGLOGF("LLM_CONNECT: connecting to %08lx:%u", (unsigned long)ip, (unsigned)llm_port);
  memset(&llm_sock, 0, sizeof(llm_sock));
  /* Watt-32 tcp_open returns 1 on success, 0 on error (see pctcp.c). */
  rc = tcp_open(&llm_sock, 0, ip, llm_port, NULL);
  if (rc == 0) {
    DBGLOGF("LLM_CONNECT: tcp_open failed rc=%d", rc);
    HLASKA("LLM: tcp_open selhal.", 4);
    return 0;
  }

  /* Slirp/NE2000 can be slow; match generous bridge-side OpenAI timeouts. */
  sock_wait_established(LLM_SK, 30, NULL, &status);
  DBGLOG("LLM_CONNECT: connected");
  llm_connected = 1;
  return 1;

sock_err:
  DBGLOGF("LLM_CONNECT: connection timeout/error status=%d", status);
  HLASKA("LLM: Nelze se pripojit ke bridge serveru.", 4);
  sock_close(LLM_SK);
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
  DWORD deadline_ms;
  WORD  n;

  if (timeout_sec < 1) timeout_sec = 1;
  deadline_ms = set_timeout(1000UL * (DWORD)timeout_sec);

  for (;;) {
    tcp_tick(LLM_SK);

    if (chk_timeout(deadline_ms)) {
      DBGLOG("llm_recv_line: timeout");
      return 0;
    }
    if (!tcp_established(&llm_sock)) {
      DBGLOG("llm_recv_line: connection closed by remote");
      llm_connected = 0;
      return 0;
    }

    n = sock_gets(LLM_SK, buf, (int)maxlen);
    if (n > 0)
      return n;
  }
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
 * LLM_SEND_RECV_WATT  (internal)
 * Send "USER <retezec1>\n", receive reply, fill dlouhe[], set pozodp=100.
 * Returns 1 for success (caller should goto OD), 0 to fall back to legacy.
 * ------------------------------------------------------------------------- */
static BYTE llm_send_recv_watt(void) {
  BYTE  msg[90];
  WORD  dlen;
  WORD  rlen;

  /* Brief waiting indicator (no "AI" prefix; reply text is shown verbatim). */
  STRANA(1);
  BARVA(barvapocitac1);
  NAPISRETEZEC("...", barvapocitac1);

  /* Build "USER <text>\n". */
  msg[0] = 'U'; msg[1] = 'S'; msg[2] = 'E'; msg[3] = 'R'; msg[4] = ' ';
  dlen = (WORD)strlen((char *)retezec1);
  if (dlen > 79) dlen = 79;
  memcpy(msg + 5, retezec1, (size_t)dlen);
  msg[5 + dlen] = '\n';
  dlen = (WORD)(6 + dlen);

  if (sock_write(LLM_SK, msg, (int)dlen) != (int)dlen) {
    DBGLOG("LLM_SEND_RECV: sock_write failed");
    llm_connected = 0;
    return 0;
  }
  DBGLOGF("LLM_SEND_RECV: sent %u bytes", (unsigned)dlen);

  /* Receive one reply line (OpenAI + tools can exceed 30s; bridge uses TIMEOUT_MS). */
  memset(dlouhe, 0, sizeof(dlouhe));
  rlen = llm_recv_line(dlouhe, (WORD)sizeof(dlouhe), 120);

  if (rlen == 0) {
    DBGLOG("LLM_SEND_RECV: no reply received");
    HLASKA("LLM: Zadna odpoved z bridge serveru.", 4);
    return 0;
  }

  DBGLOGF("LLM_SEND_RECV: received %u bytes: %.60s", (unsigned)rlen, (char *)dlouhe);

  /* Parse prefix: "REPLY " or "ERROR ". */
  if (strncmp((char *)dlouhe, "REPLY ", 6) == 0) {
    memmove(dlouhe, dlouhe + 6, (size_t)(rlen - 6 + 1));
    pozodp  = 100;        /* long-message path in ODPOVED() */
    pocetuzivvet++;       /* count this as a user turn */
    llm_odpoved_z_bridge = 1;
    return 1;
  }

  if (strncmp((char *)dlouhe, "ERROR ", 6) == 0) {
    HLASKA(dlouhe + 6, 4);
    return 0;
  }

  DBGLOGF("LLM_SEND_RECV: unexpected reply prefix: %.40s", (char *)dlouhe);
  return 0;
}

#endif /* POKYD_LLM_WATT */

/* ===========================================================================
 * Public interface — always compiled.  Stubs when POKYD_LLM_WATT is absent.
 * ========================================================================= */

/* LLM_INIT - parse host:port string and initialise Watt-32 stack.
 * Returns 1 on success, 0 on failure or when networking is not compiled in. */
BYTE LLM_INIT(BYTE *hostport) {
#ifdef POKYD_LLM_WATT
  return llm_init_watt(hostport);
#else
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
  BYTE   buf[4096];
  size_t tlen;

  if (!llm_enabled || !llm_connected || text == NULL || text[0] == 0)
    return;
  tlen = strlen((char *)text);
  if (tlen > (size_t)LLM_ASS_MAXPAY)
    tlen = (size_t)LLM_ASS_MAXPAY;
  memcpy(buf, "ASSISTANT ", 10);
  memcpy(buf + 10, text, tlen);
  buf[10 + tlen] = '\n';
  if (sock_write(LLM_SK, buf, (int)(11 + tlen)) != (int)(11 + tlen)) {
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

/* LLM_CLOSE - close the TCP socket and shut down the Watt-32 stack.
 * Called from PREDKONEC() / KONEC() on program exit. */
void LLM_CLOSE(void) {
#ifdef POKYD_LLM_WATT
  DBGLOGF("LLM_CLOSE e=%u c=%u", (unsigned)llm_enabled, (unsigned)llm_connected);
  if (llm_connected) {
    sock_close(LLM_SK);
    llm_connected = 0;
  }
  sock_exit();
  DBGLOG("LLM_CLOSE ok");
#endif
}
