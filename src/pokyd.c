/* Tento zdrojovù kùd je pod licencù GNU/GPL. Mùùete ho pouùùt k vlastnù
   potùebù, ale nesmùte jej ani programy zaloùenù na tomto kùdu vyuùùt komerùnù!

   Jednù se o zdrojovù kùd programu Pokyd (http://iqpokyd.kyblsoft.cz)
   od Aleùe Jandy, aktivnù vyvùjenùho 1999 - 2002
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
#include <graph.h>
#include <malloc.h>
#include <bios.h>

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

/* Borland compatibility names used across included legacy units. */
#define C80 _TEXTC80
#define C4350 4350
#define ffblk find_t
#define ff_name name
#define ff_fsize size
#define findfirst(path,ff,attr) _dos_findfirst((path),(attr),(ff))
#define findnext(ff) _dos_findnext((ff))
#define setvect _dos_setvect
#define getvect _dos_getvect
#define __emit__(x) ((void)0)

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

static unsigned long coreleft(void) {
  return _memavl();
}

static unsigned char *searchpath(unsigned char *filename) {
  static char located[160];
  located[0]=0;
  _searchenv((char *)filename,"PATH",located);
  return (located[0] == 0) ? NULL : (unsigned char *)located;
}

static int wherex(void) {
  _AH = 0x03;
  _BH = 0;
  geninterrupt(0x10);
  return _DL + 1;
}

static int wherey(void) {
  _AH = 0x03;
  _BH = 0;
  geninterrupt(0x10);
  return _DH + 1;
}

static void gotoxy(int x,int y) {
  _AH = 0x02;
  _BH = 0;
  _DH = (unsigned char)(y - 1);
  _DL = (unsigned char)(x - 1);
  geninterrupt(0x10);
}

static void clrscr(void) {
  _AH = 0x06;
  _AL = 0;
  _BH = 0x07;
  _CH = 0;
  _CL = 0;
  _DH = 24;
  _DL = 79;
  geninterrupt(0x10);
  gotoxy(1,1);
}

static void textcolor(int color) {
  _settextcolor((grcolor)color);
}

static void textbackground(int color) {
  _setbkcolor((long)color);
}

static void textattr(int attr) {
  textbackground((attr >> 4) & 0x0F);
  textcolor(attr & 0x0F);
}

