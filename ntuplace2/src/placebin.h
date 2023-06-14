#ifndef PLACEBIN_H
#define PLACEBIN_H

#include "placedb.h"

#include <vector>
using namespace std;

class CCellmoving;

class CPlaceBin 
{
	friend class CCellmoving; //by tchsu

	//Added by Jin 20060310
	friend class CTetrisLegal; 
	//@Added by Jin 20060310
public:
  CPlaceBin( CPlaceDB& );
  CPlaceBin();

  void   CreateGrid( int binNumber );
  void   CreateGrid( double binWidth );
  
  void   UpdateBinUsage();
  void   UpdateBinFreeSpace();
 
  double GetNonZeroBinPercent(); 
  double GetTotalOverflowPercent( const double& util );
  double GetMaxUtil();
  int    GetBinNumberH()   { return m_binNumberH; }
  int    GetBinNumberW()   { return m_binNumberW; }
  double GetBinWidth()     { return m_binWidth;   }
  double GetBinHeight()    { return m_binHeight;  }
  
  void   ShowInfo( const double& targetUtil );
  double GetPenalty( const double& targetUtil );
  
  void   OutputBinUtil( string filename );  // for gnuplot TODO
  
private:
  void   CreateGrid();
  void   ClearBinUsage();
  double GetBinX( const int& binX )
  { return m_pDB->m_coreRgn.left + binX * m_binWidth; }
  double GetBinY( const int& binY )
  { return m_pDB->m_coreRgn.bottom + binY * m_binHeight; }
  
  CPlaceDB* m_pDB;
  double m_totalMovableArea;
  double m_binWidth;
  double m_binHeight;
  int    m_binNumberW;
  int    m_binNumberH;
 
  vector< vector< double > > m_binSpace;
  vector< vector< double > > m_binUsage;
  
};


#endif
