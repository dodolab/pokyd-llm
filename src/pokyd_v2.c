/* Tento zdrojovy kod je pod licenci GNU/GPL. Muzete ho pouzit k vlastni
   potrebe, ale nesmite jej ani programy zalozene na tomto kodu vyuzit komercne!

   Jedna se o zdrojovy kod programu Pokyd (http://iqpokyd.kyblsoft.cz)
   od Alese Jandy, aktivne vyvijeneho 1999 - 2002
*/

/*
 * pokyd_v2.c - nacteni slovniku (SLOVNIK.DAT), INTRO, externi programy, pomocne systemove akce.
 * NACTI_INTELIGENCI: dekoduje datasoubor a IQ bloky; kontrolni soucty na konci souboru.
 */

/* Loads and decodes SLOVNIK.DAT into datasoubor and IQ blocks. */
BYTE NACTI_INTELIGENCI(void) {

DWORD delka,datapozice,datavelikost,slovnikzacatek,pomoc;
DWORD usekvelikost,pozice1,pozice2;
int pomocznak;
BYTE soucet1=0,soucet2=0,pocetextra,verze1,verze2;
BYTE vyslsoucet1,vyslsoucet2,znak;
BYTE dbg[180];
DWORD filevelikost,startstream;

  DBGLOG("NACTI_INTELIGENCI: begin");
  strcpy(soubor,"SLOVNIK.DAT");
  intel=fopen(soubor,"rb");
  if (intel != NULL) DBGLOG("NACTI_INTELIGENCI: opened SLOVNIK.DAT directly");
  if (intel == NULL) {
    SOUBOR("SLOVNIK.DAT");
    sprintf(dbg,"NACTI_INTELIGENCI: fallback path=%s",soubor); DBGLOG(dbg);
    intel=fopen(soubor,"rb");
   }
  if (intel == NULL) {
    strcpy(soubor,"assets\\SLOVNIK.DAT");
    intel=fopen(soubor,"rb");
    if (intel != NULL) DBGLOG("NACTI_INTELIGENCI: opened assets\\SLOVNIK.DAT");
   }
  if (intel == NULL) {
    DBGLOG("NACTI_INTELIGENCI: fopen failed (all paths)");
    HLASKA("Hrome! Nejak nemuzu otevrit soubor s inteligenci SLOVNIK.DAT!",4);
    HLASKA("Ja ale bohuzel bez toho nemuzu odpovidat, to je totiz muj mozek!",1);
    HLASKA("Jestli tento program rozbalujes z archivu, tak rozbal vsechny soubory!",1);
    if (masterboot == 0) return(1);
   }
  fseek(intel,0,SEEK_END); filevelikost=ftell(intel); fseek(intel,0,SEEK_SET);
  sprintf(dbg,"NACTI_INTELIGENCI: file size=%lu",filevelikost); DBGLOG(dbg);
  znak=getc(intel); znak^='K'; verze1=znak;
  znak=getc(intel); znak^='K'; verze2=znak;
  sprintf(dbg,"NACTI_INTELIGENCI: ver=%c%c",verze1,verze2); DBGLOG(dbg);
  if (verze1 != '7' || verze2 != '0') {				//7.0
    DBGLOG("NACTI_INTELIGENCI: wrong base version");
    HLASKA("Soubor SLOVNIK.DAT neni z teto verze! Je mi lito, nezakydas si!",4);
    if (masterboot == 0) return(1);
   }
  verzeslovniku=getc(intel)^'K';
  sprintf(dbg,"NACTI_INTELIGENCI: slovnik_ver=%u",verzeslovniku); DBGLOG(dbg);
  if (verzeslovniku != 5) {				//verze slovniku: 5
    DBGLOG("NACTI_INTELIGENCI: unsupported dictionary version");
    HLASKA("Nepodporovana verze slovniku!",4);
    if (masterboot == 0) return(1);
   }
  fseek(intel,-2,SEEK_END); vyslsoucet1=getc(intel); vyslsoucet2=getc(intel);
  fseek(intel,3,SEEK_SET); datapozice=getc(intel);
  slovnikzacatek=getc(intel); slovnikzacatek<<=8; slovnikzacatek+=getc(intel);
  datavelikost=slovnikzacatek-datapozice;
  sprintf(dbg,"NACTI_INTELIGENCI: datapoz=%lu slovnikzac=%lu datavel=%lu",
          datapozice,slovnikzacatek,datavelikost); DBGLOG(dbg);
  sprintf(dbg,"NACTI_INTELIGENCI: expected checksum=%u,%u",vyslsoucet1,vyslsoucet2); DBGLOG(dbg);

  if ((datasoubor=malloc(datavelikost)) == NULL) {
    DBGLOG("NACTI_INTELIGENCI: malloc datasoubor failed");
    HLASKA("Malo konvencni pameti pro data! Uvolni pamet zkus to znova.",4); return(1);
   }
  DBGLOG("NACTI_INTELIGENCI: malloc datasoubor ok");
  DBGLOG("NACTI_INTELIGENCI: before first mempaint");
  /* Compatibility mode: avoid intro repaint during dictionary load. */
  /* INTRO_NAPISPAMET(); */
  DBGLOG("NACTI_INTELIGENCI: after first mempaint");
  fseek(intel,0,SEEK_SET);
  for (delka=0; delka < datapozice; delka++) {
    pomocznak=getc(intel);
    if (pomocznak == EOF) {
      sprintf(dbg,"NACTI_INTELIGENCI: EOF in header at %lu/%lu",delka,datapozice); DBGLOG(dbg);
      /* Compatibility fallback: do not abort startup on short header. */
      pomocznak=0;
     }
    znak=(BYTE)pomocznak;
    soucet1^=znak; soucet1+=znak; soucet2+=znak;
   }
  DBGLOG("NACTI_INTELIGENCI: header checksum pass done");
  for (delka=0; delka < datavelikost; delka++) {
    pomocznak=getc(intel);
    if (pomocznak == EOF) {
      sprintf(dbg,"NACTI_INTELIGENCI: EOF in data at %lu/%lu",delka,datavelikost); DBGLOG(dbg);
      /* Compatibility fallback: pad missing bytes and continue. */
      pomocznak=0;
     }
    datasoubor[delka]=(BYTE)pomocznak;
   }
  DBGLOG("NACTI_INTELIGENCI: header+data loaded");

  for (delka=0; delka < datavelikost; delka++) {
    znak=datasoubor[delka]; znak^='6'; znak-='K'; znak^='K'^128;
    datasoubor[delka]=znak;
   }						//rozkoduj data
  DBGLOG("NACTI_INTELIGENCI: data decoded");
  for (delka=0; delka < datavelikost; delka++) {
    znak=datasoubor[delka];
    soucet1^=znak; soucet1+=znak; soucet2+=znak;
   }						//soucet data

  pocetextravet=datasoubor[0];
  pocetiq=datasoubor[1]; pocetiq<<=8; pocetiq+=datasoubor[2];
  sprintf(dbg,"NACTI_INTELIGENCI: pocetextravet=%u pocetiq=%u",pocetextravet,pocetiq); DBGLOG(dbg);

  if (pocetiq > POCET_IQ) {
    DBGLOG("NACTI_INTELIGENCI: pocetiq too high");
    HLASKA("Prilis novy slovnik!",4); if (masterboot == 0) return(1);
   }

  startstream=ftell(intel);
  sprintf(dbg,"NACTI_INTELIGENCI: iq stream start=%lu",startstream); DBGLOG(dbg);
  for (pozice1=0; pozice1 < pocetiq; pozice1++) {
    znak=getc(intel); znak^='6'; znak-='K'; znak^='K'^128; soucet1^=znak; soucet1+=znak; soucet2+=znak;
    usekvelikost=znak; usekvelikost<<=8;
    znak=getc(intel); znak^='6'; znak-='K'; znak^='K'^128; soucet1^=znak; soucet1+=znak; soucet2+=znak;
    usekvelikost+=znak;
    if (pozice1 < 3 || (pozice1%50) == 0 || pozice1+1 == pocetiq) {
      sprintf(dbg,"NACTI_INTELIGENCI: iq[%lu] usek=%lu",pozice1,usekvelikost); DBGLOG(dbg);
    }
    if ((inteligence[pozice1]=malloc(usekvelikost)) == NULL) {
      sprintf(dbg,"NACTI_INTELIGENCI: malloc iq[%lu] failed",pozice1); DBGLOG(dbg);
      free(datasoubor); datasoubor=NULL;
      HLASKA("Malo konvencni pameti pro slovnik! Uvolni pamet a zkus to znova.",4); return(1);
     }
    for (pozice2=0; pozice2 < usekvelikost; pozice2++) {
      znak=getc(intel); znak^='6'; znak-='K'; znak^='K'^128; soucet1^=znak; soucet1+=znak; soucet2+=znak;
      inteligence[pozice1][pozice2]=znak;
     }
    /* INTRO_NAPISPAMET(); */ KONTROLA_UTNUTI();
   }
  sprintf(dbg,"NACTI_INTELIGENCI: iq stream end=%lu",ftell(intel)); DBGLOG(dbg);
  fclose(intel); intel=NULL;

  sprintf(dbg,"NACTI_INTELIGENCI: computed checksum=%u,%u",soucet1,soucet2); DBGLOG(dbg);
  if (soucet1 != vyslsoucet1 || soucet2 != vyslsoucet2) {
    DBGLOG("NACTI_INTELIGENCI: checksum mismatch");
    HLASKA("V souboru SLOVNIK.DAT je asi nejaka chyba, nebo co. Nahraj ho znovu!",1);
    if (masterboot == 0) return(1);
   }
  DBGLOG("NACTI_INTELIGENCI: end ok");
  return(0);
 }

