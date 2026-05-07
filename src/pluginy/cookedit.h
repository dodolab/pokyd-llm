#define POZICEJMENO1 10
#define POZICEJMENO2 14

void ZMEN_POCET_RADKU(signed char cislo) {
  switch(typedit) {
    case 1: pocetslpamet+=cislo; break;
    case 2: pocetjsem+=cislo; break;
    case 3: pocetjsi+=cislo; break;
   }
  pocetradku+=cislo;
 }

void ZAPIS_STAT(void) {
BYTE puvx=wherex(),puvy=wherey();
  if (zmena == 1) {
    gotoxy(1,25); textattr(14+3*16); cprintf("*");
   }
  if (typedit != 0 && typedit != 4) {
    textattr(11+16);
    gotoxy(54,3); cprintf("%2d ",pocetradku);
    gotoxy(66,3); cprintf("Pozice: %2d ",pozicekurzory+pozicesloupce+1);
   }
  gotoxy(puvx,puvy);
 }

void RADEK(BYTE *text) {
BYTE x=wherex(),y=wherey();
  gotoxy(3,25); _AX = 0x0920; _CX = 78; _BX = 3*16; geninterrupt(0x10);
								//smaz radek
  textattr(15+3*16); cprintf(text);
  gotoxy(x,y);
 }

void KURZOR_RADEK(BYTE zapnuti) {
BYTE text[70],pozicex,pozicey,poz,barva,delka,delkax2;
  if (zapnuti == 1) barva=7; else barva=6;
  if (typedit == 0) {					//uvodni menu
    pozicex=25; delka=33; pozicey=pozicekurzory+17;
   }
  else if (typedit == 4) {				//jmeno cloveka
    pozicex=31; delka=19; pozicey=pozicekurzory+5;
   }
  else {
    delka=19; pozicey=pozicekurzory+5;
    switch(pozicekurzorx) {
      case 0: pozicex=5; break;
      case 1: pozicex=31; break;
      case 2: pozicex=57; break;
     }
   }
  delkax2=delka<<1;
  gettext(pozicex,pozicey,pozicex+delka-1,pozicey,text);
  text[delkax2]=0;
  for (poz=1; poz <= delkax2; poz+=2)
   text[poz]=barva*16+14;
  puttext(pozicex,pozicey,pozicex+delka-1,pozicey,text);
 }

