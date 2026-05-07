#include <stdio.h>

unsigned char pozice,znak;
FILE *f1,*f2;

void main(void) {
f1=fopen("TISK.DAT","rb");
f2=fopen("SILNICE.FN","w");

fprintf(f2,"BYTE font[112]={");
for (pozice=0; pozice < 112; pozice++) {
  znak=getc(f1);
  fprintf(f2,"%d,",znak);
 }
fseek(f2,-1,SEEK_CUR);
fprintf(f2,"};");
fclose(f1); fclose(f2);
}
