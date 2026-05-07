BYTE JELI_HAD(BYTE ktery,BYTE x,BYTE y);

WORD CAS18() {
  _AH=0;
  geninterrupt (0x1A);
  return(_DX);
 }

void NAPISZNAK(BYTE znak,BYTE barva) {
  if (znak != 0) _AL = znak;
  else _AL = ' ';                               //vymena nuly za mezeru
  _CX = 1; _BH = 0; _BL = barva; _AH = 9;
  geninterrupt(0x10);
 }

void NAPISZNAK2(BYTE znak,BYTE barva) {
  NAPISZNAK(znak,barva);
  gotoxy(wherex()+1,wherey());
 }

void NAPISRETEZEC(BYTE *retezec,BYTE barva) {
BYTE pozice,celkempozice=strlen(retezec),x,y;
  if (intro == 1 && kbhit() == 0) { barva-=8; x=wherex(); y=wherey(); }
NAPIS:
  for (pozice=0; pozice < celkempozice; pozice++)
   NAPISZNAK2(retezec[pozice],barva);
  if (intro == 1 && barva < 8) { barva+=8; delay(50); gotoxy(x,y); goto NAPIS; }
 }

void RASTR_NAPISZNAK(BYTE barva,BYTE rastr) {
  if (rastr == 0) NAPISZNAK(' ',barva);
  else NAPISZNAK('',barva+((barva>>4)^8));
 }

BYTE CEKEJ(BYTE *hlaska,BYTE klavesa) {
BYTE delka=(strlen(hlaska)>>1),znak;
  gettext(1,pocety-2,pocetx,pocety-2,pomocne);
  gotoxy(pocetx/2-delka,pocety-2);
  NAPISZNAK2(' ',3*16); NAPISRETEZEC(hlaska,14+3*16); NAPISZNAK2(' ',3*16);
  if (klavesa == 1 && zvuk == 1) { sound(500); delay(50); nosound(); }
#if CEKAT == 1
  if (klavesa == 0) { while (kbhit() != 0) getch(); while(kbhit() == 0); }
  else {
    znak=getch(); if (znak == 0) getch();
   }
#endif
  puttext(1,pocety-2,pocetx,pocety-2,pomocne);
  return(znak);
 }

void DOPLN_KONCOVKU(BYTE zivoty) {
  if (zivoty > 4 || zivoty == 0) strcat(pomocne,"u ");
  else if (zivoty > 1) strcat(pomocne,"y ");
  else strcat(pomocne,"  ");
 }

void NAPISBODY(void) {
  gotoxy(1,pocety);
  if (pocetx == 80) {
    if (pocethracu == 1) {
      sprintf(pomocne,"> Had: %5li bodu,  %d zivot",body1,zivoty1);
      DOPLN_KONCOVKU(zivoty1);
      NAPISRETEZEC(pomocne,8+14*16);
      sprintf(pomocne," Had v1.0 - autor Ales Janda - KYBLSoft (C) 6/2001 ");
      NAPISRETEZEC(pomocne,15+3*16);
     }
    else {
      sprintf(pomocne," 1: %5li bodu,  %d zivot",body1,zivoty1);
      DOPLN_KONCOVKU(zivoty1);
      NAPISRETEZEC(pomocne,8+14*16);
      sprintf(pomocne," 2: %5li bodu,  %d zivot",body2,zivoty2);
      DOPLN_KONCOVKU(zivoty2);
      NAPISRETEZEC(pomocne,8+10*16);
      sprintf(pomocne," Had v1.0 - KYBLSoft 6/2001 ");
      NAPISRETEZEC(pomocne,15+3*16);
     }
   }
  else {
    if (pocethracu == 1) {
      sprintf(pomocne," %5li bodu, %d zivot",body1,zivoty1);
      DOPLN_KONCOVKU(zivoty1);
      NAPISRETEZEC(pomocne,8+14*16);
      sprintf(pomocne," Had (C) KYBLSoft ");
      NAPISRETEZEC(pomocne,15+3*16);
     }
    else {
      sprintf(pomocne," %5li b,  %d zivot",body1,zivoty1);
      DOPLN_KONCOVKU(zivoty1);
      NAPISRETEZEC(pomocne,8+14*16);
      sprintf(pomocne," %5li b,  %d zivot",body2,zivoty2);
      DOPLN_KONCOVKU(zivoty2);
      NAPISRETEZEC(pomocne,8+10*16);
     }
   }
 }

