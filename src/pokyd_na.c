/* Tento zdrojovy kod je pod licenci GNU/GPL. Muzete ho pouzit k vlastni
   potrebe, ale nesmite jej ani programy zalozene na tomto kodu vyuzit komercne!

   Jedna se o zdrojovy kod programu Pokyd (http://iqpokyd.kyblsoft.cz)
   od Alese Jandy, aktivne vyvijeneho 1999 - 2002
*/

/*
 * pokyd_na.c - fullscreen UI nastaveni (NASTAV): mys, klavesove sipky, barvy, radky textu.
 * Makra S1/S2 oddeluji dve "stranky" formulare; funkce je dlouha - menit opatrne kvuli stacku.
 */

#define ZACATEK_OBR1 6
#define ZACATEK_OBR2 6
#define CELK_ZACATEK_OBR 6
#define ZACATEK_TEXTU_X 7
#define HODNOTY_X 64

/* Rutina NASTAV - viz implementace a nazvy promennych (konvence Pokyd). */
void far NASTAV(void) {

#define S1 if (strana == 1) {
#define S2 if (strana == 2) {

int mys,puvpozx,puvpozy,pozx,pozy,psanipredtim=psani;
DWORD pomoc,cassipka,castext,cassetric;
BYTE pomoc1,kdejex,kdejey,sipkapozice=ZACATEK_OBR1;
BYTE pozicemyssipka,strana=1,opakovani=0;
BYTE typsipky,pozicetextefekt,znak,stavtlacitek;

#if krokovani == 0
  kdejex=wherex(); kdejey=wherey();
START0:
  SMAZOBRAZOVKU(1); SMAZKURZOR();

START:
  if (opakovani == 1) {				//smaz cast obrazovky
    _CX = 0x0300; _DX = 0x1550; _AX = 0x0600; _BH = 0; geninterrupt(0x10);
   }
  jeli_nastaveni=2;				//JE prostredi k nastaveni
  grafika25=1;
  PIS(31,1," Pokyd - nastaveni ",14+16);
  ZAPISCAS();

  VRATDATA(strana+16);		//1. strana - 17,   2. strana - 18

  textattr(0); gotoxy(1,4); NAPIS_ODSTAVEC(dlouhe);

  if (opakovani != 0) goto PSANI;		//preskoc inicializaci mysi

  _AX = 0; geninterrupt(0x33);				//inicializuj mys
  if (_AX > 0) mys=1;

  if (mys == 1) {

    textbackground(2); gotoxy(1,3); cprintf(" ");		//kurzor mysi
    textbackground(0);

    _AX = 7; _CX = 1<<3; _DX = 80<<3; geninterrupt(0x33);
    _AX = 8; _CX = 1<<3; _DX = 23<<3; geninterrupt(0x33);	//rozpeti

    _AX = 4; _CX = 1; _DX = 1; geninterrupt(0x33);	//nastav pozici
    puvpozx=1; puvpozy=1;

    _AX = 0x1A; _BX = 50; _CX = 50; _DX = 100;
    geninterrupt(0x33);					//nastaveni citlivosti
   }
  opakovani=1;

  goto PSANI;

STANDARD:
  switch (HLASKA("Opravdu vsechna nastaveni dat standardne? [A/N] A",2+100)) {
    case 'a': case 'A': case 'y': case 'Y': case '\r':
      if (font == 0) NASTAVPOKYDFONT(); NASTAV_STANDARD(); break;
    default: break;
   }

PSANI:
  SMAZ_EFEKT(sipkapozice);
  textcolor(15);
  if (strana == 1) {
    gotoxy(HODNOTY_X,6); cprintf("%s",zenskyrod == 0 ? "muz " : "zena");
    gotoxy(HODNOTY_X,7); cprintf("%s",zenapocitac == 0 ? "muz " : "zena");
    NASTAV_NALADU(nalada,8,0);
    ZAPIS_NALADU();
    textcolor(15);
    ZAPIS_CHARAKTER(9);
    gotoxy(HODNOTY_X,10); cprintf(jmenomuzpocitac);
    gotoxy(HODNOTY_X,11); cprintf(jmenozenapocitac);
    gotoxy(HODNOTY_X,12); cprintf("%s",(pocetpocitacu-1) == 1 ? "ano" : "ne ");
    gotoxy(HODNOTY_X,13); cprintf("%s",nastvani == 1 ? "ano" : "ne ");
    gotoxy(HODNOTY_X,14); cprintf("%s",zvuk == 1 ? "ano" : "ne ");
    gotoxy(HODNOTY_X,15); cprintf("%lu  ",pocetvterin);
    gotoxy(HODNOTY_X,16); cprintf("%s",zvysovani == 1 ? "ano" : "ne ");
    gotoxy(HODNOTY_X,17); cprintf("%lu  ",delayprocenta);
    gotoxy(HODNOTY_X,18); cprintf("%s",font == 1 ? "ano" : "ne ");
    gotoxy(HODNOTY_X,19); cprintf("%d ",pocetradku);
    gotoxy(HODNOTY_X,20); cprintf("%s",textefekty == 1 ? "ano" : "ne ");
    gotoxy(HODNOTY_X,21); cprintf("%lu   ",pocetsetricvterin);
   }
  if (strana == 2) {
    gotoxy(HODNOTY_X,6); cprintf("%s",mod == 25 ? "25     " : mod == 50 ? "50     " : "puvodni");
    gotoxy(HODNOTY_X,7); cprintf("%s",slovenstina == 0 ? "ne " : "ano");
    gotoxy(HODNOTY_X,8); cprintf("%s",pravopis == 1 ? "ano" : "ne ");
    gotoxy(HODNOTY_X,9); cprintf("%s",psani == 0 ? "ne " : "ano");
    gotoxy(HODNOTY_X,10); cprintf("%s",extrasouborpsani == 1 ? "ano" : "ne ");
    gotoxy(HODNOTY_X,11); cprintf("%lu  ",vyska);
    gotoxy(HODNOTY_X,12); cprintf("%s",svtipy == 1 ? "ano    " : svtipy == 0 ? "ne     " : "nahodne");
    gotoxy(HODNOTY_X,13); cprintf("%s",spocasi == 1 ? "ano    " : spocasi == 0 ? "ne     " : "nahodne");
    gotoxy(HODNOTY_X,14); cprintf("%d ",vkolikhodin);
    gotoxy(HODNOTY_X,15); cprintf("%s",yz == 1 ? "ano" : "ne ");
    gotoxy(HODNOTY_X,16); cprintf("%s",cekaniklavesy == 1 ? "ano" : "ne ");
    gotoxy(HODNOTY_X,17); cprintf("%s",ulozeninakonci == 1 ? "ano" : "ne ");
    gotoxy(HODNOTY_X,18); cprintf("%s",akcectrlbreak == 0 ? "ne   " : akcectrlbreak == 1 ? "ano  " : "reset");
    gotoxy(HODNOTY_X,19); textcolor(barvaclovek); cprintf("barva #%d ",barvaclovek);
    gotoxy(HODNOTY_X,20); textcolor(barvapocitac1); cprintf("barva #%d ",barvapocitac1);
    gotoxy(HODNOTY_X,21); textcolor(barvapocitac2); cprintf("barva #%d ",barvapocitac2);
   }

  ZACATECNIK("Zde muzes editovat nastaveni programu. Pohybujes se mysi nebo klavesnici.",21);
  ZACATECNIK("Klavesnici pomoci sipek a Enterem, anebo rychle cervene vyznacenymi znaky.",22);
  ZACATECNIK("Nakonec nastaveni nezapomen ulozit! Provedes to mysi nebo klavesou 'U'.",23);

  ZHLTNI_MYS();

CEKEJKLAVESU:
  INTRO_NAPIS(1,sipkapozice,"\xcd\xcd\xcd\x10 ",11); cassipka=CAS18(); typsipky=0;
  pozicetextefekt=0; castext=cassipka;
CEKEJKLAVESU2:
  cassetric=time(NULL);
  if (opakovani == 2) { opakovani=1; goto POZICE; }
  if (mys == 1) {
    ZHLTNI_MYS();
    do {
      if (pocetsetricvterin != 0 && (cassetric+pocetsetricvterin) < time(NULL)) {
SETRIC: SETRIC_OBRAZOVKY(); opakovani=2; goto START0;
       }
      _AX = 3; geninterrupt(0x33); pozx=_CX>>3; pozy=_DX>>3;
      if (pozx < 1) pozx=1; if (pozy < 1) pozy=1;
      if (pozx != puvpozx || pozy != puvpozy) {		//zmena souradnic
        gotoxy(puvpozx,puvpozy+2);
        _AH = 8; _BH = 0; geninterrupt(0x10); pomoc1=_AH; pomoc=_AL;//cti znak
        if ((pomoc1>>4) > 1 && (pomoc1>>4) != 4) {
			//jestlize pozadi neni < 2 nebo 3 (kazi se u barev)
          _BL = pomoc1-32; _AL = pomoc; _AH = 9; _CX = 1; _BH = 0;
          geninterrupt(0x10);					//smaz znak
         }

POZICE:
        gotoxy(pozx,pozy+2);
        _AH = 8; _BH = 0; geninterrupt(0x10); pomoc1=_AH; pomoc=_AL;
        _BL = pomoc1+32; _AL = pomoc; _AH = 9; _CX = 1; _BH = 0; geninterrupt(0x10);
        puvpozx=pozx; puvpozy=pozy;

        cassetric=time(NULL);
       }
      CAS(0); gotoxy(1,22); KONTROLA_UTNUTI();

      if (cassipka+5 < CAS18()) {
        cassipka=CAS18(); switch(typsipky) {
          case 0: typsipky++; INTRO_NAPIS(1,sipkapozice,"\xcd\xcd\xcd\xcd\x10",11); break;
          case 1: typsipky--; INTRO_NAPIS(1,sipkapozice,"\xcd\xcd\xcd\x10 ",11); break;
         }
       }
      if (castext != CAS18() && textefekty == 1) {
        gotoxy(pozicetextefekt+ZACATEK_TEXTU_X,sipkapozice);
        znak=CTIZNAK(); NAPISZNAK(znak,CTIBARVU()^16);
        if (znak != ':') pozicetextefekt++; else pozicetextefekt=0;
        castext=CAS18();
       }

      if (SETRIC_KLAVESA() == 1) cassetric=time(NULL);

      _AX = 3; geninterrupt(0x33); stavtlacitek = _BL;
     } while ((stavtlacitek&3) == 0 && kbhit() == 0);
   }

  else while (kbhit() == 0) { KONTROLA_UTNUTI(); CAS(0); }//mys neexistuje

  if (kbhit() != 0) {				//stisknuta klavesa
    pomoc=getch(); if (zvuk == 1) { sound(300); delay(5); nosound(); }
    switch(pomoc) {
      case 0: if (kbhit() == 0) break;
       switch(getch()) {
        case 59: HELPF1(15); opakovani=0; goto START0;		//F1
        case 62: goto TAB;				//F4
        case 64: goto SETRIC;				//F6
        case 68: goto KONEC;				//F10
        case 71: if (strana > 1) {			//Home
                   strana=1; sipkapozice=ZACATEK_OBR1; goto START;
                  }
                 else if (sipkapozice > ZACATEK_OBR1) {
                   INTRO_NAPIS(1,sipkapozice,"     ",11);
                   SMAZ_EFEKT(sipkapozice);
                   sipkapozice=ZACATEK_OBR1; goto CEKEJKLAVESU;
                  }
	case 72: if (strana == 2 && sipkapozice == ZACATEK_OBR2) { strana=1; sipkapozice=21; goto START; }
                 if (sipkapozice > ZACATEK_OBR1) {	//nahoru
                   INTRO_NAPIS(1,sipkapozice,"     ",11);
                   SMAZ_EFEKT(sipkapozice);
		   sipkapozice--; goto CEKEJKLAVESU;
		  } break;
        case 73: if (sipkapozice > CELK_ZACATEK_OBR) {	//PageUp
		   INTRO_NAPIS(1,sipkapozice,"     ",11);
                   SMAZ_EFEKT(sipkapozice);
                   sipkapozice=CELK_ZACATEK_OBR; goto CEKEJKLAVESU;
                  }
                 else if (strana > 1) {
                   strana--; sipkapozice=21;
                   goto START;
                  }
                 break;
        case 79: if (strana < 2) {			//End
                   strana=2; sipkapozice=21; goto START;
                  }
                 else if (sipkapozice < 21) {
                   INTRO_NAPIS(1,sipkapozice,"     ",11);
                   SMAZ_EFEKT(sipkapozice);
                   sipkapozice=21; goto CEKEJKLAVESU;
                  }
	case 80: if (strana == 1 && sipkapozice == 21) { strana=2; sipkapozice=ZACATEK_OBR2; goto START; }
                 if (sipkapozice < 21) {		//dolu
		   INTRO_NAPIS(1,sipkapozice,"     ",11);
                   SMAZ_EFEKT(sipkapozice);
		   sipkapozice++; goto CEKEJKLAVESU;
                  } break;
        case 81: if (sipkapozice < 21) {	//PageDown
		   INTRO_NAPIS(1,sipkapozice,"     ",11);
                   SMAZ_EFEKT(sipkapozice);
                   sipkapozice=21; goto CEKEJKLAVESU;
                  }
                 else if (strana < 2) {
                   strana++; sipkapozice=ZACATEK_OBR2;
                   goto START;
                  }
                 break;
        case 92: goto ULOZ;				//Shift-F9
       } break;
      case '\r': case ' ': pozicemyssipka=sipkapozice; goto PTEJSENAPOZICI;
      case '\t': TAB: if (strana == 1) { strana=2; sipkapozice=ZACATEK_OBR2; }
                      else { strana=1; sipkapozice=ZACATEK_OBR1; } goto START;
      default: switch(pomoc&0x5F) {
        case 'P': S1 zenskyrod=MUZ_ZENA(zenskyrod,6); ZAPIS_NALADU(); break; }
                  S2 psani=ANO_NE(psani,9); break; }
        case 'O': S1 delayprocenta=NAPIS_CISLO(delayprocenta,3,0,999,17); break; }
                  S2 vkolikhodin=NAPIS_CISLO(vkolikhodin,2,0,23,14); break; }
        case 'E': S1 pocetpocitacu=ANO_NE(pocetpocitacu-1,12)+1; break; }
                  S2 akcectrlbreak=ANO_NE_RESET(akcectrlbreak,18); break; }
        case 'F': S1 textefekty=ANO_NE(textefekty,20); if (textefekty == 0) { SMAZ_EFEKT(sipkapozice); pozicetextefekt=0; } } break;
        case 'M': S1 pocetradku=NAPIS_CISLO(pocetradku,2,0,99,19); break; }
                  S2 vyska=NAPIS_CISLO(vyska,4,0,9999,11); break; }
        case 'D': S1 nalada=NASTAV_NALADU(nalada,8,1); ZAPIS_NALADU(); break; }
                  S2 extrasouborpsani=ANO_NE(extrasouborpsani,10); break; }
        case 'J': S1 nastvani=ANO_NE(nastvani,13); } break;
        case 'T': S1 pocetsetricvterin=NAPIS_CISLO(pocetsetricvterin,4,0,9999,21); break; }
                  S2 pravopis=ANO_NE(pravopis,8); break; }
        case 'A': S1 zvuk=ANO_NE(zvuk,14); break; }
                  S2 slovenstina=ANO_NE(slovenstina,7); break; }
        case 'R': S1 NAPIS_TEXT(jmenomuzpocitac,10); break; }
                  S2 spocasi=ANO_NE_NAHODNE(spocasi,13); break; }
        case 'C': S1 pocetvterin=NAPIS_CISLO(pocetvterin,2,0,99,15); break; }
                  S2 cekaniklavesy=ANO_NE(cekaniklavesy,16); break; }
        case 'I': S1 font=ANO_NE(font,18); NASTAVSPRAVNYFONT(); break; }
                  S2 svtipy=ANO_NE_NAHODNE(svtipy,12); break; }
        case 'V': S1 zvysovani=ANO_NE(zvysovani,16); break; }
                  S2 barvapocitac1=ZVOL_BARVU(barvapocitac1,20); break; }
        case 'H': S1 charakter++; if (charakter > 6) charakter=0; ZAPIS_CHARAKTER(9); ZAPIS_NALADU(); break; }
                  S2 yz=ANO_NE(yz,15); break; }
        case 'L': S1 zenapocitac=MUZ_ZENA(zenapocitac,7); ZAPIS_NALADU(); break; }
                  S2 ulozeninakonci=ANO_NE(ulozeninakonci,17); break; }
        case 'B': S2 barvaclovek=ZVOL_BARVU(barvaclovek,19); } break;
        case 'X': S2 barvapocitac2=ZVOL_BARVU(barvapocitac2,21); } break;
        case 'Z': case 'Y': S1 NAPIS_TEXT(jmenozenapocitac,11); break; }
                  S2 mod=CISLO_MODU(mod,6); break; }
        case 'N': NASTAVENI(); NASTAVSPRAVNYFONT(); goto PSANI;
        case 'U': ULOZ: NASTAVSOUBOR(1); break;
        case 'S': goto STANDARD;
        case 'K': case 0x1B: goto KONEC;
       }
     }
    goto CEKEJKLAVESU2;
   }

  else if ((stavtlacitek&1) != 0) {		//leve tlacitko mysi
    if (zvuk == 1) { sound(500); delay(5); nosound(); }
    pozicemyssipka=pozy+2;
PTEJSENAPOZICI:
    switch(pozicemyssipka) {
      case 5: S2 strana=1; sipkapozice=ZACATEK_OBR1; goto START; } break;
      case 6: S1 zenskyrod=MUZ_ZENA(zenskyrod,6); ZAPIS_NALADU(); break; }
              S2 mod=CISLO_MODU(mod,6); break; }
      case 7: S1 zenapocitac=MUZ_ZENA(zenapocitac,7); ZAPIS_NALADU(); break; }
              S2 slovenstina=ANO_NE(slovenstina,7); break; }
      case 8: S1 nalada=NASTAV_NALADU(nalada,8,1); ZAPIS_NALADU(); break; }
              S2 pravopis=ANO_NE(pravopis,8); break; }
      case 9: S1 charakter++; if (charakter > 6) charakter=0; ZAPIS_CHARAKTER(9); ZAPIS_NALADU(); break; }
              S2 psani=ANO_NE(psani,9); break; }
      case 10: S1 NAPIS_TEXT(jmenomuzpocitac,10); break; }
               S2 extrasouborpsani=ANO_NE(extrasouborpsani,10); break; }
      case 11: S1 NAPIS_TEXT(jmenozenapocitac,11); break; }
               S2 vyska=NAPIS_CISLO(vyska,4,0,9999,11); break; }
      case 12: S1 pocetpocitacu=ANO_NE(pocetpocitacu-1,12)+1; break; }
               S2 svtipy=ANO_NE_NAHODNE(svtipy,12); break; }
      case 13: S1 nastvani=ANO_NE(nastvani,13); break; }
               S2 spocasi=ANO_NE_NAHODNE(spocasi,13); break; }
      case 14: S1 zvuk=ANO_NE(zvuk,14); break; }
               S2 vkolikhodin=NAPIS_CISLO(vkolikhodin,2,0,23,14); break; }
      case 15: S1 pocetvterin=NAPIS_CISLO(pocetvterin,2,0,99,15); break; }
               S2 yz=ANO_NE(yz,15); break; }
      case 16: S1 zvysovani=ANO_NE(zvysovani,16); break; }
               S2 cekaniklavesy=ANO_NE(cekaniklavesy,16); break; }
      case 17: S1 delayprocenta=NAPIS_CISLO(delayprocenta,3,0,999,17); break; }
               S2 ulozeninakonci=ANO_NE(ulozeninakonci,17); break; }
      case 18: S1 font=ANO_NE(font,18); NASTAVSPRAVNYFONT(); break; }
               S2 akcectrlbreak=ANO_NE_RESET(akcectrlbreak,18); break; }
      case 19: S1 pocetradku=NAPIS_CISLO(pocetradku,2,1,99,19); break; }
               S2 barvaclovek=ZVOL_BARVU(barvaclovek,19); break; }
      case 20: S1 textefekty=ANO_NE(textefekty,20); if (textefekty == 0) { SMAZ_EFEKT(sipkapozice); pozicetextefekt=0; } break; }
               S2 barvapocitac1=ZVOL_BARVU(barvapocitac1,20); break; }
      case 21: S1 pocetsetricvterin=NAPIS_CISLO(pocetsetricvterin,4,0,9999,21); break; }
               S2 barvapocitac2=ZVOL_BARVU(barvapocitac2,21); break; }
      case 22: S1 strana=2; sipkapozice=ZACATEK_OBR2; goto START; } break;
      case 24: if (pozx < 15) { NASTAVSOUBOR(1); break; }
               else if (pozx < 40) { NASTAVENI(); NASTAVSPRAVNYFONT();
                                     ZHLTNI_MYS(); goto PSANI; }
               else if (pozx < 70) goto STANDARD;
               else goto KONEC;
     }
    goto CEKEJKLAVESU2;
   }
  else if ((stavtlacitek&2) != 0) {		//prave tlacitko mysi
    if (strana == 1) {
      strana=2; sipkapozice=ZACATEK_OBR2;
     }
    else {
      strana=1; sipkapozice=ZACATEK_OBR1;
     }
    goto START;
   }
    
  KONEC:
  if (psanipredtim != 1 && psani != 0) { HLASKA("Psat do souboru budu az pri pristim spusteni. Ted ne.",1); psani=2; }
  if (psanipredtim == 1 && psani == 0) {
    if (extrasouborpsani == 1) {		//smazat vznikly soubor
      SOUBOR("KYDY\\"); strcat(soubor,kydaniny);
      fclose(kydy); kydy=NULL; remove(soubor);
     }
    else {
      ZAVRISOUBORSKYDAMA("\nPsani do souboru bylo v nastaveni vypnuto.",0); fclose(kydy); kydy=NULL;
     }
   }
  grafika25=0; NASTAVSPRAVNYMOD();
  jeli_nastaveni=1;					//Prostredi neni
  gotoxy(kdejex,kdejey);
