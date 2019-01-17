
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <cfortran.h>
#include <hbook.h>
#include <kernlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <ncurses.h>

#include "dataclient.h"
#include "kinema.h"
#include "anap4.h"
#include "startup.h"

#define VERSI "0.92"

typedef struct { float PAW[PAWC_SIZE]; } PAWC_DEF;
#define PAWC COMMON_BLOCK(PAWC,pawc)
COMMON_BLOCK_DEF(PAWC_DEF,PAWC);
PAWC_DEF PAWC;

/*******************************************:
  routine structure

  analysis--+--anahead
            +--anablk--anaevt
            +--anaend
            +--anareturn

	    **************************************/


int main(int iarg, char *argv[]){
  int i;
  struct p4dat dat; /* Analysis data */
  pthread_t thread_id;
  int ptstat;
  sigset_t ss;
  int signo,ret;

  char outpath[PATH_L]; 

  //  signal(SIGINT,(void *)

  /***** Read arguments ****************/
  dat.nrun=startup(iarg,argv,&dat.flag,dat.runv,outpath);
  if(dat.nrun==-1) {
    printf("%s: Exit...\n",argv[0]);
    show_usage(argv);
    exit(-1);
  }
  /***** Initialize ****************/
  for(i=0;i<SCA_CH;i++) dat.sca[i]=0;
  dat.nevt=0;
  dat.nblk=0;
  dat.nreadbyte=0;


  /**** Open histgrams ***********/
  histopen(dat.flag,outpath);

  initscr();
  clear();
  mvprintw(0,0,"anap4 Ver %s  Analyzer for KADAIKENKYU P4",VERSI);
  if(dat.flag&OPT_G)  mvprintw(5,0,"Outpath: Global Section: %s",outpath);
  else mvprintw(5,0,"Outpath: %s",outpath);
  refresh();
  //  printf("Flag:%x\n",dat.flag);
  //  printf("Outpath:%s\n",outpath);
  //  printf("Inpath (Nrun=%d):\n",dat.nrun);
  //  for(i=0;i<dat.nrun;i++){
  //    printf("%d: %s\n",i,dat.runv[i]);
  //  }

  ptstat=pthread_create(&thread_id,NULL,sigthread,(void *)&dat);

  sigemptyset(&ss);
  ret=sigaddset(&ss,SIGINT);
  ret=sigprocmask(SIG_BLOCK,&ss,NULL);


  if(dat.flag&OPT_OL) {
    initDataClient();
    mvprintw(1,0,"Input: ONLINE");

    dat.nrun=1;
    dat.fdin=0;
  }
  for(dat.irun=0;dat.irun<dat.nrun;dat.irun++){
    analysis(&dat);
  }
  
  /**** Close histgrams ***********/
  histclose(dat.flag);
  //  endwin();
  move(LINES-1,0);
  refresh();
  exit(0);
}

int histopen(int flag,char *outpath){
  int lreq=1024,istat;
  int i,j;

  if(flag&OPT_G){
    HLIMAP(800000,outpath);
    //    printf("Global section: %s\n",outpath);
  }
  else{
    HLIMIT(PAWC_SIZE);
    HROPEN(1,"TEST",outpath,"N",lreq,istat);
  }

  histdef();

  return(1);
}

int histclose(flag){
  int istat;
  if(!(flag&OPT_G)){
    HROUT(0,istat," ");
    HREND("TEST");
  }
  /*  else{
    HFREEM(0);
    }*/
}

