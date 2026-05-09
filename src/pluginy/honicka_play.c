_AX = 3; geninterrupt(0x10);			//smazani obrazovky

_AH = 1; _CX = 0x2000; geninterrupt(0x10);		//smazani kurzoru

_AX = 0x1003; _BL = 0; geninterrupt(0x10);	//zakaz blikani, 16 pozadi

_AX = 0; geninterrupt(0x33);			//inicializace mysi

_AX = 7; _CX = 0;
 _DX = 2<<3; geninterrupt(0x33);	//nastaveni minima
_AX = 8; _CX = 0;
 _DX = 2<<3; geninterrupt(0x33);	//a maxima pozice mysi

_AX = 0x1A; _BX = 10000; _CX = 20000; geninterrupt(0x33);//nastaveni citlivosti


_AX = 0x1001; _BH = 1; geninterrupt(0x10);	//zmena barvy okraje

_AX = 4; _CX = 1; _DX = 1; geninterrupt(0x33);	//nastaveni kurzoru mysi

gotoxy(1,25); textcolor(15); textbackground(3); cprintf(" Honicka v1.1 - Ales Janda  KYBLSoft 4/2000 ");
textbackground(4); cprintf(" Body:    0/   0  Obt?znost:  0/ 0");

gotoxy(80,25); _AH=9; _BH=0; _AL=' '; _CX=1; _BL=64; geninterrupt(0x10);
						//posledni cerveny znak

gotoxy(1,1);
_AH = 9; _BH = 0; _AL = ' '; _CX = 1; _BL = 32; geninterrupt(0x10);
gotoxy(2,1);
_AH = 9; _BH = 0; _AL = ' '; _CX = 1; _BL = 32; geninterrupt(0x10);
							//nakresleni hrace
gotoxy(79,24);
_AH = 9; _BH = 0; _AL = ' '; _CX = 1; _BL = 64; geninterrupt(0x10);
gotoxy(80,24);
_AH = 9; _BH = 0; _AL = ' '; _CX = 1; _BL = 64; geninterrupt(0x10);
						//nakresleni protihrace
gotoxy(39,12);
_AH = 9; _BH = 0; _AL = '<'; _CX = 1; _BL = 14*16; geninterrupt(0x10);
gotoxy(40,12);
_AH = 9; _BH = 0; _AL = '>'; _CX = 1; _BL = 14*16; geninterrupt(0x10);
						//nakresleni premie

gotoxy(24,1); textcolor(11); textbackground(6); cprintf(" Stiskni cokoliv na zac tek hry ");

/*if ((tab=fopen("HONICKA.TAB","rb")) != NULL) {		//nacteni tabulky
  for (pozicex=0; pozicex < 20; pozicex++) fread(tabulka[pozicex],45,1,tab);
  fclose(tab);
 }*/
getch(); gotoxy(24,1); textbackground(0); cprintf("                                ");

pozicex=1; pozicey=1;