#endif

 }
/* Rutina ZVYRAZNOVAC - viz implementace a nazvy promennych (konvence Pokyd). */
void ZVYRAZNOVAC(BYTE x,BYTE y,BYTE *text,BYTE kolikate) {
BYTE pozice=0;
  gotoxy(x,y);
  kolikate--;
  while (pozice < strlen(text)) {
    if (kolikate == pozice) NAPISZNAK2(text[pozice++],12);
    else NAPISZNAK2(text[pozice++],14);
   }
 }

/* Rutina ZVYRAZNOVAC4 - viz implementace a nazvy promennych (konvence Pokyd). */
void ZVYRAZNOVAC4(BYTE y,BYTE *text,BYTE kolikate) {
  ZVYRAZNOVAC(ZACATEK_TEXTU_X,y,text,kolikate);
 }

/* Rutina MUZ_ZENA - viz implementace a nazvy promennych (konvence Pokyd). */
BYTE MUZ_ZENA(BYTE promenna,BYTE kam) {
  if (promenna > 0) promenna=0;
  else promenna=1;
  INTRO_NAPIS(HODNOTY_X,kam,promenna == 0 ? "muz " : "zena",15);
  return(promenna);
 }

/* Rutina VRAT_NAZEV_NALADY - viz implementace a nazvy promennych (konvence Pokyd). */
void VRAT_NAZEV_NALADY(BYTE *retezec) {
  switch(nalada) {
    case 0: strcpy(retezec,"vyborna "); break;
    case 1: strcpy(retezec,"dobra   "); break;
    case 2: strcpy(retezec,"normalni"); break;
    case 3: strcpy(retezec,"spatna  "); break;
    case 4: strcpy(retezec,"hrozna  "); break;
    default: strcpy(retezec,"normalni"); break;
   }
 }

