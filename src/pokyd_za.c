/* Tento zdrojovy kod je pod licenci GNU/GPL. Muzete ho pouzit k vlastni
   potrebe, ale nesmite jej ani programy zalozene na tomto kodu vyuzit komercne!

   Jedna se o zdrojovy kod programu Pokyd (http://iqpokyd.kyblsoft.cz)
   od Alese Jandy, aktivne vyvijeneho 1999 - 2002
*/

/*
 * pokyd_za.c - uvitaci napovedy pro zacatecnika (ZACATECNIK), jednorazove zobrazeni podle kodu.
 */

#define POCET_NAPOVED 10

BYTE zacatecnik=0;
BYTE zac_mista[POCET_NAPOVED][8];

/* Zobrazi hlasku pouze pokud je zapnuty zacatecnik a tato napoveda jeste nebyla (mrizka zac_mista). */
BYTE ZACATECNIK(BYTE *hlaska,BYTE kde) {
BYTE misto=kde-1;
  if (zacatecnik == 1 && zac_mista[misto/10][misto%10] == 0) {
    zac_mista[misto/10][misto%10]=1; return(HLASKA(hlaska,5));
   }
  else return(0);
 }

/* Vymaze priznaky zobrazenych napoved (napr. po prikazu uzivatele). */
void VYNULUJ_ZACATECNIK(void) {
BYTE pozice1,pozice2;
  for (pozice1=0; pozice1 < POCET_NAPOVED; pozice1++)
   for (pozice2=0; pozice2 < 8; pozice2++)
    zac_mista[pozice1][pozice2]=0;
 }
