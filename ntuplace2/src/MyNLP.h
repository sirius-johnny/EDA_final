// Copyright (C) 2004, 2005 International Business Machines and others.
// All Rights Reserved.
// This code is published under the Common Public License.
//
// $Id: MyNLP.hpp 511 2005-08-26 22:20:20Z andreasw $
//
// Authors:  Carl Laird, Andreas Waechter     IBM    2004-11-05

#ifndef __MYNLP_HPP__
#define __MYNLP_HPP__

//#include "IpTNLP.hpp"

//using namespace Ipopt;

#include "placedb.h"

#include <set>
#include <vector>
#include <string>
using namespace std;

class MyNLP 
{
public:
  MyNLP( CPlaceDB& );
  virtual ~MyNLP();

  bool get_nlp_info(int& n, int& m, int& nnz_jac_g, int& nnz_h_lag );
  bool get_bounds_info(int n, double* x_l, double* x_u, int m, double* g_l, double* g_u);
  bool get_starting_point(int n, bool init_x, double* x, bool init_z, double* z_L, double* z_U,
                          int m, bool init_lambda, double* lambda);
  bool eval_f(int n, const double* x, const double* expX, bool new_x, double& obj_value);
  bool eval_f_HPWL(int n, const double* x, const double* expX, bool new_x, double& obj_value);
  bool eval_grad_f( int n, const double* x, const double* expX, bool new_x, double* grad_f);
  
  // return true is placement is legal
  bool MySolve( double, double target_density, int currentLevel, bool noRelaxSmooth );	// solver setup
  
  int _potentialGridR;
  int m_potentialGridSize;
  double m_targetUtil;
  double target_nnb;
  bool   m_lookAheadLegalization;
  bool   m_earlyStop;
  bool   m_topLevel;
  int    m_smoothR;
  bool   m_lastNLP;
  bool   m_useBellPotentialForPreplaced;
  double m_smoothDelta;
  
private:
  bool GoSolve( double, double target_density, int currentLevel );	// real solver
  
  /**@name Methods to block default compiler methods.
   * The compiler automatically generates the following three methods.
   *  Since the default compiler implementation is generally not what
   *  you want (for all but the most simple classes), we usually 
   *  put the declarations of these methods in the private section
   *  and never implement them. This prevents the compiler from
   *  implementing an incorrect "default" behavior without us
   *  knowing. (See Scott Meyers book, "Effective C++")
   *  
   */
  //@{
  //  MyNLP();
  MyNLP(const MyNLP&);
  MyNLP& operator=(const MyNLP&);
  //@}


  CPlaceDB* m_pDB;
  vector< pair<int,int> > _cellPair;
  void UpdateBlockPosition( const double* x );
  void BoundX( const int& n, double* x, double* x_l, double* x_h );
  inline void BoundX( const int& n, double* x, double* x_l, double* x_h, const int& i );
  void LineSearch( const int& n, /*const*/ double* x, double* grad_f, double& stepSize );
  void AdjustForce( const int& n, const double* x, double* grad_f );
  void AdjustForce( const int& n, const double* x, vector<double> grad_wl, vector<double> grad_potential );
  void FindBeta( const int& n, const double* grad_f, const double* last_grad_f, double& beta );
  double _weightWire;
  double _weightDensity;
  double* x; 
  double* xBest;   // look ahead legalization 
  double* x_l;	   // lower bound
  double* x_u;     // upper bound
  double* _expX;   // exp(x)
  double* _expPins;
  double* xBak;
  double* xBak2;
  vector<double> grad_wire;
  vector<double> grad_potential;
  int m_ite;
  double m_currentStep;
  
public:
  double m_incFactor;
  double m_weightWire;

private:
  // wirelength related functions
  void calc_sum_exp_using_pin(
          const vector<int>::const_iterator& begin, const vector<int>::const_iterator& end,
          const double* x, const double* expX,
          double& sum_exp_xi_over_alpha, double& sum_exp_inv_xi_over_alpha,
          double& sum_exp_yi_over_alpha, double& sum_exp_inv_yi_over_alpha, int id=-1 );
  double GetWL( const int& n, const double* x, const double* expX, const double& alpha );
  void   UpdateExpValueForEachCell( const int& n, const double* x, double* expX, const double& alpha );
  void   UpdateExpValueForEachPin( const int& n, const double* x, double* expPins, const double& alpha );
  void   UpdateNetsSumExp( const double* x, const double* expX );

  double m_posScale;
  double _alpha;
  vector<bool> m_usePin;
  void SetUsePin(); 
  void InitModuleNetPinId();
  vector< vector<int> > m_moduleNetPinId;