/* Rutina NASTAV_NALADU - viz implementace a nazvy promennych (konvence Pokyd). */
BYTE NASTAV_NALADU(BYTE promenna,BYTE kam,BYTE mali_se_menit) {
BYTE nal[9],puvnalada=nalada;
  if (mali_se_menit == 1) {
    if (promenna < 4) promenna++;
    else promenna=0;
    naladabody=promenna*15+7;
   }
  nalada=promenna; VRAT_NAZEV_NALADY(nal);
  nalada=puvnalada;
  INTRO_NAPIS(HODNOTY_X,kam,nal,15);
  ZAPIS_NALADU();
  return(promenna);
 }

/* Rutina VRAT_NAZEV_CHARAKTERU - viz implementace a nazvy promennych (konvence Pokyd). */
void VRAT_NAZEV_CHARAKTERU(BYTE *retezec) {
  switch(charakter) {
    case 0: strcpy(retezec,"stroj     "); break;
    case 1: strcpy(retezec,"naivni    "); break;
    case 2: strcpy(retezec,"klidny    "); break;
    case 3: strcpy(retezec,"prumerny  "); break;
    case 4: strcpy(retezec,"neduverivy"); break;
    case 5: strcpy(retezec,"naladovy  "); break;
    case 6: strcpy(retezec,"vybusny   "); break;
    default: strcpy(retezec,"prumerny  "); break;
   }
 }

