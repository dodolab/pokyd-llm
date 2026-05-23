/* Tento zdrojovy kod je pod licenci GNU/GPL. Muzete ho pouzit k vlastni
   potrebe, ale nesmite jej ani programy zalozene na tomto kodu vyuzit komercne!

   Jedna se o zdrojovy kod programu Pokyd (http://iqpokyd.kyblsoft.cz)
   od Alese Jandy, aktivne vyvijeneho 1999 - 2002
*/

/*
 * pokyd.c - vstupni jednotka prekladace (Open Watcom); ostatni zdrojaky se vkladaji pres #include.
 * Rozsireni projektu: viz docs/prd.md, docs/CODEMAP.md, AGENTS.md a .cursor/rules/pokyd-*.mdc (jednotka prekladu, font INT 10h, Watcom).
 * Dulezite: po textmode()/INT 10h AX=0003 znovu NASTAVSPRAVNYFONT pred vypisem hlavicky (RAM font se smaze).
 */

#define krokovani 0
#define test 0
#define POCET_IQ 500

#include <stdio.h>
#include <string.h>
#include <conio.h>
#include <time.h>
#include <dos.h>
#include <i86.h>
#include <process.h>
#include <stdlib.h>
#include <direct.h>
#include <malloc.h>
#include <bios.h>
#include <stdarg.h>
/* Do not include <graph.h>: Watcom GraphPak (_setvideomode/_settextcolor/_setbkcolor) fights BIOS INT10h
   text output on DOSBox - black screen + flickering cursor while Pokyd writes via AH=09 elsewhere. */

/* Borland-style interrupt register compatibility for Open Watcom. */
static union REGPACK pokyd_regs;
static unsigned short pokyd_ss = 0;

#define _AX pokyd_regs.w.ax
#define _BX pokyd_regs.w.bx
#define _CX pokyd_regs.w.cx
#define _DX pokyd_regs.w.dx
#define _BP pokyd_regs.w.bp
#define _SI pokyd_regs.w.si
#define _DI pokyd_regs.w.di
#define _DS pokyd_regs.w.ds
#define _ES pokyd_regs.w.es
#define _SS pokyd_ss
#define _AH pokyd_regs.h.ah
#define _AL pokyd_regs.h.al
#define _BH pokyd_regs.h.bh
#define _BL pokyd_regs.h.bl
#define _CH pokyd_regs.h.ch
#define _CL pokyd_regs.h.cl
#define _DH pokyd_regs.h.dh
#define _DL pokyd_regs.h.dl
#define geninterrupt(no) intr((no),&pokyd_regs)

/* Logical 1..n maps to physical rows below COMMAND.COM output when non-zero. */
unsigned char pokyd_shell_rows = 0;
static unsigned char pokyd_do_consplit = 0;
static unsigned char pokyd_text_attr = 7;

/* Borland compatibility names used across included legacy units. */
#define C80 3 /* Watcom _TEXTC80: 80x25 color text - avoid pulling graph.h */
#define C4350 4350
#define ffblk find_t
#define ff_name name
#define ff_fsize size
#define findfirst(path,ff,attr) _dos_findfirst((path),(attr),(ff))
#define findnext(ff) _dos_findnext((ff))
#define setvect _dos_setvect
#define getvect _dos_getvect
#define __emit__(x) ((void)0)

/* --- Nahrazuje Borlandovske DOS rozhrani: disk, kurzor, videoram B800h, scroll (scroll pres INT 10h). --- */

static unsigned char getdisk(void) {
  unsigned drive;
  _dos_getdrive(&drive);
  return (unsigned char)(drive - 1);
}

static void setdisk(unsigned char drive) {
  unsigned drives;
  _dos_setdrive((unsigned)drive + 1,&drives);
}

static int getcurdir(int drive, char *buffer) {
  return (_getdcwd((int)drive + 1,buffer,64) == NULL) ? -1 : 0;
}

/* Watcom large-model malloc() lives in the FAR heap (DOS conventional memory).
 * _memavl() reports only the NEAR heap (DGROUP, <=64 KiB), which is essentially
 * full once Watt-32 globals are linked in - it then returns 0 even though the
 * far heap still has plenty of room. That regression made INTRO_NAPISPAMET show
 * "Pamet: 0 B" and, worse, made VTIPYSOUBOR compute maxvtipu==0 and then write
 * through a NULL pointer (smashing the DOS IVT - hard freeze on "Tridim vtipy").
 * Walking the far heap with _fheapwalk and adding the largest still-free DOS
 * block restores Borland's coreleft() semantics (memory available to malloc). */