/* Rutina SMAZ_SOUBORY - viz implementace a nazvy promennych (konvence Pokyd). */
void SMAZ_SOUBORY(BYTE pise) {
BYTE holysoubor[80],klavesa;
  VYNULUJ_ODPOVEDI(); SOUBOR("KYDY\\*.*");
START:
  findfirst(soubor,&ffblk,6);
  do {
    if (strcmp(ffblk.ff_name,kydaniny) != 0) {
					//nemazat aktualne zapisovany soubor
      strcpy(soubor,"KYDY\\");
      strcat(soubor,ffblk.ff_name);
      strcpy(holysoubor,soubor);
      SOUBOR(holysoubor);
      remove(soubor);
      CAS(0); KONTROLA_UTNUTI();
     }
    else {
DOTAZ: switch (HLASKA("Mam smazat i soubor, do ktereho ted zapisuju (hovor se prestane psat)? [A/N]",3)) {
        case 'a': case 'A': case 'y': case 'Y': kydaniny[0]=0; psani=2; goto START;
        case 'n': case 'N': case 0x1B: break;
        default: goto DOTAZ;
       }
     }
   } while (!findnext(&ffblk));
ZAVER:
  if (pise == 1) EXTRA_VETA(4);
 }

/* Rutina FORMATOVANI - viz implementace a nazvy promennych (konvence Pokyd). */
void FORMATOVANI(void) {
BYTE pozice;
DWORD cas,cislo;
  VYNULUJ_ODPOVEDI(); EXTRA_VETA(2); ODPOVED(1); VYNULUJ_ODPOVEDI();
  STRANA(pocetradku); cprintf("\rZformatovano:  Sektor 0, Cluster 0  Celkem: 0%%");
  SMAZKURZOR(); pozice=wherey(); CEKEJ(1000);
  for (cislo=0; cislo < 257; cislo++) {
    do {
      CAS(0); KONTROLA_UTNUTI();
     } while (CAS18() == cas);
    cas=CAS18();
    gotoxy(23,pozice); cprintf("%lu, Cluster %lu  Celkem: %d%%",cislo<<16,cislo<<8,(cislo*100)>>8);
   }
  if (kydy != NULL) fprintf(kydy,"  ... Formatovani harddisku ...\n");
  VYNULOVANI(1); EXTRA_VETA(3);
 }

/* Rutina NOUZE_CTRLBREAK - viz implementace a nazvy promennych (konvence Pokyd). */
void NOUZE_CTRLBREAK(void) {
  ctrlbreak++;
  if (ctrlbreak == 3) { textattr(12); cprintf("\r\nPokyd:  Je mi lito, ale jiz jsem zcela zkolaboval. Prosim kontaktujte\r\nmeho autora o teto chybe zaslanim e-mailu na iqpokyd@kyblsoft.cz ci jinak,\r\njak je uvedeno v souboru POKYD.TXT. Dekuji."); ctrlbreak++; setvect(0x1B,stara_adresa1B); setvect(0x23,stara_adresa23); abort(); }
 }

/* Rutina CTRLBREAK1 - viz implementace a nazvy promennych (konvence Pokyd). */
void interrupt CTRLBREAK1(void) {
  NOUZE_CTRLBREAK();
 }

/* Rutina CTRLBREAK2 - viz implementace a nazvy promennych (konvence Pokyd). */
void interrupt CTRLBREAK2(void) {
  ctrlbreakc=1;
  NOUZE_CTRLBREAK();
 }

/* Rutina ZAPNI_REZIDENTY - viz implementace a nazvy promennych (konvence Pokyd). */
void ZAPNI_REZIDENTY(void) {
  /* Compatibility mode for DOSBox-X + Open Watcom:
     disable custom interrupt vector hooks from the Borland-era codepath. */
  memset(&pokyd_regs, 0, sizeof(pokyd_regs));
  _AH = 0x33; _AL = 1; _DL = 1; geninterrupt(0x21);
 }

/* Rutina SPUST - viz implementace a nazvy promennych (konvence Pokyd). */
void SPUST(BYTE *co,BYTE mali_se_zapsat_obrazovka,BYTE plugin) {
signed char chyba;
//  free(vtipymisto); free(inteligence);
  fclose(kydy); kydy=NULL;
  VYPNI_REZIDENTY(plugin^1);
  if (plugin == 1) chyba=spawnl(P_WAIT,co,NULL);
  else system(co);
  ZAPNI_REZIDENTY();
  if (psani == 1) {				//znovuotevreni souboru
    SOUBOR("KYDY\\"); strcat(soubor,kydaniny);
    if ((kydy=fopen(soubor,"a")) == NULL) {
      HLASKA("Nelze znovuotevrit puvodni soubor s hovorem!",4); psani=2;
     }
    else fseek(kydy,0,SEEK_END);
   }
  if (mali_se_zapsat_obrazovka == 1) { NASTAVSPRAVNYMOD(); ZAPISOBRAZOVKU(); }
  else { memset(&pokyd_regs, 0, sizeof(pokyd_regs)); _AX = 0x1003; _BL = 1; geninterrupt(0x10); }	//povol blikani
  if (chyba == -1) HLASKA("Nedostatek pameti pro spusteni!",4);
//  if (NACTI_INTELIGENCI() == 1) { ZAPISOBRAZOVKU(); KONEC(); }
//  VTIPYSOUBOR();
 }

