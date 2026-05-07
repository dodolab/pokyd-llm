
  --------------------------------------------------------------------------
  |                                                                        |
  |    PPPPPPPPP      OOOOOOOO                 YYY     YYY  DDDDDDDD       |
  |    PPPP    PPP   OOO    OOO    KKK    KKK   YY    YYY   DDDD   DD      |
  |     PPP    PP    OO      OOO  KKK   KKK    YYYY   YYY   DDD     DD     |
  |    PPPP  PPPP   OOOO      OOO  KKK KKK      YYY  YYY   DDD      DDD    |
  |     PPPPPP       OO       OO   KKKKK          YYYY      DDDD   DDDD    |
  |     PPP         OOOO     OOO   KKKK            YY        DD    DDD     |
  |     PPPP        OOO       OO    KKKK            YY      DDD    DD      |
  |     PPP         OOOO     OOO  KKKK KKK          YY       DDD DDD       |
  |    PPP            OOOOO OOO    KKK   KKK       YYYY     DDDDDDD        |
  |                      OOOO     KKKK    KKK                              |
  |                                                                        |
  --------------------------------------------------------------------------

                                          v e r z e      7 . 0  (vydani 2005)
     http://iqpokyd.kyblsoft.cz

                                 Copyright (C) 1999-2005 Ales Janda, KYBLSoft
 
                   Motto:  My kydame, Vy kydate, Pokyd

                          
   Pokyd - jen 549kB veliky kecaci program pro DOS  (pocitano se vsemi soubo-
   =====   ry, ktere jsou v zakladnim baliku).


   =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    \                                                     \
     \             OBSAH tohoto souboru:                   \
      \                                                     \
       \       1.  Licencni podminky                         \
        \       2.  Charakteristika jednotlivych souboru      \
         \       3.  Minimalni naroky na pocitac               \
          \       4.  O co v programu Pokyd vubec jde           \
           \       5.  Co se vsechno da delat?                   \
           /\       6.  Zadavani vet                              \
          /--\       7.  Funkce klaves                             \
         /----\       8.  Nastaveni                                 \
        /------\       9.  Vtipy                                     \
       /--------\      10.  Pluginy                                   \
      /----------\      11.  Ostatni informace                         \
     /------------\      12.  Statistiky slovniku a programu            \
    /--------------\      13.  O programu                                \
   -----------------\                                                     \
                     =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=


*****  1.  Licencni podminky:

   Tento program je FREEWARE.  To v praxi znamena, ze ho muzete vesele sirit,
   ale nesmite ho menit  a musite ho nahravat se vsemi soubory.  Menit se smi
   pouze soubor VTIPY.TXT,  coz je databaze vtipu.  Z tohoto duvodu neberu na
   vtipy zadnou zaruku.  Dale muzete menit  soubor POKYD.CFG,  ktery ale neni
   v zakladnim baliku a nedistribuje se.
   Tento  program  muzete umistit  na sve  stranky,  na CD  nebo ho  i  jinak
   distribuovat,  ale nesmite za samotny nej vybirat nejake poplatky  (netyka
   se  napr. ceny  cedecka).  Take bych byl  velice rad,  kdybyste me o tomto
   kroku informovali.
   Vtipy z Pokydu  se muzou pouzit  i k jinemu ucelu,  ovsem MUSITE me  o tom
   informovat  a na dane strance (programu, cedecku)  musi byt jasne napsano,
   ze tyto vtipy jsou z programu Pokyd + internetova adresa  (viz konec doku-
   mentu).


*****  2.  Charakteristika jednotlivych souboru:

   * Zakladni soubory:
   POKYD.EXE - zakladni soubor, kterym se program spousti
   POKYD.ICO - ikona k programu (vhodna napr. jako obrazek k zastupci ve Win)
   POKYD.TXT - tento textovy soubor, zakladni textovy soubor k programu
   SLOVNIK.DAT - datovy soubor,  ve kterem jsou zapsany data k programu a vse
                 k vygenerovani odpovedi (podminky+odpovedi)
               - bez tohoto souboru se program neobejde
   TESTPKD.EXE - program,  ktery testuje,  jestli neni nejaky soubor poskozen
                 vnejsim zasahem uzivatele
   TESTPKD.TST - soubor pouzivany programem TestPkd,  jsou v nem zapsany jed-
                 notlive soubory a jejich kontrolni soucty
   TESTPKD.TXT - textovy soubor k programu TestPkd
   VERZE.TXT - textovy soubor s udajema  o jednotlivych verzich Pokydu - his-
               torie, zmeny do budoucna, nejdulezitejsi zmeny vubec
   VTIPY.TXT - textovy i datovy soubor, ve kterem jsou napsany vtipy
             - tento soubor muze uzivatel editovat, vice v kategorii "Vtipy"
             - nekontroluje ho TestPkd

   * V adresari PLUGINY:
   CAS.EXE - editor systemoveho casu a data
   COOKEDIT.EXE - editor faktu, ktere si Pokyd behem hovoru zapamatoval
   HAD.EXE - znama hra az pro 2 hrace s moznosti hrani s pocitacem
   HONICKA.EXE - hra  ovladana mysi,  ve ktere  musite  neustale utikat  pred
                 nepritelem a sbirat bonusy
   KALKUL.EXE - kalkulacka s 5 funkcemi a zavorkami
   NAJDIME.EXE - vyhledevac jmen v kalendari podle jmena nebo data
   NAJDIME.KAL - vypis jmen z kalendare pouzivane pluginem NajdiMe
   NOKOMAR.EXE - odpuzovac komaru a jineho hmyzu z mistnosti
   PALBA.EXE - strileci hra proti pocitaci
   PISMENA.EXE - hra procvicujici orientaci na klavesnici
   PLUGINY.TXT - textovy soubor ke vsem pluginum
   SILNICE.EXE - hra, ve ktere prechazite rusnou silnici

   * Ostatni soubory:
   POKYD.CFG - soubor, ve kterem je ulozeno nastaveni programu
   PUVODNI.PKD - datovy soubor  obsahujici binarni cisla  jiz drive pouzitych
                 50 vet a 200 vtipu, aby se program neopakoval.
   PROFIL.PKD  - obsahuje az 50 spojeni typu PODMET PRISUDEK PREDMET  (jinak:
                 PODSTJMENO SLOVESO PODSTJMENO), az 50 slov, kterymi se ohod-
                 notil uzivatel a az 50 slov, kterymi clovek naopak ohodnotil
                 pocitace.
   Adresar KYDY - do tohoto adresare se zapisuji jednotlive soubory s hovorem
                  (ma-li byt zapisovan - viz nize)

    Pozn.: Tyto soubory se nedistribuji, protoze se na novem pocitaci vygene-
           ruji samy v zavislosti na novem uzivateli


