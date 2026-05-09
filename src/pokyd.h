/* Tento zdrojovy kod je pod licenci GNU/GPL. Muzete ho pouzit k vlastni
   potrebe, ale nesmite jej ani programy zalozene na tomto kodu vyuzit komercne!

   Jedna se o zdrojovy kod programu Pokyd (http://iqpokyd.kyblsoft.cz)
   od Alese Jandy, aktivne vyvijeneho 1999 - 2002
*/

/*
 * pokyd.h - spolecne prototypy fragmentu vkladanych z pokyd.c (jednotka prekladu).
 * Konvence rozsireni: AGENTS.md, .cursor/rules/pokyd-architecture.mdc
 */

/* DOS console split: keep COMMAND.COM / autoexec lines visible above Pokyd (see -consplit). */
extern unsigned char pokyd_shell_rows;
extern int pokyd_intro_argc_snapshot;

void INTRO_CARY(void);
DWORD VOLNAPAMET(void);
void OBNOV_OBRAZOVKU(BYTE jak);
void VRATNAZEVDNE(BYTE *pointer,BYTE velke_pismeno);
BYTE CTIZNAK(void);
void PIS(BYTE x,BYTE y,BYTE *text,BYTE barva);
void NAPISHELP(void);
void ZAPISCAS(void);
BYTE VYBERBARVU(void);
void SMAZKURZOR(void);
void PLNYKURZOR(void);
BYTE CTIBARVU(void);
void NASTAVBARVU(BYTE r,BYTE g,BYTE b,BYTE barva);
void BARVA(BYTE barva);
DWORD CAS18(void);
BYTE TESTSTRANY(void);
void NAPISZNAK(BYTE znak,BYTE barva);
void NAPISZNAK2(BYTE znak,BYTE barva);
void VYMAZZNAKNAOBR(BYTE x,BYTE y);
void NAPISRETEZEC(BYTE *retezec,BYTE barva);
void PREVED_NA_MALA(BYTE *co);

BYTE VETA_PREDTIM(WORD cislovety);
void VYNULUJ_ODPOVEDI(void);
void EXTRA_VETA(BYTE ktera);
void INTRO_POKYDPRUH(void);
void NASTARTUJ_PROGRAM(void);
WORD VRAT_POZICI_VTIPU(void);
void CTI_INFORMACE_O_VETACH(void);
void ZAPIS_INFORMACE_O_VETACH(BYTE co,BYTE nastaveni);
void ZAVRISOUBORSKYDAMA(BYTE *text,BYTE naobrazovku);
BYTE ZJISTI_SVATEK(void);
void PREPNINACELOUOBRAZOVKU(void);
void interrupt INT24(void);
BYTE GENERUJ_ODPOVED(DWORD odkud);
BYTE VYBER_ODPOVED(void);
WORD ODRIZNIENTERY(void);
void OREZ_MEZERY(BYTE *retezec);
void ZMENMOD(void);
BYTE VRATDIAKRITIKU(BYTE pismeno);
void NAPISHLAVICKOVYRADEK(void);
DWORD VRAT_SERIOVE_CISLO(void);
void ZAPISPUVODP(DWORD pozice);
void ODROLUJ(void);
BYTE CTIKLAVESU(void);
void NASTAVKURZOR(void);
void VRATDATA(BYTE poradidat);
BYTE VRATPOZICI(void);
void ZAPISDELKUHOVORU(BYTE naobrazovku);
void CTISOURADNICE(void);
void VLOZSOURADNICE(void);
void SMAZOBRAZOVKU(BYTE jeli_font);
void PRECTIOBRAZOVKU(BYTE efekt);
void ZAPISOBRAZOVKU(void);
void SOUBOR(BYTE *souborcat);
void STRANA(BYTE kolik);
void VYNULOVANI(BYTE jestli_veta);
BYTE BLBOSTI_OPAKOVANE_PISMENO(void);
BYTE SLOVO(BYTE *slovo);
BYTE ZACATEK(BYTE *slovo);
BYTE KONECSLOVA(BYTE *slovo);
BYTE VYSKYT(BYTE *slovo);
BYTE PRIBLIZNA_NALADA(void);
BYTE DOPLN_SLOVO(BYTE ktere,BYTE pozice,BYTE velkepismeno);
void ODP(BYTE *co,WORD cislo);
void POCASI(void);
void NAPISZNAKODP(BYTE znak);
void interrupt (*stara_adresa0)(void);
void interrupt (*stara_adresa1B)(void);
void interrupt (*stara_adresa23)(void);
void interrupt (*stara_adresa24)(void);
void VYPNI_REZIDENTY(BYTE int24);
void PREKRESLI_RADEK(BYTE insert);
void SMAZZNAK(BYTE pozice,BYTE celkem);
void PREDKONEC(void);
void ZMEN_POCITAC(void);
void PREVED_NA_MALA(BYTE *co);
void SLOVO_PAMET(void);
void ZJISTI_SLOVO_PAMET(void);
BYTE SPATNE_SLOVO(BYTE *slovo,BYTE kratke);
void JSEM_PAMET(void);
void ZJISTI_JSEM_PAMET(void);
void JSI_PAMET(void);
void ZJISTI_JSI_PAMET(void);
int SPOCITEJ_NALADU(BYTE zceho);
void ZKONTROLUJ_EXTRA_SANCI(BYTE cheat);
BYTE EXTRA_SANCE_CHEAT(void);

