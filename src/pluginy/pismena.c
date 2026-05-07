#include <stdio.h>
#include <conio.h>
#include <time.h>

unsigned short sloupec,r,p=0,t=500;
char A,B,C,D,zvuk=1,kdejey,barva,radek;

void main() {
  srand(time(NULL)); nosound();
  _setcursortype(_NOCURSOR);
START:
  p=0; clrscr(); textcolor(11); textbackground(0);
  cprintf("\rAles Janda                        Pismena v1.3                     KYBLSoft 2000\n");
  textcolor(14);
  cprintf("Cilem hry je chytnou padajici pismeno tim, ze ho stisknes. Nesmi spadnout az na\n\r"
          "zem, rozlisuji se mala a velka pismena.\n\r"
          "Preji prijemnou zabavu!\n\n\r");
  textcolor(15);
  gotoxy(30,8);  cprintf("Vyber obtiznost hry:\n\r");
  gotoxy(30,10);  cprintf("     (A)mater");
  gotoxy(30,11);  cprintf("  (P)rofesional");
  gotoxy(30,12); cprintf("     (M)istr");
  gotoxy(30,13); cprintf("     (B)lazen");
  gotoxy(30,15); cprintf("  (Z)vuk:   ano"); kdejey=wherey();
  textcolor(10);
  gotoxy(30,17); cprintf("    * (K)onec *");
RUP:
  switch(C=getch()) {
    case 'a': case 'A': r=200; break;
    case 'p': case 'P': r=150; break;
    case 'm': case 'M': r=100; break;
    case 'b': case 'B': r=50; break;
    case 'k': case 'K': case 0x1B: goto KONEC;
    case 'z': case 'Z': if (zvuk == 1) zvuk=0; else zvuk=1;
      textcolor(15); gotoxy(42,kdejey); cprintf(zvuk == 0 ? "ne " : "ano");
    case 0: default: goto RUP;
   }
  clrscr(); textcolor(12);
  gotoxy(30,12); cprintf("Pripravit ke startu!"); delay(1000);
  gotoxy(30,12); cprintf("       Pozor!       "); delay(1000);
  gotoxy(30,12); cprintf("       Ted!!!"); delay(1000);
R:if ((p%20) == 0) clrscr();
  if(r==0) {
    nosound();
    gotoxy(9,20); textcolor(15);
    cprintf("Jsi velice dobry hrac. Dosahl(a) jsi maximalni mozne rychlosti.");
    gotoxy(21,21); cprintf("Zaslouzis si velkou pochvalu a uznani."); getch();
    goto START;
   }
  gotoxy(1,25); textcolor(13);
  cprintf("Pocet bodu:  ");
  textcolor(15); cprintf("%u",p);
  barva=rand()%5+10;
  t=500;
  textcolor(barva);
  A=rand()%('Z'-'A')+'A';
  if (rand()%2 == 0) A+=32;			//mala pismena
  sloupec=rand()%79+1;
  radek=1;
S:gotoxy(sloupec,radek);
  radek++; t-=10;
  if (zvuk == 1) sound(t);
  cprintf("%c",A);
  delay(r);
  if (radek==25) goto X;
  if (kbhit()!=0) goto L;
  else goto S;
L:B=getch();
  if (A == B) {
    switch(C & 0x5F) {
      case 'A': p++; break;
      case 'P': p+=2; break;
      case 'M': p+=4; break;
      case 'B': p+=8; break;
     }
    r-=2;
    goto R;
   }
  else {
X:  gotoxy(30,25); textcolor(12);
    cprintf("Konec hry. Chces hrat znovu ?"); nosound();
    delay(100); while (kbhit() != 0) getch();
GETCH:
    switch(getch()) {
      case 'a': case 'A': case 'y': case 'Y': case '\r': goto START;
      case 'n': case 'N': case 0x1B: goto KONEC;
      default: goto GETCH;
     }
   }
KONEC:
  clrscr();
  textcolor(7); cprintf(" \n\r \r");
  _setcursortype(_NORMALCURSOR);
 }
