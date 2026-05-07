BYTE ZBYTEK(DLOUHE cislo) {
  if ((signed long)cislo != cislo) return(1);
  else return(0);
 }

WORD POSUNSENACISLO(void) {
BYTE zavorky=0,pritomnostzavorek=0;
  while (((retezec1[pozice] < '0' || retezec1[pozice] > '9') && retezec1[pozice] != '.') || zavorky > 0) {
    if (retezec1[pozice] == 0) {
      if (pritomnostzavorek == 0) { strcat(retezec1,"0"); pozice++; }
      return(1);
     }
    if (retezec1[pozice] == '(') { zavorky++; pritomnostzavorek=1; }
    else if (retezec1[pozice] == ')') { zavorky--; pritomnostzavorek=1; }
    pozice++;
   }
  while ((retezec1[pozice] >= '0' && retezec1[pozice] <= '9') || retezec1[pozice] == '.')
   pozice++;
  return(0);
 }

BYTE NAJDIE(void) {
BYTE pozice,celkem=strlen(retezec2),tecka=0;
  for (pozice=0; pozice < celkem; pozice++) {
    if (retezec2[pozice] == 'e') return(pozice);		//'e' se vyskytuje
    if (retezec2[pozice] == '.') tecka=1;
   }
  if (tecka == 1) return(0);				//'e' se nevyskytuje
  else return(255);			//s retezcem se nebude vubec hybat
 }

void UPRAVVYSLEDEK(void) {
BYTE pomocpozice,celkem;
  pozice=NAJDIE();
  if (pozice == 255) return;
  if (pozice == 0) {
    pozice=strlen(retezec2)-1;
    if (strchr(retezec2,'.') != NULL) {
      while (retezec2[pozice] == '0' && retezec2[pozice] != '.')
       retezec2[pozice--]=0;				//vynechani nul
      if (retezec2[pozice] == '.') retezec2[pozice]=0;
     }
   }
  else {
    while (retezec2[--pozice] == '0' || retezec2[pozice] == '.') {
      celkem=strlen(retezec2);
      for (pomocpozice=pozice; pomocpozice < celkem; pomocpozice++)
       retezec2[pomocpozice]=retezec2[pomocpozice+1];
     }
   }
  if (retezec2[0] == 0) strcpy(retezec2,"0");
  else if (strcmp(retezec2,"-0") == 0) strcpy(retezec2,"0");
 }

void VLOZOTEVRENOUZAVORKU(WORD kde) {
WORD celkem=strlen(retezec1)+1,kdebudenula;
  kdebudenula=celkem;
  while (celkem > kde) {
    retezec1[celkem]=retezec1[celkem-1];
    celkem--;
   }
  retezec1[kde]='('; retezec1[kdebudenula]=0;
 }

void VLOZZAVRENOUZAVORKU(void) {
WORD celkem=strlen(retezec1)+1,kdebudenula;
  kdebudenula=celkem;
  while (celkem > pozice) {
    retezec1[celkem]=retezec1[celkem-1];
    celkem--;
   }
  retezec1[pozice]=')'; retezec1[kdebudenula]=0;
 }

DLOUHE PRECTICISLO(void) {
WORD carka=65535;
DLOUHE vysledek=0;
  while ((retezec1[pozice] >= '0' && retezec1[pozice] <= '9') || retezec1[pozice] == '.') {
    if (retezec1[pozice] == '.') carka=pozice;
    else { vysledek*=10; vysledek+=retezec1[pozice]-'0'; }
    pozice++;
   }
  if (carka != 65535)
   for (carka=pozice-carka-1; carka > 0; carka--) vysledek/=10;
  return(vysledek);
 }

void KONEC(void) {
  textattr(7); cprintf(" \n\r \r");
  exit();
 }

void CHYBA(BYTE *hlaska) {
  textcolor(12); cprintf("\n\r%s\n\n\r",hlaska); chyba=1;
 }

void ZKONTROLUJ1(void) {
WORD minus=0,znamenko=0,predtimpozice,celkem=strlen(retezec1);
  for (pozice=0; pozice < celkem; pozice++) {
    switch(retezec1[pozice]) {
      case '+': goto VLOZZAVORKU;
      case '-': if (minus == 0) minus=1; else minus=0;
                VLOZZAVORKU:
                if (znamenko == 1) {
                  predtimpozice=pozice;
                  VLOZOTEVRENOUZAVORKU(pozice);
		  celkem+=POSUNSENACISLO(); VLOZZAVRENOUZAVORKU();
		  pozice=predtimpozice+1; celkem+=2;
		 }
		else znamenko=1; break;
      case '*': case '/': case '^': znamenko=1; break;
      default: znamenko=0;
     }
   }
  retezec1[pozice]=0;
 }

