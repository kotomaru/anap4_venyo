#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "startup.h"




int check_option(char *arg){
  /* Return Value 
     >= 0: Option ID
     -1: Unknown option */
  const struct {char *parm; int i;}
  table[]={{"-g",OPT_G},
	   {"-hb",OPT_HB},
	   {"-online",OPT_OL}
  };
  int j;
  for(j=0;j<3;j++){
    /* when you add new operand, you MUST change an above line.*/
    if(strcmp(arg,table[j].parm)==0){
      return(table[j].i);
    }
  }
  return(-1);
}

int show_usage(char *argv[]){
  printf("%s -online -g [glob_sect] -hb [hbfile] infile1 [infile2] ....\n",argv[0]);
  printf("-online: Online analysis\n");
  printf("-g: Use global section. Name of the global section can be\n"
	 "    specified. If [glob_sect] is not given, the name of global\n"
	 "    section is TEST.\n");
  printf("-hb: Store the results into the hbook file. Name of the hbook\n"
	 "     file can be specified. If [hbfile] is not given, the name\n"
	 "     of the hbook file is test.hb.\n");
  printf("If several input files are given, all the resuls are summed up.\n");
  return(1);
}

void fopenerror(char *s){
  switch(errno){
  case EACCES:
    printf("%s:Permission denied!\n",s);break;
  case EISDIR:
    printf("%s is directory!\n",s);break;
  case ENOENT:
    printf("%s:No such a file!\n",s);break;
  default:
    printf("%s:Error(fffopen)! \n",s);break;
  }
  printf("Exit.\n");
  exit(-1);
}


int startup(int iarg,char *argv[],int *flag,char (*runv)[PATH_L],char *outpath){
  int i,irun=0,idop;
  char glpath[PATH_L];
  char hbpath[PATH_L];
  *flag=0;

  strcpy(glpath,"PAPA");
  strcpy(hbpath,"test.hb");

  for(i=1;i<iarg;i++){
    if(*(argv[i])=='-'){
      idop=check_option(argv[i]);
      if(idop<0){
	printf("%s:Unknown option (%s).\n",argv[0],argv[i]);
	return(-1);
      }
      *flag|=idop;
      switch(idop){
      case OPT_G:
	if(i+1<iarg && *(argv[i+1])!='-'){
	  if(strlen(argv[i+1])>=PATH_L){
	    printf("%s:Too long global section name (%s).\n",
		   argv[0],argv[i+1]);
	    return(-1);
	  }
	  strncpy(glpath,argv[i+1],PATH_L);
	  i++;
	}
	break;
      case OPT_HB:
	if(i+1<iarg && *(argv[i+1])!='-'){
	  if(strlen(argv[i+1])>=PATH_L){
	    printf("%s:Too long hbook file name (%s).\n",
		   argv[0],argv[i+1]);
	    return(-1);
	  }
	  strncpy(hbpath,argv[i+1],PATH_L);
	  i++;
	}
	break;
      case OPT_OL:
	if(i+1<iarg && *(argv[i+1])!='-'){
	  if(strlen(argv[i+1])>=PATH_L){
	    printf("%s:Too long global section name (%s).\n",
		   argv[0],argv[i+1]);
	    return(-1);
	  }
	  strncpy(glpath,argv[i+1],PATH_L);
	  i++;
	}
	break;
      }
    }
    else{
      if(strlen(argv[i])>=PATH_L){
	printf("%s:Too long input file path (%s).\n",argv[0],argv[i]);
	return(-1);
      }
      if(irun==MAX_NRUN){
	printf("%s:Too many input files (NRUN=%d).\n",argv[0],irun);
	return(-1);
      }
      strncpy(runv[irun++],argv[i],PATH_L);
    }
  }
  strcpy(outpath,hbpath);
  if(*flag&OPT_OL) *flag|=OPT_G;
  if(*flag&OPT_G){
    strcpy(outpath,glpath);
    if(*flag&OPT_HB) *flag-=OPT_HB;
  }
  else *flag|=OPT_HB;
  return(irun);
}
      
