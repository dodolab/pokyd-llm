/* Tento zdrojový kód je pod licencí GNU/GPL. Mùžete ho použít k vlastní
   potøebì, ale nesmíte jej ani programy založené na tomto kódu využít komerènì!

   Jedná se o zdrojový kód programu Pokyd (http://iqpokyd.kyblsoft.cz)
   od Aleše Jandy, aktivnì vyvíjeného 1999 - 2002
*/


#define TESTSLOVNIKU 0

BYTE univerzalnost;
BYTE nalezenakloudnaveta;	//0 - zadna, 1 - nekloudna, 2 - kloudna
WORD pozicepodminka;

#define VYPIS ;//if (pozicepodminka > 65535) { BYTE hlaska[80]; sprintf(hlaska,"Usp: %lu, Neusp: %lu, Slvpoz: %lu, pozpodm: %lu",uspech,neuspech,pozicepodminka,pozicepodminka); HLASKA(hlaska,2); sprintf(hlaska,"%s",inteligence+pozicepodminka-1); HLASKA(hlaska,3); }

void CHYBAPODMINKY(BYTE cislo) {
BYTE hlaska[80],hlaska2[40];
  sprintf(hlaska,"Nastala chyba %d v datovem souboru. Kontaktuj autora! Jde o zavaznou chybu!",cislo);
  HLASKA(hlaska,4);
  strncpy(hlaska,inteligence[intpozice]+pozicepodminka-1,50); hlaska[50]=0;
  sprintf(hlaska2," - podminka %u, pozice %u",intpozice+1,pozicepodminka);
  strcat(hlaska,hlaska2);
  HLASKA(hlaska,4);
 }

void CTIPODMINKY(void) {
WORD uspech;
BYTE skoncenipodminek=0,predchoziuspech;
  univerzalnost=0; intpozice=pocetextravet; predchoziuspech=0;
  docasnenaladabody=0; nalezenakloudnaveta=0;
NOVAPODMINKA:
  pozicepodminka=2;			       //preskoci pozici uspechu
  uspech=inteligence[intpozice][0]; uspech<<=8; uspech+=inteligence[intpozice][1];
  if (inteligence[intpozice][pozicepodminka] == 'e') {
    if (predchoziuspech == 1) { UKONCI_ZAVORKU(); goto DAL; }
    pozicepodminka++;
   }
  switch (PRECTIPODMINKU()) {
   case 1:
    TYP:
    switch (inteligence[intpozice][pozicepodminka++]) {
      case '5': RODX(pozicepodminka); break;	    	//RODX
      case '6': NAHODAX(pozicepodminka); break;		//NAHODAX
      case 'k': konec=1; goto TYP;			//konec=1
      case 'O': skoncenipodminek=1; goto TYP;		//goto OD
      case 's': samomluva=1; goto TYP;			//samomluva=1
      case 'z': zvuk=0; goto TYP;			//zvuk=0
      case 'Z': zvuk=1; goto TYP;			//zvuk=1
      case 't': strcpy(textpredsamomluvou,retezec1); goto TYP;	//strcpy(text...
      case 'm': samomlbody=naladabody; goto TYP;	//samomlbody=naladabody
      case 'x': univerzalnost=1; goto TYP;		//univerzalnost=1
      case 'p': pozodp=0; goto TYP;			//pozodp=0
      case 'o': nalada=rand()%5; goto TYP;		//nalada=rand()%5
      case 'P': SLOVO_PAMET(); goto TYP;		//SLOVO_PAMET()
      case 'J': JSEM_PAMET(); goto TYP;			//JSEM_PAMET()
      case 'S': JSI_PAMET(); goto TYP;			//JSI_PAMET()
      case 'M': if (JMENO_CLOVEKA() == 0) goto NEUSPECH;//JMENO_CLOVEKA();
                else goto TYP;
      default: CHYBAPODMINKY(1); return;
     }
    predchoziuspech=1; break;
   case 2:
   NEUSPECH:
    if (_AL == 2) return;
    predchoziuspech=0;
    break;
   }

  DAL:
  if (++intpozice == pocetiq || skoncenipodminek == 1) return;
  goto NOVAPODMINKA;
 }