/* Rutina SPUSTPROGRAM - viz implementace a nazvy promennych (konvence Pokyd). */
void SPUSTPROGRAM(void) {
BYTE puvpozx=wherex(),puvpozy=wherey(),cesta[80],soubor[80],argumenty[80];
BYTE puvdisk=getdisk(),puvadresar[80],pomoc1,*pointer;
signed char pomoc;
  PRECTIOBRAZOVKU(1); SMAZOBRAZOVKU(0); jeli_nastaveni=0;
  pomoc=0; strcpy(cesta,retezec1); pomoc1=strlen(cesta);
  while (pomoc < pomoc1) {			//prevedeni na mala pismena
    if (cesta[pomoc] >= 'A' && cesta[pomoc] < 'Z') cesta[pomoc]+=32;
    pomoc++;
   }
  pomoc1=strlen(cesta); pomoc=0; soubor[5]=0;
  while (pomoc < pomoc1) {				//Hledani "spust"
    strncpy(soubor,cesta+pomoc,5);
    if (strcmp(soubor,"spust") == 0) goto HLEDANIDAL;
    pomoc++;
   }
HLEDANIDAL:
  if (strlen(cesta) > pomoc+6) strcpy(cesta,cesta+pomoc+6);
  else cesta[0]=0;
  argumenty[0]=0;
  while (cesta[0] == ' ') strcpy(cesta,cesta+1);	//vynechani mezer
  if (cesta[0] == '*') {			//neformatovane spusteni
    NAPISRETEZEC("Spoustim ",13+16);
    NAPISRETEZEC(cesta+1,15+16);
    NAPISRETEZEC("...",13+16);
    cprintf("\n\r"); strcpy(soubor,cesta+1);
    goto SPUST;
   }
  else if (cesta[0] == '#') {		//spusteni primo, ne pres command.com
    pointer=searchpath(cesta+1);
    if (pointer == NULL) { HLASKA("Zadany soubor nelze nalezt v zadanem adresari ani v promenne PATH!",4); goto KONEC; }
    strcpy(soubor,pointer);
    NAPISRETEZEC("Spoustim ",13+16);
    NAPISRETEZEC(soubor,15+16);
    NAPISRETEZEC("...",13+16);
    cprintf("\n\r");
    SPUST(soubor,0,1); goto SPUST2;
   }
  pomoc1=0; while (cesta[pomoc1] != '\x0') {		//Hledani argumentu
    if (cesta[pomoc1] == ' ') {
      strcpy(argumenty,cesta+pomoc1+1);
      cesta[pomoc1]=0;
     }
    else pomoc1++;
   }
  pomoc=strlen(cesta)-1; while (pomoc > -1) {
    if (cesta[pomoc] == '\\' || cesta[pomoc] == ':') {
      strcpy(soubor,cesta+pomoc+1);
      cesta[pomoc+1]=0;
      if (cesta[pomoc] != ':' && cesta[pomoc-1] != ':')
      cesta[pomoc]=0;				//Umozneni napr. C:\
      goto DAL;
     }
    pomoc--;
   }
  strcpy(soubor,cesta); cesta[0]=0;
  DAL:
  if (cesta[1] == ':') setdisk((cesta[0]&0x5F)-'A');	//Disk
  NAPISRETEZEC("Spoustim ",13+16);
  if (soubor[0] == 0) NAPISRETEZEC("prikazovy radek",13+16);
  else {
    NAPISRETEZEC("program ",13+16);
    if (cesta[0] == 0) NAPISRETEZEC("nebo prikaz ",13+16);
    NAPISRETEZEC(soubor,15+16);
   }
  if (cesta[0] != 0) {
    NAPISRETEZEC(" v adresari ",13+16);
    NAPISRETEZEC(cesta,15+16);
   }
  if (argumenty[0] != 0) {
    NAPISRETEZEC(" s argumenty ",13+16);
    NAPISRETEZEC(argumenty,15+16);
    if (soubor[0] != 0) strcat(soubor," "); strcat(soubor,argumenty);
   }
  NAPISRETEZEC("...",13+16); cprintf("\r\n");
  puvadresar[0]='\\'; getcurdir(0,puvadresar+1);//precteni aktualniho adresare
  chdir(cesta);
SPUST:  SPUST(soubor,0,0);
SPUST2:
  SMAZKURZOR(); PIS(28,25," Pokyd - stiskni cokoliv ",13+128+16);
  chdir(puvadresar); setdisk(puvdisk);		//navrat do puvodni cesty
  textbackground(0); while (kbhit() == 0) KONTROLA_UTNUTI(); CTIKLAVESU();
KONEC:
  NASTAVSPRAVNYMOD(); ZAPISOBRAZOVKU();
  jeli_nastaveni=1; gotoxy(puvpozx,puvpozy);
  ZAPISCAS();
/*  odpovedi[0][0]=0; pozodp=0;
  switch(navratovykod) { case 0: NAHODA("Dost nejakeho spousteni a venuj se laskave zase mne.","Proc spoustis jine programy? Copak ja ti nejsem dost dobrej?","Spusteni souboru - jak jsi jiste postrehl - dopadlo uspesne.");
    case -1: NAHODA("Nic mi do toho neni, ale asi ses spletl v souboru nebo v ceste.","Napsals to blbe! Zkus to znovu!","Nerad rusim, ale napsal jsi ten soubor blbe.");
   }*/
 }

/* Rutina SMYSL_VETY_POCITACE - viz implementace a nazvy promennych (konvence Pokyd). */
void SMYSL_VETY_POCITACE(void) {

// 0 - normalni veta
// 1 - otazka
// 2 - credits
// 3 - vtip
// 4 - pocasi

  if (samomluva == 0) {
    if (strchr(odpovedi[cislo],'?') != NULL) smyslpocvety=1;
    else smyslpocvety=0;
   }
  else {
    if (strchr(retezec1,'?') != NULL) smyslpocvety=1;
    else smyslpocvety=0;
   }
 }

/* Rutina NASTAVENI - viz implementace a nazvy promennych (konvence Pokyd). */
BYTE NASTAVENI(void) {
char pozice,odpoved[80],jinesercislo=0;
DWORD seriove;
  SOUBOR("POKYD.CFG");
  if ((nastaveni = fopen(soubor,"rb")) == NULL) {
    HLASKA("Je mi lito, ale soubor POKYD.CFG neexistuje nebo nejde otevrit.",4);
   }
  else {
    hlasenispatnehocfg=0;

    zenskyrod=CTINASTAVENI(1,0,1,0);
     zenapocitac=CTINASTAVENI(0,0,1,0);
    nalada=CTINASTAVENI(0,0,4,2); naladabody=nalada*15+7;
     charakter=CTINASTAVENI(0,0,6,3);
    CTINASTAVENITEXT(jmenomuzpocitac,"Klaban");
     CTINASTAVENITEXT(jmenozenapocitac,"Daria");
    pocetpocitacu=CTINASTAVENI(0,0,1,0); pocetpocitacu++; //je jina otazka nez promenna
     nastvani=CTINASTAVENI(0,0,1,1);
    zvuk=CTINASTAVENI(0,0,1,1);
     pocetvterin=CTINASTAVENI(0,0,99,5);
    zvysovani=CTINASTAVENI(0,0,1,1);
     delayprocenta=CTINASTAVENI(0,0,999,100);
    font=CTINASTAVENI(0,0,1,1);
     pocetradku=CTINASTAVENI(0,1,99,2);
    textefekty=CTINASTAVENI(0,0,1,1);
     pocetsetricvterin=CTINASTAVENI(0,0,9999,180);
    mod=CTINASTAVENI(0,0,2,1); mod*=25;
     slovenstina=CTINASTAVENI(0,0,1,0);
    pravopis=CTINASTAVENI(0,0,1,1);
     psani=CTINASTAVENI(0,0,1,1);
    extrasouborpsani=CTINASTAVENI(0,0,1,0);
     vyska=CTINASTAVENI(0,0,9999,300);
    svtipy=CTINASTAVENI(0,0,2,2);
     spocasi=CTINASTAVENI(0,0,2,0);
    vkolikhodin=CTINASTAVENI(0,0,23,16);
     yz=CTINASTAVENI(0,0,1,0);
    cekaniklavesy=CTINASTAVENI(0,0,1,0);
     ulozeninakonci=CTINASTAVENI(0,0,1,1);
    akcectrlbreak=CTINASTAVENI(0,0,2,0);
     barvaclovek=CTINASTAVENI(0,1,15,14);
    barvapocitac1=CTINASTAVENI(0,1,15,10);
     barvapocitac2=CTINASTAVENI(0,1,15,11);
    seriove=CTINASTAVENI(0,0,0xFFFFFFFF,0);
     if (seriove != VRAT_SERIOVE_CISLO()) jinesercislo=1;

    fclose(nastaveni); nastaveni=NULL;
   }
  return(jinesercislo);
 }

/* Rutina INTRO_NAPISPAMET - viz implementace a nazvy promennych (konvence Pokyd). */
void INTRO_NAPISPAMET(void) {
DWORD velikost=coreleft();
BYTE text[20];
  sprintf(text,"     Pamet:  %lu B",velikost); INTRO_NAPIS(80-strlen(text),25,text,velikost < 50000 ? 12 : velikost < 100000 ? 4 : 8);
 }

