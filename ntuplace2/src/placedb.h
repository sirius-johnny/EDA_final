#ifndef PLACEDB_H
#define PLACEDB_H
/**
 *
@author Indark#eda
Placement Database
2005-10-25
*/

#include <cstdio>
#include <vector>
#include <string>
#include <map>
#include <iostream>
#include <cassert>
#include <set>
using namespace std;
 

#include "BinPacking.h"
#include "util.h"



class CParserLEFDEF;

extern double programStartTime;
extern CParserLEFDEF parserLEFDEF;

enum ORIENT{
	OR_N,OR_W,OR_S,OR_E,OR_FN,OR_FW,OR_FS,OR_FE
	};


class CSiteRow
{
public:
    static bool Lesser( const CSiteRow& r1, const CSiteRow& r2  ) 
    { 
        return ( r1.m_bottom < r2.m_bottom );
    }
    static bool Greater( const CSiteRow& r1, const CSiteRow& r2  ) 
    { 
        return ( r1.m_bottom > r2.m_bottom );
    }


public:
    bool isInside( const double& x, const double width )
    {
        vector<double>::const_iterator ite;
        for( ite=m_interval.begin(); ite!=m_interval.end(); ite+=2 )
        {
            if( *ite > x )
                return false;
            //cout << "  sites(" << *ite << " " << *(ite+1) << ") ";
            if( *ite <= x && *(ite+1) >= x )
                return true;
        }
        return false;
    }

public:
	CSiteRow(void);
	~CSiteRow(void);
	CSiteRow(const double& set_bottom, const double& set_height, const double& set_step ) :
		m_bottom( set_bottom ), m_height( set_height ), m_step( set_step ){}
	
	double m_bottom;        // The bottom y coordinate of this row of sites
	double m_height;        // The height of this row of sites
	double m_step;		//The minimum x step of row.	by indark

	std::vector<double> m_interval;
};




class Pin
{
    public:
	Pin() {}

	Pin( const string& name, const float& x, const float& y ) 
	    : pinName(name), xOff(x), yOff(y) {}   // 2005-08-29
	Pin( const float& x, const float& y ) 
	    : xOff(x), yOff(y) {}

	string	pinName;
	int	moduleId;
	float	xOff;
	float	yOff;
	float	absX;
	float	absY;
};

class Module
{
    public:
	Module()
	{
	    m_width  = -1;
	    m_height = -1;
	    m_area   = -1;
	    m_type   = -1;
	    m_orient = 0;
	    m_isDummy = false;
		m_isCluster = false;
	}
	Module( string name, float width=0, float height=0, bool isFixed=false ) 
	{
	    m_x = (float)0.0;
	    m_y = (float)0.0;
	    m_name = name;
	    m_width = width;
	    m_height = height;
	    m_area = width*height;
	    m_isFixed = isFixed;
	    m_type = -1;
	    assert( m_area >= 0 );
	    m_orient = 0;
	    m_isDummy = false;
		m_isCluster = false;
	}
	string GetName()		{ return m_name; } 
	float GetWidth()		{ return m_width; }
	float GetHeight()		{ return m_height; }
	float GetX()			{ return m_x; }
	float GetY()			{ return m_y; }
	float GetArea()			{ return m_area; }

	void CalcCenter()
	{ 
	    m_cx = m_x + (float)0.5 * m_width;
	    m_cy = m_y + (float)0.5 * m_height;
	}

	float  m_x, m_y;
	float  m_cx, m_cy;
	string m_name;
	float  m_width, m_height;
	bool   m_isFixed;	    // fixed block or pads
	bool   m_isOutCore;
	float  m_area;
	int    m_orient;	    // current orientation, 2005-10-20 (donnie)
	int    m_type;		    // module type id (in cell library)
	bool   m_isDummy;	    // 2006-03-02 donnie
	bool   m_isCluster;     // 2006-03-20 tchsu

	vector<int> m_pinsId;
	vector<int> m_netsId;

	bool isRotated();
	
};

typedef vector<int> Net;


class CPlaceDB
{

    friend class ParserBookshelf;
    friend class CBinPacking;
    friend class CLegalizer;
    friend class CNetinfo;
    friend class permutation;
    friend class CDiamondLegalizer;
    friend class CPrelegalizer;
    friend class de_Detail; // assignment detailed placer
    friend class CParserLEFDEF;
    friend class CParserIBMMix;

    public:
    CPlaceDB(  );
    ~CPlaceDB(void);
    void Init();


    // 2005-12-18 speedup #donnie
    void ReserveModuleMemory( const int& n );
    void ReserveNetMemory( const int& n );
    void ReservePinMemory( const int& n );
    