int analysis(struct p4dat *dat){
  int ires,flag,fdin,readbyte;
  int jevt=0,idev,olflag=0;
  unsigned short rawbuf[B_SIZE];
  int i;
  char emsg[200];
  struct stat fstat;

  dat->ireadbyte=0;
  dat->ievt=0;
  dat->iblk=0;
  dat->filesize=0;
  flag=dat->flag;

  /* Open data file *********/
  if(!(flag&OPT_OL)) {
    dat->fdin=open(dat->runv[dat->irun],O_RDONLY,NULL);
    if(dat->fdin!=-1) {
      //      printf("Open: %s\n",dat->runv[dat->irun]);
      mvprintw(1,0,"Input (%2d/%2d): %s\n",
	       dat->irun+1,dat->nrun,dat->runv[dat->irun]);
      stat(dat->runv[dat->irun],&fstat);
      dat->filesize=fstat.st_size;
    }
    else {
      fopenerror(dat->runv[dat->irun]); 
      anareturn(dat);
    }
  }
  refresh();
  fdin=dat->fdin;
#if 0
  /*** Header Block ****/
  if(!(flag&OPT_OL)) {
    readbyte=read(fdin,rawbuf,B_SIZE);
    dat->iblk++;
    anahead(rawbuf);
  }
#endif 

  while(1){
    if(flag&OPT_OL){
      getData(rawbuf,&olflag);
      if(olflag!=0 || ((rawbuf[0]==0) && (rawbuf[4]==0))) {
	sleep(1);
	continue;
      }
      readbyte=B_SIZE;
    }
    else readbyte=read(fdin,rawbuf,B_SIZE);
    if(readbyte<B_SIZE) break;
    dat->ireadbyte+=readbyte;

#if _DEBUG
    printf("Read (%d)!!\n",dat->iblk);
#endif

    dat->iblk++;

    //    if(iblk==3) for(i=0;i<SCA_CH;i++) sca[i]=0;
    switch(rawbuf[0]){
    case 0x0000:  /* Event Block */
      idev=anablk(rawbuf,dat,&jevt);
      dat->ievt+=jevt;
      showsca(dat);
      break;
    case 0x0001:  /* Header Block */
      anahead(rawbuf);
      continue;
      break;
    case 0xffff: /* Ender Block */
      if(rawbuf[1]==0){
	anaend(rawbuf);
	showsca(dat);
	anareturn(dat);
	return(1);
      }
      else{
	sprintf(emsg,
		"Fake run end block header (%04x) at blk=%d."
		" Skip this block ....\n",rawbuf[0],dat->iblk);
	dumpmsg(emsg);
      }
      break;
    default: /* Illeagal Block */
#if 0
      sprintf(emsg,
	      "Wrong block header (%04x) at blk=%d. Skip this run ....\n",
	      rawbuf[0]);
      dumpmsg(emsg);
      anareturn(dat);
      return(-1);
#endif
      sprintf(emsg,
	      "Wrong block header (%04x) at blk=%d. Skip this block....\n",
	      rawbuf[0],dat->iblk);
      dumpmsg(emsg);
      break;
    }
  }
  /*** Event Block ****/
  return(1);
}

int anareturn(struct p4dat *dat){
  int i;
  if(!(dat->flag&OPT_OL)) close(dat->fdin);
  dat->nevt+=dat->ievt;
  dat->nreadbyte+=dat->ireadbyte;
  dat->nblk+=dat->iblk;
  //  printf("Analyzed Run: %d/%d\n",dat->irun+1,dat->nrun);
  //  printf("Read byte: %d/%lld\n",dat->ireadbyte,dat->nreadbyte);
  //  printf("Blocks: %d/%lld\n",dat->iblk,dat->nblk);
  //  printf("Events: %d/%lld\n",dat->ievt,dat->nevt);
  //  for(i=0;i<SCA_CH;i++) printf("Scaler %02d: %8lld\n",i,dat->sca[i]);
  return(1);
}

int anahead(unsigned short *rawbuf){
  int i,j;
  char *buf;
  if(rawbuf[0]!=1){
    printf("No header block. Skip this run...\n");
    return(-1);
  }
  mvprintw(2,0,"Run Information:");
  /******* Run Number *************/
  buf=(char *)&rawbuf[10];
  //  for(i=0;i<8;i++) printf("%c",buf[i]);
  //  printf("\n");
  for(i=0;i<8;i++) mvprintw(3,2+i,"%c",buf[i]);
  /******* Start Time *************/
  buf=(char *)&rawbuf[15];
  //  for(i=0;i<18;i++) printf("%c",buf[i]);
  //  printf("\n");
  for(i=0;i<18;i++) mvprintw(3,i+17,"%c",buf[i]);
  /******* Header *************/
  buf=(char *)&rawbuf[50];
  //  for(i=0;i<80;i++) printf("%c",buf[i]);
  //  printf("\n");
  for(i=0;i<COLS-3;i++) mvprintw(4,i+2,"%c",buf[i]);
  refresh();
  return(1);
}

int anaend(unsigned short *rawbuf){
  int i,j;
  char *buf;

  /******* Run Number *************/
  buf=(char *)&rawbuf[10];
  //  for(i=0;i<8;i++) printf("%c",buf[i]);
  //  printf("\n");
  /******* Start and Stop Time *************/
  buf=(char *)&rawbuf[15];
  //  for(i=0;i<2;i++){
  //    for(j=0;j<18;j++) printf("%c",buf[i*18+j]);
  //    printf("\n");
  //  }
  /******* Header and Ender *************/
  buf=(char *)&rawbuf[50];
  //  for(i=0;i<2;i++){
  //    for(j=0;j<80;j++) printf("%c",buf[i*80+j]);
  //    printf("\n");
  //  }
  /******* Print Date and Time *************/
  buf=(char *)&rawbuf[34];
  //  for(i=0;i<28;i++) printf("%c",buf[i]);
  //  printf("\n");
  return(1);
}