/* Rutina OBRAZEK_POKYDU - viz implementace a nazvy promennych (konvence Pokyd). */
void OBRAZEK_POKYDU(BYTE pozicex,BYTE pozicey,BYTE barva,BYTE pruhledne) {
BYTE znak[2],kolikuzx,kolikuzy;
WORD pozicepokyd=0;
  gotoxy(pozicex,pozicey); textbackground(0);
  for (kolikuzy=0; kolikuzy < OBRAZEK_POKYDU_Y; kolikuzy++) { //Tabulka "Pokyd"
    for (kolikuzx=0; kolikuzx < OBRAZEK_POKYDU_X; kolikuzx++) {
      if ("######         #### ###      ######  "
          " ##  ##  ####   ##  ## ## ### ##  ## "
          " ##  ## ##  ##  ## ##  ##  ## ##   ##"
          " ##  ## #    ## ####   ##  ## ##   ##"
          " ##### ##    ## ## ##   ####  ##   ##"
          " ##    ##    ## ##  ##   ##   ##  ## "
          "####    ##  ## #### ###  ##  ######  "
          "         ####           ####         "
        [pozicepokyd+kolikuzx] != ' ') {
        if (pruhledne == 0) NAPISZNAK2(219,barva);
        else {
	  gettext(pozicex+kolikuzx,pozicey+kolikuzy,pozicex+kolikuzx,pozicey+kolikuzy,znak);
	  znak[1]=(znak[1]&15)+(barva<<4);
	  puttext(pozicex+kolikuzx,pozicey+kolikuzy,pozicex+kolikuzx,pozicey+kolikuzy,znak);
         }
       }
      else gotoxy(pozicex+kolikuzx+1,pozicey+kolikuzy);
     }
    gotoxy(pozicex,pozicey+kolikuzy+1); pozicepokyd+=OBRAZEK_POKYDU_X;
   }
 }

/* Ramecek pri START / radky 21-23 u sloupce 52 (nahore pod "Ctu inteligenci"). */
static void INTRO_UKOL_BOX(void) {
  static const char ukol[]=
    "+-------------------------+"
    "|                         |"
    "+-------------------------+";
  WORD pozice=0,celkem=(WORD)strlen(ukol),zatimpozice=0;
  gotoxy(52,21);
  while (zatimpozice < celkem) {
    zatimpozice+=27;
    while (pozice < zatimpozice) NAPISZNAK2((BYTE)ukol[pozice++],8+(7<<4));
    gotoxy(52,wherey()+1);
   }
 }