    inline void AddModule( const string& name, float width, float height, const bool& isFixed );
    inline void AddModuleNetId( const int& moduleId, const int& netId );
    inline void SetModuleFixed( const int& moduleId );			    // 2005-10-20
    void SetModuleOrientation( const int& moduleId, const int& orient );    // 2005-10-20
    void SetModuleType( const int& moduleId, const int& type );		    // 2005-10-20

    inline void SetDieArea( double left, double bottom, double right, double top );
    void SetCoreRegion();    // Set core region according to the m_sites.
    inline void SetCoreRegion( double left, double bottom, double right, double top );

    inline int GetModuleId( const string& name );	// Include blocks and pads
    int GetMovableBlockNumber();

    void AddNet( Net n );
    void AddNet( set<int> n );
    int AddPin( const int& moduleId, const float& xOff, const float& yOff );
    int AddPin( const int& moduleId, const string& pinName, const float& xOff, const float& yOff );	// 2005-08-29

    // Call CreateNameMap() after reading all modules and terminals and before reading nets.
    int    CreateModuleNameMap();
    void   ClearModuleNameMap();  // Clear memory
    void   PrintModules();

    void   PrintNets();
    //void CalcChipWH();

    double CalcHPWL();       // Calc center-to-center, pin-to-pin HPWL
    double GetHPWL()	{ return m_HPWL;   }
    double GetHPWLp2p()	{ return m_HPWLp2p;   }
    double GetHPWLdensity( double util );

    double GetWidth()   { return (m_coreRgn.right-m_coreRgn.left); }
    double GetHeight()  { return (m_coreRgn.top-m_coreRgn.bottom); }

    double GetNetLength( int netId );   // Get the HPWL of a single net.

    int    GetModuleNumber() { return (int)m_modules.size(); }
    int	   GetNetNumber()    { return (int)m_nets.size(); }
    
    void OutputMatlabFigure( const char* filename );
    void OutputGnuplotFigure( const char* filename, bool withCellMove, bool showMsg)
    	{ OutputGnuplotFigureWithZoom(filename,withCellMove,showMsg,false); }
    void OutputGnuplotFigure( const char* filename, bool withCellMove) 
    	{ OutputGnuplotFigureWithZoom(filename,withCellMove,false,false); }
    void OutputGnuplotFigureWithZoom( const char* filename, bool withCellMove, bool showMsg, bool withZoom );
    void OutputGnuplotFigureWithMacroPin( const char* filename, bool withCellMove );


    double GetFixBlockArea( const double& left, const double& bottom, const double& right, const double& top );

    inline vector<int> GetModulePins( const int id );
    inline vector<int> GetModuleNets( const int& id );
    
    void CalcModuleCenter( const int& id );
    void GetModuleCenter( const int& id, double& x, double& y );
    void SetModuleLocation( const int& id, float x, float y );
    bool MoveModuleCenter( const int& id, float cx, float cy );

    // Pin ================================
    int GetPinNumber()
    {
	return (int)m_pins.size();
    }
    void CalcPinLocation( const int& pid )
    {
	m_pins[pid].absX = m_modules[m_pins[pid].moduleId].m_cx + m_pins[pid].xOff;
	m_pins[pid].absY = m_modules[m_pins[pid].moduleId].m_cy + m_pins[pid].yOff;
    }
    void CalcPinLocation( const int& pid, float cx, float cy )
    {
	m_pins[pid].absX = cx + m_pins[pid].xOff;
	m_pins[pid].absY = cy + m_pins[pid].yOff;		
    }
    void GetPinLocation( const int& pid, double &x, double &y )
    {
	x = m_pins[pid].absX;
	y = m_pins[pid].absY;
    }
    string GetPinName( int pid )
    {
	return m_pins[pid].pinName;
    }
    // ====================================

    /*void FixModuleOrientation()	// all set to !rotate	
    {
	//for( int i=0; i<(int)m_modules.size(); i++ )
	//	m_modules[i].rotate = false;
    }*/

    inline void SaveBlockLocation();
    inline void RestoreBlockLocation();
    inline void SaveBestBlockLocation();
    inline void RestoreBestBlockLocation();

    void MoveBlockToLeft( double factor );
    void MoveBlockToCenter( double factor );
    void MoveBlockToBottom( double factor );

    int m_nModules;			
    int m_nPins, m_nNets;

    void OutputGSRC( const char* filename );
    void OutputPL( const char* filename, bool setOutOrientN=false );

    // 2006-03-06
    void SetAllBlockMovable();
    

    // 2005-12-02 (donnie)
public:
    int  CalculateFixedModuleNumber();
    int  GetUsedPinNum();
    bool ModuleInCore( const int& i );
    void OutputBookshelf( const char* prefix, bool setOutOrientN=false );
    void OutputNodes( const char* filename, bool setOutOrientN=false );
private:
    void OutputAUX( const char* filename, const char* prefix );

    void OutputNets( const char* filename, bool setOutOrientN=false );
    void OutputSCL( const char* filename );
    void OutputWTS( const char* filename );
    
public:

