#include <stdio.h>
#include <conio.h>

void main(void) {
textcolor(11); textbackground(0);
cprintf("\rAles Janda                       NoKomar v1.01                   KYBLSoft 3/2000");
textcolor(14); cprintf("\n\r"
"Navod na vypuzeni komaru: Vypnete pripadne hudby na pozadi, otevrete okno,\n\r"
"zaktivujte NoKomara libovolnou klavesou, odejdete z mistnosti a zavrete za\n\r"
"sebou dvere. Kdyz si uz myslite, ze ten drasavej zvuk speakeru vsechny komary\n\r"
"a jine hmyzy vypudil, vratte se do mistnosti, zavrete okno, ukoncete NoKomara,\n\r"
"pokud je Vam zima, zapnete si topeni a jste v pohode.\n\n\r"
"Pozn.: Pokud to nepomuze, zhasnete v mistnosti, zatahnete zavesy a otevrete\n\r"
"do vedlejsi mistnosti. Pokud tam uz nekdo je, tak tim lip, alespon se ta havet\n\r"
"zabavi a nebude otravovat Vas.\n\r"
"A pokud nepomuze ani to, tak to zkuste napred placackou, pak sprejema, a kdyz\n\r"
"se uspech stale nebude dostavovat, pokropte pokoj svecenou vodou, jelikoz Vam\n\r"
"uz nic jineho nepujde.\n\n\r"
"Takze ted:  Aktivujte program NoKomar libovolnou klavesou.");
_setcursortype(_NOCURSOR); getch();
sound(5000); textcolor(15+128);
cprintf("\rNoKomar je aktivovan, stisknete cokoliv pro ukonceni.     ");
getch(); nosound(); textcolor(14);
cprintf("\rNoKomar je deaktivovan, program ukoncen.                  ");
textcolor(10); cprintf("              \n\n\rOpetovnou prijemnou praci Vam preje Ales Janda, autor.");
textcolor(7); cprintf("              \n\r          \r");
_setcursortype(_NORMALCURSOR);
}