void KURZOR(BYTE kde) {
BYTE podtim[62],pozice;
  gettext(25,kde,55,kde,podtim);
  for (pozice=1; pozice < 62; pozice+=2)
   podtim[pozice]^=4*16;
  puttext(25,kde,55,kde,podtim);
 }

void DEJ_PREMII(void) {
WORD pozice;
  START:
  premiex=rand()%pocetx+1; premiey=(rand()%(pocety-1))+1;
  for (pozice=0; pozice < delkahada1; pozice++)
   if (had1[pozice][0] == premiex && had1[pozice][1] == premiey) goto START;
  for (pozice=0; pozice < delkahada2; pozice++)
   if (had2[pozice][0] == premiex && had2[pozice][1] == premiey) goto START;

  gotoxy(premiex,premiey); RASTR_NAPISZNAK(12*16,rastr&2);
  typpremie++; blikpremie=20;
 }

BYTE JELI_HAD(BYTE ktery,BYTE x,BYTE y) {
signed short pozice;
  if (modulo == 1) {
    if (x == 0) x=pocetx;
    if (y == 0) y=pocety-1;
    if (x == pocetx+1) x=1;
    if (y == pocety) y=1;
   }
  else if (x == 0 || y == 0 || x == pocetx+1 || y == pocety) return(1);
  if ((ktery&1) == 1) {
    for (pozice=delkahada1-1; pozice >= 0; pozice--) {
      if (had1[pozice][0] == x && had1[pozice][1] == y) return(1);
     }
   }
  if ((ktery&2) == 2) {
    for (pozice=delkahada2-1; pozice >= 0; pozice--) {
      if (had2[pozice][0] == x && had2[pozice][1] == y) return(1);
     }
   }
  return(0);
 }

BYTE JELI_HAD_PAST(BYTE smer,BYTE x,BYTE y) {
  if (JELI_HAD(3,x,y) == 1) return(1);
  if (past == 0) return(0);

  if (srazkajedenbod==1)
   nosound();
  if (srazkajedenbod == 1 && rand()%2 == 0) {//oba hadi se srazili
    if (aktualnihad == 1 && smer == jedenbodsmer1) return(1);
    if (aktualnihad == 2 && smer == jedenbodsmer2) return(1);
   }
  if (past == 1) return(0);

  switch(smer) {
    case 0: case 2:
      if (JELI_HAD(3,x,y-1) == 1 && JELI_HAD(3,x,y+1) == 1) return(1); break;
    case 1: case 3:
      if (JELI_HAD(3,x-1,y) == 1 && JELI_HAD(3,x+1,y) == 1) return(1); break;
   }
  if (past == 2) return(0);

  if (smer == 0 || smer == 1) {
    if (JELI_HAD(3,x-1,y) == 1 && JELI_HAD(3,x,y-1) == 1 && JELI_HAD(3,x+1,y+1) == 1) return(1); }
  if (smer == 1 || smer == 2) {
    if (JELI_HAD(3,x+1,y) == 1 && JELI_HAD(3,x,y-1) == 1 && JELI_HAD(3,x-1,y+1) == 1) return(1); }
  if (smer == 2 || smer == 3) {
    if (JELI_HAD(3,x+1,y) == 1 && JELI_HAD(3,x,y+1) == 1 && JELI_HAD(3,x-1,y-1) == 1) return(1); }
  if (smer == 0 || smer == 3) {
    if (JELI_HAD(3,x-1,y) == 1 && JELI_HAD(3,x,y+1) == 1 && JELI_HAD(3,x+1,y-1) == 1) return(1); }

  return(0);
 }

