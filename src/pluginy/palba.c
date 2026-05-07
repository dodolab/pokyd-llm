#include <stdio.h>
#include <dos.h>
#include <conio.h>
#include <string.h>

#define POCET_STREL 80

#define BYTE unsigned char
#define DWORD unsigned long

BYTE poziceclovek,pozicepocitac;
BYTE strelaclovek[POCET_STREL][2],strelapocitac[POCET_STREL][2];
BYTE zivotyclovek,zivotypocitac;
BYTE odposlednistrely,poslpoc;
BYTE rychlost=4,zivoty=10,hustotastrel=10,obtiznost=0;
DWORD cas;

#include "palba.h"

void main(void) {
ZACATEK: ZACATEK();
_AX = 3; geninterrupt(0x10); _setcursortype(_NOCURSOR);
textattr(11+1*16); cprintf(" Ales Janda - Palba verze 1.1 - KYBLSoft 2000 ");
textattr(14+6*16); cprintf(" Zivoty:   clovek     pocitac     ");

zivotyclovek=zivoty; zivotypocitac=zivoty;
poziceclovek=3; pozicepocitac=25;
odposlednistrely=10; poslpoc=10;

{ BYTE pozice;					//vynulovani pozic strel
for (pozice=0; pozice < POCET_STREL; pozice++) {
  strelaclovek[pozice][0]=0; strelapocitac[pozice][0]=0;
 }
}

HRA:
NAKRESLI_DELA(); NAPISZIVOTY();

if (zivotyclovek == 0 || zivotypocitac == 0) {		//konec hry
  gotoxy(10,25); textattr(12+16); cprintf(" %shral jsi. Chces hrat znovu? [A/N]:  A ",zivotyclovek == 0 ? "Pro" : "Vy");
  sound(zivotyclovek == 0 ? 200 : 1000); delay(500); nosound();
  while (kbhit() != 0) getch();
  ZNOVU:
  switch(_AL = getch()) {
    case 'n': case 'N': case 0x1B: KONEC();
    case 0: case 224: case 255: goto ZNOVU;
   }
  _AH=6; _CH=2; _CL=0; _DH=24; _DL=79; _AL=0; _BL=0; geninterrupt(0x10);
  goto ZACATEK;					//smaz okno a jdi na zacatek
 }

delay(rychlost*5); nosound();

POPOSUN_STRELY();
if (kbhit() != 0)
switch(getch()) {
  case 0: case 224: switch(getch()) {
    case 72: if (poziceclovek > 3) { SMAZDELO(0); poziceclovek--; } break;
    case 80: if (poziceclovek < 25) { SMAZDELO(0); poziceclovek++; } break;
   } break;
  case 0x1B: nosound(); goto ZACATEK;
 }
while (kbhit() != 0) getch();

if (rand()%5 == 0) {					//poposun pocitac
POPOSUNPOCITAC:
  SMAZDELO(1); pozicepocitac+=(rand()%3)-1;
  if (pozicepocitac == 2) pozicepocitac=3;
  if (pozicepocitac == 26) pozicepocitac=25;
  goto STRELA;
 }
else if (obtiznost > 0 && VRAT_NEBEZPECI() == 1) goto POPOSUNPOCITAC;
else if (obtiznost == 2 && rand()%7 == 0) {	//pocitac jde za clovekem
  SMAZDELO(1);
  if (pozicepocitac > poziceclovek) { pozicepocitac--; if (VRAT_NEBEZPECI() == 1) pozicepocitac++; }
  else if (pozicepocitac < poziceclovek) { pozicepocitac++; if (VRAT_NEBEZPECI() == 1) pozicepocitac--; }
 }
STRELA:
if (poslpoc >= hustotastrel) { VYSTREL(1); poslpoc=0; }   //pocitac vystreli
poslpoc++;

_AH = 2; geninterrupt(0x16);
if (_AL%4 != 0)
 if (odposlednistrely >= hustotastrel) { VYSTREL(0); odposlednistrely=0; }
if (odposlednistrely < hustotastrel) odposlednistrely++;
goto HRA;
}
