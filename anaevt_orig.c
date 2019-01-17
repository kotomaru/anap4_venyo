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
  int i,ip,j;
  int segsize,ids,ips,ipsn,count;
  unsigned short qdc[N_QDC];
  unsigned short tdc[N_TDC];
  unsigned short adc[N_ADC];
  double qdcc[N_QDC];
  double tdcc[N_TDC];
  double adcc[N_ADC];
  const double tpar[N_TDC][2]={{0.0, 0.01}, {0.0, 0.01},{0.0, 0.01},{0.0, 0.01}};

  char tmpbuf[200];
  
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
   
      /*   for(i=0;i<N_ADC;i++) {
	tmpdat=rawbuf[ip++];
	tmpch=(tmpdat>>12)&0xf;
	adc[i]=(tmpdat&0xfff);
#if _DEBUG
	sprintf(tmpbuf,"ADC:   %2d: %04x  ch:%02d  adc:%4d\n",i,tmpdat,tmpch,
		adc[i]);
	dumpmsg(tmpbuf);
#endif
 
      }
      */
    }
  }
/**** Decode Event Data Above***************/


  /**** Data Analysis Here ***************/

  /* Energy calibration */
  for(i=0;i<N_QDC;i++){
    qdcc[i]=1.*qdc[i];
  }

  /* time calibration */
    for(i=0;i<N_TDC;i++) {
      tdcc[i]=(tdc[i]-tpar[i][0])*tpar[i][1];
      }
  

  /*
  for(i=0;i<N_ADC;i++){
    adcc[i]=(double)adc[i];
  }
  */
  /**** Data Analysis Above ***************/
  
  /*********** Booking here **********/
  
  for(i=0;i<N_QDC;i++){
    HF1(10+i,qdc[i],1.0);
  }

  
  for(i=0;i<N_TDC;i++){
    HF1(20+i,tdc[i],1.0);
  }
  
  /*
  for(i=0;i<N_ADC;i++){
    HF1(30+i,adc[i],1.0);
  }
  */

  HF2(50,qdc[0],qdc[1],1.0);
  

  double lnqdc12 = log(((double)qdc[0])/((double)qdc[1]));
  double qdc12 = ((double)qdc[0])*((double)qdc[1]);
  HF2(51,lnqdc12,qdc[0],1.0);
  HF2(52,lnqdc12,qdc[1],1.0);
  HF2(53,lnqdc12,qdc12,1.0);    

  /*********** Booking Above **********/
  return(ip);
}