BYTE EDITUJ(BYTE menitradky) {
BYTE text[17],pozice,znak,pocatekx,pocetradek,pozicepamet,navrat,esc=1;
   pozicepamet=pozicekurzory+pozicesloupce;
START:
  if (esc == 1) RADEK("Napis novy obsah polozky nebo Esc pro stornovani.");
  else RADEK("Napis novy obsah polozky; akci nelze stornovat Escapem.");
  KURZOR_RADEK(1);
  switch(pozicekurzorx) {
    case 0: pocatekx=5; break;
    case 1: pocatekx=31; break;
    case 2: pocatekx=57; break;
   }

  if (menitradky == 1) {
    if (pozicepamet == pocetradku) ZMEN_POCET_RADKU(1);
   }

  gotoxy(pocatekx,pozicekurzory+5);
  textattr(14+7*16); cprintf("                  ");
  gotoxy(pocatekx+2,pozicekurzory+5); pozice=0;
  _setcursortype(_NORMALCURSOR);
  GETCH:
  switch(znak=getch()) {
    case 0: getch(); goto GETCH;
    case '\r': if (pozice == 0) goto GETCH; text[pozice]=0;
               switch(typedit) {
                 case 1: strcpy(slovapamet[pozicepamet][pozicekurzorx],text); break;
                 case 2: strcpy(slovajsem[pozicepamet],text); break;
                 case 3: strcpy(slovajsi[pozicepamet],text); break;
                }
	       zmena=1; navrat=1; break;
    case '\b': if (pozice > 0) { text[--pozice]=0; cprintf("\b \b"); }
               goto GETCH;
    case 0x1B: gotoxy(pocatekx+2,pozicekurzory+5); cprintf("              ");
               gotoxy(pocatekx+2,pozicekurzory+5);
               switch(typedit) {
                 case 1: cprintf(slovapamet[pozicepamet][pozicekurzorx]); break;
                 case 2: cprintf(slovajsem[pozicepamet]); break;
                 case 3: cprintf(slovajsi[pozicepamet]); break;
		} navrat=0; break;
    default: if (pozice >= 14 || znak < 32) goto GETCH;
	     if (typedit != 1 || pozicekurzorx == 0) {
               if (znak >= 'A' && znak <= 'Z') znak+=32;
               if (znak < 'a' || znak > 'z') goto GETCH;
              }
	     text[pozice++]=znak; cprintf("%c",znak);
             goto GETCH;
   }

  if (menitradky == 1) {
    if (typedit == 2) {
      if (slovajsem[pozicepamet][0] == 0) ZMEN_POCET_RADKU(-1);
     }
    else if (typedit == 3) {
      if (slovajsi[pozicepamet][0] == 0) ZMEN_POCET_RADKU(-1);
     }
   }

  if (typedit == 1) {			//nevyplnene vsechny 3 polozky
    esc=0;
    if (slovapamet[pozicepamet][0][0] == 0 && slovapamet[pozicepamet][1][0] == 0 && slovapamet[pozicepamet][2][0] == 0) {
      if (menitradky == 1) ZMEN_POCET_RADKU(-1);
     }
    else if (slovapamet[pozicepamet][0][0] == 0) { KURZOR_RADEK(0); pozicekurzorx=0; goto START; }
    else if (slovapamet[pozicepamet][1][0] == 0) { KURZOR_RADEK(0); pozicekurzorx=1; goto START; }
    else if (slovapamet[pozicepamet][2][0] == 0) { KURZOR_RADEK(0); pozicekurzorx=2; goto START; }
   }

  _setcursortype(_NOCURSOR);
  return(navrat);
 }

void VYTVOR_SLOUPCE(void) {
  _AX = 0x0600; _CX = 0x0100; _DX = 0x174F; _BH = 16; geninterrupt(0x10);
							//smaz obrazovku
  pozicesloupce=0; pozicekurzory=0;
  if (typedit == 1) pozicekurzorx=0;
  else pozicekurzorx=1;
  switch(typedit) {
    case 1: pocetradku=pocetslpamet; break;
    case 2: pocetradku=pocetjsem; break;
    case 3: pocetradku=pocetjsi; break;
   }
  textattr(11+16); gotoxy(5,3); cprintf("Editace faktu, typ dat: %d",typedit);
  gotoxy(39,3); cprintf("Pocet polozek: %d",pocetradku);
  return;
 }

