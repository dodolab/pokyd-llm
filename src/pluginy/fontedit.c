#include <stdio.h>
#include <conio.h>

#define SIRKA 8
#define DELKA (7*SIRKA)
typedef unsigned char BYTE;
typedef unsigned int WORD;

WORD biosreadkey()
{
  WORD znak;
  asm {
    mov ah,0
    int 0x16
    mov znak,ax
  }
  return(znak);
}

void getxy(BYTE x, BYTE y)
{
  asm {
    mov ah,3
    mov bh,0
    int 0x10
    mov x,DL
    mov y,DH
  }
}

main() {
  FILE *f;
  BYTE bit=0x80,znak=0,cast,pozice;
  BYTE x=1,y=1;
  WORD c;
  static BYTE pole[DELKA];

  if ((f=fopen("TISK.DAT","rb")) == NULL) {
    puts("Nemuzu otevrit TISK.DAT !!!");
    return;
  }
  fread(pole, 2, DELKA, f);
  fclose(f);

  textmode(1);
  gotoxy(1,18); puts("Pohyb mezi znaky 0 a‘ 255: PgUp a PgDn");
  gotoxy(1,20); puts("Pohyb uvnit© znaku: ›ipky");
  gotoxy(1,22); puts("Ulo‘en¡ + konec: ESC");
  gotoxy(1,24); puts("Zmˆna hodnoty bitu: jin˜ znak");
  gotoxy(1,1);
ZNAKY:
  x=wherex(),y=wherey();
  for (cast=0; cast < 16; cast++) {
    bit=pole[(znak<<4)+cast]; gotoxy(1,cast+1);
    for (pozice=0; pozice < 8; pozice++) {
      if (bit > 127) printf("Û");
      else printf(" ");
      bit<<=1;
     }
   }
  gotoxy(20,1);
  printf("Znak: %3d  ('%c')",znak,znak);
  gotoxy(x,y);
  if ((c = biosreadkey()) == 0x4900) {           //Page Up
    if (znak != 0) znak--;
    goto ZNAKY;
  }
  if (c == 0x5100) {                             //Page Down
    if (znak != 255) znak++;
    goto ZNAKY;
  }
  if (c != 0x11B) {                              //ESC
    if (c == 0x4800) {                           //sipka NAHORU
      if (wherey() > 1) gotoxy(wherex(),wherey()-1);
    }
    else if (c == 0x5000) {                      //sipka DOLU
      if (wherey() < 16) gotoxy(wherex(),wherey()+1);
    }
    else if (c == 0x4D00) {                      //sipka VPRAVO
      if (wherex() < 8) gotoxy(wherex()+1,wherey());
    }
    else if (c == 0x4B00) {                      //sipka VLEVO
      if (wherex() > 1) gotoxy(wherex()-1,wherey());
     }
    else {
      bit=1<<(8-wherex());
      pole[(znak<<4)+(wherey()-1)]^=bit;
     }
    goto ZNAKY;
   }
KONEC:
  textmode(3);
  if ((f=fopen("TISK.DAT","wb")) == NULL) {
    puts("Nemuzu otevrit TISK.DAT !!!");
    return;
  }
  fwrite(pole, 2, DELKA, f);
  fclose(f);
  return;
}