BYTE ZKONTROLUJ2(void) {
WORD pozice1,pozice2=0,celkem=strlen(retezec1),znak,zavorky=0;
BYTE posledni=2;	//0 - cislo, 1 - znamenko, 2 - cokoliv, 3 - zavorka (,
			//4 - zavorka )

  cispozice=0;
  for (pozice1=0; pozice1 < celkem; pozice1++) {
    znak=retezec1[pozice1]; if (znak == ',') { znak='.'; retezec1[pozice1]='.'; }
    if ((znak >= '0' && znak <= '9') || znak == '.') {
      if (posledni == 4) retezec2[pozice2++]='*';
      retezec2[pozice2]=retezec1[pozice1]; pozice2++; posledni=0; goto DOBRY;
     }
    switch(znak) {
      case '-': case '+': if (posledni != 0 && posledni != 4) {
						    //pred znamenkem neni cislo
		  if (posledni == 2) {		//nic
		    retezec2[pozice2++]='0';	//vlozit pred minus nulu
		   }
		  else {
		    retezec2[pozice2++]='('; pozice=pozice1;
		    pozice++; PRECTICISLO(); VLOZZAVRENOUZAVORKU();
		    zavorky++; celkem++;
		   }
		 }
                posledni=1; break;
      case '*': case '/': case '^': /*case '!':*/
                if (posledni == 1) {
                  CHYBA("Chyba operatoru!"); return(1);
                 }
                else posledni=1; break;
      case '(': if (posledni == 0 || posledni == 4)	//mezi cislo a zavorku
                 retezec2[pozice2++]='*';		//dosadit krat
		zavorky++; posledni=2; break;
      case ')': if (zavorky > 0) zavorky--;
                else {
                  CHYBA("Zavorka navic!"); return(1);
                 }
		posledni=4; break;
      case ' ': goto DOBRY;
      default:
        textcolor(12); cprintf("Neznamy znak '%c'!\n\n\r",znak); return(1);
     }
    retezec2[pozice2]=retezec1[pozice1]; pozice2++;
    DOBRY:;
   }
  retezec2[pozice2]=0;
  if (zavorky > 0) {
    CHYBA("Neuzavrena zavorka!"); return(1);
   }
  strcpy(retezec1,retezec2);
  return(0);
 }

void VLOZZAVORKY(void) {
WORD priorita=0,pozpredcislem=pozice;
ZNOVU:
  PRECTICISLO();
  switch(retezec1[pozice]) {
    case '+': case '-': while (priorita > 0) { VLOZZAVRENOUZAVORKU(); priorita--; pozice++; }
			break;
    case '*': case '/': if (priorita < 1) { VLOZOTEVRENOUZAVORKU(pozpredcislem); pozice++; }
			if (priorita > 1) VLOZZAVRENOUZAVORKU();
			priorita=1; break;
    case '^': while (priorita < 2) { VLOZOTEVRENOUZAVORKU(pozpredcislem); pozice++; priorita++; }
	      break;				//priorita=2;
    case '(': pozice++; VLOZZAVORKY(); goto DAL;
    case ')': while (priorita-- > 0) { VLOZZAVRENOUZAVORKU(); pozice++; }
	      return;
   }
  pozpredcislem=pozice+1;
  DAL:
  pozice++; goto ZNOVU;
 }

DLOUHE VYPOCITEJ(void) {
DLOUHE cislo1,cislo2;
BYTE znamenko;
  if (retezec1[pozice] == '(') { pozice++; cislo1=VYPOCITEJ(); }
  else cislo1=PRECTICISLO();
  while (retezec1[pozice] != ')') {
    znamenko=retezec1[pozice++];
    if (retezec1[pozice] == '(') { pozice++; cislo2=VYPOCITEJ(); }
    else cislo2=PRECTICISLO();
    switch (znamenko) {
      case '+': cislo1+=cislo2; break;
      case '-': cislo1-=cislo2; break;
      case '*': cislo1*=cislo2; break;
      case '/': if (cislo2 == 0) {
		  CHYBA("Deleni nulou!"); return(0);
                 }
                cislo1/=cislo2; break;
      case '^': if (cislo1 < 0 && ZBYTEK(cislo2) != 0) {
		  CHYBA("Chyba pri umocnovani!"); return(0);
		 }
                else if (cislo1 == 10) cislo1=pow10(cislo2);
                else cislo1=pow(cislo1,cislo2);
     }
   }
  pozice++; return(cislo1);
 }  

void SMAZRADEK(void) {
  gotoxy(1,wherey());
  _AL=' '; _AH=9; _CX=80; _BX=14; geninterrupt(0x10);
 }

void SMAZZNAK(WORD pozice,WORD celkem) {
WORD pomoc;
  if (pozice == 0) return;
  for (pomoc=pozice; pomoc <= celkem; pomoc++)
   retezec1[pomoc-1]=retezec1[pomoc];
  retezec1[--celkem]=0; pozice--;
 }

void SMAZKURZOR(void) {
  _AH = 1;
  _CX = 0x2000;
  geninterrupt (0x10);		//smaz kurzor
 }

void NASTAVKURZOR(void) {
  _AH = 1;
  _CX = 0x0406;
  geninterrupt (0x10);		//nastav kurzor
 }

void PLNYKURZOR(void){
  _AH = 1;
  _CX = 0x1F;
  geninterrupt(0x10);
 }

