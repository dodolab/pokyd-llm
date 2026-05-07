BYTE VRATPOZICI(void);
void NAPISZNAK(BYTE znak,BYTE barva);
void UDELEJ_POZADI(void);
void SKRYJ_POZADI(void);
BYTE CTIZNAK(void);
BYTE CTIBARVU(void);
void NAPISHODINY(signed char koliky);
void PRIDEJ_CAS(void);
void UBER_CAS(void);
BYTE NASTAV_DATUM(void);
BYTE NASTAV_CAS(void);
void NAPISDEN(void);

BYTE VRATPOZICI(void) {
BYTE mistoy,mistox,kolik;
  switch(pozicehodiny) {
    case 0: mistoy=0; mistox=14; kolik=2; break;
    case 1: mistoy=0; mistox=17; kolik=2; break;
    case 2: mistoy=0; mistox=20; kolik=4; break;
    case 3: mistoy=1; mistox=14; kolik=2; break;
    case 4: mistoy=1; mistox=17; kolik=2; break;
    case 5: mistoy=1; mistox=20; kolik=2; break;
   }
  gotoxy(mistox,kdejey+mistoy);
  return(kolik);
 }

void NAPISZNAK(BYTE znak,BYTE barva) {
  _BL = barva; _AL = znak; _CX = 1; _BH = 0; _AH = 9; geninterrupt(0x10);
  gotoxy(wherex()+1,wherey());
 }

void UDELEJ_POZADI(void) {
BYTE pozice=0,y=wherey(),kolik;
  kolik=VRATPOZICI();
  while (pozice < kolik) {
    NAPISZNAK(CTIZNAK(),32+14);
    pozice++;
   }
  gotoxy(1,y);
 }

void SKRYJ_POZADI(void) {
BYTE pozice=0,y=wherey(),kolik;
  kolik=VRATPOZICI();
  while (pozice < kolik) {
    NAPISZNAK(CTIZNAK(),14);
    pozice++;
   }
  gotoxy(1,y);
 }

BYTE CTIZNAK(void) {
  _AH = 8; _BH = 0; geninterrupt(0x10);
  return(_AL);
 }

BYTE CTIBARVU(void) {
  _AH = 8; _BH = 0; geninterrupt(0x10);
  return(_AH);
 }

void NAPISHODINY(signed char koliky) {
BYTE celkem=strlen(hodiny),pozice=0,barva;
  gotoxy(14,wherey()+koliky);			//algebraicky soucet
  while (pozice < celkem) {
    barva=CTIBARVU()>>4;
    NAPISZNAK(hodiny[pozice++],(barva<<4)+14);
   }
 }

void PRIDEJ_CAS(void) {
  switch(pozicehodiny) {
    case 0: den++; if (NASTAV_DATUM() != 0) den--; break;
    case 1: mesic++; if (NASTAV_DATUM() != 0) mesic--; break;
    case 2: rok++; if (NASTAV_DATUM() != 0) rok--; break;
    case 3: hodina++; if (NASTAV_CAS() != 0) hodina--; break;
    case 4: minuta++; if (NASTAV_CAS() != 0) minuta--; break;
    case 5: vterina++; if (NASTAV_CAS() != 0) vterina--; break;
   }
 }

void UBER_CAS(void) {
  switch(pozicehodiny) {
    case 0: den--; if (NASTAV_DATUM() != 0) den++; break;
    case 1: mesic--; if (NASTAV_DATUM() != 0) mesic++; break;
    case 2: rok--; if (NASTAV_DATUM() != 0) rok++; break;
    case 3: hodina--; if (NASTAV_CAS() != 0) hodina++; break;
    case 4: minuta--; if (NASTAV_CAS() != 0) minuta++; break;
    case 5: vterina--; if (NASTAV_CAS() != 0) vterina++; break;
   }
 }

BYTE NASTAV_DATUM(void) {
  _AH = 0x2B; _CX = rok; _DH = mesic; _DL = den; geninterrupt(0x21);
  return(_AL);
 }

BYTE NASTAV_CAS(void) {
  _AH = 0x2D; _CH = hodina; _CL = minuta; _DH = vterina; _DL = 6; geninterrupt(0x21);
  return(_AL);
 }

void NAPISDEN(void) {
BYTE jmeno[8];
  switch(jmenodne) {
    case 0: strcpy(jmeno," Nedele"); break;
    case 1: strcpy(jmeno,"Pondeli"); break;
    case 2: strcpy(jmeno," Utery "); break;
    case 3: strcpy(jmeno," Streda"); break;
    case 4: strcpy(jmeno,"Ctvrtek"); break;
    case 5: strcpy(jmeno," Patek "); break;
    case 6: strcpy(jmeno," Sobota"); break;
   }
  gotoxy(28,kdejey+1); textcolor(11);
  cprintf("%s",jmeno);
 }