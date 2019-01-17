#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "lorlib.h"
#include "kinema.h"

/******************************************************/
double getkine(double *p){
  /* Calculate kinetic enegy from four momentum
    double *p ... four momentum (in)
    return value ... kinetic energy */
  double m,kine;
  m=sqrt(scapro4(p,p));
  kine=p[0]-m;
  return(kine);
}

/******************************************************/
void unitvec(double *v1, double *v2){
  /* Make unit vector 
     double *v1 ... input vector
     double *v2 ... output vector */
  double vmag;
  int i;
  vmag=sqrt(scapro(v1,v1));
  for(i=0;i<3;i++) v2[i]=v1[i]/vmag;
}

/******************************************************/
double decay2body(double *p1, double *p2, double m0, double m1, double m2){
  /* Calculate 2-body decay in rest frame of m0.
     m0 must be larger than m1 + m2.
     Assume uniform decay 
     double *p1 ... four momentum of m1
     double *p2 ... four momentum of m2
     double m0, m1, m2 ... rest mass 
     return value ... m0  
     If m0 < (m1+m2) then -m0 will be returned.*/
  double pmag,pmag2,theta,phi;
  double m02,m12,m22;
  int i;
  if(m0<m1+m2) return(-1.0*m0);
  /*  theta=drand48()*M_PI;*/
  theta=acos(1-2.0*drand48());
  phi=drand48()*2.0*M_PI;
  m02=m0*m0;
  m12=m1*m1;
  m22=m2*m2;
  pmag2=m02*m02 + m12*m12 + m22*m22- 2*(m02*m12 + m12*m22 + m22*m02);
  pmag2/=(4*m02);
  pmag=sqrt(pmag2);

  p1[3]=pmag*cos(theta);
  p1[2]=pmag*sin(theta)*sin(phi);
  p1[1]=pmag*sin(theta)*cos(phi);

  for(i=1;i<4;i++) p2[i]=-p1[i];
  p1[0]=sqrt(m12+pmag2);
  p2[0]=sqrt(m22+pmag2);
  return(m0);
}

/******************************************************/
double vecadd4(double *p1, double *p2, double *p3, double f1, double f2){
  /* Add four vector with the factors of f1 and f2.
     Namely, p3 = f1 * p1 + f2 * p2
     double *p1 ... four vector
     double *p2 ... four vector
     double *p3 ... f1 * p1 + f2 * p2
     double f1 ... factor 1
     double f2 ... factor 2
     return value ... |p3| */
  int i;
  double tp1[4],tp2[4];
  for(i=0;i<4;i++){
    tp1[i]=p1[i];
    tp2[i]=p2[i];
  }
  for(i=0;i<4;i++) p3[i]=f1*tp1[i]+f2*tp2[i];
  return(sqrt(scapro4(p3,p3)));
}

/******************************************************/
double vecadd(double *v1, double *v2, double *v3, double f1, double f2){
  /* Add three vector with the factors of f1 and f2.
     Namely, v3 = f1 * v1 + f2 * v2
     double *v1 ... three vector
     double *v2 ... three vector
     double *v3 ... f1 * v1 + f2 * v2
     double f1 ... factor 1
     double f2 ... factor 2
     return value ... |v3| */
  int i;
  double tv1[3],tv2[3];
  for(i=0;i<3;i++){
    tv1[i]=v1[i];
    tv2[i]=v2[i];
  }
  for(i=0;i<3;i++) v3[i]=f1*tv1[i]+f2*tv2[i];
  return(sqrt(scapro(v3,v3)));
}
  
/******************************************************/
double getbeta(double *p,double *b){
  /* Calculate beta from four momentum
     double *p ... four momentum (4-vec)
     double *b ... calculated beta (3-vec)
     return value ... |b| */
  double pmag;
  int i;
  for(i=0;i<3;i++) b[i]=p[i+1]/p[0];
  return(sqrt(scapro(b,b)));
}

/******************************************************/	 
double scapro(double *v1,double *v2){
/* Calculate scalar product (3dim.)
   return value ... scalar product. */
  double ans=0;
  int i;
  for(i=0;i<3;i++){
    ans+=v1[i]*v2[i];
  }
  return(ans);
}

/******************************************************/
double scapro4(double *p1,double *p2){
  /* Calculate scalar product of 4-vector (E2-p2)
     return value ... scalar product. */
  double ans;
  ans=p1[0]*p2[0]-scapro(&(p1[1]),&(p2[1]));
  return(ans);
}

/******************************************************/
double lortra(double *p1, double *p2, double *b){
/* Lorentz Transformation
 *p1 ... four vector before transformation (E,p)
 *p2 ... four vector after transformation (E,p)
 *b  ... Beta for transformation
 return value ... Invariant mass               */
  int i,j;
  double gfac,b2;
  double lormat[4][4];

  b2=scapro(b,b);
  if(b2==0){
    for(i=0;i<4;i++) p2[i]=p1[i];
  }
  else{
    gfac=1.0/sqrt(1-b2);
    
    lormat[0][0]=gfac;
    lormat[0][1]=-gfac*b[0];
    lormat[0][2]=-gfac*b[1];
    lormat[0][3]=-gfac*b[2];
    
    lormat[1][1]=1.0+(gfac-1.0)*b[0]*b[0]/b2;
    lormat[1][2]=(gfac-1.0)*b[0]*b[1]/b2;
    lormat[1][3]=(gfac-1.0)*b[0]*b[2]/b2;
    
    lormat[2][2]=1.0+(gfac-1.0)*b[1]*b[1]/b2;
    lormat[2][3]=(gfac-1.0)*b[1]*b[2]/b2;
    
    lormat[3][3]=1.0+(gfac-1.0)*b[2]*b[2]/b2;
    
    for(i=0;i<4;i++){
      for(j=i+1;j<4;j++){
	lormat[j][i]=lormat[i][j];
      }
    }
    
    for(i=0;i<4;i++){
      p2[i]=0;
      for(j=0;j<4;j++){
	p2[i]+=lormat[i][j]*p1[j];
      }
    }
  }
  return(sqrt(scapro4(p2,p2)));
}
/******************************************************/

/**************************************************************/
double rotvec(double *v1, double *v2, int i, double thr){
  /* Rotate v1 into v2 around the i-th axis by th radian */
  /*  double *v1 ... three vector (in)
      double *v2 ... three vector (out)
      int i ... Rotation axis [0,1,2] (in)
      double thr ... Rotation angle in radian (in) 
      return value ... thr
  */
  int i1,i2,i3;
  i1=(i+1)%3;
  i2=(i+2)%3;
  i3=i%3;
  v2[i1]=v1[i1]*cos(thr)-v1[i2]*sin(thr);
  v2[i2]=v1[i1]*sin(thr)+v1[i2]*cos(thr);
  v2[i3]=v1[i3];
  return(acos(scapro(v1,v2)/sqrt(scapro(v1,v1)*scapro(v2,v2))));
}
/**************************************************************/

