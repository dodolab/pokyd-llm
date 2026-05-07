#include <stdio.h>
#include <conio.h>
#include <dos.h>
#include <string.h>

#define BYTE unsigned char
#define WORD unsigned short
#define DWORD unsigned long

BYTE jmena[4000],retezec[31],jmeno[31],jmeno2[31],soubor[80],znak,mesic,pocetradku;
WORD pozicejmeno,pozice,pozice2,poziceden,den,rok;
DWORD pocetjmen,velikostsouboru;

FILE *kalendar;

#include "najdime.v"

void main(int argc, char *argv[]) {
textcolor(11); textbackground(0);
cprintf("\rAles Janda             Vyhledavani jmen v kalendari v1.5         KYBLSoft 8/2002\n\r");

znak=wherey();
for (pocetradku=wherey(); pocetradku < 250; pocetradku++) {
  gotoxy(1,pocetradku); if (wherey() != pocetradku) break;
 }
if (pocetradku >= 250 || pocetradku < 4)
 pocetradku=25;		                //nejaka blbost, tak standardne

pocetradku-=3;				//aby to bylo pouzitelne
gotoxy(1,znak);                         //navrat na puvodni pozici

textcolor(12); cprintf("Nacitam jmena...");
SOUBOR("NAJDIME.KAL");
if ((kalendar=fopen(soubor,"rb")) == NULL) {
  cprintf("\rNemuzu otevrit soubor NAJDIME.KAL!\n");
  goto KONEC;
 }
cprintf("\r                \r");
fseek(kalendar,0,SEEK_END); velikostsouboru=ftell(kalendar);
fseek(kalendar,366*2,SEEK_SET); fread(jmena,velikostsouboru,1,kalendar);

if (argc < 2) {

  _AH = 0x2A; geninterrupt(0x21);
  znak = _AL; den = _DL; mesic = _DH; rok = _CX;
  NAJDIJMENO();
  POSUNJMENA(-2); znak+=5;
  for (pozice=0; pozice < 7; pozice++) {
    textcolor(13); switch(znak%7) {
      case 1: cprintf("pondeli"); break;
      case 2: cprintf("utery  "); break;
      case 3: cprintf("streda "); break;
      case 4: cprintf("ctvrtek"); break;
      case 5: cprintf("patek  "); break;
      case 6: cprintf("sobota "); break;
      case 0: cprintf("nedele "); break;
     }
    znak++;
    textattr(10); cprintf("  %2u.%2d.  -  ",den,mesic); NAPISJMENO();
    if (pozice == 2) { textattr(15); cprintf("    - dnes -"); }
    cprintf("\r\n"); POSUNJMENA(1);
   }

  textcolor(15); cprintf("\n\n\r"
  "Specialni znaky pro vyhledavani (pred jmenem):\n\r"
  " * (ASCII 42) - muzske jmeno\n\r"
  " % (ASCII 37) - zenske jmeno\n\r"
  " # (ASCII 35) - statni svatek\n\r"
  " ~ (klavesa nad Tabem) - vyznamny den\n\n\r");

NAPIS:
  pozice=0;
  textattr(14); cprintf("\rZadej kus hledaneho jmena nebo datum (dd.mm.):                                ");
  NAPISPOCET:
  _setcursortype(_NOCURSOR);
  retezec[pozice]=0; gotoxy(59,wherey()-1);
  if (JELI_CISLICE(retezec[0]) == 0) {			//jedna se o jmeno
    pozice2=POCET_NALEZENYCH_JMEN();
    if (pozice2 > 0) {
      textattr(11); cprintf(" Nalezenych jmen: %3u",pozice2);
     }
    else {
      textattr(12); cprintf("  Zadne takove jmeno!");
      if (kbhit() == 0) {
        delay(80); gotoxy(60,wherey()); textattr(15); cprintf(" Zadne takove jmeno!");
        delay(80); gotoxy(60,wherey()); textattr(12); cprintf(" Zadne takove jmeno!");
       }
     }
   }
  else {					//jedna se o datum
    textattr(9);
    cprintf("   - zadavani data - ",POCET_NALEZENYCH_JMEN());
    pozice2=500;				//aby to nebyla nula
   }

  textattr(14); gotoxy(48+pozice,wherey()+1); _setcursortype(_NORMALCURSOR);
  GETCH:
  switch(znak=getch()) {
    case 0: getch(); goto GETCH;
    case '\r': _setcursortype(_NOCURSOR);
               retezec[pozice]=0; gotoxy(59,wherey()-1);
               cprintf("                     \n");
               break;
    case '\b': if (pozice > 0) { retezec[--pozice]=0; cprintf("\b \b"); }
               goto NAPISPOCET;
    case 0x1B: goto KONEC;
    case '.': if (pozice == 0) goto GETCH;
              if (pozice2 == 0) goto NAPISPOCET;	//zadne takove jmeno
              if (strchr(retezec,'.') != strrchr(retezec,'.')) goto GETCH; //2 tecky
	      retezec[pozice++]='.'; cprintf(".",znak);
              goto NAPISPOCET;
    default: if (pozice >= 30) goto GETCH;
             if (pozice2 == 0) goto NAPISPOCET;		//zadne takove jmeno
             if (znak > 96 && znak < 123) znak-=32;
	     if ((znak < 'A' || znak > 'Z') && (znak < '0' || znak > '9') && znak != '.' && znak != '#' && znak != '~' && znak != '' && znak != '' && znak != '*' && znak != '%' && znak != ',' && znak != ' ') goto GETCH;
	     retezec[pozice++]=znak; cprintf("%c",znak);
             goto NAPISPOCET;
   }
  cprintf("\n\r");
 }
else {
  argv[1][29]=0;
  strcpy(retezec,argv[1]);
  for (pozice=0; pozice < 30; pozice++)
   if (retezec[pozice] > 96 && retezec[pozice] < 123) retezec[pozice]-=32;
 }

_setcursortype(_NOCURSOR);
if (strchr(retezec,'.') != NULL) goto DATUM;

pozicejmeno=0; pocetjmen=0; poziceden=0;
PREVED_RETEZEC_NA_JMENO();
for (pozice=0; pozice < 366; pozice++) {
  poziceden++;
  strcpy(jmeno2,jmena+pozicejmeno);
  strupr(jmeno2);
  if (strstr(jmeno2,jmeno) != NULL) {			//nalezeno jmeno
    den=poziceden; mesic=1;
    if (den > 31) { mesic++; den-=31;
     if (den > 29) { mesic++; den-=29;
      if (den > 31) { mesic++; den-=31;
       if (den > 30) { mesic++; den-=30;
	if (den > 31) { mesic++; den-=31;
	 if (den > 30) { mesic++; den-=30;
	  if (den > 31) { mesic++; den-=31;
	   if (den > 31) { mesic++; den-=31;
	    if (den > 30) { mesic++; den-=30;
	     if (den > 31) { mesic++; den-=31;
	      if (den > 30) { mesic++; den-=30;
    }}}}}}}}}}}
    textcolor(10); cprintf("\r%2u.%2d.  -  ",den,mesic);
    NAPISJMENO(); cprintf("\r\n"); pocetjmen++;
    if ((pocetjmen%pocetradku) == 0) {
      textcolor(15); cprintf("\rStiskni cokoliv pro pokracovani nebo Esc pro ukonceni.");
      znak=getch(); cprintf("\r                                                      ");
      if (znak == 0x1B) goto NAPIS;
     }
   }
  pozicejmeno+=strlen(jmena+pozicejmeno)+1;
 }
textcolor(9);
if (pocetjmen == 0) cprintf("\rNenalezeno zadne jmeno s danymi znaky.");
else cprintf("\rPocet nalezenych jmen:  %lu",pocetjmen);
goto OTAZKAZNOVU;

DATUM:
den=0; mesic=0; pozice2=1;				//zacina se dnem
PREVED_RETEZEC_NA_JMENO();
for (pozice=0; jmeno[pozice] > 0; pozice++) {
  if (jmeno[pozice] == '.') pozice2=2;
  else if (jmeno[pozice] < '0' || jmeno[pozice] > '9') {
    textcolor(12); cprintf("\rChyba zadani data (konkretne '%c' jako %d. znak) !",retezec/* neprevedeny retezec */[pozice],pozice+1);
    goto OTAZKAZNOVU;
   }
  else {
    if (pozice2 == 1) { den*=10; den+=jmeno[pozice]-'0'; }
    else { mesic*=10; mesic+=jmeno[pozice]-'0'; }
   }
 }
if (den > 31) goto SPATNEDATUM;
if (den == 31 && (mesic == 2 || mesic == 4 || mesic == 6 || mesic == 9 || mesic == 11)) goto SPATNEDATUM;
if (den == 30 && mesic == 2) goto SPATNEDATUM;
if (den == 0 || mesic == 0 || mesic > 12) goto SPATNEDATUM;

textcolor(10); cprintf("\r%2u.%2d.  -  ",den,mesic);

pozice=0;
if (mesic > 1) { pozice+=31; mesic--; }
if (mesic > 1) { pozice+=29; mesic--; }
if (mesic > 1) { pozice+=31; mesic--; }
if (mesic > 1) { pozice+=30; mesic--; }
if (mesic > 1) { pozice+=31; mesic--; }
if (mesic > 1) { pozice+=30; mesic--; }
if (mesic > 1) { pozice+=31; mesic--; }
if (mesic > 1) { pozice+=31; mesic--; }
if (mesic > 1) { pozice+=30; mesic--; }
if (mesic > 1) { pozice+=31; mesic--; }
if (mesic > 1) pozice+=30;

pozice+=den; pozicejmeno=0;
while (--pozice > 0) pozicejmeno+=strlen(jmena+pozicejmeno)+1;
NAPISJMENO();
textcolor(9); cprintf("\r\nJmeno bylo nalezeno.");
goto OTAZKAZNOVU;

SPATNEDATUM:
textcolor(12); cprintf("\rJe zadano spatne datum!");

OTAZKAZNOVU:
textcolor(13); cprintf("  Znovu? [A/N]  A\b"); _setcursortype(_NORMALCURSOR);
switch(getch()) {
  case 0x1B: case 'n': case 'N': goto KONEC;
  default: goto NAPIS;
 }
KONEC:
textcolor(7); cprintf("   \n\r  \r"); _setcursortype(_NORMALCURSOR);
}