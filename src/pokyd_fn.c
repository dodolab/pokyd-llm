/* Tento zdrojovy kod je pod licenci GNU/GPL. Muzete ho pouzit k vlastni
   potrebe, ale nesmite jej ani programy zalozene na tomto kodu vyuzit komercne!

   Jedna se o zdrojovy kod programu Pokyd (http://iqpokyd.kyblsoft.cz)
   od Alese Jandy, aktivne vyvijeneho 1999 - 2002
*/

/*
 * pokyd_fn.c - prace s fontem VGA v textovem rezimu (BIOS INT 10h).
 *
 * Uskali:
 * - Po INT 10h AX=0003 (textmode / nastaveni modu) se znovu nahraje ROM font - je nutne
 *   znovu zavolat NASTAVPOKYDFONT / NASTAVSPRAVNYFONT pred vypisem zavislym na vlastnich glyfech.
 * - VRATDIAKRITIKU() pouziva kody ze zmeneneznaky[] (RAM font), ne "nahodne" CP437 bajty.
 * - ZMENZNAKY: pred volanim typicky vynulovat REGPACK (historicky spatne BH/BX rozbijely DOSBox-X).
 */

#define POCET_ZMENENYCH_ZNAKU 9

/* Cilove pozice ve VGA RAM fontu pro nahrane ceske glyfy (poradi = poradi uploadu v puvodnim Borland kodu). */
BYTE zmeneneznaky[POCET_ZMENENYCH_ZNAKU]={
	201,205,187,186,188,200,231,237,251};

/* Nahraje CX znaku od offsetu DX z pameti ES:BP do generatoru znaku (INT 10h AX=1100h). */
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

/* Starsi rozhrani: jeden glyph z aktualniho offsetu v dlouhe[] do dalsiho slotu zmeneneznaky (cyklicky). */
void NASTAVJEDENZNAK(BYTE znak) {
static BYTE misto=0;
BYTE kde=misto<<4;
  ZMENZNAKY(znak,1,dlouhe+kde);
  misto++; if (misto == POCET_ZMENENYCH_ZNAKU) misto=0;
 }

/* Zkopiruje radek ROM fontu 8x16 (INT 10h AX=1130h, BH=6) do out16[16]. */
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

/* Prida jednoduchy tvar hacku nad prvni neprazdny radek glyfu (napr. s). */
static void pokyd_pridat_hacek(BYTE *g) {
  unsigned top, r;
  for (top = 0; top < 16 && g[top] == 0; top++) {}
  if (top < 2) return;
  r = top - 2;
  g[r] |= 0x3C;
  g[r + 1] |= 0x66;
 }

/* Prida jednoduchou carku (akut) nad glyf - pouzito pro Y/y ze zakladnich Y/y. */
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

/*
 * Nahraje standardni ROM 8x16 a doplni vybrane sloty (s, Y, y) syntetickymi glyfy.
 * Sloty odpovidaji zmeneneznaky[6..8] - musi sedet s VRATDIAKRITIKU().
 */
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

/* BIOS "single plane" nacteni vychoziho 8x16 fontu (INT 10h AX=1114h) - kompletne vynulovat REGPACK. */
void NASTAVNORMALNIFONT(void) {
  /* Use BIOS default 8x16 ROM font directly. Clear the full REGPACK first:
     leftover BH/BX bits from earlier INT 10h calls have crashed DOSBox-X. */
  memset(&pokyd_regs, 0, sizeof(pokyd_regs));
  pokyd_regs.w.ax = 0x1114;
  pokyd_regs.w.bx = 0;
  intr(0x10, &pokyd_regs);
 }

/* Podle nastaveni font bud pokydovsky (ceske glyfy), nebo cisty ROM font. */
void NASTAVSPRAVNYFONT(void) {
  if (font == 1) NASTAVPOKYDFONT();
  else NASTAVNORMALNIFONT();
 }