BYTE ZJISTI_ZABLOKOVANI(BYTE ktereho) {
BYTE x,y;
  if (maxnabour != 255) return(0);	//pouze pri uplne toleranci nabourani

  if (ktereho == 1) {
    x=had1[0][0]; y=had1[0][1];
    if (JELI_HAD(1,x-1,y) == 1 &&
         JELI_HAD(1,x,y-1) == 1 &&
          JELI_HAD(1,x+1,y) == 1 &&
           JELI_HAD(1,x,y+1) == 1) {
      return(1);
     }
   }
  else if (ktereho == 2) {
    x=had2[0][0]; y=had2[0][1];
    if (JELI_HAD(2,x-1,y) == 1 &&
         JELI_HAD(2,x,y-1) == 1 &&
          JELI_HAD(2,x+1,y) == 1 &&
           JELI_HAD(2,x,y+1) == 1) {
      return(1);
     }
   }
  else if (ktereho == 3) {
    x=had1[0][0]; y=had1[0][1];
    if (JELI_HAD(3,x-1,y) == 1 &&
         JELI_HAD(3,x,y-1) == 1 &&
          JELI_HAD(3,x+1,y) == 1 &&
           JELI_HAD(3,x,y+1) == 1) {
      x=had2[0][0]; y=had2[0][1];
      if (JELI_HAD(3,x-1,y) == 1 &&
           JELI_HAD(3,x,y-1) == 1 &&
            JELI_HAD(3,x+1,y) == 1 &&
             JELI_HAD(3,x,y+1) == 1) {
        return(1);
       }
     }
   }
  return(0);
 }

BYTE NARAZ(void) {
WORD pozice;
  if (narazenyhad == 1 || narazenyhad == 3) {
    narazeni1++;
    if (narazeni1 == maxnabour) return(1);
    if (narazeni1 == 100) narazeni1=1;  //aby se to hejbalo, ale furt bylo
   }
  if (narazenyhad == 2 || narazenyhad == 3) {
    narazeni2++;
    if (narazeni2 == maxnabour) return(1);
    if (narazeni2 == 100) narazeni2=1;
   }
  return(0);
 }

BYTE NARAZENI(BYTE pozicex1, BYTE pozicey1, BYTE pozicex2, BYTE pozicey2) {
BYTE puvnarazeni1=narazeni1,puvnarazeni2=narazeni2;
BYTE narazenihadi=0;
signed short pozice;

  if (pocethracu == 2) {
    if (pozicex1 == pozicex2 && pozicey1 == pozicey2) { //srazka v jednom bode
      srazkajedenbod=1; jedenbodsmer1=smer1; jedenbodsmer2=smer2;
      narazenyhad=3; if (NARAZ() == 1) return(3);
     }
    else srazkajedenbod=0;
   }

  if (ZJISTI_ZABLOKOVANI(1) == 1) narazenihadi|=1;
  if (pocethracu == 2) {
    if (ZJISTI_ZABLOKOVANI(2) == 1) narazenihadi|=2;
    if (narazenihadi == 0) { if (ZJISTI_ZABLOKOVANI(3) == 1) narazenihadi=3; }
   }

  narazenyhad=1;
  switch(smer1) {
    case 0:                             //vlevo
      if (pozicex1 == 0) { if (NARAZ() == 1) narazenihadi|=1; } break;
    case 1:                             //nahoru
      if (pozicey1 == 0) { if (NARAZ() == 1) narazenihadi|=1; } break;
    case 2:                             //vpravo
      if (pozicex1 == pocetx+1) { if (NARAZ() == 1) narazenihadi|=1; } break;
    case 3:                             //dolu
      if (pozicey1 == pocety) { if (NARAZ() == 1) narazenihadi|=1; } break;
   }
  if (pocethracu == 2) {
    narazenyhad=2;
    switch(smer2) {
      case 0:                           //vlevo
        if (pozicex2 == 0) { if (NARAZ() == 1) narazenihadi|=2; } break;
      case 1:                           //nahoru
	if (pozicey2 == 0) { if (NARAZ() == 1) narazenihadi|=2; } break;
      case 2:                           //vpravo
        if (pozicex2 == pocetx+1) { if (NARAZ() == 1) narazenihadi|=2; } break;
      case 3:                           //dolu
        if (pozicey2 == pocety) { if (NARAZ() == 1) narazenihadi|=2; } break;
     }
   }

  narazenyhad=1;
  for (pozice=delkahada1-1; pozice >= 0; pozice--)              //narazka sam do sebe
   if (had1[pozice][0] == pozicex1 && had1[pozice][1] == pozicey1) { if (NARAZ() == 1) narazenihadi|=1; }

  if (pocethracu == 2) {
    for (pozice=delkahada2-1; pozice >= 0; pozice--)
     if (had2[pozice][0] == pozicex1 && had2[pozice][1] == pozicey1) { if (NARAZ() == 1) narazenihadi|=1; }

    narazenyhad=2;
    for (pozice=delkahada1-1; pozice >= 0; pozice--)            //narazka sam do sebe
     if (had1[pozice][0] == pozicex2 && had1[pozice][1] == pozicey2) { if (NARAZ() == 1) narazenihadi|=2; }
    for (pozice=delkahada2-1; pozice >= 0; pozice--)
     if (had2[pozice][0] == pozicex2 && had2[pozice][1] == pozicey2) { if (NARAZ() == 1) narazenihadi|=2; }
   }

  if (puvnarazeni1 == narazeni1) narazeni1=0;
  if (puvnarazeni2 == narazeni2) narazeni2=0;

  return(narazenihadi);
 }