  vector<double> m_nets_sum_exp_xi_over_alpha;
  vector<double> m_nets_sum_exp_yi_over_alpha;
  vector<double> m_nets_sum_exp_inv_xi_over_alpha;
  vector<double> m_nets_sum_exp_inv_yi_over_alpha;

  vector<double> m_nets_sum_p_x_pos;
  vector<double> m_nets_sum_p_y_pos;
  vector<double> m_nets_sum_p_inv_x_pos;
  vector<double> m_nets_sum_p_inv_y_pos;
  vector<double> m_nets_sum_p_x_neg;
  vector<double> m_nets_sum_p_y_neg;
  vector<double> m_nets_sum_p_inv_x_neg;
  vector<double> m_nets_sum_p_inv_y_neg;
  
  // diffusion bin
  vector< vector<double> > m_binForceX;
  vector< vector<double> > m_binForceY;
  void UpdateBinForce();
  void GetDiffusionGrad( const double* x, const int& i, double& gradX, double& gradY );
  
  // potential grid related variables/functions
  double m_alwaysOverPotential;
  vector< double > _cellPotentialNorm;		// cell potential normalization factor
  vector< vector<double> > m_gridPotential;
  double m_potentialGridWidth;
  double m_potentialGridHeight;
  double _potentialRY;
  double _potentialRX;
  //double _expPotential;
  double GetNonZeroGridPercent();
  double GetMaxPotential();
  double GetAvgPotential();
  double GetTotalOverPotential();   // 2006-03-01
  void   OutputPotentialGrid( string filename );	// for gnuplot
  void   OutputDensityGrid( string filename );		// for gnuplot
  void   PrintPotentialGrid();
  void   GetPotentialGrad( const double* x, const int& i, double& gradX, double& gradY );
  void   CreatePotentialGrid();
  void   ClearPotentialGrid();
  void   UpdatePotentialGrid( const double* x );
  void   UpdatePotentialGridBase( const double* x );	    // compute preplaced block potential
  void   SmoothPotentialBase( const double& delta );	    // 2006-03-04
  double GetGridWidth();
  void   GetGridCenter( const int& gx, const int& gy, double& x1, double& y1 );
  double GetXGrid( const int& gx );
  double GetYGrid( const int& gy );
  double GetDensityPanelty();
  double GetPotentialToGrid( const double& x1, const int& gx );         // 1D
  double GetGradPotentialToGrid( const double& x1, const int& gx );	// 1D
  void   GetClosestGrid( const double& x1, const double& y1, int& gx, int& gy );
  struct potentialStruct    
  {
      potentialStruct( const int& x, const int& y, const double& p ) 
	  : gx(x), gy(y), potential(p)
	  {}
      int gx;
      int gy;
      double potential;
  };
  void UpdateExpBinPotential( double utl );	// 2006-03-14
  vector< vector<double> > m_basePotential;	// 2006-03-03 (donnie) preplaced block potential 
  vector< vector<double> > m_binFreeSpace;	// 2006-03-16 (donnie) free space in the bin 
  vector< vector<double> > m_basePotentialOri;	// 2006-03-03 (donnie) preplaced block potential 
  vector< vector<double> > m_expBinPotential;	// 2006-03-14 (donnie) preplaced block potential 
  
  
  // bell-shaped functions
  inline double GetPotential( const double& x1, const double& x2, const double& r );
  inline double GetPotential( const double& x1, const double& x2, const double& r, const double& w );
  inline double GetGradPotential( const double& x1, const double& x2, const double& r );
  inline double GetGradPotential( const double& x1, const double& x2, const double& r, const double& w );
  inline double GetGradGradPotential( const double& x1, const double& x2, const double& r ); 

  
  // density grid related functions
  vector< vector<double> > m_gridDensity;
  vector< vector<double> > m_gridDensitySpace;	// avaliable space in the bin
  double m_alwaysOverArea;
  //double m_totalMovableModuleArea;
  //double m_totalFixedModuleArea;
  double m_totalFreeSpace;
  double m_gridDensityWidth;
  double m_gridDensityHeight;
  double m_gridDensityTarget;
  void   UpdateDensityGrid( const int& n, const double* x );
  void   UpdateDensityGridSpace( const int& n, const double* x );
  void   CheckDensityGrid();
  void   CreateDensityGrid( int nGrid );
  void   ClearDensityGrid();
  double GetDensityGridPanelty();
  double GetMaxDensity();
  double GetAvgOverDensity();
  double GetTotalOverDensity();
  double GetTotalOverDensityLB();
  double GetNonZeroDensityGridPercent();
  void   GetDensityGrad( const double* x, const int& i, double& gradX, double& gradY ); // UNUSE
  
};


#endif
