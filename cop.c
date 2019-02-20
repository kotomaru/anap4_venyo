switch(p[1]){/* wether both mod's front_cnt=back_cnt */
 case 0:
  if(tdc_nn[i][0]==tdc_nn[(i+1)%2][0]){/* wether mod_0_cnt==mod_1_cnt */
      //int ichx,ichy;
      p2=tdc_nn[0][0]; /* p = both mod's cnt */
      switch (p2){
      case 1: // hitted 1 particles
	//	for(i=0;i<N_ADC_MOD;i++){
	  for(j=0;j<p2;j++){
	    HF2(10000+p2,20*(1-i)+hit_ch[i][j][0],hit_ch[i][j][1],1.0);
	    }
	  //}
	break;
      case 2: // hitted 2 particles
	//	for(i=0;i<N_ADC_MOD;i++){
	  for(j=0;j<p2;j++){
	    HF2(10000+p2,20*(1-i)+hit_ch[i][j][0],hit_ch[i][j][1],1.0);
	  }
	  //}
	break;
      case 3: // hitted 3 particles
	/* if(FLUG[0]>=p2 && FLUG[1]>=p2){ */
	/*   HF2(37,FLUG[0],FLUG[1],1.0); */
	/* } */
	
	/* //3 particle order with tdcc */
	//	for(i=0;i<N_ADC_MOD;i++){
	  for(k=0;k<2;k++){//x & y
	    for(j=0;j<p2;j++){
	      hitch_tdc_odr[i][j][k]=hit_ch[i][j][k];
	    }
	    for(j=0;j<p2;j++){
	      for(jj=j+1;jj<p2;jj++){
		int nj,njj;
		nj=tdcc_al[i*N_ADC+k*16+hitch_tdc_odr[i][j][k]][0];
		njj=tdcc_al[i*N_ADC+k*16+hitch_tdc_odr[i][jj][k]][0];
		if(nj>njj){
		  //	    printf("%d %d,",j,jj);
		  itmp=hitch_tdc_odr[i][j][k];
		  hitch_tdc_odr[i][j][k]=hitch_tdc_odr[i][jj][k];
		  hitch_tdc_odr[i][jj][k]=itmp;
		}
	      }
	      //	printf("%d.%f,",i*N_ADC+k*16+hitch_tdc_odr[i][j][k],tdcc_al[i*N_ADC+k*16+hitch_tdc_odr[i][j][k]][0]);
	      
	    }
	    
	    //3 alpha coincidence flug
	    if((tdcc_al[i*N_ADC+k*16+hitch_tdc_odr[i][1][k]][0]-tdcc_al[i*N_ADC+k*16+hitch_tdc_odr[i][0][k]][0])>=coinmin
	       &&(tdcc_al[i*N_ADC+k*16+hitch_tdc_odr[i][2][k]][0]-tdcc_al[i*N_ADC+k*16+hitch_tdc_odr[i][0][k]][0])<coinmax){
	      coinflug[i][k]=1;
	    }
	    
	    //hist of tdc difference between 3 alphas
	    for(j=1;j<p2;j++){
	      HF1(3000+i,tdcc_al[i*N_ADC+k*16+hitch_tdc_odr[i][j][k]][0]-tdcc_al[i*N_ADC+k*16+hitch_tdc_odr[i][0][k]][0],1.0);
	      if(FLUG[0]>=p2 && FLUG[1]>=p2){
		HF1(3010+i,tdcc_al[i*N_ADC+k*16+hitch_tdc_odr[i][j][k]][0]-tdcc_al[i*N_ADC+k*16+hitch_tdc_odr[i][0][k]][0],1.0);
		if(coinflug[i][k]){
		  HF1(3020+i,tdcc_al[i*N_ADC+k*16+hitch_tdc_odr[i][j][k]][0]-tdcc_al[i*N_ADC+k*16+hitch_tdc_odr[i][0][k]][0],1.0);
		}
	      }
	    }
	  }
	  //}
	
	//hist of check for nombering 6alpha
	  //	for(i=0;i<N_ADC_MOD;i++){
	  for(j=0;j<p2;j++){
	    if(FLUG[0]>=p2 && FLUG[1]>=p2){
	      if(coinflug[i][0]*coinflug[i][1]){//3 alpha coin x y
		if(coinflug[(i+1)%2][0]*coinflug[(i+1)%2][1]){//6alpha coin x y
		  HF2(3030+3*i+j,hit_adc[i][j][0]-hit_adc[i][j][1],
		      tdcc_al[i*N_ADC+0*16+hit_ch[i][j][0]][0]-tdcc_al[i*N_ADC+1*16+hit_ch[i][j][1]][0],1.0);
		  HF2(3040+3*i+j,hit_adc[i][j][0],
		      tdcc_al[i*N_ADC+0*16+hit_ch[i][j][0]][0]-tdcc_al[i*N_ADC+1*16+hit_ch[i][j][1]][0],1.0);
		}
	      }
	    }
	  }
	  //}
	
	
	  //	for(i=0;i<N_ADC_MOD;i++){
	  //  printf("test8");
	  for(j=0;j<p2;j++){
	    ichx=hit_ch[i][j][0];
	    ichy=hit_ch[i][j][1];
	    ene0=hit_adc[i][j][0];
	    ene1=hit_adc[i][j][1];
	    ene_sum3a[i][0]=ene_sum3a[i][0]+hit_adc[i][j][0];//x
	    ene_sum3a[i][1]+=hit_adc[i][j][1];//y
	    
	    HF2(10000+p2,20*(1-i)+ichx,ichy,1.0);
	    if(FLUG[0]>=p2 && FLUG[1]>=p2){
	      if(coinflug[i][0]*coinflug[i][1]){//3 alpha coin x y 
		if(coinflug[(i+1)%2][0]*coinflug[(i+1)%2][1]){//6alpha coin x y
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
	  if(FLUG[0]>=p2 && FLUG[1]>=p2){//tdc gate
	    HF1(50010+i,ex12c[i],1.0);
	    if(coinflug[i][0]*coinflug[i][1]){//3 alpha coin x y
	      HF1(50020+i,ex12c[i],1.0);
	      if(coinflug[(i+1)%2][0]*coinflug[(i+1)%2][1]){
		HF1(50030+i,ex12c[i],1.0);
		HF1(50100+2*i+0,ene_sum3a[i][0],1.0);
		HF1(50100+2*i+1,ene_sum3a[i][1],1.0);
	      }
	    }
	  }
	}
  }//i finish
      50002flug[i]=1;
      HF2(50002,ex12c[0],ex12c[1],1.0);
	if(FLUG[0]>=p2 && FLUG[1]>=p2){
	  50012flug=1;
	  HF2(50012,ex12c[0],ex12c[1],1.0);
	  if(coinflug[0][0]*coinflug[0][1]){//3 alpha coin x y right
	    if(coinflug[1][0]*coinflug[1][1]){//6alpha coin x y
	      
	      HF2(50032,ex12c[0],ex12c[1],1.0);
	      for(i=0;i<N_TDC1;i++){
		HF1(45,tdcc_al[i][0],1.0);
		HF2(46,adcc_al[(int)i/32][(int)i%32],tdc_al[i][0]-tzero,1.0);
		/* 	if((i<=15)||(i>=32&&i<=47)){ */
		/* 	  HF1(47,tdcc_al[i][0],1.0); */
		/* 	} */
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
  }
 
