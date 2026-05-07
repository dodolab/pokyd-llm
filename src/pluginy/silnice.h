BYTE PRECTI_ZNAK(void) {
BYTE znak;
  znak=getc(f)^'K';
  soucet^=znak; soucet+=znak;
  return(znak);
 }

void ZAPIS_ZNAK(BYTE znak) {
  putc(znak^'K',f);
  soucet^=znak; soucet+=znak;
 }

BYTE VRAT_NEBEZPECI(void) {
  if (pozicex < 15 && smery[pozicey] == 'R') return(1);
  if (pozicex > 63 && smery[pozicey] == 'L') return(1);		//aby vyjel
  if (prekazky[pozicey+1][pozicex] != ' ' || prekazky[pozicey+1][pozicex+1] != ' ')
   return(1);
  if (smery[pozicey+1] == 'R' && prekazky[pozicey+1][pozicex-1] != ' ')
   return(1);
  if (smery[pozicey+1] == 'L' && prekazky[pozicey+1][pozicex+2] != ' ')
   return(1);
  return(0);
 }

DWORD CAS18() {
DWORD cas18;

  _AH=0;
  geninterrupt (0x1A);
  cas18=_CX;
  cas18<<= 16;
  cas18+=_DX;
  return(cas18);
 }

void ZVYS_BODY(void) {
  body++; gotoxy(51,1); textattr(15+4*16); cprintf("%4u",body);
  if (body%500 == 0) {
    zivoty++; gotoxy(66,1); textattr(15+4*16);
    cprintf("%2d",zivoty); if (zvuk == 1) { sound(500); delay(200); sound(530); delay(20); } else delay(220);
   }
 }

BYTE ZAPIS_CAS(void) {
static DWORD puvodnicas=0;
  if (CAS18() > puvodnicas+level/3+5) {
    puvodnicas=CAS18();
    gotoxy(cas,25); textattr(0); cprintf(" ");
    if (cas == 1) return(1);
    else cas--;
   }
  return(0);
 }

#define HUSTOTA (level/7+2)

void VYGENERUJ_PREKAZKY(void) {
BYTE pozice1,pozice2,autak=0;
  for (pozice1=0; pozice1 < 21; pozice1++) {
    for (pozice2=0; pozice2 < 80; pozice2++) {
      if (autak == 0) {
        if (rand()%hustota == 0) { prekazky[pozice1][pozice2]='й'; autak=1; goto DAL; }
       }
      if (autak == 1) {				//zbytek auta
        if (rand()%HUSTOTA != 0) { prekazky[pozice1][pozice2]='к'; goto DAL; }
        else { prekazky[pozice1][pozice2]='л'; autak=2; goto DAL; }
       }
      prekazky[pozice1][pozice2]=' '; autak=0;
      DAL:;
     }

    if (smery[pozice1-2] == 'L' && smery[pozice1-1] == 'L') smery[pozice1]='R';
    else if (smery[pozice1-2] == 'R' && smery[pozice1-1] == 'R') smery[pozice1]='L';
    else if (rand()%2 == 0 && pozice1 > 0) {
      if (smery[pozice1-1] == 'L') smery[pozice1]='R'; else smery[pozice1]='L';
     }
    else if (rand()%2 == 0) smery[pozice1]='L';		//vlevo
    else smery[pozice1]='R';				//vpravo
    if (rand()%2 == 0) rychlejsi[pozice1]='R';
    else rychlejsi[pozice1]='N';
    prekazky[pozice1][80]=0;
   }
  for (pozice1=0; pozice1 < 80; pozice1++)
   prekazky[21][pozice1]=' ';
  prekazky[21][80]=0;

  gotoxy(2,25); textattr(12);
  for (cas=1; cas < 79; cas++) {
    cprintf("ш");
   }
  pozicex=40; pozicey=24;
  if (level < 5) rychlostdema=4;
  else if (level < 12) rychlostdema=3;
  else if (level < 18) rychlostdema=2;
  else rychlostdema=1;
 }