int anablk(unsigned short *rawbuf,struct p4dat *dat,int *jevt){
  int ievt=0,idevt,ip,i,j;
  int evthd,evtsize,idf;
  int ipnext;
  unsigned int *dsca;

  char tmpemsg[200];

#if 0
  sprintf(tmpemsg,
	  "%04x %04x %04x %04x %04x %04x %04x %04x \n",
	  rawbuf[0],rawbuf[1],rawbuf[2],rawbuf[3],
	  rawbuf[4],rawbuf[5],rawbuf[6],rawbuf[7]);
  dumpmsg(tmpemsg);
  sprintf(tmpemsg,
	  "%04x %04x %04x %04x %04x %04x %04x %04x \n",
	  rawbuf[8],rawbuf[9],rawbuf[10],rawbuf[11],
	  rawbuf[12],rawbuf[13],rawbuf[14],rawbuf[15]);
  dumpmsg(tmpemsg);
#endif 

  for(ip=4;ip<B_SIZE;){
    /////////////////
    evthd=rawbuf[ip++];
    if(evthd&0x8000) evtsize=(evthd&0x7fff);
    else{
      char emsg[200];
      sprintf(emsg,
	      "Wrong event header at ievt=%d (%04x)."
	      " Skip this block (%d)...\n",ievt+1,evthd,dat->iblk);
      dumpmsg(emsg);
      return(-1);
    }
    if(evtsize==0x7fff) break;
    ievt++;
    ipnext=ip+evtsize-1;
    idf=rawbuf[ip++];
    idevt=rawbuf[ip++];
    ///
    //    sprintf(tmpemsg,
    //	    "event size:%d  idf:%04x idevt:%04x\n",
    //	    evtsize,idf,idevt);
    //    dumpmsg(tmpemsg);
    ///
    if(ievt!=idevt) {
      char emsg[200];
      sprintf(emsg,
	      "Wrong event number (count:%d read:%d). Skip this block...\n",
	     ievt,idevt);
      dumpmsg(emsg);
      return(-1);
    }
#if _DEBUG
    printf("New Evt %d in Blk %d.\n",ievt,dat->iblk);
#endif
    /********** Event analysis routine ***************/
    //    ip+=anaevt(ipnext-ip,rawbuf+ip,dat);
    anaevt(ipnext-ip,rawbuf+ip,dat);
    ip+=(ipnext-ip);
  }
  /*******  Read Scaler ***********/
  dsca=(unsigned int *)&rawbuf[B_SIZE/2-SCA_CH*2];
  for(i=0;i<SCA_CH;i++) dat->sca[i]=dsca[i];

  //  for(i=0;i<SCA_CH;i++) printf("%02d: %d\n",i,dsca[i]);
  /*  printf("\n");*/
  *jevt=ievt;
  //  sleep(1);
  return(1);
}


void sighan(struct p4dat *dat){
  anareturn(dat);
  histclose(dat->flag);
  //  endwin();
  //  move(LINES-1,0);
  // refresh();
  exit(1);
}

void *sigthread(void *arg){
  sigset_t ss;
  struct p4dat *dat;
  int ret,signo;

  sigemptyset(&ss);
  ret=sigaddset(&ss,SIGINT);
  ret=sigprocmask(SIG_BLOCK,&ss,NULL);

  dat=(struct p4dat *)arg;
  while(1){
    if(sigwait(&ss,&signo)==0) sighan(dat);
  }
}

void dumpmsg(char *buf){
  move(9+SCA_CH,0);
  insertln();
  printw("%s",buf);
  refresh();
}


void showaratio(struct p4dat *dat){
  static int ipg=0;
  static char mark[]="\\|/-";
  int igauge,irat;
  double ratio;
  ratio=(double)dat->ireadbyte/dat->filesize;
  irat=(int )(ratio*100);
  igauge=(int )(ratio*(COLS-5));
  if(ipg!=igauge) mvprintw(6,ipg,"=");
  mvprintw(6,igauge,"%c",mark[dat->iblk%4]);
  mvprintw(6,COLS-4,"%2d%%",irat);
  refresh();
  ipg=igauge;
}

void showsca(struct p4dat *dat){
  int i;
 if((dat->flag&OPT_OL)) {
    mvprintw(7,0,"Analyzed Events: %8d (%4d blk) [%4d kB]:\n",
	     dat->ievt,dat->iblk,dat->ireadbyte/1024);
  }
  else{
    mvprintw(7,0,"Analyzed Events: %8d (%4d blk) [%4d kB/%4d kB]:\n",
	     dat->ievt,dat->iblk,dat->ireadbyte/1024,
	     dat->filesize/1024);
    showaratio(dat);
  }
  mvprintw(8,0,"Scalers:");

  /* Scaler show changed on 17/07/07 for large value */
  for(i=0;i<SCA_CH;i++) mvprintw(9+i,2,"%02d: %15lld\n",i,dat->sca[i]);

  refresh();
}
