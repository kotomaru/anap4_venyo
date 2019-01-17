
#ifndef _LORLIBH
#define _LORLIBH
double lortra(double *p1, double *p2, double *b);
double scapro(double *v1, double *v2);
void unitvec(double *v1, double *v2);
double scapro4(double *p1, double *p2);
double getbeta(double *p,double *b);
double vecadd4(double *p1, double *p2, double *p3, double f1, double f2);
double vecadd(double *v1, double *v2, double *v3, double f1, double f2);
double decay2body(double *p1, double *p2, double m0, double m1, double m2);
double getkine(double *p);
double rotvec(double *v1, double *v2, int i, double thr);
#endif