static unsigned long coreleft(void) {
  struct _heapinfo hinfo;
  unsigned long total = 0;
  int rc;

  hinfo._pentry = NULL;
  for (;;) {
    rc = _fheapwalk(&hinfo);
    if (rc != _HEAPOK) break;
    if (hinfo._useflag == _FREEENTRY)
      total += (unsigned long)hinfo._size;
  }

  /* INT 21h AH=48h with BX=FFFFh: DOS leaves the largest free block size
   * (paragraphs) in BX. Counts memory the far heap can still grow into. */
  memset(&pokyd_regs, 0, sizeof(pokyd_regs));
  _AH = 0x48;
  _BX = 0xFFFF;
  geninterrupt(0x21);
  total += (unsigned long)_BX * 16UL;

  return total;
}

static unsigned char *searchpath(unsigned char *filename) {
  static char located[160];
  located[0]=0;
  _searchenv((char *)filename,"PATH",located);
  return (located[0] == 0) ? NULL : (unsigned char *)located;
}

static int wherex(void) {
  memset(&pokyd_regs, 0, sizeof(pokyd_regs));
  _AH = 0x03;
  _BH = 0;
  geninterrupt(0x10);
  return _DL + 1;
}

static int wherey(void) {
  int py;
  memset(&pokyd_regs, 0, sizeof(pokyd_regs));
  _AH = 0x03;
  _BH = 0;
  geninterrupt(0x10);
  py = _DH + 1;
  if (pokyd_shell_rows != 0) {
    if (py <= (int)pokyd_shell_rows) return 1;
    return py - (int)pokyd_shell_rows;
  }
  return py;
}

static void gotoxy(int x,int y) {
  int phys_y;
  if (x < 1) x = 1;
  else if (x > 80) x = 80;
  if (y < 1) y = 1;
  else if (y > 50) y = 50;
  phys_y = y;
  memset(&pokyd_regs, 0, sizeof(pokyd_regs));
  _AH = 0x02;
  _BH = 0;
  if (pokyd_shell_rows != 0 && y >= 1) phys_y = y + (int)pokyd_shell_rows;
  _DH = (unsigned char)(phys_y - 1);
  _DL = (unsigned char)(x - 1);
  geninterrupt(0x10);
}

/* Watcom cprintf("\n") does not advance the BIOS cursor used by wherex/wherey/gotoxy.
   STRANA() mixed the two - stale wherey() gave pozicedatumcas=0 and gotoxy(...,0) broke DH. */
static void pokyd_emit_nl(void) {
  memset(&pokyd_regs, 0, sizeof(pokyd_regs));
  _AH = 0x0E;
  _BH = 0;
  _AL = '\r';
  geninterrupt(0x10);
  memset(&pokyd_regs, 0, sizeof(pokyd_regs));
  _AH = 0x0E;
  _BH = 0;
  _AL = '\n';
  geninterrupt(0x10);
}

static void clrscr(void) {
  memset(&pokyd_regs, 0, sizeof(pokyd_regs));
  _AH = 0x06;
  _AL = 0;
  _BH = 0x07;
  if (pokyd_shell_rows != 0) {
    _CH = (unsigned char)pokyd_shell_rows;
    _CL = 0;
    _DH = 24;
    _DL = 79;
  } else {
    _CH = 0;
    _CL = 0;
    _DH = 24;
    _DL = 79;
  }
  geninterrupt(0x10);
  gotoxy(1,1);
}

static void pokyd_select_display_page0(void) {
  memset(&pokyd_regs, 0, sizeof(pokyd_regs));
  _AH = 0x05;
  _AL = 0x00;
  geninterrupt(0x10);
}

static void textcolor(int color) {
  pokyd_text_attr = (unsigned char)((pokyd_text_attr & 0xF0) | (color & 0x0F));
}

static void textbackground(int color) {
  pokyd_text_attr = (unsigned char)(((color & 0x0F) << 4) | (pokyd_text_attr & 0x0F));
}

static void textattr(int attr) {
  pokyd_text_attr = (unsigned char)attr;
}

static void textmode(int mode) {
  if (mode == C4350) {
    memset(&pokyd_regs, 0, sizeof(pokyd_regs));
    _AX = 0x0003;
    geninterrupt(0x10);
    memset(&pokyd_regs, 0, sizeof(pokyd_regs));
    _AX = 0x1112;
    _BL = 0;
    geninterrupt(0x10);
    memset(&pokyd_regs, 0, sizeof(pokyd_regs));
    _AX = 0x1202;
    _BL = 0x30;
    geninterrupt(0x10);
  } else {
    memset(&pokyd_regs, 0, sizeof(pokyd_regs));
    _AX = 0x0003;
    geninterrupt(0x10);
  }
}

