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
      // sprintf(hnam,"ADC %s ch:%02d",mod[j],i);
      // HBOOK1(500+N_ADC*j+i,hnam,2048,0.,4096.,0);
      // sprintf(hnam,"aligned ADC mod:%1d ch:%02d",j,i);
      //           HBOOK1(600+N_ADC*j+i,hnam,2048,0.,4096.,0);
      // HBOOK1(600+N_ADC*j+i,hnam,1024,0.,4096.,0);
      sprintf(hnam,"calib-ADC %s ch:%02d",mod[j],i);
      //      HBOOK1(700+N_ADC*j+i,hnam,2048,0.,32.,0);
      HBOOK1(800+N_ADC*j+i,hnam,2048,-10.,64.,0);
      // sprintf(hnam,"calib-ADC %s ch:%02d with TDC Gate",mod[j],i);
      //HBOOK1(900+N_ADC*j+i,hnam,2048,-10.,64.,0);
      /// sprintf(hnam,"calib-ADC %s ch:%02d case not adcNo1",mod[j],i);
      //HBOOK1(400+N_ADC*j+i,hnam,2048,-10.,64.,0);
    }
    sprintf(hnam, "ADC ch-monitor %s",mod[j]);
    HBOOK2(10+j,hnam,32,0.,32.,128,0.,4096.,0);
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
  
  HBOOK2(30,"TDC ch-monitor",128,0.,128.,256,-6000.,12000.,0);
  HBOOK2(40,"TDC ch-monitor with TDC Gate",128,0.,128.,256,0.,8000.,0);
  
  HBOOK1(31,"TDC ch-counter",128,0.,128.,0);
  HBOOK1(41,"TDC ch-counter with TDC Gate",128,0.,128.,0);
  sprintf(hnam,"TDC ch vs TDC count");
  HBOOK2(33,hnam,64,0.,64.,10,0.,10.,0);
  sprintf(hnam,"TDCC vs TDC ch TDC ADC gate");
  HBOOK2(38,hnam,256,0.,800.,64,0.,64.,0);

  for(i=0;i<N_TDC1;i++){
    // sprintf(hnam,"TDC %02d",i);
    //HBOOK1(1000+i,hnam,2048,-6000.,12000.,0);
    sprintf(hnam,"calib TDC %02d",i);
    HBOOK1(2000+i,hnam,2048,-600.,1200.,0);
    // sprintf(hnam,"TDC vs ADC",i);
    // HBOOK2(2000+i,hnam,2048,-6000.,12000.,1024,0.,64.,0);
    // sprintf(hnam,"TDC %02d with TDC Gate ",i);
    //HBOOK1(1100+i,hnam,2048,-6000.,12000.,0);
    //sprintf(hnam,"TDC vs ADC %02d with TDC Gate",i);
    //HBOOK2(2100+i,hnam,64,-6000.,6000.,64,0.,64.,0);
   
  }
 
  sprintf(hnam,"DIFF of TDCC in 3 alpha event 00");
  HBOOK1(3000,hnam,500,-100.,400.,0);
  sprintf(hnam,"DIFF of TDCC in 3 alpha event 01");
  HBOOK1(3001,hnam,500,-100.,400.,0);
  sprintf(hnam,"DIFF of TDCC in 3 alpha event with tdc Gate  00");
  HBOOK1(3010,hnam,500,-100.,400.,0);
  sprintf(hnam,"DIFF of TDCC in 3 alpha event with tdc Gate 01");
  HBOOK1(3011,hnam,500,-100.,400.,0);
  sprintf(hnam,"DIFF of TDCC in 3 alpha event with tdcGate-coincidence 00");
  HBOOK1(3020,hnam,500,-100.,400.,0);
  sprintf(hnam,"DIFF of TDCC in 3 alpha event with tdcGate-coincidence 01");
  HBOOK1(3021,hnam,500,-100.,400.,0);

  for(i=0;i<N_ADC_MOD;i++){
    for(j=0;j<3;j++){
      sprintf(hnam,"adccx-adccy vs tdccx-tdccy in 6alpha coin MOD%1d hit%d",i,j);
      HBOOK2(3030+3*i+j,hnam,64,-6.,10.,64,-20.,20.,0);
      sprintf(hnam,"adccx vs tdccx-tdccy in 6alpha coin MOD%1d hit%d",i,j);
      HBOOK2(3040+3*i+j,hnam,64,0.,10.,64,-20.,20.,0);
    }
  }

  // HBOOK1(32,"TDC ch-counter2",128,0.,128.,0);
  
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
  sprintf(hnam,"hit pattern %1d x %1d except with TDC Gate",3,3);
  HBOOK2(10030+3,hnam,44,-4.,40.,48.,-16.,32.,0);

  HBOOK1(20000,"strip 0 front",16,0.,16.,0);
  HBOOK1(20001,"strip 0 back",16,0.,16.,0);
  HBOOK1(20002,"strip 1 front",16,0.,16.,0);
  HBOOK1(20003,"strip 1 back",16,0.,16.,0);
  HBOOK1(20004,"right and left front",32,0.,32.,0);
  HBOOK1(20005,"right and left back",32,0.,32.,0);

  // HBOOK2(34,"No. of particle within Gate right vs left",10,0.,10.,10,0.,10.,0);
  //HBOOK2(35,"No. of particle within Gate STRICT1 right vs left",10,0.,10.,10,0.,10.,0);
  //HBOOK2(36,"No. of particle within Gate STRICT2 right vs left",10,0.,10.,10,0.,10.,0);
  //HBOOK2(37,"No. of particle within Gate STRICT3 right vs left",10,0.,10.,10,0.,10.,0);
  HBOOK1(50000,"invariant mass of 12C mod 0 ",128,0.,16.,0);
  HBOOK1(50001,"invariant mass of 12C mod 1 ",128,0.,16.,0);
  HBOOK2(50002,"invariant mass of 12C 0vs1",128,0.,16.,128,0.,16.,0);

  HBOOK1(50010,"invariant mass of 12C mod 0 with TDC-Gate ",128,0.,16.,0);
  HBOOK1(50011,"invariant mass of 12C mod 1 with TDC-Gate ",128,0.,16.,0);
  HBOOK2(50012,"invariant mass of 12C 0vs1 with TDC-Gate ",128,0.,16.,128,0.,16.,0);
  HBOOK1(50020,"invariant mass of 12C mod 0 with TDC-Gate 3alpha coin",128,0.,16.,0);
  HBOOK1(50021,"invariant mass of 12C mod 1 with TDC-Gate 3alpha coin",128,0.,16.,0);
  HBOOK1(50030,"invariant mass of 12C mod 0 with TDC-Gate 6alpha coin",128,0.,16.,0);
  HBOOK1(50031,"invariant mass of 12C mod 1 with TDC-Gate 6alpha coin",128,0.,16.,0);
  HBOOK2(50032,"invariant mass of 12C 0vs1 with TDC-Gate 6alpha coin ",128,0.,16.,128,0.,16.,0);
  for(i=0;i<N_ADC_MOD;i++){
  sprintf(hnam,"sum adcc of 3alpha in 6alphacoin MOD %1d x",i);
  HBOOK1(50100+2*i+0,hnam,128,0.,32.,0);
  sprintf(hnam,"sum adcc of 3alpha in 6alphacoin MOD %1d y",i);
  HBOOK1(50100+2*i+1,hnam,128,0.,32.,0);
    }
  HBOOK1(45,"all tdcc with all gate",256,-600.,600.,0);
  HBOOK2(46,"all adcc vs tdc with all gate",32,0.,10.,128,0.,8000.,0);
  // HBOOK1(47,"all tdcc with all gate only omote",256,-600.,600.,0);
  /*2018/06/08 homework */
  
  return(1);
}