*****  3.  Minimalni naroky na pocitac:

   Procesor:          286 nebo vyssi
   Pamet:             260 kB konvencni pameti
   Monitor:           VGA a vyssi, s omezenim lze i na CGA
   Operacni system:   DOS verze 4.0 a vyssi nebo jakakoliv verze Windows
                      (v dobe vzniku programu Win 3.x/95/98/ME/NT/2000/XP)
   Velikost disku:    priblizne 549 kB (v nejhorsim jezdi i pri 201kB)
   Zvukove zarizeni:  PC speaker (nemusi byt)
   Jina zarizeni:     Mys (mimo Honicky nemusi byt)


*****  4.  O co v programu Pokyd vubec jde:

   Je to jednoduchy. Timto programem muzete kecat s vasim pocitacem! Muze byt
   muz i zena a on i vy se stridate po napsani jedne nebo i vice vet.
   Pochopitelne, nemuzete cekat totalni inteligenci, preci jenom je to stroj,
   ale  rozumi dosti zakladnim slovum  a podle toho  vybere nejlepsi odpoved.
   Krome toho si postupem casu  zapamatuje urcite veci,  ktere dokaze pouzit,
   cimz se znacne zkvalitni hovor.
   Nikdy neopakuje vetu dvakrat za sebou  (ani ob vetu),  minimalni opakovani
   jsou 2 vety mezi. Umi spravne zareagovat na 233 moznosti slovnich spojeni,
   na ktere muze odpovedet celkem 2330 vetami.


