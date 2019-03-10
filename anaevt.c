#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cfortran.h>
#include <hbook.h>
#include <kernlib.h>

#include "kinema.h"
#include "lorlib.h"
#include "anap4.h"

#define MAX_TDC 1000
//#define N_DUP 32
//#define N_TDC1 64
const long int TIME_H = 12000;//max:12000
const long int TIME_L = -600;//min:-600

int anaevt(int evtlen,unsigned short *rawbuf,struct p4dat *dat){
  int i,ip,j,ii,jj,k,num,n;
  double tmp;
  int tmp_tdc;
  double tmp_tdcc;
  int itmp;
  int segsize,ids,ips,ipsn,count;
  //  unsigned short qdc[N_QDC];
  int tdc[N_TDC][N_DUP]={{0}};
  int tdc_al[N_TDC][N_DUP]={{0}};
  unsigned int adc[N_ADC_MOD][N_ADC]={{0}};
  unsigned int adc_al[N_ADC_MOD][N_ADC]={{0}};
  //  double qdcc[N_QDC];
  double tdcc[N_TDC][N_DUP]={{0}};
  double tdcc_al[N_TDC][N_DUP]={{0}};
  double adcc[N_ADC_MOD][N_ADC]={{0}};
  double adcc_al[N_ADC_MOD][N_ADC]={{0}};
  double adc2[N_ADC_MOD][2][16]={{{0}}};//divide adc into RL xy

  int tdc12C[N_TDC][N_DUP]={{0}};
  int tdc12C_al[N_TDC][N_DUP]={{0}};
  double tdcc12C[N_TDC][N_DUP]={{0}};
  double tdcc12C_al[N_TDC][N_DUP]={{0}};
  double adcc12C[N_ADC_MOD][N_ADC]={{0}};//adc gate is large for 12C KE
  double adcc12C_al[N_ADC_MOD][N_ADC]={{0}};
  double adc12C2[N_ADC_MOD][2][16]={{{0}}};//adc2 for 12C
  //  double adccNo1[N_ADC_MOD]={-1000.,-1000.};
  //  const double tpar[N_TDC][2]={{0.0, 0.1}};
  double tpar[N_TDC][2];//for tdc calibration
  int tdc_cnt[N_TDC]={};
  int tdc_cnt_al[N_TDC]={};
  int tdc12C_cnt[N_TDC]={};
  int tdc12C_cnt_al[N_TDC]={};
  //  short tdc_nn[2][2]={};//[right &left][x & y]
  int tdc_nn[2][2]={};//[right &left][x & y]
  int tdc12C_nn[2][2]={};//[right &left][x & y]
  int hitch_tdc_odr[N_ADC_MOD][16][2]={{{0}}}; //3 hit tdc in order [2]=omote ura
  
  short tmpbuf[200];
  /* variables for v1190 */
  int ilt; /* Leading or Trailing */
  int ichan;
  int ihit=0;
  unsigned int idata;
  unsigned int raw_v1190[2][MAX_TDC];
  int tzero;
  long int t_measure;
  /*variables and array for ADC calibration*/
  int vch,vch_am;
  //f(x)=dsl*x+dseg x:ADC
  double vsl,vseg; // buffer vsl:slope from ADC to pulse, vseg:segment from ADC to pulse
  double vped,vAm,vpedsig,vAmsig;//buffer for Am dat
  double buff;//buffer for tdc calib file
  double EAm = 5.48;
  int ir,il;
  int ichc[N_ADC_MOD][N_DUP];//ch array for calib [right or left][0~31 (ch)]
  double dsl[N_ADC_MOD][N_DUP];//slope array for calib [right or left][0~31 (ch)]
  double dseg[N_ADC_MOD][N_DUP];//segment array for calib [right or left][0~31 (ch)]
  double dped[N_ADC_MOD][N_DUP];//241Ampedestal ADC array for calib [right or left][0~31 (ch)]
  double dAm[N_ADC_MOD][N_DUP];//241Am ADC array for calib [right or left][0~31 (ch)]
  
  int sicnt[N_ADC_MOD][2]={{}};//[][xy]
  int sicnt12C[N_ADC_MOD][2]={{}};//[][xy]
  /************* Clear Event Buffer *****************/
  /*
  for(i=0;i<N_QDC;i++) {
    qdc[i]=0;
    qdcc[i]=0.;
  }
  */
//  for(i=0;i<N_TDC;i++){
//    tdc[i]=0;
//    tdcc[i]=0.;
//  }
//  for(i=0;i<N_ADC;i++){
//    for(j=0;j<N_ADC_MOD;j++){
//      adc[j][i]=0;
//      adcc[j][i]=0.;
//    }
//  }
  count=0;
  /************* Clear Event Buffer *****************/

  /************* Decode Event Data Here *****************/
  ip=0;
  while(ip<evtlen){  
    int tmpdat,tmpch,tmpnwd;
    int ily=0; /* Data Layer */
               /* 0: Grobal Header, Global Trailer */
	       /* 1: TDC Header, TDC Trailer x*/
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
//	for(scaid=0;scaid<SCA_CH;scaid++){
//	  tmpdat = rawbuf[ip++];
//	  tmpdat += (rawbuf[ip++]*0x10000);
//	  dat->sca[scaid] = tmpdat;
//	}
//	break;

      default:
	ip++;ip++; /* Skip segment ids != 1 */
      }
    }
  }
