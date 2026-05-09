#include <stdio.h>
#include <conio.h>
#include <dos.h>

#define BYTE unsigned char

unsigned long delka,celkemdelka,pomoc,pocetsouboru;
BYTE soub[80],soubor[80],pocetchyb=0,kontrola1,kontrola2,znak;
BYTE verze=0,verze2,verze3,pozice,chybaint24;

void interrupt (*stara_adresa24)(void);

FILE *test,*hodnoty;

#include "testpkd_body.c"

void main(void) {
textcolor(11); textbackground(0);
cprintf("\rAles Janda            Pokyd - testovaci program v1.6               KYBLSoft 2005");

_setcursortype(_NOCURSOR);
stara_adresa24=getvect(0x24);
setvect(0x24,INT24);					//Kriticka chyba DOSu

textcolor(14); cprintf("\n\rKontroluji soubory....\n\r");
textcolor(10); cprintf("TESTPKD.TST");
SOUBOR("TESTPKD.TST");
if ((hodnoty=fopen(soub,"rb")) == NULL) {
  textcolor(12); cprintf("\rNeexistuje soubor s kontrolnimi soucty TESTPKD.TST !\n\r"); pocetchyb=1; goto CHYBY;
 }
fseek(hodnoty,0,SEEK_END); celkemdelka=ftell(hodnoty)-2; fseek(hodnoty,0,SEEK_SET);
kontrola1=0; kontrola2=0;
for (delka=0; delka < celkemdelka; delka++) {
  znak=getc(hodnoty); kontrola1^=znak; kontrola1^='K'; kontrola1+=znak;
  kontrola2+=znak;
 }
if (kontrola1 != getc(hodnoty) || kontrola2 != getc(hodnoty)) {
  textcolor(12); cprintf("\rChyba v souboru TESTPKD.TST !\n\r"); pocetchyb=1; goto CHYBY;
 }
fseek(hodnoty,0,SEEK_SET);
verze=getc(hodnoty)^'K'; verze2=getc(hodnoty)^'K'; verze3=getc(hodnoty)^'K';
if (verze != '7' || verze2 != '0' || verze3 != '0') {		//verze 7.00
  textcolor(12); cprintf("\rNespravna verze souboru TESTPKD.TST (verze %c.%c%c)! Presto zkontrolovat? [A/N] A\b",verze,verze2,verze3);
  _setcursortype(_NORMALCURSOR);
  pocetchyb=1; verze=1;
  switch(getch()) {
    case 'a': case 'A': case 'y': case 'Y': case '\r': cprintf("\r\n"); goto TEST;
    default: cprintf("\r\n"); goto CHYBY;
   }
 }
else verze=0;
TEST:
_setcursortype(_NOCURSOR);
fseek(hodnoty,3,SEEK_SET);
pocetsouboru=(getc(hodnoty)^'K')+1;
while (pocetsouboru > 0) {
  for (pozice=0; (znak=getc(hodnoty)^'K') > 0; pozice++)
    soubor[pozice]=znak;
  soubor[pozice]=0;
  TESTUJ(soubor);
  pocetsouboru--;
 }
cprintf("\r                   \r");
if (pocetchyb == 0) {
  textcolor(15); cprintf("Vse OK, stisknete cokoliv pro ukonceni.");
  getch();
 }
else {
  if (verze == 1 && pocetchyb == 1) {			//jina verze
    textcolor(15); cprintf("Soubory jsou jinak OK");
   }
CHYBY:
  _setcursortype(_NOCURSOR);
  textcolor(12); cprintf("\n*** Nalezeny chyby! Pocet: %d\n\r",pocetchyb);
  textcolor(13);
cprintf("Tyto chyby opravite nahranim souboru ze zdroje (napr. z internetove adresy\n\r"
        "http://iqpokyd.kyblsoft.cz). Prosim nerozsirujte tuto vadnou verzi,\n\r"
        "program je stejne freeware a zbytecne se nici.\n\r");
  textcolor(14); gotoxy(24,wherey());
  cprintf("Stisknete cokoliv pro ukonceni.");
  getch();
 }

KONEC:
fclose(hodnoty);
setvect(0x24,stara_adresa24);
textcolor(7); textbackground(0); cprintf("\n\r \n\r \r"); _setcursortype(_NORMALCURSOR);
}
