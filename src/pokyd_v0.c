/* Tento zdrojový kód je pod licencí GNU/GPL. Mùžete ho použít k vlastní
   potøebì, ale nesmíte jej ani programy založené na tomto kódu využít komerènì!

   Jedná se o zdrojový kód programu Pokyd (http://iqpokyd.kyblsoft.cz)
   od Aleše Jandy, aktivnì vyvíjeného 1999 - 2002
*/


BYTE VETA_PREDTIM(WORD cislovety) {
BYTE pozice;
  for (pozice=0; pozice < 50; pozice++)
   if (cislovety == puvodp[pozice]) return(pozice);
  return(50);
 }

void VYNULUJ_ODPOVEDI(void) {
  pozodp=0; skutecnychodp=0; pocetrealtimekecu=0;
 }

void EXTRA_VETA(BYTE ktera) {
  intpozice=ktera-1;
  DBGLOGF("EXTRA_VETA(%u): intpozice=%u iq0='%c' pozodp=%u",
          (unsigned)ktera, (unsigned)intpozice,
          inteligence[intpozice] != NULL ? inteligence[intpozice][0] : '?',
          (unsigned)pozodp);
  switch(inteligence[intpozice][0]) {
    case '5': RODX(1); break;
    case '6': NAHODAX(1); break;
    default:
      DBGLOGF("EXTRA_VETA: unexpected iq[0] (not 5/6), skipping RODX/NAHODAX");
      break;
   }
 }

void INTRO_POKYDPRUH(void) {
#define ZACATEKX 5
#define ZACATEKY 2
#define KONECX 41
#define KONECY 9

static BYTE pozicepruhu=ZACATEKX-1;
BYTE obraz[20],pozice;
  if (pozicepruhu > KONECX || pozicepruhu < ZACATEKX-1) {
    if (textefekty == 1) pozicepruhu++;
    return;
   }
  gettext(pozicepruhu,ZACATEKY,pozicepruhu,KONECY,obraz);
  for (pozice=0; pozice < 20; pozice+=2) {
    if (obraz[pozice] == 219 && obraz[pozice+1] == 12) obraz[pozice+1]=15;
   } puttext(pozicepruhu,ZACATEKY,pozicepruhu,KONECY,obraz);
  pozicepruhu++; gettext(pozicepruhu,ZACATEKY,pozicepruhu,KONECY,obraz);
  for (pozice=0; pozice < 20; pozice+=2) {
    if (obraz[pozice] == 219 && obraz[pozice+1] == 15) obraz[pozice+1]=12;
   } puttext(pozicepruhu,ZACATEKY,pozicepruhu,KONECY,obraz);
 }

void NASTARTUJ_PROGRAM(void) {
  DBGLOG("NASTARTUJ: begin");
  nosound();
  DBGLOG("NASTARTUJ: after nosound");
  SMAZKURZOR();
  DBGLOG("NASTARTUJ: after SMAZKURZOR");
  delkastrany=TESTSTRANY();
  if (pokyd_shell_rows != 0) {
    if (delkastrany > 24) pokyd_shell_rows = 0;
    else {
      int avail = 25 - (int)pokyd_shell_rows;
      if (avail < 10) pokyd_shell_rows = 0;
      else {
        signed char nd = (signed char)(avail - 1);
        if (delkastrany > nd) delkastrany = nd;
      }
     }
   }
  DBGLOG("NASTARTUJ: after TESTSTRANY");
  srand(nahodacislo=time(NULL));
  DBGLOG("NASTARTUJ: after srand");
  ZAPNI_REZIDENTY();
  DBGLOG("NASTARTUJ: after ZAPNI_REZIDENTY");
  PREPNINACELOUOBRAZOVKU();
  DBGLOG("NASTARTUJ: after PREPNINACELOUOBRAZOVKU");
  kydy=NULL; nastaveni=NULL; vtipys=NULL; intel=NULL; vety=NULL;
  DBGLOG("NASTARTUJ: end");
 }

WORD VRAT_POZICI_VTIPU(void) {
WORD pozice;
BYTE kolik,max;
  if (celkemvtipu < 200) max=celkemvtipu; else max=200;
  do {
    pozice=rand()%celkemvtipu;
    for (kolik=0; kolik < 200; kolik++)
     if (pozice == puvvtipy[kolik]) break;
   } while (kolik < max-1);

  for (kolik=200; kolik > 0; kolik--)
   puvvtipy[kolik]=puvvtipy[kolik-1];
  puvvtipy[0]=pozice;
  return(pozice);
 }

void CTI_INFORMACE_O_VETACH(void) {
BYTE pozice,pozice2,verze;
WORD hodnota;
  SOUBOR("PUVODNI.PKD");
  if ((vety=fopen(soubor,"rb")) == NULL) goto VETY2;
  fseek(vety,5,SEEK_SET);

  verze=getc(vety);
  fseek(vety,2,SEEK_CUR);		//preskoc 2 cisla verze Pokydu
  if (verze != 3) {			//verze: 3
    fclose(vety); vety=NULL; goto VETY2;
   }
  _CH=getc(vety); _CL=getc(vety); puvodnipkdrok=_CX;
  puvodnipkdmesic=getc(vety); puvodnipkdden=getc(vety);
  puvodnipkdhodina=getc(vety); puvodnipkdminuta=getc(vety);
  pocetsouboru=getc(vety); pocetsouboru<<=8; pocetsouboru+=getc(vety);
  _AH = 0x2A; geninterrupt(0x21);
  pozice=_DH; pozice2=_DL;
  if (pozice != puvodnipkdmesic || pozice2 != puvodnipkdden) pocetsouboru=0;
  verze=getc(vety);			//doted pouzivana verze slovniku
  VRATDATA(19);			//verze aktualniho slovniku
  pozice=getc(vety); pozice2=getc(vety);	//den, mesic vydani slovniku
  if (verze != dlouhe[0]) {
    sprintf(dlouhe,"ZMENEN SLOVNIK: puvodni verze: %d (%d.%d.%u), nynejsi verze: %d (%d.%d.%u)",verze,pozice,pozice2,(WORD)getc(vety)+2000,dlouhe[0],dlouhe[1],dlouhe[2],(WORD)dlouhe[3]+2000);
    HLASKA(dlouhe,1);
   }
  else _AL=getc(vety);		//preskoc 1 cislo (rok data slovniku)
    
  for (pozice=0; pozice < 50; pozice++) {
    hodnota=0;
    for (pozice2=0; pozice2 < 2; pozice2++) {
      hodnota<<=8; _CH=getc(vety); hodnota+=_CH;
     }
    puvodp[pozice]=hodnota;
   }
  for (pozice=0; pozice < 200; pozice++) {
    hodnota=0;
    for (pozice2=0; pozice2 < 2; pozice2++) {
      hodnota<<=8; _CH=getc(vety); hodnota+=_CH;
     }
    puvvtipy[pozice]=hodnota;
   }
  fclose(vety); vety=NULL;

VETY2:
  SOUBOR("PROFIL.PKD");
  if ((vety=fopen(soubor,"rb")) == NULL) goto KONEC;
  fseek(vety,5,SEEK_SET);

  verze=getc(vety);
  fseek(vety,2,SEEK_CUR);		//preskoc 2 cisla verze Pokydu
  if (verze != 1) {			//verze: 1
    fclose(vety);
    HLASKA("Soubor s tvym profilem pochazi z jine verze Pokydu, takze vytvorim novy.",1);
    goto KONEC;
   }
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

  KONEC: fclose(vety); vety=NULL;
 }