void NAPISSLOVA(BYTE proc) {
BYTE pozice;
  textattr(14+6*16);
  if (proc == 0) {				//nove sloupce
    _AX = 0x0600; _CX = 0x041E; _DX = 0x1630; _BH = 6*16; geninterrupt(0x10);
    if (typedit == 1) {
      _AX = 0x0600; _CX = 0x0404; _DX = 0x1616; _BH = 6*16; geninterrupt(0x10);
      _AX = 0x0600; _CX = 0x0438; _DX = 0x164A; _BH = 6*16; geninterrupt(0x10);
     }
    for (pozice=0; pozice < 19; pozice++) {
      switch(typedit) {
        case 1:
          gotoxy(7,pozice+5); cprintf(slovapamet[pozice+pozicesloupce][0]);
          gotoxy(33,pozice+5); cprintf(slovapamet[pozice+pozicesloupce][1]);
          gotoxy(59,pozice+5); cprintf(slovapamet[pozice+pozicesloupce][2]);
          break;
        case 2:
          gotoxy(33,pozice+5); cprintf(slovajsem[pozice+pozicesloupce]);
          break;
        case 3:
          gotoxy(33,pozice+5); cprintf(slovajsi[pozice+pozicesloupce]);
       }
     }
   }
  else if (proc == 1) {				//dolu
    _AX = 0x0601; _CX = 0x041E; _DX = 0x1630; _BH = 6*16; geninterrupt(0x10);
    if (typedit == 1) {
      _AX = 0x0601; _CX = 0x0404; _DX = 0x1616; _BH = 6*16; geninterrupt(0x10);
      _AX = 0x0601; _CX = 0x0438; _DX = 0x164A; _BH = 6*16; geninterrupt(0x10);
     }
    switch(typedit) {
      case 1:
	gotoxy(7,23); cprintf(slovapamet[pozicesloupce+18][0]);
	gotoxy(33,23); cprintf(slovapamet[pozicesloupce+18][1]);
	gotoxy(59,23); cprintf(slovapamet[pozicesloupce+18][2]);
	break;
      case 2:
	gotoxy(33,23); cprintf(slovajsem[pozicesloupce+18]);
	break;
      case 3:
	gotoxy(33,23); cprintf(slovajsi[pozicesloupce+18]);
	break;
     }
   }
  else {					//nahoru
    _AX = 0x0701; _CX = 0x041E; _DX = 0x1630; _BH = 6*16; geninterrupt(0x10);
    if (typedit == 1) {
      _AX = 0x0701; _CX = 0x0404; _DX = 0x1616; _BH = 6*16; geninterrupt(0x10);
      _AX = 0x0701; _CX = 0x0438; _DX = 0x164A; _BH = 6*16; geninterrupt(0x10);
     }
    switch(typedit) {
      case 1:
	gotoxy(7,5); cprintf(slovapamet[pozicesloupce][0]);
	gotoxy(33,5); cprintf(slovapamet[pozicesloupce][1]);
	gotoxy(59,5); cprintf(slovapamet[pozicesloupce][2]);
	break;
      case 2:
	gotoxy(33,5); cprintf(slovajsem[pozicesloupce]);
	break;
      case 3:
	gotoxy(33,5); cprintf(slovajsi[pozicesloupce]);
	break;
     }
   }
 }

BYTE HLASKA(BYTE *text,BYTE barvapozadi) {
WORD barva;
BYTE znak,puvpozx=wherex(),puvpozy=wherey(),predtim[161],odkud;
BYTE klavesa;

  gettext(1,23,80,23,predtim);

  odkud=40-(strlen(text)>>1);
  gotoxy(odkud,23); textattr(15+(barvapozadi<<4)); cprintf(" %s ",text);
  sound(500); delay(100);
  gotoxy(odkud,23); textattr(12+(barvapozadi<<4)); cprintf(" %s ",text);
  delay(100); nosound();
  gotoxy(odkud,23); textattr(15+(barvapozadi<<4)); cprintf(" %s ",text);
  klavesa=getch(); if (klavesa == 0) getch();
  DAL:
  puttext(1,23,80,23,predtim);

  gotoxy(puvpozx,puvpozy);
  return(klavesa);
 }

void SMAZ_POLOZKU(void) {
BYTE pozice,aktpozice;
  aktpozice=pozicekurzory+pozicesloupce;
  if (aktpozice == pocetradku) {
    RADEK("Chces smazat neco, co vlastne ani neexistuje.");
    HLASKA("Toto misto neni polozka; sem muzes pouze napsat novou polozku!",2); return;
   }
  switch(typedit) {
    case 1: for (pozice=aktpozice; pozice < pocetradku; pozice++) {
	      strcpy(slovapamet[pozice][0],slovapamet[pozice+1][0]);
	      strcpy(slovapamet[pozice][1],slovapamet[pozice+1][1]);
	      strcpy(slovapamet[pozice][2],slovapamet[pozice+1][2]);
	     }
	    slovapamet[pocetradku-1][0][0]=0;
	    slovapamet[pocetradku-1][1][0]=0;
	    slovapamet[pocetradku-1][2][0]=0;
	    break;
    case 2: for (pozice=aktpozice; pozice < pocetradku; pozice++) {
	      strcpy(slovajsem[pozice],slovajsem[pozice+1]);
	     }
	    slovajsem[pocetradku-1][0]=0;
	    break;
    case 3: for (pozice=aktpozice; pozice < pocetradku; pozice++) {
	      strcpy(slovajsi[pozice],slovajsi[pozice+1]);
	     }
	    slovajsi[pocetradku-1][0]=0;
	    break;
   }
  ZMEN_POCET_RADKU(-1);
  if (pozicesloupce+18 > pocetradku && pozicesloupce > 0) {
    pozicesloupce--; pozicekurzory++;
   }
  NAPISSLOVA(0); KURZOR_RADEK(1);
  ZAPIS_STAT();
 }

