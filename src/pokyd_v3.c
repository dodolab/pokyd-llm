/* Tento zdrojovy kod je pod licenci GNU/GPL. Muzete ho pouzit k vlastni
   potrebe, ale nesmite jej ani programy zalozene na tomto kodu vyuzit komercne!

   Jedna se o zdrojovy kod programu Pokyd (http://iqpokyd.kyblsoft.cz)
   od Alese Jandy, aktivne vyvijeneho 1999 - 2002
*/

/*
 * pokyd_v3.c - nizkourovnovy vystup na obrazovku (INT 10h), barvy, kurzor, NAPISRETEZEC.
 * Veskery uzivatelsky viditelny text prochazi zde pres BIOS AH=09 / vlastni NAPISZNAK.
 */

/* Dekorativni cary uvodni obrazovky INTRO. */
void INTRO_CARY(void) {
  INTRO_CARA(18,61,80);
  INTRO_CARA(19,37,67);
  INTRO_CARA(20,21,51);
  INTRO_CARA(21,13,43);
  INTRO_CARA(22,8,38);
  INTRO_CARA(23,5,35);
  INTRO_CARA(24,2,32);
  INTRO_CARA(25,1,31);
 }

/* Odhad volne konvencni pameti pro kontroly pluginu / uvodu. */
DWORD VOLNAPAMET(void) {
DWORD kolik=coreleft();
  if (kolik > 65536) kolik=65536;
  if (kolik > 16) kolik-=16;			//4 otevrene soubory
  return(kolik);
 }

/* Obnovi graficky/textovy rezim a prekresli UI (volano z prikazu "obnov obrazovku"). */
void OBNOV_OBRAZOVKU(BYTE jak) {
BYTE mod;
  CTISOURADNICE(); SMAZKURZOR(); PRECTIOBRAZOVKU(0); 
  if (jak == 1) for (mod=0; mod < 0xF; mod++) textmode(mod);
  textmode(0x10);
  SMAZOBRAZOVKU(1); NASTAVSPRAVNYMOD(); ZAPISOBRAZOVKU();
  NAPISHLAVICKOVYRADEK(); VLOZSOURADNICE(); 
  if (jak == 1) { DATUM(1); CAS(1); KALENDAR(1); ZAPIS_NALADU(); } else ZAPISCAS();
 }

/* Vyplni nazev dne podle DOS DATA (INT 21h AH=2Ah); volitelne velke pocatecni pismeno. */
void VRATNAZEVDNE(BYTE *pointer,BYTE velke_pismeno) {
  memset(&pokyd_regs, 0, sizeof(pokyd_regs));
  _AH = 0x2A; geninterrupt(0x21);
  switch(_AL) {
    case 1: strcpy(pointer,"pondeli"); break;
    case 2: strcpy(pointer,"utery"); break;
    case 3: strcpy(pointer,"streda"); break;
    case 4: strcpy(pointer,"ctvrtek"); break;
    case 5: strcpy(pointer,"patek"); break;
    case 6: strcpy(pointer,"sobota"); break;
    case 0: strcpy(pointer,"nedele"); break;
   }
  if (velke_pismeno == 1) pointer[0]&=0x5F;
 }

/* Precte znak a atribut pod kurzorem (INT 10h AH=08). */
BYTE CTIZNAK(void) {
  memset(&pokyd_regs, 0, sizeof(pokyd_regs));
  _AH = 8; _BH = 0; geninterrupt(0x10);
  return(_AL);
 }

/* Zkratka: pozice kurzoru + barevny retezec. */
void PIS(BYTE x,BYTE y,BYTE *text,BYTE barva) {
  gotoxy(x,y);
  NAPISRETEZEC(text,barva);
 }

/* Celoobrazova napoveda (HELPF1). */
void NAPISHELP(void) {
  PRECTIOBRAZOVKU(1); CTISOURADNICE(); HELPF1(14);
  NASTAVSPRAVNYMOD(); ZAPISOBRAZOVKU(); VLOZSOURADNICE(); ZAPISCAS();
 }

/* Aktualizuje cas v hlavicce a naladu. */
void ZAPISCAS(void) {
  CAS(1); ZAPIS_NALADU();
 }