void NAPIS_ODSTAVEC(BYTE *text);
void HELPF1(BYTE typ);
void INTRO_CARA(BYTE pozy,BYTE odkud,BYTE kam);
void INTRO_NAPIS(BYTE pozx,BYTE pozy,BYTE *text,BYTE barva);
void ZAPIS_NALADU(void);
void KONEC(void);
void KONTROLA_UTNUTI(void);
void KALENDAR(BYTE cteni);
void DATUM(BYTE ano_nebo_ne);
void CAS(BYTE ano_nebo_ne);
BYTE HLASKA(BYTE *text,BYTE barvapozadi);
void ZHLTNI_MYS(void);
void CEKEJ(DWORD jakdlouho);
void PIPNI(DWORD frekvence, WORD delka);
void NASTAV50RADKU(void);
void NASTAVSPRAVNYMOD(void);
void HLEDEJSOUBOR(void);
void RUZNASLOVA(BYTE *co,BYTE *naco);
void RUZNASLOVAZAC(BYTE *co,BYTE *naco);
void ODPOVED(BYTE odradkovani);
void SETRID(void);
void NASTAVSOUBOR(BYTE hlaska);
void VTIPYSOUBOR(void);
void VTIPY(void);
WORD NAJDIDALSI(WORD odkud,BYTE okolik);
void REALTIMEKEC(void);
void NAHODAX(WORD misto);
void RODX(WORD misto);
void HLASENI_SPTANEHO_CFG(void);
DWORD CTINASTAVENI(BYTE jeli_zacatek,int minimal,DWORD maximal,int normal);
BYTE VYBER_PLUGINU(void);
BYTE SETRIC_KLAVESA(void);
void SETRIC_POPOSUN_OBRAZOVKU(void);
void SETRIC_OBRAZOVKY(void);

BYTE NACTI_INTELIGENCI(void);
void SMAZ_SOUBORY(BYTE pise);
void FORMATOVANI(void);
void interrupt CTRLBREAK(void);
void ZAPNI_REZIDENTY(void);
void SPUST(BYTE *co,BYTE mali_se_zapsat_obrazovka,BYTE plugin);
void SPUSTPROGRAM(void);
void SMYSL_VETY_POCITACE(void);
BYTE NASTAVENI(void);
void INTRO_NAPISPAMET(void);
void OBRAZEK_POKYDU(BYTE pozicex,BYTE pozicey,BYTE barva,BYTE pruhledne);
void INTRO(int argc,BYTE puvpozy);
void NAPIS_GOTOXY(BYTE x,BYTE y);
void NAPIS(void);
void interrupt INT0(void);
BYTE SAMOHLASKA(BYTE znak);
BYTE VYPUST_E(BYTE *jmeno);
void VRAT_5_PAD(void);

void NASTAVJEDENZNAK(BYTE znak);
void NASTAVPOKYDFONT(void);
void NASTAVNORMALNIFONT(void);
void NASTAVSPRAVNYFONT(void);

void far NASTAV(void);
void ZVYRAZNOVAC(BYTE x,BYTE y,BYTE *text,BYTE kolikate);
void ZVYRAZNOVAC4(BYTE y,BYTE *text,BYTE kolikate);
BYTE MUZ_ZENA(BYTE promenna,BYTE kam);
BYTE NASTAV_NALADU(BYTE promenna,BYTE kam,BYTE mali_se_menit);
void VRAT_NAZEV_NALADY(BYTE *retezec);
void VRAT_NAZEV_CHARAKTERU(BYTE *retezec);
void ZAPIS_CHARAKTER(BYTE kam);
BYTE ANO_NE(BYTE promenna,BYTE kam);
BYTE ANO_NE_RESET(BYTE promenna,BYTE kam);
BYTE ANO_NE_NAHODNE(BYTE promenna,BYTE kam);
BYTE CISLO_MODU(BYTE promenna,BYTE kam);
BYTE ZVOL_BARVU(BYTE barva,BYTE kam);
DWORD NAPIS_CISLO(DWORD hodnota,DWORD jakdlouhe,DWORD min,DWORD max,BYTE kam);
void NAPIS_TEXT(BYTE *jaky,BYTE kam);
void NASTAV_STANDARD(void);
void SMAZ_EFEKT(BYTE kde);

void CTIPODMINKY(void);
BYTE PRECTIPODMINKU(void);
void UKONCI_ZAVORKU(void);

BYTE ZACATECNIK(BYTE *hlaska,BYTE kde);
void VYNULUJ_ZACATECNIK(void);

/* pokyd_llm.c - remote LLM mode via Watt-32 TCP (compiled in only with POKYD_LLM_WATT). */
BYTE LLM_INIT(BYTE *hostport);
BYTE LLM_CONNECT(void);
BYTE LLM_SEND_RECV(void);
void LLM_CLOSE(void);