/* Rutina INTRO - viz implementace a nazvy promennych (konvence Pokyd). */
void INTRO(int argc,BYTE puvpozy) {
#define NAST_Y 15

WORD rok;
BYTE pomoc=0,ah,al,otaznikpoz=0,prvnispusteni=0,zacatecniknabidka=0,den,mesic;
BYTE intro_val_attr;
static BYTE opakovani=0;
DWORD blikaniverze=0,otaznikcas=0,cassetric;
//DWORD far *int1,*int3;

  DBGLOG("INTRO: entry");
  CTI: SMAZOBRAZOVKU(0);
  CTI2: SMAZKURZOR();
  jeli_nastaveni=0;				//nepsat cas, datum, nic
  grafika25=1;					//je nastaveno 25 radku
  OBRAZEK_POKYDU(2,1,6,0);
  INTRO_CARY();

  PIS(15,11,"v e r z e    ",14);
  NAPISRETEZEC("7 . 0",11);				//verze 7.0

  memset(&pokyd_regs, 0, sizeof(pokyd_regs));
  _AH = 0x2A; geninterrupt(0x21); den = _DL; mesic = _DH; rok = _CX;

  INTRO_NAPIS(54,8,"Autor programu Ales Janda",7);
  INTRO_NAPIS(56,9,"KYBLSoft (C) 1999-2005",7);
  INTRO_NAPIS(57,10,"4. vydani, verze 2005",15);

  INTRO_NAPIS(55,2,"http://iqpokyd.kyblsoft.cz",11);

  INTRO_NAPIS(59,3,"Napsano v Borland C++",7);

  if ((den == 29 && mesic == 8) || (den == 23 && mesic == 9) || (den == 25 && mesic == 12)) {
    PIS(70,4,"",14+4*16);
   }
  INTRO_NAPIS(72,4,"freeware",2);

  INTRO_NAPISPAMET();

  INTRO_NAPIS(51,19,"Pri psani otazek pis otaznik!",15);

  INTRO_NAPIS(52,13,"Napoveda - F1   Pluginy - F2",14);
  INTRO_NAPIS(50,14,"Mod uceni pro zacatecniky - F9",14);
  INTRO_NAPIS(49,15,"Test Pokydu - 'T'   Konec - 'K'",14);
  INTRO_NAPIS(52,16,"jina klavesa -",14);
  INTRO_NAPIS(67,16,"START POKYDU!",10);

  INTRO_UKOL_BOX();

  KONTROLA_UTNUTI();
  if (rok == 2005) {			//Nova verze
    PIS(69,1," NOVA VERZE ",14+128+4*16);
   }
  else if (rok < 2005 || (rok == 2005 && mesic < 11)) {	//spatne datum (11/2005)
    PIS(65,1," SPATNE DATUM ! ",14+128+4*16);
    if (zvuk == 1) { sound(1000); delay(50); nosound(); }
   }

  if (opakovani == 0) {					//detekovat jen poprve
    SOUBOR("PUVODNI.PKD");
    if ((nastaveni=fopen(soubor,"rb")) == NULL) zacatecniknabidka=1;
    else {
      fclose(nastaveni); nastaveni=NULL;
     }

    SOUBOR("POKYD.CFG");
    if ((nastaveni=fopen(soubor,"rb")) == NULL) {
      if (prvnispusteni == 0) HLASKA("Soubor POKYD.CFG nenalezen, nastaveni programu je standardni.",1);
     }
    else {
      fclose(nastaveni); nastaveni=NULL;
      PIS(59,22,"Ctu nastaveni",12+7*16);
      zacatecniknabidka|=NASTAVENI();	//jestli je 0 a NASTAVENI() 1, bude 1
     }
    if (zacatecniknabidka == 1) {
      PIS(54,22,"Prvni spusteni, vitej !",1+7*16); prvnispusteni=1;
      switch(HLASKA("Zdravim noveho uzivatele! Mam te postupne zaucovat, jak pouzivat program? [A/N]",1)) {
        case 'a': case 'A': case 'y': case 'Y':
          zacatecnik=1; VYNULUJ_ZACATECNIK(); break;
        default:
          HLASKA("Pokud toto budes chtit v budoucnu zvolit, zmacknes F9.",1); break;
       }
     }
   }
  INTRO_NAPIS(3,NAST_Y,"Kontrola nastaveni:",15);
  INTRO_NAPIS(2,NAST_Y+1,"--------------------------------         zmena:",15);

  NASTAVENI:
  INTRO_NAPIS(2,NAST_Y+2,"Pohlavi uzivatele:",14); if (zenskyrod == 0) INTRO_NAPIS(23,NAST_Y+2,"muz ",13);
  else INTRO_NAPIS(23,NAST_Y+2,"zena",13); INTRO_NAPIS(34,NAST_Y+2,"'P'",11);

  INTRO_NAPIS(2,NAST_Y+3,"Pohlavi pocitace:",14); if (zenapocitac == 0) INTRO_NAPIS(23,NAST_Y+3,"muz ",13);
  else INTRO_NAPIS(23,NAST_Y+3,"zena",13); INTRO_NAPIS(34,NAST_Y+3,"F3",11);

  /* Full VGA attribute byte (bg from intro + fg 13). PIS(...,13) used only nibble 13 -> wrong glyphs. */
  gotoxy(23,NAST_Y+2);
  intro_val_attr = (CTIBARVU() & (BYTE)240) | (BYTE)13;

  INTRO_NAPIS(2,NAST_Y+4,"Nalada pocitace:",14);
  VRAT_NAZEV_NALADY(dlouhe);
  { BYTE pozice=0, celkem=(BYTE)strlen((char *)dlouhe);
    while (pozice < celkem) {
      gotoxy((BYTE)(23+pozice),NAST_Y+4);
      NAPISZNAK(dlouhe[pozice], intro_val_attr);
      pozice++;
     }
   }
  INTRO_NAPIS(34,NAST_Y+4,"F7/F8",11);

  INTRO_NAPIS(2,NAST_Y+5,"Charakter pocitace:",14);
  VRAT_NAZEV_CHARAKTERU(dlouhe);
  { BYTE pozice=0, celkem=(BYTE)strlen((char *)dlouhe);
    while (pozice < celkem) {
      gotoxy((BYTE)(23+pozice),NAST_Y+5);
      NAPISZNAK(dlouhe[pozice], intro_val_attr);
      pozice++;
     }
   }
  INTRO_NAPIS(34,NAST_Y+5,"F5",11);

  INTRO_NAPIS(6,NAST_Y+6,"...vsechna nastaveni - F4, ulozit - 'U'",10);

  KONTROLA_UTNUTI();
  if (opakovani == 0) {

/*    int1=(DWORD far *)getvect(1); int3=(DWORD far *)getvect(3);
    sprintf(dlouhe,"DEBUG: %lu !",int3-int1);
    HLASKA(dlouhe,4);
    if ((int3-int1) != 7 && (int3-int1) != 0) {
      exit(0);
     }*/

    INTRO_UKOL_BOX();
    PIS(54,22,"    Ctu inteligenci    ",12+7*16);
    if (NACTI_INTELIGENCI() == 1) { ZAPISOBRAZOVKU(); KONEC(); }
    if (font == 1) NASTAVPOKYDFONT();
    CTI_INFORMACE_O_VETACH(); nactenyslovnik=1;
   }

  if (puvodnipkdzapsani == 0 && puvodnipkdmesic != 0) {//kdyz uz byl Pokyd spusten
    INTRO_NAPIS(2,25,"Posledni spusteni:",4);
    sprintf(soubor,"%d.%d.%u %d:%02d",puvodnipkdden,puvodnipkdmesic,puvodnipkdrok,puvodnipkdhodina,puvodnipkdminuta);
    INTRO_NAPIS(22,25,soubor,2);
   }

   if (opakovani == 0) {
    pomoc++;
    KONTROLA_UTNUTI();
    if (psani != 0) {
      psani=1;
      PIS(54,22," Hledam spravny soubor ",12+7*16);
      HLEDEJSOUBOR();
      KONTROLA_UTNUTI();
     }
    INTRO_NAPISPAMET();
    PIS(54,22,"      Tridim vtipy     ",12+7*16);
    VTIPYSOUBOR();
    pomoc--;
   }

  INTRO_NAPISPAMET();
  PIS(58,22,"Stiskni cokoliv",9+7*16);
  if (opakovani == 0) {
    if (den == 29 && mesic == 8) HLASKA("Dnes mam narozeniny! Dne 29.8.1999 vysla 1. verze Pokydu!",2);
    opakovani=1;
   }

#if krokovani == 0
  if (introakcespusteni == 1) { introakcespusteni=0; goto TEST; }
  if (introakcespusteni == 2) { introakcespusteni=0; SETRIC_OBRAZOVKY(); goto CTI; }
  if (introakcespusteni == 3) { introakcespusteni=0; ZAPIS_INFORMACE_O_VETACH(3,0); goto ULOZ; }
  if (introakcespusteni == 4) { introakcespusteni=0; goto HELP; }
  if (introakcespusteni == 5) { introakcespusteni=0; goto PLUGINY; }
  if (introakcespusteni == 6) { introakcespusteni=0; goto JDINANASTAV; }
  if (introakcespusteni == 7) { introakcespusteni=0; goto KONEC; }
  if (introakcespusteni == 8) { introakcespusteni=0; zacatecnik=1; VYNULUJ_ZACATECNIK(); }

ZACATECNIK:
  ZACATECNIK("Napovidat ti budu touto formou, kazdou takovou hlasku musis potvrdit klavesou.",11);
  ZACATECNIK("Kazdou napovedu ti ukazu pouze jednou, takze cti pozorne!",12);
  ZACATECNIK("Prohledni si text na obrazovce a rozhodni se. Hovor zacnes napr. Enterem.",13);
GETCH:
  DBGLOG("INTRO: enter GETCH");
  cassetric=time(NULL);
  if (argc < 2) {
    while (kbhit() == 0) {
      if (pocetsetricvterin != 0 && (cassetric+pocetsetricvterin) < time(NULL)) {
        SETRIC_OBRAZOVKY(); goto CTI;
       }
      if (CAS18() > blikaniverze+3) {			//blikani cisla verze
	if ((blikaniverze=VYBERBARVU()) < 10) blikaniverze=11;
        PIS(15,11,"v e r z e    ",blikaniverze);
	NAPISRETEZEC("7 . 0",VYBERBARVU());		//verze 7.0
	blikaniverze=CAS18();
       }
      KONTROLA_UTNUTI();
      delay(7);
      if (SETRIC_KLAVESA() == 1) cassetric=time(NULL);
     }

    pomoc=getch();
    DBGLOGF("INTRO: key=%u", (unsigned)pomoc);
    switch(pomoc) {		//neni-li pritomen argument, cekej na klavesu
      case 0: case 224: switch(getch()) {
        case 16: case 37: case 45: goto SKONCI;		//Alt-Q, Alt-K, Alt-X
	case 59: HELP: HELPF1(1); goto CTI;			//F1
        case 60: PLUGINY: spustenipodprogramu=VYBER_PLUGINU();	//F2
		 if (spustenipodprogramu > 0) goto UPLNYKONEC;
                 else goto CTI2;
        case 61: if (zenapocitac == 0) zenapocitac=1;		//F3
                 else zenapocitac=0; goto NASTAVENI;
	case 62: goto JDINANASTAV;				//F4
        case 63: charakter++; if (charakter > 6) charakter=0;	//F5
                 goto NASTAVENI;
        case 64: SETRIC_OBRAZOVKY(); goto CTI;			//F6
        case 65: if (nalada > 0) nalada--;			//F7
                 naladabody=nalada*15+7; goto NASTAVENI;
        case 66: if (nalada < 4) nalada++;			//F8
                 naladabody=nalada*15+7; goto NASTAVENI;
        case 67: zacatecnik=1; VYNULUJ_ZACATECNIK();		//F9
                 goto ZACATECNIK;
        case 68: goto SKONCI;					//F10
        case 92: goto ULOZ;					//Shift-F9
        case 106: case 107: goto SKONCI;		//Alt-F3, Alt-F4
	default: break;
       } break;
      case 'n': case 'N':
        JDINANASTAV: NASTAV(); goto CTI;
      case 'p': case 'P': if (zenskyrod==0) zenskyrod=1;
                          else zenskyrod=0; goto NASTAVENI;
      case 'u': case 'U':
ULOZ:	NASTAVSOUBOR(1); goto GETCH;
      case 't': case 'T':
TEST:   SMAZOBRAZOVKU(0); SOUBOR("TESTPKD.EXE");
        NASTAVSPRAVNYFONT(); SPUST(soubor,0,1);
        SMAZKURZOR(); goto CTI;
      case 'k': case 'K':
SKONCI:         pocetsouboru--;			//pocet spusteni
                if (psani == 1) {
                  if (zapisovani == 0 && extrasouborpsani == 1) {//smazani
                    fclose(kydy); kydy=NULL; SOUBOR("KYDY\\");	//souboru
                    strcat(soubor,kydaniny); remove(soubor);	//s kydama
                   }
                  else ZAVRISOUBORSKYDAMA("\nPokyd byl predcasne ukoncen.",1);
                 }
                grafika25=0; NASTAVSPRAVNYMOD(); ZAPISOBRAZOVKU(); ZAPIS_INFORMACE_O_VETACH(3,1); KONEC();
      default: break;
     }
   }
#endif

KONEC:
  DBGLOGF("INTRO: exit KONEC intro_arg=%d", pokyd_intro_argc_snapshot);
  /* Always safe handoff to main: never NASTAVSPRAVNYMOD/SMAZOBRAZOVKU here (DOSBox-X crash). */
  ODROLUJ();
  DBGLOG("INTRO: after ODROLUJ");
  /* Do not use INTRO's argc here - stack pressure can corrupt it (log showed intro_arg=200).
     Use snapshot from main; clamp Y so gotoxy never asks BIOS for an invalid row. */
  if (pokyd_intro_argc_snapshot >= 2) {
    grafika25 = 0;
    { int gy = (int)(unsigned char)puvpozy;
      if (gy < 1 || gy > 25) gy = 1;
      gotoxy(1, (BYTE)gy);
     }
   }
  pokyd_finish_intro_handoff();
  DBGLOG("INTRO: video handoff complete");
  DBGLOG("INTRO: handing off to pokyd_run_after_intro (avoid RET from INTRO stack)");
  pokyd_run_after_intro();
  DBGLOG("INTRO: ret?");
  return;
UPLNYKONEC:
  /* Return from plugin picker: restore saved screen without video mode reset. */
  DBGLOG("INTRO: UPLNYKONEC");
  grafika25=0;
  ZAPISOBRAZOVKU();
  gotoxy(1,puvpozy);
  DBGLOG("INTRO: UPLNYKONEC -> pokyd_run_after_intro");
  pokyd_run_after_intro();
  DBGLOG("INTRO: ret? U");
  return;
 }

