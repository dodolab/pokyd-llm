#include <stdio.h>
#include <conio.h>
#include <dos.h>
#include <string.h>

#define BYTE unsigned char

BYTE hodina,minuta,vterina,pozicehodiny=0,stopcasu=0;
BYTE den,mesic,kdejey,hodiny[11],jmenodne;
int rok;

#include "cas.h"

void main() {
textcolor(11); textbackground(0);
cprintf("\rAles Janda               Nastaveni data a casu v1.2                KYBLSoft 2002\n\r");

_setcursortype(_NOCURSOR);
textcolor(15); cprintf("     Datum:\n\r     Cas:\n\r");
textcolor(13); cprintf("\n\r     Zmena data a casu: klavesy  ,");
textcolor(14); cprintf("-\b\b\b+");
gotoxy(30,wherey()-3); textcolor(9); cprintf("Den:");
gotoxy(1,wherey()+1);

START:
gotoxy(50,wherey()-1); textcolor(10); textbackground(0);
 cprintf("Cas:  %s",stopcasu == 0 ? "aktivni" : "pasivni");
gotoxy(50,wherey()+1); textcolor(13);
if (stopcasu == 0) {
  _AH = 0x2A; geninterrupt(0x21); den=_DL;  mesic=_DH;  rok=_CX;  jmenodne=_AL;
  _AH = 0x2C; geninterrupt(0x21); hodina=_CH;  minuta=_CL;  vterina=_DH;

  cprintf("(Z)astavit cas");
 }
else cprintf("(O)bnovit cas ");

textcolor(14); textbackground(0);
sprintf(hodiny,"%02d.%02d.%lu",den,mesic,rok); NAPISHODINY(-1);
sprintf(hodiny,"%02d:%02d:%02d",hodina,minuta,vterina); NAPISHODINY(+1);

kdejey=wherey()-1;
NAPISDEN();

UDELEJ_POZADI();
if (kbhit() == 0) goto START;			       //prekresleni hodin
switch(getch()) {
  case 0: switch(getch()) {
    case 72: if (pozicehodiny > 2) {			//nahoru
	       SKRYJ_POZADI(); pozicehodiny-=3; UDELEJ_POZADI();
	      } break;
    case 80: if (pozicehodiny < 3) {			//dolu
	       SKRYJ_POZADI(); pozicehodiny+=3; UDELEJ_POZADI();
	      } break;
    case 75: if (pozicehodiny%3 > 0) {			//vlevo
	       SKRYJ_POZADI(); pozicehodiny--; UDELEJ_POZADI();
	      } break;
    case 77: if ((pozicehodiny+1)%3 > 0) {		//vpravo
	       SKRYJ_POZADI(); pozicehodiny++; UDELEJ_POZADI();
	      } break;
    default: break;
   } break;
  case 0x1B: if (stopcasu == 1) {
    NASTAV_DATUM(); NASTAV_CAS();
   } goto KONEC;
  case '+': PRIDEJ_CAS(); break;
  case '-': UBER_CAS(); break;
  case 'z': case 'Z': if (stopcasu == 0) stopcasu=1; break;
  case 'o': case 'O': if (stopcasu == 1) {
    NASTAV_CAS(); NASTAV_DATUM(); stopcasu=0;
   } break;
  default: break;
 }
goto START;

KONEC: textcolor(7); cprintf("\n\r \r");
}
