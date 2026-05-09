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

/* ROM 8x16 via INT 10h AX=1130 BH=6; upload patched glyphs with INT 10h AX=1100 (ZMENZNAKY). */
static void pokyd_zkopiruj_rom_znak(BYTE znak, BYTE *out16) {
  union REGPACK regs;
  BYTE far *font;
  unsigned i;
  memset(&regs, 0, sizeof(regs));
  regs.w.ax = 0x1130;
  regs.h.bh = 6;
  intr(0x10, &regs);
  font = MK_FP(regs.w.es, regs.w.bp);
  for (i = 0; i < 16; i++)
    out16[i] = font[(unsigned)znak * 16u + i];
 }

static void pokyd_pridat_hacek(BYTE *g) {
  unsigned top, r;
  for (top = 0; top < 16 && g[top] == 0; top++) {}
  if (top < 2) return;
  r = top - 2;
  g[r] |= 0x3C;
  g[r + 1] |= 0x66;
 }

static void pokyd_pridat_carku(BYTE *g) {
  unsigned top;
  for (top = 0; top < 16 && g[top] == 0; top++) {}
  if (top == 0) {
    g[0] |= 0x18;
    return;
  }
  if (top >= 2) {
    g[top - 2] |= 0x18;
    g[top - 1] |= 0x30;
  } else {
    g[0] |= 0x18;
  }
 }

void NASTAVPOKYDFONT(void) {
  BYTE g[16];

  NASTAVNORMALNIFONT();

  pokyd_zkopiruj_rom_znak((BYTE)'s', g);
  pokyd_pridat_hacek(g);
  ZMENZNAKY(zmeneneznaky[6], 1, g);

  pokyd_zkopiruj_rom_znak((BYTE)'Y', g);
  pokyd_pridat_carku(g);
  ZMENZNAKY(zmeneneznaky[7], 1, g);

  pokyd_zkopiruj_rom_znak((BYTE)'y', g);
  pokyd_pridat_carku(g);
  ZMENZNAKY(zmeneneznaky[8], 1, g);
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
