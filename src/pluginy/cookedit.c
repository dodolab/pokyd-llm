#include <stdio.h>
#include <conio.h>
#include <dos.h>

#define BYTE unsigned char
#define WORD unsigned short
#define DWORD unsigned long

BYTE slovapamet[50][3][15],slovajsem[50][15],slovajsi[50][15];
BYTE jmenocloveka[15],jmenocloveka5pad[16];
BYTE pozicekurzorx,pozicekurzory,pozicesloupce,zmena=0,poprve=1,pocetradku;
BYTE soubor[80],pocetslpamet,pocetjsem,pocetjsi,typedit;
BYTE hlverze,vedlverze;

FILE *vety;

#include "cookedit.h"

void main(void) {
_AX = 3; geninterrupt(0x10); _setcursortype(_NOCURSOR);
textattr(15+3*16); cprintf(" Ales Janda                   Cookies Editor v1.2                 KYBLSoft 2001 ");
pozicekurzory=0; typedit=1;
START:
_AX = 0x0600; _CX = 0x0100; _DX = 0x174F; _BH = 16; geninterrupt(0x10);
_AX = 0x0600; _CX = 0x1800; _DX = 0x184F; _BH = 3*16; geninterrupt(0x10);
_AX = 0x0600; _CX = 0x0200; _DX = 0x0B4F; _BH = 7*16; geninterrupt(0x10);

gotoxy(1,4); textattr(7*16); cprintf(""
" Program Cookies Editor umoznuje editovat jednotliva fakta, ktera si program\r\n"
" Pokyd zapamatoval. Je rozdelen do 4 casti:\r\n"
"  1.  editace jednoduchych vet typu \"[podmet] [sloveso] [predmet]\"\r\n"
"  2.  editace vsech slov typu \"jsem [slovo]\"\r\n"
"  3.  editace vsech slov typu \"jsi [slovo]\"\r\n"
"  4.  editace jmena cloveka\r\n\n"
" Nyni si vyber jednu z techto casti k prohlizeni a editaci:");

textattr(14+6*16);
gotoxy(24,14); cprintf("ษอออออออออออออออออออออออออออออออออป");
gotoxy(24,15); cprintf("บ            Kategorie            บ");
gotoxy(24,16); cprintf("บ                                 บ");
gotoxy(24,17); cprintf("บ 1. [podmet] [sloveso] [predmet] บ");
gotoxy(24,18); cprintf("บ 2. jsem [slovo]                 บ");
gotoxy(24,19); cprintf("บ 3. jsi [slovo]                  บ");
gotoxy(24,20); cprintf("บ 4. jmeno cloveka                บ");
gotoxy(24,21); cprintf("ศอออออออออออออออออออออออออออออออออผ");

if (poprve == 0) goto MENU;
RADEK("Nacitam data...");
switch(CTI_INFORMACE_O_VETACH()) {
  case 1: RADEK("Novy soubor nebo konec programu?");
    OTAZKA: switch(HLASKA("Nemuzu otevrit soubor s fakty! Mam zalozit novy? [A/N] A",2)) {
      case 'a': case 'A': case 'y': case 'Y': case '\r': ZALOZ_NOVY(); break;
      case 'n': case 'N': case 0x1B: KONEC(0);
      default: goto OTAZKA;
     }
    break;
  case 2: RADEK("Bohuzel, spatna verze souboru. I kdybych ho editoval, Pokyd by ho ignoroval.");
    HLASKA("Je mi lito, ale soubor PROFIL.PKD pochazi z jine verze nez je tento plugin!",4);
    KONEC(0);
 }
poprve=0;


MENU:
pozicekurzory=typedit-1; typedit=0; KURZOR_RADEK(1);
ZAPIS_STAT();

DOTAZ: RADEK("Vyber si, co chces editovat.");
switch(getch()) {
  case 0: switch(getch()) {
    case 72: if (pozicekurzory > 0) {				//nahoru
               KURZOR_RADEK(0); pozicekurzory--; KURZOR_RADEK(1);
              } break;
    case 80: if (pozicekurzory < 3) {
               KURZOR_RADEK(0); pozicekurzory++; KURZOR_RADEK(1);
              } break;
    default: break;
   } goto DOTAZ;
  case '\r': typedit=pozicekurzory+1; break;
  case '1': typedit=1; break;
  case '2': typedit=2; break;
  case '3': typedit=3; break;
  case '4': typedit=4; break;
  case 0x1B: KONEC(1); goto DOTAZ;
  default: goto DOTAZ;
 }

if (typedit == 4) {
  JMENO_CLOVEKA(); goto START;
 }

VYTVOR_SLOUPCE();
NAPISSLOVA(0);
POHYB_SIPKAMA();

goto START;
}