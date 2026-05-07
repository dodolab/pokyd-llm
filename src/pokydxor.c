#include <stdio.h>

unsigned char xor[1000],znak;
unsigned long pozice=0,delka;

FILE *f,*fx;

void main() {
f=fopen("pokydx.txt","rb");
fx=fopen("pokydx.pkd","wb");
fseek(f,0,SEEK_END); delka=ftell(f); fseek(f,0,SEEK_SET);
while (pozice < delka) { znak=getc(f); xor[pozice++]=znak; }
delka=strlen(xor); pozice=0;
while (pozice < delka) xor[pozice++]^=200;
fclose(f); f=fopen("pokydx.h","w");
fprintf(f,"{ ",delka+1);
for (pozice=0; pozice < delka; pozice++) fprintf(fx,"%c",xor[pozice]);
putc(0,fx);
fprintf(f,
"unsigned long pozice=0;\n"
"\nVRATDATA(8); while (pozice < %lu) dlouhe[pozice++]^=200;\n"
"dlouhe[pozice]=0; pozodp=100; goto OD; }\n",delka);
fclose(f); fclose(fx);
}