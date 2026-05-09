/* Tento zdrojový kód je pod licencí GNU/GPL. Mùžete ho použít k vlastní
   potøebì, ale nesmíte jej ani programy založené na tomto kódu využít komerènì!

   Jedná se o zdrojový kód programu Pokyd (http://iqpokyd.kyblsoft.cz)
   od Aleše Jandy, aktivnì vyvíjeného 1999 - 2002
*/


#define POCET_NAPOVED 10

BYTE zacatecnik=0;
BYTE zac_mista[POCET_NAPOVED][8];

BYTE ZACATECNIK(BYTE *hlaska,BYTE kde) {
BYTE misto=kde-1;
  if (zacatecnik == 1 && zac_mista[misto/10][misto%10] == 0) {
    zac_mista[misto/10][misto%10]=1; return(HLASKA(hlaska,5));
   }
  else return(0);
 }

void VYNULUJ_ZACATECNIK(void) {
BYTE pozice1,pozice2;
  for (pozice1=0; pozice1 < POCET_NAPOVED; pozice1++)
   for (pozice2=0; pozice2 < 8; pozice2++)
    zac_mista[pozice1][pozice2]=0;
 }