#define CEKAT 1

#include <stdio.h>
#include <conio.h>
#include <dos.h>

#define BYTE unsigned char
#define WORD unsigned short
#define DWORD unsigned long

WORD had1[4000][2],had2[4000][2];
BYTE premiex,premiey,smer1,smer2,klavesa,past,srazkajedenbod,jedenbodsmer1,jedenbodsmer2;
WORD prodlouzeni1,prodlouzeni2,typpremie;
BYTE blikpremie,narazeni1,narazeni2,narazenyhad,zvuk,rastr,cary,pocitac;
BYTE pocetx,pocety,rychlost,pocethracu,pomocne[160],tolernabour,maxnabour;
BYTE zivoty1,zivoty2,celkemzivotu,modulo,prodlouzeni,celkdelkahada,intro;
BYTE aktualnihad,puvsmer1,puvsmer2;
signed long body1,body2;
WORD delkahada1,delkahada2;

FILE *f;

#include "had.h"

void main(void) {
BYTE pozicemenu,puvsmer1,puvsmer2;

SOUBOR("HAD.CFG");
if ((f=fopen(pomocne,"rb")) != NULL) {
  fseek(f,7,SEEK_SET);
  if (getc(f) == 40) { pocetx=40; pocety=25; } else { pocetx=80; pocety=50; }
  if (getc(f) == 2) pocethracu=2; else pocethracu=1;
  pocitac=getc(f); if (pocitac > 3) pocitac=0;
  rychlost=getc(f); if (rychlost == 0 || rychlost > 5) rychlost=3;
  celkdelkahada=getc(f); if (celkdelkahada > 6) celkdelkahada=0;
  prodlouzeni=getc(f); if (prodlouzeni > 5) prodlouzeni=3;
  celkemzivotu=getc(f); if (celkemzivotu == 0 || celkemzivotu > 9) celkemzivotu=5;
  if (getc(f) == 0) zvuk=0; else zvuk=1;
  tolernabour=getc(f); if (tolernabour > 4) tolernabour=1;
  if (getc(f) == 1) modulo=1; else modulo=0;
  rastr=getc(f); if (rastr > 3) rastr=0;
  if (getc(f) == 1) cary=1; else cary=0;
  fclose(f);
 }
else {
  pocethracu=1; rychlost=3; pocetx=80; pocety=50; celkemzivotu=5;
  zvuk=1; tolernabour=1; modulo=0; prodlouzeni=3; rastr=0;
 }
srand(CAS18());

ZACATEK:
textmode(C80); _AX = 3; geninterrupt(0x10); _setcursortype(_NOCURSOR);
gotoxy(16,24); NAPISRETEZEC("Had v1.0 - autor Ales Janda - KYBLSoft (C) 2001",14);
gotoxy(27,25); NAPISRETEZEC("- plugin k programu Pokyd -",6);
intro=1;
gotoxy(37,3); NAPISRETEZEC("M E N U",11);
gotoxy(36,4); NAPISRETEZEC("--=====--",9);
pozicemenu=6;

MENU:
gotoxy(36,6); NAPISRETEZEC("Start hry",15);

gotoxy(26,7); NAPISRETEZEC("Velikost hraci plochy:",15);
gotoxy(50,7); NAPISRETEZEC(pocetx == 40 ? "40x24" : "80x49",10);

gotoxy(33,8); NAPISRETEZEC("1. hrac:",15);
gotoxy(43,8);
if ((pocitac&1) == 1) NAPISRETEZEC("pocitac",10);
else NAPISRETEZEC("clovek ",10);

gotoxy(33,9); NAPISRETEZEC("2. hrac:",15);
gotoxy(43,9);
if (pocethracu == 1) NAPISRETEZEC("zadny  ",10);
else if ((pocitac&2) == 2) NAPISRETEZEC("pocitac",10);
else NAPISRETEZEC("clovek ",10);

gotoxy(32,10); NAPISRETEZEC("Rychlost:",15);
gotoxy(43,10); switch(rychlost) {
  case 1: NAPISRETEZEC("nejmensi",10); break;
  case 2: NAPISRETEZEC("  mala  ",10); break;
  case 3: NAPISRETEZEC("stredni ",10); break;
  case 4: NAPISRETEZEC(" velka  ",10); break;
  case 5: NAPISRETEZEC("nejvetsi",10); break;
 }

gotoxy(34,11); NAPISRETEZEC("Delka hada:",15);
gotoxy(47,11);
if (celkdelkahada == 0) strcpy(pomocne,"2 ");
else sprintf(pomocne,"%d",celkdelkahada*5); NAPISRETEZEC(pomocne,10);

gotoxy(31,12); NAPISRETEZEC("Prodlouzeni:",15);
gotoxy(45,12); switch(prodlouzeni) {
  case 0: NAPISRETEZEC(" zadne  ",10); break;
  case 1: NAPISRETEZEC("nejmensi",10); break;
  case 2: NAPISRETEZEC("  male  ",10); break;
  case 3: NAPISRETEZEC("stredni ",10); break;
  case 4: NAPISRETEZEC(" velke  ",10); break;
  case 5: NAPISRETEZEC("nejvetsi",10); break;
 }

gotoxy(33,13); NAPISRETEZEC("Pocet zivotu:",15);
gotoxy(48,13); sprintf(pomocne,"%d ",celkemzivotu); NAPISRETEZEC(pomocne,10);

gotoxy(36,14); NAPISRETEZEC("Zvuk:",15);
gotoxy(43,14); NAPISRETEZEC(zvuk == 1 ? "ano" : "ne ",10);

gotoxy(27,15); NAPISRETEZEC("Tolerance nabourani:",15);
gotoxy(49,15); switch(tolernabour) {
  case 0: NAPISRETEZEC(" zadna ",10); break;
  case 1: NAPISRETEZEC(" mala  ",10); break;
  case 2: NAPISRETEZEC("stredni",10); break;
  case 3: NAPISRETEZEC(" velka ",10); break;
  case 4: NAPISRETEZEC(" uplna ",10); break;
 }

gotoxy(29,16); NAPISRETEZEC("Konec obrazovky:",15);
gotoxy(47,16); NAPISRETEZEC(modulo == 0 ? " zed  " : "modulo",10);

gotoxy(34,17); NAPISRETEZEC("Rastr:",15);
gotoxy(42,17); switch(rastr) {
  case 0: NAPISRETEZEC("nikde ",10); break;
  case 1: NAPISRETEZEC("pozadi",10); break;
  case 2: NAPISRETEZEC(" had  ",10); break;
  case 3: NAPISRETEZEC("vsude ",10); break;
 }

gotoxy(31,18); NAPISRETEZEC("Cary za hadama:",15);
gotoxy(48,18); NAPISRETEZEC(cary == 1 ? "ano" : "ne ",10);

gotoxy(36,19); NAPISRETEZEC("Konec hry",15);

intro=0;
KURZOR(pozicemenu);

GETCH: switch(getch()) {
  case 0: switch(getch()) {
    case 72: if (pozicemenu > 6) { KURZOR(pozicemenu); pozicemenu--; goto MENU; }
	     else goto GETCH;
    case 80: if (pozicemenu < 19) { KURZOR(pozicemenu); pozicemenu++; goto MENU; }
	     else goto GETCH;
    default: goto PIPNI;
   }
  case 0x1B: if (pozicemenu == 19) goto KONEC;
	     else { KURZOR(pozicemenu); pozicemenu=19; goto MENU; }
  case '\r': case ' ': switch(pozicemenu) {
    case 6: goto HRA;
    case 7: if (pocetx == 40) { pocetx=80; pocety=50; }
	    else { pocetx=40; pocety=25; } KURZOR(pozicemenu); goto MENU;
    case 8: pocitac^=1; KURZOR(pozicemenu); goto MENU;
    case 9: if (pocethracu == 1) { pocethracu=2; pocitac|=2; }
	    else if ((pocitac&2) == 2) pocitac-=2;
            else pocethracu=1;
            KURZOR(pozicemenu); goto MENU;
    case 10: rychlost++; if (rychlost == 6) rychlost=1; KURZOR(pozicemenu); goto MENU;
    case 11: celkdelkahada++; if (celkdelkahada == 7) celkdelkahada=0; KURZOR(pozicemenu); goto MENU;
    case 12: prodlouzeni++; if (prodlouzeni == 6) prodlouzeni=0; KURZOR(pozicemenu); goto MENU;
    case 13: celkemzivotu++; if (celkemzivotu == 10) celkemzivotu=1; KURZOR(pozicemenu); goto MENU;
    case 14: zvuk^=1; KURZOR(pozicemenu); goto MENU;
    case 15: tolernabour++; if (tolernabour == 5) tolernabour=0; KURZOR(pozicemenu); goto MENU;
    case 16: modulo^=1; KURZOR(pozicemenu); goto MENU;
    case 17: rastr++; if (rastr == 4) rastr=0; KURZOR(pozicemenu); goto MENU;
    case 18: cary^=1; KURZOR(pozicemenu); goto MENU;
    case 19: goto KONEC;
   }
PIPNI: default: if (zvuk == 1) sound(100); delay(100); nosound(); goto GETCH;
 }

HRA:
zivoty1=celkemzivotu; zivoty2=celkemzivotu;
body1=0; body2=0;
switch(tolernabour) {
  case 0: maxnabour=1; break;
  case 1: maxnabour=2; break;
  case 2: maxnabour=3; break;
  case 3: maxnabour=5; break;
  case 4: maxnabour=255; break;
 }

_AX = 3; geninterrupt(0x10);

if (pocetx == 40) textmode(C40); else textmode(C4350);
_setcursortype(_NOCURSOR);
if (modulo == 0) {
  _AX = 0x1001; _BH = 1; geninterrupt(0x10);	//okraj
 }
_AX = 0x1003; _BL = 0; geninterrupt(0x10);	//zakaz blikani

HRA2:
for (typpremie=1; typpremie < pocety; typpremie++) {
  gotoxy(1,typpremie);
  _CX = (WORD)pocetx; if (rastr&1 == 1) _AL = 7; else _AL = ' ';
  _AH = 9; _BX = 8; geninterrupt(0x10);
 }					//smaz obrazovku nebo nahraz rastrem

if (celkdelkahada == 0) prodlouzeni1=1;
else prodlouzeni1=celkdelkahada*5-1;
prodlouzeni2=prodlouzeni1;

typpremie=1; blikpremie=20;

premiex=0; premiey=0;

narazeni1=0; smer1=2; delkahada1=1;
had1[0][0]=3; had1[0][1]=8;
if (pocethracu == 2) {
  narazeni2=0; smer2=2; delkahada2=1; blikpremie=10;
  had2[0][0]=3; had2[0][1]=15;
 }
else delkahada2=0;
while (prodlouzeni1 > 0) POPOSUN_HADY();
DEJ_PREMII();
NAPISBODY();

CEKEJ("Stiskni cokoliv pro zacatek hry",0);
goto HRA1;

START:
if (POPOSUN_HADY() == 1) {

  if (narazenyhad == 1 || narazenyhad == 3) { zivoty1--; body1-=50; }
  if (narazenyhad == 2 || narazenyhad == 3) { zivoty2--; body2-=50; }

  NAPISBODY();
#if CEKAT == 1
  if (zvuk == 1) sound(300); delay(500); nosound();
#endif
  if (pocethracu == 1) {
    if (maxnabour != 255) CEKEJ("Had naboural.",1);
    else CEKEJ("Had se zablokoval.",1);
   }
  else {
    if (maxnabour != 255) {
      switch(narazenyhad) {
        case 1: CEKEJ("Hrac 1 narazil.",1); break;
        case 2: CEKEJ("Hrac 2 narazil.",1); break;
        case 3: CEKEJ("Oba hraci narazili.",1); break;
       }
     }
    else {
      switch(narazenyhad) {
        case 1: CEKEJ("Hrac 1 se zablokoval.",1); break;
        case 2: CEKEJ("Hrac 2 se zablokoval.",1); break;
        case 3: CEKEJ("Oba hraci se zablokovali.",1); break;
       }
     }
   }
  goto DALSIZIVOT2;

DALSIZIVOT1:
  if (narazenyhad&1 == 1) zivoty1--;
  if (narazenyhad&2 == 2) zivoty2--;
  NAPISBODY();

DALSIZIVOT2:
  if (zivoty1 == 0 || zivoty2 == 0) {
    if (pocethracu == 1) CEKEJ("Konec hry.",1);
    else {
      if (body1 > body2) CEKEJ("Konec hry. Vyhral hrac 1.",1);
      if (body1 < body2) CEKEJ("Konec hry. Vyhral hrac 2.",1);
      if (body1 == body2) CEKEJ("Konec hry. Hraci skoncili stejne.",1);
     }
#if CEKAT == 0
getch();
#endif
    goto ZACATEK;
   }
  goto HRA2;
 }
if (blikpremie > 0) {
  gotoxy(premiex,premiey);
  if (blikpremie%4 == 0) RASTR_NAPISZNAK(15*16,rastr&2);
  else if (blikpremie%4 == 2) RASTR_NAPISZNAK(12*16,rastr&2);
  blikpremie--;
 }
NAPISBODY();
#if CEKAT == 1
switch(rychlost) {
  case 1: delay(640); break;
  case 2: delay(320); break;
  case 3: delay(160); break;
  case 4: delay(80); break;
  case 5: delay(40); break;
  case 6: delay(20); break;
  default: geninterrupt(3);
	   CEKEJ("Chyba!",1);
	   CEKEJ("Chyba!",1);
	   CEKEJ("Chyba!",1);
 }
#endif

HRA1:

if (narazeni1 == 0) puvsmer1=smer1;
if (narazeni2 == 0) puvsmer2=smer2;

if ((pocitac&1) == 1) { aktualnihad=1; INTELIGENCE(); }
if ((pocitac&2) == 2) { aktualnihad=2; INTELIGENCE(); }

KBHIT: if (kbhit() != 0) {
  klavesa=getch();
  if (pocethracu == 2 && (pocitac&2) == 0) {
    switch(klavesa) {
      case 'a': case 'A': if (puvsmer2 != 2) smer2=0; break;	//vlevo
      case 'w': case 'W': if (puvsmer2 != 3) smer2=1; break;	//nahoru
      case 'd': case 'D': if (puvsmer2 != 0) smer2=2; break;	//vpravo
      case 'x': case 'X':
      case 's': case 'S': if (puvsmer2 != 1) smer2=3; break;	//dolu
      default: goto KBHIT2;
     }
   }
KBHIT2:
  if ((pocitac&1) == 0) {
    switch(klavesa) {
      case 0: switch(getch()) {
        case 75: if (puvsmer1 != 2) smer1=0; break;		//vlevo
        case 72: if (puvsmer1 != 3) smer1=1; break;		//nahoru
        case 77: if (puvsmer1 != 0) smer1=2; break;		//vpravo
        case 80: if (puvsmer1 != 1) smer1=3; break;		//dolu
       } break;
      default: goto KBHIT3;
     }
   }
KBHIT3:
  switch(klavesa) {
    case 'p': case 'P': CEKEJ("Pauza - stiskni cokoliv",0); goto KBHIT;
SKONCENIDOTAZ: case 0x1B: switch (CEKEJ("[R]estart, [K]onec hry, [S]torno",1)&0x5F) {
                 case 'R': narazenyhad=pocethracu|1; goto DALSIZIVOT1;
                 case 'K': goto ZACATEK;
                 default: goto KBHIT;
		}
    case 0: if ((pocitac&1) == 1) getch();	//spolknuti bufferu
   }
  goto KBHIT;
 }
goto START;

KONEC:
SOUBOR("HAD.CFG");
if ((f=fopen(pomocne,"wb")) != NULL) {
  fwrite("HadCfg-",7,1,f);
  putc(pocetx,f);
  putc(pocethracu,f);
  putc(pocitac,f);
  putc(rychlost,f);
  putc(celkdelkahada,f);
  putc(prodlouzeni,f);
  putc(celkemzivotu,f);
  putc(zvuk,f);
  putc(tolernabour,f);
  putc(modulo,f);
  putc(rastr,f);
  putc(cary,f);
  fclose(f);
 }
_AX = 3; geninterrupt(0x10); _setcursortype(_NORMALCURSOR);
}
