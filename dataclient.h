/* dataclient.h
   Nov 5,2002  H.Baba (Rikkyo Univ.)

   Using Shared Memory and Semaphore
   Get Block Data from dataserver
*/

#include "./common.h"
#ifndef BLKSIZE
#define BLKSIZE 0x4000
#endif

extern void initDataClient(void);
extern void closeGetData(void);
extern void openRawData(char name[132],int *fnamelen,int *flag);
extern void closeRawData(void);
extern int get_buf(void);
extern void getData(unsigned short *ret,int *flag);