*****  5.  Co se vsechno da delat?

   Na rozdil od jinych "kecacich" programu  tento navic umi napr. predpovidat
   pocasi. Staci napsat

    POCASI

   a pocitac zapne virtualni radio a vy se dozvite, kolik bude stupnu, jestli
   bude zatazeno,  biopredpoved i stav ozonu.  Ale neberte to na velkou vahu,
   je to jenom priblizne. O svatky je spatny signal prijmu radia.

   Take se muzete pobavit napsanim vtipu.
   Zadejte

    REKNI VTIP

   a pocitac vybere jeden z nahodne vybranych vtipu.  Dalsi informace najdete
   v tomto souboru v kapitole Vtipy (viz nize).

   V pripade, ze vas ten hnusny zvuk na speakeru uz drasa, muzete ho vypnout.
   Staci napsat

    VYPNI ZVUK
     nebo
    ZAPNI ZVUK

   Pocitac provede vami zadanou instrukci a vy budete zase v pohode.

   Pozn.:  Zapnout ci vypnout zvuk muzete take rychle stisknutim Alt-S.

   Jako temer vsechny programy, i tento ma nejake nastaveni.
   Pro tento ucel napiste

    NASTAV

   a ukaze se vam prostredi,  ve kterem si muzete nastavit dane prvky (o nas-
   taveni se rozepisuji nize).

   * Tip: pro rychle nastaveni bez ztraty radku zmacknete F4.

   Muzete dat take:

    NASTAV STANDARDNE

   a ulozi se standardni nastaveni.
   Dalsi moznost je:

    ULOZ NASTAVENI

   a ulozi se aktualni nastaveni.

   * Tip: pro rychle ulozeni nastaveni bez ztraty radku zmacknete Shift-F9.

   Pozn.: Toto nastaveni se da  take zmenit  modifikovanim souboru  POKYD.CFG
          nebo ulozenim v prostredi nastaveni.
          Pokud chcete automaticky ukladat nastaveni pri skonceni programu,
          prepnete  v nastaveni parametr  "ulozit nastaveni pri skonceni pro-
          gramu" do polohy "ano".

   Kdyz uz vas nebavi kydat, muzete pocitac nechat mluvit s druhym pocitacem.
   Druhy pocitac je stejneho pohlavi jako ten prvni.
   Napiste:

    MLUV S DRUHYM POCITACEM
   nebo
    MLUV SAM SE SEBOU
   nebo
    TRP SAMOMLUVOU

   a pocitac si bude tak dlouho kydat sam pro sebe,  dokud nezacnete zase vy
   manualne psat "svoje" vety.
   Pozn.:  Lze take vyvolat z prostredi pluginu (klavesa F2).

   Pokud chcete v prubehu Pokydu spustit jiny program, napiste:

   1.  SPUST <prikaz nebo program>
      nebo
   2.  SPUST *<prikaz nebo program>
      nebo
   3.  SPUST #<program.pripona>

   * Pripad cislo 1:
   Program se automaticky presune  do adresare s danym souborem  a spusti ho.
   Po ukonceni se presune zpet.
   * Pripad cislo 2:
   Podprogram se spusti neformatovane, tj. tak, jako kdybyste prikaz za hvez-
   dickou napsali do normalniho prikazoveho radku
   * Pripad cislo 3:
   Spusti program (ne prikaz!) primo,  ne pres command.com.  Vyhoda je v tom,
   ze na  samotny program  zbyde  vice pameti  a jeho spusteni  je rychlejsi.
   Tento prikaz je nutno zadavat i s priponou, nesmi se jednat o davkove sou-
   bory.  Pokud neni soubor  na zadane ceste,  hleda se  v systemove promenne
   PATH.  Program se spousti neformatovane,  tj.  napred neskoci  do zadaneho
   adresare. U tohoto pripadu nelze pouzivat argument! Take nemusi vsude cho-
   dit programy pro Windows.  Zadany soubor vsak nemusi mit spustitelnou pri-
   ponu.

   Pozn.:  Za nazvem programu muzete normalne psat argumenty.
           Kdyz napisete pouze SPUST bez parametru,  spusti se novy prikazovy
           radek.
           Klicove slovo "spust" se take nabidne zavolanim funkce z prostredi
           pluginu (dostanete se klavesou F2).
          
   Pokud si z nekoho  chcete udelat  legraci,  je zde jeste moznost  simulace
   formatovani harddusku. Staci zadat:

    FORMATUJ

   a pan Format uz se cini.
   Pozn.: Pri formatovani  "nechroupe" harddisk.  Z tohoto duvodu  doporucuji
          prehravat pritom nejakou hudbu (ale to zase nevypada tak strasne).
          Tuto funkci muzete take spustit z prostredi pluginu  (tam se dosta-
          nete klavesou F2).

   Kdyz uz se vam zda,  ze tech souboru s hovory mate moc  a nechce se vam je
   rucne mazat, muzete pouzit prikaz

    VYMAZ SOUBORY
   popr.
    SMAZ KYDY

   Vsechny tyto soubory se smazou mimo toho,  do ktereho se zapisuje (zapisu-
   je-li se). Na tento soubor se program extra zepta.
   Pozn.: Vymazou se i soubory bez hovoru,  zkratka vsechny, ktere nemaji at-
          ribut Read Only (Jen ke cteni). Nemazou se adresare.
          Tuto funkci muzete zavolat z prostredi pluginu,  do ktere se dosta-
          nete stiskem klavesy F2.

   Kdyz se nejak zkazi font, zvlaste se to muze stat pri prepinani oken ve
   Windows (a stava se to!), muzete zadat jednoduchy prikaz:

    OBNOV FONT

   Font by ted mel byt uz v poradku.  Jestlize neni,  musite se s tim smirit,
   nebo to spravit nejak jinak, napr. opakovanym prepinanim oken.

   * 1. tip: Tento prikaz muzete zavolat take klavesou F9 bez ztraty radku.
   * 2. tip: Pokud chcete "poradne" obnovit obrazovku, zmacknete Ctrl-F9.

   Jestli vam nevyhovuje  vyska radku  (zda se vam,  ze jich tam je moc  nebo
   malo, muzete napsat

    ZMEN MOD

   V pripade, ze je nastaveno 25 radku, bude jich 50 a naopak.
   Zmena poctu radek na 50 pujde pouze na karte VGA a vyssi,  na nizsich kar-
   tach zustane 25 radku.
   * Tip: Zmenit mod muzete rychle stiskem Alt-F9.

   Pokud chcete spustit testovaci program Pokydu, zadejte prikaz

    OTESTUJ SE

   Program se spusti na stejne ceste, jako je Pokyd  (jmenuje se TESTPKD.EXE)
   a vy zjistite, zda mate spravnou kopii programu.

   Jestli honem chcete vedet  informace o programu bez toho,  abyste skoncili
   program a prohledli si tento dokument, napiste:

    POMOC, HELP, CREDITS
     nebo proste jenom
    ?

   Strucna napoveda se vzapeti napise na novou obrazovku.
   Pozn.: Rychle zhlednuti bez ztraty radku muzete vyvolat klavesou F1.

   No a kdyz chcete program ukoncit, napiste

    AHOJ, CAU, NAZDAR, SKONCI, KONEC, PRYC, SYSTEM, EXIT, RETURN

   nebo podobnymi prikazy.

   * Tip 1:
         Program  muzete  ukoncit  take klavesou  F10  ci kombinacemi  klaves
         Alt-X, Alt-K, Alt-Q, Ctrl-Q, Alt-F4 a Alt-F3.

   * Tip 2:
         Kdyz chcete opravdu RYCHLE vylezt,  muzete utnout program  klavesami
         Ctrl-Break, pripadne Ctrl-C.  Program stisk pozna a hlaseni o preru-
         seni napise i do souboru, je-li hovor zapisovan.
      Pozn.: Kdyz program zamrzne  a nereaguje ani  na zmacknuti  Ctrl-Break,
             stisknete Ctrl-Break 3x.  To vyvola predcasne ukonceni, ktere je
             ovsem krajne nouzove a dost nestabilni  -  jinymi slovy: program
             se sice ukonci, ale pocitac nejspis stejne zatuhne.

        
   Jinak si muzete zadavat prikazy jake chcete  a program budto bude rozumet,
   nebo nebude. To si vyzkousejte sami.


*****  6.  Zadavani vet

   Pri psani vet  muzete pouzit  mnoho zjednoduseni,  s kterymi  muzete  psat
   rychleji. Meli byste si zapamatovat tyto 3 body:

   -  v otazkach MUSITE psat otaznik! Znacne se tim zkvalitni rozhovor
   -  z prikazoveho radku DOSu nepiste diakritikou (v prostredi to ani nelze)
   -  pouzivejte klavesove zkratky.  Je to pohodlnejsi  (viz nize  v kapitole
      Funkce klaves)

   Vety specifikujte co nejpresneji, napiste je pokud mozno celou vetou.

   Snazte se psat jen hole vety, pokud je to mozne. Nepiste vice vet.



*****  7.  Funkce klaves:

   * Pri psani vety:

     pismena, cisla, znaky - psani vety
     Enter - potvrzeni napsane vety (nelze napsat prazdnou vetu)
     Backspace - mazani vlevo od kurzoru
     Del - mazani vpravo od kurzoru
     sipky - pohyb kurzorem po vete
     Ctrl-sipky - pohyb kurzorem po slovech
     Home - skok kurzorem na zacatek vety
     End - skok kurzorem na konec vety
     Esc - vymazani aktualni vety
     Insert - zapne/vypne mod prepisovani
     Ctrl-Backspace - smazani slova pred kurzorem

     F1 - napoveda
     F2 - prehled a vyber pluginu
     F3, Ctrl-E - opakovat minulou vetu
     F4 - prohlizeni a editovani nastaveni
     F5 - navrat na pocatecni (uvodni) obrazovku
     F6 - spusteni setrice obrazovky
     F7/F8 - zlepsit/zhorsit naladu pocitace
     F9 - obnoveni fontu a cele obrazovky
     Ctrl-F9 - poradne obnoveni fontu a cele obrazovky  vcetne napsani svatku
               a data a casu
     Alt-F9 - zmena poctu radku (25/50)
     Shift-F9 - ulozeni aktualniho nastaveni
     Alt-C - zmena pohlavi cloveka
     Alt-P - zmena pohlavi pocitace
     Alt-S - zapnuti/vypnuti zvuku
     Alt-F - prepinani mezi standardnim fontem a fontem Pokydu
     Alt-E - vypnuti/zapnuti textovych efektu
     Alt-V - zobrazeni cisla verze programu, jeho vydani a verze slovniku
     F10, Alt-X, Alt-K, Alt-Q, Ctrl-Q, Alt-F4, Alt-F3 - konec programu
                                                        (po potvrzeni dotazu)

   * V prostredi pluginu:

     sipky - pohyb kurzorem nahoru a dolu
     Enter, mezera - spusteni pluginu u kurzoru
     <cislo> nebo Alt-<cislo> - rychle spusteni pluginu, kde <cislo> je pora-
                                dove cislo pluginu; cislovani pluginu je 1-9,
                                posledni plugin je 0.
     S,V,K,F - na prikazovy radek nabidne akci odpovidajici zvyraznenemu pis-
               menu v textu
     F1 - napoveda
     F6 - spusteni setrice obrazovky
     Esc - navrat do predchozi obrazovky

   * V prostredi nastaveni:

     sipky - pohyb kurzorem nahoru a dolu;  pri pohybu za okraj nastaveni  se
             ukazou dalsi parametry nastaveni
     Enter, mezera - nastaveni parametru u kurzoru
     Tab, F4 - prepnuti do obrazovky dalsich parametru; ma stejny vyznam jako
               pri presunuti kurzoru za okraj nastaveni
     PageDown - presun na dolni radek obrazovky, pripadne na dalsi obrazovku
     PageUp - presun na zacatek obrazovky, pripadne na predchozi obrazovku
     Home - presun na zacatek prvni obrazovky
     End - presun na spodni radek druhe obrazovky
     Zvyraznena pismena - nastaveni parametru zvyrazneneho danym pismenem
     U, Shift-F9 - ulozeni nastaveni
     S - vsechny parametry na obou obrazovkach nastavi standardne (po dotazu)
     N - nacte to nastaveni, ktere je aktualne ulozene v souboru POKYD.CFG
     F6 - spusteni setrice obrazovky
     F1 - napoveda
     K, Esc - navrat do predchozi obrazovky
     leve tlacitko mysi - zmena polozky, na ktere se nachazi kurzor mysi
     prave tlacitko mysi - prepnuti do druhe obrazovky nastaveni;  ma stejnou
                           funkci jako klavesa F4

   * V uvodni obrazovce:

     F1 - zobrazi napovedu k Pokydu
     F2 - prostredi pluginu
     F3 - zmeni pohlavi pocitace
     N, F4 - prostredi vsech parametru nastaveni
     F5 - nastavi/vypne automatickou zmenu nalady podle hovoru
     F6 - spusteni setrice obrazovky
     F7 - zlepsi naladu pocitace
     F8 - zhorsi naladu pocitace
     F9 - zapne hlasky pro zacatecniky  (u ruznych akci se zobrazuje napoveda
          formou hlasek)
     U, Shift-F9 - ulozi nastaveni
     P - zmena pohlavi u cloveka
     T - spusti se  externi testovaci program  TestPkd,  zda-li  jsou vsechny
         soubory k Pokydu spravne
     Esc, F10 - navrat do predchozi obrazovky (pokud byl Pokyd prave spusten,
                ukonci se
     K, Alt-Q, Alt-K, Alt-X, Alt-F3, Alt-F4 - ukonceni programu


   Poznamky:
   Ctrl-Alt (kdykoliv behem programu) - zrychli psani textu na obrazovku

   Pokud mate zapnutou ceskou klavesnici a pisete s hacky a carkami,  program
   je sam prevede na stejna pismena bez diakritiky.

   Pri nezmacknuti niceho v prubehu n sekund (lze nastavit) se napise upozor-
   neni na to, ze pocitac je jeste tady a abyste se mu venoval(a).


*****  8.  Nastaveni

   Jako v jinych programech,  i tento ma nejake nastaveni.  Pro objasneni  tu
   uvedu vsechny parametry najednou.

   Jednotlive prvky jsou tyto:
---------------------------------*

   * Pohlavi uzivatele - toto nastaveni  zjistuje,  je-li  uzivatel  muz nebo
          zena a podle toho ho oslovuje.
          Pozn.:  1. Rod take vidite na prouzku  vedle casu.  Je-li na pozadi
                     modra barva, je nastaven muz, pri cervene zena.
                  2. Pohlavi cloveka  lze take rychle zmenit  primo pri psani
                     vet klavesami Alt-C.
        Standard: muz

   * Pohlavi pocitace - zda-li ma pocitac predstavovat muze nebo zenu.
          Pozn.:  1. Rod take vidite na prouzku  vedle casu.  Jsou-li srdicko
                     a vykricnik zlute, je nastaven muz, pri cervene zena.
                  2. Pohlavi pocitace lze take rychle zmenit  primo pri psani
                     vet klavesami Alt-P.
        Standard: muz

   * Nalada pocitace - udava  aktualni naladu pocitace.  Kdyz chcete,  aby na
          vas byl hodny, dejte si vybornou a pocitac vas bude spise utesovat.
          Pri normalni nalade je pocitac stejny jako prumerny clovek,  a kdyz
          se chcete vylozene hadat,  zvolte si hroznou.  Pocitac bude odporo-
          vat, nadavat atd. Muzete si samozrejme nastavit i dobrou nebo spat-
          nou naladu, coz jsou meziclanky, ale to uz zavisi jenom na vas.
          Pozn.: Toto nastaveni muzete menit primo pri hovoru klavesami F7-8.
                 Take ho vidite na prouzku vedle casu  (cim blize je znak sr-
                 dicku, tim je nalada lepsi).
        Standard: normalni

   * Charakter pocitace - rika, jaky ma byt pocitac, jestli se ma snadno roz-
          cilit ci naopak zustat klidny za vsech okolnosti.  Kdyz je nastaven
          charakter "stroj", pocitac vubec nemeni svoji naladu. Druhy charak-
          teru jsou tyto:  stroj, naivni, klidny, prumerny, neduverivy, nala-
          dovy, vybusny.
        Standard: prumerny

   * Jmeno pocitace muzskeho rodu - zde  zadate jmeno,  ktere ma mit  pocitac
          jakozto muz. Jmeno musi mit delku v rozmezi 1 - 15 znaku.
        Standard: Klaban

   * Jmeno pocitace zenskeho rodu - stejne jako u predchoziho pripadu,  pouze
          se to tyka pocitace jakozto zeny.
        Standard: Daria

   * Zapojeni druheho pocitace do hovoru pri mlceni - kdyz je nastaveno "ano"
          a vy budete dlouho mlcet,  zapoji se do hovoru  druhy pocitac.  Pri
          preruseni jejich hovoru vami se bavite s tim pocitacem,  ktery rekl
          posledni vetu.
        Standard: ne

   * Kdyz je hodne nastvany, ukonci se -  kdyz vas bude mit pocitac az po krk
          a muze menit naladu podle situace (viz vyse), lze si nastavit,  aby
          se nastval  mirou nejvyssi  a ukoncil hovor.  Je to celkem prijemne
          zpestreni hovoru, avsak nekomu se takovy konec nemusi moc libit.
        Standard: ano

   * Aktivni zvuk na speakeru - je-li zapnut, hraje na speakeru. Nemusi hrat,
          kdyz je malo procent z rychlosti odpovidani (viz nize).
          Pozn.: Zvuk lze zapinat/vypinat take behem hovoru klavesami Alt-S.
        Standard: ano

   * Pocet vterin, po kterych reknu dalsi kec - po uplynuti teto doby ve vte-
          rinach rekne pocitac dalsi vetu k tematu. Nula znamena, ze se nebu-
          dou psat tyto vety vubec.
        Standard: 5

   * Zvysovani vyse popsaneho poctu vterin o jednu - kdyz je  tento  parametr
          nastaven,  bude se prodleva mezi vyse zminenymi vetami  zvysovat po
          1 vterine.
        Standard: ano

   * Rychlost psani odpovedi v procentech - dosti neobvykly,  mozna tezko po-
          chopitelny, ale dosti zabavny i uzitecny parametr.
          Priklad: na napsani jednoho pismena je cekaci doba 30 ms (pri 100%)
          a veta o 40 znacich tedy trva 30 * 40 = 1200 ms, coz je pres 1 vte-
          rinu.  Pri zadani 50% trva  jednomu pismenu k napsani 15 ms  a veta
          o 40 znacich se tak napise za 15 * 40 = 600 ms, takze to bude trvat
          2x mene. Tyto cekaci doby plati i u jinych operaci, ale to neni tak
          viditelne.
        Standard: 100

   * Pouzivani vlastniho fontu Pokydu - tento parametr urcuje, zda-li ma pro-
          gram pouzivat vlastni font, nebo jestli ma zustat font puvodne nas-
          taveny. Pri pouzivani fontu Pokydu se pouziva v radku s nazvem pro-
          gramu diakritika,  pri spusteni podprogramu  nebo ukonceni  se font
          automaticky vypne.  Tento parametr  nema smysl  u 50radkoveho modu,
          tam je font porad standardni.
          Pozn.: Toto nastaveni lze menit i pri hovoru klavesami Alt-F.
        Standard: ano

   * Kolikrat se ma odradkovat po vete - pocet radku (min. 1), ktere se odro-
          luji po napsani vety clovekem ci pocitacem.
        Standard: 2

   * Pouzivani efektu  v programu  (napr.  vypisovani vet) -  tento  parametr
          udava,  zda-li se maji pri standardnim vypisovani vet pismena efek-
          tivne objevovat  a vypisovat.  Totez plati i na  rolovani obrazovky
          apod.  Tato vlastnost se take teoreticky da zakazat, pokud je odpo-
          vidaci doba v procentech 0,  potom jsou ovsem vsechny operace maxi-
          malne rychle. 
        Standard: ano

   * Za kolik vterin spustit setric obrazovky  -  udava,  za jakou dobu se ma
          sam spustit setric obrazovky. Pokud zadate hodnotu 0, setric se sam
          nespusti  nikdy.  Setric  se spusti pouze,  kdyz pisete vetu,  jste
          v prostredi napovedy,  pluginu,  nastaveni nebo v uvodni obrazovce.
          Nespusti se napr.  kdyz se vas pocitac na neco pta (typickou "hlas-
          kou").
          Setric lze take rychle spustit pri psani vety klavesou F6.
        Standard: 180  (3 minuty)

   * Pocatecni obrazovkovy mod (pocet radku) - zde nastavujete pocet radku na
          obrazovce,  cili jak velika budou pismena  a kolik se jich vejde na
          obrazovku. Je zde i hodnota "puvodni". Pri tomto nastaveni bude po-
          cet radku takovy, jako pri spusteni, tj. nemeni se.  Pozor! Toto je
          pouze  pocatecni nastaveni,  okamzita zmena poctu radku  se provadi
          behem psani klavesami Alt-F9 nebo prikazem ZMEN MOD.
        Standard: 25

   * Aktivni slovenstina - urcuje,  zda-li ma typicke  slovenske slova  (som,
          bol atd.) nahradit ceskymi a tim vete lepe porozumet.
          Pozn.: Kdyz je zapnuta tato funkce a uzivatel rekne,  jak se jmenu-
                 je, pocitac ho bude oslovovat 1. padem (tedy jako Slovaka).
        Standard: ne

   * Tolerance spatneho pravopisu nekterych slov - pokud  nejste  cestinarsky
          nadan a pisete s chybami, mel byste si zapnout toto nastaveni. Nek-
          tera nejuzivanejsi slova spatne napsana bude brat jako dobre napsa-
          na.  Dale taky pri vyskytu nejcastejsich otazkovych slovech  (proc,
          jak atd.), kdyz zapomenete napsat otaznik,  se veta automaticky oz-
          naci jako otazka.  Pokud ale nepisete s hrubymi chybami  a otazniky
          nezapominate, mel byste to mit vypnute, protoze pocitac muze nejake
          klicove slovo prevest na jine nebo vetu oznamovaci brat jako otazku
          a potom samozrejme nerozumi spravne.
        Standard: ano

   * Zapis hovoru do souboru - program  pise hovor  pouze tehdy,  jestli toto
          nastaveni bylo jiz pri spusteni,  kdy pro nej najde spravny soubor.
          Blizsi informace o tomto souboru viz nize.
        Standard: ano

   * Kazdy hovor zapisovat do extra souboru - pokud je zapnute zapisovani ho-
          voru do souboru,  lze si timto parametrem nastavit,  zda-li se maji
          vsechny hovory psat do jednoho souboru (KYDY\KYDY.TXT) nebo pokazde
          do extra souboru.
        Standard: ne

   * Nadmorska vyska v metrech - pouziva se pro spravnou predpoved pocasi.
        Standard: 300

   * Psani vtipu na startu - muze byt i nahodne. Vtip se napise, i kdyz zaci-
          nate hovor a az potom odpovi.
        Standard: nahodne

   * Psani predpovedi pocasi na startu - plati pro ni ty same podminky jako u
          psani vtipu.
        Standard: ne

   * Od kolika hodin se ma psat predpoved pocasi na zitrek - urcuje, v kterou
          denni dobu (od kolika hodin) vas zajima spise  pocasi na dalsi den,
          nez na dnesni den.
        Standard: 16

   * Prohozeni klaves Y a Z  -  pri psani vety prohazuje tyto 2 klavesy  (pro
          anglicky pisici uzivatele na ceske klavesnici a naopak).
        Standard: ne

   * Cekani klavesu pri ukonceni programu - pokud je nastaven tento parametr,
          program jeste pred uplnem ukonceni ceka na klavesu, aby si uzivatel
          mohl precist statistiku hovoru.
        Standard: ne

   * Ulozit nastaveni pri ukonceni programu - je-li aktivni, pri kazdem ukon-
          ceni se ulozi  aktualni nastaveni,  a to vcetne  momentalni nalady.
          To tedy znamena, ze pokud mate nastaveno napr. dobrou naladu a zme-
          nu nalady podle aktualni situaci a nastvete ho,  nalada se zhorsi a
          taktez se i ulozi.
        Standard: ano

   * Pri ukonceni Ctrl-Breakem smazat obrazovku  -  nahrazuje  tzv. BOSS key,
          ktera slouzi zamestnancum k rychlemu ukonceni nepovoleneho programu
          pred blizicim se sefem.  Tady slouzi tak,  ze je-li  tento parametr
          nastaven na hodnotu  "ano",  pri ukonceni klavesami  Ctrl-Break  ci
          Ctrl-C se ihned smaze obrazovka a pote se program ukonci.  Pokud je
          nastaveno "ne", program se take ukonci, ale obrazovka se nesmaze.
        * Hodnota "reset": v DOSu se restartuje pocitac a ve Windows se okam-
          zite uzavre okno  (a to i kdyz je Pokyd spusten z jineho programu).
          Jde tedy o nejtvrdsi ukonceni programu.
          Pozn.: - tato funkce neni 100% zarucena ve vsech systemech
                 - pred resetnutim se normalne ulozi nastaveni
        Standard: ne

   * Barva textu psaneho clovekem - vyber z 15 barev, je to barva, jakou pise
          uzivatel, clovek, tedy vy.
        Standard: 14

   * Barva textu psaneho pocitacem 1  -  plati  stejne podminky jako  u textu
          cloveka, je to barva odpovedi pocitace 1.  Tento pocitac s clovekem
          mluvi standardne.
        Standard: 10

   * Barva textu psaneho pocitacem 2  -  barva  u pocitace #2.  Tento pocitac
          mluvi v pripadech:
           - dlouho nic nerikate, takze se vybavuje s prvnim pocitacem
             (musite mit zapnuto parametr "zapojeni druheho pocitace...")
           - v predchozim pripade jste hovor prerusili, kdyz mluvil pocitac 2
           - pocitac mluvi sam se sebou (s druhym pocitacem) na vas prikaz
        Standard: 11


   V konfiguracnim souboru je na zacatku jeste jedna polozka:

   * Seriove cislo disku C - podle tohoto ctyrbajtoveho cisla vyjadrujici se-
          riove cislo program pozna,  zda-li Pokyd nebyl prenesen na jiny po-
          citac.  Ne, ze by se to snad nemelo,  ale program pouze upozorni na
          to,  aby si novy uzivatel sam nastavil program dle sveho  a nabidne
          se i kurz pro zacatecniky  (viz nize).  Pokud toto cislo prepisete,
          bude vas brat jako noveho uzivatele, coz se zmeni, az kdyz nastave-
          ni znovu ulozite. Tento parametr se nezadava v nastaveni.
        Standard: neexistuje


   Dole jsou vybery "Ulozit",  coz ulozi nastaveni,  "Nacist nastaveni" nacte
   tu konfiguraci,  ktera je ulozena v souboru (tedy tu,  kterou jste ulozili
   naposledy), "Standardni nastaveni" zase nastavi standardni nastaveni a pri
   polozce "Konec" se prostredi nastaveni ukonci.

   Pozn.: Kdyz neexistuje soubor POKYD.CFG  ci v nem nesouhlasi seriove cislo
          (viz vyse),  nabidne se tzv. amatersky kurz pouzivani programu  pro
          zacatecniky (cokoli udelate, k tomu se napred ukaze napoveda).
          Tento kurz lze i kdykoli zapnout v uvodni obrazovce klavesou F9.


*****  9.  Vtipy

   Vtipy lze volne editovat, pridavat i ubirat. Jsou napsane v souboru
   VTIPY.TXT.  Je to mimochodem  jediny soubor,  ktery se siri dal  a muze se
   editovat.
   Format:  Kazdy vtip zacina znakem # a Enterem.
   Chcete-li tedy pridat vtip, staci napsat

    #
    Vtip

   nebo taky

    #komentar vtipu
    Vtip

   Za normalnich  okolnosti  (dostatek pameti)  lze nacist  az  16.384 vtipu,
   z nichz kazdy ma delku max. 4000 znaku. S Pokydem se standardne dodava 654
   vtipu.
   Pozn.: Program si vtipy "nenacte",  pouze si zapamatuje mista,  kde v sou-
          boru zacinaji a vtipy "taha" rovnou ze souboru.  Je tedy nutne, aby
          soubor s vtipy byl v nezmenene podobe stale na disku.


*****  10.  Pluginy

   K Pokydu jsou dodany pridavne pluginy v adresari PLUGINY. Jsou to:

    COOKEDIT - umoznuje editovat fakta,  ktere si Pokyd  behem hovoru o vas a
               vasem okoli zapamatoval.  Tento plugin je urcen zejmena pokro-
               cilejsim a program dlouho uzivajicim uzivatelum.
    CAS - pomoci tohoto programu si muzete presne a snadno nastavit systemovy
          cas na vasem pocitaci. Obsahuje specialni funkci "zastavit cas",
          ktera jeste zjednodusuje nastavit opravdu presny cas.
    HAD - znama hra,  ve ktere  ovladate hada,  zerete bonusy  a postupne  se
          prodluzujete.  Je zde  moznost hrani  soucasne  dvou hracu  a hrani
          s pocitacem.
    HONICKA - hra ovladana mysi,  kde vas jakozto zeleny ctverecka honi "cer-
              venak".
    KALKUL - standardni kalkulacka. Staci zadat priklad, ktery muze obsahovat
             operace +,-,*,/,^ a libovolne zavorek a program ho spocita.
    NAJDIME - vyhledavac jmen v kalendari.  Lze hledat podle casti jmena nebo
              data.
    NOKOMAR - odpuzovac komaru z mistnosti.  Pokud mate v pokoji podobnou ha-
              vet, zkuste tento program.
    PALBA - zabavna strileci hra,  ve kterem mate rozstrilet sveho protivnika
            driv, nez to udela vam.
    PISMENA - hra na  procvicovani klavesnice,  kde musite stisknout  zadanou
              klavesu driv, nez "spadne" dolu.
    SILNICE - hra,  ve ktere musite prejit rusnou silnici, aniz by vas cokoli
              srazilo.

   Pozn.:   Blizsi  informace  k jednotlivym  pluginum  naleznete  v  souboru
            PLUGINY.TXT v adresari PLUGINY.

   Tyto programy se spousti pomoci prostredi na pluginy  po stisku klavesy F2
   nebo "rucne" primo v adresari PLUGINY.

   V tomto jejich prostredi  je take moznost  "Spustit jiny program",  pomoci
   ktere (stiskem S) muzete spustit program primo z Pokydu. Po ukonceni toho-
   to programu nasleduje navrat do Pokydu.
   Funkce "Vymazat soubory s hovorem" (klavesa V) smaze vsechny soubory v ad-
   resari KYDY.  Pokud je zapnuto nastaveni  zapisu hovoru do souboru,  tento
   soubor nebude smazan.
   Pozn.: Muzete vykonat take prikazem SMAZ SOUBORY (viz vyse).
   Po pouziti funkce "Kydani pocitacu" (klavesa K) se nabidne veta zacinajici
   hovor mezi dvema pocitaci.
   A stiskem pismena 'F'  pro funkci "Zformatovat harddisk"  se nabidne veta,
   ktera po potvrzeni zapricini  zformatovani harddisku  (samozrejme se jedna
   pouze o simulaci formatovani).
   Tyto moznosti  muzete vyvolat  samozrejme  take obycejnym napsanim  daneho
   prikazu, aniz byste museli spoustet prostredi pluginu.


*****  11.  Ostatni informace

   Vlastni program je velmi stabilni a rychly.  V dobe vydani programu nebyla
   znama zadna chyba, kdy by Pokyd "ztuhnul" ci jinak projevil chybu.
   Rychlost:  u pocitacu  s procesorem  Pentium 133MHz  (byl na nem  vytvoren
   a testovan)  stihne  roztridit text,  "vymenit"  nektera  slova,  porovnat
   s naucenymi slovy, zjistit obsah textu (projit cely slovnik) a vygenerovat
   odpoved za 0.02 sekundy (!).  Program by tedy mel bez problemu chodit i na
   velmi starych pocitacich.


   Pri spousteni programu Pokyd nemusite byt v adresari s Pokydem. Program si
   sam najde, odkud byl spusten a vsechny pridavne soubory "taha" primo z te-
   to cesty. Tam i zapisuje prubeh hovoru (viz nize).


   Pokud  mate zapnuty  hovor  s dvema pocitaci  a  druhy pocitac  se dostane
   na radu, pricemz rekne posledni vetu,  budete mluvit s nim.  V praxi je to
   to same, lisi se to jen barvou a "pocitem, ze mluvim s nekym jinym". Pokud
   byste chteli opet kydat s prvnim pocitacem, pustte ho na radu (napr. nech-
   te druhy pocitac chvili mluvit sam se sebou a prvni se zase prida).


   Normalne hovor zacina pocitac. Pokud chcete zacinat vy, spustite program a
   jako parametr zadate svoji prvni vetu.
   Priklad:

    POKYD.EXE Jak se mas?

   a pocitac rovnou odpovi.

   Z prikazoveho radku muzete delat i jine akce. Syntaxe je
    POKYD -akce
     nebo
    POKYD /akce

   Jednotlive akce jsou tyto:
    POKYD -readonly   - program nebude zapisovat na zadny disk, zapis pro nej
                        proste bude "tabu". Tim samozrejme nelze ulozit napr.
                        nastaveni nebo profil. Zapisovat muzou pouze pluginy.
    POKYD -test       - otestovani spravnosti souboru programem TestPkd
    POKYD -setric     - okamzite zapnuti setrice obrazovky
    POKYD -uloz       - ulozi se nastaveni a profil
    POKYD -napoveda   - ukaze se napoveda
    POKYD -pluginy    - spusti se prostredi pluginu
    POKYD -nastaveni  - spusti se prostredi nastaveni
    POKYD -pokyd      - spusti se prostredi vlastniho hovoru
    POKYD -zacatecnik - zapnou se hlasky pro zacatecniky

   Akce, ktere maji neco spustit, toto spusti ihned po nabehnuti programu.
   Tyto parametry nelze kombinovat mezi sebou (plati posledni parametr),  ale
   lze je kombinovat s napsanim pocatecni vety.
   Lze tedy napr. napsat:

    POKYD -test Ahoj, jak se mas?
     nebo klidne i
    POKYD Ahoj, jak -test se mas?

   V techto pripadech se napred otestuje spravnost souboru a hned potom Pokyd
   odpovi na vetu "Ahoj, jak se mas?".

   Cely hovor  se muze zapisovat  do dvou druhu souboru - do souboru KYDY.TXT
   nebo do souboru typu  11223444.txt,  pricemz 11 znamena den,  22 je mesic,
   3 oznacuje posledni cifru roku a 444 nejmensi mozne cislo, ktere se zjisti
   ihned po startu Pokydu.
   Tento soubor se zapise  do podadresare KYDY  na ceste  s umistenim Pokydu.
   Jestlize tento adresar neexistuje, bude vytvoren.
   Pokud bude  predchazejici hovor  chybne ukoncen  a zustane po nem  prazdny
   soubor, program to pozna a prepise ho.
   Pri preteceni cisel (neboli kdy Pokyd spustite 1000x v jednom dni a priby-
   vajici soubory nemazete) program na chybu upozorni  a hovor se nebude psat
   nikam.
   V pripade,  ze se hovor  zapisuje do souboru KYDY.TXT,  zapise se na konec
   tohoto souboru.

   Struktura souboru s hovorem:  napred je hlavicka (info o programu, kontakt
   na autora a podobne blbosti). Dale samotny hovor (C - clovek, P - pocitac)
   a na konci je hlaseni o zpusobu ukonceni, pocet vet a doba hovoru.
   Pokud v se pripojil i druhy pocitac (napr. kdyz dlouho nic nedelate), pise
   se za "P" cislo pocitace, tedy P1 nebo P2.

   Na zacatku, kdyz je verze jeste cerstva, pise "NOVA VERZE". Kdyz verze no-
   va uz neni, tento text se tam nenapise,  ale jinak program pracuje stejne!
   Misto "NOVA VERZE" se tam take muze napsat  "SPATNE DATUM !".  To rika, ze
   datum nastavene na pocitaci  je starsi nez datum vyroby programu,  a tudiz
   chybne. Program ale jako v predchozim pripade bude chodit uplne stejne.


   Horni radek se roluje do te doby,  nez je zcela nahore.  Pak se roluje jen
   okno pod nim.  To plati i v pripade, ze neni nastaven standardni 25radkovy
   mod, ale 50radkovy mod.


   Pri spusteni s presmerovanim do souboru (pripadne nikam - pr.: pokyd >nul)
   program bude chodit korektne,  tj. tak, jak by chodil bez tohoto presmero-
   vani. Ve vystupnim souboru nebude nic.


   Jinak Pokyd ma celkem 12 efektu  u vet psanych pocitacem,  2 u vet psanych
   clovekem a 8 efektu smazani obrazovky, ktere jsou vzdy nahodne volene.


   Pokud chcete okamzite urychlit psani na obrazovku  ("ty efekty jsou hrozne
   zdlouhave"),  stisknete kdykoliv  zaroven tlacitka  Ctrl a Alt.  Vystup se
   zrychli na zhruba 200 znaku za sekundu.


*****  12.  Statistiky slovniku a programu

   Statistika se vztahuje k 1. vydani slovniku v teto verzi (zjistite Alt-V).

   Prumerne:
     delka odpovedi:       44 znaku (vcetne mezer)
     delka 1 slova:         4 znaky
     slov ve vete:          8 slov
   Nejdelsi slovo:
     v odpovedich:         14 znaku
     v podminkach:         11 znaku
   Pocet slov:
     v odpovedich:      24019
     v podminkach:       1110
   Podminkovych elementu:
     celkem              1357
     na jednu podminku      6

   Pocet vet:
    Normalni:                     2038 odpovedi
    V zavislosti na rodu cloveka:  362 odpovedi
   --------------------------------------------
   Celkem:                        2400 odpovedi

   Pocet extra vet k zenskemu rodu pocitace:  396 odpovedi

   Velikosti:
    Zdrojovy kod:     631kB (17453 radku, 63 souboru)
    Textove soubory:  109kB (2340 radku, 4 soubory)


*****  13.  O programu

   Tento program je velmi stabilni  a nebyla nalezena chyba.  Pokud by se vam
   nekdy nekde  za jakychkoli podminek stalo,  ze pocitac neco  napsal spatne
   (paznaky),  pravopisnu chybu (zarucene by tam zadna nemela byt),  preklep,
   projevil jinou chybu nebo dokonce ztuhnul(!), prosim, kontaktujte me.
   Chyba bude pokud mozno ihned odstranena, treba ze je sebemensi a i kdyz by
   tento program byl uz hodne zastaraly.

   Predpoklada se, ze tato verze je posledni verzi programu Pokyd.  Nemusi to
   byt pravda;  muzou vyjit dalsi opravne  i nove vylepsujici verze. Soucasny
   system programu  vsak neumoznuje  prilisne zlepseni  a tak je vydavana  uz
   jen  jina verze  tohoto programu  -  IQ Pokyd.  Naleznete ho  (stejne jako
   nejnovejsi  vydani  tohoto  "stareho"  Pokydu)  na  nasich  www  strankach
   "http://iqpokyd.kyblsoft.cz",  kde si muzete stahnout  take spoustu jinych
   uzitecnych programu, dozvedet se mnoho informaci ci si zahrat hru.

   Adresa na autora (KYBLSoft):
                                          
                      -------------------------------------
                      |  Adresa:   Ales Janda             |**
                      |      Sidl. U Cukrovaru 1069       |**
                      |       Kralupy nad Vltavou         |**
                      |           PSC:  278 01            |**
                      |-----------------------------------|**
                      |  Telefon:       777 641 869       |**
                      |-----------------------------------|**
                      |  E-mail:                          |**
                      |    iqpokyd@kyblsoft.cz            |**
                      |  Internet (aktualizace):          |**
                      |    http://iqpokyd.kyblsoft.cz     |**
                      -------------------------------------**
                         ************************************

   Dekuji:
    - svemu bratrovi  Mirkovi za predlohu programu  pro vytvareni fontu  a za
      spoustu technickych vylepseni
    - svemu tatovi za znalosti programovani a ochotu k pripadne pomoci
    - Ondreji Pouzarovi a Davidovi Korinkovi za testovani programu
    - vsem prispevovatelum slovniku, tedy inteligence programu
    - Martinu Gramesovi a Robinu Martinezovi za zdravou kritiku
    - Michaele Chomatove  za to,  ze je nejlepsi holka na svete  a ze me bere
      takoveho, jaky jsem (tak tuhle vetu tu nechavam uz jen z nostalgie)
    - autorum vyborneho prekladace Borland C++
    - autorum komprese aPACK,  ktera je ta  nejlepsi vnitrni komprese,  jakou
      jsem kdy videl
    - autorum elektronickeho manualu Sysman, bez ktereho bych se asi uz tezko
      obesel
    - vsem pisatelum e-mailu, dopisu a prispevku i volajicim,  kteri mi pora-
      dili, co mam zlepsit a dali mi motivaci k dalsi tvorbe
    - vsem uzivatelum za sireni programu a tim i podporovani freewaru

   Hezkou zabavu vam preje
                                                   Ales Janda (* 1983), autor