BYTE POPOSUN_HADY(void) {
WORD pozice;
BYTE pozicex1,pozicey1,pozicex2,pozicey2;

  switch(smer1) {
    case 0:                             //vlevo
      pozicex1=had1[0][0]-1; pozicey1=had1[0][1]; break;
    case 1:                             //nahoru
      pozicex1=had1[0][0]; pozicey1=had1[0][1]-1; break;
    case 2:                             //vpravo
      pozicex1=had1[0][0]+1; pozicey1=had1[0][1]; break;
    case 3:                             //dolu
      pozicex1=had1[0][0]; pozicey1=had1[0][1]+1; break;
   }

  if (pocethracu == 2) {
    switch(smer2) {
      case 0:                           //vlevo
        pozicex2=had2[0][0]-1; pozicey2=had2[0][1]; break;
      case 1:                           //nahoru
        pozicex2=had2[0][0]; pozicey2=had2[0][1]-1; break;
      case 2:                           //vpravo
        pozicex2=had2[0][0]+1; pozicey2=had2[0][1]; break;
      case 3:                           //dolu
        pozicex2=had2[0][0]; pozicey2=had2[0][1]+1; break;
     }
   }

  if (modulo == 1) {
    if (pozicex1 == 0) pozicex1=pocetx;
    if (pozicey1 == 0) pozicey1=pocety-1;
    if (pozicex1 == pocetx+1) pozicex1=1;
    if (pozicey1 == pocety) pozicey1=1;

    if (pocethracu == 2) {
      if (pozicex2 == 0) pozicex2=pocetx;
      if (pozicey2 == 0) pozicey2=pocety-1;
      if (pozicex2 == pocetx+1) pozicex2=1;
      if (pozicey2 == pocety) pozicey2=1;
     }
   }

  narazenyhad=NARAZENI(pozicex1,pozicey1,pozicex2,pozicey2);
  if (narazenyhad != 0) return(1);

  if (narazeni1 == 0) {
    if (prodlouzeni1 == 0) {
      gotoxy(had1[delkahada1-1][0],had1[delkahada1-1][1]);
      if (cary == 0) RASTR_NAPISZNAK(0,rastr&1);
      else NAPISZNAK('',6);
     }
    for (pozice=delkahada1; pozice > 0; pozice--) {
      had1[pozice][0]=had1[pozice-1][0];
      had1[pozice][1]=had1[pozice-1][1];
     }
    had1[0][0]=pozicex1; had1[0][1]=pozicey1;
    if (prodlouzeni1 > 0) {
      delkahada1++; prodlouzeni1--;
     }
   }
  if (pocethracu == 2 && narazeni2 == 0) {
    if (prodlouzeni2 == 0) {
      gotoxy(had2[delkahada2-1][0],had2[delkahada2-1][1]);
      if (cary == 0) RASTR_NAPISZNAK(0,rastr&1);
      else NAPISZNAK('',2);
     }
    for (pozice=delkahada2; pozice > 0; pozice--) {
      had2[pozice][0]=had2[pozice-1][0];
      had2[pozice][1]=had2[pozice-1][1];
     }
    had2[0][0]=pozicex2; had2[0][1]=pozicey2;
    if (prodlouzeni2 > 0) {
      delkahada2++; prodlouzeni2--;
     }
   }

  if (narazeni1 == 0) {
    gotoxy(had1[0][0],had1[0][1]); NAPISZNAK('',14*16+6);
    for (pozice=1; pozice < delkahada1; pozice++) {
      gotoxy(had1[pozice][0],had1[pozice][1]); RASTR_NAPISZNAK(14*16,rastr&2);
     }
   }

  if (pocethracu == 2 && narazeni2 == 0) {
    gotoxy(had2[0][0],had2[0][1]); NAPISZNAK('',10*16+2);
    for (pozice=1; pozice < delkahada2; pozice++) {
      gotoxy(had2[pozice][0],had2[pozice][1]); RASTR_NAPISZNAK(10*16,rastr&2);
     }
   }

  if (had1[0][0] == premiex && had1[0][1] == premiey) {
#if CEKAT == 1
    if (zvuk == 1) sound(1000); delay(500); nosound();
#endif
    switch(prodlouzeni) {
      case 0: break;
      case 1: prodlouzeni1+=typpremie/4; break;
      case 2: prodlouzeni1+=typpremie/2; break;
      case 3: prodlouzeni1+=typpremie; break;
      case 4: prodlouzeni1+=typpremie*2; break;
      case 5: prodlouzeni1+=typpremie*4; break;
     }
    body1+=typpremie; body1+=blikpremie; DEJ_PREMII();
   }

  if (pocethracu == 2) {
    if (had2[0][0] == premiex && had2[0][1] == premiey) {
#if CEKAT == 1
      if (zvuk == 1) sound(1000); delay(500); nosound();
#endif
      switch(prodlouzeni) {
        case 0: break;
        case 1: prodlouzeni2+=typpremie/4; break;
        case 2: prodlouzeni2+=typpremie/2; break;
        case 3: prodlouzeni2+=typpremie; break;
        case 4: prodlouzeni2+=typpremie*2; break;
        case 5: prodlouzeni2+=typpremie*4; break;
       }
      body2+=typpremie; body2+=blikpremie; DEJ_PREMII();
     }
   }

  if (narazeni1 == 0) puvsmer1=smer1;
   else smer1=puvsmer1;
  if (narazeni2 == 0) puvsmer2=smer2;
   else smer2=puvsmer2;
  return(0);
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
    if ((pomocne[cislo] = *uk2++) == 0) break;
   }
  pozice=strlen(pomocne)-1;
  while (pomocne[pozice] != '\\') {
    if (pozice == 0) { pomocne[0]=0; goto KONEC; }//zabraneni blbostem v pomocne[]
    else pozice--;
   }
  pomocne[pozice+1]=0;
  KONEC:
  strcpy(rootsoubor,pomocne);
  strcat(rootsoubor,souborcat);
  strcpy(pomocne,rootsoubor);

  strcpy(assetssoubor,pomocne);
  strcat(assetssoubor,"assets\\");
  strcat(assetssoubor,souborcat);
  assettest=fopen(assetssoubor,"rb");
  if (assettest != NULL) {
    fclose(assettest);
    strcpy(pomocne,assetssoubor);
   }
 }