void VLOZ_POLOZKU(void) {
BYTE pozice,aktpozice;
  if (pocetradku == 50) {			//plne
    RADEK("Maximalni pocet polozek je 50.");
    HLASKA("Vsech 50 polozek je jiz zaplneno! Musis nejakou smazat!",2); return;
   }
  aktpozice=pozicekurzory+pozicesloupce;
  switch(typedit) {
    case 1: for (pozice=pocetradku; pozice > aktpozice; pozice--) {
              strcpy(slovapamet[pozice][0],slovapamet[pozice-1][0]);
              strcpy(slovapamet[pozice][1],slovapamet[pozice-1][1]);
              strcpy(slovapamet[pozice][2],slovapamet[pozice-1][2]);
             }
            slovapamet[aktpozice][0][0]=0;		//vynulovani
            slovapamet[aktpozice][1][0]=0;		//novych
            slovapamet[aktpozice][2][0]=0;		//polozek
            break;
    case 2: for (pozice=pocetradku; pozice > aktpozice; pozice--)
	     strcpy(slovajsem[pozice],slovajsem[pozice-1]);
	    slovajsem[aktpozice][0]=0;			//vynulovani
	    break;
    case 3: for (pozice=pocetradku; pozice > aktpozice; pozice--)
	     strcpy(slovajsi[pozice],slovajsi[pozice-1]);
	    slovajsi[aktpozice][0]=0;			//vynulovani
	    break;
   }
  NAPISSLOVA(0);
  ZMEN_POCET_RADKU(1);
  if (EDITUJ(0) == 0) SMAZ_POLOZKU();
  else ZAPIS_STAT();
 }

void ZALOZ_NOVY(void) {
BYTE pozice;

  pocetslpamet=0; pocetjsem=0; pocetjsi=0;
  for (pozice=0; pozice < 50; pozice++) {
    slovapamet[pozice][0][0]=0; slovapamet[pozice][1][0]=0; slovapamet[pozice][2][0]=0;
    slovajsem[pozice][0]=0;
    slovajsi[pozice][0]=0;
   }
  zmena=1;
 }

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
    if (pozice == 0) { soubor[0]=0; goto KONEC; }//zabraneni blbostem v soubor[]
    else pozice--;
   }
  soubor[pozice+1]=0;
  KONEC:
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

BYTE CTI_INFORMACE_O_VETACH(void) {
BYTE pozice,pozice2;
WORD hodnota;
  SOUBOR("..\\PROFIL.PKD");
  if ((vety=fopen(soubor,"rb")) == NULL) return(1);
  fseek(vety,5,SEEK_SET);
  if (getc(vety) != 1) {				//verze: 1
    fclose(vety); return(2);
   }
  hlverze=getc(vety); vedlverze=getc(vety);		//verze Pokydu

  pocetslpamet=getc(vety); if (pocetslpamet > 50) pocetslpamet=0;
  for (pozice=0; pozice < pocetslpamet; pozice++) {
    fread(slovapamet[pozice][0],15,1,vety);
    fread(slovapamet[pozice][1],15,1,vety);
    fread(slovapamet[pozice][2],15,1,vety);
   }
  pocetjsem=getc(vety); if (pocetjsem > 50) pocetjsem=0;
  for (pozice=0; pozice < pocetjsem; pozice++)
   fread(slovajsem[pozice],15,1,vety);

  pocetjsi=getc(vety); if (pocetjsi > 50) pocetjsi=0;
  for (pozice=0; pozice < pocetjsi; pozice++)
   fread(slovajsi[pozice],15,1,vety);

  fread(jmenocloveka,15,1,vety);
  fread(jmenocloveka5pad,16,1,vety);

  fclose(vety); vety=NULL;
  return(0);
 }

