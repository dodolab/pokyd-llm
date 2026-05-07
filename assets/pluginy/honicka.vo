unsigned char CAS18() {

unsigned char cas18;

  _AH=0;
  geninterrupt (0x1A);
  cas18=_CX;
  cas18<<= 16;
  cas18+=_DX;
  if (cas18 != cas) { cas=cas18; return(1); }
  else { cas=cas18; return(0); }
 }