void ZAPIS_INFORMACE_O_VETACH(BYTE co,BYTE nastaveni) {
BYTE pozice;
WORD pracovni;

  if (readonlymod == 1) return;

  if ((co&1) != 1) goto VETY2;

  SOUBOR("PUVODNI.PKD");
  if ((vety=fopen(soubor,"wb")) == NULL) goto VETY2;

  fwrite("Pokyd\x3\x7\x0",8,1,vety);			//verze 3, Pokyd 7.0

  _AH = 0x2A; geninterrupt(0x21);
  puvodnipkdmesic = _DH; puvodnipkdden = _DL;
  putc(_CH,vety); putc(_CL,vety);		//rok
  putc(puvodnipkdmesic,vety); putc(puvodnipkdden,vety);
  _AH = 0x2C; geninterrupt(0x21);
  puvodnipkdhodina = _CH; puvodnipkdminuta = _CL;
  putc(puvodnipkdhodina,vety); putc(puvodnipkdminuta,vety);

  _CX = pocetsouboru+1; putc(_CH,vety); putc(_CL,vety);

  VRATDATA(19); fwrite(dlouhe,4,1,vety);

  for (pozice=0; pozice < 50; pozice++) {
    _CX=puvodp[pozice];
    putc(_CH,vety);
    putc(_CL,vety);
   }
  for (pozice=0; pozice < 200; pozice++) {
    pracovni=puvvtipy[pozice];
    putc(pracovni>>8,vety);
    putc((BYTE)pracovni,vety);
   }
  fclose(vety); vety=NULL;
  puvodnipkdzapsani=1;

VETY2:

  if ((co&2) != 2) goto DAL;
  SOUBOR("PROFIL.PKD");
OTEVRENI:
  if ((vety=fopen(soubor,"wb")) == NULL) {
    if (ctrlbreak == 0) {			//zacyklil by se
      HLASKA("Nepodarilo se mi zapsat soubor PROFIL.PKD! Tvuj profil nebude ulozen!",4);
OTAZKA:
      switch(HLASKA("Mam se pokusit soubor znovu zapsat? [A/N] A",4)) {
        case 'a': case 'A': case 'y': case 'Y': case '\r': goto OTEVRENI;
        case 'n': case 'N': case 0x1B: break;
        default: goto OTAZKA;
       }
     }
    goto DAL;
   }
  fwrite("Pokyd\x1\x7\x0",8,1,vety);			//verze 3, Pokyd 7.0
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

DAL:
  if (nastaveni == 1 && ulozeninakonci == 1) NASTAVSOUBOR(0);
 }

void ZAVRISOUBORSKYDAMA(BYTE *text,BYTE naobrazovku) {
  if (kydy != NULL) fprintf(kydy,text);
  ZAPISDELKUHOVORU(naobrazovku);
 }

BYTE ZJISTI_SVATEK(void) {
  _AH = 0x2A; geninterrupt(0x21);
  if (_AL == 0 || strchr(svatek,'#') != NULL) return(1);	//je svatek
  else return(0);					//neni svatek
 }

void PREPNINACELOUOBRAZOVKU(void) {
  /* Keep startup safe under DOSBox + Watcom: avoid legacy screen probes
     and avoid CRT text functions that can block in this environment. */
  pozicehlavicka=1;
 }

void interrupt INT24(void) {
  chybaint24=1;
 }

BYTE GENERUJ_ODPOVED(DWORD odkud) {			//vyber ze 3 odpovedi
WORD pozice1,pozice2,pozice3,misto1,misto2,misto3;
WORD nejlepsi[3]={50,50,50},nejhodnota,nejpoz;
signed char nejlepsipoz;
  misto1=NAJDIDALSI(odkud,nalada);
  misto2=NAJDIDALSI(misto1,1);
  misto3=NAJDIDALSI(misto2,1);
  pozice1=nalada; pozice2=nalada+1; pozice3=nalada+2;
  for (nejlepsipoz=49; nejlepsipoz >= 0; nejlepsipoz--) {
    if (pozice1+intpozice*10 == puvodp[nejlepsipoz]) nejlepsi[0]=nejlepsipoz;
    if (pozice2+intpozice*10 == puvodp[nejlepsipoz]) nejlepsi[1]=nejlepsipoz;
    if (pozice3+intpozice*10 == puvodp[nejlepsipoz]) nejlepsi[2]=nejlepsipoz;
   }
  nejhodnota=0;
  for (nejlepsipoz=0; nejlepsipoz < 3; nejlepsipoz++)
   if (nejlepsi[nejlepsipoz] > nejhodnota)		//zjisteni nejlepsi
    nejhodnota=nejlepsi[nejlepsipoz];			//hodnoty
  nejpoz=0;
  for (nejlepsipoz=0; nejlepsipoz < 3; nejlepsipoz++)
   if (nejlepsi[nejlepsipoz] == nejhodnota)
    nejlepsi[nejpoz++]=nejlepsipoz;		//zjisteni vsech nejlepsich
  switch(nejlepsi[rand()%nejpoz]) {
    case 0: mistovint=misto1; return(pozice1);
    case 1: mistovint=misto2; return(pozice2);
    default: mistovint=misto3; return(pozice3);
   }
 }

BYTE VYBER_ODPOVED(void) {
BYTE mista[50],nejlepsi[50],pozice,pozice2;
WORD cislo;
  DBGLOGF("VYBER_ODPOVED: pozodp=%u skutecnychodp=%u", (unsigned)pozodp, (unsigned)skutecnychodp);
  for (pozice=0; pozice < 50; pozice++) mista[pozice]=0;	//vynulovani
  for (pozice=0; pozice < pozodp; pozice++) {
    cislo=cislaodp[pozice];
    for (pozice2=0; pozice2 < 50; pozice2++)
     if (cislo == puvodp[pozice2]) { mista[pozice]=pozice2; goto DAL; }
    mista[pozice]=50;					//nevyskytuje se
    DAL:;
   }
  cislo=0;
  for (pozice=0; pozice < pozodp; pozice++)
   if (mista[pozice] > cislo)
    cislo=mista[pozice];			//zjisteni nejvyssiho cisla
  pozice2=0;			//vybrani nahodneho z nejlepsich
  for (pozice=0; pozice < pozodp; pozice++)
   if (mista[pozice] == cislo)
    nejlepsi[pozice2++]=pozice;
  if (pozice2 == 0) {
    DBGLOG("VYBER_ODPOVED: no candidate (pozice2==0) returning 0");
    return(0);
   }
  cislo=nejlepsi[rand()%pozice2];
  DBGLOGF("VYBER_ODPOVED: picked slot=%u", (unsigned)cislo);
  return((BYTE)cislo);
 }

WORD ODRIZNIENTERY(void) {
WORD pozice1=0,pozice2=0,celkem=strlen(dlouhe);
  while (pozice1 < celkem) {
    if (dlouhe[pozice1] != '\r') {
      dlouhe[pozice2]=dlouhe[pozice1]; pozice2++;
     }
    pozice1++;
   }
  dlouhe[pozice2]=0;
  return(pozice2);
 }

void OREZ_MEZERY(BYTE *retezec) {
WORD pozice=strlen(retezec)-1;
  while (retezec[pozice] == ' ') {
    retezec[pozice]=0; pozice--;
   }
 }