static int bioskey(int cmd) {
  union REGS regs;
  regs.h.ah = (unsigned char)cmd;
  int86(0x16,&regs,&regs);
  return regs.x.ax;
}

static int gettext(int left,int top,int right,int bottom,void *buffer) {
  unsigned short far *video = (unsigned short far *)MK_FP(0xB800,0);
  unsigned short *dst = (unsigned short *)buffer;
  int x,y;
  if (pokyd_shell_rows != 0) {
    top += (int)pokyd_shell_rows;
    bottom += (int)pokyd_shell_rows;
  }
  for (y=top; y<=bottom; y++) {
    for (x=left; x<=right; x++) {
      *dst++ = video[(y-1)*80 + (x-1)];
    }
  }
  return 1;
}

static int puttext(int left,int top,int right,int bottom,void *buffer) {
  unsigned short far *video = (unsigned short far *)MK_FP(0xB800,0);
  unsigned short *src = (unsigned short *)buffer;
  int x,y;
  if (pokyd_shell_rows != 0) {
    top += (int)pokyd_shell_rows;
    bottom += (int)pokyd_shell_rows;
  }
  for (y=top; y<=bottom; y++) {
    for (x=left; x<=right; x++) {
      video[(y-1)*80 + (x-1)] = *src++;
    }
  }
  return 1;
}

static FILE *pokyd_dbglog;

static void DBGLOG(const char *msg) {
  if (pokyd_dbglog == NULL) {
    /* FAT often shows DEBUG.LOG; line-buffering can lose lines if the app stops cold */
    pokyd_dbglog = fopen("DEBUG.LOG", "a");
    if (pokyd_dbglog == NULL)
      pokyd_dbglog = fopen("debug.log", "a");
    if (pokyd_dbglog != NULL)
      setvbuf(pokyd_dbglog, NULL, _IONBF, 0);
  }
  if (pokyd_dbglog != NULL) {
    fprintf(pokyd_dbglog, "%s\n", msg);
    fflush(pokyd_dbglog);
  }
}

static void DBGLOGF(const char *fmt, ...) {
  char buf[512];
  va_list ap;
  va_start(ap, fmt);
  vsprintf(buf, fmt, ap);
  va_end(ap);
  DBGLOG(buf);
}

/* Forward decl for INTRO KONEC (included below); definition after all includes. */
/* Ukonceni INTRO: bezpecny prechod na stejny stack jako main (kvuli velkemu lokalnimu ramu INTRO). */
static void pokyd_finish_intro_handoff(void);
/* Pokracovani main po INTRO: textmode vymaze uzivatelske fonty - nutne NASTAVSPRAVNYFONT pred hlavickou. */
static void pokyd_run_after_intro(void);

static int pokyd_main_argc;
static char **pokyd_main_argv;

#include "pokyd_pr.c"
#include "pokyd.h"
#include "pokyd_za.c"
#include "pokyd_v0.c"
#include "pokyd_v1.c"
#include "pokyd_v2.c"
#include "pokyd_v3.c"
#include "pokyd_fn.c"
#include "pokyd_na.c"
#include "pokyd_sl.c"
#include "pokyd_llm.c"

/* Po INTRO: synchronizace video rezimu s delkastrany, ROM font pokud font==1 (detailni glyfy az v pokyd_run_after_intro). */
static void pokyd_finish_intro_handoff(void) {
  grafika25 = 0;
  if (pokyd_shell_rows == 0) {
    memset(&pokyd_regs, 0, sizeof(pokyd_regs));
    pokyd_regs.w.ax = 0x0003;
    intr(0x10, &pokyd_regs);
    if (delkastrany > 24) {
      delkastrany = 24;
      pozicehlavicka = 1;
      pozicedatumcas = 2;
     }
    SMAZKURZOR();
    if (font == 1) NASTAVNORMALNIFONT();
   } else {
    NASTAVSPRAVNYMOD();
   }
 }