BYTE ZAPIS_INFORMACE_O_VETACH(void) {
BYTE pozice,pozice2;
  SOUBOR("..\\PROFIL.PKD");
  ZNOVU:
  RADEK("Ukladam vety do souboru...");
  if ((vety=fopen(soubor,"wb")) == NULL) {
    RADEK("Chyba pri zapisovani souboru ..\\PROFIL.PKD.");
    DOTAZ: switch(HLASKA("Soubor se nepodarilo zapsat! Mam to zkusit znovu? [A/N] A",4)) {
      case 'a': case 'A': case 'y': case 'Y': case '\r': goto ZNOVU;
      case 'n': case 'N': return(1);
      default: goto DOTAZ;
     }
   }
  fwrite("Pokyd",5,1,vety);
  putc(1,vety);						//verze: 1
  putc(hlverze,vety); putc(vedlverze,vety);			//verze Pokydu

  putc(pocetslpamet,vety);
  for (pozice=0; pozice < pocetslpamet; pozice++) {
    fwrite(slovapamet[pozice][0],15,1,vety);
    fwrite(slovapamet[pozice][1],15,1,vety);
    fwrite(slovapamet[pozice][2],15,1,vety);
   }
  putc(pocetjsem,vety);
  for (pozice=0; pozice < pocetjsem; pozice++)
   fwrite(slovajsem[pozice],15,1,vety);

  putc(pocetjsi,vety);
  for (pozice=0; pozice < pocetjsi; pozice++)
   fwrite(slovajsi[pozice],15,1,vety);

  fwrite(jmenocloveka,15,1,vety);
  fwrite(jmenocloveka5pad,16,1,vety);

  fclose(vety); vety=NULL;
  return(0);
 }

void KONEC(BYTE otazka) {
  if (otazka == 1 && zmena == 1) {
    RADEK("Maji se ulozit nove vety a fakta, nebo se maji nechat puvodni?");
    OTAZKA:
    switch (HLASKA("Ulozit nove vety a fakta? [A/N] A",2)) {
      case 'a': case 'A': case 'y': case 'Y': case '\r':
        if (ZAPIS_INFORMACE_O_VETACH() == 0) break;
        else return;
      case 'n': case 'N': break;
      case 0x1B: return;
      default: goto OTAZKA;
     }
   }
  _AX = 3; geninterrupt(0x10);
  gotoxy(1,1); textattr(14); cprintf("Dekujeme za pouziti programu Cookies Editor pro Pokyd!\r\n");
  textattr(15); cprintf("Ales Janda  KYBLSoft 2001");
  textattr(7); cprintf("\r\n  \r");
  exit();
 }

