/* DATA analysis code for RDF *********/

#ifndef _ANAPA

#define _DEBUG 0 
#define PAWC_SIZE 10000000
#define DEP_TDC 4

#define PATH_L 200
#define MAX_NRUN 50
#define B_SIZE 16384
#define SCA_CH 12

struct p4dat {
  /** Do not edit following variables ********************/
  /* Class Main */
  int nrun,irun;
  char runv[MAX_NRUN][PATH_L]; /* list of input file */

  long long nevt;  /* Number of the all events */
  long long nblk;  /* Number of the all blocks */

  unsigned long long int sca[SCA_CH]; /* changed on 17/07/07 for large value */
  long long nreadbyte;    /* Data read bytes */
  int flag;        /* Options */
  int fdin;        /* Input file */

  /* Class analysis */
  int ireadbyte;   /* Data read bytes in each run*/
  int filesize;    /* File size in each run*/
  int ievt;  /* Number of the events in each run */
  int iblk;  /* Number of block in each run */
  /** Do not edit above variables ************************/
};

int analysis(struct p4dat *dat);
int anareturn(struct p4dat *dat);
int anahead(unsigned short *rawbuf);
int anablk(unsigned short *rawbuf,struct p4dat *dat,int *jevt);
int anaevt(int evtlen,unsigned short *rawbuf,struct p4dat *dat);
int anaend(unsigned short *rawbuf);

int histopen(int flag,char *outpath);
int histclose(int flag);
int histdef();

void sighan(struct p4dat *dat);
void *sigthread(void *dat);

void dumpmsg(char *buf);
void showaratio(struct p4dat *dat);
void showsca(struct p4dat *dat);

#include "userdef.h"
#define _ANAPA 1
#endif
