/* Tento zdrojový kód je pod licencí GNU/GPL. Mùžete ho použít k vlastní
   potøebì, ale nesmíte jej ani programy založené na tomto kódu využít komerènì!

   Jedná se o zdrojový kód programu Pokyd (http://iqpokyd.kyblsoft.cz)
   od Aleše Jandy, aktivnì vyvíjeného 1999 - 2002
*/


#define POCET_ZMENENYCH_ZNAKU 9

BYTE zmeneneznaky[POCET_ZMENENYCH_ZNAKU]={
	201,205,187,186,188,200,231,237,251};
//struct REGPACK reg;

void ZMENZNAKY(WORD zacatek,WORD kolik,BYTE *pamet) {
  union REGPACK regs;
  /* Use int86x with explicit segment registers instead of legacy inline
     assembly register saves, which are not available in this Watcom port. */
  memset(&regs,0,sizeof(regs));
  regs.w.cx = kolik;
  regs.w.dx = zacatek;
  regs.w.es = FP_SEG(pamet);
  regs.w.bp = FP_OFF(pamet);
  regs.w.ax = 0x1100;
  regs.w.bx = 0x1000;
  intr(0x10,&regs);
 }

void NASTAVJEDENZNAK(BYTE znak) {
static BYTE misto=0;
BYTE kde=misto<<4;
  ZMENZNAKY(znak,1,dlouhe+kde);
  misto++; if (misto == POCET_ZMENENYCH_ZNAKU) misto=0;
 }

void NASTAVPOKYDFONT(void) {
  /* Temporary safety fallback for DOSBox-X + Watcom build:
     keep ROM font instead of uploading custom glyph tables. */
  NASTAVNORMALNIFONT();
 }

void NASTAVNORMALNIFONT(void) {
  /* Use BIOS default 8x16 ROM font directly. Clear the full REGPACK first:
     leftover BH/BX bits from earlier INT 10h calls have crashed DOSBox-X. */
  memset(&pokyd_regs, 0, sizeof(pokyd_regs));
  pokyd_regs.w.ax = 0x1114;
  pokyd_regs.w.bx = 0;
  intr(0x10, &pokyd_regs);
 }

void NASTAVSPRAVNYFONT(void) {
  if (font == 1) NASTAVPOKYDFONT();
  else NASTAVNORMALNIFONT();
 }
