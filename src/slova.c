#define konecne 1
					//1 - konecne, 0 - ladeni
#define zakodovani 1

#include <stdio.h>
#include <dos.h>
#include <stdlib.h>
#include <conio.h>
#include <string.h>

#define POCET_DAT 19
#define POCET_EXTRA_VET 18

unsigned long delka=0,D,radky,poziceradek=1,pocetvet=0,Dvety,konechlavicky;
unsigned long rodx=0,nahodax=0,koliknerodu=0,puvodnipozice,poslednipozice;
unsigned long pozicepredextra,pocetslov=0,pocetpodmslov=0,pocetzenvet=0;
unsigned long podmoperatoru=0;
unsigned short delkaradku,mezera,datamisto[POCET_DAT],pozicenaradku;
unsigned char znak,typ,hodnota,znamenko,pocetdat,zavorka,uvozovky=0;
unsigned char prvniradek,enter=0,delkaslova,poddelkaslova,maxdelkaslova=0,maxpoddelkaslova=0;
unsigned char nejslovo[80],slovo[80],zavorky,kolikatyznak,operator;

FILE *pkd,*slv,*sl1,*pkdx;

void main() {
printf("1/2");
pkd=fopen("SLOVA.PKD","rb");
slv=fopen("SLOVNIK.DAT","wb"); fclose(slv);
slv=fopen("SLOVNIK.DAT","rb+");
pkdx=fopen("POKYDX.PKD","rb");
fseek(pkd,0,SEEK_END); D=ftell(pkd); fseek(pkd,0,SEEK_SET);

putc('7'^'K',slv); putc('0'^'K',slv);	//verze 7.0
putc(6^'K',slv);			//verze slovniku: 6
fwrite("\x0\x0\x0",3,1,slv);		//misto pro odkazy na data a vety
fwrite(" Soubor s odpovedmi a daty k programu Pokyd v7.0 - KYBLSoft 2005 ",65,1,slv);
konechlavicky=ftell(slv);
fwrite("",3+POCET_DAT*2,1,slv);    //vynechani mista pro odkazy na extra a data
puvodnipozice=ftell(slv); fseek(slv,3,SEEK_SET);
putc(konechlavicky,slv); fseek(slv,puvodnipozice,SEEK_SET);

for (pocetdat=0; pocetdat < POCET_DAT; pocetdat++) {
  while (getc(pkd) != '\r') delka++;		//vynechat komentar
  fseek(pkd,3,SEEK_CUR); delka+=3;		//vynechat 2 entery
  poziceradek+=2;
  datamisto[pocetdat]=ftell(slv)-konechlavicky;		//zapamatuj si pozice
  while ((znak=getc(pkd)) != 0 || znak != 2) {			//zapis data
    putc(znak,slv); delka++;
    if (znak == '\r') enter=1;
    else if (znak == '\n' && enter == 1) poziceradek++;
    else enter=0;
   }
  putc(0,slv);					//pripsat nulu na konec
  fseek(pkd,4,SEEK_CUR); delka+=4;		//vynechat 2 entery
  poziceradek+=2;
 }

fseek(slv,konechlavicky,SEEK_SET);
putc(POCET_EXTRA_VET,slv); fwrite("\x0\x0",2,1,slv);
for (pocetdat=0; pocetdat < POCET_DAT; pocetdat++) {
  putc(datamisto[pocetdat]>>8,slv);
  putc(datamisto[pocetdat]%256,slv);
 }

fseek(slv,0,SEEK_END);				//skoc zpatky na konec

poslednipozice=ftell(slv); fseek(slv,4,SEEK_SET);
putc(poslednipozice>>8,slv); putc(poslednipozice%256,slv);
fseek(slv,poslednipozice,SEEK_SET);
pozicepredextra=poslednipozice;

for (pocetdat=0; pocetdat < POCET_EXTRA_VET; pocetdat++) {
  puvodnipozice=ftell(slv); fwrite("\x0\x0",2,1,slv); //mista na poz. dalsi vety
  while ((znak=getc(pkd)) != '\r');		//komentar
  poziceradek++;
  fseek(pkd,1,SEEK_CUR); delka++;		//vynechat 1 enter
  switch(typ=getc(pkd)) {
    case '5': putc('5',slv); rodx++; break;
    case '6': putc('6',slv); nahodax++; break;
    default: printf("\rChyba typu na radku %lu!\n",poziceradek); goto KONEC;
   }
  hodnota=10;						//pocatecni hodnota
  switch(getc(pkd)) {
    case '+': hodnota-=getc(pkd)-'0'; break;		//hodnoty jsou
    case '-': hodnota+=getc(pkd)-'0'; break;		//naopak
    default: printf("\rChyba hodnoty na radku %lu\n",poziceradek); goto KONEC;
   }
  putc(hodnota,slv); delka+=2;
  pocetvet+=10;
  fseek(pkd,2,SEEK_CUR); delka+=3; poziceradek++;		//dalsi radek
  for (radky=0; radky < 10; radky++) {
    kolikatyznak=1;
    if (radky == 7) {			//dalsi radek a psani real-time kecu
      fseek(pkd,2,SEEK_CUR); delka+=2; poziceradek++;
     }
    mezera=1; delkaslova=0;
    while ((znak=getc(pkd)) != '\r') {
      if (mezera == 1) {
        if (znak == ' ') {
          printf("\rMezera na zacatku vety na radku %lu!\n",poziceradek);
          goto KONEC;
         }
        if (znak == 255 && typ == '5') koliknerodu++;
        if ((znak == 0 || znak == 255) && typ == '6') {
          printf("\rNa radku %lu ma byt typ 5, ne 6!\n",poziceradek-1);
          goto KONEC;
         }
        mezera=0;
       }
      if ((kolikatyznak == 1 && typ == '6') || (kolikatyznak == 2 && typ == '5'))
       if ((znak < 'A' || znak > 'Z') && znak != '*' && znak != '#') {
         printf("\rVeta nezacina platnym znakem na radku %lu!\n",poziceradek);
         goto KONEC;
        }
      if (znak == ' ') {
        slovo[delkaslova]=0; if (maxdelkaslova < delkaslova) {
          maxdelkaslova=delkaslova; strcpy(nejslovo,slovo);
         }
        delkaslova=0; pocetslov++;
       }
      else if ((znak >= 'A' && znak <= 'Z') || (znak >= 'a' && znak <= 'z'))
       slovo[delkaslova++]=znak;
      else delkaslova=0;
      if (znak == '') { pocetzenvet++; pocetslov++; kolikatyznak=0; }
      putc(znak,slv); delka++; Dvety++; kolikatyznak++;
     }
    if (maxdelkaslova < delkaslova) {
      maxdelkaslova=delkaslova; strcpy(nejslovo,slovo);
     }
    pocetslov++;				//posledni slovo
    if (znak != '.' && znak != '!' && znak != '?' && znak != ':') { printf("\rVeta nekonci platnym operatorem na radku %lu !",poziceradek); goto KONEC; }
    znak=getc(pkd); delka+=2; poziceradek++;		//dalsi radek
    if (typ == '5') Dvety--;			//vynechani pocatecniho znaku
    putc(0,slv);					//nula na konci
   }
  poslednipozice=ftell(slv); fseek(slv,puvodnipozice,SEEK_SET);
  putc((poslednipozice-puvodnipozice-2)>>8,slv); putc((poslednipozice-puvodnipozice-2)%256,slv);
  fseek(slv,poslednipozice,SEEK_SET);
  fseek(pkd,2,SEEK_CUR); poziceradek++; delka+=2;     //dalsi (prazdny) radek
 }

do {
  /* preserve original structure; value was only used for debugging */
  D=ftell(pkd);
  puvodnipozice=ftell(slv);
  fwrite("\x0\x0\x0\x0",4,1,slv);		//misto na zapsani pozici
  pozicenaradku=0;
PRVNI:
  prvniradek=1; znak=getc(pkd);
  if (znak != '(' && znak != 'e') {
    if (znak == '~') goto SCITANI;
    else printf("Podminka neni na radku %lu!",poziceradek); exit(0);
   }
PODMINKA:
  if (prvniradek == 1) {
    pozicenaradku++;
    if (znak == '(') zavorky=1; else zavorky=0;
    if (znak == 'e') { putc('e',slv); goto PRVNI; }
    operator=2;
    podmoperatoru++;		//na radku je o 1 vic nez je pocet & a |
   }
  pozicenaradku=1;
  while ((znak=getc(pkd)) != '\r') {
    pozicenaradku++;
    if (znak == '~')
     goto SCITANI;		//konec souboru
    if (znak == '\"') {			// "slovo"  ->  slovo\x0
      if (uvozovky == 0) { pocetpodmslov++; poddelkaslova=0; uvozovky=1; goto PODMINKAKONEC; }
      else { znak=0; uvozovky=0; if (poddelkaslova > maxpoddelkaslova) maxpoddelkaslova=poddelkaslova; }
     }
    else poddelkaslova++;

    if (prvniradek == 1) {	//kontrola spravnosti operatoru & (0) a | (1)
      if (znak == '&') {
        if (operator == 1) {			// |
          printf("\rSpatne pouziti operatoru & a | na radku %lu, pozice %u!",poziceradek,pozicenaradku); exit(0);
         }
        else operator=0;
        podmoperatoru++;
       }
      if (znak == '|') {
        if (operator == 0) {			// &
          printf("\rSpatne pouziti operatoru & a | na radku %lu, pozice %u!",poziceradek,pozicenaradku); exit(0);
         }
        else operator=1;
        podmoperatoru++;
       }
     }

    if (uvozovky == 0) {
      if (znak == '(') { zavorky++; operator^=1; }
      else if (znak == ')') {
        if (zavorky > 0) zavorky--;
        else { printf("\rMoc pravych zavorek na radku %lu!",poziceradek); exit(0); }
        operator^=1;
       }
     }
    putc(znak,slv); delka++;
PODMINKAKONEC:;
   }			//precteni a zapsani podminky

  if (zavorky > 0) { printf("\rNeuzavrene zavorky na radku %lu!",poziceradek); exit(0); }

  if (prvniradek == 1) {
    poslednipozice=ftell(slv); fseek(slv,puvodnipozice+2,SEEK_SET);
    putc((poslednipozice-puvodnipozice)>>8,slv);
    putc((poslednipozice-puvodnipozice)%256,slv);
    fseek(slv,poslednipozice,SEEK_SET);
    prvniradek=0;
   }

  znak=getc(pkd); poziceradek++; delka+=2;
  znak=getc(pkd);
  if (znak != '5' && znak != '6') {  //dalsi radek podminky
    fseek(pkd,-1,SEEK_CUR); goto PODMINKA;
   }
  fseek(pkd,-1,SEEK_CUR);				//dalsi radek
  switch(typ=getc(pkd)) {
    case '5': putc('5',slv); rodx++; break;
    case '6': putc('6',slv); nahodax++; break;
    default: printf("\rChyba typu na radku %lu!\n",poziceradek); goto KONEC;
   }
  hodnota=10;						//pocatecni hodnota
  switch(getc(pkd)) {
    case '+': hodnota-=getc(pkd)-'0'; break;		//hodnoty jsou
    case '-': hodnota+=getc(pkd)-'0'; break;		//naopak
    default: printf("\rChyba hodnoty na radku %lu\n",poziceradek); goto KONEC;
   }
  putc(hodnota,slv); delka+=2;
  pocetvet+=10;
  fseek(pkd,2,SEEK_CUR); delka+=3; poziceradek++;		//dalsi radek
  for (radky=0; radky < 10; radky++) {
    kolikatyznak=1;
    if (radky == 7) {			//dalsi radek a psani real-time kecu
      fseek(pkd,2,SEEK_CUR); delka+=2; poziceradek++;
     }
    mezera=1; delkaslova=0;
    while ((znak=getc(pkd)) != '\r') {
      if (mezera == 1) {
        if (znak == ' ') {
          printf("\rMezera na zacatku vety na radku %lu!\n",poziceradek);
          goto KONEC;
         }
        if (znak == 255 && typ == '5') koliknerodu++;
        if ((znak == 0 || znak == 255) && typ == '6') {
          printf("\rNa radku %lu ma byt typ 5, ne 6!\n",poziceradek-1);
          goto KONEC;
         }
        mezera=0;
       }
      if ((kolikatyznak == 1 && typ == '6') || (kolikatyznak == 2 && typ == '5'))
       if ((znak < 'A' || znak > 'Z') && znak != '*' && znak != '#') {
         printf("\rVeta nezacina platnym znakem na radku %lu!\n",poziceradek);
         goto KONEC;
        }
      if (znak == ' ') {
        slovo[delkaslova]=0; if (maxdelkaslova < delkaslova) {
          maxdelkaslova=delkaslova; strcpy(nejslovo,slovo);
         }
        delkaslova=0; pocetslov++;
       }
      else if ((znak >= 'A' && znak <= 'Z') || (znak >= 'a' && znak <= 'z'))
       slovo[delkaslova++]=znak;
      else delkaslova=0;
      if (znak == '') { pocetzenvet++; pocetslov++; kolikatyznak=0; }
      putc(znak,slv); delka++; Dvety++; kolikatyznak++;
     }
    if (maxdelkaslova < delkaslova) {
      maxdelkaslova=delkaslova; strcpy(nejslovo,slovo);
     }
    pocetslov++;				//posledni slovo
    if (znak != '.' && znak != '!' && znak != '?' && znak != ':') { printf("\rVeta nekonci platnym operatorem na radku %lu !",poziceradek); goto KONEC; }
    znak=getc(pkd); delka+=2; poziceradek++;		//dalsi radek
    if (typ == '5') Dvety--;			//vynechani pocatecniho znaku
    fwrite("\x0",1,1,slv);				//nula na konci
   }
  poslednipozice=ftell(slv); fseek(slv,puvodnipozice,SEEK_SET);
  putc((poslednipozice-puvodnipozice-2)>>8,slv); putc((poslednipozice-puvodnipozice-2)%256,slv);
  fseek(slv,poslednipozice,SEEK_SET);
  fseek(pkd,2,SEEK_CUR); poziceradek++; delka+=2;     //dalsi (prazdny) radek
  if (delka%10 == 0) printf(".");
 } while (delka < D);

SCITANI:
fseek(slv,konechlavicky+1,SEEK_SET); putc((nahodax+rodx)>>8,slv);
putc((nahodax+rodx)%256,slv); fseek(slv,0,SEEK_END);

//fwrite("ttttt",5,1,slv);				//konec souboru
printf("\r                                                    \r2/2");
fclose(slv);
#if zakodovani == 1
sl1=fopen("POKYD.SL1","wb"); slv=fopen("SLOVNIK.DAT","rb");
fseek(slv,0,SEEK_END); D=ftell(slv); fseek(slv,0,SEEK_SET);
typ=0; hodnota=0;
for (delka=0; delka < konechlavicky; delka++) {
  znak=getc(slv); typ^=znak; typ+=znak; hodnota+=znak; putc(znak,sl1);
 }
for (delka=konechlavicky; delka < D; delka++) {
  znak=getc(slv); typ^=znak; typ+=znak; hodnota+=znak;
  znak^='K'^128; znak+='K'; znak^='6'; putc(znak,sl1);
  if (delka%6000 == 0) printf(".");
 }
fclose(slv); putc(typ,sl1); putc(hodnota,sl1); fclose(sl1);
remove("SLOVNIK.DAT"); rename("POKYD.SL1","SLOVNIK.DAT");
#endif
if (typ == 0 || hodnota == 0) {
  printf("\r   \n************  Upozorneni: Kontrolni soucet souboru SLOVNIK.DAT je nulovy!\n-----------------------------------------------------------------------------\n");
  getch();
 }
printf("\rPrekladaci program k programu Pokyd se uspesne dokoncil.   - hlavicka %d B\n",konechlavicky);
printf("\nDelky:");
printf("\n puvodni soubor SLOVA.PKD:      %lu B",ftell(pkd));
printf("\n vysledny soubor SLOVNIK.DAT:   %lu B",D+1);
printf("\nPrelozeno radku:        %5lu",poziceradek);
printf("\nPrumerna delka vety:    %5d znaku",Dvety/(pocetvet+pocetzenvet));
printf("\nPrumerna delka slova:   %5d znaku",Dvety/pocetslov-1);
printf("\nPrumerne slov ve vete:  %5d",pocetslov/(pocetvet+pocetzenvet));
printf("\nDelka nejdelsiho slova: %5d (%s), v podmince %d znaku",maxdelkaslova,nejslovo,maxpoddelkaslova);
printf("\nPocet slov ve slovniku: %5lu, v podminkach %lu",pocetslov,pocetpodmslov);
printf("\nPocet zenskych vet:     %5lu, celkem tedy %lu",pocetzenvet,pocetvet+pocetzenvet);
printf("\nPodminkovych elementu:  %5lu",podmoperatoru);
printf("\nElementu na podminku:   %5lu",podmoperatoru/(nahodax+rodx-POCET_EXTRA_VET));
printf("\nPamet na data:          %5lu B",pozicepredextra-konechlavicky);
printf("\nPamet na inteligenci:  %6lu B",D+1-pozicepredextra);
printf("\n\nPocet funkci:");
printf("\n NAHODA:  %3lu (%2lu vet), skutecnych vet:  %lu",nahodax,nahodax*10,pocetvet-(rodx*10-koliknerodu));
printf("\n ROD:     %3lu (%2lu vet), skutecnych vet:  %lu",rodx,rodx*10,rodx*10-koliknerodu);
printf("\nttttttttttttttttttttttttttt");
printf("\nCelkem:   %3lu (%2lu vet)\n",nahodax+rodx,pocetvet);
KONEC:
fclose(pkd); fclose(slv);
}