static void pokyd_run_after_intro(void) {
  int argc = pokyd_effective_argc;
  char **argv = pokyd_main_argv;

/* Hlavni smycka konverzace zacina zde po uvodni obrazovce. */
DBGLOG("main: INTRO returned");
textmode(C80);
clrscr();
pokyd_select_display_page0();
DBGLOG("main: after textmode+clrscr");

if (mod == 25 && delkastrany == 49) { SMAZOBRAZOVKU(1); pozicehlavicka=1; pozicedatumcas=2; delkastrany=24; }
else if (mod == 50 && delkastrany == 24) {
  _AX = 0x1A00; geninterrupt(0x10); if (_AL != 0x1A) {
    HLASKA("50ti radkovy mod je podporovan pouze na kartach VGA a vyssich!",4);
    mod=25;
    HLASKA("Nastaveni pocatecniho poctu radek bylo zmeneno na 25.",1);
   }
  else {
    NASTAV50RADKU(); delkastrany=49;
   }
 }
/* textmode(C80) reloads the ROM font and clears RAM glyph patches from INTRO;
   republish Pokyd slots (s/Y/y) before the header uses VRATDIAKRITIKU(). */
NASTAVSPRAVNYFONT();
NAPISHLAVICKOVYRADEK();
DBGLOG("main: after NAPISHLAVICKOVYRADEK");
/* LLM TCP connect is deferred to first LLM_SEND_* call (lazy in pokyd_llm.c). */
if (llm_enabled) {
  DBGLOGF("main: LLM enabled, bridge %s:%u (connect on first use)", (char *)llm_host, (unsigned)llm_port);
 }
DBGLOGF("main: video state after header wherex=%d wherey=%d delkastrany=%u",
        wherex(), wherey(), (unsigned)delkastrany);

SMAZKURZOR();
jeli_nastaveni=1;		//0 - nepsat cas, 1 - normal, 2 - nastaveni
STRANA(1);
BARVA(13); STRANA(1);
{ int py = wherey();
  pozicedatumcas = (BYTE)((py > 1) ? py - 1 : 2);
}
ZAPISCAS();
stranaradek=0;
{ int ty = wherey();
  if (ty < 2) ty = 2;
  gotoxy(1, ty - 1);
 }
zapisovani=1;
DBGLOGF("main: input line cursor wherex=%d wherey=%d pozicedatumcas=%u barvapocitac1=%u (PC green default 10)",
        wherex(), wherey(), (unsigned)pozicedatumcas, (unsigned)barvapocitac1);

puvodnicas=time(NULL);
DBGLOG("main: entering ZACATEK");

ZACATEK:
barvapocitac0=barvapocitac1;
ZAPIS_NALADU();
odpovedi[0][0]=0;

ZACATECNIK("Pocitac te nejdrive uvita a zacne hovor.",01);

CAS(1);	ZAPIS_NALADU();		//kvuli prepsani zacatecnickym textem
textcolor(11);
if (llm_enabled && !llm_connected) {
  DBGLOG("main: LLM connect before welcome/VTIPY");
  if (LLM_CONNECT()) {
    LLM_SEND_CONFIG();
    LLM_ZOBRAZ_PRIPOJENO();
    DBGLOG("main: LLM bridge ready");
  } else {
    LLM_ZOBRAZ_CHYBU((BYTE *)"LLM: Bridge nedostupny.");
    DBGLOG("main: LLM bridge not ready yet (classic fallback)");
  }
}
DBGLOG("main: before optional startup messages");
DBGLOGF("main: svtipy=%d spocasi=%d celkemvtipu=%lu llm=%u/%u",
        (int)svtipy, (int)spocasi, (unsigned long)celkemvtipu,
        (unsigned)llm_enabled, (unsigned)llm_connected);
/* LLM mode: first line is the welcome initiative, not startup joke/weather/resume. */
if (llm_enabled == 0) {
  if (svtipy == 1 || (svtipy == 2 && rand()%2 == 0)) {
    DBGLOG("main: calling VTIPY() (startup joke path)");
    pozodp=1;
    VTIPY();
    DBGLOGF("main: after VTIPY pozodp=%u skutecnychodp=%u", (unsigned)pozodp, (unsigned)skutecnychodp);
   }
  if (spocasi == 1 || (spocasi == 2 && rand()%2 == 0)) {
    DBGLOG("main: calling POCASI()+ODPOVED (startup weather path)");
    pozodp=1;
    POCASI();
    ODPOVED(1);
    DBGLOGF("main: after POCASI ODPOVED pozodp=%u", (unsigned)pozodp);
   }
 }
VYNULUJ_ODPOVEDI();
DBGLOGF("main: after optional startup messages pozodp=%u skutecnychodp=%u", (unsigned)pozodp, (unsigned)skutecnychodp);

if (argc > 1) {
  cislo=0; retezec1[0]=0;
  while (++cislo != argc) {
    if (strlen(retezec1)+strlen(argv[cislo]) > 79) {
      argv[cislo][79-strlen(retezec1)]=0;
      strcat(retezec1,argv[cislo]);
      goto ZAARGC;
     }
    if (argv[cislo][0] != '-' && argv[cislo][0] != '/') {
      strcat(retezec1,argv[cislo]);
      strcat(retezec1," ");
     }
   }
  { size_t rl = strlen(retezec1);
    if (rl > 0) retezec1[rl-1]=0;
   }
ZAARGC:
  /* Switches only: do not skip EXTRA_VETA / initial OD (otherwise no opening reply). */
  if (retezec1[0] != 0) {
    STRANA(pocetradku); pocetuzivvet++;
    retezec1[79]=0; pocetvet=1;
    NAPISRETEZEC(retezec1,barvaclovek);
    if (kydy != NULL) fprintf(kydy,"C: %s\n",retezec1);
    SETRID();
    strcpy(puvretezec,retezec1);
    goto POSTARTU;
   }
 }

if (llm_enabled == 0 && pocetsouboru > 0 && pocetsouboru < 1000 && rand()%2 == 0) {
  VYNULUJ_ODPOVEDI();
  EXTRA_VETA(1);
  sprintf(predtimretezec,odpovedi[0],pocetsouboru+1); strcpy(odpovedi[0],predtimretezec);
  ODPOVED(1);
  VYNULUJ_ODPOVEDI(); odpovedi[0][0]=0;
 }

DBGLOG("main: before welcome / EXTRA_VETA 7/8");
{ BYTE llm_startup_welcome = 0;
if (llm_enabled != 0) {
  if (LLM_SEND_INITIATIVE((BYTE *)"welcome", 0) != 0) {
    if (llm_connected) LLM_ZOBRAZ_PRIPOJENO();
    DBGLOG("main: LLM welcome (replaces EXTRA_VETA 7/8)");
    ODPOVED(1);
    VYNULUJ_ODPOVEDI();
    llm_startup_welcome = 1;
   }
  else {
    DBGLOG("main: LLM welcome failed, classic EXTRA_VETA fallback");
    EXTRA_VETA(7);
    EXTRA_VETA(8);
   }
 }
else {
  EXTRA_VETA(7);
  DBGLOGF("main: after EXTRA_VETA(7) pozodp=%u skutecnychodp=%u", (unsigned)pozodp, (unsigned)skutecnychodp);
  EXTRA_VETA(8);
  DBGLOGF("main: after EXTRA_VETA(8) pozodp=%u skutecnychodp=%u", (unsigned)pozodp, (unsigned)skutecnychodp);
 }
{ unsigned ip;
  DBGLOGF("main: welcome queue pozodp=%u barvapocitac0 will be=%u", (unsigned)pozodp, (unsigned)barvapocitac1);
  for (ip = 0; ip < (unsigned)pozodp && ip < 8u; ip++)
    DBGLOGF("main: odpovedi[%u]=\"%.200s\"", ip, (char *)odpovedi[ip]);
 }

docasnenaladabody=0;
if (llm_startup_welcome != 0) {
  DBGLOG("main: LLM welcome shown, skip empty initial ODPOVED");
  goto START;
 }
}
DBGLOG("main: before initial goto OD");

goto OD;


START:
naladabody+=docasnenaladabody;
if (naladabody < 0) naladabody=0; if (naladabody > 80) naladabody=80;   //nesmi presahnout
if (charakter != 0) nalada=naladabody/15; if (nalada > 4) nalada=4;
ZAPIS_NALADU();
if (samomluva == 0) {
  VYNULOVANI(0);
  retezec1[0]=0; retezec2[0]=0;
  strcpy(predtimretezec,puvretezec);
  if (pocetuzivvet == 0) {
    ZACATECNIK("Ted napises to, co pocitaci chces sdelit, popr. odpovedet, zeptat se atd.",02);
    ZACATECNIK("Poznamka: Pokud chces napsat otazku, musis za ni napsat otaznik!",03);
   } else ZACATECNIK("Tak to pokracuje stale dal. Normalni napoveda se ti ukaze, kdyz stisknes F1.",04);
  NAPIS(); VYNULUJ_ODPOVEDI();
 }
else {
  if (kbhit() != 0) {				//konec samomluvy
KONECSAMOML: strcpy(puvretezec,textpredsamomluvou);
    naladabody=samomlbody;				//vraceni do puvodniho stavu
    samomluva=0; goto START;
   }
  ZMEN_POCITAC();
  if (llm_enabled != 0 &&
      LLM_INITIATIVE_SHOW((BYTE *)"samomluva", 0, 0) != 0) {
    { DWORD cek;
      DBGLOG("main: LLM samomluva");
      strcpy(retezec2, odpovedi[cislo]);
      for (cek = strlen((char *)dlouhe) * 50 + 300; cek > 0; cek -= 50) {
        CEKEJ(50); if (kbhit() != 0) goto KONECSAMOML;
       }
     }
    goto START;
   }
  strcpy(predtimretezec,retezec1);
  strcpy(retezec1,odpovedi[cislo]);		//minula odpoved je prikazem
  strcpy(puvretezec,retezec1);
  VYNULOVANI(1); strcpy(retezec1,puvretezec);
  SMAZKURZOR();
  for (cislo=strlen(retezec1)*50+300; cislo > 0; cislo-=50) {
    CEKEJ(50); if (kbhit() != 0) goto KONECSAMOML;
   }
 }

#if test == 1
for (testcyklus=0; testcyklus < 10000; testcyklus++) {
#endif

SETRID();
/* LLM hook: if connected, forward the user's sentence to the bridge instead of
   running the rule engine. On success, dlouhe[] is filled and pozodp==100;
   goto OD triggers ODPOVED() via the long-message path (same as VTIPY/POCASI).
   On failure (network error, bridge down) execution falls through to the legacy
   rule engine transparently - LLM mode is non-destructive. */
if (llm_enabled && LLM_SEND_RECV()) goto OD;
POSTARTU:

#if krokovani == 0

if (retezec1[0] == 'c' && retezec1[1] == 'h' &&		//cheat
 retezec1[2] == '7' && retezec1[3] == '0' && retezec1[4] == 0 &&  //verze 7.0
 vypnutecheaty == 0) {
  #include "pokydx.h"
 }

if (retezec1[0] == '*' && vypnutecheaty == 0) {		//cheat
  if (strcmp(retezec1+1,"pamrd1") == 0 && nadavani == 0) {
    HLASKA("Budu ti tak nadavat, az to nebude hezky!",5); nadavani=1; goto START;
   }
  else if (strcmp(retezec1+1,"pamrd0") == 0 && nadavani == 1) {
    HLASKA("Tak dobre! Nebudu ti nadavat! Jsem totiz strasne hodnej pocitac!",5); nadavani=0; goto START;
   }
 }

if (EXTRA_SANCE_CHEAT() == 1) goto START;

if (nadavani == 1) {
  { BYTE puv_nalada_insult = nalada;
    nalada = (BYTE)(rand() % 5);
    if (llm_enabled != 0 &&
        LLM_INITIATIVE_SHOW((BYTE *)"insult", 0, 1) != 0) {
      DBGLOG("main: LLM insult (replaces EXTRA_VETA 16/17)");
     }
    else {
      EXTRA_VETA(16); EXTRA_VETA(17);
      ODPOVED(1);
     }
    nalada = puv_nalada_insult;
    nadavani = 1;
   }
  goto START;
 }

if (SLOVO("spust") != 255) {			//spousteni programu
  SPUSTPROGRAM(); goto START;
 }

if (((SLOVO("credits") != 255 || SLOVO("help") != 255 || SLOVO ("pomoc") != 255) && pocetslov==1) || (otazka==1 && pocetslov==0) || (SLOVO("kdo") != 255 && SLOVO("te") != 255 && (SLOVO("udelal") != 255 || SLOVO("vyrobil") != 255 || SLOVO("naprogramoval") != 255 || SLOVO("vymyslel") != 255)))
 { PRECTIOBRAZOVKU(1); CTISOURADNICE(); HELPF1(1); NASTAVSPRAVNYMOD(); ZAPISOBRAZOVKU(); VLOZSOURADNICE();
 smyslpocvety=2; goto START; }

if ((SLOVO("obnov") != 255 || SLOVO("nacti") != 255) && (SLOVO("font") != 255 || SLOVO("pismo") != 255)) {
  OBNOV_OBRAZOVKU(0); goto START;
 }
if (ZACATEK("nastav") != 255 && SLOVO("uloz") == 255) {
  if (ZACATEK("normal") != 255 || ZACATEK("standar") != 255) {
    NASTAV_STANDARD(); NASTAVSOUBOR(1); if (psani == 2) psani=1; goto START;
   }
  else { PRECTIOBRAZOVKU(1); NASTAV(); ZAPISOBRAZOVKU(); NAPISHLAVICKOVYRADEK(); goto START; }
 }
#endif

if (ZACATEK("uloz") != 255 && (SLOVO("nastaveni") != 255 || SLOVO("fakta") != 255 || ZACATEK("slov") != 255 || SLOVO("konfiguraci") != 255 || SLOVO("profil") != 255)) {
 ZAPIS_INFORMACE_O_VETACH(2,0); NASTAVSOUBOR(1);
 if (psani == 2) { strcpy(odpovedi[0],"Psani do souboru se projevi az pri pristim spusteni Pokydu."); goto OD; }
 else goto START; }

#if krokovani == 0
if (SLOVO("pocasi") != 255 || (ZACATEK("jak") != 255 && SLOVO("bude") != 255)) { ZACATECNIK("Timto prikazem pokladas otazku, jake bude pocasi.",41); POCASI(); goto OD; }
if (SLOVO("formatuj") != 255 || SLOVO("zformatuj") != 255) { ZACATECNIK("Nyni se (jako) zformatuje harddisk.",42); FORMATOVANI(); goto OD; }
if ((ZACATEK("smaz") != 255 || ZACATEK("vymaz") != 255) && (SLOVO("soubory") != 255 || SLOVO("kydy") != 255)) {
  if (readonlymod == 1) {
    HLASKA("Pokyd je v modu \"jen ke cteni\". Soubory nemuzou byt smazany.",4);
    goto START;
   }
  ZACATECNIK("Po tomto prikaze se vymazou vsechny soubory s hovorem mimo aktualniho.",43);
  PIS(1,wherey(),"Mazu soubory, prosim cekejte... ",15+2*16); SMAZ_SOUBORY(1);
  BARVA(barvaclovek); NAPISRETEZEC(retezec1,barvaclovek);
  goto OD;
 }

if (SLOVO("zmen") != 255 && (ZACATEK("obrazovk") != 255 || SLOVO("mod") != 255 || ZACATEK("radk") != 255)) {
  ZMENMOD(); goto START;
 }

if ((SLOVO("testuj") != 255 || SLOVO("otestuj") != 255) && (SLOVO("se") != 255 || SLOVO("pokyd") != 255 || SLOVO("sebe") != 255 || SLOVO("program") != 255)) {
  ZACATECNIK("Program TestPkd nyni otestuje, zda jsou vsechny soubory k Pokydu v poradku.",44);
  CTISOURADNICE(); PRECTIOBRAZOVKU(1); SMAZOBRAZOVKU(0);
  NASTAVSPRAVNYFONT(); SOUBOR("TESTPKD.EXE");
  SPUST(soubor,1,1); NASTAVSPRAVNYMOD(); ZAPISOBRAZOVKU(); VLOZSOURADNICE(); goto START;
 }

if (SLOVO("rekni") != 255)
 if (ZACATEK("vtip") != 255 || ZACATEK("ftip") != 255 || ZACATEK("for") != 255 || ZACATEK("anekdot") != 255) {
   ZACATECNIK("Po tomto prikaze se vzdy napise jeden z mnoha vtipu, ktere jsou ve VTIPY.TXT.",45);
   VTIPY(); goto START;
  }

#endif

ZJISTI_SLOVO_PAMET();
ZJISTI_JSEM_PAMET();
ZJISTI_JSI_PAMET();

CTIPODMINKY();

ZACATECNIK("Pocitac ti nyni na tvoji vetu odpovi.",05);

#if test == 1
}						//dalsi cyklus testovani
#endif

OD:
DBGLOGF("main: at OD before ODPOVED pozodp=%u barvapocitac1=%u barvapocitac2=%u wherey=%d",
        (unsigned)pozodp, (unsigned)barvapocitac1, (unsigned)barvapocitac2, wherey());
if (cisloaktualnihopocitace == 1) barvapocitac0=barvapocitac1;
else barvapocitac0=barvapocitac2;
DBGLOGF("main: ODPOVED(1) barvapocitac0=%u", (unsigned)barvapocitac0);
ODPOVED(1);
DBGLOG("main: at OD after ODPOVED");
CAS(0);
if (konec != 1) {
  ZKONTROLUJ_EXTRA_SANCI(0);
  goto START;
 }
DBGLOGF("main: quit k=%d llm=%u/%u", (int)konec, (unsigned)llm_enabled, (unsigned)llm_connected);
ZAVRISOUBORSKYDAMA("\nProgram Pokyd byl ukoncen.",1);
ZAPIS_INFORMACE_O_VETACH(3,1);
STRANA(1);
LLM_CLOSE();
PREDKONEC();
KONEC();
}