/* Nahodna barva (intro efekty). */
BYTE VYBERBARVU(void) {
  return((rand()%15)+1);
 }

/* Skryje textovy kurzor (INT 10h AH=01, CX=2000h). */
void SMAZKURZOR(void) {
  memset(&pokyd_regs, 0, sizeof(pokyd_regs));
  _AH = 1;
  _CX = 0x2000;
  geninterrupt(0x10);		//smaz kurzor
 }

/* Plny blok kurzor. */
void PLNYKURZOR(void){
  memset(&pokyd_regs, 0, sizeof(pokyd_regs));
  _AH = 1;
  _CX = 0x1F;
  geninterrupt(0x10);
 }

/* Aktualni barva znaku (AH z INT 10h AH=08). */
BYTE CTIBARVU(void) {
  memset(&pokyd_regs, 0, sizeof(pokyd_regs));
  _AH = 8; _BH = 0; geninterrupt(0x10);
  return(_AH);
 }

/* Programovatelna paleta VGA (legacy INT 10h AX=1010h). */
void NASTAVBARVU(BYTE r,BYTE g,BYTE b,BYTE barva) {
  memset(&pokyd_regs, 0, sizeof(pokyd_regs));
  _BX = barva; _DH = r; _CH = g; _CL = b;
  _AX = 0x1010; geninterrupt(0x10);
 }

/* Vyplni radek mezerami dane barvy a nastavi textcolor (stdio + BIOS). */
void BARVA(BYTE barva) {
  gotoxy(1,wherey());
  memset(&pokyd_regs, 0, sizeof(pokyd_regs));
  _AH = 9;
  _BH = 0; _AL = ' '; _CX = 80; _BL = barva;
  geninterrupt(0x10);
  textcolor(barva);
 }

/* BIOS casovac 18.2 Hz (INT 1Ah AH=0). */
DWORD CAS18(void) {
DWORD cas;

  memset(&pokyd_regs, 0, sizeof(pokyd_regs));
  _AH=0;
  geninterrupt (0x1A);
  cas=_CX;
  cas<<= 16;
  cas+=_DX;
  return(cas);
 }

/*
 * Pocet radku textove obrazovky - zjednoduseno (fixni 25r.) kvuli DOSBox/Watcom,
 * aby neblokovalo ctenim BIOS neplatnych radku.
 */
BYTE TESTSTRANY(void) {
  /* Legacy cursor probing can hang in some DOSBox/Open Watcom setups.
     Default to 25-line mode (index 24) and let runtime mode switch logic
     move to 50 rows later when explicitly requested. */
  return(24);
 }

/* Vypis jednoho znaku pres INT 10h AH=09 (barva v BL). */
void NAPISZNAK(BYTE znak,BYTE barva) {
  memset(&pokyd_regs, 0, sizeof(pokyd_regs));
  if (znak != 0) _AL = znak;
  else _AL = ' ';				//vymena nuly za mezeru
  _CX = 1; _BH = 0; _BL = barva; _AH = 9;
  geninterrupt(0x10);
 }

/* Jeden znak a posun kurzoru doprava. */
void NAPISZNAK2(BYTE znak,BYTE barva) {
  NAPISZNAK(znak,barva);
  gotoxy(wherex()+1,wherey());
 }

/* Prepise pozici mezerou. */
void VYMAZZNAKNAOBR(BYTE x,BYTE y) {
  gotoxy(x,y); NAPISZNAK(' ',0);
 }

/*
 * Vypis retezce znak po znaku - pouziva i kody z VRATDIAKRITIKU (ceske sloty ve fontu).
 */
void NAPISRETEZEC(BYTE *retezec,BYTE barva) {
BYTE pozice=0,celkempozice=strlen(retezec);
  while (pozice < celkempozice) {
    NAPISZNAK2(retezec[pozice],barva);
    pozice++;
   }
 }

/* ASCII A-Z na a-z v miste. */
void PREVED_NA_MALA(BYTE *co) {
BYTE pozice;
  for (pozice=0; co[pozice] != 0; pozice++) {
    if (co[pozice] >= 'A' && co[pozice] <= 'Z') co[pozice]+=32;
   }
 }
