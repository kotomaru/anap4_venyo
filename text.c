#include <stdio.h>
#include "lorlib.h"
#include <math.h>

int main (void){
  double v[3]={1,0,0};
  double vr[3];
  double theta=M_PI;
  int i;
  rotvec(v,vr,0,theta);
  for(i=0;i<3;i++){
    printf("rotated vector[%d]=%8.3f \n",vr[i]);
  }
  return 0;
}
