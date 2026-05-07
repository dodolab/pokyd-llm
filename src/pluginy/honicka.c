#include <stdio.h>
#include <conio.h>
#include <dos.h>

int smerx=1,smery=1,pozicex=0,pozicey=0,srazka=0,puvpozx=1,puvpozy=1;
int nepritelx=79,nepritely=24,obtiznost=2,maxobtiznost;
int premiex=39,premiey=12;
unsigned long cas;
signed long body=0,maxbody;
//unsigned char tabulka[20][46];

//FILE *tab;

#include "honicka.vo"

void main() {
#include "honicka.in"

HRA:

_AX = 3; geninterrupt(0x33);			//zavolani interruptu 33
smerx = _CX>>3; smery = _DX>>3;			//zjisteni pozice

gotoxy(pozicex,pozicey);
_AH = 9; _BH = 0; _AL = ' '; _CX = 2; _BL = 0; geninterrupt(0x10);
							//smazani hrace

if (smerx == 0 && pozicex > 1) pozicex-=2;
else if (smerx == 2 && pozicex < 79) pozicex+=2;
if (smery == 0 && pozicey > 1) pozicey--;
else if (smery == 2 && pozicey < 24) pozicey++;

gotoxy(pozicex,pozicey);
_AH = 9; _BH = 0; _AL = ' '; _CX = 2; _BL = 32; geninterrupt(0x10);
							//nakresleni hrace

while (CAS18() == 0) {}

if (srazka > 0) {
  srazka--; if (srazka == 0) {
    _AX = 0x1001; _BH = 1; geninterrupt(0x10);	//navrat okraje na modrou
   }
 }


if (rand()%obtiznost != 0 && srazka == 0) {		//nahodne nicnedelani

  if (nepritelx == premiex && nepritely == premiey) {
    gotoxy(premiex,premiey);
    _AH = 9; _BH = 0; _AL = '<'; _CX = 1; _BL = 14*16; geninterrupt(0x10);
    gotoxy(premiex+1,premiey);
    _AH = 9; _BH = 0; _AL = '>'; _CX = 1; _BL = 14*16; geninterrupt(0x10);
						//nakresleni premie
    }
  else {
    gotoxy(nepritelx,nepritely);
    _AH = 9; _BH = 0; _AL = ' '; _CX = 2; _BL = 0; geninterrupt(0x10);
							//smazani nepritele
   }
  if (rand()%(obtiznost>>1) == 0) {}
   else { puvpozx=pozicex; puvpozy=pozicey; }
						//nahodne zlepseni inteligence
  if (nepritelx < puvpozx) nepritelx+=2;
  else if (nepritelx > puvpozx) nepritelx-=2;
  if (nepritely < puvpozy) nepritely++;
  else if (nepritely > puvpozy) nepritely--;	//inteligence protihrace
 }

gotoxy(nepritelx,nepritely);
_AH = 9; _BH = 0; _AL = ' '; _CX = 2; _BL = 64; geninterrupt(0x10);
						//nakresleni protihrace

if (nepritelx == pozicex && nepritely == pozicey && srazka == 0) {//chytnul te!
  _AX=0x1001; _BH=36; geninterrupt(0x10);	//zcervenani okraje
  sound(rand()%100+50); delay(500); nosound(); srazka=10; body-=100;
 }
else body++;

if (premiex == pozicex && premiey == pozicey) {		//chytl jsi premii

  _AX = 0x1001; _BH = 54; geninterrupt(0x10);	//zmena okraje na zlutou

  body+=150; sound(1000); delay(200); nosound();
  premiex=rand()%40; premiex<<=1; premiex++; premiey=(rand()%24)+1;
  gotoxy(premiex,premiey);
  _AH = 9; _BH = 0; _AL = '<'; _CX = 1; _BL = 14*16; geninterrupt(0x10);
  gotoxy(premiex+1,premiey);
  _AH = 9; _BH = 0; _AL = '>'; _CX = 1; _BL = 14*16; geninterrupt(0x10);
						//nakresleni premie

  _AX = 0x1001; _BH = 1; geninterrupt(0x10);	//navrat okraje na modrou
 }

if (body > maxbody) maxbody=body;
if (obtiznost > maxobtiznost) maxobtiznost=obtiznost;	//maximalni vykony

if (body > 0) obtiznost=body/100+2;
else obtiznost=2;

gotoxy(53,25); printf("%4ld/%4ld ",body,maxbody);
gotoxy(75,25); printf("%2ld",obtiznost-2);
printf("/%2ld",maxobtiznost-2);

puvpozx=pozicex; puvpozy=pozicey;

if (kbhit() == 0) goto HRA;
else switch(getch()) {
  case 0x1B: goto KONEC;
  case 'M': case 'm': body=maxbody; goto HRA;			//cheat
  case 'P': case 'p':						//pauza
    gotoxy(31,23); textcolor(14); textbackground(5);
    cprintf(" Stiskni cokoliv "); getch();
    gotoxy(31,23); textbackground(0); cprintf("                 ");
  default: goto HRA;
 }

KONEC:

/*clrscr();
_AX = 0x1010; _BX = 0; _CH = 0xFF; _CL = 0; _DH = 0xFF;
geninterrupt(0x10);		//zmena barvy 0 na zlutou

for (pozicex=0; pozicex < 20; pozicex++) {
  cprintf("%d. %s",pozicex+1,tabulka[pozicex]);
  body=tabulka[pozicex][41]+(tabulka[pozicex][42]<<8)+(tabulka[pozicex][43]<<16)+(tabulka[pozicex][44]<<24);
  cprintf(" - %lu bodu\n\r",body);
 }
getch();
*/

_AX = 3; geninterrupt(0x10);
textcolor(14); textbackground(0); cprintf("Dekujeme za zahrani Honicky v1.1 !");

_AX=0x1001; _BH=0; geninterrupt(0x10);		//navraceni okraje
_AX=0x1003; _BH=1; geninterrupt(0x10);		//povoleni blikani, 8 pozadi
textcolor(7); cprintf(" \n\rNaprogramoval Ales Janda * KYBLSoft 2000\n\r ");
}