BYTE POPOSUN_PREKAZKY(void) {
BYTE pozice1,pozice2,barva;
static BYTE smer='L';
static DWORD cas;
DWORD pomoc;

  pomoc=CAS18();
  if (pomoc > cas) nosound();
  if (pomoc < cas+rychlost && pomoc >= cas) goto DAL;	//2. podm. - pulnoc
  cas=pomoc;

  if (zvuk == 1) { if (smer == 'L') sound(200); else sound(300); }
  for (pozice1=0; pozice1 < 22; pozice1++) {
    if (smery[pozice1] == 'L' && (smer == 'L' || rychlejsi[pozice1] == 'R')) {
      for (pozice2=0; pozice2 < 79; pozice2++) {
        prekazky[pozice1][pozice2]=prekazky[pozice1][pozice2+1];
       }
      switch (prekazky[pozice1][78]) {
	case 202: case 203: if (rand()%HUSTOTA == 0) prekazky[pozice1][79]='л';
		  else prekazky[pozice1][79]='к'; break;
	case 204: prekazky[pozice1][79]=' '; break;
	case ' ': if (rand()%hustota == 0) prekazky[pozice1][79]='й';
		  else prekazky[pozice1][79]=' '; break;
	default: break;
       }
     }
    else if (smery[pozice1] == 'R' && (smer == 'R' || rychlejsi[pozice1] == 'R')) {
      for (pozice2=79; pozice2 > 0; pozice2--) {
        prekazky[pozice1][pozice2]=prekazky[pozice1][pozice2-1];
       }
      switch (prekazky[pozice1][1]) {
	case 204: case 203: if (rand()%HUSTOTA == 0) prekazky[pozice1][0]='й';
			    else prekazky[pozice1][0]='к'; break;
	case 202: prekazky[pozice1][0]=' '; break;
	case ' ': if (rand()%hustota == 0) prekazky[pozice1][0]='л';
		  else prekazky[pozice1][0]=' '; break;
       }
     }
    if (smery[pozice1] == 'L') barva=2; else barva=3;
    if (rychlejsi[pozice1] == 'R') barva+=8;
    gotoxy(1,pozice1+3); textattr(barva); cprintf(prekazky[pozice1]);
   }

  if (smer == 'L') smer='R'; else smer='L';

  DAL:
  ZAPIS_CAS();
  gotoxy(pozicex,pozicey);
  if (prekazky[pozicey-3][pozicex-1] > 200 || prekazky[pozicey-3][pozicex] > 200) {
    textattr(12); cprintf("мн"); if (zvuk == 1) { sound(100); delay(500); sound(60); delay(700); } else delay(1200);
    zivoty--;
    while (kbhit() != 0) getch();
    return(1);
   }
  else {
    textattr(14); cprintf("хи");
   }

  gotoxy(51,1); textcolor(15); textbackground(4); cprintf("%4u",body);

  return(0);
 }

void ZAVED_FONT(void) {
  reg.r_ax = 0x1100;
  reg.r_cx = 7;
  reg.r_dx = 200;
  reg.r_bx = 0x1000;
  reg.r_es = FP_SEG(font);
  reg.r_bp = FP_OFF(font);
  intr(0x10,&reg);
 }

/*
void NASTAV_NORMALNI_FONT(void) {
BYTE *tabulka;
  reg.r_ax = 0x1130;
  reg.r_bx = 0x0600;
  intr(0x10,&reg);
  tabulka=MK_FP(reg.r_es,reg.r_bp);			//vraceni fontu

  tabulka+=200*16;

  reg.r_ax = 0x1100;
  reg.r_cx = 7;
  reg.r_dx = 200;
  reg.r_bx = 0x1000;
  reg.r_es = FP_SEG(tabulka);
  reg.r_bp = FP_OFF(tabulka);
  intr(0x10,&reg);
 }*/

void SOUBOR(BYTE *souborcat) {
char pozice,cislo;
int *ukazatel;
char *uk2;
char rootsoubor[160],assetssoubor[160];
FILE *assettest;
  _AH = 0x62;
  geninterrupt (0x21);
  ukazatel = (int *)MK_FP(_BX,0x2C);
  uk2 = (char *)MK_FP(*ukazatel,0);
  for (;;uk2++) {
    if (*uk2 == 0 && *(uk2+1) == 0 && *(uk2+2) == 1 &&
	*(uk2+3) == 0) break;
   }
  uk2+= 4;				//uk2=prikazovy radek
  for(cislo=0; cislo < 80; cislo++) {
    if ((soubor[cislo] = *uk2++) == 0) break;
   }
  pozice=strlen(soubor)-1;
  while (soubor[pozice] != '\\') {
    if (pozice == 0) { soubor[0]=0; return; }	//zabraneni blbostem v soubor[]
    else pozice--;
   }
  soubor[pozice+1]=0;
  strcpy(rootsoubor,soubor);
  strcat(rootsoubor,souborcat);
  strcpy(soubor,rootsoubor);

  strcpy(assetssoubor,soubor);
  strcat(assetssoubor,"assets\\");
  strcat(assetssoubor,souborcat);
  assettest=fopen(assetssoubor,"rb");
  if (assettest != NULL) {
    fclose(assettest);
    strcpy(soubor,assetssoubor);
   }
 }
