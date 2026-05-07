#include <stdio.h>

unsigned char pozice,soucet,xor;

void main(int argc, char *argv[]) {
soucet=0; xor=0;
for (pozice=0; pozice < strlen(argv[1]); pozice++) {
  soucet+=argv[1][pozice];
  xor^=argv[1][pozice]; xor+=argv[1][pozice];
 }
printf("Soucet: %d\nXor: %d\n",soucet,xor);
}
