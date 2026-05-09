/* Tento zdrojovy kod je pod licenci GNU/GPL. Muzete ho pouzit k vlastni
   potrebe, ale nesmite jej ani programy zalozene na tomto kodu vyuzit komercne!

   Jedna se o zdrojovy kod programu Pokyd (http://iqpokyd.kyblsoft.cz)
   od Alese Jandy, aktivne vyvijeneho 1999 - 2002
*/

/*
 * pokyd_pr.c - globalni promenne a typy ("datova jednotka" bez funkci).
 *
 * Poradi: vklada se jako prvni z fragmentu - vsechny ostatni jednotky k nemu pristupuji.
 * dlouhe[] je sdileny velky buffer (retezce z VRATDATA, docasne vypisy, fontove radky).
 * datasoubor ukazuje na dekodovany obsah ze SLOVNIK.DAT po NACTI_INTELIGENCI().
 */

#define BYTE unsigned char
#define WORD unsigned short
#define DWORD unsigned long


/* --- Generovani odpovedi (aktualni vstup, vyhodnoceni slov, fronta odpovedi) --- */

BYTE veta[40][15],retezec1[81],retezec2[81],odpovedi[20][80];
BYTE puvretezec[81],soubor[80],predtimretezec[81],kydaniny[15];
WORD intpozice;

signed short naladabody=37,samomlbody;
signed short docasnenaladabody;
BYTE nalada=2;			//0 - nejlehci ... 4 - nejtezsi
signed char pocetslov,otazka,souhlas,pozodp;


BYTE slovapamet[50][3][16],slovajsem[50][16],slovajsi[50][16];
BYTE jmenocloveka[15],jmenocloveka5pad[16];
BYTE pocetslpamet,pocetjsem,pocetjsi,cisloslpamet=255,cislojsem=255,cislojsi=255;


/* --- Uzivatelske nastaveni (profil, casovace, jmena, graficke mody) --- */

BYTE ulozeninakonci=1;
BYTE pocetpocitacu=1;
DWORD pocetvterin=5,pocetsetricvterin=180;
signed char zvuk=1,yz=0,psani=2;
DWORD vyska=300;
BYTE akcectrlbreak=0,charakter=3,extrasouborpsani=0,hlasenispatnehocfg=0;
BYTE jmenomuzpocitac[16]="Klaban",jmenozenapocitac[16]="Daria";


signed char svtipy=2,spocasi=0,stranaradek=0,delkastrany;
BYTE obrazovka[50*80*2],samomluva=0,ctrlbreak=0,ctrlbreakc=0,chybaint24=0,smyslpocvety=0;
BYTE textpredsamomluvou[81],spustenipodprogramu=0,pocetradku=2,mod=25,pravopis=1;
DWORD datum=0,pomoc,delayprocenta=100,pocetvet=0,pocetuzivvet=0,cashovoru=0;
BYTE barvapocitac1=10,barvapocitac2=11,barvapocitac0=10,textefekty=1;
BYTE barvaclovek=14,zenskyrod=0,zenapocitac=0,vkolikhodin=16,jeli_nastaveni=0;
BYTE pozicedatumcas,x,y,grafika25,nastvani=1,svatek[35];
BYTE *datasoubor,nazevdne[10],font=1,pozicehlavicka,zapisovani=0,slovenstina=0;
BYTE *inteligence[POCET_IQ],pocetextravet,poslednislovo;
DWORD cas=0-1,*vtipymisto,puvodp[50],cislaodp[20],mistovint,puvodnicas;
WORD puvvtipy[200],pocetiq=POCET_IQ,realodp[20][2],celkemodp,skutecnychodp;
BYTE pocetrealtimekecu,pocetrealtimekecu2,int0=0,zvysovani=1,cekaniklavesy=0;
BYTE cisloaktualnihopocitace=1,realtsamoml=0,verzeslovniku;

/* --- Ostatni runtime stav (vtipy, slovnik, cheaty, priznaky ukonceni) --- */

BYTE nadavani=0;
BYTE nactenyslovnik=0;
WORD pocetsouboru=0;
DWORD celkemvtipu;
BYTE vtipy=0;
BYTE dlouhe[4001];
BYTE masterboot=0;
signed char konec=0;
DWORD pozvtips,pozvtipy,delkavtipu,nahodacislo;
DWORD cislo;


BYTE puvodnipkdmesic=0,puvodnipkdden,puvodnipkdhodina,puvodnipkdminuta;
WORD puvodnipkdrok;
BYTE puvodnipkdzapsani=0,introakcespusteni=0,xtrzapsani=0,xtrzapiskam=0;
BYTE readonlymod=0,vypnutecheaty=0,psanivetyskryto=0;

/* Set before INTRO(): INTRO uses a huge stack frame; Watcom can corrupt the incoming
   argc slot so KONEC sees garbage (e.g. 200), then "argc>=2" runs gotoxy(1,badY) and hangs. */
int pokyd_intro_argc_snapshot = 0;

/* --- Remote LLM mode (pokyd_llm.c / Watt-32) --- */
/* 1 = -llm=host:port was parsed and sock_init() succeeded. */
BYTE llm_enabled   = 0;
/* 1 = TCP connection to the Node bridge is currently open. */
BYTE llm_connected = 0;
/* Bridge host string (hostname or dotted-decimal IP). */
BYTE llm_host[64]  = "";
/* Bridge TCP port. Default 8765. */
WORD llm_port      = 8765;

#if test == 1
DWORD testcyklus;
#endif

struct ffblk ffblk;
struct d { int nuly; long seriove; char rebeliny[20]; } data;

FILE *kydy, *nastaveni, *vtipys, *intel, *vety;

/* Stack size is set by wcl -k... in build.sh (DGROUP ?64KiB; _stklen alone does not override link). */

#define OBRAZEK_POKYDU_X 37
#define OBRAZEK_POKYDU_Y 8

