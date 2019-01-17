#include <errno.h>
#include "anap4.h"

/* OPT_?? must be give as a bit pattern. */
#define OPT_G  0x01
#define OPT_HB  0x02
#define OPT_OL  0x04

int startup(int iarg,char *argv[],int *flag,char (*runv)[PATH_L],char *outpath);
int check_option(char *arg);
int show_usage(char *argv[]);
void fopenerror(char *s);