/**** Decode Event Data Above***************/


  /**** Data Analysis Here ***************/
 
  /* ADC calibration */

  FILE *fright,*fleft;
  //calibration after beam time
  fright = fopen("../calib/Am_result00.txt","r");
  fleft = fopen("../calib/Am_result01.txt","r");
  //calibration before beam time
  //fright = fopen("../calib/ADC_calib_mod00_ver1.dat","r");
  //fleft = fopen("../calib/ADC_calib_mod01_ver1.dat","r");
  if(fright==NULL){
    puts("calib0 file cannot open");
    return -1;
  }
  if(fleft==NULL){
    puts("calib1 file cannot open");
    return -1;
  }
  while(fscanf(fright,"%d %lf %lf",&vch,&vsl,&vseg)!=EOF){
    //    adcc[0][vch-101] = adc[0][vch-101]*dsl+dseg;
    adcc[0][vch] = adc[0][vch]*vsl+vseg;
  }
  while(fscanf(fleft,"%d %lf %lf",&vch,&vsl,&vseg)!=EOF){
    adcc[1][vch] = adc[1][vch]*vsl+vseg;
  }
  fclose(fright);
  fclose(fleft);
 
  /*****adcc gate here****/
 const float adccmin=1.;//ADC Gate to eliminate crosstalk
 const float adccmax=12.;
 const float adcc12Cmin=1.;
 const float adcc12Cmax=24.;
 const double diff32result=10.; 
  
  //make adcc for 12C graund state 
  for(i=0;i<N_ADC_MOD;i++){
    for(j=0;j<N_ADC;j++){
      adcc12C[i][j]=adcc[i][j];
      if(adcc[i][j]<adcc12Cmin || adcc[i][j]>adcc12Cmax){
	adcc12C[i][j]=0.;
      }
    }
  }
  //3a adc gate here
  for(i=0;i<N_ADC_MOD;i++){
    for(j=0;j<N_ADC;j++){
      if(adcc[i][j]<adccmin || adcc[i][j]>adccmax){
	adcc[i][j]=0.;
      }
    }
  }

 /* v1190 data rearrangement & timing cut */
  /*if adc isnt within gate, dont analyze*/

  for(i=0;i<N_TDC;i++){
    for(j=0;j<N_DUP;j++){
      tdc12C[i][j]=tdc[i][j];
    }
    tdc_cnt[i]=0;
    tdc12C_cnt[i]=0;
  }
  for(i=0;i<ihit;i++){
    //    t_measure=raw_v1190[1][i]-tzero; 
    t_measure=raw_v1190[1][i]; 
    if(t_measure<(TIME_H+tzero) && t_measure>(TIME_L+tzero)){
      j=raw_v1190[0][i];
      if(tdc_cnt[j]<N_DUP){
	if(j<=N_TDC1){
	  if(adcc[(int)j/32][(int)j%32]!=0.){
	    tdc[j][tdc_cnt[j]++]=t_measure;
	  }
	  else{
	    tdc[j][tdc_cnt[j]]=0;
	  }
      	}
      	else{
	  tdc[j][tdc_cnt[j]++]=t_measure;
      	}
      }
      if(tdc12C_cnt[j]<N_DUP){
	if(j<=N_TDC1){
	  if(adcc12C[(int)j/32][(int)j%32]!=0.){
	    tdc12C[j][tdc12C_cnt[j]++]=t_measure;
	  }
	  else{
	    tdc12C[j][tdc12C_cnt[j]]=0;
	  }
      	}
      	else{
	  tdc12C[j][tdc12C_cnt[j]++]=t_measure;
      	}
      }

    }
  }
	

  /* time calibration */
  /* tdcc is -ref , tdc is not -ref*/
  for(i=0;i<N_TDC;i++){
    tpar[i][0]=0.;
    tpar[i][1]=0.1; //from channel to ns
  }
  FILE *ftdc;
  ftdc = fopen("../calib/tdc_am.txt","r");
  if(ftdc==NULL){
    puts("tdc calib file cannot open");
    return -1;
  }
  
  i=0;
  while(fscanf(ftdc,"%lf %lf",&tpar[i][0],&buff)!=EOF){
    if(i>=N_TDC1){break;}
    for(j=0;j<N_DUP;j++){
      if(tdc[i][j]>0){
	tdcc[i][j] = (tdc[i][j]-tpar[i][0]+3000-tzero)*tpar[i][1];
      }
      if(tdc12C[i][j]>0){
	tdcc12C[i][j] = (tdc12C[i][j]-tpar[i][0]+3000-tzero)*tpar[i][1];
      }
      //     else{tdcc[i][j]=0.;tdcc12C[i][j]=0.;}
    }
    // printf("%f %f\n",tpar[i][0],buff);
    i++;
  }
  fclose(ftdc);
  
  
  /* for(i=0;i<N_TDC;i++) { */
  /*   for(j=0;j<N_DUP;j++){ */
  /*     //      tdcc[i][j]=(tdc[i][j]-tpar[i][0])*tpar[i][1]; */
  /*     tdcc[i][j]=(tdc[i][j]-tpar[0][0])*tpar[i][1]; */
  /*   } */
  /* } */
  
  
  /**** adc &tdc channel align here ****/
  for(j=0;j<N_ADC_MOD;j++){
    for(i=0;i<16;i++){
      adc_al[j][i] = adc[j][i];
      adcc_al[j][i] = adcc[j][i];
      adcc12C_al[j][i] = adcc12C[j][i];
      tdc_cnt_al[j*N_ADC+i] = tdc_cnt[j*N_ADC+i];
      tdc12C_cnt_al[j*N_ADC+i] = tdc12C_cnt[j*N_ADC+i];
      for(k=0;k<N_DUP;k++){
	tdc_al[j*N_ADC+i][k] = tdc[j*N_ADC+i][k];
	tdcc_al[j*N_ADC+i][k] = tdcc[j*N_ADC+i][k];
	tdc12C_al[j*N_ADC+i][k] = tdc12C[j*N_ADC+i][k];
	tdcc12C_al[j*N_ADC+i][k] = tdcc12C[j*N_ADC+i][k];
      }
    }
    for(i=16;i<24;i++){
      adc_al[j][i] = adc[j][39-i];
      adcc_al[j][i] = adcc[j][39-i];
      adcc12C_al[j][i] = adcc12C[j][39-i];
      tdc_cnt_al[j*N_ADC+i] = tdc_cnt[j*N_ADC+39-i];
      tdc12C_cnt_al[j*N_ADC+i] = tdc12C_cnt[j*N_ADC+39-i];
      for(k=0;k<N_DUP;k++){
	tdc_al[j*N_ADC+i][k] = tdc[j*N_ADC+39-i][k];
	tdcc_al[j*N_ADC+i][k] = tdcc[j*N_ADC+39-i][k];
	tdc12C_al[j*N_ADC+i][k] = tdc12C[j*N_ADC+39-i][k];
	tdcc12C_al[j*N_ADC+i][k] = tdcc12C[j*N_ADC+39-i][k];
      }
    }
    for(i=24;i<32;i++){
      adc_al[j][i] = adc[j][55-i];
      adcc_al[j][i] = adcc[j][55-i];
      adcc12C_al[j][i] = adcc12C[j][55-i];
      tdc_cnt_al[j*N_ADC+i] = tdc_cnt[j*N_ADC+55-i];
      tdc12C_cnt_al[j*N_ADC+i] = tdc12C_cnt[j*N_ADC+55-i];
      for(k=0;k<N_DUP;k++){
	tdc_al[j*N_ADC+i][k] = tdc[j*N_ADC+55-i][k];
	tdcc_al[j*N_ADC+i][k] = tdcc[j*N_ADC+55-i][k];
	tdc12C_al[j*N_ADC+i][k] = tdc12C[j*N_ADC+55-i][k];
	tdcc12C_al[j*N_ADC+i][k] = tdcc12C[j*N_ADC+55-i][k];
      }
    }
  }
  /**** adc channel align above ****/

  /***No.1 adc search here***/
  /* for(i=0;i<N_ADC_MOD;i++){ */
  /*   for(j=0;j<N_ADC;j++){ */
  /*     if(adcc_al[i][j]>adccNo1[i]){ */
  /* 	adccNo1[i]=adcc_al[i][j]; */
  /*     } */
  /*   } */
  /* } */
  /***No.1 adc search above***/

  //alligned & ascending order tdc & tdcc
  for(i=0;i<N_TDC1;i++){
    for(j=0;j<tdc_cnt_al[i];j++){
      for(k=j+1;k<tdc_cnt_al[i];k++){
	if(tdc_al[i][j]>tdc_al[i][k]){
	  tmp_tdc=tdc_al[i][j];
	  tdc_al[i][j]=tdc_al[i][k];
	  tdc_al[i][k]=tmp_tdc;
	  tmp_tdcc=tdcc_al[i][j];
	  tdcc_al[i][j]=tdcc_al[i][k];
	  tdcc_al[i][k]=tmp_tdcc;
	}
      }
    }
    for(j=0;j<tdc12C_cnt_al[i];j++){
      for(k=j+1;k<tdc12C_cnt_al[i];k++){
	if(tdc12C_al[i][j]>tdc12C_al[i][k]){
	  tmp_tdc=tdc12C_al[i][j];
	  tdc12C_al[i][j]=tdc12C_al[i][k];
	  tdc12C_al[i][k]=tmp_tdc;
	  tmp_tdcc=tdcc12C_al[i][j];
	  tdcc12C_al[i][j]=tdcc12C_al[i][k];
	  tdcc12C_al[i][k]=tmp_tdcc;
	}
      }
    }

  }

   //si cnt for x+y
  for(i=0;i<N_ADC_MOD;i++){
    for(j=0;j<2;j++){
      for(k=0;k<16;k++){
	if(tdc_cnt_al[i*32+j*16+k]>0){
	  sicnt[i][j]++;
	}
	if(tdc12C_cnt_al[i*32+j*16+k]>0){
	  sicnt12C[i][j]++;
	}
      }
    }
  }
  
  //  divide tdc hits into [2][2]
  for(i=0;i<N_TDC1;i++){
    tdc_nn[(int)i/32][(i/16)%2]+=tdc_cnt[i];
    tdc12C_nn[(int)i/32][(i/16)%2]+=tdc12C_cnt[i];
  }

  double ex12c[2]={};
  double vec12c[2][4]={};
  double vec[2][3][4]={};
  double rvec[3]={};
  double ene_sum3a[N_ADC_MOD][2]={{}};//sum of 3alpha TKE [][xy]
  
  const double m12c=931.5*12;
  const double mhe=931.5*4+2.4;;
  double phe;
  
  int ichx;
  int ichy;
  double ene0;
  double ene1;
  
  /* const double theta=24./180*M_PI; */ 
  const double theta=35./180*M_PI;
  const double l = 77.5;	/* correct Si mount  */
  // const double l = 112.5;	/* mistaken Si mount */
  
  /**making tdc Gate flug here **/ 
  double tdccmin=200.;
  double tdccmax=500.;
  
  int flug[N_ADC_MOD][N_ADC]={};//(if within ADC GATE) = 1
  int FLUG[N_ADC_MOD][2]={{}};//SUM of flug [RL][xy]
  int HFflug_tdcGate[N_TDC1]={};
  int flug12C[N_ADC_MOD][N_ADC]={};//(if within ADC GATE) = 1
  int FLUG12C[N_ADC_MOD][2]={{}};//SUM of flug [RL][xy]
  
  // flug for coincidence event
  int coinflug[N_ADC_MOD][2]={{0}};//[rightleft][xy]
  double coinmin=0.;
  double coinmax=20.;
  
  for(i=0;i<N_TDC1;i++){//TDC ch = 0~63
    HF2(33,i,tdc_cnt_al[i]*2,1.0);
    if(tdc_al[i][0]!=0 && sicnt[(int)i/32][(int)((i/16)%2)]>0
       // if(tdc_al[i][0]!=0 && sicnt[(int)i/32][0]>0
       && (tdcc_al[i][0])>tdccmin && (tdcc_al[i][0])<tdccmax){
      if(adcc_al[(int)i/32][(int)i%32]>adccmin
	 && adcc_al[(int)i/32][(int)i%32]<adccmax){
	HFflug_tdcGate[i]=1;
	flug[(int)i/32][(int)i%32]=1;
	HF2(38,tdcc_al[i][0],i,1.0);
      }
    }
    if(tdc12C_al[i][0]!=0 && sicnt12C[(int)i/32][(int)((i/16)%2)]>0
       // if(tdc_al[i][0]!=0 && sicnt[(int)i/32][0]>0
       && (tdcc12C_al[i][0])>tdccmin && (tdcc12C_al[i][0])<tdccmax){
      if(adcc12C_al[(int)i/32][(int)i%32]>adcc12Cmin
	 && adcc12C_al[(int)i/32][(int)i%32]<adcc12Cmax){
	flug12C[(int)i/32][(int)i%32]=1;
      }
    }
  }
  for(i=0;i<N_ADC_MOD;i++){
    for(j=0;j<N_ADC/2;j++){
      FLUG[i][0]=FLUG[i][0]+flug[i][j];
      FLUG12C[i][0]=FLUG12C[i][0]+flug12C[i][j];
    }
    for(j=N_ADC/2;j<N_ADC;j++){
      FLUG[i][1]=FLUG[i][1]+flug[i][j];
      FLUG12C[i][1]=FLUG12C[i][1]+flug12C[i][j];
    }
  }
  HF2(34,FLUG[0][0],FLUG[1][0],1.0);
  //making tdc Gate flug above

  //  make vector of Si pixels
  int adc_ch[N_ADC_MOD][2][16]; // temporary adc channels [RL][XY][16]
  // short pixx[N_ADC_MOD][N_ADC/2]; // temporary coordinates
  //short pixy[N_ADC_MOD][N_ADC/2];
  int hit_c[N_ADC_MOD]; // temporary hits count of each Si
  unsigned int p2=0; // hit count
  unsigned int pn[N_ADC_MOD]={0}; // particle number for 11 22 33
  unsigned int p32[N_ADC_MOD][2]={{0}}; // particle number for 32 23
  unsigned int pp[2]={};
  unsigned int hit_ch[N_ADC_MOD][16][2]; // coordinates against pn
  double hit_adc[N_ADC_MOD][16][2];
  
  for(i=0;i<N_ADC_MOD;i++){
    for(j=0;j<2;j++){
      for(k=0;k<16;k++){
	adc_ch[i][j][k]=-1;
	// pixx[i][j]=-1;
	// pixy[i][j]=-1;
	hit_ch[i][k][j]=-1;
	hit_adc[i][k][j]=0.;
      }
    }
  }
  
  int p[2]={100,100};
  
  for(i=0;i<N_ADC_MOD;i++){ // loop modele 0,1
    pn[i]=0;
    /* for(k=0;k<2;k++){ */
    /*   for(j=0;j<16;j++){ */

    /* 	  adc_ch[i][k][j]=j;//checkcheck [i][k][j]?? */

    /*   } */
    /* } */
    if((tdc_nn[i][0]!=0)&&(tdc_nn[i][1]!=0)){
      p[i] = tdc_nn[i][0]-tdc_nn[i][1];
      if(p[i]==-1){
	//	printf("-1");
	p[i]=101;//for switch 
      }
    }
  }
  
  switch(p[0]){
  case 0: // same counts xy of right Si
    switch(p[1]){
    case 0://same cnt xy of both RL(1vs1,2vs2,3vs3)
      HF2(35,FLUG[0][0],FLUG[1][0],1.0);
      for(i=0;i<N_ADC_MOD;i++){
	for(j=0;j<N_ADC;j++){
	  if(tdc_cnt_al[32*i+j]>0){ // select ch
	    adc2[i][j/16][j%16]=adcc_al[i][j];
	    //	    adc_ch[i][j/16][j%16]=adc_ch[i][j/16][j%16];
	    adc_ch[i][j/16][j%16]=j%16;
	  }
	  else{
	    adc2[i][j/16][j%16]=0.;
	    adc_ch[i][j/16][j%16]=-1;
	  }
	}

	for(n=0;n<2;n++){
	  coinflug[i][n]=0;
	  for(j=0;j<16;j++){
	    for(k=j+1;k<16;k++){
	      if(adc2[i][n][j]<adc2[i][n][k]){
		tmp=adc2[i][n][j];
		num=adc_ch[i][n][j];
		adc2[i][n][j]=adc2[i][n][k];
		adc_ch[i][n][j]=adc_ch[i][n][k];
		adc2[i][n][k]=tmp;
		adc_ch[i][n][k]=num;
	      }
	    }
	  }
	}
	for(k=0;k<tdc_nn[i][0];k++){
	  hit_ch[i][pn[i]][0]=adc_ch[i][0][k];
	  hit_ch[i][pn[i]][1]=adc_ch[i][1][k];
	  hit_adc[i][pn[i]][0]=adc2[i][0][k];
	  hit_adc[i][pn[i]][1]=adc2[i][1][k];
	  pn[i]++;
	}
	for(j=0;j<pn[i];j++){
	  HF2(50+i,hit_ch[i][j][0],hit_ch[i][j][1],1.0);
	}
      }
      if(tdc_nn[0][0]==tdc_nn[1][0]){/* wether mod_0_cnt==mod_1_cnt */
	//int ichx,ichy;
	p2=tdc_nn[0][0]; /* p = both mod's cnt */
	//      printf("%u,",p2);
	/* HF2(35,FLUG[0],FLUG[1],1.0); */
	/* if(FLUG[0]>=p2 && FLUG[1]>=p2){ */
	/* 	HF2(36,FLUG[0],FLUG[1],1.0); */
	/* } */
	switch (p2){
	case 1: // hitted 1 particles 1vs1
	  for(i=0;i<N_ADC_MOD;i++){
	    for(j=0;j<p2;j++){
	      HF2(10000+p2,20*(1-i)+hit_ch[i][j][0],hit_ch[i][j][1],1.0);
	    }
	  }
	  break;
	case 2: // hitted 2 particles 2 vs 2
	  for(i=0;i<N_ADC_MOD;i++){
	    for(j=0;j<p2;j++){
	      HF2(10000+p2,20*(1-i)+hit_ch[i][j][0],hit_ch[i][j][1],1.0);
	    }
	  }
	  break;
	case 3: // hitted 3 particles 3vs3
	  HF2(36,FLUG[0][0],FLUG[1][0],1.0);
	  /* if(FLUG[0]>=p2 && FLUG[1]>=pv2){ */
	  /*   HF2(37,FLUG[0],FLUG[1],1.0); */
	  /* } */
	  /* //3 particle order with tdcc */
	  for(i=0;i<N_ADC_MOD;i++){
	    for(k=0;k<2;k++){//x & y
	      for(j=0;j<p2;j++){
		hitch_tdc_odr[i][j][k]=hit_ch[i][j][k];
	      }
	      for(j=0;j<p2;j++){
		for(jj=j+1;jj<p2;jj++){
		  double nj,njj;
		  nj=tdcc_al[i*N_ADC+k*16+hitch_tdc_odr[i][j][k]][0];
		  njj=tdcc_al[i*N_ADC+k*16+hitch_tdc_odr[i][jj][k]][0];
		  if(nj>njj){
		    //	    printf("%d %d,",j,jj);
		    itmp=hitch_tdc_odr[i][j][k];
		    hitch_tdc_odr[i][j][k]=hitch_tdc_odr[i][jj][k];
		    hitch_tdc_odr[i][jj][k]=itmp;
		  }
		}
	      }
	      
	      //3 alpha coincidence flug
	      if((tdcc_al[i*N_ADC+k*16+hitch_tdc_odr[i][1][k]][0]-tdcc_al[i*N_ADC+k*16+hitch_tdc_odr[i][0][k]][0])>=coinmin
		 &&(tdcc_al[i*N_ADC+k*16+hitch_tdc_odr[i][2][k]][0]-tdcc_al[i*N_ADC+k*16+hitch_tdc_odr[i][0][k]][0])<coinmax){
		coinflug[i][k]=1;
	      }
	      
	      //hist of tdc difference between 3 alphas
	      for(j=1;j<p2;j++){
		HF1(3000+i,tdcc_al[i*N_ADC+k*16+hitch_tdc_odr[i][j][k]][0]-tdcc_al[i*N_ADC+k*16+hitch_tdc_odr[i][0][k]][0],1.0);
		if(FLUG[0][0]>=p2 && FLUG[1][0]>=p2){
		  HF1(3010+i,tdcc_al[i*N_ADC+k*16+hitch_tdc_odr[i][j][k]][0]-tdcc_al[i*N_ADC+k*16+hitch_tdc_odr[i][0][k]][0],1.0);
		  if(coinflug[i][k]){
		    HF1(3020+i,tdcc_al[i*N_ADC+k*16+hitch_tdc_odr[i][j][k]][0]-tdcc_al[i*N_ADC+k*16+hitch_tdc_odr[i][0][k]][0],1.0);
		  }
		}
	      }
	    }
	  }
	  /* for(i=0;i<N_TDC1;i++){ */
	  /*   HF1(47,tdcc_al[i][0],1.0); */
	  /* } */
	  
	  //hist of check for nombering 6alpha
	  for(i=0;i<N_ADC_MOD;i++){
	    for(j=0;j<p2;j++){
	      if(FLUG[0][0]>=p2 && FLUG[1][0]>=p2){
		if(coinflug[i][0]*coinflug[i][1]){//3 alpha coin x y
		  if(coinflug[(i+1)%2][0]*coinflug[(i+1)%2][1]){//6alpha coin x y
		    HF2(3030+3*i+j,hit_adc[i][j][0]-hit_adc[i][j][1],
			tdcc_al[i*N_ADC+0*16+hit_ch[i][j][0]][0]-tdcc_al[i*N_ADC+1*16+hit_ch[i][j][1]][0],1.0);
		    // HF2(3040+3*i+j,hit_adc[i][j][0],
		    //	tdcc_al[i*N_ADC+0*16+hit_ch[i][j][0]][0]-tdcc_al[i*N_ADC+1*16+hit_ch[i][j][1]][0],1.0);
		  }
		}
	      }
	    }
	  }

	  /**!!!!invariant mass of 12C **/ 
	  for(i=0;i<N_ADC_MOD;i++){
	    //  printf("test8");
	    for(j=0;j<p2;j++){
	      ichx=hit_ch[i][j][0];
	      ichy=hit_ch[i][j][1];
	      ene0=hit_adc[i][j][0];
	      ene1=hit_adc[i][j][1];
	      ene_sum3a[i][0]=ene_sum3a[i][0]+hit_adc[i][j][0];//x
	      ene_sum3a[i][1]+=hit_adc[i][j][1];//y
	      
	      HF2(10000+p2,20*(1-i)+ichx,ichy,1.0);
	      if(FLUG[0][0]>=p2 && FLUG[1][0]>=p2){
		if(coinflug[0][0]*coinflug[0][1]){//3 alpha coin x y right
		  if(coinflug[1][0]*coinflug[1][1]){//6alpha coin x y
		    HF2(10030+p2,20*(1-i)+ichx,ichy,1.0);
		  }
		}
	      }
	      
	      vec[i][j][0]=ene0+mhe;
	      
	      rvec[0]=((double)ichx-7.5)*3.0;
	      rvec[1]=(7.5-(double)ichy)*3.0;
	      rvec[2]=l;
	      
	      unitvec(rvec,rvec);
	      rotvec(rvec,&vec[i][j][1],1,pow(-1,i)*theta);
	      
	      phe=sqrt(vec[i][j][0]*vec[i][j][0]-mhe*mhe);
	      vecadd(&vec[i][j][1],rvec,&vec[i][j][1],phe,0.0);
	      vecadd4(vec[i][j],vec12c[i],vec12c[i],1,1);
	      //	    printf("%d:%d  p:%8.3f ene:%8.3f\n",i,j,phe,ene0);
	      //printf("%d:%d: %8.3f   \n",i,j,
	      //	   sqrt(scapro4(vec[i][j],vec[i][j])));
	    }
	    ex12c[i]=sqrt(scapro4(vec12c[i],vec12c[i]))-m12c;
	    //	  printf("ex:%8.3f\n",ex12c[i]);
	    HF1(50000+i,ex12c[i],1.0);
	    if(FLUG[0][0]>=p2 && FLUG[1][0]>=p2){//tdc gate
	      HF1(50010+i,ex12c[i],1.0);
	      if(coinflug[i][0]*coinflug[i][1]){//3 alpha coin x y
		HF1(50020+i,ex12c[i],1.0);
		if(coinflug[(i+1)%2][0]*coinflug[(i+1)%2][1]){
		  HF1(50030+i,ex12c[i],1.0);
		  HF1(50200+2*i+0,ene_sum3a[i][0],1.0);
		  HF1(50200+2*i+1,ene_sum3a[i][1],1.0);
		}
	      }
	    }
	  }
	  HF2(50002,ex12c[0],ex12c[1],1.0);
	  if(FLUG[0][0]>=p2 && FLUG[1][0]>=p2){
	    HF2(50012,ex12c[0],ex12c[1],1.0);
	    if(coinflug[0][0]*coinflug[0][1]){//3 alpha coin x y right
	      if(coinflug[1][0]*coinflug[1][1]){//6alpha coin x y
		HF2(50032,ex12c[0],ex12c[1],1.0);
		HF2(50033,ex12c[0],ex12c[1],1.0);
		/* FILE *fright3a,*fleft3a; */
		/* fright3a = fopen("dat/3a_result_right.txt","a"); */
		/* fleft3a = fopen("dat/3a_result_left.txt","a"); */
		/* if(fright3a==NULL){ */
		/*   puts("3a result right file cannot open"); */
		/*   return -1; */
		/* } */
		/* if(fleft3a==NULL){ */
		/*   puts("3a result left file cannot open"); */
		/*   return -1; */
		/* } */
		/* if(ex12c[0]>=7.1&&ex12c[0]<=8.0 */
		/*    &&ex12c[1]>=7.4&&ex12c[1]<=7.8){ */
		/*   fprintf(fright3a,"%lf %d %d %lf %d %d %lf %d %d\n", */
		/* 	  hit_adc[0][0][0],hit_ch[0][0][0],hit_ch[0][0][1], */
		/* 	  hit_adc[0][1][0],hit_ch[0][1][0],hit_ch[0][1][1], */
		/* 	  hit_adc[0][2][0],hit_ch[0][2][0],hit_ch[0][2][1]); */
		/*   fprintf(fleft3a,"%lf %d %d %lf %d %d %lf %d %d\n", */
		/* 	  hit_adc[1][0][0],hit_ch[1][0][0],hit_ch[1][0][1], */
		/* 	  hit_adc[1][1][0],hit_ch[1][1][0],hit_ch[1][1][1], */
		/* 	  hit_adc[1][2][0],hit_ch[1][2][0],hit_ch[1][2][1]); */
		/* } */
		/* fclose(fright3a); */
		/* fclose(fleft3a); */
		for(i=0;i<N_TDC1;i++){
		  if(tdcc_al[i][0]!=0.){
		  HF1(45,tdcc_al[i][0],1.0);
		  HF2(46,adcc_al[(int)i/32][(int)i%32],tdc_al[i][0]-tzero,1.0);
		  }
		}
		//all alpha check
		/*    FILE *outfile; */
		/*       outfile = fopen("dat/6alpha.dat","a"); */
		/*       if(outfile == NULL){ */
		/* 	printf("cannot open output file\n"); */
		/* 	exit(1); */
		/*       } */
		/*       for() */
		/*       fprintf(outfile,"%d,%d,%f,", */
		/* 	      i,tdcc_al[i][0],adcc_al[(int)i/32][(int)i%32]); */
		/*     } */
		/*   } */
		/*   fprintf(outfile,"\n"); */
		/*   fclose(outfile); */
		/* } */
	      }
	    }
	  }
	  break;
	default:
	  break;
	}
      }
      break;
      /*p[0],p[1]=0 finnish 1vs1 2vs2 3vs3 above */

    case 1://p[0]=0&&p[1]=1
      break;
      
    case 101:
      break;
    default:
      break;
    }
  case 1://p[0]=1 
    break;
  case 101:
    break;
  default:
    break;
  }

  /******************************************************************/
  
  /**3vs3 && 3vs2event**/
  
  for(i=0;i<N_ADC_MOD;i++){
    /*clear hit_ch*/
    for(j=0;j<2;j++){
      for(k=0;k<16;k++){
	adc2[i][j][k]=0.;adc2[(i+1)%2][j][k]=0.;
	adc12C2[i][j][k]=0.;adc12C2[(i+1)%2][j][k]=0.;
	adc_ch[i][j][k]=-1;adc_ch[(i+1)%2][j][k]=-1;
      	hit_ch[i][k][j]=-1;hit_ch[(i+1)%2][k][j]=-1;
	hitch_tdc_odr[i][k][j]=-1;
	hitch_tdc_odr[(i+1)%2][k][j]=-1;
      	hit_adc[i][k][j]=0.;
	hit_adc[(i+1)%2][k][j]=0.;
      }
    }
    for(j=0;j<2;j++){
      p32[i][j]=0;
      p32[(i+1)%2][j]=0;
    }

    //i=0->R:3alpha,L=32  i=1->R=32,L:3alpha
    if(tdc_nn[i][0]==3&&tdc_nn[i][1]==3){//MOD i = 33

      /******i=0->R=3alpha, L:x=2,y=3*******/
      /******i=1->R:x=2,y=3, L=3alpha*******/
      if(tdc_nn[(i+1)%2][0]==3&&tdc12C_nn[(i+1)%2][1]==2){//x=3,y=2
	for(j=0;j<N_ADC;j++){
	  if(tdc_cnt_al[32*i+j]>0){ //MOD i x3 y3
	    adc2[i][j/16][j%16]=adcc_al[i][j];
	    adc_ch[i][j/16][j%16]=j%16;
	  }
	  if(j<N_ADC/2){
	    if(tdc_cnt_al[32*((i+1)%2)+j]>0){
	      adc2[(i+1)%2][(int)j/16][j%16]
		=adcc_al[(i+1)%2][j];
	      adc_ch[(i+1)%2][j/16][j%16]=j%16;
	    }
	  }
	  if(j>=N_ADC/2){
	    if(tdc12C_cnt_al[32*((i+1)%2)+j]>0){
	      adc12C2[(i+1)%2][(int)j/16][j%16]
		=adcc12C_al[(i+1)%2][j];
	      adc_ch[(i+1)%2][j/16][j%16]=j%16;
	    }
	  }
	}
	for(n=0;n<2;n++){
	  coinflug[i][n]=0;coinflug[(i+1)%2][n]=0;
	  for(j=0;j<16;j++){
	    for(k=j+1;k<16;k++){
	      if(adc2[i][n][j]<adc2[i][n][k]){//MODi x3y3 order
		tmp=adc2[i][n][j];num=adc_ch[i][n][j];
		adc2[i][n][j]=adc2[i][n][k];
		adc_ch[i][n][j]=adc_ch[i][n][k];
		adc2[i][n][k]=tmp;adc_ch[i][n][k]=num;
	      }
	      if(n==0){//3alpha of x order
		if(adc2[(i+1)%2][n][j]<adc2[(i+1)%2][n][k]){
		  tmp=adc2[(i+1)%2][n][j];
		  num=adc_ch[(i+1)%2][n][j];
		  adc2[(i+1)%2][n][j]=adc2[(i+1)%2][n][k];
		  adc_ch[(i+1)%2][n][j]=adc_ch[(i+1)%2][n][k];
		  adc2[(i+1)%2][n][k]=tmp;
		  adc_ch[(i+1)%2][n][k]=num;
		}
	      }
	      if(n==1){//2alpha signal y order
		if(adc12C2[(i+1)%2][n][j]<adc12C2[(i+1)%2][n][k]){
		  tmp=adc12C2[(i+1)%2][n][j];
		  num=adc_ch[(i+1)%2][n][j];
		  adc12C2[(i+1)%2][n][j]=adc12C2[(i+1)%2][n][k];
		  adc_ch[(i+1)%2][n][j]=adc_ch[(i+1)%2][n][k];
		  adc12C2[(i+1)%2][n][k]=tmp;
		  adc_ch[(i+1)%2][n][k]=num;
		}
	      }
	    }
	  }
	}
	for(j=0;j<2;j++){
	  for(k=0;k<tdc_nn[i][j];k++){
	    hit_ch[i][p32[i][j]][j]=adc_ch[i][j][k];
	    hit_adc[i][p32[i][j]][j]=adc2[i][j][k];
	    p32[i][j]++;
	  }
	}
	for(k=0;k<tdc_nn[(i+1)%2][0];k++){
	  hit_ch[(i+1)%2][p32[(i+1)%2][0]][0]
	    =adc_ch[(i+1)%2][0][k];
	  hit_adc[(i+1)%2][p32[(i+1)%2][0]][0]
	    =adc2[(i+1)%2][0][k];
	  p32[(i+1)%2][0]++;	  
	}
	for(k=0;k<tdc12C_nn[(i+1)%2][1];k++){
	  hit_ch[(i+1)%2][p32[(i+1)%2][1]][1]
	    =adc_ch[(i+1)%2][1][k];
	  hit_adc[(i+1)%2][p32[(i+1)%2][1]][1]
	    =adc12C2[(i+1)%2][1][k];
	  p32[(i+1)%2][1]++;
	}
	
	p2=tdc_nn[(i+1)%2][0];//p2=3

	/*divide not MODi y -> 3 signals */
	  double diffadc[6]={};//adc difference for 3*3 solve
	  double mindiffadc=100;
	  int ix1,iy1;//1 particle not divided
	  int ix2[2],iy2[2];//y will be devided 2 by x adc

	  for(k=0;k<2;k++){//two signals of y
	    for(j=0;j<p2;j++){
	      ix2[0]=(j-1)*(j-2)/2;//0->1,1,2->0
	      ix2[1]=((j-1)*(j-2)/2+j)%2+1;//0,1->2,2->1
	      diffadc[3*k+j]=fabs(hit_adc[(i+1)%2][k][1]
	      			  -hit_adc[(i+1)%2][j][0])
	      	+2*fabs(hit_adc[(i+1)%2][(k+1)%2][1]
			-hit_adc[(i+1)%2][ix2[0]][0]
			-hit_adc[(i+1)%2][ix2[1]][0]);
	      /* diffadc[3*k+j]=(hit_adc[(i+1)%2][k][1]/hit_adc[(i+1)%2][j][0]) */
	      /* 	/(hit_adc[(i+1)%2][(k+1)%2][1]/(hit_adc[(i+1)%2][ix2[0]][0]+hit_adc[(i+1)%2][ix2[1]][0])); */
	      /* diffadc[3*k+j]=fabs(1.-diffadc[3*k+j]); */
	    }
	  }
	  //  printf("......");
	  for(j=0;j<6;j++){//search min of x y adcc
	    if(diffadc[j]<mindiffadc){
	      mindiffadc=diffadc[j];
	      iy1=(int)j/3;
	      iy2[0]=(iy1+1)%2;
	      iy2[1]=2;
	      if(j<=2){
		ix1=j;
		ix2[0]=(ix1-1)*(ix1-2)/2;//0->1,1,2->0
		ix2[1]=((ix1-1)*(ix1-2)/2+ix1)%2+1;//0,1->2,2->1
	      }
	      else if(j>2){
		ix1=j-3;
		ix2[0]=(ix1-1)*(ix1-2)/2;//0->1,1,2->0
		ix2[1]=((ix1-1)*(ix1-2)/2+ix1)%2+1;//0,1->2,2->1
	      }
	    }
	  }
	  // printf("%d %d %lf,",ix2[0],ix2[1],mindiffadc);
 	  HF2(50300+i,
	      hit_adc[(i+1)%2][ix2[0]][0]+hit_adc[(i+1)%2][ix2[1]][0],
	      hit_adc[(i+1)%2][iy2[0]][1],1.0);
	  HF2(50305+i,
	      hit_adc[(i+1)%2][0][0]+hit_adc[(i+1)%2][1][0],hit_adc[(i+1)%2][0][1],1.0);
	  int flugdevide=0; //whether divide correctly or not
	  double finaldiff=10.;
	  finaldiff = fabs(hit_adc[(i+1)%2][ix2[0]][0]
			   +hit_adc[(i+1)%2][ix2[1]][0]
			   -hit_adc[(i+1)%2][iy2[0]][1]);
	  if(finaldiff<=diff32result){
	    flugdevide=1;
	  }
	  if(flugdevide){
	    //divide y2 into 2signals
	    hit_ch[(i+1)%2][iy2[1]][1]=hit_ch[(i+1)%2][iy2[0]][1];
	    hit_adc[(i+1)%2][iy2[1]][1]
	      =hit_adc[(i+1)%2][iy2[0]][1]*hit_adc[(i+1)%2][ix2[1]][0]
	      /(hit_adc[(i+1)%2][ix2[0]][0]+hit_adc[(i+1)%2][ix2[1]][0]);
	    hit_adc[(i+1)%2][iy2[0]][1]=hit_adc[(i+1)%2][iy2[0]][1]*hit_adc[(i+1)%2][ix2[0]][0]
	      /(hit_adc[(i+1)%2][ix2[0]][0]+hit_adc[(i+1)%2][ix2[1]][0]);
	    //MOD(i+1)%2 y reorder with adcc
	    for(k=0;k<p2;k++){
	      for(j=k+1;j<p2;j++){
		if(hit_adc[(i+1)%2][k][1]<hit_adc[(i+1)%2][j][1]){
		  tmp=hit_adc[(i+1)%2][k][1];
		  num=hit_ch[(i+1)%2][k][1];
		  hit_adc[(i+1)%2][k][1]=hit_adc[(i+1)%2][j][1];
		  hit_ch[(i+1)%2][k][1]=hit_ch[(i+1)%2][j][1];
		  hit_adc[(i+1)%2][j][1]=tmp;
		  hit_ch[(i+1)%2][j][1]=num;
		}
	      }
	    }
	    
	    //3 particle order with tdcc
	    for(n=0;n<N_ADC_MOD;n++){
	      for(k=0;k<2;k++){//x & y
		for(j=0;j<p2;j++){
		  hitch_tdc_odr[n][j][k]=hit_ch[n][j][k];
		}
		for(j=0;j<p2;j++){
		  for(jj=j+1;jj<p2;jj++){
		    double nj,njj;
		    nj=tdcc_al[n*N_ADC+k*16+hitch_tdc_odr[n][j][k]][0];
		    njj=tdcc_al[n*N_ADC+k*16+hitch_tdc_odr[n][jj][k]][0];
		    if(nj>njj){
		      //	    printf("%d %d,",j,jj);
		      itmp=hitch_tdc_odr[n][j][k];
		      hitch_tdc_odr[n][j][k]=hitch_tdc_odr[n][jj][k];
		      hitch_tdc_odr[n][jj][k]=itmp;
		    }
		  }
		}
		//3 alpha coincidence flug
		if((tdcc_al[n*N_ADC+k*16+hitch_tdc_odr[n][1][k]][0]-tdcc_al[n*N_ADC+k*16+hitch_tdc_odr[n][0][k]][0])>=coinmin
		   &&(tdcc_al[n*N_ADC+k*16+hitch_tdc_odr[n][2][k]][0]-tdcc_al[n*N_ADC+k*16+hitch_tdc_odr[n][0][k]][0])<coinmax){
		  // divede y adc gate
		  if(hit_adc[n][0][k]>adccmin&&hit_adc[n][0][k]<adccmax
		     &&hit_adc[n][1][k]>adccmin&&hit_adc[n][1][k]<adccmax
		     &&hit_adc[n][2][k]>adccmin&&hit_adc[n][2][k]<adccmax){
		    coinflug[n][k]=1;
		  }
		}
		for(j=1;j<p2;j++){
		  HF1(3040+2*i+n,tdcc_al[n*N_ADC+k*16+hitch_tdc_odr[n][j][k]][0]-tdcc_al[n*N_ADC+k*16+hitch_tdc_odr[n][0][k]][0],1.0);
		  if(FLUG[0][0]>=p2 && FLUG[1][0]>=p2){//tdc gate only x
		    if(coinflug[n][k]){
		      HF1(3050+2*i+n,tdcc_al[n*N_ADC+k*16+hitch_tdc_odr[n][j][k]][0]-tdcc_al[n*N_ADC+k*16+hitch_tdc_odr[n][0][k]][0],1.0);
		    }
		  }
		}
	      }
	    }
	    
	    /**!!!!invariant mass of 12C**/	  
	    for(n=0;n<N_ADC_MOD;n++){
	      //  printf("test8");
	      for(j=0;j<p2;j++){
		ichx=hit_ch[n][j][0];
		ichy=hit_ch[n][j][1];
		ene0=hit_adc[n][j][0];
		ene1=hit_adc[n][j][1];
		
		if(FLUG[0][0]>=p2 && FLUG[1][0]>=p2){
		  if(coinflug[0][0]*coinflug[0][1]){//3 alpha coin x y right
		    if(coinflug[1][0]*coinflug[1][1]){//6alpha coin x y
		      HF2(10050+p2*i,20*(1-n)+ichx,ichy,1.0);
		    }
		  }
		}
		
		vec[n][j][0]=ene0+mhe;
		
		rvec[0]=((double)ichx-7.5)*3.0;
		rvec[1]=(7.5-(double)ichy)*3.0;
		rvec[2]=l;
		
		unitvec(rvec,rvec);
		rotvec(rvec,&vec[n][j][1],1,pow(-1,n)*theta);
		
		phe=sqrt(vec[n][j][0]*vec[n][j][0]-mhe*mhe);
		vecadd(&vec[n][j][1],rvec,&vec[n][j][1],phe,0.0);
		vecadd4(vec[n][j],vec12c[n],vec12c[n],1,1);
	      }
	      ex12c[n]=sqrt(scapro4(vec12c[n],vec12c[n]))-m12c;
	      HF1(50040+2*i+n,ex12c[n],1.0);
	      if(FLUG[0][0]>=p2 && FLUG[1][0]>=p2){//tdc gate
		if(coinflug[n][0]*coinflug[n][1]){//3 alpha coin x y
		  if(coinflug[(n+1)%2][0]*coinflug[(n+1)%2][1]){
		    HF1(50050+2*i+n,ex12c[n],1.0);
		  }
		}
	      }
	    }
	    // HF2(50042,ex12c[0],ex12c[1],1.0);
	    if(FLUG[0][0]>=p2 && FLUG[1][0]>=p2){
	      if(coinflug[0][0]*coinflug[0][1]){//3 alpha coin x y right
		if(coinflug[1][0]*coinflug[1][1]){//6alpha coin x y
		  HF2(50055+i,ex12c[0],ex12c[1],1.0);
		  HF2(50057+i,ex12c[0],ex12c[1],1.0);
		}
	      }
	    }
	  }
      }
      
      /******i=0->R=3alpha, L:x=2,y=3*******/
      /******i=1->R:x=2,y=3, L=3alpha*******/
      
      if(tdc12C_nn[(i+1)%2][0]==2&&tdc_nn[(i+1)%2][1]==3){//x=2,y=3
	for(j=0;j<N_ADC;j++){
	  if(tdc_cnt_al[32*i+j]>0){ //MOD i x3 y3
	    adc2[i][j/16][j%16]=adcc_al[i][j];
	    adc_ch[i][j/16][j%16]=j%16;
	  }
	  if(j>=N_ADC/2){
	    if(tdc_cnt_al[32*((i+1)%2)+j]>0){
	      adc2[(i+1)%2][(int)j/16][j%16]
		=adcc_al[(i+1)%2][j];
	      adc_ch[(i+1)%2][j/16][j%16]=j%16;
	    }
	  }
	  if(j<N_ADC/2){
	    if(tdc12C_cnt_al[32*((i+1)%2)+j]>0){
	      adc12C2[(i+1)%2][(int)j/16][j%16]
		=adcc12C_al[(i+1)%2][j];
	      adc_ch[(i+1)%2][j/16][j%16]=j%16;
	    }
	  }
	}
	for(n=0;n<2;n++){
	  coinflug[i][n]=0;coinflug[(i+1)%2][n]=0;
	  for(j=0;j<16;j++){
	    for(k=j+1;k<16;k++){
	      if(adc2[i][n][j]<adc2[i][n][k]){//MODi x3y3 order
		tmp=adc2[i][n][j];num=adc_ch[i][n][j];
		adc2[i][n][j]=adc2[i][n][k];
		adc_ch[i][n][j]=adc_ch[i][n][k];
		adc2[i][n][k]=tmp;adc_ch[i][n][k]=num;
	      }
	      if(n==1){//3alpha of y order
		if(adc2[(i+1)%2][n][j]<adc2[(i+1)%2][n][k]){
		  tmp=adc2[(i+1)%2][n][j];
		  num=adc_ch[(i+1)%2][n][j];
		  adc2[(i+1)%2][n][j]=adc2[(i+1)%2][n][k];
		  adc_ch[(i+1)%2][n][j]=adc_ch[(i+1)%2][n][k];
		  adc2[(i+1)%2][n][k]=tmp;
		  adc_ch[(i+1)%2][n][k]=num;
		}
	      }
	      if(n==0){//2alpha signal x order
		if(adc12C2[(i+1)%2][n][j]<adc12C2[(i+1)%2][n][k]){
		  tmp=adc12C2[(i+1)%2][n][j];
		  num=adc_ch[(i+1)%2][n][j];
		  adc12C2[(i+1)%2][n][j]=adc12C2[(i+1)%2][n][k];
		  adc_ch[(i+1)%2][n][j]=adc_ch[(i+1)%2][n][k];
		  adc12C2[(i+1)%2][n][k]=tmp;
		  adc_ch[(i+1)%2][n][k]=num;
		}
	      }
	    }
	  }
	}
	for(j=0;j<2;j++){
	  for(k=0;k<tdc_nn[i][j];k++){
	    hit_ch[i][p32[i][j]][j]=adc_ch[i][j][k];
	    hit_adc[i][p32[i][j]][j]=adc2[i][j][k];
	    p32[i][j]++;
	  }
	}
	for(k=0;k<tdc_nn[(i+1)%2][1];k++){//y
	  hit_ch[(i+1)%2][p32[(i+1)%2][1]][1]
	    =adc_ch[(i+1)%2][1][k];
	  hit_adc[(i+1)%2][p32[(i+1)%2][1]][1]
	    =adc2[(i+1)%2][1][k];
	  p32[(i+1)%2][1]++;	  
	}
	for(k=0;k<tdc12C_nn[(i+1)%2][0];k++){//x
	  hit_ch[(i+1)%2][p32[(i+1)%2][0]][0]
	    =adc_ch[(i+1)%2][0][k];
	  hit_adc[(i+1)%2][p32[(i+1)%2][0]][0]
	    =adc12C2[(i+1)%2][0][k];
	  p32[(i+1)%2][0]++;
	}
	
	p2=tdc_nn[(i+1)%2][1];//p2=3

	/*divide not MODi x -> 3 signals */
	double diffadc[6]={};//adc difference for 3*3 solve
	double mindiffadc=100;
	int ix1,iy1;//1 particle not divided
	int ix2[2],iy2[2];//x will be devided 2 by y adc
	
	for(k=0;k<2;k++){//two signals of x
	  for(j=0;j<p2;j++){//three signals of y
	    iy2[0]=(j-1)*(j-2)/2;//0->1,1,2->0
	    iy2[1]=((j-1)*(j-2)/2+j)%2+1;//0,1->2,2->1
	    diffadc[3*k+j]=fabs(hit_adc[(i+1)%2][k][0]
				-hit_adc[(i+1)%2][j][1])
	      +2*fabs(hit_adc[(i+1)%2][(k+1)%2][0]
		      -hit_adc[(i+1)%2][iy2[0]][1]
			-hit_adc[(i+1)%2][iy2[1]][1]);
	  }
	}
	for(j=0;j<6;j++){//search min of x y adcc
	  if(diffadc[j]<mindiffadc){
	    mindiffadc=diffadc[j];
	    ix1=(int)j/3;
	    ix2[0]=(ix1+1)%2;
	    ix2[1]=2;
	    if(j<=2){
	      iy1=j;
	      iy2[0]=(iy1-1)*(iy1-2)/2;//0->1,1,2->0
	      iy2[1]=((iy1-1)*(iy1-2)/2+iy1)%2+1;//0,1->2,2->1
	    }
	    else if(j>2){
	      iy1=j-3;
	      iy2[0]=(iy1-1)*(iy1-2)/2;//0->1,1,2->0
	      iy2[1]=((iy1-1)*(iy1-2)/2+iy1)%2+1;//0,1->2,2->1
	    }
	  }
	}
	// printf("%d %d %lf,",ix2[0],ix2[1],mindiffadc);
	HF2(50302+i
	    ,hit_adc[(i+1)%2][ix2[0]][0]
	    ,hit_adc[(i+1)%2][iy2[0]][1]+hit_adc[(i+1)%2][iy2[1]][1],1.0);
	
	int flugdevide=0; //whether divide correctly or not
	double finaldiff=10.;
	finaldiff = fabs(hit_adc[(i+1)%2][iy2[0]][1]
			 +hit_adc[(i+1)%2][iy2[1]][1]
			 -hit_adc[(i+1)%2][ix2[0]][0]);
	if(finaldiff<=diff32result){
	  flugdevide=1;
	}
	if(flugdevide){
	  //divide x2 into 2signals
	  hit_ch[(i+1)%2][ix2[1]][0]=hit_ch[(i+1)%2][ix2[0]][0];
	  hit_adc[(i+1)%2][ix2[1]][0]
	    =hit_adc[(i+1)%2][ix2[0]][0]*hit_adc[(i+1)%2][iy2[1]][1]
	    /(hit_adc[(i+1)%2][iy2[0]][1]+hit_adc[(i+1)%2][iy2[1]][1]);
	  hit_adc[(i+1)%2][ix2[0]][0]
	    =hit_adc[(i+1)%2][ix2[0]][0]*hit_adc[(i+1)%2][iy2[0]][1]
	    /(hit_adc[(i+1)%2][iy2[0]][1]+hit_adc[(i+1)%2][iy2[1]][1]);
	  //MOD(i+1)%2 x reorder with adcc
	  for(k=0;k<p2;k++){
	    for(j=k+1;j<p2;j++){
	      if(hit_adc[(i+1)%2][k][0]<hit_adc[(i+1)%2][j][0]){
		tmp=hit_adc[(i+1)%2][k][0];
		num=hit_ch[(i+1)%2][k][0];
		hit_adc[(i+1)%2][k][0]=hit_adc[(i+1)%2][j][0];
		hit_ch[(i+1)%2][k][0]=hit_ch[(i+1)%2][j][0];
		hit_adc[(i+1)%2][j][0]=tmp;
		hit_ch[(i+1)%2][j][0]=num;
	      }
	    }
	  }
	    
	  //3 particle order with tdcc
	  for(n=0;n<N_ADC_MOD;n++){
	    for(k=0;k<2;k++){//x & y
	      for(j=0;j<p2;j++){
		hitch_tdc_odr[n][j][k]=hit_ch[n][j][k];
	      }
	      for(j=0;j<p2;j++){
		for(jj=j+1;jj<p2;jj++){
		  double nj,njj;
		  nj=tdcc_al[n*N_ADC+k*16+hitch_tdc_odr[n][j][k]][0];
		  njj=tdcc_al[n*N_ADC+k*16+hitch_tdc_odr[n][jj][k]][0];
		  if(nj>njj){
		    //	    printf("%d %d,",j,jj);
		    itmp=hitch_tdc_odr[n][j][k];
		    hitch_tdc_odr[n][j][k]=hitch_tdc_odr[n][jj][k];
		    hitch_tdc_odr[n][jj][k]=itmp;
		  }
		}
	      }
	      //3 alpha coincidence flug
	      if((tdcc_al[n*N_ADC+k*16+hitch_tdc_odr[n][1][k]][0]-tdcc_al[n*N_ADC+k*16+hitch_tdc_odr[n][0][k]][0])>=coinmin
		 &&(tdcc_al[n*N_ADC+k*16+hitch_tdc_odr[n][2][k]][0]-tdcc_al[n*N_ADC+k*16+hitch_tdc_odr[n][0][k]][0])<coinmax){
		//adc gate again including divide x adc gate
		if(hit_adc[n][0][k]>adccmin&&hit_adc[n][0][k]<adccmax
		   &&hit_adc[n][1][k]>adccmin&&hit_adc[n][1][k]<adccmax
		   &&hit_adc[n][2][k]>adccmin&&hit_adc[n][2][k]<adccmax){
		  coinflug[n][k]=1;
		}
	      }
	      for(j=1;j<p2;j++){
		HF1(3044+2*i+n,tdcc_al[n*N_ADC+k*16+hitch_tdc_odr[n][j][k]][0]-tdcc_al[n*N_ADC+k*16+hitch_tdc_odr[n][0][k]][0],1.0);
		if(FLUG[0][1]>=p2 && FLUG[1][1]>=p2){//tdc gate only y
		  if(coinflug[n][k]){
		    HF1(3054+2*i+n,tdcc_al[n*N_ADC+k*16+hitch_tdc_odr[n][j][k]][0]-tdcc_al[n*N_ADC+k*16+hitch_tdc_odr[n][0][k]][0],1.0);
		  }
		}
	      }
	    }
	  }
	  
	  /**!!!!invariant mass of 12C**/	  
	  for(n=0;n<N_ADC_MOD;n++){
	    //  printf("test8");
	    for(j=0;j<p2;j++){
	      ichx=hit_ch[n][j][0];
	      ichy=hit_ch[n][j][1];
	      ene0=hit_adc[n][j][0];
	      ene1=hit_adc[n][j][1];
	      
	      if(FLUG[0][1]>=p2 && FLUG[1][1]>=p2){
		if(coinflug[0][0]*coinflug[0][1]){//3 alpha coin x y right
		  if(coinflug[1][0]*coinflug[1][1]){//6alpha coin x y
		    HF2(10060+p2*i,20*(1-n)+ichx,ichy,1.0);
		  }
		}
	      }
	      
	      vec[n][j][0]=ene1+mhe;//using x adc
	      
	      rvec[0]=((double)ichx-7.5)*3.0;
	      rvec[1]=(7.5-(double)ichy)*3.0;
	      rvec[2]=l;
	      
	      unitvec(rvec,rvec);
	      rotvec(rvec,&vec[n][j][1],1,pow(-1,n)*theta);
	      
	      phe=sqrt(vec[n][j][0]*vec[n][j][0]-mhe*mhe);
	      vecadd(&vec[n][j][1],rvec,&vec[n][j][1],phe,0.0);
		vecadd4(vec[n][j],vec12c[n],vec12c[n],1,1);
	    }
	    ex12c[n]=sqrt(scapro4(vec12c[n],vec12c[n]))-m12c;
	      HF1(50060+2*i+n,ex12c[n],1.0);
	      if(FLUG[0][1]>=p2 && FLUG[1][1]>=p2){//tdc gate
		if(coinflug[n][0]*coinflug[n][1]){//3 alpha coin x y
		  if(coinflug[(n+1)%2][0]*coinflug[(n+1)%2][1]){
		    HF1(50070+2*i+n,ex12c[n],1.0);
		  }
		}
	      }
	  }
	  // HF2(50042,ex12c[0],ex12c[1],1.0);
	  if(FLUG[0][1]>=p2 && FLUG[1][1]>=p2){
	    if(coinflug[0][0]*coinflug[0][1]){//3 alpha coin x y right
	      if(coinflug[1][0]*coinflug[1][1]){//6alpha coin x y
		HF2(50075+i,ex12c[0],ex12c[1],1.0);
		HF2(50077+i,ex12c[0],ex12c[1],1.0);
	      }
	    }
	  }
	}
      }
      
    }
  }    

   /******************************************************************/
  
  /**3vs2 && 3vs2event**/
  
  for(i=0;i<N_ADC_MOD;i++){
    /*clear hit_ch*/
    for(n=0;n<N_ADC_MOD;n++){
      for(j=0;j<2;j++){
	for(k=0;k<16;k++){
	  adc2[n][j][k]=0.;
	  adc12C2[n][j][k]=0.;
	  adc_ch[n][j][k]=-1;
	  hit_ch[n][k][j]=-1;
	  hitch_tdc_odr[n][k][j]=-1;
	  hit_adc[n][k][j]=0.;
	}
      }
      for(j=0;j<2;j++){
	p32[n][j]=0;
      }
    }

    //i=0->R:32,L23  i=1->R=23,L:32
    if(tdc12C_nn[i][0]==2&&tdc_nn[i][1]==3){//MOD i x=2,y=3

      /******i=0->R=32, L:x=2,y=3*******/
      /******i=1->R:x=2,y=3, L=32*******/
      if(tdc_nn[(i+1)%2][0]==3&&tdc12C_nn[(i+1)%2][1]==2){//x=3,y=2
	for(j=0;j<N_ADC;j++){
	  if(j<N_ADC/2){
	    if(tdc12C_cnt_al[32*i+j]>0){ //MOD i x2
	      adc12C2[i][j/16][j%16]=adcc12C_al[i][j];
	      adc_ch[i][j/16][j%16]=j%16;
	    }
	    if(tdc_cnt_al[32*((i+1)%2)+j]>0){
	      adc2[(i+1)%2][(int)j/16][j%16]
		=adcc_al[(i+1)%2][j];
	      adc_ch[(i+1)%2][j/16][j%16]=j%16;
	    }
	  }
	  if(j>=N_ADC/2){
	    if(tdc_cnt_al[32*i+j]>0){
	      adc2[i][(int)j/16][j%16]
		=adcc_al[i][j];
	      adc_ch[i][j/16][j%16]=j%16;
	    }
	    if(tdc12C_cnt_al[32*((i+1)%2)+j]>0){
	      adc12C2[(i+1)%2][(int)j/16][j%16]
		=adcc12C_al[(i+1)%2][j];
	      adc_ch[(i+1)%2][j/16][j%16]=j%16;
	    }
	  }
	}
	for(n=0;n<2;n++){
	  coinflug[i][n]=0;coinflug[(i+1)%2][n]=0;
	  for(j=0;j<16;j++){
	    for(k=j+1;k<16;k++){
	      if(n==0){//x order
		if(adc12C2[i][n][j]<adc12C2[i][n][k]){
		  tmp=adc12C2[i][n][j];
		  num=adc_ch[i][n][j];
		  adc12C2[i][n][j]=adc12C2[i][n][k];
		  adc_ch[i][n][j]=adc_ch[i][n][k];
		  adc12C2[i][n][k]=tmp;
		  adc_ch[i][n][k]=num;
		}
		if(adc2[(i+1)%2][n][j]<adc2[(i+1)%2][n][k]){
		  tmp=adc2[(i+1)%2][n][j];
		  num=adc_ch[(i+1)%2][n][j];
		  adc2[(i+1)%2][n][j]=adc2[(i+1)%2][n][k];
		  adc_ch[(i+1)%2][n][j]=adc_ch[(i+1)%2][n][k];
		  adc2[(i+1)%2][n][k]=tmp;
		  adc_ch[(i+1)%2][n][k]=num;
		}
	      }
	      if(n==1){//y order
		 if(adc2[i][n][j]<adc2[i][n][k]){//MODi y3 order
		   tmp=adc2[i][n][j];num=adc_ch[i][n][j];
		   adc2[i][n][j]=adc2[i][n][k];
		   adc_ch[i][n][j]=adc_ch[i][n][k];
		   adc2[i][n][k]=tmp;adc_ch[i][n][k]=num;
		 }
		if(adc12C2[(i+1)%2][n][j]<adc12C2[(i+1)%2][n][k]){
		  tmp=adc12C2[(i+1)%2][n][j];
		  num=adc_ch[(i+1)%2][n][j];
		  adc12C2[(i+1)%2][n][j]=adc12C2[(i+1)%2][n][k];
		  adc_ch[(i+1)%2][n][j]=adc_ch[(i+1)%2][n][k];
		  adc12C2[(i+1)%2][n][k]=tmp;
		  adc_ch[(i+1)%2][n][k]=num;
		}
	      }
	    }
	  }
	}
	for(k=0;k<tdc12C_nn[i][0];k++){
	    hit_ch[i][p32[i][0]][0]=adc_ch[i][0][k];
	    hit_adc[i][p32[i][0]][0]=adc12C2[i][0][k];
	    p32[i][0]++;
	}
	for(k=0;k<tdc_nn[i][1];k++){
	    hit_ch[i][p32[i][1]][1]=adc_ch[i][1][k];
	    hit_adc[i][p32[i][1]][1]=adc2[i][1][k];
	    p32[i][1]++;
	}
	for(k=0;k<tdc_nn[(i+1)%2][0];k++){
	  hit_ch[(i+1)%2][p32[(i+1)%2][0]][0]
	    =adc_ch[(i+1)%2][0][k];
	  hit_adc[(i+1)%2][p32[(i+1)%2][0]][0]
	    =adc2[(i+1)%2][0][k];
	  p32[(i+1)%2][0]++;	  
	}
	for(k=0;k<tdc12C_nn[(i+1)%2][1];k++){
	  hit_ch[(i+1)%2][p32[(i+1)%2][1]][1]
	    =adc_ch[(i+1)%2][1][k];
	  hit_adc[(i+1)%2][p32[(i+1)%2][1]][1]
	    =adc12C2[(i+1)%2][1][k];
	  p32[(i+1)%2][1]++;
	}
	
	p2=tdc_nn[(i+1)%2][0];//p2=3

	/*divide MODi-x&MOD(i+1)%2-y -> 3 signals */
	  double diffadc[N_ADC_MOD][6]={};//adc difference for 3*3 solve
	  double mindiffadc[N_ADC_MOD]={100.};
	  int ix1[N_ADC_MOD],iy1[N_ADC_MOD];//1 particle not divided
	  int ix2[N_ADC_MOD][2],iy2[N_ADC_MOD][2];//will be devided 2 adc
	  //HERERE
	  for(k=0;k<2;k++){//two signals of x MOD[i]
	    for(j=0;j<p2;j++){//three signals of y
	      iy2[i][0]=(j-1)*(j-2)/2;//0->1,1,2->0
	      iy2[i][1]=((j-1)*(j-2)/2+j)%2+1;//0,1->2,2->1
	      diffadc[i][3*k+j]=fabs(hit_adc[i][k][0]
				-hit_adc[i][j][1])
		+2*fabs(hit_adc[i][(k+1)%2][0]
		      -hit_adc[i][iy2[i][0]][1]
			-hit_adc[i][iy2[i][1]][1]);
	    }
	  }
	  for(j=0;j<6;j++){//search min of x y adcc
	    if(diffadc[i][j]<mindiffadc[i]){
	      mindiffadc[i]=diffadc[i][j];
	      ix1[i]=(int)j/3;
	      ix2[i][0]=(ix1[i]+1)%2;
	      ix2[i][1]=2;
	    if(j<=2){
	      iy1[i]=j;
	      iy2[i][0]=(iy1[i]-1)*(iy1[i]-2)/2;//0->1,1,2->0
	      iy2[i][1]=((iy1[i]-1)*(iy1[i]-2)/2+iy1[i])%2+1;//0,1->2,2->1
	    }
	    else if(j>2){
	      iy1[i]=j-3;
	      iy2[i][0]=(iy1[i]-1)*(iy1[i]-2)/2;//0->1,1,2->0
	      iy2[i][1]=((iy1[i]-1)*(iy1[i]-2)/2+iy1[i])%2+1;//0,1->2,2->1
	    }
	  }
	}

	  for(k=0;k<2;k++){//two signals of y MOD(i+1)%2
	    for(j=0;j<p2;j++){
	      ix2[(i+1)%2][0]=(j-1)*(j-2)/2;//0->1,1,2->0
	      ix2[(i+1)%2][1]=((j-1)*(j-2)/2+j)%2+1;//0,1->2,2->1
	      diffadc[(i+1)%2][3*k+j]=fabs(hit_adc[(i+1)%2][k][1]
	      			  -hit_adc[(i+1)%2][j][0])
	      	+2*fabs(hit_adc[(i+1)%2][(k+1)%2][1]
			-hit_adc[(i+1)%2][ix2[(i+1)%2][0]][0]
			-hit_adc[(i+1)%2][ix2[(i+1)%2][1]][0]);
	    }
	  }
	  //  printf("......");
	  for(j=0;j<6;j++){//search min of x y adcc
	    if(diffadc[(i+1)%2][j]<mindiffadc[(i+1)%2]){
	      mindiffadc[(i+1)%2]=diffadc[(i+1)%2][j];
	      iy1[(i+1)%2]=(int)j/3;
	      iy2[(i+1)%2][0]=(iy1[(i+1)%2]+1)%2;
	      iy2[(i+1)%2][1]=2;
	      if(j<=2){
		ix1[(i+1)%2]=j;
		ix2[(i+1)%2][0]=(ix1[(i+1)%2]-1)*(ix1[(i+1)%2]-2)/2;//0->1,1,2->0
		ix2[(i+1)%2][1]=((ix1[(i+1)%2]-1)*(ix1[(i+1)%2]-2)/2+ix1[(i+1)%2])%2+1;//0,1->2,2->1
	      }
	      else if(j>2){
		ix1[(i+1)%2]=j-3;
		ix2[(i+1)%2][0]=(ix1[(i+1)%2]-1)*(ix1[(i+1)%2]-2)/2;//0->1,1,2->0
		ix2[(i+1)%2][1]=((ix1[(i+1)%2]-1)*(ix1[(i+1)%2]-2)/2+ix1[(i+1)%2])%2+1;//0,1->2,2->1
	      }
	    }
	  }

 	  HF2(50310+2*i+i,
	      hit_adc[i][ix2[i][0]][0],
	      hit_adc[i][iy2[i][0]][1]+hit_adc[i][iy2[i][1]][1],1.0);
	   HF2(50310+2*i+(i+1)%2,
	      hit_adc[(i+1)%2][ix2[(i+1)%2][0]][0]
	       +hit_adc[(i+1)%2][ix2[(i+1)%2][1]][0],
	      hit_adc[(i+1)%2][iy2[(i+1)%2][0]][1],1.0);
	 
	   int flugdevide[N_ADC_MOD]={0}; //whether divide correctly or not
	   double finaldiff[N_ADC_MOD]={10.};
	   finaldiff[i] = fabs(hit_adc[i][iy2[i][0]][1]
			       +hit_adc[i][iy2[i][1]][1]
			       -hit_adc[i][ix2[i][0]][0]);
	   finaldiff[(i+1)%2] = fabs(hit_adc[(i+1)%2][ix2[(i+1)%2][0]][0]
				     +hit_adc[(i+1)%2][ix2[(i+1)%2][1]][0]
				     -hit_adc[(i+1)%2][iy2[(i+1)%2][0]][1]);
	   for(j=0;j<N_ADC_MOD;j++){
	     if(finaldiff[j]<=diff32result){
	       flugdevide[j]=1;
	     }
	   }
	   
	   //	   printf("%d%d,",flugdevide[0],flugdevide[1]);
	  
      	  if(flugdevide[i]*flugdevide[(i+1)%2]){
	    //divide x2 into 2signals
	    hit_ch[i][ix2[i][1]][0]=hit_ch[i][ix2[i][0]][0];
	    hit_adc[i][ix2[i][1]][0]
	      =hit_adc[i][ix2[i][0]][0]*hit_adc[i][iy2[i][1]][1]
	      /(hit_adc[i][iy2[i][0]][1]+hit_adc[i][iy2[i][1]][1]);
	    hit_adc[i][ix2[i][0]][0]
	      =hit_adc[i][ix2[i][0]][0]*hit_adc[i][iy2[i][0]][1]
	      /(hit_adc[i][iy2[i][0]][1]+hit_adc[i][iy2[i][1]][1]);
	    //MOD(i+1)%2 x reorder with adcc
	    for(k=0;k<p2;k++){
	      for(j=k+1;j<p2;j++){
		if(hit_adc[i][k][0]<hit_adc[i][j][0]){
		  tmp=hit_adc[i][k][0];
		  num=hit_ch[i][k][0];
		  hit_adc[i][k][0]=hit_adc[i][j][0];
		  hit_ch[i][k][0]=hit_ch[i][j][0];
		  hit_adc[i][j][0]=tmp;
		  hit_ch[i][j][0]=num;
		}
	      }
	    }
	    //divide y2 into 2signals
      	    hit_ch[(i+1)%2][iy2[(i+1)%2][1]][1]=hit_ch[(i+1)%2][iy2[(i+1)%2][0]][1];
      	    hit_adc[(i+1)%2][iy2[(i+1)%2][1]][1]
      	      =hit_adc[(i+1)%2][iy2[(i+1)%2][0]][1]*hit_adc[(i+1)%2][ix2[(i+1)%2][1]][0]
      	      /(hit_adc[(i+1)%2][ix2[(i+1)%2][0]][0]+hit_adc[(i+1)%2][ix2[(i+1)%2][1]][0]);
      	    hit_adc[(i+1)%2][iy2[(i+1)%2][0]][1]=hit_adc[(i+1)%2][iy2[(i+1)%2][0]][1]*hit_adc[(i+1)%2][ix2[(i+1)%2][0]][0]
      	      /(hit_adc[(i+1)%2][ix2[(i+1)%2][0]][0]+hit_adc[(i+1)%2][ix2[(i+1)%2][1]][0]);
      	    //MOD(i+1)%2 y reorder with adcc
      	    for(k=0;k<p2;k++){
      	      for(j=k+1;j<p2;j++){
      		if(hit_adc[(i+1)%2][k][1]<hit_adc[(i+1)%2][j][1]){
      		  tmp=hit_adc[(i+1)%2][k][1];
      		  num=hit_ch[(i+1)%2][k][1];
      		  hit_adc[(i+1)%2][k][1]=hit_adc[(i+1)%2][j][1];
      		  hit_ch[(i+1)%2][k][1]=hit_ch[(i+1)%2][j][1];
      		  hit_adc[(i+1)%2][j][1]=tmp;
      		  hit_ch[(i+1)%2][j][1]=num;
      		}
      	      }
      	    }
	    
      	    //3 particle order with tdcc
      	    for(n=0;n<N_ADC_MOD;n++){
      	      for(k=0;k<2;k++){//x & y
      		for(j=0;j<p2;j++){
      		  hitch_tdc_odr[n][j][k]=hit_ch[n][j][k];
      		}
      		for(j=0;j<p2;j++){
      		  for(jj=j+1;jj<p2;jj++){
      		    double nj,njj;
      		    nj=tdcc_al[n*N_ADC+k*16+hitch_tdc_odr[n][j][k]][0];
      		    njj=tdcc_al[n*N_ADC+k*16+hitch_tdc_odr[n][jj][k]][0];
      		    if(nj>njj){
      		      //	    printf("%d %d,",j,jj);
      		      itmp=hitch_tdc_odr[n][j][k];
      		      hitch_tdc_odr[n][j][k]=hitch_tdc_odr[n][jj][k];
      		      hitch_tdc_odr[n][jj][k]=itmp;
      		    }
      		  }
      		}
      		//3 alpha coincidence flug
      		if((tdcc_al[n*N_ADC+k*16+hitch_tdc_odr[n][1][k]][0]-tdcc_al[n*N_ADC+k*16+hitch_tdc_odr[n][0][k]][0])>=coinmin
      		   &&(tdcc_al[n*N_ADC+k*16+hitch_tdc_odr[n][2][k]][0]-tdcc_al[n*N_ADC+k*16+hitch_tdc_odr[n][0][k]][0])<coinmax){
      		  // divede y adc gate
      		  if(hit_adc[n][0][k]>adccmin&&hit_adc[n][0][k]<adccmax
      		     &&hit_adc[n][1][k]>adccmin&&hit_adc[n][1][k]<adccmax
      		     &&hit_adc[n][2][k]>adccmin&&hit_adc[n][2][k]<adccmax){
      		    coinflug[n][k]=1;
      		  }
      		}
      		for(j=1;j<p2;j++){
      		  HF1(3060+2*i+n,tdcc_al[n*N_ADC+k*16+hitch_tdc_odr[n][j][k]][0]-tdcc_al[n*N_ADC+k*16+hitch_tdc_odr[n][0][k]][0],1.0);
      		  if(FLUG[i][1]>=p2 && FLUG[(i+1)%2][0]>=p2){//tdc gate only 3 signal Si
      		    if(coinflug[n][k]){
      		      HF1(3070+2*i+n,tdcc_al[n*N_ADC+k*16+hitch_tdc_odr[n][j][k]][0]-tdcc_al[n*N_ADC+k*16+hitch_tdc_odr[n][0][k]][0],1.0);
      		    }
      		  }
      		}
      	      }
      	    }
	    printf("%d%d%d%d,",coinflug[0][0],coinflug[0][1],coinflug[1][0],coinflug[1][1]);	    
      	    /**!!!!invariant mass of 12C**/
      	    for(n=0;n<N_ADC_MOD;n++){
      	      //  printf("test8");
      	      for(j=0;j<p2;j++){
      		ichx=hit_ch[n][j][0];
      		ichy=hit_ch[n][j][1];
      		ene0=hit_adc[n][j][0];
      		ene1=hit_adc[n][j][1];
		
      		if(FLUG[i][1]>=p2 && FLUG[(i+1)%2][0]>=p2){
      		  if(coinflug[0][0]*coinflug[0][1]){//3 alpha coin x y right
      		    if(coinflug[1][0]*coinflug[1][1]){//6alpha coin x y
      		      HF2(10070+p2*i,20*(1-n)+ichx,ichy,1.0);
      		    }
      		  }
      		}
		if(n==i){
		  vec[n][j][0]=ene1+mhe;
		}
		if(n==(i+1)%2){
		  vec[n][j][0]=ene0+mhe;
		}
		
      		rvec[0]=((double)ichx-7.5)*3.0;
      		rvec[1]=(7.5-(double)ichy)*3.0;
      		rvec[2]=l;
		
      		unitvec(rvec,rvec);
      		rotvec(rvec,&vec[n][j][1],1,pow(-1,n)*theta);
		
      		phe=sqrt(vec[n][j][0]*vec[n][j][0]-mhe*mhe);
      		vecadd(&vec[n][j][1],rvec,&vec[n][j][1],phe,0.0);
      		vecadd4(vec[n][j],vec12c[n],vec12c[n],1,1);
      	      }
      	      ex12c[n]=sqrt(scapro4(vec12c[n],vec12c[n]))-m12c;
      	      HF1(50080+2*i+n,ex12c[n],1.0);
      	      if(FLUG[i][1]>=p2 && FLUG[(i+1)%2][0]>=p2){//tdc gate
      		if(coinflug[n][0]*coinflug[n][1]){//3 alpha coin x y
      		  if(coinflug[(n+1)%2][0]*coinflug[(n+1)%2][1]){
      		    HF1(50090+2*i+n,ex12c[n],1.0);
      		  }
      		}
      	      }
      	    }
      	    // HF2(50042,ex12c[0],ex12c[1],1.0);
      	    if(FLUG[i][1]>=p2 && FLUG[(i+1)%2][0]>=p2){
      	      if(coinflug[0][0]*coinflug[0][1]){//3 alpha coin x y right
      		if(coinflug[1][0]*coinflug[1][1]){//6alpha coin x y
      		  HF2(50095+i,ex12c[0],ex12c[1],1.0);
      		  HF2(50097+i,ex12c[0],ex12c[1],1.0);
      		}
      	      }
      	    }
      	  }
      }
    }
    
  }
  /********R=32 L=32***************************/
    /*clear hit_ch*/
  for(i=0;i<N_ADC_MOD;i++){
    for(j=0;j<2;j++){
      for(k=0;k<16;k++){
	adc2[i][j][k]=0.;
	adc12C2[i][j][k]=0.;
	adc_ch[i][j][k]=-1;
	hit_ch[i][k][j]=-1;
	hitch_tdc_odr[i][k][j]=-1;
	hit_adc[i][k][j]=0.;
      }
    }
    for(j=0;j<2;j++){
      p32[i][j]=0;
    }
  }
  
  if(tdc_nn[0][0]==3&&tdc12C_nn[0][1]==2){//both x=3,y=2
    if(tdc_nn[1][0]==3&&tdc12C_nn[1][1]==2){//x=3,y=2
      for(i=0;i<N_ADC_MOD;i++){
	for(j=0;j<N_ADC;j++){
	  if(j<N_ADC/2){
	    if(tdc_cnt_al[32*i+j]>0){
	      adc2[i][(int)j/16][j%16]
		=adcc_al[i][j];
	      adc_ch[i][j/16][j%16]=j%16;
	    }
	  }
	  if(j>=N_ADC/2){
	    if(tdc12C_cnt_al[32*i+j]>0){
	      adc12C2[i][(int)j/16][j%16]
		=adcc12C_al[i][j];
	      adc_ch[i][j/16][j%16]=j%16;
	    }
	  }
	}
      }
      for(i=0;i<N_ADC_MOD;i++){
	for(n=0;n<2;n++){
	  coinflug[i][n]=0;
	  for(j=0;j<16;j++){
	    for(k=j+1;k<16;k++){
	      if(n==0){//x order
		if(adc2[i][n][j]<adc2[i][n][k]){
		  tmp=adc2[i][n][j];
		  num=adc_ch[i][n][j];
		  adc2[i][n][j]=adc2[i][n][k];
		  adc_ch[i][n][j]=adc_ch[i][n][k];
		  adc2[i][n][k]=tmp;
		  adc_ch[i][n][k]=num;
		}
	      }
	      if(n==1){//y order
		if(adc12C2[i][n][j]<adc12C2[i][n][k]){
		  tmp=adc12C2[i][n][j];
		  num=adc_ch[i][n][j];
		  adc12C2[i][n][j]=adc12C2[i][n][k];
		  adc_ch[i][n][j]=adc_ch[i][n][k];
		  adc12C2[i][n][k]=tmp;
		  adc_ch[i][n][k]=num;
		}
	      }
	    }
	  }
	}
	for(k=0;k<tdc_nn[i][0];k++){
	    hit_ch[i][p32[i][0]][0]=adc_ch[i][0][k];
	    hit_adc[i][p32[i][0]][0]=adc2[i][0][k];
	    p32[i][0]++;
	}
	for(k=0;k<tdc12C_nn[i][1];k++){
	    hit_ch[i][p32[i][1]][1]=adc_ch[i][1][k];
	    hit_adc[i][p32[i][1]][1]=adc12C2[i][1][k];
	    p32[i][1]++;
	}
      }
	
      p2=tdc_nn[0][0];//p2=3

      /*divide MODi-x&MOD(i+1)%2-y -> 3 signals */
      double diffadc[N_ADC_MOD][6]={};//adc difference for 3*3 solve
      double mindiffadc[N_ADC_MOD]={100.};
      int ix1[N_ADC_MOD],iy1[N_ADC_MOD];//1 particle not divided
      int ix2[N_ADC_MOD][2],iy2[N_ADC_MOD][2];//will be devided 2 adc

      for(i=0;i<N_ADC_MOD;i++){
	for(k=0;k<2;k++){//two signals of y 
	  for(j=0;j<p2;j++){//three signals of x
	    ix2[i][0]=(j-1)*(j-2)/2;//0->1,1,2->0
	    ix2[i][1]=((j-1)*(j-2)/2+j)%2+1;//0,1->2,2->1
	    diffadc[i][3*k+j]=fabs(hit_adc[i][k][1]
					 -hit_adc[i][j][0])
	      +2*fabs(hit_adc[i][(k+1)%2][1]
		      -hit_adc[i][ix2[i][0]][0]
		      -hit_adc[i][ix2[i][1]][0]);
	  }
	}
	//  printf("......");
	for(j=0;j<6;j++){//search min of x y adcc
	  if(diffadc[i][j]<mindiffadc[i]){
	    mindiffadc[i]=diffadc[i][j];
	    iy1[i]=(int)j/3;
	    iy2[i][0]=(iy1[i]+1)%2;
	    iy2[i][1]=2;
	    if(j<=2){
	      ix1[i]=j;
	      ix2[i][0]=(ix1[i]-1)*(ix1[i]-2)/2;//0->1,1,2->0
	      ix2[i][1]=((ix1[i]-1)*(ix1[i]-2)/2+ix1[i])%2+1;//0,1->2,2->1
	    }
	    else if(j>2){
	      ix1[i]=j-3;
	      ix2[i][0]=(ix1[i]-1)*(ix1[i]-2)/2;//0->1,1,2->0
	      ix2[i][1]=((ix1[i]-1)*(ix1[i]-2)/2+ix1[i])%2+1;//0,1->2,2->1
	    }
	  }
	}

	HF2(50320+i,
	    hit_adc[i][ix2[i][0]][0]+hit_adc[i][ix2[i][1]][0],
	    hit_adc[i][iy2[i][0]][1],1.0);
      }
	 
      int flugdevide[N_ADC_MOD]={0}; //whether divide correctly or not
      double finaldiff[N_ADC_MOD]={10.};
      for(i=0;i<N_ADC_MOD;i++){
	finaldiff[i] = fabs(hit_adc[i][ix2[i][0]][0]
			    +hit_adc[i][ix2[i][1]][0]
				  -hit_adc[i][iy2[i][0]][1]);
	if(finaldiff[i]<=diff32result){
	  flugdevide[i]=1;
	}
      }

      if(flugdevide[0]*flugdevide[1]){
	for(i=0;i<N_ADC_MOD;i++){
	  //divide y2 into 2signals
	  hit_ch[i][iy2[i][1]][1]=hit_ch[i][iy2[i][0]][1];
	  hit_adc[i][iy2[i][1]][1]
	    =hit_adc[i][iy2[i][0]][1]*hit_adc[i][ix2[i][1]][0]
	    /(hit_adc[i][ix2[i][0]][0]+hit_adc[i][ix2[i][1]][0]);
	  hit_adc[i][iy2[i][0]][1]
	    =hit_adc[i][iy2[i][0]][1]*hit_adc[i][ix2[i][0]][0]
	    /(hit_adc[i][ix2[i][0]][0]+hit_adc[i][ix2[i][1]][0]);
	//MOD(i+1)%2 y reorder with adcc
	  for(k=0;k<p2;k++){
	    for(j=k+1;j<p2;j++){
	      if(hit_adc[i][k][1]<hit_adc[i][j][1]){
		tmp=hit_adc[i][k][1];
		num=hit_ch[i][k][1];
		hit_adc[i][k][1]=hit_adc[i][j][1];
		hit_ch[i][k][1]=hit_ch[i][j][1];
		hit_adc[i][j][1]=tmp;
		hit_ch[i][j][1]=num;
	      }
	    }
	  }
  
	  //3 particle order with tdcc
	  for(k=0;k<2;k++){//x & y
	    for(j=0;j<p2;j++){
	      hitch_tdc_odr[i][j][k]=hit_ch[i][j][k];
	    }
	    for(j=0;j<p2;j++){
	      for(jj=j+1;jj<p2;jj++){
		double nj,njj;
		nj=tdcc_al[i*N_ADC+k*16+hitch_tdc_odr[i][j][k]][0];
		njj=tdcc_al[i*N_ADC+k*16+hitch_tdc_odr[i][jj][k]][0];
		if(nj>njj){
		  //	    printf("%d %d,",j,jj);
		  itmp=hitch_tdc_odr[i][j][k];
		  hitch_tdc_odr[i][j][k]=hitch_tdc_odr[i][jj][k];
		  hitch_tdc_odr[i][jj][k]=itmp;
		}
	      }
	    }
	    //3 alpha coincidence flug
	    if((tdcc_al[i*N_ADC+k*16+hitch_tdc_odr[i][1][k]][0]-tdcc_al[i*N_ADC+k*16+hitch_tdc_odr[i][0][k]][0])>=coinmin
	       &&(tdcc_al[i*N_ADC+k*16+hitch_tdc_odr[i][2][k]][0]-tdcc_al[i*N_ADC+k*16+hitch_tdc_odr[i][0][k]][0])<coinmax){
	      // divede y adc gate
	      if(hit_adc[i][0][k]>adccmin&&hit_adc[i][0][k]<adccmax
		 &&hit_adc[i][1][k]>adccmin&&hit_adc[i][1][k]<adccmax
		 &&hit_adc[i][2][k]>adccmin&&hit_adc[i][2][k]<adccmax){
		coinflug[i][k]=1;
	      }
	    }
	    for(j=1;j<p2;j++){
	      HF1(3080+i,tdcc_al[i*N_ADC+k*16+hitch_tdc_odr[i][j][k]][0]-tdcc_al[i*N_ADC+k*16+hitch_tdc_odr[i][0][k]][0],1.0);
	      if(FLUG[i][0]>=p2 && FLUG[(i+1)%2][0]>=p2){//tdc gate only 3 signal Si
		if(coinflug[i][k]){
		  HF1(3090+i,tdcc_al[i*N_ADC+k*16+hitch_tdc_odr[i][j][k]][0]-tdcc_al[i*N_ADC+k*16+hitch_tdc_odr[i][0][k]][0],1.0);
		}
	      }
	    }
	  }
	}
	/**!!!!invariant mass of 12C**/
	for(i=0;i<N_ADC_MOD;i++){
	  //  printf("test8");
	  for(j=0;j<p2;j++){
	    ichx=hit_ch[i][j][0];
	    ichy=hit_ch[i][j][1];
	    ene0=hit_adc[i][j][0];
	    ene1=hit_adc[i][j][1];
	    
	    if(FLUG[0][0]>=p2 && FLUG[1][0]>=p2){
	      if(coinflug[0][0]*coinflug[0][1]){//3 alpha coin x y right
		if(coinflug[1][0]*coinflug[1][1]){//6alpha coin x y
		  HF2(10080,20*(1-i)+ichx,ichy,1.0);
		}
	      }
	    }
	   
	    vec[i][j][0]=ene0+mhe;
	 		
	    rvec[0]=((double)ichx-7.5)*3.0;
	    rvec[1]=(7.5-(double)ichy)*3.0;
	    rvec[2]=l;
	    
	    unitvec(rvec,rvec);
	    rotvec(rvec,&vec[i][j][1],1,pow(-1,n)*theta);
	    
	    phe=sqrt(vec[i][j][0]*vec[i][j][0]-mhe*mhe);
	    vecadd(&vec[i][j][1],rvec,&vec[i][j][1],phe,0.0);
	    vecadd4(vec[i][j],vec12c[i],vec12c[i],1,1);
	  }
	  ex12c[i]=sqrt(scapro4(vec12c[i],vec12c[i]))-m12c;
	  HF1(50100+i,ex12c[i],1.0);
	  if(FLUG[i][0]>=p2 && FLUG[(i+1)%2][0]>=p2){//tdc gate
	    if(coinflug[i][0]*coinflug[i][1]){//3 alpha coin x y
	      if(coinflug[(i+1)%2][0]*coinflug[(i+1)%2][1]){
		HF1(50110+i,ex12c[i],1.0);
	      }
	    }
	  }
	}
	
	if(FLUG[0][0]>=p2 && FLUG[1][0]>=p2){
	  if(coinflug[0][0]*coinflug[0][1]){//3 alpha coin x y right
	    if(coinflug[1][0]*coinflug[1][1]){//6alpha coin x y
	      HF2(50115,ex12c[0],ex12c[1],1.0);
	      HF2(50116,ex12c[0],ex12c[1],1.0);
	    }
	  }
	}
      }
    }
  }


  /**********   R=23, L=23   *********************/    
   /*clear hit_ch*/
  for(i=0;i<N_ADC_MOD;i++){
    for(j=0;j<2;j++){
      for(k=0;k<16;k++){
	adc2[i][j][k]=0.;
	adc12C2[i][j][k]=0.;
	adc_ch[i][j][k]=-1;
	hit_ch[i][k][j]=-1;
	hitch_tdc_odr[i][k][j]=-1;
	hit_adc[i][k][j]=0.;
      }
    }
    for(j=0;j<2;j++){
      p32[i][j]=0;
    }
  }
 
  if(tdc12C_nn[0][0]==2&&tdc_nn[0][1]==3){//both x=2,y=3
    if(tdc12C_nn[1][0]==2&&tdc_nn[1][1]==3){
      for(i=0;i<N_ADC_MOD;i++){
	for(j=0;j<N_ADC;j++){
	  if(j<N_ADC/2){
	    if(tdc12C_cnt_al[32*i+j]>0){ //MOD i x2
	      adc12C2[i][j/16][j%16]=adcc12C_al[i][j];
	      adc_ch[i][j/16][j%16]=j%16;
	    }
	  }
	  if(j>=N_ADC/2){
	    if(tdc_cnt_al[32*i+j]>0){
	      adc2[i][(int)j/16][j%16]
		=adcc_al[i][j];
	      adc_ch[i][j/16][j%16]=j%16;
	    }
	  }
	}
      }
      for(i=0;i<N_ADC_MOD;i++){
	for(n=0;n<2;n++){
	  coinflug[i][n]=0;
	  for(j=0;j<16;j++){
	    for(k=j+1;k<16;k++){
	      if(n==0){//x order
		if(adc12C2[i][n][j]<adc12C2[i][n][k]){
		  tmp=adc12C2[i][n][j];
		  num=adc_ch[i][n][j];
		  adc12C2[i][n][j]=adc12C2[i][n][k];
		  adc_ch[i][n][j]=adc_ch[i][n][k];
		  adc12C2[i][n][k]=tmp;
		  adc_ch[i][n][k]=num;
		}
	      }
	      if(n==1){//y order
		if(adc2[i][n][j]<adc2[i][n][k]){//MODi y3 order
		  tmp=adc2[i][n][j];num=adc_ch[i][n][j];
		  adc2[i][n][j]=adc2[i][n][k];
		  adc_ch[i][n][j]=adc_ch[i][n][k];
		  adc2[i][n][k]=tmp;adc_ch[i][n][k]=num;
		}
	      }
	    }
	  }
	}
	for(k=0;k<tdc12C_nn[i][0];k++){
	    hit_ch[i][p32[i][0]][0]=adc_ch[i][0][k];
	    hit_adc[i][p32[i][0]][0]=adc12C2[i][0][k];
	    p32[i][0]++;
	}
	for(k=0;k<tdc_nn[i][1];k++){
	    hit_ch[i][p32[i][1]][1]=adc_ch[i][1][k];
	    hit_adc[i][p32[i][1]][1]=adc2[i][1][k];
	    p32[i][1]++;
	}
      }
	
      p2=tdc_nn[0][1];//p2=3

      /*divide MODi-x&MOD(i+1)%2-y -> 3 signals */
      double diffadc[N_ADC_MOD][6]={};//adc difference for 3*3 solve
      double mindiffadc[N_ADC_MOD]={100.};
      int ix1[N_ADC_MOD],iy1[N_ADC_MOD];//1 particle not divided
      int ix2[N_ADC_MOD][2],iy2[N_ADC_MOD][2];//will be devided 2 adc

      for(i=0;i<N_ADC_MOD;i++){
	for(k=0;k<2;k++){//two signals of x MOD[i]
	  for(j=0;j<p2;j++){//three signals of y
	    iy2[i][0]=(j-1)*(j-2)/2;//0->1,1,2->0
	    iy2[i][1]=((j-1)*(j-2)/2+j)%2+1;//0,1->2,2->1
	    diffadc[i][3*k+j]=fabs(hit_adc[i][k][0]
				   -hit_adc[i][j][1])
	      +2*fabs(hit_adc[i][(k+1)%2][0]
		      -hit_adc[i][iy2[i][0]][1]
			-hit_adc[i][iy2[i][1]][1]);
	  }
	}
	for(j=0;j<6;j++){//search min of x y adcc
	  if(diffadc[i][j]<mindiffadc[i]){
	    mindiffadc[i]=diffadc[i][j];
	    ix1[i]=(int)j/3;
	    ix2[i][0]=(ix1[i]+1)%2;
	    ix2[i][1]=2;
	    if(j<=2){
	      iy1[i]=j;
	      iy2[i][0]=(iy1[i]-1)*(iy1[i]-2)/2;//0->1,1,2->0
	      iy2[i][1]=((iy1[i]-1)*(iy1[i]-2)/2+iy1[i])%2+1;//0,1->2,2->1
	    }
	    else if(j>2){
	      iy1[i]=j-3;
	      iy2[i][0]=(iy1[i]-1)*(iy1[i]-2)/2;//0->1,1,2->0
	      iy2[i][1]=((iy1[i]-1)*(iy1[i]-2)/2+iy1[i])%2+1;//0,1->2,2->1
	    }
	  }
	}

	HF2(50322+i,
	    hit_adc[i][ix2[i][0]][0],
	    hit_adc[i][iy2[i][0]][1]+hit_adc[i][iy2[i][1]][1],1.0);
      }
      
      int flugdevide[N_ADC_MOD]={0}; //whether divide correctly or not
      double finaldiff[N_ADC_MOD]={10.};
      for(i=0;i<N_ADC_MOD;i++){
	finaldiff[i] = fabs(hit_adc[i][iy2[i][0]][1]
			    +hit_adc[i][iy2[i][1]][1]
			    -hit_adc[i][ix2[i][0]][0]);
	if(finaldiff[i]<=diff32result){
	       flugdevide[i]=1;
	}
      }
      
      if(flugdevide[0]*flugdevide[1]){
	for(i=0;i<N_ADC_MOD;i++){
	  //divide x2 into 2signals
	  hit_ch[i][ix2[i][1]][0]=hit_ch[i][ix2[i][0]][0];
	  hit_adc[i][ix2[i][1]][0]
	    =hit_adc[i][ix2[i][0]][0]*hit_adc[i][iy2[i][1]][1]
	    /(hit_adc[i][iy2[i][0]][1]+hit_adc[i][iy2[i][1]][1]);
	  hit_adc[i][ix2[i][0]][0]
	    =hit_adc[i][ix2[i][0]][0]*hit_adc[i][iy2[i][0]][1]
	    /(hit_adc[i][iy2[i][0]][1]+hit_adc[i][iy2[i][1]][1]);
	  //MOD(i+1)%2 x reorder with adcc
	  for(k=0;k<p2;k++){
	    for(j=k+1;j<p2;j++){
	      if(hit_adc[i][k][0]<hit_adc[i][j][0]){
		tmp=hit_adc[i][k][0];
		num=hit_ch[i][k][0];
		hit_adc[i][k][0]=hit_adc[i][j][0];
		hit_ch[i][k][0]=hit_ch[i][j][0];
		hit_adc[i][j][0]=tmp;
		hit_ch[i][j][0]=num;
	      }
	    }
	  }
	  //3 particle order with tdcc
	  for(k=0;k<2;k++){//x & y
	    for(j=0;j<p2;j++){
	      hitch_tdc_odr[i][j][k]=hit_ch[i][j][k];
	    }
	    for(j=0;j<p2;j++){
	      for(jj=j+1;jj<p2;jj++){
		double nj,njj;
		nj=tdcc_al[i*N_ADC+k*16+hitch_tdc_odr[i][j][k]][0];
		njj=tdcc_al[i*N_ADC+k*16+hitch_tdc_odr[i][jj][k]][0];
		if(nj>njj){
		  //	    printf("%d %d,",j,jj);
		  itmp=hitch_tdc_odr[i][j][k];
		  hitch_tdc_odr[i][j][k]=hitch_tdc_odr[i][jj][k];
		  hitch_tdc_odr[i][jj][k]=itmp;
		}
	      }
	    }
	    //3 alpha coincidence flug
	    if((tdcc_al[i*N_ADC+k*16+hitch_tdc_odr[i][1][k]][0]-tdcc_al[i*N_ADC+k*16+hitch_tdc_odr[i][0][k]][0])>=coinmin
	       &&(tdcc_al[i*N_ADC+k*16+hitch_tdc_odr[i][2][k]][0]-tdcc_al[i*N_ADC+k*16+hitch_tdc_odr[i][0][k]][0])<coinmax){
	      // divede y adc gate
	      if(hit_adc[i][0][k]>adccmin&&hit_adc[i][0][k]<adccmax
		 &&hit_adc[i][1][k]>adccmin&&hit_adc[i][1][k]<adccmax
		 &&hit_adc[i][2][k]>adccmin&&hit_adc[i][2][k]<adccmax){
		coinflug[i][k]=1;
	      }
	    }
	    for(j=1;j<p2;j++){
	      HF1(3080+2+i,tdcc_al[i*N_ADC+k*16+hitch_tdc_odr[i][j][k]][0]-tdcc_al[i*N_ADC+k*16+hitch_tdc_odr[i][0][k]][0],1.0);
	      if(FLUG[i][1]>=p2 && FLUG[(i+1)%2][1]>=p2){//tdc gate only 3 signal Si
		if(coinflug[i][k]){
		  HF1(3090+2+i,tdcc_al[i*N_ADC+k*16+hitch_tdc_odr[i][j][k]][0]-tdcc_al[i*N_ADC+k*16+hitch_tdc_odr[i][0][k]][0],1.0);
		}
	      }
	    }
	  }
	}
	
	/**!!!!invariant mass of 12C**/
	for(i=0;i<N_ADC_MOD;i++){
	  for(j=0;j<p2;j++){
	    ichx=hit_ch[i][j][0];
	    ichy=hit_ch[i][j][1];
	    ene0=hit_adc[i][j][0];
	    ene1=hit_adc[i][j][1];
	    
	    if(FLUG[0][1]>=p2 && FLUG[1][1]>=p2){
	      if(coinflug[0][0]*coinflug[0][1]){//3 alpha coin x y right
		if(coinflug[1][0]*coinflug[1][1]){//6alpha coin x y
		  HF2(10083,20*(1-1)+ichx,ichy,1.0);
		}
	      }
	    }

	    vec[i][j][0]=ene1+mhe;
		
	    rvec[0]=((double)ichx-7.5)*3.0;
	    rvec[1]=(7.5-(double)ichy)*3.0;
	    rvec[2]=l;
	    
	    unitvec(rvec,rvec);
	    rotvec(rvec,&vec[i][j][1],1,pow(-1,n)*theta);
	    
	    phe=sqrt(vec[i][j][0]*vec[i][j][0]-mhe*mhe);
	    vecadd(&vec[i][j][1],rvec,&vec[i][j][1],phe,0.0);
	    vecadd4(vec[i][j],vec12c[i],vec12c[i],1,1);
	  }
	  ex12c[i]=sqrt(scapro4(vec12c[i],vec12c[i]))-m12c;
	  HF1(50102+i,ex12c[i],1.0);
	  if(FLUG[i][1]>=p2 && FLUG[(i+1)%2][1]>=p2){//tdc gate
	    if(coinflug[i][0]*coinflug[i][1]){//3 alpha coin x y
	      if(coinflug[(i+1)%2][0]*coinflug[(i+1)%2][1]){
		HF1(50112+i,ex12c[i],1.0);
	      }
	    }
	  }
	}
	if(FLUG[0][1]>=p2 && FLUG[1][1]>=p2){
	  if(coinflug[0][0]*coinflug[0][1]){//3 alpha coin x y right
	    if(coinflug[1][0]*coinflug[1][1]){//6alpha coin x y
	      HF2(50117,ex12c[0],ex12c[1],1.0);
	      HF2(50118,ex12c[0],ex12c[1],1.0);
	    }
	  }
	}
      }
    }
  }


  /* /\**12C(0+1)+3a event**\/ */
  
  /* //i=0->R:3alpha,L:12C  i=1->R:12C,L=3alpha */
  /* for(i=0;i<N_ADC_MOD;i++){ */
  /*   /\*clear hit_ch*\/ */
  /*   for(j=0;j<2;j++){ */
  /*     for(k=0;k<16;k++){ */
  /* 	adc2[i][j][k]=0.; */
  /* 	adc12C2[i][j][k]=0.; */
  /* 	adc_ch[i][j][k]=-1; */
  /*     	hit_ch[i][k][j]=-1; */
  /* 	hitch_tdc_odr[i][k][j]=-1; */
  /*     	hit_adc[i][k][j]=0.; */
  /*     } */
  /*     ene_sum3a[i][j]=0.; */
  /*   } */
  /*   pn[0]=0;pn[1]=0; */

  /*   if(tdc_nn[i][0]==3&&tdc_nn[i][1]==3){ */
  /*     if(tdc12C_nn[(i+1)%2][0]==2&&tdc12C_nn[(i+1)%2][1]==2){ */
  /* 	for(j=0;j<N_ADC;j++){ */
  /* 	  if(tdc_cnt_al[32*i+j]>0){ // select ch */
  /* 	    adc2[i][j/16][j%16]=adcc_al[i][j]; */
  /* 	    adc_ch[i][j/16][j%16]=j%16; */
  /* 	  } */
  /* 	  if(tdc12C_cnt_al[32*((i+1)%2)+j]>0){ */
  /* 	    adc12C2[(i+1)%2][(int)j/16][j%16]=adcc12C_al[(i+1)%2][j]; */
  /* 	    adc_ch[(i+1)%2][j/16][j%16]=j%16; */
  /* 	  } */
  /* 	} */
  /* 	for(n=0;n<2;n++){ */
  /* 	  coinflug[i][n]=0; */
  /* 	  for(j=0;j<16;j++){ */
  /* 	    for(k=j+1;k<16;k++){ */
  /* 	      if(adc2[i][n][j]<adc2[i][n][k]){ */
  /* 		tmp=adc2[i][n][j];num=adc_ch[i][n][j]; */
  /* 		adc2[i][n][j]=adc2[i][n][k]; */
  /* 		adc_ch[i][n][j]=adc_ch[i][n][k]; */
  /* 		adc2[i][n][k]=tmp;adc_ch[i][n][k]=num; */
  /* 	      } */
  /* 	      if(adc12C2[(i+1)%2][n][j]<adc12C2[(i+1)%2][n][k]){ */
  /* 		tmp=adc12C2[(i+1)%2][n][j];num=adc_ch[(i+1)%2][n][j]; */
  /* 		adc12C2[(i+1)%2][n][j]=adc12C2[(i+1)%2][n][k]; */
  /* 		adc_ch[(i+1)%2][n][j]=adc_ch[(i+1)%2][n][k]; */
  /* 		adc12C2[(i+1)%2][n][k]=tmp;adc_ch[(i+1)%2][n][k]=num; */
  /* 	      } */
  /* 	    } */
  /* 	  } */
  /* 	} */
  /* 	for(k=0;k<tdc_nn[i][0];k++){ */
  /* 	  hit_ch[i][pn[i]][0]=adc_ch[i][0][k]; */
  /* 	  hit_ch[i][pn[i]][1]=adc_ch[i][1][k]; */
  /* 	  hit_adc[i][pn[i]][0]=adc2[i][0][k]; */
  /* 	  hit_adc[i][pn[i]][1]=adc2[i][1][k]; */
  /* 	  pn[i]++; */
  /* 	} */
  /* 	for(k=0;k<tdc12C_nn[(i+1)%2][0];k++){ */
  /* 	  hit_ch[(i+1)%2][pn[(i+1)%2]][0]=adc_ch[(i+1)%2][0][k]; */
  /* 	  hit_ch[(i+1)%2][pn[(i+1)%2]][1]=adc_ch[(i+1)%2][1][k]; */
  /* 	  hit_adc[(i+1)%2][pn[(i+1)%2]][0]=adc12C2[(i+1)%2][0][k]; */
  /* 	  hit_adc[(i+1)%2][pn[(i+1)%2]][1]=adc12C2[(i+1)%2][1][k]; */
  /* 	  pn[(i+1)%2]++; */
  /* 	} */
  /* 	ene_sum3a[i][0]=hit_adc[i][0][0]+hit_adc[i][1][0]+hit_adc[i][2][0];//x */
  /* 	ene_sum3a[i][1]=hit_adc[i][0][1]+hit_adc[i][1][1]+hit_adc[i][2][1];//y */
  /* 	HF2(4000+2*i,ene_sum3a[i][0],hit_adc[(i+1)%2][0][0],1.0); */
  /* 	HF2(4000+2*i+1,ene_sum3a[i][1],hit_adc[(i+1)%2][0][1],1.0); */

  /* 	for(k=0;k<2;k++){//x & y */
  /* 	  for(j=0;j<3;j++){//3alpha */
  /* 	    hitch_tdc_odr[i][j][k]=hit_ch[i][j][k]; */
  /* 	  } */
  /* 	  for(j=0;j<p2;j++){ */
  /* 	    for(jj=j+1;jj<p2;jj++){ */
  /* 	      double nj,njj; */
  /* 	      nj=tdcc_al[i*N_ADC+k*16+hitch_tdc_odr[i][j][k]][0]; */
  /* 	      njj=tdcc_al[i*N_ADC+k*16+hitch_tdc_odr[i][jj][k]][0]; */
  /* 	      if(nj>njj){ */
  /* 		//	    printf("%d %d,",j,jj); */
  /* 		itmp=hitch_tdc_odr[i][j][k]; */
  /* 		hitch_tdc_odr[i][j][k]=hitch_tdc_odr[i][jj][k]; */
  /* 		hitch_tdc_odr[i][jj][k]=itmp; */
  /* 	      } */
  /* 	    } */
  /* 	  } */
	      
  /* 	  //3 alpha coincidence flug only i */
  /* 	  if((tdcc_al[i*N_ADC+k*16+hitch_tdc_odr[i][1][k]][0]-tdcc_al[i*N_ADC+k*16+hitch_tdc_odr[i][0][k]][0])>=coinmin */
  /* 	     &&(tdcc_al[i*N_ADC+k*16+hitch_tdc_odr[i][2][k]][0]-tdcc_al[i*N_ADC+k*16+hitch_tdc_odr[i][0][k]][0])<coinmax){ */
  /* 	    coinflug[i][k]=1; */
  /* 	  } */
	  
  /* 	//12C coin with timelug btwn 12C and first a */
  /* 	  if(fabs(tdcc12C_al[((i+1)%2)*N_ADC+k*16+hit_ch[(i+1)%2][0][k]][0] */
  /* 		  -tdcc_al[i*N_ADC+k*16+hitch_tdc_odr[i][0][k]][0]) */
  /* 	     <50.){ */
  /* 	    coinflug[(i+1)%2][k]=1; */
  /* 	  } */
  /* 	  HF1(4010+i,tdcc12C_al[((i+1)%2)*N_ADC+k*16+hit_ch[(i+1)%2][0][k]][0] */
  /* 	      -tdcc_al[i*N_ADC+k*16+hitch_tdc_odr[i][0][k]][0],1.0); */
  /* 	} */
  /* 	//excited energy of 12C!!! */
  /* 	//the easiest cal */
  /* 	const double Ex3a=7.65; */
  /* 	const double Ebeam=56.7; */
  /* 	const double u12C=931.478;//MeV */
  /* 	const double Mg24=23.9850417;//unit u12C */

  /* 	double ExMg=Ebeam/2.+(24.-Mg24)*u12C; */
  /* 	double Ex12C; */
	
  /* 	if(FLUG[i][0]>=3&&FLUG12C[(i+1)%2][0]>=2){ */
  /* 	  if(coinflug[i][0]*coinflug[i][1]){ */
  /* 	    Ex12C = ExMg-Ex3a-ene_sum3a[i][0]-hit_adc[(i+1)%2][0][0]; */
  /* 	    if(coinflug[(i+1)%2][0]*coinflug[(i+1)%2][1]){ */
  /* 	    //consider only x(omote) */
  /* 	      HF1(60000+i,Ex12C,1.0); */
  /* 	    } */
  /* 	  } */
  /* 	} */
  /*     } */
  /*   } */
  /* } */
    /**** Data Analysis Above ***************/
    
    /*********** Booking here **********/
  
  /*
    for(i=0;i<N_QDC;i++){
    HF1(10+i,qdc[i],1.0);
    }
 */