/* Rutina ZAPIS_CHARAKTER - viz implementace a nazvy promennych (konvence Pokyd). */
void ZAPIS_CHARAKTER(BYTE kam) {
BYTE charak[11];
  VRAT_NAZEV_CHARAKTERU(charak);
  INTRO_NAPIS(HODNOTY_X,kam,charak,15);
 }

/* Rutina ANO_NE - viz implementace a nazvy promennych (konvence Pokyd). */
BYTE ANO_NE(BYTE promenna,BYTE kam) {
  if (promenna > 0) promenna=0;
  else promenna=1;
  INTRO_NAPIS(HODNOTY_X,kam,promenna == 1 ? "ano" : "ne ",15);
  return(promenna);
 }

/* Rutina ANO_NE_RESET - viz implementace a nazvy promennych (konvence Pokyd). */
BYTE ANO_NE_RESET(BYTE promenna,BYTE kam) {
  promenna++; if (promenna > 2) promenna=0;
  INTRO_NAPIS(HODNOTY_X,kam,promenna == 0 ? "ne   " : promenna == 1 ? "ano  " : "reset",15);
  return(promenna);
 }

/* Rutina ANO_NE_NAHODNE - viz implementace a nazvy promennych (konvence Pokyd). */
BYTE ANO_NE_NAHODNE(BYTE promenna,BYTE kam) {
  switch(promenna) {
    case 0: promenna=2; break;				//ne - nahodne
    case 1: promenna=0; break;				//ano - ne
    case 2: promenna=1;					//nahodne - ano
   }
  INTRO_NAPIS(HODNOTY_X,kam,promenna == 0 ? "ne " : promenna == 1 ? "ano    " : "nahodne",15);
  return(promenna);
 }

