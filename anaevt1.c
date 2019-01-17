#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cfortran.h>
#include <hbook.h>
#include <kernlib.h>

#include "kinema.h"
#include "anap4.h"

#define MAX_TDC 1000
const long int TIME_H = 2000;
const long int TIME_L = -2000;

int anaevt(int evtlen,unsigned short *rawbuf,struct p4dat *dat){
  int i,ip,j,ii,jj;
  int segsize,ids,ips,ipsn,count;
  //  unsigned short qdc[N_QDC];
  long long int tdc[N_TDC];
  unsigned int adc[N_ADC_MOD][N_ADC];
  //  double qdcc[N_QDC];
  double tdcc[N_TDC];
  double adcc[N_ADC_MOD][N_ADC];
  const double tpar[N_TDC][2]={{0.0, 0.01}, {0.0, 0.01},{0.0, 0.01},{0.0, 0.01}};

  char tmpbuf[200];
  /* variables for v1190 */
  int ilt; /* Leading or Trailing */
  int ichan;
  int ihit=0;
  unsigned int idata;
  unsigned int raw_v1190[2][MAX_TDC];
  unsigned int tzero;
  long int t_measure;
  /************* Clear Event Buffer *****************/
  /*
  for(i=0;i<N_QDC;i++) {
    qdc[i]=0;
    qdcc[i]=0.;
  }
  */
  for(i=0;i<N_TDC;i++){
    tdc[i]=0;
    tdcc[i]=0.;
  }
  for(i=0;i<N_ADC;i++){
    for(j=0;j<N_ADC_MOD;j++){
      adc[j][i]=0;
      adcc[j][i]=0.;
    }
  }
  count=0;
  /************* Clear Event Buffer *****************/

  /************* Decode Event Data Here *****************/
  ip=0;
  while(ip<evtlen){  
    int tmpdat,tmpch,tmpnwd;
    int ily=0; /* Data Layer */
               /* 0: Grobal Header, Global Trailer */
	       /* 1: TDC Header, TDC Trailer */
               /* 2: TDC Data */
    int tdcid;
    int scaid;
    
    /** Segment Header ************/
    segsize=rawbuf[ip++];
    ipsn=ip+segsize-1;
    ids=rawbuf[ip++]; // ids -> segment id
    
#if _DEBUG
    ips=0;
    printf(" New Seg %d\n",ids);
#endif    
    while(ip<ipsn){ /*** Segment loop ***/
      switch(ids){
      case 1:  /*** MADC32 No.1 ****/     
	tmpdat=rawbuf[ip++];
	tmpdat+=(rawbuf[ip++]*0x10000);
	if((tmpdat & 0xc0000000)==0x40000000){ /* Header */
	  unsigned int tmpmod,tmpnwd;
	  tmpmod=((tmpdat>>16)&0x00ff);
	  tmpnwd=(tmpdat&0x3ff);
#if _DEBUG
	  printf("  MADC32:ID %02x:Count %2d\n",tmpmod,tmpnwd);
#endif
	  for(i=0;i<tmpnwd;i++){
	    tmpdat=rawbuf[ip++];
	    tmpdat+=(rawbuf[ip++]*0x10000);
	    switch(tmpdat & 0xc0000000){
	    case 0x0000000: /* Data */
	      if((tmpdat & 0xffe00000)==0x04000000) { /* Data event */
		tmpch=(tmpdat & 0x01f0000) >> 16;
		//tmpch += (tmpmod&0x1)*32;
		//adc[tmpch]=(tmpdat & 0xffff);
		adc[(tmpmod&0x1)][tmpch]=(tmpdat & 0xffff);
#if _DEBUG
		printf("  ADC:0x%08x:  Ch:%2d  Adc:%4d\n", 
		       tmpdat,tmpch,adc[tmpch]);
		/*
		  sprintf(tmpbuf,"ADC:   %2d: %08x  ch:%02d  adc:%4d\n",
		  i, tmpdat,tmpch, qdc[i]);
		  dumpmsg(tmpbuf);*/
#endif
	      }
	      else {
		printf("  MADC32: Non event data (0x%08x).\n",tmpdat);
	      }
	      break;
	    case 0xc0000000: /* End of Event data */
#if _DEBUG
	      printf("  ADC:0x%08x:  Timestamp:0x%08x\n",
		     tmpdat,(tmpdat & 0x3fffffff));
#endif
	      break;
	    default:
	      printf("  MADC32:Unknown data format (%08x)\n",tmpdat);
	    }
	  }
	}
	else{
	  printf("  MADC32:Wrong header word (%08x)\n",tmpdat);
	}
	break;

      case 2: /******** v1190 *********/
	tmpdat=rawbuf[ip++];
	tmpdat+=(rawbuf[ip++]*0x10000);
	switch((tmpdat>>27)&0x1f){
	case 0x08:  /* Global Header */
	  ily=1;
	  break;
	case 0x10: /* Global Trailer */
	  if(ily!=1){
	    printf("V1190 Global Trailer error (Ly:%d).\n",ily);
	  }else{
	    ily=0;
	  }
	  break;
	case 0x11: /* Extended Trigger Time Tag */
	  break;
	case 0x01: /* TDC Header */
	  if(ily!=1){
	    printf("V1190 TDC Header error (Ly:%d).\n",ily);
	  }else{
	    ily=2;
	  }
	  break;
	case 0x00: /* TDC Measurement */
	  if(ily!=2){
	    printf("V1190 TDC Data error (Ly:%d).\n",ily);
	    return(-1);
	  }
	  ilt=(tmpdat>>26)&0x1;
	  if(ilt==0){
	    ichan=(tmpdat>>19)&0x7f;
	    raw_v1190[0][ihit]=ichan;
	    idata=tmpdat&0x7ffff;
	    raw_v1190[1][ihit++]=idata;
	    if(ichan==V1190_REF) tzero=idata; //reference timing e.g. RF etc...
	    if(ihit==MAX_TDC){
	      fprintf(stderr, "Number of hit of V1190 reaches the maximum number.\n");
	      return -1;
	    }
	  }
	  break;
	case 0x04: /* TDC Error */
	  printf(" V1190 TDC ERR\n");
	  break;
	case 0x03: /* TDC Trailer */
	  if(ily!=2){
	    printf("V1190 TDC Trailer error (Ly:%d).\n",ily);
	  }else{
	    ily=1;
	  }
	  break;
	default:
	  printf(" V1190:%08x Unknown format.\n",tmpdat);
	}
      break;

      /********* V560 **********/
      case 3:
//	if(scaid<SCA_CH){
//	  tmpdat = rawbuf[ip++];
//	  tmpdat += (rawbuf[ip++]*0x10000);
//	  dat->sca[scaid++]=tmpdat;
//	}
	for(scaid=0;scaid<SCA_CH;scaid++){
	  tmpdat = rawbuf[ip++];
	  tmpdat += (rawbuf[ip++]*0x10000);
	  dat->sca[scaid] = tmpdat;
	}
	break;

      default:
	ip++;ip++; /* Skip segment ids != 1 */
      }
    }
  }
/**** Decode Event Data Above***************/


  /**** Data Analysis Here ***************/

  /* v1190 data rearrangement & timing cut */
  for(i=0;i<ihit;i++){
    //    t_measure=raw_v1190[1][i]-tzero; 
    t_measure=raw_v1190[1][i]; 
//    if(t_measure<TIME_H && t_measure>TIME_L){
//      tdc[v1190_raw[0][i]]=v1190_raw[1][ihit];
//    }
    tdc[raw_v1190[0][i]]=t_measure;
  }
	

  /* time calibration */
  for(i=0;i<N_TDC;i++) {
    tdcc[i]=(tdc[i]-tpar[i][0])*tpar[i][1];
  }
  

  /* ADC calibration */

 
  /**** Data Analysis Above ***************/
  
  /*********** Booking here **********/

#if 1  
  /*
  for(i=0;i<N_QDC;i++){
    HF1(10+i,qdc[i],1.0);
  }
  */

  for(i=0;i<N_TDC;i++){
    if(tdc[i]>0){
      HF1(20+i,tdc[i]-tzero,1.0);
      HF1(3000,i,1.0);
      HF2(4000,i,tdc[i]-tzero,1.0);
    }
  }
  for(j=0;j<N_ADC_MOD;j++){
    for(i=0;i<N_ADC;i++){
      if(tdc[N_ADC*j+i]>0){
	HF1(300+N_ADC*j+i,adc[j][i],1.0);
	HF2(2000+j,i,adc[j][i],1.0);
	HF1(1000+j,i,1.0);
      }
    }
  }
#endif
  

  /*********** Booking Above **********/
  return(ip);
}
