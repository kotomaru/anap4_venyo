#include <stdio.h>
#include <cfortran.h>
#include <kernlib.h>
#include <hbook.h>
#include "anap4.h"

int histdef(){
  int i;
  char hnam[100];

  for(i=0;i<N_QDC;i++){
    sprintf(hnam,"QDC %02d",i);
    HBOOK1(10+i,hnam,4096,0.,4096.,0);
  }

  for(i=0;i<N_TDC;i++){
    sprintf(hnam,"TDC %02d",i);
    HBOOK1(20+i,hnam,4096,0.,4096.,0);
  }
  /*
  for(i=0;i<N_ADC;i++){
    sprintf(hnam,"ADC %02d",i);
    HBOOK1(30+i,hnam,4096,0.,4096.,0);
  }
  */
  
  HBOOK2(50,"QDC1 vs QDC2",128,0.,2048.,128,0.,2048.,0);
  HBOOK2(51,"ln(QDC1/QDC2) VS QDC1",128,-10.,10.,128,0.,2048.,0);
  HBOOK2(52,"ln(QDC1/QDC2) VS QDC2",128,-10.,10.,128,0.,2048.,0);
  HBOOK2(53,"ln(QDC1/QDC2) VS QDC1QDC2",128,-10.,10.,128,0.,4048000.,0);

  return(1);
}