/* Rutina CISLO_MODU - viz implementace a nazvy promennych (konvence Pokyd). */
BYTE CISLO_MODU(BYTE promenna,BYTE kam) {
  if (promenna == 25) promenna=50;
   else if (promenna == 50) promenna=0;
    else promenna=25;
  INTRO_NAPIS(HODNOTY_X,kam,promenna == 25 ? "25     " : promenna == 50 ? "50     " : "puvodni",15);
  return(promenna);
 }

/* Rutina ZVOL_BARVU - viz implementace a nazvy promennych (konvence Pokyd). */
BYTE ZVOL_BARVU(BYTE barva,BYTE kam) {
BYTE barv[15];
  if (barva < 15) barva++;
  else barva=1;
  sprintf(barv,"barva #%d ",barva);
  INTRO_NAPIS(HODNOTY_X,kam,barv,barva);
  return(barva);
 }

/* Rutina NAPIS_CISLO - viz implementace a nazvy promennych (konvence Pokyd). */
DWORD NAPIS_CISLO(DWORD hodnota,DWORD jakdlouhe,DWORD min,DWORD max,BYTE kam) {
BYTE pozice=0,cislo[5];
DWORD vyslednecislo=0;
  gotoxy(HODNOTY_X,kam); textcolor(15);
  while (pozice++ < jakdlouhe) NAPISZNAK2(' ',15);		//vymazani
  gotoxy(HODNOTY_X,wherey());			//navrat na zacatek
  pozice=0; NASTAVKURZOR();
KLAVESA:
  while (kbhit() == 0) { KONTROLA_UTNUTI(); CAS(0); }	//Ctrl-Break a cas
  pomoc=CTIKLAVESU();
  if (pomoc > 47 && pomoc < 58 && pozice < jakdlouhe) {
    NAPISZNAK2(pomoc,15); cislo[pozice++]=pomoc;
   }
  else if (pomoc == 8 && pozice > 0) {				//Backspace
    textattr(15); cprintf("\b \b"); cislo[--pozice]=0;
   }
  else if (pomoc == '\r') {					//Enter
    SMAZKURZOR(); goto KONEC;
   }
  else if (pomoc == 0x1B) {					//Esc
    SMAZKURZOR(); vyslednecislo=hodnota; goto KONEC2;
   }
  if (zvuk == 1) { sound(700); delay(30); nosound(); }
  goto KLAVESA;
  KONEC:
  for (jakdlouhe=0; jakdlouhe < pozice; jakdlouhe++) {
    vyslednecislo*=10; vyslednecislo+=cislo[jakdlouhe]-'0';
   }
  if (vyslednecislo < min || vyslednecislo > max) vyslednecislo=hodnota;
  KONEC2:
  gotoxy(wherex()-pozice,wherey()); textcolor(15);
  cprintf("%lu    ",vyslednecislo); return(vyslednecislo);
 }

