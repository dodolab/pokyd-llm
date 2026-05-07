void KONEC(void) {
  textattr(7); cprintf("\n \r \n \r"); exit();
 }

void NAPISZIVOTY(void) {
  textattr(14+6*16);
  gotoxy(65,1); cprintf("%2d",zivotyclovek);
  gotoxy(78,1); cprintf("%2d",zivotypocitac);
 }

void VYSTREL(BYTE kdo) {
BYTE kterastrela;
  if (kdo == 0) {					//clovek
    kterastrela=0;
    while (strelaclovek[kterastrela][0] != 0) kterastrela++;	//hledej
    strelaclovek[kterastrela][0]=5; strelaclovek[kterastrela][1]=poziceclovek;
   }
  else {
    kterastrela=0;
    while (strelapocitac[kterastrela][0] != 0) kterastrela++;	//hledej
    strelapocitac[kterastrela][0]=76; strelapocitac[kterastrela][1]=pozicepocitac;
   }
 }

void SMAZDELO(BYTE jake) {
  if (jake == 0) gotoxy(2,poziceclovek);	//clovek
  else gotoxy(77,pozicepocitac);		//pocitac
  textattr(0); cprintf("   ");
 }

void NAKRESLI_DELA(void) {

  gotoxy(2,poziceclovek);			//clovek
  textattr(2*16); cprintf("  ");
  textattr(2); cprintf(">");

  gotoxy(77,pozicepocitac);			//pocitac
  textattr(4); cprintf("<");
  textattr(4*16); cprintf("  ");
 }

void POPOSUN_STRELY(void) {
BYTE pozice;
  for (pozice=0; pozice < POCET_STREL; pozice++) {		//clovek
    if (strelaclovek[pozice][0] == 0) goto CLOVEKKONEC;
    gotoxy(strelaclovek[pozice][0],strelaclovek[pozice][1]);
    textattr(10); cprintf(" ÄÄ");
    strelaclovek[pozice][0]++;
    if (strelaclovek[pozice][0] == 77) {		//na konci
      cprintf("\b\b  ");				//vymazat strelu
      strelaclovek[pozice][0]=0;
      if (pozicepocitac == strelaclovek[pozice][1]) {	//zasah
        zivotypocitac--; sound(1000);
       }
     }
    CLOVEKKONEC:;
   }
  for (pozice=0; pozice < POCET_STREL; pozice++) {		//pocitac
    if (strelapocitac[pozice][0] == 0) goto POCITACKONEC;
    strelapocitac[pozice][0]--;
    gotoxy(strelapocitac[pozice][0],strelapocitac[pozice][1]);
    textattr(12); cprintf("ÄÄ ");
    if (strelapocitac[pozice][0] == 4) {		//na konci
      cprintf("\b\b\b   ");				//vymazat strelu
      strelapocitac[pozice][0]=0;
      if (poziceclovek == strelapocitac[pozice][1]) {	//zasah
        zivotyclovek--; sound(200);
       }
     }
    POCITACKONEC:;
   }
 }

void ZACATEK(void) {
BYTE kdejey;
  _AX = 3; geninterrupt(0x10); _setcursortype(_NOCURSOR); nosound();
  textattr(11); cprintf("Ales Janda                        Palba v1.1                       KYBLSoft 2000\n\r");
  textattr(10); cprintf(""
  "V teto hre jde o to, ze musite (jste zeleny ctverecek vlevo) sestrelit sveho\n\r"
  "nepritele (cerveny ctverecek vpravo).\n\n\r"
  "Ovladani:  Pohyb:     sipky nahoru/dolu\n\r"
  "           Strileni:  Shift\n\n\r");
  kdejey=wherey();
NASTAVENI: gotoxy(1,kdejey);
  textattr(14); cprintf(""
  "Nastaveni hry:\n\r"
  "        Rychlost:  %2d      zmena:  +,-     (0..nejrychlejsi .. 10..nejpomalejsi)\r"
  "          Zivotu:  %2d      zmena:  vlevo/vpravo                         (5 - 95)\r"
  "       Obtiznost:  %s zmena:  1,2,3                    (mala/stredni/velka)\r"
  "   Hustota strel:  %2d      zmena:  nahoru/dolu     (1..nejvetsi .. 40..nejmensi)\n\r",rychlost,zivoty,obtiznost == 0 ? "mala   " : obtiznost == 1 ? "stredni" : "velka  ",hustotastrel);
  textattr(15); cprintf("Jina klavesa - START HRY !");
  switch(getch()) {
    case '+': if (rychlost < 10) rychlost++; goto NASTAVENI;
    case '-': if (rychlost > 0) rychlost--; goto NASTAVENI;
    case '1': obtiznost=0; goto NASTAVENI;
    case '2': obtiznost=1; goto NASTAVENI;
    case '3': obtiznost=2; goto NASTAVENI;
    case 0x1B: KONEC();
    case 0: switch(getch()) {
      case 72: if (hustotastrel < 40) hustotastrel++; goto NASTAVENI;//nahoru
      case 75: if (zivoty > 5) zivoty-=5; goto NASTAVENI;	//vlevo
      case 77: if (zivoty < 95) zivoty+=5; goto NASTAVENI;	//vpravo
      case 80: if (hustotastrel > 1) hustotastrel--; goto NASTAVENI;	//dolu
     }
    default: return;
   }
 }

BYTE VRAT_NEBEZPECI(void) {
BYTE pozice=0;
  while (pozice < POCET_STREL) {
    if (strelaclovek[pozice][0] > 74 && strelaclovek[pozice][1] == pozicepocitac)
     return(1);						//nebezpeci
    pozice++;
   }
  return(0);
 }