void PREKRESLI_RADEK(BYTE insert) {
BYTE pozice=0,celkem=strlen(retezec1)+1,puvpozx=wherex(),znak,barva;
  SMAZKURZOR(); gotoxy(1,wherey());
  while (pozice < celkem) {
    znak=retezec1[pozice];
    if (znak == 0) znak = ' ';			//nula se vymeni za mezeru
    _AL=znak; _AH=9; _CX=1; _BX=14; geninterrupt(0x10);
    pozice++;
    gotoxy(wherex()+1,wherey());
   }
  gotoxy(puvpozx,wherey());
  if (insert == 0) NASTAVKURZOR(); else PLNYKURZOR();
 }

void NAPIS(void) {
signed char pozice,celkem;
BYTE znak,pomoc,insert=0,scankod,pozicey=wherey();
  pozice=0; celkem=0; retezec1[0]=0;
  textcolor(14); SMAZRADEK();
  UPLNYSTART:
  if (insert == 0) NASTAVKURZOR();
  else PLNYKURZOR();

NAPISSTART:
  gotoxy(pozice+1,pozicey);
  _AH = 0x10; geninterrupt(0x16);		//cteni znaku z klavesnice
  scankod=_AH; znak=_AL;
  if (scankod >= 71 && scankod <= 82) switch(scankod) {
     case 71: if (znak == '7' || znak == 0) { znak='7'; goto DAL; } else break;
     case 72: if (znak == '8' || znak == 0) { znak='8'; goto DAL; } else break;
     case 73: if (znak == '9' || znak == 0) { znak='9'; goto DAL; } else break;
     case 75: if (znak == '4' || znak == 0) { znak='4'; goto DAL; } else break;
     case 76: if (znak == '5' || znak == 0) { znak='5'; goto DAL; } else break;
     case 77: if (znak == '6' || znak == 0) { znak='6'; goto DAL; } else break;
     case 79: if (znak == '1' || znak == 0) { znak='1'; goto DAL; } else break;
     case 80: if (znak == '2' || znak == 0) { znak='2'; goto DAL; } else break;
     case 81: if (znak == '3' || znak == 0) { znak='3'; goto DAL; } else break;
     case 82: if (znak == '0' || znak == 0) { znak='0'; goto DAL; } else break;
    }
  switch(scankod) {
    case 28: goto NAPISDAL;			//Levy Enter
    case 224: if (znak == '\r') goto NAPISDAL;	//Pravy Enter
    case 14: if (znak == 127) {			//Ctrl-Backspace
	pomoc=pozice; //if (pozice > 0) pozice--;
	while (pozice > 0 && retezec1[pozice-1] == ' ') pozice--;
	while (pozice > 0 && retezec1[pozice-1] != ' ') pozice--;
	gotoxy(celkem-pomoc+pozice+1,pozicey);
	while (pomoc > pozice) {
	  SMAZZNAK(pomoc,celkem); pomoc--; celkem--;
	  cprintf(" ");
         }
       }
      else if (znak == '\b') {			//Backspace
	if (pozice == 0) goto NAPISSTART;
        SMAZZNAK(pozice,celkem); celkem--; pozice--;
        gotoxy(pozice+1,pozicey);
       }
      else goto DAL;
      PREKRESLI_RADEK(insert); goto NAPISSTART;
    case 71:					//Home
      pozice=0; gotoxy(1,pozicey); goto NAPISSTART;
    case 75:					//doleva
      if (pozice > 0) pozice--;
      gotoxy(pozice+1,pozicey); goto NAPISSTART;
    case 77:					//doprava
      if (pozice < celkem) pozice++;
      gotoxy(pozice+1,pozicey); goto NAPISSTART;
    case 79:					//End
      pozice=celkem; gotoxy(pozice+1,pozicey); goto NAPISSTART;
    case 82:					//Insert
      if (insert == 0) insert=1; else insert=0;
      goto UPLNYSTART;
    case 83:					//Delete
      if (celkem > pozice) {
	for (pomoc=pozice; pomoc <= celkem; pomoc++)
	 retezec1[pomoc]=retezec1[pomoc+1];
	PREKRESLI_RADEK(insert);
	gotoxy(pozice+1,pozicey); celkem--;
       }
      goto NAPISSTART;
   }
  DAL:
  if (znak == 0x1B) {				//Esc
    if (celkem == 0) KONEC();
    SMAZRADEK(); celkem=0; pozice=0; goto NAPISSTART;
   }
  if (znak < ' ' || znak > 127) goto NAPISSTART;
  if (insert == 0) {
    if (celkem == 79) goto NAPISSTART;
    for (pomoc=celkem+1; pomoc > pozice; pomoc--)
     retezec1[pomoc]=retezec1[pomoc-1];
    retezec1[pozice++]=znak; retezec1[++celkem]=0;
   }
  else {
    if (pozice == 79) goto NAPISSTART;
    retezec1[pozice++]=znak; if (pozice > celkem) celkem=pozice;
    retezec1[celkem]=0;
   }
  PREKRESLI_RADEK(insert); gotoxy(pozice+1,pozicey);
  goto NAPISSTART;
  NAPISDAL:
  retezec1[celkem]=0; cprintf("\n\r");
 }