/* Rutina NAPIS_TEXT - viz implementace a nazvy promennych (konvence Pokyd). */
void NAPIS_TEXT(BYTE *jaky,BYTE kam) {
BYTE pozice=0,text[16];
  gotoxy(HODNOTY_X,kam); textcolor(15);
  while (pozice++ < 16) NAPISZNAK2(' ',15);		//vymazani
  gotoxy(HODNOTY_X,wherey());			//navrat na zacatek
  pozice=0; NASTAVKURZOR();
KLAVESA:
  while (kbhit() == 0) { KONTROLA_UTNUTI(); CAS(0); }	//Ctrl-Break a cas
  pomoc=CTIKLAVESU();
  if (pomoc >= ' ' && pomoc < 128 && pozice < 15) {
    NAPISZNAK2(pomoc,15); text[pozice++]=pomoc;
   }
  else if (pomoc == 8 && pozice > 0) {				//Backspace
    textattr(15); cprintf("\b \b"); text[--pozice]=0;
   }
  else if (pomoc == '\r') {					//Enter
    SMAZKURZOR(); text[pozice]=0; goto KONEC;
   }
  else if (pomoc == 0x1B) {					//Esc
    SMAZKURZOR(); goto KONEC2;
   }
  if (zvuk == 1) { sound(600); delay(30); nosound(); }
  goto KLAVESA;
  KONEC:
  if (pozice > 0) strcpy(jaky,text);
  KONEC2:
  gotoxy(HODNOTY_X,wherey()); pozice=0; while (pozice++ < 16) NAPISZNAK2(' ',15);
  gotoxy(HODNOTY_X,wherey()); textcolor(15); cprintf(jaky);
 }

