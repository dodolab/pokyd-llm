#include <stdio.h>

unsigned long delka,celkdelka;
unsigned char znak,soucet1,soucet2;

#define POCET_SOUBORU 18

FILE *f1,*f2;

void TEST(unsigned char *soubor) {
unsigned char celkem=strlen(soubor),pozice,kodnazev[30];
  for (pozice=0; pozice < celkem; pozice++) kodnazev[pozice]=soubor[pozice]^'K';
  fwrite(kodnazev,celkem,1,f2); putc('K',f2);	//zapsani jmena souboru
  if ((f1=fopen(soubor,"rb")) == NULL) { fwrite("\x0\x0\x0\x0",4,1,f2); return; }
  fseek(f1,0,SEEK_END); celkdelka=ftell(f1); fseek(f1,0,SEEK_SET);
  putc((celkdelka>>16)^75,f2);
  putc((celkdelka<<16>>24)^75,f2);
  putc((celkdelka<<24>>24)^75,f2);				//zapis delku
  soucet1=0; soucet2=0;
  for (delka=0; delka < celkdelka; delka++) {
    znak=getc(f1);
    soucet1^=znak; soucet1+=znak;
    soucet2+=znak;
   }
  fclose(f1);
  putc(soucet1,f2);
  putc(soucet2,f2);
  printf(".");
 }

void main() {
f2=fopen("TESTPKD.TST","wb");
putc('7'^'K',f2);
putc('0'^'K',f2);
putc('0'^'K',f2);	//verze 7.00
putc(POCET_SOUBORU^'K',f2);

TEST("TESTPKD.EXE");
TEST("TESTPKD.TXT");
TEST("SLOVNIK.DAT");
TEST("POKYD.EXE");
TEST("POKYD.ICO");
TEST("POKYD.TXT");
TEST("VERZE.TXT");
TEST("PLUGINY\\COOKEDIT.EXE");
TEST("PLUGINY\\CAS.EXE");
TEST("PLUGINY\\HAD.EXE");
TEST("PLUGINY\\HONICKA.EXE");
TEST("PLUGINY\\KALKUL.EXE");
TEST("PLUGINY\\NAJDIME.EXE");
TEST("PLUGINY\\NAJDIME.KAL");
TEST("PLUGINY\\NOKOMAR.EXE");
TEST("PLUGINY\\PALBA.EXE");
TEST("PLUGINY\\PISMENA.EXE");
TEST("PLUGINY\\SILNICE.EXE");
TEST("PLUGINY\\PLUGINY.TXT");
fclose(f2);
f1=fopen("TESTPKD.TST","rb");
fseek(f1,0,SEEK_END); celkdelka=ftell(f1); fseek(f1,0,SEEK_SET);
soucet1=0; soucet2=0;
for (delka=0; delka < celkdelka; delka++) {
  znak=getc(f1);
  soucet1^=znak; soucet1^='K'; soucet1+=znak;
  soucet2+=znak;
 }
fclose(f1);
f2=fopen("TESTPKD.TST","ab"); fseek(f2,0,SEEK_END);
putc(soucet1,f2); putc(soucet2,f2);
fclose(f2);
}