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
#define N_DUP 32
//#define N_TDC1 64
const long int TIME_H = 12000;//max:12000
const long int TIME_L = -600;//min:-600

int anaevt(int evtlen,unsigned short *rawbuf,struct p4dat *dat){
  int i,ip,j,ii,jj,k,num,n;
  double tmp;
  int segsize,ids,ips,ipsn,count;
  //  unsigned short qdc[N_QDC];
  int tdc[N_TDC][N_DUP]={};
  int tdc_al[N_TDC][N_DUP]={};
  unsigned int adc[N_ADC_MOD][N_ADC]={};
  unsigned int adc_al[N_ADC_MOD][N_ADC]={};
  //  double qdcc[N_QDC];
  double tdcc[N_TDC][N_DUP]={};
  double tdcc_al[N_TDC][N_DUP]={};
  double adcc[N_ADC_MOD][N_ADC]={};
  double adcc_al[N_ADC_MOD][N_ADC]={};
  double adc2[N_ADC_MOD][2][16]={};
  double adccNo1[N_ADC_MOD]={-1000.,-1000.};
  const double tpar[N_TDC][2]={{0.0, 0.1}};
  int tdc_cnt[N_TDC]={};
  int tdc_cnt_al[N_TDC]={};
  short tdc_nn[2][2]={};

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
  double EAm = 5.48;
  int ir,il;
  int ichc[N_ADC_MOD][N_DUP];//ch array for calib [right or left][0~31 (ch)]
  double dsl[N_ADC_MOD][N_DUP];//slope array for calib [right or left][0~31 (ch)]
  double dseg[N_ADC_MOD][N_DUP];//segment array for calib [right or left][0~31 (ch)]
  double dped[N_ADC_MOD][N_DUP];//241Ampedestal ADC array for calib [right or left][0~31 (ch)]
  double dAm[N_ADC_MOD][N_DUP];//241Am ADC array for calib [right or left][0~31 (ch)]
  
  int sicnt[2];
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

  /* v1190 data rearrangement & timing cut */
  for(i=0;i<N_TDC;i++){
    tdc_cnt[i]=0;
  }
  for(i=0;i<ihit;i++){
    //    t_measure=raw_v1190[1][i]-tzero; 
    t_measure=raw_v1190[1][i]; 
    if(t_measure<(TIME_H+tzero) && t_measure>(TIME_L+tzero)){
      j=raw_v1190[0][i];
      if(tdc_cnt[j]<N_DUP){
	tdc[j][tdc_cnt[j]++]=t_measure;
      }
    }
  }
	

  /* time calibration */
  for(i=0;i<N_TDC;i++) {
    for(j=0;j<N_DUP;j++){
      //      tdcc[i][j]=(tdc[i][j]-tpar[i][0])*tpar[i][1];
      tdcc[i][j]=(tdc[i][j]-tpar[0][0])*tpar[0][1];
    }
  }
  

  /* ADC calibration */

  FILE *fright,*fleft;
  fright = fopen("../calib/Am_result00.txt","r");
  fleft = fopen("../calib/Am_result01.txt","r");
  // fright = fopen("../calib/ADC_calib_mod00_ver1.dat","r");
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

  //calibration by data after beam time using pulse fitting
  //I cant think segment of Am  
  /*  FILE *fright,*fleft; */
  /*  FILE *frightAm,*fleftAm; */
  /* fright = fopen("../calib/after_result00.txt","r"); */
  /* fleft = fopen("../calib/after_result01.txt","r"); */
  /* frightAm = fopen("../calib/Am_calb00.txt","r"); */
  /* fleftAm = fopen("../calib/Am_calb01.txt","r"); */
  /* if(fright==NULL){ */
  /*   puts("calib0 file cannot open"); */
  /*   return -1; */
  /* } */
  /* if(fleft==NULL){ */
  /*   puts("calib1 file cannot open"); */
  /*   return -1; */
  /* } */
  /*  if(frightAm==NULL){ */
  /*   puts("Amcalib0 file cannot open"); */
  /*   return -1; */
  /* } */
  /* if(fleftAm==NULL){ */
  /*   puts("Amcalib1 file cannot open"); */
  /*   return -1; */
  /* } */

  /* //input calib data from txt file */
  /* while(fscanf(fright,"%d %lf %lf",&vch,&vsl,&vseg)!=EOF){ */
  /*   ichc[0][vch]=vch;dsl[0][vch]=vsl;dseg[0][vch]=vseg; */
  /* } */
  /* while(fscanf(fleft,"%d %lf %lf",&vch,&vsl,&vseg)!=EOF){ */
  /*   ichc[1][vch]=vch;dsl[1][vch]=vsl;dseg[1][vch]=vseg; */
  /* } */
  /* vch_am=0; */
  /* while(fscanf(frightAm,"%lf %lf %lf %lf",&vped,&vpedsig,&vAm,&vAmsig)!=EOF){ */
  /*   dped[0][vch_am]=vped;dAm[0][vch_am]=vAm; */
  /*   vch_am++; */
  /* } */
  /* vch_am=0; */
  /*  while(fscanf(fleftAm,"%lf %lf %lf %lf",&vped,&vpedsig,&vAm,&vAmsig)!=EOF){ */
  /*   dped[1][vch_am]=vped;dAm[1][vch_am]=vAm; */
  /*   vch_am++; */
  /* } */
  
  /* fclose(fright); */
  /* fclose(fleft); */
  /* fclose(frightAm); */
  /* fclose(fleftAm); */
  /* //ADC calib start */
  /* //adcc=f(adc)/f(adc of 5.48MeV)*5.48 */
  /* for(j=0;j<N_ADC_MOD;j++){ */
  /*   for(i=0;i<N_ADC;i++){ */
  /*     adcc[j][i]=((double)adc[j][i]*dsl[j][i]+dseg[j][i])\ */
  /* 	           /(dAm[j][i]*dsl[j][i]+dseg[j][i])\ */
  /* 	          *EAm; */
  /*   } */
  /* } */

  /**** adc channel align here ****/
  for(j=0;j<N_ADC_MOD;j++){
    for(i=0;i<16;i++){
      adc_al[j][i] = adc[j][i];
      adcc_al[j][i] = adcc[j][i];
      tdc_cnt_al[j*N_ADC+i] = tdc_cnt[j*N_ADC+i];
      for(k=0;k<N_DUP;k++){
	tdc_al[j*N_ADC+i][k] = tdc[j*N_ADC+i][k];
	tdcc_al[j*N_ADC+i][k] = tdcc[j*N_ADC+i][k];
      }
    }
    for(i=16;i<24;i++){
      adc_al[j][i] = adc[j][39-i];
      adcc_al[j][i] = adcc[j][39-i];
      tdc_cnt_al[j*N_ADC+i] = tdc_cnt[j*N_ADC+39-i];
      for(k=0;k<N_DUP;k++){
	tdc_al[j*N_ADC+i][k] = tdc[j*N_ADC+39-i][k];
	tdcc_al[j*N_ADC+i][k] = tdcc[j*N_ADC+39-i][k];
      }
    }
    for(i=24;i<32;i++){
      adc_al[j][i] = adc[j][55-i];
      adcc_al[j][i] = adcc[j][55-i];
      tdc_cnt_al[j*N_ADC+i] = tdc_cnt[j*N_ADC+55-i];
      for(k=0;k<N_DUP;k++){
	tdc_al[j*N_ADC+i][k] = tdc[j*N_ADC+55-i][k];
	tdcc_al[j*N_ADC+i][k] = tdcc[j*N_ADC+55-i][k];
      }
    }
  }
  /**** adc channel align above ****/

  /***No.1 adc search here***/
  for(i=0;i<N_ADC_MOD;i++){
    for(j=0;j<N_ADC;j++){
      if(adcc_al[i][j]>adccNo1[i]){
	adccNo1[i]=adcc_al[i][j];
      }
    }
  }
  
  /***No.1 adc search here***/



//  divide adc into x and y axis
  for(i=0;i<N_ADC_MOD;i++){
    for(j=0;j<N_ADC;j++){
      adc2[i][(int)j/16][j%16]=adcc_al[i][j];
    }
  }
//  divide tdc hits into [2][2]
  for(i=0;i<N_TDC1;i++){
    tdc_nn[(int)i/32][(i/16)%2]+=tdc_cnt[i];
  }

//  make vector of Si pixels
  short adc_ch[2][16]; // temporary adc channels
  short pixx[N_ADC_MOD][N_ADC/2]; // temporary coordinates
  short pixy[N_ADC_MOD][N_ADC/2];
  short hit_c[N_ADC_MOD]; // temporary hits count of each Si
  unsigned short p2=0; // hit count
  unsigned short pn=0; // particle number
  unsigned short pp[2]={};
  unsigned short hit_ch[N_ADC_MOD][16][2]; // coordinates against pn
  double hit_adc[N_ADC_MOD][16][2];

  for(i=0;i<2;i++){
    for(j=0;j<16;j++){
      adc_ch[i][j]=-1;
      pixx[i][j]=-1;
      pixy[i][j]=-1;
    }
    for(j=0;j<16;j++){
      for(k=0;k<2;k++){
	hit_ch[i][j][k]=-1;
	hit_adc[i][j][k]=0.;
      }
    }
  }

  unsigned short p[2]={};

  for(i=0;i<N_ADC_MOD;i++){ // loop modele 0,1
    pn=0;
    for(k=0;k<2;k++){
      for(j=0;j<16;j++){
	adc_ch[k][j]=j;
      }
    }
    p[i]=(tdc_nn[i][0]>tdc_nn[i][1]) ? tdc_nn[i][0]-tdc_nn[i][1]:tdc_nn[i][1]-tdc_nn[i][0];    
    switch(p[i]){
    case 0: // same counts
      for(j=0;j<N_ADC;j++){
	if(tdc_cnt_al[32*i+j]>0){ // select ch
	  adc2[i][j/16][j%16]=adcc_al[i][j];
	}
      }
      for(n=0;n<2;n++){
	for(j=0;j<16;j++){
	  for(k=j+1;k<16;k++){
	    if(adc2[i][n][j]<adc2[i][n][k]){
	      tmp=adc2[i][n][j];
	      num=adc_ch[n][j];
	      adc2[i][n][j]=adc2[i][n][k];
	      adc_ch[n][j]=adc_ch[n][k];
	      adc2[i][n][k]=tmp;
	      adc_ch[n][k]=num;
	    }
	  }
	}
      }
      for(k=0;k<tdc_nn[i][0];k++){
	hit_ch[i][pn][0]=adc_ch[0][k];
	hit_ch[i][pn][1]=adc_ch[1][k];
	hit_adc[i][pn][0]=adc2[i][0][k];
	hit_adc[i][pn][1]=adc2[i][1][k];
	pn++;
      }
      for(j=0;j<pn;j++){
	HF2(50+i,hit_ch[i][j][0],hit_ch[i][j][1],1.0);
      }
      break;
      /* ****************************************** */
    /* case 1:   //diff 1 */
    /*   for(j=0;j<N_ADC;j++){ */
    /* 	if(tdc_cnt_al[32*i+j]>0){ // select ch */
    /* 	  adc2[i][j/16][j%16]=adcc_al[i][j]; */
    /* 	} */
    /*   } */
    /*   for(n=0;n<2;n++){ */
    /* 	for(j=0;j<16;j++){ */
    /* 	  for(k=j+1;k<16;k++){ */
    /* 	    if(adc2[i][n][j]<adc2[i][n][k]){ */
    /* 	      tmp=adc2[i][n][j]; */
    /* 	      num=adc_ch[n][j]; */
    /* 	      adc2[i][n][j]=adc2[i][n][k]; */
    /* 	      adc_ch[n][j]=adc_ch[n][k]; */
    /* 	      adc2[i][n][k]=tmp; */
    /* 	      adc_ch[n][k]=num; */
    /* 	    } */
    /* 	  } */
    /* 	} */
    /*   } */
    /*   for(n=0;n<2;n++){ */
    /* 	for(k=0;k<tdc_nn[i][n];k++){ */
    /* 	  hit_ch[i][pp[n]][0]=adc_ch[0][k]; */
    /* 	  hit_ch[i][pp[n]][1]=adc_ch[1][k]; */
    /* 	  hit_adc[i][pp[n]][0]=adc2[i][0][k]; */
    /* 	  hit_adc[i][pp[n]][1]=adc2[i][1][k]; */
    /* 	  pp[n]++; */
    /* 	} */
    /*   } */
    /*   for(n=0;n<2;n++){ */
    /* 	for(j=0;j<pp[n];j++){ */
    /* 	  HF2(60+i,hit_ch[i][j][0],hit_ch[i][j][1],1.0); */
    /* 	} */
    /*   } */
    /*   break; */

    default:
      break;
    }
  }

  double ex12c[2]={};
  double vec12c[2][4]={};
  double vec[2][3][4]={};
  double rvec[3]={};

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
  
  if((p[0]==0)&&(p[1]==0)){	/* wether both mod's front_cnt=back_cnt */
    if(tdc_nn[0][0]==tdc_nn[1][0]){ /* wether mod_0_cnt==mod_1_cnt */
      // int ichx,ichy;
      p2=tdc_nn[0][0];		    /* p = both mod's cnt */
      switch (p2){
      case 1: // hitted 1 particles
  	for(i=0;i<N_ADC_MOD;i++){
	  for(j=0;j<p2;j++){
	    HF2(10000+p2,20*(1-i)+hit_ch[i][j][0],hit_ch[i][j][1],1.0);
	  }
	}
	break;
      case 2: // hitted 2 particles
  	for(i=0;i<N_ADC_MOD;i++){
	  for(j=0;j<p2;j++){
	    HF2(10000+p2,20*(1-i)+hit_ch[i][j][0],hit_ch[i][j][1],1.0);
	  }
	}
	break;
      case 3: // hitted 3 particles
  	for(i=0;i<N_ADC_MOD;i++){
	  for(j=0;j<p2;j++){
	    ichx=hit_ch[i][j][0];
	    ichy=hit_ch[i][j][1];
	    ene0=hit_adc[i][j][0];
	    ene1=hit_adc[i][j][1];

	    HF2(10000+p2,20*(1-i)+ichx,ichy,1.0);
	    
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
	}
	HF2(50002,ex12c[0],ex12c[1],1.0);
	break;

      default:
	break;
      }
    }
  }


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
//  tdc_cnt[]: hit count of each tdc channel

  for(j=0;j<2;j++){
    for(i=0;i<N_ADC_MOD;i++){
      sicnt[i]=0;
      for(k=0;k<16;k++){
	if(tdc_cnt[i*32+j*16+k]>0){
	  sicnt[i]++;
	}
      }
      HF1(20000+2*i+j,sicnt[i],1.0);
    }
    if(j==0){
    HF1(20004,sicnt[0]+sicnt[1],1.0);
    }
  }
  HF1(20005,sicnt[0]+sicnt[1],1.0);

//  check Si strip
  for(j=0;j<N_ADC_MOD;j++){
    for(i=0;i<N_ADC;i++){
//      if(tdc[N_ADC*j+i][0]-tzero>0){
      if(tdc_cnt[j*N_ADC+i]>0.){
	if(sicnt[j]>0){
	  HF2(10+j,i,adc[j][i],1.0);
	}
	HF1(10+N_ADC_MOD+j,i,1.0);
	//	HF1(20+N_ADC_MOD+j,i,1.0);
      }
      if(tdc_cnt_al[j*N_ADC+i]>0.){
	  HF2(20+j,i,adcc_al[j][i],1.0);
      }
      // HF1(500+N_ADC*j+i,adc[j][i],1.0);
      if(sicnt[0]+sicnt[1]==1) {
	HF1(700+N_ADC*j+i,adcc_al[j][i],1.0);
	//	HF1(1000+j,i,1.0);
	HF1(170+j,adcc[j][i],1.0);
      }
	// }
      if(tdc_cnt_al[j*N_ADC+i]>0.){
	//		HF1(600+N_ADC*j+i,adc_al[j][i],1.0);
		HF1(800+N_ADC*j+i,adcc_al[j][i],1.0);
		HF1(170+j+N_ADC_MOD,adcc_al[j][i],1.0);
      }
    }
  }

   //alligned & ascending order tdc 
  double tmp2;
  for(i=0;i<N_TDC1;i++){
    for(j=0;j<tdc_cnt_al[i];j++){
      for(k=j+1;k<tdc_cnt_al[i];k++){
	if(tdc_al[i][j]>tdc_al[i][k]){
	  tmp2=tdc_al[i][j];
	  tdc_al[i][j]=tdc_al[i][k];
	  tdc_al[i][k]=tmp2;
	}
      }
    }
  }

  for(i=0;i<N_TDC1;i++){
    for(j=0;j<tdc_cnt_al[i];j++){
      if(tdc_al[i][j]!=0 && sicnt[i/32]>0){
	HF2(30,i,tdc_al[i][j]-tzero,1.0);
	HF1(31,i,1.0);
	HF1(1000+i,tdc_al[i][j]-tzero,1.0);
	//	HF2(2000+i,tdc_al[i][j]-tzero,adcc_al[(int)i/16][(int)i%16],1.0);
      }
    }
  }
  
  //TDC Gate 2500~5000 & the first TDC only 20190130 shiyo modified
   
  float gtmin=2500.;//TDC Gate
  float gtmax=5000.;
  float adccmin=0.2;//ADC Gate to eliminate crosstalk
  float adccmax=10.;
  int flug[N_ADC_MOD][N_ADC]={};//(if within ADC GATE) = 1
  int FLUG[N_ADC_MOD]={};//SUM of flug
  
  for(i=0;i<N_TDC1;i++){//TDC ch = 0~63
    HF2(33,i,tdc_cnt_al[i]*2,1.0);
    if(tdc_al[i][0]!=0 && sicnt[i/32]>0
       && (tdc_al[i][0]-tzero)>gtmin && (tdc_al[i][0]-tzero)<gtmax)
  	{
  	  HF2(40,i,tdc_al[i][0]-tzero,1.0);
  	  HF1(41,i,1.0);
	  HF1(1100+i,tdc_al[i][0]-tzero,1.0);
	  HF1(900+i,adcc_al[(int)i/32][(int)i%16],1.0);

	  /***except closs talk ***/
	  if(adcc_al[(int)i/32][(int)i%16] != adccNo1[(int)i/32]){
	    HF1(400+i,adcc_al[(int)i/32][(int)i%16],1.0);
	}
	  if(adcc_al[(int)i/32][(int)i%16]>adccmin
	     && adcc_al[(int)i/32][(int)i%16]<adccmax){
	    HF2(2100+i,tdc_al[i][0]-tzero,adcc_al[(int)i/32][(int)i%16],1.0);
	    flug[(int)i/32][(int)i%16]=1;
	  }
	}
  }
  for(i=0;i<N_ADC_MOD;i++){
    for(j=0;j<N_ADC/2;j++){
      FLUG[i]=FLUG[i]+flug[i][j];
    }
  }
  HF2(34,FLUG[0],FLUG[1],1.0);

  double re_ex12c[2]={};
  double re_vec12c[2][4]={};
  double re_vec[2][3][4]={};
  double re_rvec[3]={};

  double re_phe;

  int re_ichx;
  int re_ichy;
  double re_ene0;
  double re_ene1;

  
  if((p[0]==0)&&(p[1]==0)){	/* wether both mod's front_cnt=back_cnt */
    if(tdc_nn[0][0]==tdc_nn[1][0]){ /* wether mod_0_cnt==mod_1_cnt */
      p2=tdc_nn[0][0];/* p = both mod's cnt */
       HF2(35,FLUG[0],FLUG[1],1.0);
      if(FLUG[0]>=p2 && FLUG[1]>=p2){
	 HF2(36,FLUG[0],FLUG[1],1.0);
	switch (p2){
	  /* case 1: // hitted 1 particles */
	  /*   for(i=0;i<N_ADC_MOD;i++){ */
	  /*     for(j=0;j<p2;j++){ */
	  /* 	HF2(10000+p2,20*(1-i)+hit_ch[i][j][0],hit_ch[i][j][1],1.0); */
	  /*     } */
	  /*   } */
	  /*   break; */
	  /* case 2: // hitted 2 particles */
	  /*   for(i=0;i<N_ADC_MOD;i++){ */
	  /*     for(j=0;j<p2;j++){ */
	  /* 	HF2(10000+p2,20*(1-i)+hit_ch[i][j][0],hit_ch[i][j][1],1.0); */
	  /*     } */
	  /*   } */
	  /*   break; */
	case 3: // hitted 3 particles
	  HF2(37,FLUG[0],FLUG[1],1.0);
	  for(i=0;i<N_ADC_MOD;i++){
	    for(j=0;j<p2;j++){
	      re_ichx=hit_ch[i][j][0];
	      re_ichy=hit_ch[i][j][1];
	      re_ene0=hit_adc[i][j][0];
	      re_ene1=hit_adc[i][j][1];
	      
	      HF2(10010+p2,20*(1-i)+re_ichx,re_ichy,1.0);
	      
	      re_vec[i][j][0]=re_ene0+mhe;
	      
	      re_rvec[0]=((double)re_ichx-7.5)*3.0;
	      re_rvec[1]=(7.5-(double)re_ichy)*3.0;
	      re_rvec[2]=l;
	      
	      unitvec(re_rvec,re_rvec);
	      rotvec(re_rvec,&re_vec[i][j][1],1,pow(-1,i)*theta);
	      
	      re_phe=sqrt(re_vec[i][j][0]*re_vec[i][j][0]-mhe*mhe);
	      vecadd(&re_vec[i][j][1],re_rvec,&re_vec[i][j][1],re_phe,0.0);
	      vecadd4(re_vec[i][j],re_vec12c[i],re_vec12c[i],1,1);
	      //	    printf("%d:%d  p:%8.3f ene:%8.3f\n",i,j,phe,ene0);
	      //printf("%d:%d: %8.3f   \n",i,j,
	      //	   sqrt(scapro4(vec[i][j],vec[i][j])));
	    }
	    re_ex12c[i]=sqrt(scapro4(re_vec12c[i],re_vec12c[i]))-m12c;
	    //	  printf("ex:%8.3f\n",ex12c[i]);
	    HF1(50010+i,re_ex12c[i],1.0);
	    
	  }
	  HF2(50012,re_ex12c[0],re_ex12c[1],1.0);
	  break;
	  
	default:
	  break;
	}
      }
    }
  }
  
  
  /*********** Booking Above **********/
  return(ip);
}