/* Rutina NASTAV_STANDARD - viz implementace a nazvy promennych (konvence Pokyd). */
void NASTAV_STANDARD(void) {
  zenskyrod=0; zenapocitac=0; nalada=2; naladabody=37; charakter=3;
  strcpy(jmenomuzpocitac,"Klaban"); strcpy(jmenozenapocitac,"Daria");
  nastvani=1; zvuk=1; yz=0; pravopis=1;
  pocetvterin=5; zvysovani=1; delayprocenta=100; font=1; pocetradku=2;
  textefekty=1; pocetsetricvterin=180; mod=25; slovenstina=0; psani=1;
  extrasouborpsani=0; vyska=300; svtipy=2; spocasi=0;
  vkolikhodin=16; cekaniklavesy=0; ulozeninakonci=1; akcectrlbreak=0;
  barvaclovek=14; barvapocitac1=10; barvapocitac2=11;
 }

/* Rutina SMAZ_EFEKT - viz implementace a nazvy promennych (konvence Pokyd). */
void SMAZ_EFEKT(BYTE kde) {
BYTE text[(HODNOTY_X-ZACATEK_TEXTU_X+1)*2],pozice;
  gettext(ZACATEK_TEXTU_X,kde,HODNOTY_X,kde,text);
  for (pozice=1; pozice < 111; pozice+=2)
   if ((text[pozice]&16) == 16)
    text[pozice]^=16;
  puttext(ZACATEK_TEXTU_X,kde,HODNOTY_X,kde,text);
 }