void INTELIGENCE(void) {			//inteligence pocitace
BYTE pozicex,pozicey,premx=premiex,premy=premiey,smer;
signed char rozdil;

  past=3;
  if (aktualnihad == 1) { pozicex=had1[0][0]; pozicey=had1[0][1]; smer=smer1; }
  else { pozicex=had2[0][0]; pozicey=had2[0][1]; smer=smer2; }

  if (modulo == 1) {
    rozdil=pozicex-premx; if (rozdil < 0) rozdil*=-1;
    if (rozdil >= pocetx/2) {
      if (pozicex < premx) premx=0; else premx=pocetx+1;
     }
    rozdil=pozicey-premy; if (rozdil < 0) rozdil*=-1;
    if (rozdil >= pocety/2) {
      if (pozicey < premy) premy=0; else premy=pocety;
     }
   }
  if (premx > pozicex && premy > pozicey) {
SMER1:   if (JELI_HAD_PAST(2,pozicex+1,pozicey) == 0 && smer != 0) smer=2;
    else if (JELI_HAD_PAST(3,pozicex,pozicey+1) == 0 && smer != 1) smer=3;
    else if (JELI_HAD_PAST(0,pozicex-1,pozicey) == 0 && smer != 2) smer=0;
    else if (JELI_HAD_PAST(1,pozicex,pozicey-1) == 0 && smer != 3) smer=1;
    else if (past > 0) { past--; goto SMER1; }
   }
  else if (premx > pozicex && premy < pozicey) {
SMER2:   if (JELI_HAD_PAST(2,pozicex+1,pozicey) == 0 && smer != 0) smer=2;
    else if (JELI_HAD_PAST(1,pozicex,pozicey-1) == 0 && smer != 3) smer=1;
    else if (JELI_HAD_PAST(3,pozicex,pozicey+1) == 0 && smer != 1) smer=3;
    else if (JELI_HAD_PAST(0,pozicex-1,pozicey) == 0 && smer != 2) smer=0;
    else if (past > 0) { past--; goto SMER2; }
   }
  else if (premx < pozicex && premy > pozicey) {
SMER3:   if (JELI_HAD_PAST(0,pozicex-1,pozicey) == 0 && smer != 2) smer=0;
    else if (JELI_HAD_PAST(3,pozicex,pozicey+1) == 0 && smer != 1) smer=3;
    else if (JELI_HAD_PAST(2,pozicex+1,pozicey) == 0 && smer != 0) smer=2;
    else if (JELI_HAD_PAST(1,pozicex,pozicey-1) == 0 && smer != 3) smer=1;
    else if (past > 0) { past--; goto SMER3; }
   }
  else if (premx < pozicex && premy < pozicey) {
SMER4:   if (JELI_HAD_PAST(0,pozicex-1,pozicey) == 0 && smer != 2) smer=0;
    else if (JELI_HAD_PAST(1,pozicex,pozicey-1) == 0 && smer != 3) smer=1;
    else if (JELI_HAD_PAST(2,pozicex+1,pozicey) == 0 && smer != 0) smer=2;
    else if (JELI_HAD_PAST(3,pozicex,pozicey+1) == 0 && smer != 1) smer=3;
    else if (past > 0) { past--; goto SMER4; }
   }
  else if (premx == pozicex && premy > pozicey) {
SMER5:   if (JELI_HAD_PAST(3,pozicex,pozicey+1) == 0 && smer != 1) smer=3;
    else if (JELI_HAD_PAST(0,pozicex-1,pozicey) == 0 && smer != 2) smer=0;
    else if (JELI_HAD_PAST(2,pozicex+1,pozicey) == 0 && smer != 0) smer=2;
    else if (JELI_HAD_PAST(1,pozicex,pozicey-1) == 0 && smer != 3) smer=1;
    else if (past > 0) { past--; goto SMER5; }
   }
  else if (premx == pozicex && premy < pozicey) {
SMER6:   if (JELI_HAD_PAST(1,pozicex,pozicey-1) == 0 && smer != 3) smer=1;
    else if (JELI_HAD_PAST(0,pozicex-1,pozicey) == 0 && smer != 2) smer=0;
    else if (JELI_HAD_PAST(2,pozicex+1,pozicey) == 0 && smer != 0) smer=2;
    else if (JELI_HAD_PAST(3,pozicex,pozicey+1) == 0 && smer != 1) smer=3;
    else if (past > 0) { past--; goto SMER6; }
   }
  else if (premx > pozicex && premy == pozicey) {
SMER7:   if (JELI_HAD_PAST(2,pozicex+1,pozicey) == 0 && smer != 0) smer=2;
    else if (JELI_HAD_PAST(1,pozicex,pozicey-1) == 0 && smer != 3) smer=1;
    else if (JELI_HAD_PAST(3,pozicex,pozicey+1) == 0 && smer != 1) smer=3;
    else if (JELI_HAD_PAST(0,pozicex-1,pozicey) == 0 && smer != 2) smer=0;
    else if (past > 0) { past--; goto SMER7; }
   }
  else if (premx < pozicex && premy == pozicey) {
SMER8:   if (JELI_HAD_PAST(0,pozicex-1,pozicey) == 0 && smer != 2) smer=0;
    else if (JELI_HAD_PAST(1,pozicex,pozicey-1) == 0 && smer != 3) smer=1;
    else if (JELI_HAD_PAST(3,pozicex,pozicey+1) == 0 && smer != 1) smer=3;
    else if (JELI_HAD_PAST(2,pozicex+1,pozicey) == 0 && smer != 0) smer=2;
    else if (past > 0) { past--; goto SMER8; }
   }

  if (aktualnihad == 1) smer1=smer;
  else smer2=smer;
 }
