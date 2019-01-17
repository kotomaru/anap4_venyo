#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cfortran.h>
#include <hbook.h>
#include <kernlib.h>

#include "kinema.h"
#include "anap4.h"



int anaevt(int evtlen,unsigned short *rawbuf,struct p4dat *dat){
  int i,ip;
  int segsize,ids,ips,ipsn,count;
  unsigned short qdc[N_QDC];
  unsigned short qdcoff[N_QDC];
  unsigned short tdc[N_TDC];
  unsigned short adc[N_ADC];
  double qdcc[N_QDC];
  double tdcc[N_TDC];
  double adcc[N_ADC];
  //const double qpar[N_QDC][2]={{0.0, 0.01}, {0.0, 0.01}};
  const double tpar[N_TDC][2]={{0.0, 0.01}, {0.0, 0.01}};
  //unsigned short qdcp0[N_QDC]={0,30};
  char tmpbuf[200];
  double qx,tx;
  double times,divide;
  int diff;

  /************* Clear Event Buffer *****************/
  for(i=0;i<N_QDC;i++) {
    qdc[i]=0;
    qdcc[i]=0.;
  }
  for(i=0;i<N_TDC;i++){
    tdc[i]=0;
    tdcc[i]=0.;
  }
  for(i=0;i<N_ADC;i++){
    adc[i]=0;
    adcc[i]=0.;
  }
  count=0;
  
  /************* Clear Event Buffer *****************/

  /************* Decode Event Data Here *****************/
  ip=0;
  while(ip<evtlen){  
    int tmpdat,tmpch;

    /** Segment Header ************/
    segsize=rawbuf[ip++];
    ipsn=ip+segsize-1;
    ids=rawbuf[ip++];

#if _DEBUG
    ips=0;
    printf(" New Seg %d\n",ids);
#endif

    while(ip<ipsn){ /*** Segment loop ***/
      /*** 7166 ****/
#if _DEBUG
      printf("  7166\n");
#endif
      for(i=0;i<N_QDC;i++) {
	tmpdat=rawbuf[ip++];
	tmpch=(tmpdat>>12)&0xf;
	qdc[i]=(tmpdat&0xfff);
#if _DEBUG
	sprintf(tmpbuf,"QDC:   %2d: %04x  ch:%02d  adc:%4d\n",i,tmpdat,tmpch,
		qdc[i]);
	dumpmsg(tmpbuf);
#endif
      }
      for(i=0;i<N_TDC;i++) {
	tmpdat=rawbuf[ip++];
	tmpch=(tmpdat>>12)&0xf;
	tdc[i]=(tmpdat&0xfff);
#if _DEBUG
	sprintf(tmpbuf,"TDC:   %2d: %04x  ch:%02d  adc:%4d\n",i,tmpdat,tmpch,
		tdc[i]);
	dumpmsg(tmpbuf);
#endif
      }
      for(i=0;i<N_ADC;i++) {
	tmpdat=rawbuf[ip++];
	tmpch=(tmpdat>>12)&0xf;
	adc[i]=(tmpdat&0xfff);
#if _DEBUG
	sprintf(tmpbuf,"ADC:   %2d: %04x  ch:%02d  adc:%4d\n",i,tmpdat,tmpch,
		adc[i]);
	dumpmsg(tmpbuf);
#endif
      }
    }
  }
  /**** Decode Event Data Above***************/


  /**** Data Analysis Here ***************/

/* energy calibration */
/*  
    for(i=0;i<N_QDC;i++) {
    qdcc[i]=(qdc[i]-qpar[i][0])*qpar[i][1];
    qdcoff[i]=qdc[i]-qdcp0[i];
    }
*/
 
/* time calibration */
/*  for(i=0;i<N_TDC;i++) {
    tdcc[i]=(tdc[i]-tpar[i][0])*tpar[i][1];
  }
*/
/*loction calibration*/

  for(i=0;i<N_QDC;i++){
    qdcc[i]=1.*qdc[i];
  }
    
  for(i=0;i<N_TDC;i++){
    tdcc[i]=1.*tdc[i];
  }

  
    
/*normalization*/

  qx=100.*log(qdcc[1]/qdcc[0])/((log(qdcc[0]*qdcc[1]))-8.);

  tx=100.*(tdcc[0]-tdcc[1])/(4350.-3950.);


  for(i=0;i<N_TDC;i++){
    if(tdc[i]>50&&tdc[i]<3000){
      count++;}}
  /**** Data Analysis Above ***************/
  
  /*********** Booking here **********/
  
  for(i=0;i<N_QDC;i++){
    HF1(10+i,qdc[i],1.0);
    //HF1(10+N_QDC+i,qdcoff[i],1.0);
  }
  for(i=0;i<N_TDC;i++){
        
    HF1(20+i,tdc[i],1.0);
    
  }
  for(i=0;i<N_ADC;i++){
    HF1(30+i,adc[i],1.0);
    /*if(count==2){
      HF1(30+N_ADC+i,adc[i],1.0);
      }*/
  }
  for(i=0;i<N_ADC;i++){
    if(500<adc[i]&&adc[i]<1500){
      HF1(40,i,1.0);
    }
  }
  //  if(adc[0]>1600&&adc[0]<1800){
  /*if(adc[0]>700&&adc[0]<850){
    HF1(50,adc[1],1.0);
    }*/
  //  if(adc[0]>1800&&adc[0]<2000){
  /* if(adc[0]>850&&adc[0]<950){
    HF1(51,adc[1],1.0);
    if(tdc[0]<4095&&tdc[1]<4095){
      HF2(60,adc[1],tdc[1]-tdc[0],1.0);
    }
    }*/
  //HF2(40,adc[0],adc[1],1.0);
  //HF1(41,count,1.0);
  //HF2(61,tdc[0],tdc[1],1.0);
  /*
  times=qdc[0]*qdc[1]/30.0;
  divide=qdc[0]*300.0/qdc[1];
  diff=tdc[0]-tdc[1]+4096;
  HF1(25,diff,1.0);
  HF1(30,times,1.0);
  if(qdc[1]==4095){
    return(ip);
  } else{
    HF1(30,times,1.0);
    HF1(31,divide,1.0);
    HF2(32,divide,qdc[0],1.0);
    HF2(33,divide,qdc[1],1.0);
  }
  //  HF1(35,tdc[0]-tdc[1],1.0);
  //  HF1(36,tdcc[0]-tdcc[1],1.0);
  
  HF2(35,qx,tx,1.0);
  HF1(36,qx,1.0);
  HF1(37,tx,1.0);
  //  HF2(40,qdc[0],qdc[1],1.0);
  HF2(40,tdc[2]-0.5*fabs(diff),qdc[2],1.0);
  */
  /*********** Booking Above **********/
  return(ip);
}