//  ***** memo *****
//  silicon number (0,1)=(right,left)
//  silicon strip(ch) (0--15ch:x-axis,16--31ch:y-axis)
//  N_ADC_MOD: 2
//  N_ADC: 32
//  adc[right or left][ch]: adc
//  adcc[right or left][ch]: calibrated adc
//  adc_al[right or left][ch]: adc aligned
//  adcc_al[right or left][ch]: calibrated and aligned adc
//  tdc[ch][event_num]: tdc
//  tdcc[ch][event_num]: calibrated tdc
//  tdc_cnt[]: hit count of each tdc hannel

  for(i=0;i<N_TDC1;i++){
    for(j=0;j<tdc_cnt_al[i];j++){
      if(tdc_al[i][j]!=0 && sicnt[i/32][(i/16)%2]>0){
	HF2(40,i,tdc_al[i][j]-tzero,1.0);
	HF2(42,i,tdcc_al[i][j],1.0);
	HF1(31,i,1.0);
	//	HF1(1000+i,tdc_al[i][j]-tzero,1.0);
	HF1(1000+i,tdcc12C_al[i][j],1.0);
	HF1(2000+i,tdcc_al[i][j],1.0);
	// HF2(2000+i,tdc_al[i][j]-tzero,adcc_al[(int)i/16][(int)i%16],1.0);
      }
    }
  }
  
  
  
  for(j=0;j<2;j++){
    for(i=0;i<N_ADC_MOD;i++){
      HF1(20000+2*i+j,sicnt[i][j],1.0);
    }
    if(j==0){
      HF1(20004,sicnt[0][j]+sicnt[1][j],1.0);
    }
  }
  HF1(20005,sicnt[0][1]+sicnt[1][1],1.0);
  
  //  check Si strip
  for(j=0;j<N_ADC_MOD;j++){
    for(i=0;i<N_ADC;i++){
      //      if(tdc[N_ADC*j+i][0]-tzero>0){
      if(tdc_cnt[j*N_ADC+i]>0.){
	if(sicnt[j][i/16]>0){
	  HF2(10+j,i,adc[j][i],1.0);
	}
	HF1(10+N_ADC_MOD+j,i,1.0);
	//	HF1(20+N_ADC_MOD+j,i,1.0);
      }
      if(tdc_cnt_al[j*N_ADC+i]>0.){
	HF2(20+j,i,adcc_al[j][i],1.0);
      }
     
           // }
      if(tdc_cnt_al[j*N_ADC+i]>0.){
	//	HF1(600+N_ADC*j+i,adc_al[j][i],1.0);
	HF1(800+N_ADC*j+i,adcc_al[j][i],1.0);
	HF1(170+j,adcc[j][i],1.0);
	HF1(170+j+N_ADC_MOD,adcc_al[j][i],1.0);
      }
    }
  }
  
  /**TDC gate & 1st TDC with in one strip**/
  for(i=0;i<N_TDC1;i++){
    if(HFflug_tdcGate[i]){
      HF1(41,i,1.0);
      // HF1(1100+i,tdc_al[i][0]-tzero,1.0);
      HF1(900+i,adcc_al[(int)i/32][(int)i%32],1.0);
      // HF2(2100+i,tdc_al[i][0]-tzero,adcc_al[(int)i/32][(int)i%32],1.0);
    }
  }

  /*********** Booking Above **********/
  return(ip);
}