    double m_rowHeight;			// height of the rows

//    private: 
//	by indark
public:
    map<string, int> m_moduleMapNameId;
    vector<Module> m_modules;
    vector<Module> m_modules_bak;
    vector<Module> m_modules_bak_best;
    vector<Net>    m_nets;
    vector<double> m_netsWeight;    // 2006-01-10 (donnie)
    vector<Pin> m_pins;
    vector<Pin> m_pins_bak;
    vector<Pin> m_pins_bak_best;
    double m_HPWL;					// Half-Perimeter Wirelength (center2center)
    double m_HPWLp2p;				// Half-Perimeter Wirelength (pin2pin)
    double m_totalModuleArea;			// Sum of module area
    double m_totalFreeSpace;			// 2006-03-02 (donnie)
    double m_totalMovableModuleArea;		// 2006-03-02 (donnie)
    int    m_totalMovableModuleNumber;		// 2006-03-02 (donnie)
    double m_totalMovableLargeMacroArea;	// 2006-03-13 (donnie)
    CRect  m_coreRgn;				// Core region
    CRect  m_coreRgn2;				// Core region (backup)
    CRect  m_coreRgnShrink;
    CRect  m_dieArea;


    //Added by Jin
public:
    vector<CSiteRow> m_sites;
    //@Added by Jin

    //==============(indark)===============
    void AdjustCoordinate();
    void CheckRowHeight(double row_height);
    void Align();
    //@====================================
    
    vector<CSiteRow>::iterator GetRow( double y );
    void RestoreCoreRgn(void);
    void ShowRows();
    
    //private:	//commented by indark
    void GetRegionWeightedCenter(double left, double bottom, 
	                         double right, double top, 
				 double& x, double& y);
    void GetCorePinCenter(double& x, double& y);


    int part_options [9];

public:

    //Added by Jin
    CPoint GetRegionWeightedCenter( const double &left, const double &right, 
	                            const double &bottom, const double &top );
    CPoint GetCoreCenter( void );
    void RemoveFixedBlockSite();
    //@Added by Jin

    // donnie
    bool CheckStdCellOrient();
    void ShowDBInfo();
    void ShowDensityInfo(); // 2006-03-02 (donnie)

    int CreateDummyFixedBlock();
    int realModuleNumber;
    void RemoveDummyFixedBlock();

	//Added by Jin 20060228
	double GetModuleTotalNetLength( const int& mid );
	//Remove the sites under macros (without fixing any module)
	void RemoveMacroSite();	
	//@Added by Jin 20060228
};



///////////////////////////////////////////////////////////////////////


void CPlaceDB::AddModule( const string& name, float width, float height, const bool& isFixed )
{
    assert( width >= 0 );
    assert( height >= 0 );
    m_modules.push_back( Module( name, width, height, isFixed ) );
}

void CPlaceDB::AddModuleNetId( const int& moduleId, const int& netId )
{
    m_modules[moduleId].m_netsId.push_back( netId );
}

void CPlaceDB::SetModuleFixed( const int& moduleId )	// 2005-10-20
{
    m_modules[moduleId].m_isFixed = true;
}

void CPlaceDB::SetDieArea( double left, double bottom, double right, double top )
{
    m_dieArea.left   = left;
    m_dieArea.bottom = bottom;
    m_dieArea.right  = right;
    m_dieArea.top    = top;
}

void CPlaceDB::SetCoreRegion( double left, double bottom, double right, double top )
{
    m_coreRgn.bottom = bottom;
    m_coreRgn.left   = left;
    m_coreRgn.right  = right;
    m_coreRgn.top    = top;
}

int CPlaceDB::GetModuleId( const string& name )	// Include blocks and pads
{
    map<string, int>::const_iterator ite = m_moduleMapNameId.find( name );
    if( ite == m_moduleMapNameId.end() )
	return -1;
    return ite->second;
}

vector<int> CPlaceDB::GetModulePins( int id )
{
    return m_modules[id].m_pinsId;
}

void CPlaceDB::SaveBlockLocation()
{
    m_modules_bak = m_modules;  // save block positions
    m_pins_bak = m_pins;        // save pin positions
}

void CPlaceDB::RestoreBlockLocation()
{
    m_modules = m_modules_bak;  // restore block positions
    m_pins = m_pins_bak;        // restore pin positions
}

void CPlaceDB::SaveBestBlockLocation()
{
    m_modules_bak_best = m_modules;  // save block positions
    m_pins_bak_best = m_pins;        // save pin positions
}

void CPlaceDB::RestoreBestBlockLocation()
{
    m_modules = m_modules_bak_best;  // restore block positions
    m_pins = m_pins_bak_best;        // restore pin positions
}

vector<int> CPlaceDB::GetModuleNets( const int& i )
{
    return m_modules[i].m_netsId;
}

#endif