void ZMENMOD(void) {
BYTE kdejex=wherex(),kdejey=wherey(),pocatek;
  if (delkastrany == 24) {			//zjisteni graficke karty VGA
    _AX = 0x1A00; geninterrupt(0x10); if (_AL != 0x1A) {
      HLASKA("Tato funkce neni podporovana kartami starsimi nez VGA. Je mi lito.",4);
      return;
     }
    ZACATECNIK("Nyni se Pokyd prepne na 50-ti radkovy mod.",51); delkastrany=49;
   }
  else {
    ZACATECNIK("Nyni se Pokyd prepne na 25-ti radkovy mod.",52);
    delkastrany=24; if (kdejey <= 24) pocatek=1;
    else {
      pocatek=kdejey-24;
      if (pozicehlavicka < pocatek) pozicehlavicka=1; else pozicehlavicka-=pocatek-1;
      pozicedatumcas=pozicehlavicka+1;
      if (stranaradek > 23) stranaradek=23;
     }
    gettext(1,pocatek,80,pocatek+24,obrazovka);
    textmode(C80);      //tady se to na LCD dlouze prepina :-( (pozn. 2005)
   }
  NASTAVSPRAVNYMOD();
  gotoxy(1,VRATPOZICI()-1);
  if (delkastrany == 24) {
    puttext(1,1,80,25,obrazovka); if (kdejey > 25) kdejey=25;
   }
  NAPISHLAVICKOVYRADEK(); ZAPISCAS();
KONEC:
  gotoxy(kdejex,kdejey);
 }

BYTE VRATDIAKRITIKU(BYTE pismeno) {
  if ((delkastrany == 24 || grafika25 == 1) && font == 1);
  else return(pismeno);					//bez diakritiky
  if (pismeno == 's') return('ç');
  else return('í');
 }

void NAPISHLAVICKOVYRADEK(void) {
BYTE kdejex=wherex(),kdejey=wherey(),nazev[15];
  gotoxy(1,pozicehlavicka);
  sprintf(nazev,"Ale%c",VRATDIAKRITIKU('s'));
  NAPISRETEZEC(nazev,11);
  NAPISRETEZEC(" Janda                        Pokyd v7.0                       ",11);
  sprintf(nazev,"K%cBLSoft 2005",VRATDIAKRITIKU('Y'));
  NAPISRETEZEC(nazev,11);
  gotoxy(kdejex,kdejey);
 }

DWORD VRAT_SERIOVE_CISLO(void) {
  _DS=FP_SEG(&data);
  _DX=FP_OFF(&data);
  _AX=0x6900;
  _BL=3;
  geninterrupt(0x21);
  return(data.seriove);
 }

void ZAPISPUVODP(DWORD pozice) {
BYTE poz=49;
  while (poz-- > 0) {
    puvodp[poz+1]=puvodp[poz];
   }
  puvodp[0]=pozice;
 }

void ODROLUJ(void) {
BYTE pozice,pozice2,pocetradku=24,celkem,typ,strzeno[80];
DWORD cas=0;
  DBGLOG("ODROLUJ: begin");
  SMAZKURZOR();
  if (textefekty == 0) { SMAZOBRAZOVKU(1); DBGLOG("ODROLUJ: SMAZOBRAZOVKU done"); return; }
  if (pokyd_shell_rows != 0) {
    clrscr();
    DBGLOG("ODROLUJ: consplit region clear only");
    return;
   }
  if (delkastrany == 49 && grafika25 == 0) pocetradku=49;

  switch(rand()%8) {
   case 0: case 1:			//rolovani vertikalne
    celkem=(pocetradku>>1)+1; typ=rand()%2;
    for (pozice=0; pozice < celkem; pozice++) {
      do KONTROLA_UTNUTI(); while (cas == CAS18());
      memset(&pokyd_regs, 0, sizeof(pokyd_regs));
      _AH = typ+6; _DH = celkem-1; _CX = 0; _DL = 79; _BH = 0; _AL = 1; geninterrupt(0x10);
      memset(&pokyd_regs, 0, sizeof(pokyd_regs));
      _AH = (typ^1)+6; _DH = pocetradku; _CH = celkem-1; _CL = 0; _DL = 79; _BH = 0; _AL = 1; geninterrupt(0x10);
      cas=CAS18();
     }
    break;
   case 2:				//zleva
    for (pozice=0; pozice < 80; pozice++) {
      memset(&pokyd_regs, 0, sizeof(pokyd_regs));
      _CX = pozice; _DL = pozice; _DH=pocetradku; _AX = 0x600; _BH = 0; geninterrupt(0x10);
      delay(6); KONTROLA_UTNUTI();
     }
    break;
   case 3:				//zprava
    for (pozice=80; pozice > 0; pozice--) {
      memset(&pokyd_regs, 0, sizeof(pokyd_regs));
      _CX = pozice-1; _DL = pozice-1; _DH=pocetradku; _AX = 0x600; _BH = 0; geninterrupt(0x10);
      delay(6); KONTROLA_UTNUTI();
     }
    break;
   case 4:				//horizontalni propletani
    pocetradku+=2;
    for (pozice=1; pozice < 81; pozice++) {
      for (celkem=1; celkem < pocetradku; celkem+=2)
       VYMAZZNAKNAOBR(pozice,celkem);
      for (celkem=2; celkem < pocetradku; celkem+=2)
       VYMAZZNAKNAOBR(81-pozice,celkem);
      delay(6); KONTROLA_UTNUTI();
     }
    break;
   case 5:				//vertikalni propletani
    for (celkem=1; celkem <= (pocetradku+1); celkem++) {
      do KONTROLA_UTNUTI(); while (cas == CAS18()); cas=CAS18();
      for (pozice=1; pozice < 81; pozice+=2)
       VYMAZZNAKNAOBR(pozice,celkem);
      for (pozice=2; pozice < 81; pozice+=2)
       VYMAZZNAKNAOBR(pozice,pocetradku-celkem+2);
     }
    break;
   case 6:				//strhavani zezhora
     for (pozice=0; pozice < 80; pozice++) strzeno[pozice]=1;

     for (pozice=0; pozice < ((pocetradku<<1)+4); pozice++) {
       for (pozice2=0; pozice2 < 40; pozice2++) {
NAHODA6: typ=rand()%80;
         if (strzeno[typ] > (pozice>>1) && (rand()%2) == 0) goto NAHODA6;

         if (strzeno[typ] == (pocetradku+2)) {	//smazani posledniho radku
           gotoxy(typ+1,pocetradku+1); NAPISZNAK(' ',7);
           goto DAL6;
          }
         if (strzeno[typ] == (pocetradku+3)) goto NAHODA6;

         gotoxy(typ+1,strzeno[typ]);
         NAPISZNAK(CTIZNAK(),(rand()%(128-16))+16);
         if (strzeno[typ] > 1) { gotoxy(typ+1,strzeno[typ]-1); NAPISZNAK(' ',7); }

DAL6:    strzeno[typ]++;
        }
       delay(20); KONTROLA_UTNUTI();
      }
    break;
   case 7:				//strhavani zezdola
     for (pozice=0; pozice < 80; pozice++) strzeno[pozice]=1;

     for (pozice=0; pozice < ((pocetradku<<1)+4); pozice++) {
       for (pozice2=0; pozice2 < 40; pozice2++) {
NAHODA7: typ=rand()%80;
         if (strzeno[typ] > (pozice>>1) && (rand()%2) == 0) goto NAHODA7;

         if (strzeno[typ] == (pocetradku+2)) {	//smazani posledniho radku
           gotoxy(typ+1,1); NAPISZNAK(' ',7);
           goto DAL7;
          }
         if (strzeno[typ] == (pocetradku+3)) goto NAHODA7;

         gotoxy(typ+1,pocetradku+2-strzeno[typ]);
         NAPISZNAK(CTIZNAK(),(rand()%(128-16))+16);
         if (strzeno[typ] > 1) { gotoxy(typ+1,pocetradku+2-strzeno[typ]+1); NAPISZNAK(' ',7); }

DAL7:    strzeno[typ]++;
        }
       delay(20); KONTROLA_UTNUTI();
      }
    break;
   }
  DBGLOG("ODROLUJ: end");
 }

