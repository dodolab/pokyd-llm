void interrupt INT24(void) {
  chybaint24=1;
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
    if ((soub[cislo] = *uk2++) == 0) break;
   }
  pozice=strlen(soub)-1;
  while (soub[pozice] != '\\') {
    if (pozice == 0) { soub[0]=0; return; }	//zabraneni blbostem v soub[]
    else pozice--;
   }
  soub[pozice+1]=0;
  strcpy(rootsoubor,soub);
  strcpy(assetssoubor,soub);
  strcat(assetssoubor,"assets\\");
  strcat(assetssoubor,souborcat);
  strcat(rootsoubor,souborcat);
  strcpy(soub,rootsoubor);
  assettest=fopen(assetssoubor,"rb");
  if (assettest != NULL) {
    fclose(assettest);
    strcpy(soub,assetssoubor);
   }
 }

void TESTUJ(BYTE *soubor) {
  chybaint24=0;
  textcolor(10); cprintf("\r%s        ",soubor);
  SOUBOR(soubor);
  if ((test=fopen(soub,"rb")) == NULL) {
    textcolor(12); cprintf("\rSoubor %s nenalezen nebo nejde otevrit!\n\r",soubor);
     fseek(hodnoty,5,SEEK_CUR); pocetchyb++; goto KONEC;
   }
  fseek(test,0,SEEK_END); celkemdelka=ftell(test); fseek(test,0,SEEK_SET);
  delka=(getc(hodnoty)^75);
   delka<<=16;
  pomoc=(getc(hodnoty)^75); delka+=pomoc<<8;
  delka+=getc(hodnoty)^75;					//cti delku
  if (delka != celkemdelka) {
    textcolor(12); cprintf("\rSoubor %s nema spravnou delku (%lu misto %lu)!\n\r",soubor,celkemdelka,delka);
    fseek(hodnoty,2,SEEK_CUR); pocetchyb++; goto KONEC;
   }
  kontrola1=0; kontrola2=0;
  for (delka=0; delka < celkemdelka; delka++) {
    znak=getc(test);
    kontrola1^=znak;				//vyxoruj znakem
    kontrola1+=znak;				//a pricti ho
    kontrola2+=znak;
   }
  pomoc=ftell(hodnoty);
  if (chybaint24 == 1) {
    textcolor(12); cprintf("\rSoubor %s nejde precist!\n\r",soubor);
    pocetchyb++;
    fseek(hodnoty,pomoc+2,SEEK_SET);
   }
  else if (kontrola1 != getc(hodnoty) || kontrola2 != getc(hodnoty)) {
    textcolor(12); cprintf("\rSoubor %s nema spravny kontrolni soucet!\n\r",soubor);
    pocetchyb++;
    fseek(hodnoty,pomoc+2,SEEK_SET);
   }
  KONEC: fclose(test);
 }
