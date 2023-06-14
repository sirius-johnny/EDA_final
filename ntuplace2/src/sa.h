//---------------------------------------------------------------------------
#ifndef sa_btreeH
#define sa_btreeH
//---------------------------------------------------------------------------
#include "fplan.h"
//---------------------------------------------------------------------------
extern double init_avg;
extern double avg_ratio;
extern double lamda;
using namespace ntueda;

double SA_Floorplan(FPlan &fp, int k, int local=0, double term_T=0.1);
double SA_Floorplan_moon(FPlan &fp, int k, int local/*=0*/, double term_T/*=0.1*/ );
double SA_Floorplan_classic(FPlan &fp, int k, int local/*=0*/, double term_T/*=0.1*/ );
double SA_Floorplan_TW(FPlan &fp, int k, int local/*=0*/, double term_T/*=0.1*/ );

double Random_Floorplan(FPlan &fp,int times);
//---------------------------------------------------------------------------
#endif