BYTE CTIKLAVESU(void) {
BYTE klavesa;
  klavesa=getch();
  if (klavesa == 0) getch();
  if (yz == 1) {
    if ((klavesa&0x5F) == 'Y') klavesa++;
    else if ((klavesa&0x5F) == 'Z') klavesa--;
   }
  return(klavesa);
 }

void NASTAVKURZOR(void) {
  memset(&pokyd_regs, 0, sizeof(pokyd_regs));
  if (grafika25 == 0 && delkastrany == 49) _CX = 0x0709;
  else _CX = 0x0C0D;
  _AH = 1;
  _BH = 0;
  geninterrupt(0x10);		//nastav kurzor (BH=video page)
 }

void VRATDATA(BYTE poradidat) {
WORD odkaz=(poradidat<<1)+1;
WORD pozice=0;
  pozice=datasoubor[odkaz]; pozice<<=8;
  pozice+=datasoubor[odkaz+1];

  for (odkaz=0; datasoubor[pozice] > 0; odkaz++)
   dlouhe[odkaz]=datasoubor[pozice++];
  dlouhe[odkaz]=0;
 }

BYTE VRATPOZICI(void) {	//zjisti, je-li v nastaveni a vrati souradnici y
  if (jeli_nastaveni == 2) return(2);
  else return(pozicedatumcas);
 }

void ZAPISDELKUHOVORU(BYTE naobrazovku) {
BYTE text[60],nove[50];
DWORD sekundy=cashovoru;
  if (naobrazovku == 1) STRANA(1);
  sprintf(text,"Vet:  %lu   Doba hovoru:  ",pocetvet);
  if (sekundy >= 3600) {			//i s hodinami
    sprintf(nove,"%d:",sekundy/3600);
    strcat(text,nove);
    sekundy=sekundy%3600;
   }
  sprintf(nove,"%02d:",sekundy/60);
  strcat(text,nove);
  sprintf(nove,"%02d",sekundy%60);
  strcat(text,nove);
  if (cashovoru > 0) pocetvet=pocetvet*60/cashovoru; else pocetvet=0;
  sprintf(nove,"   Rychlost:  %lu vet/min",pocetvet);
  strcat(text,nove);
  if (naobrazovku == 1 && (ctrlbreak == 0 || akcectrlbreak == 0)) {
    gotoxy(39-(strlen(text)>>1),wherey()); NAPISRETEZEC(" ",14+16);
    NAPISRETEZEC(text,14+16); NAPISRETEZEC(" ",14+16);
    STRANA(1);
   }
  if (kydy != NULL) fprintf(kydy,"\n%s\n",text);
 }

void CTISOURADNICE(void) {
  x=wherex(); y=wherey();
 }

void VLOZSOURADNICE(void) {
  gotoxy(x,y);
 }

void SMAZOBRAZOVKU(BYTE jeli_font) {
  BYTE prepnuto_50na25 = 0;
//  if (delkastrany > 24) { _AX = 4; geninterrupt(0x10); }//kvuli zblblymu fontu
  if (delkastrany > 24 && jeli_font == 1 && pokyd_shell_rows == 0) {
    memset(&pokyd_regs, 0, sizeof(pokyd_regs));
    pokyd_regs.w.ax = 0x0003;
    intr(0x10, &pokyd_regs);  //prepnuti 50 radku na 25 (upr.2005)
    prepnuto_50na25 = 1;
    /* Keep globals in sync with hardware (was 49 rows, now 25). */
    delkastrany = 24;
    pozicehlavicka = 1;
    pozicedatumcas = 2;
   }
  clrscr();
  /* After AX=0003 some DOSBox-X builds hang on INT 10h font reload; BIOS mode set already picks ROM font. */
  if (jeli_font == 1 && font == 1 && prepnuto_50na25 == 0) NASTAVPOKYDFONT();
 }

void PRECTIOBRAZOVKU(BYTE efekt) {
BYTE pozice,radek;
  SMAZKURZOR();
  gettext(1,1,80,delkastrany+1,obrazovka);
  if (efekt == 1 && textefekty == 1) {
    for (radek=1; radek < delkastrany+2; radek++) {
      gettext(1,radek,80,radek,dlouhe);
      for (pozice=1; pozice < 160; pozice+=2) dlouhe[pozice]&=119;
      puttext(1,radek,80,radek,dlouhe);
     }
    delay(150);
   }
 }

