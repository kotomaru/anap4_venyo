/* dataclient.c
   Sep 9,2000  H.Baba (Rikkyo Univ.)

   Using Shared Memory and Semaphore
   Get Block Data from dataserver
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <unistd.h>

#include "./common.h"
#ifndef BLKSIZE
#define BLKSIZE 0x4000
#endif

char com;
unsigned short buf[BLKSIZE/2];
FILE *datafile;
static int ninit = 0;
int modeflag; /* 0 = ONLINE , 1 = OFFLINE */
int shmid,semid;
unsigned int blocknum,nblocknum;
char *shmp;
int blksize = BLKSIZE;

#if defined(__GNU_LIBRARY__) && !defined(_SEM_SEMUN_UNDEFINED)
#else
union semun {
  int val;                        /* value for SETVAL */
  struct semid_ds *buf;           /* buffer for IPC_STAT & IPC_SET */
  unsigned short *array;          /* array for GETALL & SETALL */
  struct seminfo *__buf;          /* buffer for IPC_INFO */
};
#endif

/* union semun semunion */

struct sembuf semb;
union semun semunion;

/* Semaphore Interface */
void semaphore_p(void){
  semb.sem_op = -1;
  semop(semid,&semb,1);
}
void semaphore_v(void){
  semb.sem_op = 1;
  semop(semid,&semb,1);
}

/* for ONLINE */
void initDataClient(void){
  if(ninit == 0){
    blocknum = 0;
    nblocknum = 0;
    modeflag = 0;
    com = 1;
    memset(buf,0,blksize);
    /* Shared Memory */
    if((shmid = shmget(SHMKEY,blksize+4,IPC_CREAT|0777)) == -1){
      perror("Can't create shared memory.\n");
      exit(1);
    }
    shmp = shmat(shmid,0,0);
    
    /* Semaphore */
    if((semid = semget(SEMKEY,1,IPC_CREAT|0666)) == -1){
      perror("Can't create semaphore.\n");
      shmdt(shmp);
      shmctl(shmid,IPC_RMID,0);
      exit(1);
    }
    semunion.val = 1;
    if(semctl(semid,0,SETVAL,semunion) == -1){
      perror("Can't control semaphore.\n");
      shmdt(shmp);
      shmctl(shmid,IPC_RMID,0);
      semctl(semid,0,IPC_RMID,semunion);
      exit(1);
    }
    memset(shmp,0,blksize);
    ninit = 1;
  }
}

void closeGetData(void){
}


/* for OFFLINE */
void openRawData(char name[132],int *fnamelen,int *flag){
  char filename[132];

  memset(buf,0,blksize);
  memset(filename,0,sizeof(filename));

  modeflag = 1;
  *flag = 0;
  strncpy(filename,name,*fnamelen);

  if((datafile = fopen(filename,"r")) == NULL){
    *flag = 1;
  }
}

void closeRawData(void){
  fclose(datafile);
}

int get_buf(void){
  int dataflag,size;

  if(modeflag == 0){
    semaphore_p();
    memcpy((char *)&nblocknum,shmp+blksize,4);
    if(nblocknum == blocknum){
      dataflag = 1;
    }else if(nblocknum == 0xffffffff){
      dataflag = 2;
    }else{
      blocknum = nblocknum;
      dataflag = 0;
    }
  }else{
    size = fread(buf,2,blksize/2,datafile);
    if(size == blksize/2){
      dataflag = 0;
    }else{
      dataflag = 3;
    }
  }

  return dataflag;
}

void getData(unsigned short *ret,int *flag){
  usleep(1);
  *flag = get_buf(); 
  if(*flag == 0){
    if(modeflag == 0){
      memcpy(ret,shmp,blksize);
    }else{
      memcpy(ret,buf,blksize);
    }
  }
  if(modeflag == 0){
    semaphore_v();
  }
}