/* Vstup programu: prepinace pred INTRO, pak INTRO ? pokyd_run_after_intro (normalne se sem nevraci). */
int main(int argc, char *argv[]) {
pokyd_main_argc = argc;
pokyd_main_argv = argv;
pokyd_shell_rows = 0;
pokyd_do_consplit = 0;
if (argc > 1) {
  docasnenaladabody=0;				//docasne pouziti
  for (cislo=1; cislo < argc; cislo++) {
    strcpy(dlouhe,argv[cislo]); if (dlouhe[0] == '/') dlouhe[0]='-';
    if (dlouhe[0] == '"') memmove(dlouhe, dlouhe + 1, strlen((char *)dlouhe));
    { WORD qlen = (WORD)strlen((char *)dlouhe);
      if (qlen > 0 && dlouhe[qlen - 1] == '"') dlouhe[qlen - 1] = 0;
    }
    if (stricmp(dlouhe,"-test") == 0) introakcespusteni=1;
    else if (stricmp(dlouhe,"-setric") == 0) introakcespusteni=2;
    else if (stricmp(dlouhe,"-uloz") == 0) introakcespusteni=3;
    else if (stricmp(dlouhe,"-napoveda") == 0) introakcespusteni=4;
    else if (stricmp(dlouhe,"-?") == 0) introakcespusteni=4;
    else if (stricmp(dlouhe,"-pluginy") == 0) introakcespusteni=5;
    else if (stricmp(dlouhe,"-nastaveni") == 0) introakcespusteni=6;
    else if (stricmp(dlouhe,"-pokyd") == 0) introakcespusteni=7;
    else if (stricmp(dlouhe,"-zacatecnik") == 0) introakcespusteni=8;
    else if (stricmp(dlouhe,"-consplit") == 0) pokyd_do_consplit=1;
    else if (stricmp(dlouhe,"-readonly") == 0) readonlymod=1;
    else if (stricmp(dlouhe,"-masterboot") == 0) masterboot=1;
    else if (stricmp(dlouhe,"-bezcheatu") == 0) vypnutecheaty=1;
    else if (stricmp(dlouhe,"-bezvsechcheatu") == 0) vypnutecheaty=2;
    else if (strnicmp(dlouhe,"-llm",4) == 0) {
      /* Accept: -llm=host:port  or  -llm:host:port  or  -llm host:port
         or  -llm host port (DOS-safe: COMMAND.COM treats colons specially). */
      BYTE *sep = (BYTE *)dlouhe + 4;
      BYTE hostport_buf[80];
      if (*sep == '=' || *sep == ':') {
        sep++;
        if (*sep) LLM_INIT(sep);
        else if (cislo + 1 < argc) LLM_INIT((BYTE *)argv[++cislo]);
      } else if (*sep == 0 && cislo + 2 < argc) {
        WORD hlen = (WORD)strlen((char *)argv[cislo + 1]);
        WORD plen = (WORD)strlen((char *)argv[cislo + 2]);
        if (hlen + 1 + plen + 1 <= (WORD)sizeof(hostport_buf)) {
          sprintf((char *)hostport_buf, "%s:%s", argv[cislo + 1], argv[cislo + 2]);
          LLM_INIT(hostport_buf);
          cislo += 2;
        }
      } else if (*sep == 0 && cislo + 1 < argc) {
        LLM_INIT((BYTE *)argv[++cislo]);
      }
     }
    else docasnenaladabody=1;
   }
  if (docasnenaladabody == 0) argc=0;
 }
pokyd_effective_argc = argc;
if (pokyd_do_consplit) {
  int ry, rx;
  memset(&pokyd_regs, 0, sizeof(pokyd_regs));
  _AH = 0x03;
  _BH = 0;
  geninterrupt(0x10);
  ry = _DH + 1;
  rx = _DL + 1;
  pokyd_shell_rows = (unsigned char)ry;
  if (rx > 1) pokyd_shell_rows++;
  if (pokyd_shell_rows > 12 || (25 - (int)pokyd_shell_rows) < 12) pokyd_shell_rows = 0;
}
DBGLOGF("main: argc=%d llm=%u p=%u", argc, (unsigned)llm_enabled, (unsigned)llm_port);
DBGLOG("main: start");
NASTARTUJ_PROGRAM();
DBGLOG("main: after NASTARTUJ_PROGRAM");
/* Junk argv leaves argc>=2 but INTRO must still wait for a key; keep real argc for argv
   handling below (e.g. initial sentence from command line). */
{ int intro_argc = argc;
  if (argc > 1 && docasnenaladabody != 0) intro_argc = 1;
  pokyd_intro_argc_snapshot = intro_argc;
  DBGLOGF("main: INTRO arg=%d", intro_argc);
  INTRO(intro_argc, pozicehlavicka);
  DBGLOG("main: INTRO ret?");
 }
 /* Normal path: INTRO calls pokyd_run_after_intro() then KONEC/exit(0); never reaches here. */
 return 0;
}