void ZAPISOBRAZOVKU(void) {
WORD odkud,dokud;
BYTE radek,pozice;
  if (textefekty == 1) {
    odkud=0;
    for (radek=1; radek < delkastrany+2; radek++) {
      dokud=odkud+160;
      for (pozice=0; odkud < dokud; pozice++,odkud++)
       dlouhe[pozice]=obrazovka[odkud];
      for (pozice=1; pozice < 160; pozice+=2) dlouhe[pozice]&=119;
      puttext(1,radek,80,radek,dlouhe);
     }
    delay(150);
   }
  puttext(1,1,80,delkastrany+1,obrazovka);
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

void STRANA(BYTE kolik) {
  while (stranaradek < delkastrany-1 && kolik > 0) {
    if (wherey() == delkastrany+1) { pozicedatumcas--; pozicehlavicka--; }
    //_DH=delkastrany;_AL=1;_BH=0;_CX=0;_DL=79;_AH=6;geninterrupt(0x10);
    pokyd_emit_nl();
    if (stranaradek < delkastrany-1) stranaradek++;
    kolik--;
   }
  if (kolik > 0) {			//Rolovani okna
    memset(&pokyd_regs, 0, sizeof(pokyd_regs));
    _AL = kolik; _BH = 0;
    _CH =  (unsigned char)(2 + pokyd_shell_rows);
    _CL =  0;
    _DH = (unsigned char)(delkastrany + pokyd_shell_rows);
    _DL = 79;
    _AH =  6;
    geninterrupt(0x10);
   }
  gotoxy(1,wherey());
 }

void VYNULOVANI(BYTE jestli_veta) {
BYTE cislo;
  for (cislo=0; cislo < 20; cislo++) {
    odpovedi[cislo][0]=0;			//Vynulovani odpovedi
    cislaodp[cislo]=0;				//Vynulovani cisel odpovedi
   }
  if (jestli_veta == 1)
   for (cislo=0; cislo < 40; cislo++) veta[cislo][0]=0;    //Vynulovani vety
  celkemodp=skutecnychodp; pozodp=0; skutecnychodp=0;
 }

BYTE BLBOSTI_OPAKOVANE_PISMENO(void) {
BYTE pozice,celkem=strlen(retezec1),znak=0,pocet=1;
  for (pozice=0; pozice < celkem; pozice++) {
    if (retezec1[pozice] == znak) {
      pocet++;
      if (pocet == 3) {
        switch(znak) {
          case '.': case '!': case '?': case ' ': case '1': case '2': case '3':
          case '4': case '5': case '6': case '7': case '8': case '9': case '0':
            break;
          default: return(1);
         }
       }
     }
    else pocet=1;
    znak=retezec1[pozice];
   }
  return(0);
 }

BYTE SLOVO(BYTE *slovo) {
BYTE pozice;
  pozice=0;
  while (pozice < pocetslov) {
    if (stricmp(veta[pozice],slovo) == 0) { poslednislovo=pozice; return(pozice); }
    pozice++;
   }
  return(255);
 }

BYTE ZACATEK(BYTE *slovo) {
BYTE pomoc[80],celkem;
BYTE pozice=0;
  celkem=strlen(slovo);
  while (pozice < pocetslov) {
    strcpy(pomoc,veta[pozice]);
    pomoc[celkem]=0;
    if (stricmp(pomoc,slovo) == 0) { poslednislovo=pozice; return(pozice); }
    pozice++;
   }
  return(255);
 }

BYTE KONECSLOVA(BYTE *slovo) {
BYTE celkem=strlen(slovo),pozice=0,slovocelkem;
  while (pozice < pocetslov) {
    slovocelkem=strlen(veta[pozice]);
    if (slovocelkem >= celkem) {	//kdyz je slovo mensi nez konec,
					//nema to cenu
      if (stricmp(veta[pozice]+slovocelkem-celkem,slovo) == 0) { poslednislovo=pozice; return(pozice); }
     }
    pozice++;
   }
  return(255);
 }

BYTE VYSKYT(BYTE *slovo) {
BYTE retezecvety[80],pomoc,znak;
  strcpy(retezecvety,retezec1);
  for(pomoc=0; retezecvety[pomoc] > 0; pomoc++) {
    znak=retezecvety[pomoc];
    if (znak >= 'A' && znak <= 'Z')
     retezecvety[pomoc]+=32;			//prevedeni na mala pismena
   }
  if (strstr(retezecvety,slovo) != NULL) return(0);	//obsahuje
  else return(255);
 }

BYTE PRIBLIZNA_NALADA(void) {
BYTE navrat=nalada;
  navrat+=(rand()%3)-1;
  if (navrat == 255) navrat=0;				// -1
  else if (navrat == 5) navrat=4;
  return(navrat);
 }

void ODP(BYTE *co,WORD cislo) {			//pridani odpovedi do fronty
BYTE odpoved[80];
BYTE velkepismeno=1,pozice1=0,pozice2=0,celkem=strlen(co)+1;

  if (zenapocitac == 1 && strchr(co,12) != NULL) {
    for (pozice1=0; co[pozice1] != ''; pozice1++);
    pozice1++;
   }
  while (pozice1 < celkem) {
    switch(co[pozice1]) {
      case '*':
        odpoved[pozice2]=0;
        switch(co[++pozice1]) {
          case '0':			//osloveni
            if (jmenocloveka5pad[0] != 0 && samomluva == 0) {//osloveni jmenem
              strcat(odpoved,jmenocloveka5pad); break;
             }
            if (zenskyrod == 0) switch(PRIBLIZNA_NALADA()) {
              case 0: strcat(odpoved,"chlapce"); break;
              case 1: strcat(odpoved,"hochu"); break;
              case 2: strcat(odpoved,"rebele"); break;
              case 3: strcat(odpoved,"chlapecku"); break;
              case 4: strcat(odpoved,"frajere"); break;
             }
            else switch(PRIBLIZNA_NALADA()) {
              case 0: strcat(odpoved,"devce"); break;
              case 1: strcat(odpoved,"devenko"); break;
              case 2: strcat(odpoved,"slecinko"); break;
              case 3: strcat(odpoved,"holcicko"); break;
              case 4: strcat(odpoved,"zenska"); break;
             }
            pozice2=strlen(odpoved)-1;
            break;
          case '1':			//jmeno muzskeho pocitace
            strcat(odpoved,jmenomuzpocitac); break;
          case '2':			//jmeno muzskeho pocitace
            strcat(odpoved,jmenozenapocitac); break;
          case '3':			//jmeno muzskeho pocitace
            strcat(odpoved,jmenocloveka); break;
          case '4':			//jmeno muzskeho pocitace
            strcat(odpoved,jmenocloveka5pad); break;
          default:
            HLASKA("Nastala chyba 12 ve slovniku! Kontaktuj autora! Jde o zavaznou chybu!",4);
            strcat(odpoved,"<CHYBA>"); break;
         }
	if (velkepismeno == 1) odpoved[pozice2]&=0x5F;
        pozice2=strlen(odpoved)-1; velkepismeno=0;
        break;
      case '#':
        odpoved[pozice2]=0;
        switch(co[++pozice1]) {
          case '1':				//posledni slovo
            strcat(odpoved,veta[poslednislovo]); break;
          case '2':				//1. slovo v pameti
            strcat(odpoved,slovapamet[cisloslpamet][0]); break;
          case '3':				//2. slovo v pameti
            strcat(odpoved,slovapamet[cisloslpamet][1]); break;
          case '4':				//3. slovo v pameti
            strcat(odpoved,slovapamet[cisloslpamet][2]); break;
          case '5':				//aktualni slovo za "jsem"
            strcat(odpoved,slovajsem[cislojsem]); break;
          case '6':				//aktualni slovo za "jsi"
            strcat(odpoved,slovajsi[cislojsi]); break;
          case '7':			//vyraz za poslednim nalezenym slovem
	    strcat(odpoved,veta[poslednislovo+1]);
            if (strlen(veta[poslednislovo+1]) <= 4) {	//predlozky pred
              if (poslednislovo+2 < pocetslov) {	//normalnimi slovy
                strcat(odpoved," ");
		strcat(odpoved,veta[poslednislovo+2]);
               }
             } break;
          case '8':		//upraveny vyraz za poslednim nalezenym slovem
	    if (stricmp(veta[poslednislovo+1],"jsi") == 0) strcat(odpoved,"jsem");
	    else if (stricmp(veta[poslednislovo+1],"nejsi") == 0) strcat(odpoved,"nejsem");
	    else if (stricmp(veta[poslednislovo+1],"jsem") == 0) strcat(odpoved,"jsi");
	    else if (stricmp(veta[poslednislovo+1],"nejsem") == 0) strcat(odpoved,"nejsi");
	    else if (stricmp(veta[poslednislovo+1],"budes") == 0) strcat(odpoved,"budu");
	    else if (stricmp(veta[poslednislovo+1],"nebudes") == 0) strcat(odpoved,"nebudu");
	    else if (stricmp(veta[poslednislovo+1],"budu") == 0) strcat(odpoved,"budes");
	    else if (stricmp(veta[poslednislovo+1],"nebudu") == 0) strcat(odpoved,"nebudes");
	    else if (stricmp(veta[poslednislovo+1],"mas") == 0) strcat(odpoved,"mam");
	    else if (stricmp(veta[poslednislovo+1],"nemas") == 0) strcat(odpoved,"nemam");
	    else if (stricmp(veta[poslednislovo+1],"mam") == 0) strcat(odpoved,"mas");
	    else if (stricmp(veta[poslednislovo+1],"nemam") == 0) strcat(odpoved,"nemas");
            else strcat(odpoved,veta[poslednislovo+1]);
            break;
          case '9':		//zbytek posledne nalezeneho slova za n znakama
            strcat(odpoved,veta[poslednislovo]+co[++pozice1]-'0'); break;
          default:
            HLASKA("Nastala chyba 11 ve slovniku! Kontaktuj autora! Jde o zavaznou chybu!",4);
            strcat(odpoved,"<CHYBA>");
            break;
         }
	if (velkepismeno == 1) odpoved[pozice2]&=0x5F;
        pozice2=strlen(odpoved)-1; velkepismeno=0;
        break;
      case '.': case '!': case '?': velkepismeno=1;
                odpoved[pozice2]=co[pozice1]; break;
      case ' ': odpoved[pozice2]=co[pozice1]; break;
      case 12: odpoved[pozice2]=0; pozice1=celkem; break;
      default: velkepismeno=0; odpoved[pozice2]=co[pozice1];
        break;
     }
    pozice1++; pozice2++;
   }
  if (odpoved[0] >= 'a' && odpoved[0] <= 'z') odpoved[0]-=32;
  if (pozodp < 20) {
    strcpy(odpovedi[pozodp],odpoved);
    cislaodp[pozodp]=cislo;
    DBGLOGF("ODP: queued slot=%u cislo=%u len=%u preview=\"%.72s\"",
            (unsigned)pozodp, (unsigned)cislo, (unsigned)strlen((char *)odpoved), odpoved);
    pozodp++; skutecnychodp++;
   }
  else {
    DBGLOG("ODP: queue full (pozodp>=20), answer dropped");
   }
 }

void POCASI(void) {
long pres=0,den,ozon=0;
signed long stupen=0,pomoc;
char pulrok,bio,pomoc1[40];
BYTE novyden=0,spatnysignal=0,textpocasi[300];

  pres = time(NULL)/60/60/24;
  _AH = 0x2C; geninterrupt(0x21); if (_CH >= vkolikhodin) novyden=1;
  if (novyden == 1) pres++;			//novy den
  den = pres;
  srand(32);
  do {
    stupen = stupen + ((rand() % 7) - 3);
    if (stupen < -7) stupen = -7;
    if (stupen > 7) stupen = 7;
   } while (--den != 0);
  den=pres;

PRES:
  pres-=366;
  if (pres < 367) goto PRES2;
  pres-=365;
  if (pres < 367) goto PRES2;
  pres-=365;
  if (pres < 367) goto PRES2;
  pres-=365;
  if (pres < 367) goto PRES2;
  goto PRES;
PRES2:
  if (pres < 0) pres+=366;
  if (pres > 183) { pres=366-pres; /* pulrok=1; */ }

//Stupne

  if (stupen < -3 && pres < 90) bio=3;
  if (stupen > -4 && stupen < 4 && pres < 90) bio=2;
  if (stupen > 3 && pres < 90) bio=1;
  if (stupen < -3 && pres > 89) bio=1;
  if (stupen > -4 && stupen < 4 && pres > 89) bio=2;
  if (stupen > 3 && pres > 89) bio=3;

  dlouhe[0]=0;
  if (ZJISTI_SVATEK() == 1) { VRATDATA(10); spatnysignal=1; }	//svatek
  strcat(dlouhe,"Zapinam radio (cvak) ");
  sprintf(textpocasi,"...a nyni predpoved pocasi na %s den:\n\r",novyden==0 ? "dnesni" :  "zitrejsi");
  pomoc=pres*1000/4975-vyska*9/1000+stupen;			//stupne
  sprintf(pomoc1,"Teplota: %d stup",pomoc);
  if (pomoc == 1 || pomoc == -1) strcat(pomoc1,"en\n\r");
  else if (pomoc < 5 && pomoc > -5 && pomoc != 0) strcat(pomoc1,"ne\n\r");
  else strcat(pomoc1,"nu\n\r");
  strcat(textpocasi,pomoc1);

  if (pomoc < -2 || pomoc > 2) {
    if (stupen < -4) strcat(textpocasi,"Cely den bude destivo a zatazeno.");
    else if (stupen < -1) strcat(textpocasi,"Bude oblacno az zatazeno");
    else if (stupen < 1) strcat(textpocasi,"Bude polojasno az oblacno");
    else if (stupen < 4) strcat(textpocasi,"Cely den bude jasno az polojasno");
    else strcat(textpocasi,"Cely den bude jasno");
   }
  else {						//snih
    if (stupen < -4) strcat(textpocasi,"Budou se tvorit zaveje a snehove jazyky");
    else if (stupen < -1) strcat(textpocasi,"Bude snezit, mozne snehove jazyky");
    else if (stupen < 1) strcat(textpocasi,"Bude oblacno, vyskytuje se moznost snezeni");
    else if (stupen < 4) strcat(textpocasi,"Jasno az polojasno, pripadny snih bude roztavat");
    else strcat(textpocasi,"Po cely den bude jasno, pokud je snih, roztaje");
   }

  sprintf(pomoc1,"\n\rBiopredpoved: %d ",bio); strcat(textpocasi,pomoc1);
  if (bio == 1) strcat(textpocasi,"(mirna zatez)");
  if (bio == 2) strcat(textpocasi,"(stredni zatez)");
  if (bio == 3) strcat(textpocasi,"(tezka zatez)");


// Ozon

  srand(32);
  do ozon=(rand() % 196)+188; while(--den != 0);
  sprintf(pomoc1,"\n\rOzon: %d Dobsonovych jednotek, ",ozon); strcat(textpocasi,pomoc1);
  pomoc=0;
  if (ozon < 286) {
    pomoc=100-((100*ozon)/286)-1;
    sprintf(pomoc1,"coz je %d ",pomoc); strcat(textpocasi,pomoc1);
   }
  else if (ozon >= 286) {
    pomoc=-100+((100*ozon)/286);
    sprintf(pomoc1,"coz je %d ",pomoc); strcat(textpocasi,pomoc1);
   }

  strcat(textpocasi,"procent");
  if (pomoc == 1) strcat(textpocasi,"o");
  else if (pomoc > 1 && pomoc < 5) strcat(textpocasi,"a");

  if (ozon < 286) strcat(textpocasi," po");
  else strcat(textpocasi," na");
  strcat(textpocasi,"d dlouhodobym prumerem ");

  strcat(textpocasi,"\n\r      ");
  if (pomoc < 10) strcat(textpocasi,"(normalni stav)");
  else if (pomoc > 9 && pomoc < 20 && ozon < 286) strcat(textpocasi,"(podprumerny stav)");
  else if (pomoc > 9 && pomoc < 20 && ozon > 286) strcat(textpocasi,"(nadprumerny stav)");
  else if (pomoc > 19 && pomoc < 30 && ozon < 286) strcat(textpocasi,"(vysoce podprumerny stav)");
  else if (pomoc > 19 && pomoc < 30 && ozon > 286) strcat(textpocasi,"(vysoce nadprumerny stav)");
  else if (pomoc > 29 && ozon < 286) strcat(textpocasi,"(extremne podprumerny stav)"); 
  else if (pomoc > 29 && ozon > 286) strcat(textpocasi,"(extremne nadprumerny stav)");

  nahodacislo=time(NULL); srand(nahodacislo);

  if (spatnysignal == 1) {			//spatny signal prijmu
    stupen=strlen(textpocasi);
    for (pomoc=0; pomoc < stupen; pomoc++)
     if (rand()%9 == 0 && textpocasi[pomoc] > 31) textpocasi[pomoc]='.';
   }
  strcat(dlouhe,textpocasi); strcat(dlouhe,"\n\r(cvak)");
  pozodp=100; smyslpocvety=4;
 }

void NAPISZNAKODP(BYTE znak) {
  BYTE b = barvapocitac0;
  /* barvapocitac* are usually conio fg indices 0-15; BIOS needs full attribute (bg|fg). */
  if ((b & 0xF0) == 0) b = (BYTE)(0x00 | (b & 0x0F));
  NAPISZNAK2(znak,b);
 }

void interrupt (*stara_adresa0)(void);

void interrupt (*stara_adresa1B)(void);
void interrupt (*stara_adresa23)(void);

void interrupt (*stara_adresa24)(void);

void VYPNI_REZIDENTY(BYTE int24) {
  /* Compatibility mode: startup no longer installs custom vectors. */
  (void)int24;
 }

void PREKRESLI_RADEK(BYTE insert) {
BYTE pozice=0,celkem=strlen(retezec1)+1,puvpozx=wherex(),znak,barva;
  if (psanivetyskryto == 1) return;
  SMAZKURZOR(); gotoxy(1,wherey());
  while (pozice < celkem) {
    znak=retezec1[pozice];
    if (znak == 0) znak = ' ';			//nula se vymeni za mezeru
    barva=CTIBARVU()&240;			//>>4<<4
    NAPISZNAK2(znak,barva+barvaclovek);
    pozice++;
   }
  gotoxy(puvpozx,wherey());
  if (insert == 0) NASTAVKURZOR();
  else PLNYKURZOR();
 }

void SMAZZNAK(BYTE pozice,BYTE celkem) {
BYTE pomoc;
  if (pozice == 0) return;
  for (pomoc=pozice; pomoc <= celkem; pomoc++)
   retezec1[pomoc-1]=retezec1[pomoc];
  retezec1[--celkem]=0; pozice--;
 }

void PREDKONEC(void) {
  if (cekaniklavesy == 1) {
    textattr(12+128); gotoxy(21,wherey());
    cprintf(" >>>  Stiskni cokoliv pro ukonceni  <<<"); 
    while (kbhit() == 0) { CAS(0); KONTROLA_UTNUTI(); }
    CTIKLAVESU(); BARVA(7);
   }
 }

void ZMEN_POCITAC(void) {
  if (cisloaktualnihopocitace == 1) { cisloaktualnihopocitace++; barvapocitac0=barvapocitac2; }
  else { cisloaktualnihopocitace--; barvapocitac0=barvapocitac1; }
 }

void SLOVO_PAMET(void) {
BYTE pozice=pocetslpamet;
  if (pozice == 50) {				//zase od zacatku
    for (pozice=0; pozice < 50-1; pozice++) {
      strcpy(slovapamet[pozice][0],slovapamet[pozice+1][0]);
      strcpy(slovapamet[pozice][1],slovapamet[pozice+1][1]);
      strcpy(slovapamet[pozice][2],slovapamet[pozice+1][2]);
     }
    pozice=50-1;
   }
  else pocetslpamet++;
  strcpy(slovapamet[pozice][0],veta[0]);
  strcpy(slovapamet[pozice][1],veta[1]);
  strcpy(slovapamet[pozice][2],veta[2]);

  PREVED_NA_MALA(slovapamet[pozice][0]);
  PREVED_NA_MALA(slovapamet[pozice][1]);
  PREVED_NA_MALA(slovapamet[pozice][2]);
 }

void ZJISTI_SLOVO_PAMET(void) {
BYTE pozice1,pozice2,vybrpozice,vybrane[50];
  vybrpozice=0;
  for (pozice1=0; pozice1 < pocetslpamet; pozice1++)
   for (pozice2=0; pozice2 < pocetslov; pozice2++)
    if (stricmp(slovapamet[pozice1][0],veta[pozice2]) == 0)
     vybrane[vybrpozice++]=pozice1;
  if (vybrpozice == 0) cisloslpamet=255;
  else cisloslpamet=vybrane[rand()%vybrpozice];
 }

BYTE SPATNE_SLOVO(BYTE *slovo,BYTE kratke) {
  if (kratke == 1 && strlen(slovo) < 4) return(1); //kratke spojky a predlozky
  if (slovo[strlen(slovo)-1] == 'e') return(1);		//prislovce
  if (stricmp(slovo,"proste") == 0) return(1);
  if (stricmp(slovo,"zkratka") == 0) return(1);
  if (stricmp(slovo,"mezi") == 0) return(1);
  if (stricmp(slovo,"tady") == 0) return(1);
  if (stricmp(slovo,"peknej") == 0) return(1);
  if (stricmp(slovo,"skoro") == 0) return(1);
  if (stricmp(slovo,"temer") == 0) return(1);
  if (stricmp(slovo,"totiz") == 0) return(1);
  if (stricmp(slovo,"celej") == 0) return(1);
  if (stricmp(slovo,"vubec") == 0) return(1);
  if (stricmp(slovo,"uplne") == 0) return(1);
  if (strnicmp(slovo,"teprv",5) == 0) return(1);
  if (strnicmp(slovo,"tak",3) == 0) return(1);
  if (strnicmp(slovo,"ted",3) == 0) return(1);
  return(0);
 }

void JSEM_PAMET(void) {
BYTE pozice,slovo[15];
  strcpy(slovo,veta[SLOVO("jsem")+1]);

  if (SPATNE_SLOVO(slovo,1) == 1) return;

  for (pozice=0; pozice < pocetjsem; pozice++) {
    if (stricmp(slovo,slovajsem[pozice]) == 0) return;
   }

  pozice=pocetjsem;
  if (pozice == 50) {				//zase od zacatku
    for (pozice=0; pozice < 50-1; pozice++) {
      strcpy(slovajsem[pozice],slovajsem[pozice+1]);
     }
    pozice=50-1;
   }
  else pocetjsem++;
  PREVED_NA_MALA(slovo);
  strcpy(slovajsem[pozice],slovo);
 }

void ZJISTI_JSEM_PAMET(void) {
BYTE pozice1,pozice2,vybrpozice,vybrane[50];
  vybrpozice=0;
  for (pozice1=0; pozice1 < pocetjsem; pozice1++)
   for (pozice2=0; pozice2 < pocetslov; pozice2++) {
     if (stricmp(slovajsem[pozice1],veta[pozice2]) == 0) {
       vybrane[vybrpozice++]=pozice1;
      }
    }
  if (vybrpozice == 0) cislojsem=255;
  else cislojsem=vybrane[rand()%vybrpozice];
 }

void JSI_PAMET(void) {
BYTE pozice,slovo[15];
  strcpy(slovo,veta[SLOVO("jsi")+1]);

  if (SPATNE_SLOVO(slovo,1) == 1) return;
  for (pozice=0; pozice < pocetjsi; pozice++) {
    if (stricmp(slovo,slovajsi[pozice]) == 0) return;
   }

  pozice=pocetjsi;
  if (pozice == 50) {				//zase od zacatku
    for (pozice=0; pozice < 50-1; pozice++) {
      strcpy(slovajsi[pozice],slovajsi[pozice+1]);
     }
    pozice=50-1;
   }
  else pocetjsi++;
  PREVED_NA_MALA(slovo);
  strcpy(slovajsi[pozice],slovo);
 }

void ZJISTI_JSI_PAMET(void) {
BYTE pozice1,pozice2,vybrpozice,vybrane[50];
  vybrpozice=0;
  for (pozice1=0; pozice1 < pocetjsi; pozice1++)
   for (pozice2=0; pozice2 < pocetslov; pozice2++)
    if (stricmp(slovajsi[pozice1],veta[pozice2]) == 0)
     vybrane[vybrpozice++]=pozice1;
  if (vybrpozice == 0) cislojsi=255;
  else cislojsi=vybrane[rand()%vybrpozice];
 }

BYTE JMENO_CLOVEKA(void) {
BYTE pozice;
  pozice=SLOVO("jmenuji"); if (pozice == 255) pozice=SLOVO("jmenuju");
  ZNOVU:
  if (pozice == 255 || (pozice+1) == pocetslov) return(0);
  pozice++;
  if (stricmp(veta[pozice],"jako") == 0) return(0);
  if (stricmp(veta[pozice],"se") != 0 && SPATNE_SLOVO(veta[pozice],0) == 0) {
    strcpy(jmenocloveka,veta[pozice]);
    VRAT_5_PAD();
   }
  else goto ZNOVU;
  return(1);
 }

int SPOCITEJ_NALADU(BYTE zceho) {
int nalada=zceho-10;
  switch(charakter) {
    case 0: nalada=0; break;				//stroj
    case 1: if (nalada < 0) {				//naivni
	      nalada*=5; nalada/=3;
	     } break;
    case 2: nalada*=3;					//klidny
	    nalada/=5; break;
    case 3: break;					//prumerny
    case 4: if (nalada > 0) {				//neduverivy
	      nalada*=5; nalada/=3;
	     }
	    else {
	      nalada*=3; nalada/=5;
	     }
	    break;
    case 5: nalada*=5;					//naladovy
	    nalada/=3; break;
    case 6: if (nalada > 6) {				//vybusny
	      nalada*=5; nalada/=2;
	     }
	    else {
	      nalada*=3; nalada/=2;
	     } break;
   }
  return(nalada);
 }

void ZKONTROLUJ_EXTRA_SANCI(BYTE cheat) {
BYTE puvdelayprocenta=delayprocenta,klic,soucet,znakb;
int znak;
WORD pozice1,pozice2,celkem;
DWORD seriove;
  if (cheat == 255) goto ANO;

  if (xtrzapsani != 0) return;

  if (cashovoru > 7500			//vic jak 2.? hodiny
  && pocetuzivvet > 1500		//a uzivatel rekl dost vet
  && (pocetvet/pocetuzivvet) < 4	//a nenechal pocitace mluvit dlouho
  && naladabody == 0			//pocitac ma nejlepsi naladu
  && docasnenaladabody < 0);		//a mel by jeste lepsi
  else return;

  ANO:

  NASTAVBARVU(63,63,63,0); delay(200);
  NASTAVBARVU(0,0,0,0); delay(300);
  NASTAVBARVU(63,63,63,0); delay(300);
  NASTAVBARVU(0,0,0,0);

  VRATDATA(12); celkem=strlen(dlouhe);
  for (pozice1=0,pozice2=0; pozice1 < celkem; pozice1++) {
    if (dlouhe[pozice1] == '') {
      if (zenapocitac == 1) { dlouhe[pozice2]='a'; pozice2++; }
     }
    else if (dlouhe[pozice1] == '') {
      if (zenapocitac == 1) dlouhe[pozice2-1]=dlouhe[pozice1+1];
      pozice1++;
     }
    else {
      dlouhe[pozice2]=dlouhe[pozice1]; pozice2++;
     }
   }
  dlouhe[pozice2]=0; if (delayprocenta < 100) delayprocenta=100; pozodp=100;
  ODPOVED(1);
  delayprocenta=puvdelayprocenta;
  while (kbhit() == 0) { CAS(0); KONTROLA_UTNUTI(); }
  CTIKLAVESU();

  dlouhe[0]=7; dlouhe[1]=0;			//verze 7.0
  dlouhe[2]=cheat;
  _AH = 0x2A; geninterrupt(0x21);		//datum
  dlouhe[3]=_DL; dlouhe[4]=_DH; dlouhe[5]=_CH; dlouhe[6]=_CL;
  _AH = 0x2C; geninterrupt(0x21);		//cas
  dlouhe[7]=_CH; dlouhe[8]=_CL;
  dlouhe[9]=(BYTE)rand(); dlouhe[10]=(BYTE)rand(); dlouhe[11]=(BYTE)rand(); dlouhe[12]=(BYTE)rand();
  seriove=VRAT_SERIOVE_CISLO(); seriove>>=16; _CX=(WORD)seriove;
  dlouhe[13]=_CH; dlouhe[14]=_CL;
  _CX=(WORD)VRAT_SERIOVE_CISLO(); dlouhe[15]=_CH; dlouhe[16]=_CL;
  _CX=(WORD)pocetvet; dlouhe[17]=_CH; dlouhe[18]=_CL;
  _CX=(WORD)pocetuzivvet; dlouhe[19]=_CH; dlouhe[20]=_CL;

  klic=(BYTE)rand();
  for (pozice1=0,soucet=0; pozice1 < 21; pozice1++) {		//zakodovani
    soucet^=dlouhe[pozice1]; soucet+=dlouhe[pozice1];
    dlouhe[pozice1]^=pozice1; dlouhe[pozice1]+=pozice1; dlouhe[pozice1]^='K'+pozice1;
    dlouhe[pozice1]^=klic;
   }

ZAPIS:
  if (readonlymod == 1) {
READONLYDOTAZ: switch(HLASKA("Pokyd je v modu \"jen ke cteni\". Chces presto zapsat tento soubor? [A/N]",2)) {
      case 'a': case 'A': case 'y': case 'Y': break;
      case 'n': case 'N': return;
      default: goto READONLYDOTAZ;
     }
   }
  intel=NULL; xtrzapiskam='C';
  if ((intel=fopen("C:\\POKYD.XTR","wb")) == NULL) {
    if ((HLASKA("Soubor C:\\POKYD.XTR nelze vytvorit. Chces soubor zapsat na disketu? [A/N] A",4+100)&0x5F) != 'N') {
ZAPISNADISKETU:
      xtrzapiskam='A';
      if ((intel=fopen("A:\\POKYD.XTR","wb")) == NULL) {
        if ((HLASKA("Soubor A:\\POKYD.XTR nelze vytvorit. Zkusit znovu? [A/N] A",4+100)&0x5F) != 'N') goto ZAPISNADISKETU;
       }
     }
    if (intel == NULL) { HLASKA("Soubor nebude zapsan. Skoda.",1); xtrzapsani=2; xtrzapiskam=0; return; }
   }
  fwrite("Extra soubor k Pokydu pro nejvernejsi uzivatele - nerozsirovat, ale poslat na iqpokyd@kyblsoft.cz spolecne se svym jmenem a mistem bydliste. Za to ti autor posle specialni verzi Pokydu s venovanim.\r\n",201,1,intel);
  putc(klic,intel);
  fwrite(dlouhe,21,1,intel);

  for (pozice1=0,pozice2=strlen(retezec1)+1; pozice1 < pozice2; pozice1++) {
    znakb=retezec1[pozice1]; soucet^=znakb; soucet+=znakb;
    znakb^=klic; znakb^=pozice1; znakb+=klic; putc(znakb,intel);
   }

  SOUBOR("POKYD.CFG");
  if ((nastaveni=fopen(soubor,"rb")) != NULL) {
      pozice1=0;
NASTCT: znak=getc(nastaveni); if (znak != -1) {
      znakb=(BYTE)znak; soucet^=znakb; soucet+=znakb;
      znakb^=klic; znakb^=pozice1; znakb+=klic; putc(znakb,intel);
      pozice1++; goto NASTCT;
     }
    fclose(nastaveni); nastaveni=NULL;
   }
  putc(soucet,intel);
  fclose(intel); intel=NULL;
  sprintf(dlouhe,"Soubor %c:\\POKYD.XTR uspesne zapsan.",xtrzapiskam);
  HLASKA(dlouhe,3);
  HLASKA("Tento soubor spolecne se svym jmenem a bydlistem odesli na Alesovu adresu.",1);
  xtrzapsani=1;
 }

BYTE EXTRA_SANCE_CHEAT(void) {
BYTE pozice,soucet,xor;
  if (strlen(retezec1) == 31 && pocetslov == 5 && VYSKYT("_bonu") != 255 && VYSKYT("ea") != 255 && VYSKYT("extra_s") != 255) {
    for (soucet=0,xor=0,pozice=0; pozice < 31; pozice++) {
      soucet+=retezec1[pozice]; xor^=retezec1[pozice]; xor+=retezec1[pozice];
     }
    if (soucet == 162 && xor == 12 &&	//KOD_BONUSOVY_EXTRA_SOUBOR_CHEAT
     vypnutecheaty == 0) {
      ZKONTROLUJ_EXTRA_SANCI(255); return(1);
     }
   }
  return(0);
 }
