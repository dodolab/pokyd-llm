#include <stdio.h>
#include <conio.h>
#include <dos.h>
#include <string.h>
#include <math.h>

#define BYTE unsigned char
#define WORD unsigned short
#define DWORD unsigned long
#define DLOUHE long double

BYTE retezec1[350],retezec2[350],chyba=0;
WORD cispozice,pozice;
DLOUHE vysledek;


#include "kalkul.h"

int matherr(struct exception *a) {
  if (chyba == 0) { textcolor(12); cprintf("\n\rChyba pri pocitani!\n\n\r"); chyba=1; }
  return(1);
 }

void main(int argc, char *argv[]) {
textattr(11); cprintf("\rAles Janda                     Kalkulacka v1.3                     KYBLSoft 2000\n");
if (argc > 1) {
  retezec1[0]=0; pozice=1;
  while (pozice < argc) {
    strcat(retezec1,argv[pozice]); pozice++;
   }
  textcolor(14); cprintf("%s\n\r",retezec1);
 }
else {
START:
  chyba=0; textcolor(13); cprintf("Zadej priklad:\r\n");
  NAPIS();
 }
ZKONTROLUJ1();
if (ZKONTROLUJ2() == 1) goto START;
pozice=strlen(retezec1);
retezec1[pozice]=')'; retezec1[pozice+1]=0;
pozice=0; VLOZZAVORKY();
retezec1[pozice+2]=0;
textcolor(2); cprintf("\n\rPriklad: %s\b ",retezec1);
pozice=0; vysledek=VYPOCITEJ();
if (chyba == 1) goto START;
textcolor(10); cprintf("\n\n\rVysledek:  ");
sprintf(retezec2,"%Lf",vysledek);
 UPRAVVYSLEDEK();
if (strcmp(retezec2,"0") == 0) sprintf(retezec2,"%Lg",vysledek);
cprintf("%s\n\n\r",retezec2);

goto START;
}