void POHYB_SIPKAMA(void) {
BYTE pohyb,abspozice;
  KURZOR_RADEK(1);
  START:
  RADEK("Enter - editace, Insert - vlozeni polozky, Delete - smazani polozky.");
  GETCH:
  ZAPIS_STAT();
  abspozice=pozicekurzory+pozicesloupce;
  switch(getch()) {
    case 0: switch(getch()) {
      case 68: return;					//F10
      case 71: pohyb=4; goto POHYB;			//Home
      case 72: pohyb=0; goto POHYB;			//Nahoru
      case 73: pohyb=6; goto POHYB;			//PageUp
      case 75: pohyb=1; goto POHYB;			//Vlevo
      case 77: pohyb=2; goto POHYB;			//Vpravo
      case 79: pohyb=5; goto POHYB;			//End
      case 80: pohyb=3; goto POHYB;			//Dolu
      case 81: pohyb=7; goto POHYB;			//PageDown
      case 82: VLOZ_POLOZKU(); goto START;		//Insert
      case 83: SMAZ_POLOZKU(); zmena=1; goto START;	//Delete
      default: goto GETCH;
     }
    case '\r': EDITUJ(1); ZAPIS_STAT(); goto START;
    case 0x1B: return;					//Esc
    default: goto GETCH;
   }
  POHYB:
  switch(pohyb) {
    case 0: if (pozicekurzory > 0) { KURZOR_RADEK(0); pozicekurzory--; KURZOR_RADEK(1); }
	    else if (pozicesloupce > 0) { KURZOR_RADEK(0); pozicesloupce--; NAPISSLOVA(2); KURZOR_RADEK(1); }
	    break;
    case 1: if (typedit == 1 && pozicekurzorx > 0) {
              KURZOR_RADEK(0); pozicekurzorx--; KURZOR_RADEK(1);
             } break;
    case 2: if (typedit == 1 && pozicekurzorx < 2) {
              KURZOR_RADEK(0); pozicekurzorx++; KURZOR_RADEK(1);
             } break;
    case 3: if (abspozice == pocetradku) break;		  //konec
	    if (pozicekurzory < 18) { KURZOR_RADEK(0); pozicekurzory++; KURZOR_RADEK(1); }
	    else if (pozicesloupce < 50-19) { KURZOR_RADEK(0); pozicesloupce++; NAPISSLOVA(1); KURZOR_RADEK(1); }
            break;
    case 4: if (abspozice == 0) break;
            pozicekurzory=0; pozicesloupce=0; NAPISSLOVA(0); KURZOR_RADEK(1);
            break;
    case 5: if (abspozice == pocetradku || abspozice == 49) break;
            if (pocetradku > 18) {		//vice stranek
	      pozicekurzory=18; pozicesloupce=pocetradku-18;
	      if (pocetradku == 50) pozicesloupce--;
	      NAPISSLOVA(0); KURZOR_RADEK(1);
             }
            else {
              KURZOR_RADEK(0); pozicekurzory=pocetradku; KURZOR_RADEK(1);
             }
            break;
    case 6: if (abspozice == 0) break;			//na zacatku
            if (pozicekurzory > 0) {
              KURZOR_RADEK(0); pozicekurzory=0;
             }
            else {
	      if (pozicesloupce < 18) pozicesloupce=0;
              else pozicesloupce-=18;
              NAPISSLOVA(0);
             }
            KURZOR_RADEK(1);
            break;
    case 7: if (abspozice == pocetradku) break;		//konec
	    if (pozicekurzory == 18) {
	      if (abspozice+18 > pocetradku) pozicesloupce=pocetradku-18;
							//bude na konci
	      else pozicesloupce+=18;
	      NAPISSLOVA(0);
	     }
	    else if (pocetradku > 18) {
	      KURZOR_RADEK(0); pozicekurzory=18;
	     }
	    else {
	      KURZOR_RADEK(0); pozicekurzory=pocetradku;
	     }
	    KURZOR_RADEK(1);
            break;
   }
  goto GETCH;
 }

BYTE SAMOHLASKA(BYTE znak) {
  switch(znak&0x5F) {
    case 'A': case 'E': case 'I': case 'O': case 'U': case 'Y': return(1);
    default: return(0);
   }
 }

BYTE VYPUST_E(BYTE *jmeno) {		//vraci 1, je-li jmeno vyjimecne
BYTE prposledni=strlen(jmeno)-2,pozice;
  if ((jmeno[prposledni]&0x5F) == 'E') {
    if (prposledni > 0) {		//delsi jmeno nez 2 znaky
      if (SAMOHLASKA(jmeno[prposledni-1]) == 1) return(1);
     }
    for (pozice=0; pozice < prposledni; pozice++) { //musi byt i jina samohlaska
      if (SAMOHLASKA(jmeno[pozice]) == 1) goto DAL;
     }
    for (pozice=1; pozice < prposledni-1; pozice++) { //nebo slabikotvorne r,l
      switch(jmeno[pozice]) { case 'l': case 'L': case 'r': case 'R': goto DAL; }
     }
    return(1);
    DAL:
    jmeno[prposledni]=jmeno[prposledni+1];
    jmeno[prposledni+1]=0; return(0);
   }
  return(0);
 }