static void textmode(int mode) {
  if (mode == C4350) {
    _AX = 0x0003;
    geninterrupt(0x10);
    _AX = 0x1112;
    _BL = 0;
    geninterrupt(0x10);
    _AX = 0x1202;
    _BL = 0x30;
    geninterrupt(0x10);
  } else {
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
  for (y=top; y<=bottom; y++) {
    for (x=left; x<=right; x++) {
      video[(y-1)*80 + (x-1)] = *src++;
    }
  }
  return 1;
}

static void DBGLOG(const char *msg) {
  FILE *dbg = fopen("debug.log","a");
  if (dbg != NULL) {
    fprintf(dbg,"%s\n",msg);
    fclose(dbg);
  }
}

#include "pokyd.pr"
#include "pokyd.h"
#include "pokyd.za"
#include "pokyd.v0"
#include "pokyd.v1"
#include "pokyd.v2"
#include "pokyd.v3"
#include "pokyd.fn"
#include "pokyd.na"
#include "pokyd.sl"

void main(int argc, char *argv[]) {
DBGLOG("main: start");
NASTARTUJ_PROGRAM();
DBGLOG("main: after NASTARTUJ_PROGRAM");
if (argc > 1) {
  docasnenaladabody=0;				//docasne pouziti
  for (cislo=1; cislo < argc; cislo++) {
    strcpy(dlouhe,argv[cislo]); if (dlouhe[0] == '/') dlouhe[0]='-';
    if (stricmp(dlouhe,"-test") == 0) introakcespusteni=1;
    else if (stricmp(dlouhe,"-setric") == 0) introakcespusteni=2;
    else if (stricmp(dlouhe,"-uloz") == 0) introakcespusteni=3;
    else if (stricmp(dlouhe,"-napoveda") == 0) introakcespusteni=4;
    else if (stricmp(dlouhe,"-?") == 0) introakcespusteni=4;
    else if (stricmp(dlouhe,"-pluginy") == 0) introakcespusteni=5;
    else if (stricmp(dlouhe,"-nastaveni") == 0) introakcespusteni=6;
    else if (stricmp(dlouhe,"-pokyd") == 0) introakcespusteni=7;
    else if (stricmp(dlouhe,"-zacatecnik") == 0) introakcespusteni=8;
    else if (stricmp(dlouhe,"-readonly") == 0) readonlymod=1;
    else if (stricmp(dlouhe,"-masterboot") == 0) masterboot=1;
    else if (stricmp(dlouhe,"-bezcheatu") == 0) vypnutecheaty=1;
    else if (stricmp(dlouhe,"-bezvsechcheatu") == 0) vypnutecheaty=2;
    else docasnenaladabody=1;
   }
  if (docasnenaladabody == 0) argc=0;
 }
INTRO: DBGLOG("main: before INTRO");
INTRO(argc,pozicehlavicka);
DBGLOG("main: after INTRO");
textbackground(0);

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
NAPISHLAVICKOVYRADEK();

SMAZKURZOR();
jeli_nastaveni=1;		//0 - nepsat cas, 1 - normal, 2 - nastaveni
STRANA(1);
BARVA(13); STRANA(1);
pozicedatumcas=wherey()-1;
ZAPISCAS();
stranaradek=0; gotoxy(1,wherey()-1); zapisovani=1;

puvodnicas=time(NULL);

ZACATEK:
barvapocitac0=barvapocitac1;
ZAPIS_NALADU();
odpovedi[0][0]=0;

ZACATECNIK("Pocitac te nejdrive uvita a zacne hovor.",01);

CAS(1);	ZAPIS_NALADU();		//kvuli prepsani zacatecnickym textem
textcolor(11);
if (svtipy == 1 || (svtipy == 2 && rand()%2 == 0)) { pozodp=1; VTIPY(); }
if (spocasi == 1 || (spocasi == 2 && rand()%2 == 0)) { pozodp=1; POCASI(); ODPOVED(1); }
VYNULUJ_ODPOVEDI();

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
  retezec1[strlen(retezec1)-1]=0;
ZAARGC:
  STRANA(pocetradku); pocetuzivvet++;
  retezec1[79]=0; pocetvet=1;
  NAPISRETEZEC(retezec1,barvaclovek);
  if (kydy != NULL) fprintf(kydy,"C: %s\n",retezec1);
  SETRID();
  strcpy(puvretezec,retezec1);
  goto POSTARTU;
 }

if (pocetsouboru > 0 && pocetsouboru < 1000 && rand()%2 == 0) {
  VYNULUJ_ODPOVEDI(); EXTRA_VETA(1);
  sprintf(predtimretezec,odpovedi[0],pocetsouboru+1); strcpy(odpovedi[0],predtimretezec);
  ODPOVED(1); VYNULUJ_ODPOVEDI(); odpovedi[0][0]=0;
 }

EXTRA_VETA(7); EXTRA_VETA(8);

docasnenaladabody=0;

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
  nadavani=nalada; nalada=rand()%5; EXTRA_VETA(16); EXTRA_VETA(17);
  ODPOVED(1); nalada=nadavani; nadavani=1; goto START;
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
if (cisloaktualnihopocitace == 1) barvapocitac0=barvapocitac1;
else barvapocitac0=barvapocitac2;
ODPOVED(1);
CAS(0);
if (konec != 1) {
  ZKONTROLUJ_EXTRA_SANCI(0);
  goto START;
 }
ZAVRISOUBORSKYDAMA("\nProgram Pokyd byl ukoncen.",1);
ZAPIS_INFORMACE_O_VETACH(3,1);
STRANA(1);
PREDKONEC();
KONEC();
}
