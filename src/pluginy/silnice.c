#include <stdio.h>
#include <conio.h>
#include <dos.h>

#define BYTE unsigned char
#define WORD unsigned short
#define DWORD unsigned long

BYTE prekazky[22][81],smery[21],rychlejsi[21],soubor[80];
BYTE pozicex,pozicey,cas,demo,soucet;
BYTE rychlost,hustota,level,zivoty,rychlostdema,maxlevel=0,zvuk=1,stiny=0;
WORD body;
DWORD democas;

FILE *f;

struct REGPACK reg;

#include "silnice.fn"
#include "silnice.h"
#include "silnice.tab"

void main(void) {
VYBER:
_AX = 3; geninterrupt(0x10); textattr(15);
VYBER2: gotoxy(1,1);
cprintf("Silnice v1.1 - autor Ales Janda  KYBLSoft 2000\n\n\r"
        "   S - Start nove hry  \n\r"
        "   D - Demo            \n\r"
        "   T - Tabulka         \n\r"
        "   K - Konec           \n\n\r"
        " Nastaveni:\n\r"
        "  Zvuk  - %s (Z - zmenit)\n\r"
        "  Stiny - %s (N - zmenit)\n\n\r"
        "Tak co chces? ",zvuk == 1 ? "ano" : "ne ",stiny == 1 ? "ano" : "ne ");
srand(CAS18());
GETCHZAC: switch(getch()) {
  case 's': case 'S': case '\r': demo=0; goto ZACATEK;
  case 'd': case 'D': demo=1; goto ZACATEK;
  case 't': case 'T': TABULKA(0,0); getch(); goto VYBER;
  case 'k': case 'K': case 0x1B: goto KONEC;
  case 'z': case 'Z': zvuk^=1; goto VYBER2;
  case 'n': case 'N': stiny^=1; goto VYBER2;
  default: goto GETCHZAC;
 }

ZACATEK:
_AX = 3; geninterrupt(0x10); _setcursortype(_NOCURSOR); textcolor(15);

textbackground(2); cprintf(" Silnice v1.1 - Ales Janda  KYBLSoft 2000 ");
ZAVED_FONT();

UPLNYSTART:

rychlost=12,hustota=20,level=1,zivoty=5,body=0;

START: gotoxy(44,1); textcolor(15);
textbackground(4); cprintf(" Body: %4u  Zivoty:  %2d  Level: %3d ",body,zivoty,level);
gotoxy(22,24); textbackground(0); cprintf("                                    ");

VYGENERUJ_PREKAZKY(); democas=CAS18();

do {
  if (demo == 1) {
    if (CAS18() < democas+rychlostdema) goto WHILE;
    democas=CAS18();
    if (kbhit() != 0) if (getch() == 0x1B) goto VYBER;
    gotoxy(pozicex,pozicey); textattr(stiny == 0 ? 0 : 8);
    pozicex--; pozicey-=3;
    if (pozicey == 0) {
      cprintf("хи");
      pozicey=2; pozicex++; goto DALSILEVEL;		//dalsi level
     }
    if (prekazky[pozicey-1][pozicex] == ' ' && prekazky[pozicey-1][pozicex+1] == ' ') {
      if (prekazky[pozicey-1][pozicex-1] != ' ' && smery[pozicey-1] == 'R') goto ZANAHORU;
      if (prekazky[pozicey-1][pozicex+2] != ' ' && smery[pozicey-1] == 'L') goto ZANAHORU;
      cprintf("хи");
      ZVYS_BODY(); pozicey--;				//nahoru
      goto KONECDEMO;
     }
ZANAHORU:
    if (prekazky[pozicey][pozicex-1] != ' ' || pozicex == 0) {
      cprintf("хи");
      if (prekazky[pozicey+1][pozicex-1] != ' ' || prekazky[pozicey+1][pozicex] != ' ' || prekazky[pozicey+1][pozicex+1] != ' ' || prekazky[pozicey+1][pozicex+2] != ' ' || smery[pozicey] != smery[pozicey-1] || rychlejsi[pozicey] != rychlejsi[pozicey-1]) {
        if ((pozicex < 15 || pozicex > 63) && VRAT_NEBEZPECI() == 0) {
          pozicey++; goto KONECDEMO;			//dolu
         }
	if (smery[pozicey] == 'R') pozicex++;		//vpravo
       }
      else pozicey++;					//dolu
     }
    else if (prekazky[pozicey][pozicex+2] != ' ' || pozicex == 78) {
      cprintf("хи");
      if (prekazky[pozicey+1][pozicex-1] != ' ' || prekazky[pozicey+1][pozicex] != ' ' || prekazky[pozicey+1][pozicex+1] != ' ' || prekazky[pozicey+1][pozicex+2] != ' ' || smery[pozicey] != smery[pozicey-1] || rychlejsi[pozicey] != rychlejsi[pozicey-1]) {
	if ((pozicex < 15 || pozicex > 63) && VRAT_NEBEZPECI() == 0) {
	  pozicey++; goto KONECDEMO;			//dolu
	 }
	if (smery[pozicey] == 'L') pozicex--;		//vlevo
       }
      else pozicey++;					//dolu
     }
    else if (smery[pozicey-1] == 'L' && (smery[pozicey] == 'R' || prekazky[pozicey][pozicex+3] == ' ') && pozicex <= 63) {
      cprintf("хи");
      pozicex++;					//vpravo
     }
    else if (smery[pozicey-1] == 'R' && (smery[pozicey] == 'L' || prekazky[pozicey][pozicex-2] == ' ') && pozicex >= 15) {
      cprintf("хи");
      pozicex--;					//vlevo
     }

    else if (prekazky[pozicey][pozicex-1] != ' ' && prekazky[pozicey][pozicex+2] == ' ' && smery[pozicey] == 'R') {
      cprintf("хи");
      pozicex++;					//vpravo
     }
    else if (prekazky[pozicey][pozicex+2] != ' ' && prekazky[pozicey][pozicex-1] == ' ' && smery[pozicey] == 'L') {
      cprintf("хи");
      pozicex--;					//vlevo
     }
    if (pozicex == 255) pozicex=0;
    if (pozicex == 79) pozicex=78;

    KONECDEMO:
    pozicex++; pozicey+=3;
    goto WHILE;
   }
  if (kbhit() != 0)
   switch(getch()) {
     case 0: gotoxy(pozicex,pozicey); textattr(stiny == 0 ? 0 : 8); cprintf("хи");
							//neviditelny stin
     switch(getch()) {
       case 72: if (--pozicey == 2) goto DALSILEVEL;		//Nahoru
                ZVYS_BODY(); break;
       case 75: if (pozicex > 1) pozicex--; break;		//Vlevo
       case 77: if (pozicex < 79) pozicex++; break;		//Vpravo
       case 80: if (pozicey < 24) pozicey++; break;		//Dolu
      } break;
     case 0x1B: goto DALSIHRA;
     case 'p': case 'P': gotoxy(37,24); textattr(15+6*16+128); cprintf(" Pauza ");
               getch(); gotoxy(37,24); textattr(0); cprintf("       "); break;
    }
WHILE:;
 } while (POPOSUN_PREKAZKY() == 0);

nosound();
if (zivoty == 0) {			//konec hry
DALSIHRA:
  if (demo == 1) { body=0; level=0; }
  nosound(); gotoxy(67,1); textattr(15+4*16); cprintf("0"); delay(500);
  TABULKA(body,level);
  gotoxy(1,25); textattr(15); cprintf(" Konec hry. Chces hrat znovu? [A/N] ");
GETCH: switch(getch()) {
    case 'a': case 'A': case 'y': case 'Y': case '\r': goto ZACATEK;
    case 'n': case 'N': case 0x1B: goto VYBER;
    default: goto GETCH;
   }
 }
goto START;

DALSILEVEL:
gotoxy(pozicex,pozicey); textattr(15+128); cprintf("хи");
if (zvuk == 1) { sound(300); delay(400); sound(500); delay(400); sound(700); delay(400); sound(1000); delay(600); }
while (kbhit() != 0) getch();

while (cas > 1) {     				//odecet casu
  textattr(0); gotoxy(cas,25); cprintf(" "); if (zvuk == 1) sound((rand()%20)+200);
  cas--; ZVYS_BODY(); delay(20);
 }
delay(200); if (zvuk == 1) sound(500);			//pricteni levele
for (cas=level; cas > 0; cas--) ZVYS_BODY();
delay(100);
if (hustota%3 == 0) if (rychlost > 2) rychlost--;
if (hustota > 2) hustota--;
level++;
while (kbhit() != 0) getch();
gotoxy(pozicex,pozicey); textattr(0); cprintf("  ");
goto START;

KONEC:
_AX = 3; geninterrupt(0x10);
textattr(12); cprintf("Silnice v1.1 - Ales Janda  KYBLSoft 2000\n\r");
textattr(10); cprintf("Vysledky hry:  %u bodu, LEVEL %d.",body,level);
textattr(7); cprintf(" \n\r  \r");
_setcursortype(_NORMALCURSOR);
}
