void PREVED_RETEZEC_NA_JMENO(void) {
BYTE pozice;
  for (pozice=0; retezec[pozice] != 0; pozice++) {
    switch(retezec[pozice]) {
      case '*': jmeno[pozice]=11; break;
      case '%': jmeno[pozice]=12; break;  //prevod na muzske a zenske znaky
      default: jmeno[pozice]=retezec[pozice]; break;
     }
   }
  jmeno[pozice]=0;
 }

void SOUBOR(BYTE *souborcat) {
char pozice,cislo;
int *ukazatel;
char *uk2;
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
  strcat(soubor,souborcat);
 }

void NAPISJMENO(void) {
BYTE pozice;
  textcolor(10);
  for (pozice=0; jmena[pozicejmeno+pozice] != 0; pozice++) {
    switch(jmena[pozicejmeno+pozice]) {
      case '': pozice++; textcolor(11); break;
      case '': pozice++; textcolor(12); break;
      case '#': pozice++; textcolor(15); break;
      case '~': pozice++; textcolor(7); break;
      case ',': textcolor(10); break;
     }
    cprintf("%c",jmena[pozicejmeno+pozice]);
   }
 }

void NAJDIJMENO(void) {
BYTE pozicemesic=mesic;
WORD poz=0;
  if (pozicemesic > 1) { poz+=31; pozicemesic--; }
  if (pozicemesic > 1) { poz+=29; pozicemesic--; }
  if (pozicemesic > 1) { poz+=31; pozicemesic--; }
  if (pozicemesic > 1) { poz+=30; pozicemesic--; }
  if (pozicemesic > 1) { poz+=31; pozicemesic--; }
  if (pozicemesic > 1) { poz+=30; pozicemesic--; }
  if (pozicemesic > 1) { poz+=31; pozicemesic--; }
  if (pozicemesic > 1) { poz+=31; pozicemesic--; }
  if (pozicemesic > 1) { poz+=30; pozicemesic--; }
  if (pozicemesic > 1) { poz+=31; pozicemesic--; }
  if (pozicemesic > 1) poz+=30;

  poz+=den; pozicejmeno=0;
  while (--poz > 0) pozicejmeno+=strlen(jmena+pozicejmeno)+1;
 }

void POSUNJMENA(int okolik) {
  while (okolik < 0) {
    if (den == 1) {
      if (mesic == 2 || mesic == 4 || mesic == 6 || mesic == 8 || mesic == 9 || mesic == 11) {
        den=31; mesic--;
       }
      else if (mesic == 1) {
        den=31; mesic=12;
       }
      else if (mesic == 3) {
        if ((rok%4) == 0) { den=29; mesic=2; }
        else { den=28; mesic=2; }
       }
      else {
        den=30; mesic--;
       }
     }
    else den--;
    okolik++;
   }
  while (okolik > 0) {
    if (den == 30 && (mesic == 4 || mesic == 6 || mesic == 9 || mesic == 11)) {
      den=1; mesic++;
     }
    else if (den == 31) {
      den=1; if (mesic == 12) mesic=1; else mesic++;
     }
    else if (den == 29 && mesic == 2) { den=1; mesic=3; }
    else if (den == 28 && mesic == 2) {
      if ((rok%4) == 0) den++;
      else { den=1; mesic=3; }
     }
    else den++;
    okolik--;
   }
  NAJDIJMENO();
 }

WORD POCET_NALEZENYCH_JMEN(void) {
WORD poz,nalezeno=0;

  pozicejmeno=0; PREVED_RETEZEC_NA_JMENO();
  for (poz=0; poz < 366; poz++) {
    strcpy(jmeno2,jmena+pozicejmeno);
    strupr(jmeno2);
    if (strstr(jmeno2,jmeno) != NULL) nalezeno++;	//nalezeno jmeno
    pozicejmeno+=strlen(jmena+pozicejmeno)+1;
   }
  return(nalezeno);
 }

BYTE JELI_CISLICE(BYTE znak) {
  if (znak >= '0' && znak <= '9') return(1);
  else return(0);
 }