void VRAT_5_PAD(void) {
BYTE pomoc[17],posledni,poslznak,prposlznak;
  strcpy(jmenocloveka5pad,jmenocloveka);
  posledni=strlen(jmenocloveka5pad);
  if (posledni < 2) return;                    //kratke jmeno (0 nebo 1 znak)

  prposlznak=0; poslznak=0;

  strcpy(pomoc,jmenocloveka5pad+posledni-3);
  if (posledni >= 2) {                  //3 znaky a vic
    if (stricmp(pomoc,"tel") == 0) { poslznak='i'; goto DAL; }
    if (stricmp(pomoc,"del") == 0) { poslznak='i'; goto DAL; }
    if (stricmp(pomoc,"cel") == 0) { poslznak='i'; goto DAL; }
    if (stricmp(pomoc,"zen") == 0) { poslznak='e'; goto DAL; }
   }
  if (stricmp(jmenocloveka5pad,"vul") == 0) { strcpy(jmenocloveka5pad,"vole"); goto DAL; }
  if (stricmp(jmenocloveka5pad,"Dagmar") == 0) goto DAL;
  if (stricmp(jmenocloveka5pad,"Ingrid") == 0) goto DAL;
  if (stricmp(jmenocloveka5pad,"Miriam") == 0) goto DAL;
  if (stricmp(jmenocloveka5pad,"Ester") == 0) goto DAL;
  if (stricmp(jmenocloveka5pad,"Rut") == 0) goto DAL;
  if (stricmp(jmenocloveka5pad,"Rehor") == 0) { poslznak='i'; goto DAL; }
  if (stricmp(jmenocloveka5pad,"kral") == 0) { poslznak='i'; goto DAL; }

  posledni--;
  switch((jmenocloveka5pad[posledni]&0x5F)+32) {
    case 'a':
      prposlznak='o'; break;
    case 'b': case 'd': case 'f': case 'm': case 'p': case 'r': case 't':
    case 'v': case 'w':
      poslznak='e'; break;
    case 'c': poslznak=strlen(jmenocloveka5pad); VYPUST_E(jmenocloveka5pad);
              if (poslznak != strlen(jmenocloveka5pad)) poslznak='e';
              else poslznak='i'; break;			//jestli se E vynecha
    case 'g': case 'k':
      VYPUST_E(jmenocloveka5pad); poslznak='u'; break;
    case 'h':
      poslznak='u'; break;
    case 'j': case 's': case 'x': case 'z':
      poslznak='i'; break;
    case 'l': case 'n': case 'q':
      if (VYPUST_E(jmenocloveka5pad) == 1) poslznak='i';
      else poslznak='e'; break;
   }
  DAL:
  posledni=strlen(jmenocloveka5pad)-1;
  if (prposlznak != 0) jmenocloveka5pad[posledni]=prposlznak;
  jmenocloveka5pad[posledni+1]=poslznak;
  if (poslznak != 0) jmenocloveka5pad[posledni+2]=0;
 }