BYTE PRECTIPODMINKU(void) {
BYTE hodnota1,podminka,hodnota2,zatimhodnota=1;
  ZACATEK:
  switch(inteligence[intpozice][pozicepodminka++]) {		//hodnota1
    case 'S': hodnota1=SLOVO(inteligence[intpozice]+pozicepodminka);//SLOVO
	      pozicepodminka+=strlen(inteligence[intpozice]+pozicepodminka)+1; break;
    case 'Z': hodnota1=ZACATEK(inteligence[intpozice]+pozicepodminka);//ZACATEK
	      pozicepodminka+=strlen(inteligence[intpozice]+pozicepodminka)+1; break;
    case '(': zatimhodnota=PRECTIPODMINKU(); goto ANDNEBOOR;
    case ')': return(zatimhodnota);
    case 'o': hodnota1=otazka; break;				//otazka
    case 'd': hodnota1=pozodp; break;				//pozodp
    case 'V': hodnota1=VYSKYT(inteligence[intpozice]+pozicepodminka);//VYSKYT
	      pozicepodminka+=strlen(inteligence[intpozice]+pozicepodminka)+1; break;
    case 's': hodnota1=samomluva; break;			//samomluva
    case 'p': hodnota1=pocetslov; break;			//pocetslov
    case 'v': hodnota1=smyslpocvety; break;			//smyslpocvety
    case 'u': hodnota1=pocetuzivvet; break;			//pocetuzivvet
    case 'K': hodnota1=KONECSLOVA(inteligence[intpozice]+pozicepodminka);//KONECSLOVA
	      pozicepodminka+=strlen(inteligence[intpozice]+pozicepodminka)+1; break;
    case 'h': hodnota1=souhlas; break;				//souhlas
    case 'z': hodnota1=zvuk; break;				//zvuk
    case 'y': hodnota1=zenskyrod; break;			//zenskyrod
    case 'e': hodnota1=zenapocitac; break;			//zenapocitac
    case 'x': hodnota1=univerzalnost; break;			//univerzalnost
    case 'b': hodnota1=naladabody; break;			//naladabody
    case 'f': hodnota1=docasnenaladabody; break;		//docasnenaladabody
    case 'n': hodnota1=nastvani; break;				//nastvani
    case 'a': hodnota1=charakter; break;			//charakter
    case 'm': hodnota1=cisloslpamet; break;			//cisloslpamet
    case 'i': hodnota1=cislojsem; break;			//cislojsem
    case 'l': hodnota1=cislojsi; break;				//cislojsi
    case 'c': hodnota1=0;			//cislo uzivatelovy blbosti
              if (stricmp(predtimretezec,retezec1) == NULL) hodnota1=1;
              else if (BLBOSTI_OPAKOVANE_PISMENO() == 1) hodnota1=2;
              break;
    case 'j': if (nalezenakloudnaveta == 0) {		//nalezenakloudnaveta
                if (pozodp == 0) {			//nebyla nalezena
                  nalezenakloudnaveta=1; hodnota1=0;
                 }
                else {
                  nalezenakloudnaveta=2; hodnota1=1;	//byla nalezena
                 }
               }
              else if (nalezenakloudnaveta == 1)	//byla, ale ne kloudna
               hodnota1=0;
              else if (nalezenakloudnaveta == 2)	//byla nalezena
               hodnota1=1;
              break;
    default: CHYBAPODMINKY(2); return(2);
   }
  switch(podminka=inteligence[intpozice][pozicepodminka++]) {
    case 'j': if (hodnota1 != 255) zatimhodnota=1;
              else zatimhodnota=0; goto ANDNEBOOR;
    case 'n': if (hodnota1 == 255) zatimhodnota=1;
              else zatimhodnota=0; goto ANDNEBOOR;
   }

  switch(inteligence[intpozice][pozicepodminka]) {		//hodnota2
    case '.': hodnota2=255; break;
    case 'p': hodnota2=pocetslov-1; break;
    case '0': case '1': case '2': case '3': case '4':
    case '5': case '6': case '7': case '8': case '9':
	      hodnota2=inteligence[intpozice][pozicepodminka]-'0'; break;
    case 'O': hodnota2=80; break;
    default: CHYBAPODMINKY(3); return(2);
   } pozicepodminka++;

  switch(podminka) {						//podminka
    case '!': if (hodnota1 != hodnota2) zatimhodnota=1;
              else zatimhodnota=0; break;
    case '=': if (hodnota1 == hodnota2) zatimhodnota=1;
              else zatimhodnota=0; break;
    case '<': if (hodnota1 < hodnota2) zatimhodnota=1;
              else zatimhodnota=0; break;
    case '>': if (hodnota1 > hodnota2) zatimhodnota=1;
              else zatimhodnota=0; break;
    default: CHYBAPODMINKY(4); return(2);
   }

ANDNEBOOR:
  switch (inteligence[intpozice][pozicepodminka++]) {		// & nebo |
#if TESTSLOVNIKU == 0
    case '|': if (zatimhodnota == 1) { UKONCI_ZAVORKU(); return(1); }
              goto ZACATEK;
    case '&': if (zatimhodnota == 0) { UKONCI_ZAVORKU(); return(0); }
	      goto ZACATEK;
    case ')': pozicepodminka--; goto ZACATEK;
    default: CHYBAPODMINKY(5); return(2);
#endif
#if TESTSLOVNIKU == 1
    case '|': case '&': goto ZACATEK;
    case ')': pozicepodminka--; zatimhodnota=0; goto ZACATEK;
    default: CHYBAPODMINKY(5); return(2);
#endif

   }
 }

void UKONCI_ZAVORKU(void) {
BYTE zavorka=1;
  while (zavorka > 0) {
    switch(inteligence[intpozice][pozicepodminka++]) {
      case '(': zavorka++; break;
      case ')': zavorka--; break;
     }
   }
 }
