#include <stdio.h>
#include <cfortran.h>
#include <kernlib.h>
#include <hbook.h>
#include "anap4.h"

int histdef(){
  int i,j;
  char hnam[100];
  char *mod[2]={"right","left"};

  /*
  for(i=0;i<N_QDC;i++){
    sprintf(hnam,"QDC %02d",i);
    HBOOK1(10+i,hnam,4096,0.,4096.,0);
  }
  */

  for(j=0;j<N_ADC_MOD;j++){
    for(i=0;i<N_ADC;i++){
      sprintf(hnam,"ADC %s ch:%02d",mod[j],i);
      HBOOK1(500+N_ADC*j+i,hnam,2048,0.,4096.,0);
           sprintf(hnam,"aligned ADC mod:%1d ch:%02d",j,i);
	   //           HBOOK1(600+N_ADC*j+i,hnam,2048,0.,4096.,0);
           HBOOK1(600+N_ADC*j+i,hnam,1024,0.,4096.,0);
      sprintf(hnam,"calib-ADC %s ch:%02d",mod[j],i);
      //      HBOOK1(700+N_ADC*j+i,hnam,2048,0.,32.,0);
           sprintf(hnam,"aligned calib-ADC mod:%1d ch:%02d",j,i);
           HBOOK1(800+N_ADC*j+i,hnam,2048,-10.,64.,0);
    }
    sprintf(hnam, "ADC ch-monitor %s",mod[j]);
    HBOOK2(10+j,hnam,32,0.,32.,128,0.,4096.,0);
    sprintf(hnam,"ADC ch-counter %s",mod[j]);
    //    HBOOK2(20+j,hnam,32,0.,32.,128,0.,4096.,0);
    sprintf(hnam,"ADC ch-counter %s",mod[j]);
    HBOOK1(10+N_ADC_MOD+j,hnam,32,0.,32.,0);
    sprintf(hnam, "calib-ADC ch-monitor mod:%1d",j);
    HBOOK2(20+j,hnam,32,0.,32.,128,0.,64.,0);
    sprintf(hnam, "calib-ADC ch-conter mod:%1d",j);
    HBOOK1(20+j+N_ADC_MOD,hnam,32,0.,32.,0);
    sprintf(hnam,"hit pattern %s",mod[j]);
    HBOOK2(50+j,hnam,20,0.,20.,20,0.,20.,0);
    sprintf(hnam,"all strip %s",mod[j]);
    HBOOK1(170+j,hnam,2048,0.,64.,0);
       sprintf(hnam,"all aligned strip mod:%1d",j);
       HBOOK1(170+j+N_ADC_MOD,hnam,2048,0.,4096.,0);
  }
  for(i=0;i<N_TDC;i++){
    sprintf(hnam,"TDC %02d",i);
    HBOOK1(1000+i,hnam,2048,-6000.,12000.,0);
  }
  HBOOK2(30,"TDC ch-monitor",128,0.,128.,256,-6000.,12000.,0);
  HBOOK1(31,"TDC ch-counter",128,0.,128.,0);
  HBOOK1(32,"TDC ch-counter2",128,0.,128.,0);
//  for(i=0;i<3;i++){
//    for(j=0;j<2;j++){
//      sprintf(hnam,"3a hit patter mod:%1d pn:%1d",j,i);
//      HBOOK2(33000+i+100*j,hnam,16,0.,16.,16,0.,16.,0);
//    }
//  }

  for(i=1;i<4;i++){
    sprintf(hnam,"hit pattern %1d x %1d",i,i);
    HBOOK2(10000+i,hnam,44,-4.,40.,48.,-16.,32.,0);
  }

  HBOOK1(20000,"strip 0 front",16,0.,16.,0);
  HBOOK1(20001,"strip 0 back",16,0.,16.,0);
  HBOOK1(20002,"strip 1 front",16,0.,16.,0);
  HBOOK1(20003,"strip 1 back",16,0.,16.,0);
  HBOOK1(20004,"right and left front",32,0.,32.,0);
  HBOOK1(20005,"right and left back",32,0.,32.,0);

  HBOOK1(50000,"invariant mass of 12C mod 0 ",128,0.,32.,0);
  HBOOK1(50001,"invariant mass of 12C mod 1 ",128,0.,32.,0);
  HBOOK2(50002,"invariant mass of 12C 0vs1",128,0.,16.,128,0.,16.,0);


  /*2018/06/08 homework */
  
  return(1);
}