/* Rutina NAPIS_GOTOXY - viz implementace a nazvy promennych (konvence Pokyd). */
void NAPIS_GOTOXY(BYTE x,BYTE y) {
  if (psanivetyskryto == 0) gotoxy(x,y);
 }

/* Rutina NAPIS - viz implementace a nazvy promennych (konvence Pokyd). */
void NAPIS(void) {
signed char pozice,celkem,epozice,esmer,efekt;
DWORD cas18,dalsikec,cassetric;
BYTE znak,pomoc,kdejex,kdejey,insert=0,scankod,prohozeni=1,pozicey,pocetkecvterin=pocetvterin;
BYTE skrytepsanizatim=0;
  pozice=0; celkem=0; retezec1[0]=0; pocetrealtimekecu2=0;
  cassetric=time(NULL);
  UPLNYSTART1:
  SMAZKURZOR(); STRANA(pocetradku);
  BARVA(barvaclovek); esmer=1; epozice=0; pozicey=wherey();
  efekt=rand()%2; if (efekt == 1) epozice=0;
  cas18=CAS18(); if (psanivetyskryto == 0) NAPISRETEZEC(retezec1,barvaclovek);
  goto NAPISSTART2;
  NAPISSTART:
  cassetric=time(NULL);

  NAPISSTART2:
  dalsikec=time(NULL);

  if (psanivetyskryto == 1) gotoxy(1,pozicey);

  if (spustenipodprogramu > 0) BARVA(barvaclovek); 
  if (spustenipodprogramu == 1) {
    strcpy(retezec1,"spust "); spustenipodprogramu=0;
    if (psanivetyskryto == 0) NAPISRETEZEC(retezec1,barvaclovek); pozice=6; celkem=6;
   }
  if (spustenipodprogramu == 2) {
    strcpy(retezec1,"vymaz soubory"); spustenipodprogramu=0;
    if (psanivetyskryto == 0) NAPISRETEZEC(retezec1,barvaclovek); pozice=13; celkem=13;
   }
  if (spustenipodprogramu == 3) {
    strcpy(retezec1,"mluv s druhym pocitacem"); spustenipodprogramu=0;
    if (psanivetyskryto == 0) NAPISRETEZEC(retezec1,barvaclovek); pozice=23; celkem=23;
   }
  if (spustenipodprogramu == 4) {
    strcpy(retezec1,"zformatuj mi harddisk"); spustenipodprogramu=0;
    if (psanivetyskryto == 0) NAPISRETEZEC(retezec1,barvaclovek); pozice=21; celkem=21;
   }

  if (strcmp(retezec1,"*#06#") == 0) {
    HLASKA("A hele! Mobilovej maniak! Smutny! Identifikacni cislo je 765930777641869.",7);
    retezec1[0]=0; pozice=0; celkem=0; BARVA(barvaclovek); dalsikec=time(NULL);
   }

  if (insert == 0) NASTAVKURZOR();
  else PLNYKURZOR();

  do {
    KONTROLA_UTNUTI();					//Ochrana Ctrl-Break
    if (pocetsetricvterin != 0 && (cassetric+pocetsetricvterin) < time(NULL)) {
SETRIC:  PRECTIOBRAZOVKU(1); SETRIC_OBRAZOVKY(); NASTAVSPRAVNYMOD();
      ZAPISOBRAZOVKU(); goto NAPISSTART;
     }
    if (celkem == 0 && pocetvterin > 0 && dalsikec+pocetkecvterin < time(NULL)) {
      llm_idle_prah_vterin = (WORD)pocetkecvterin;
      ZACATECNIK("Nyni pocitac rekne dalsi vetu, aniz by cekal na tebe.",06);
      REALTIMEKEC(); dalsikec=time(NULL); VYNULOVANI(0);
      if (zvysovani == 1 && pocetkecvterin < 255) pocetkecvterin++;
      if (pocetrealtimekecu == 0 && pocetpocitacu == 2)
       pocetkecvterin=pocetvterin;		//hovor se nezpomaluje
      goto UPLNYSTART1;
     }
    if (textefekty == 0) goto POZICE;
    if (psanivetyskryto == 0) switch(efekt) {
      case 0:						//Behajici barva
	if (epozice > celkem) {			//Rychlejsi Backspace
	  PIS(epozice+1,pozicey," ",barvaclovek);
	  epozice=celkem; esmer=0;
	  gotoxy(celkem,pozicey);
	 }
	if (CAS18() != cas18 && celkem > 1) {		//Efekt pri psani
	  cas18=CAS18(); textcolor(barvaclovek);
	  if (esmer == 0 && epozice < 1)
	   esmer=1;
	  if (esmer == 1) {		//Z leva do prava
	    gotoxy(epozice+1,pozicey);
	    NAPISZNAK2(retezec1[epozice],barvaclovek);	//Stare pismeno
	    epozice++;
	    NAPISZNAK2(retezec1[epozice],barvaclovek+16);
	    if (epozice+1 == celkem) esmer=0;
	   }
	  else {				//Z prava do leva
	    gotoxy(epozice+1,pozicey);
	    NAPISZNAK(retezec1[epozice],barvaclovek);
	    epozice--;
	    gotoxy(epozice+1,pozicey);
	    NAPISZNAK2(retezec1[epozice],barvaclovek+16);
	    if (epozice < 1) esmer=1;
	   } goto POZICE;
      case 1:					//Nahodne meneni barev
            if (CAS18() != cas18 && celkem > 0) {
	      cas18=CAS18();
	      if (epozice < celkem) {	//kdyz neni rychlejsi BackSpace
		gotoxy(epozice+1,pozicey);
		NAPISZNAK(retezec1[epozice],barvaclovek);
	       }
	      epozice=rand()%celkem;
	      gotoxy(epozice+1,pozicey);
	      NAPISZNAK2(retezec1[epozice],VYBERBARVU());
	     }
            goto POZICE;
	}
POZICE:
       textbackground(0);
       gotoxy(pozice+1,pozicey);
     }
    CAS(0);					//bezeni hodin
    if (puvodnicas != cas) {
      cashovoru++; puvodnicas=cas;		//pridani max. 1 vteriny kvuli
						//uzivatelove zmene casu
     }
    if (SETRIC_KLAVESA() == 1) cassetric=time(NULL);    //vynulovani setrice
//    ZJISTI_ALT();				//napsani helpu F?
   } while (kbhit() == 0);
  dalsikec=time(NULL);				//vynulovani kecu
  _AH = 0x10; geninterrupt(0x16);		//cteni znaku z klavesnice
  scankod=_AH; znak=_AL;

  if (scankod == 17 && znak == 0 && skrytepsanizatim == 1) {  //Alt-W
    skrytepsanizatim=0;
    if (psanivetyskryto == 0) {
      psanivetyskryto=1;
      NASTAVBARVU(255,0,0,0); delay(200);
      BARVA(barvaclovek);
      delay(200); NASTAVBARVU(0,0,0,0);
     }
    else {
      psanivetyskryto=0;
      NASTAVBARVU(0,255,0,0); delay(200);
      PREKRESLI_RADEK(insert); gotoxy(pozice+1,pozicey);
      delay(200); NASTAVBARVU(0,0,0,0);
     }
    goto NAPISSTART;
   }
  if (scankod == 119 && znak == 224 && vypnutecheaty != 2)	//Ctrl-Home
   skrytepsanizatim=1;
  else skrytepsanizatim=0;

  switch(scankod) {
    case 28: if (celkem > 0) goto NAPISDAL;	//Levy Enter
             else goto NAPISSTART;
    case 224: if (znak == '\r') if (celkem > 0)	//Pravy Enter
               goto NAPISDAL; else goto NAPISSTART;
/*    case 147:					//Ctrl-Delete
      if (znak != 255) break;
      pomoc=celkem;
      while (retezec1[pozice] == ' ' && pozice < celkem) {
	SMAZZNAK(pozice+1,celkem); celkem--; }
      while (retezec1[pozice] != ' ' && pozice < celkem) {
	SMAZZNAK(pozice+1,celkem); celkem--; }
      while (retezec1[pozice] == ' ' && pozice < celkem) {
	SMAZZNAK(pozice+1,celkem); celkem--; }
      PREKRESLI_RADEK(insert);
      gotoxy(celkem+1,pozicey);
      while (celkem < pomoc) {
	NAPISZNAK2(' ',barvaclovek); pomoc--;
       }
      goto NAPISSTART;*/
    case 14: if (znak == 127) {			//Ctrl-Backspace
	pomoc=pozice; //if (pozice > 0) pozice--;
	while (pozice > 0 && retezec1[pozice-1] == ' ') pozice--;
	while (pozice > 0 && retezec1[pozice-1] != ' ') pozice--;
	NAPIS_GOTOXY(celkem-pomoc+pozice+1,pozicey);
	while (pomoc > pozice) {
	  SMAZZNAK(pomoc,celkem); pomoc--; celkem--;
	  if (psanivetyskryto == 0) NAPISZNAK2(' ',barvaclovek);
         }
       }
      else if (znak == '\b') {			//Backspace
	if (pozice == 0) goto NAPISSTART;
        SMAZZNAK(pozice,celkem); celkem--; pozice--;
        NAPIS_GOTOXY(pozice+1,pozicey); PIPNI(20,8);
       }
      else break;
      if (efekt == 0 && celkem == 1) BARVA(barvaclovek); //zustavajici barva
      PREKRESLI_RADEK(insert); goto NAPISSTART;
    case 16:					//Alt-Q, Ctrl-Q
      if (znak != 0 && znak != 17) break; else goto UKONCENI;
    case 18: if (znak == 5) goto F3;		//Ctrl-E
      if (znak != 0) break;			//Alt-E
      if (textefekty == 0) { textefekty=1; HLASKA("Nyni budu psat efektne.",2); }
      else { textefekty=0; HLASKA("Dobre, prestanu delat ty efekty.",2); }
      goto NAPISSTART;
    case 25: if (znak != 0) break;		//Alt-P
      if (zenapocitac == 0) { zenapocitac=1; ZAPIS_NALADU();
        sprintf(dlouhe,"Dobre, budu tedy %s.",jmenozenapocitac); HLASKA(dlouhe,2);
       }
      else { zenapocitac=0; ZAPIS_NALADU();
        sprintf(dlouhe,"Dobre, budu tedy %s.",jmenomuzpocitac); HLASKA(dlouhe,2);
       }
      goto NAPISSTART;
    case 31: if (znak != 0) break;		//Alt-S
      if (zvuk == 1) { zvuk=0; HLASKA("Vypnul jsem zvuk.",2); }
      else { zvuk=1; HLASKA("Zapnul jsem zvuk.",2); }
      goto NAPISSTART;
    case 33: if (znak != 0) break;		//Alt-F
      if (font == 1) {
        font=0; NAPISHLAVICKOVYRADEK(); NASTAVSPRAVNYFONT();
        HLASKA("Nastavil jsem standardni font.",2);
       }
      else {
        font=1; NAPISHLAVICKOVYRADEK(); NASTAVSPRAVNYFONT();
        HLASKA("Nastavil jsem font Pokydu.",2);
       }
      goto NAPISSTART;
    case 37: if (znak != 0) break;		//Alt-K
      if (znak != 0) break; else goto UKONCENI;
    case 45: if (znak != 0) break;		//Alt-X
UKONCENI: if (HLASKA("Coze? Ty chces ukoncit tenhle program? To snad ne! [A/N] A",6+100) == 'a' || _AL == 'A' || _AL == 'y' || _AL == 'Y' || _AL == '\r') {
        if (llm_enabled == 0 || llm_connected == 0 ||
            LLM_INITIATIVE_SHOW((BYTE *)"goodbye", 0, 0) == 0) {
          EXTRA_VETA(9); BARVA(barvapocitac1); ODPOVED(0);
         }
        else BARVA(barvapocitac1);
        ZAVRISOUBORSKYDAMA("\nProgram Pokyd byl ukoncen funkcni klavesou.",1);
        ZAPIS_INFORMACE_O_VETACH(3,1); STRANA(1); PREDKONEC(); KONEC();
       }
      else if (_AL != 'n' && _AL != 'N' && _AL != 0x1B) goto UKONCENI;
      goto NAPISSTART;
    case 46: if (znak != 0) break;		//Alt-C
      if (zenskyrod == 0) { zenskyrod=1; ZAPIS_NALADU();
        if (zenapocitac == 0) HLASKA("Budu tedy s tebou mluvit jako s zenou.",2);
        else HLASKA("Udelame si tady damskou jizdu.",2);
       }
      else { zenskyrod=0; ZAPIS_NALADU();
        if (zenapocitac == 0) HLASKA("Ted budeme mluvit jako muz s muzem.",2);
        else HLASKA("Nyni s tebou budu mluvit jako s muzem.",2);
       }
      goto NAPISSTART;
    case 47: if (znak != 0) break;		//Alt-V
      VRATDATA(19);
      sprintf(dlouhe,"Pokyd verze 7.0, 4. vydani programu, %d. vydani slovniku (%d.%d.%u)",dlouhe[0],dlouhe[1],dlouhe[2],(unsigned int)dlouhe[3]+2000);
      HLASKA(dlouhe,6); goto NAPISSTART;
    case 59:					//F1
      NAPISHELP();
      goto NAPISSTART;
    case 60:					//F2
      SMAZKURZOR(); CTISOURADNICE(); PRECTIOBRAZOVKU(1);
      spustenipodprogramu=VYBER_PLUGINU();
      SMAZKURZOR(); ZAPISOBRAZOVKU(); VLOZSOURADNICE(); ZAPISCAS();
      NASTAVKURZOR(); goto NAPISSTART;
    case 61:					//F3
      F3: SMAZKURZOR();
      BARVA(barvaclovek);
      strcpy(retezec1,puvretezec);
      pozice=strlen(retezec1); celkem=pozice; NAPIS_GOTOXY(celkem+1,pozicey);
      PREKRESLI_RADEK(insert);
      PIPNI(500,30); goto NAPISSTART;
    case 62:					//F4
      kdejex=wherex(); kdejey=pozicey; SMAZKURZOR(); PRECTIOBRAZOVKU(1);
      NASTAV();
      ZAPISOBRAZOVKU(); NAPISHLAVICKOVYRADEK(); ZAPISCAS();
      ZAPIS_NALADU(); NAPIS_GOTOXY(1,kdejey); if (psanivetyskryto == 0) {
        NAPISRETEZEC(retezec1,barvaclovek); NAPISZNAK(' ',barvaclovek);
        gotoxy(kdejex,kdejey);
       }
      NASTAVKURZOR();
      goto NAPISSTART;
    case 63:					//F5
      PRECTIOBRAZOVKU(0); ODROLUJ(); INTRO(1,pozicey); goto NAPISSTART;
    case 64:					//F6
      goto SETRIC;
    case 65:					//F7
      if (nalada > 0) {
	nalada--; ZAPIS_NALADU();
	naladabody=nalada*15+7;
       }
      else PIPNI(200,50);
      goto NAPISSTART;
    case 66:					//F8
      if (nalada < 4) {
	nalada++; ZAPIS_NALADU();
	naladabody=nalada*15+7;
       }
      else PIPNI(200,50);
      goto NAPISSTART;
    case 67:					//F9
      OBNOV_OBRAZOVKU(0); NASTAVKURZOR(); goto NAPISSTART;
    case 68: goto UKONCENI;			//F10
    case 71:					//Home
      pozice=0; NAPIS_GOTOXY(1,pozicey); goto NAPISSTART;
    case 75:					//doleva
      if (pozice > 0) pozice--;
      NAPIS_GOTOXY(pozice+1,pozicey); goto NAPISSTART;
    case 77:					//doprava
      if (pozice < celkem) pozice++;
      NAPIS_GOTOXY(pozice+1,pozicey); goto NAPISSTART;
    case 79:					//End
      pozice=celkem; NAPIS_GOTOXY(pozice+1,pozicey); goto NAPISSTART;
    case 82:					//Insert
      if (insert == 0) insert=1; else insert=0;
      goto NAPISSTART;
    case 83:					//Delete
      if (celkem > pozice) {
	for (pomoc=pozice; pomoc <= celkem; pomoc++)
	 retezec1[pomoc]=retezec1[pomoc+1];
	PREKRESLI_RADEK(insert);
	NAPIS_GOTOXY(pozice+1,pozicey); celkem--;
       }
      goto NAPISSTART;
    case 92:					//Shift-F9
      NASTAVSOUBOR(1); goto NAPISSTART;
    case 102: if (znak != 0) break;		//Ctrl-F9
      OBNOV_OBRAZOVKU(1); NASTAVKURZOR(); goto NAPISSTART;
    case 106: case 107:				//Alt-F3, Alt-F4
      if (znak != 0) break; else goto UKONCENI;
    case 112:					//Alt+F9
      ZMENMOD(); pozicey=wherey(); goto NAPISSTART;
    case 115:					//Ctrl-vlevo
      if (pozice > 0) pozice--;
      while (pozice > 0 && retezec1[pozice] == ' ')
       pozice--;				//vynechani mezer
      while (pozice > 0) {
	pozice--;
	if (retezec1[pozice] == ' ') { pozice++; goto NAPISSTART; }
       }
      goto NAPISSTART;
    case 116:					//Ctrl-vpravo
      while (pozice < celkem && retezec1[pozice] == ' ') pozice++;
      while (pozice < celkem) {
	pozice++;
	if (retezec1[pozice] == ' ') goto NAPISSTART;
       }
      goto NAPISSTART;
/*    case 147:					//Ctrl-Delete
      while (retezec1[pozice] == ' ' && pozice < celkem) {
	SMAZZNAK(pozice+1,celkem); celkem--;
       }
      while (retezec1[pozice] != ' ' && pozice < celkem) {
	SMAZZNAK(pozice+1,celkem); celkem--;
       }
      BARVA(barvaclovek); PREKRESLI_RADEK(insert);
      goto NAPISSTART;*/
    case 32: if (znak == 'd' || znak == 'D') break; //Alt-Ctrl-LShift-RShift-D
             if (vypnutecheaty == 2) goto NAPISSTART;
             _AH=2; geninterrupt(0x16);
             if (_AL%16 == 15 && znak == 0) {
	       sprintf(dlouhe,"<Debug info> Nahoda: %lu, vtipu: %lu, pamet: %lu B, spusteni: %u",nahodacislo,celkemvtipu,coreleft(),pocetsouboru+1); HLASKA(dlouhe,7);
               sprintf(dlouhe,"Vet: %d, jsem: %d, jsi: %d, nalada: %u",pocetslpamet,pocetjsem,pocetjsi,naladabody); HLASKA(dlouhe,7);
               sprintf(dlouhe,"Slovnik:  verze: %d, vet: %u, extra vet: %u",verzeslovniku,pocetiq-pocetextravet,pocetextravet); HLASKA(dlouhe,7);
	      } goto NAPISSTART;
     }
  if (znak == 0x1B) {					//Esc
    if (delayprocenta == 0 || psanivetyskryto == 1 || textefekty == 0) goto ESCSMAZ;
    SMAZKURZOR(); cislo=strlen(retezec1);
    for (celkem=0; celkem < cislo; celkem++) {
      gotoxy(1,pozicey);
      for (pozice=0; pozice < celkem; pozice++) NAPISRETEZEC("\xdb",barvaclovek);
      NAPISRETEZEC("\xdb",barvaclovek);
      CEKEJ(200/cislo);
     }
    while (celkem > 0) {
      gotoxy(celkem--,pozicey);
      NAPISRETEZEC("\xdb ",barvaclovek);
      CEKEJ(200/cislo);
     }
    ESCSMAZ: retezec1[0]=0; pozice=0; celkem=0; BARVA(barvaclovek);
    NASTAVKURZOR(); PIPNI(30,30); goto NAPISSTART;
   }
  else if (znak < 32) goto NAPISSTART;
  else if (znak > 127) {
    switch(scankod) {
      case 3: case 11: znak='e'; break;
      case 4: znak='s'; break;
      case 5: znak='c'; break;
      case 6: znak='r'; break;
      case 7: znak='z'; break;
      case 8: znak='y'; break;
      case 9: znak='a'; break;
      case 10: znak='i'; break;
      case 26: case 39: znak='u'; break;
      default: goto NAPISSTART;
     }
    prohozeni=0;
   }
  DAL:
  if (celkem < 79 || (insert == 1 && pozice < 79)) {
    if (yz == 1 && prohozeni == 1) switch(znak & 0x5F) {
      case 'Y': znak++; break;					//prohozeni
      case 'Z': znak--; break;					//Y a Z
     } prohozeni=1;
    if (insert == 0) {				//neni-li zapnut insert
      for (pomoc=celkem+1; pomoc > pozice; pomoc--)
       retezec1[pomoc]=retezec1[pomoc-1];
      retezec1[pozice++]=znak; retezec1[++celkem]=0;
     }
    else {
      retezec1[pozice++]=znak; if (pozice > celkem) celkem=pozice;
      retezec1[celkem]=0;
     }
    PREKRESLI_RADEK(insert); NAPIS_GOTOXY(pozice+1,pozicey);
    PIPNI((scankod+5)*50,5);
   }
  else PIPNI(900,30);				//pipnuti na konci radku
  goto NAPISSTART;
  NAPISDAL:
  retezec1[celkem]=0;
  strcpy(puvretezec,retezec1);
  if (kydy != NULL) {
    if (psanivetyskryto == 0) fprintf(kydy,"C: %s\n",retezec1);
    else fprintf(kydy,"C <veta skryta>\n",retezec1);
   }
  PIPNI(200,30);
  SMAZKURZOR(); textcolor(barvaclovek);
  if (psanivetyskryto == 0) cprintf("\r%s",retezec1); pocetvet++; pocetuzivvet++;
  psanivetyskryto=0;
//  _AX = 0x8301; geninterrupt(0x15);		//vypnuti casovace
 }

/* Rutina INT0 - viz implementace a nazvy promennych (konvence Pokyd). */
void interrupt INT0(void) {
unsigned *p=(unsigned *)MK_FP(_SS,_BP+0x12);
  *p+= 2;			                //posun za instrukci DIV
  int0=1;
 }

/* Rutina SAMOHLASKA - viz implementace a nazvy promennych (konvence Pokyd). */
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

/* Rutina VRAT_5_PAD - viz implementace a nazvy promennych (konvence Pokyd). */
void VRAT_5_PAD(void) {
BYTE pomoc[17],posledni,poslznak,prposlznak;
  strcpy(jmenocloveka5pad,jmenocloveka);
  if (slovenstina == 1) return;
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
              _BH=strlen(jmenocloveka5pad);
              if (_BH != poslznak) poslznak='e';
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
