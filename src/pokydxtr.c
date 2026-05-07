#define POCET_BAJTU 21

#include <stdio.h>

FILE *f;

unsigned char pamet[120],nastaveni[3000],retezec[80],pozice,soucet,klic,znak;
unsigned short rok;
unsigned long seriove,celkemdelka,pozice1;

void main(void) {
if ((f=fopen("POKYD.XTR","rb")) == NULL) { printf("Soubor POKYD.XTR nenalezen!"); return; }
fseek(f,0,SEEK_END); celkemdelka=ftell(f); fseek(f,0,SEEK_SET);
fread(pamet,201,1,f);
if (strcmp(pamet,"Extra soubor k Pokydu pro nejvernejsi uzivatele - nerozsirovat, ale poslat na iqpokyd@kyblsoft.cz spolecne se svym jmenem a mistem bydliste. Za to ti autor posle specialni verzi Pokydu s venovanim.\r\n") != 0) {
  printf("*** Neplatny identifikator! (ze by jeste stary e-mail?)\n");
  return;
 }
else printf("Identifikator OK\n");

klic=getc(f);
fread(pamet,POCET_BAJTU,1,f); soucet=0;
for (pozice=0; pozice < POCET_BAJTU; pozice++) {
  pamet[pozice]^=klic;
  pamet[pozice]^='K'+pozice;
  pamet[pozice]-=pozice;
  pamet[pozice]^=pozice;
  soucet^=pamet[pozice]; soucet+=pamet[pozice];
 }

pozice=0;
do {
  znak=getc(f);
  znak-=klic; znak^=pozice; znak^=klic;
  soucet^=znak; soucet+=znak;
  retezec[pozice]=znak; pozice++;
 } while (znak != 0);

celkemdelka=celkemdelka-ftell(f)-1;

fread(nastaveni,celkemdelka,1,f);
nastaveni[celkemdelka]=0;
for (pozice1=0; pozice1 < celkemdelka; pozice1++) {
  nastaveni[pozice1]-=klic;
  nastaveni[pozice1]^=(unsigned char)pozice1;
  nastaveni[pozice1]^=klic;
  soucet^=nastaveni[pozice1]; soucet+=nastaveni[pozice1];
 }
if (soucet != getc(f)) printf("*** Neplatny kontrolni soucet!\n");
else printf("Kontrolni soucet OK\n");
printf("Verze: %d.%d",pamet[0],pamet[1]);
if (pamet[2] == 0) printf(" - cheat nepouzit\n"); else printf(" - POUZIT CHEAT!\n");
printf("Datum: %d.%d.",pamet[3],pamet[4]);
rok=(pamet[5]<<8)+pamet[6]; printf("%u  ",rok);
printf("%d:%02d\n",pamet[7],pamet[8]);
printf("Nahodna cisla: %d,%d,%d,%d\n",pamet[9],pamet[10],pamet[11],pamet[12]);
for (seriove=0,pozice=13; pozice < 17; pozice++) {
  seriove<<=8; seriove+=pamet[pozice];
 }
printf("Seriove cislo disku C: %lu\n",seriove);
printf("Pocet vet celkem: %u\n",(unsigned short)((pamet[17]<<8)+pamet[18]));
printf("Pocet vet, ktere rekl clovek: %u\n",(unsigned short)((pamet[19]<<8)+pamet[20]));
printf("Dekodovaci klic: %d\n\n",klic);

printf("Posledni veta: %s\n\n",retezec);

if (nastaveni[0] != 0) {
  printf("Soubor s nastavenim:\n");
  printf(nastaveni);
 }
else printf("Soubor s nastavenim nebyl zapsan.\n");
fclose(f);
}