void JMENO_CLOVEKA(void) {
BYTE maxdelka,kterejmeno,pozice,znak,text[16],implicitnitext;
  pozicekurzory=POZICEJMENO1-5; pozicekurzorx=1;
  _AX = 0x0600; _CX = 0x0100; _DX = 0x174F; _BH = 16; geninterrupt(0x10);
  kterejmeno=1;
  textattr(11+16); gotoxy(33,POZICEJMENO1-2); cprintf("Jmeno cloveka:");
  textattr(14+6*16); gotoxy(31,POZICEJMENO1); cprintf("                   ");
  textattr(11+16); gotoxy(23,POZICEJMENO2-2); cprintf("Jmeno cloveka v 5. pade (osloveni):");
  textattr(14+6*16); gotoxy(31,POZICEJMENO2); cprintf("                   ");
  gotoxy(33,POZICEJMENO1); cprintf(jmenocloveka);
  gotoxy(33,POZICEJMENO2); cprintf(jmenocloveka5pad);
  KURZOR_RADEK(1);
  START:
  _setcursortype(_NOCURSOR); ZAPIS_STAT();
  implicitnitext=0; RADEK("Stiskem Enteru muzes editovat polozku, ktera je zvyraznena.");
  GETCHKURZOR:
  switch(getch()) {
    case 0: switch(getch()) {
      case 72: if (pozicekurzory != (POZICEJMENO1-5)) { KURZOR_RADEK(0); pozicekurzory=POZICEJMENO1-5; kterejmeno=1; KURZOR_RADEK(1); } break;
      case 80: if (pozicekurzory != (POZICEJMENO2-5)) { KURZOR_RADEK(0); pozicekurzory=POZICEJMENO2-5; kterejmeno=2; KURZOR_RADEK(1); } break;
     } goto GETCHKURZOR;
    case 0x1B: return;
    case '\r': goto EDITACE;
    default: goto GETCHKURZOR;
   }

EDITACE:
  ZAPIS_STAT();
  if ((pozicekurzory+5) == POZICEJMENO1) {
    kterejmeno=1; maxdelka=14;
    RADEK("Maximalni delka jmena je 14 znaku.     Enter - potvrzeni, Esc - storno");
   }
  else {
    kterejmeno=2; maxdelka=15;
    RADEK("Maximalni delka jmena je 15 znaku.     Enter - potvrzeni, Esc - storno");
   }

  if (kterejmeno == 2 && jmenocloveka[0] == 0) {
    kterejmeno=1; KURZOR_RADEK(0);
    pozicekurzory=POZICEJMENO1-5; KURZOR_RADEK(1);
    goto EDITACE;
   }

  gotoxy(33,pozicekurzory+5); textattr(14+7*16); cprintf("               ");
  if (implicitnitext == 0) pozice=0;			//vymaz policko
  else {
    strcpy(text,jmenocloveka5pad);
    gotoxy(33,pozicekurzory+5); cprintf(text);
    pozice=strlen(text);
   }
  gotoxy(33+pozice,pozicekurzory+5);
  _setcursortype(_NORMALCURSOR);
  GETCH:
  switch(znak=getch()) {
    case 0: getch(); goto GETCH;
    case '\r': text[pozice]=0;
               if (kterejmeno == 1) {
                 if (text[0] == 0 && jmenocloveka[0] != 0) {
                   _setcursortype(_NOCURSOR);
                   switch(HLASKA("Opravdu chcete vymazat jmeno? [A/N] A",2)) {
                     case 'a': case 'A': case 'y': case 'Y': case '\r': break;
                     default: textattr(14+7*16); _setcursortype(_NORMALCURSOR); goto GETCH;
                    }
                  }
                 strcpy(jmenocloveka,text);
                }
               else strcpy(jmenocloveka5pad,text);
               zmena=1; break;
    case '\b': if (pozice > 0) { text[--pozice]=0; cprintf("\b \b"); }
               goto GETCH;
    case 0x1B: gotoxy(33,pozicekurzory+5); cprintf("               ");
               gotoxy(33,pozicekurzory+5);
               if (kterejmeno == 1) cprintf(jmenocloveka);
               else cprintf(jmenocloveka5pad);
	       goto START;
    default: if (pozice >= maxdelka || znak < ' ') goto GETCH;
             if ((znak >= 'A' && znak <= 'Z') ||
                 (znak >= 'a' && znak <= 'z') ||
                 (znak >= '0' && znak <= '9'));
             else goto GETCH;
	     text[pozice++]=znak; cprintf("%c",znak);
             goto GETCH;
   }
  _setcursortype(_NOCURSOR);

  if (kterejmeno == 1) {
    if (jmenocloveka[0] != 0) {
      VRAT_5_PAD(); kterejmeno=2;
      KURZOR_RADEK(0); pozicekurzory=POZICEJMENO2-5; KURZOR_RADEK(1);
      implicitnitext=1; goto EDITACE;
     }
    else {
      jmenocloveka5pad[0]=0;
      textattr(6*16+14); gotoxy(33,POZICEJMENO2); cprintf("               ");
     }
   }

  goto START;
 }
