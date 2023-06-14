#include <cmath>
#include <ctime>
#include <map>
#include <set>
#include <list>
#include <fstream>
#include <algorithm>
using namespace std;

#include "placedb.h"
//#include "libhmetis.h"	// 2005-10-24 (indark,chen)
//#include "patoh.h"	        // 2005/2/2 (tung-chieh)
#include "util.h"
//#include "BinPacking.h"
//#include "FMPart.h"		
//#include "btree.h"		// B_Tree
//#include "sa.h"
#include "lefdef.h"

#include "placebin.h"


const double EXPECT_DENSITY = 1.00;

CSiteRow::CSiteRow(void)
: m_bottom(0)
, m_height(0)
{
}

CSiteRow::~CSiteRow(void)
{
}


/////////////////////////////////////////////////////

bool Module::isRotated()
{
    return ( (m_orient % 2) == 1 );	// E, W, FE, FW
}

/////////////////////////////////////////////////////

CPlaceDB::CPlaceDB( ) 
{

}

CPlaceDB::~CPlaceDB(void)
{
}

void CPlaceDB::Init()
{
    //---------------
    m_nModules = m_modules.size();
    m_nNets    = m_nets.size();
    m_nPins    = m_pins.size();
    //printf( "Module # = %d\n", m_nModules );
    //printf( "Net # = %d\n", m_nNets );
    //printf( "Pin # = %d\n", m_nPins );
    //---------------

    // 2005-12-28, 2006-03-02 add movableModuleArea & freeSpace
    m_totalModuleArea = 0;
    m_totalMovableModuleArea = 0;
    m_totalFreeSpace = (m_coreRgn.right - m_coreRgn.left) * (m_coreRgn.top - m_coreRgn.bottom);
    m_totalMovableModuleNumber = 0;
    m_totalMovableLargeMacroArea = 0;
    for( int i=0; i<(int)m_modules.size(); i++ )
    {
	m_modules[i].m_isOutCore = false;
	if( m_modules[i].m_isFixed )
	{
	   if( m_modules[i].m_cx < m_coreRgn.left || m_modules[i].m_cx > m_coreRgn.right ||
	       m_modules[i].m_cy < m_coreRgn.bottom || m_modules[i].m_cy > m_coreRgn.top )
	   {
	       m_modules[i].m_isOutCore = true;
	       continue;
	   }
	   else
	   {
		m_totalFreeSpace -= 
		    getOverlap( m_coreRgn.left, m_coreRgn.right, m_modules[i].m_x, m_modules[i].m_x + m_modules[i].m_width ) *
		    getOverlap( m_coreRgn.bottom, m_coreRgn.top, m_modules[i].m_y, m_modules[i].m_y + m_modules[i].m_height );
	   }
	}
	else
	{
	    m_totalMovableModuleNumber ++;
	    m_totalMovableModuleArea += m_modules[i].m_area;
	    if( m_modules[i].m_height > m_rowHeight * 2 )
	    {
		m_totalMovableLargeMacroArea += m_modules[i].m_area;
	    }
	}
	m_totalModuleArea += m_modules[i].m_area;
    }
   
    
    // 2005-12-27 (donnie) sort netsId in each block
    for( int i=0; i<(int)m_modules.size(); i++ )
    {
	sort( m_modules[i].m_netsId.begin(), m_modules[i].m_netsId.end() );
    }
   
    // check rows
    if( m_sites.size() == 0 )
    {
	printf( "Error, no row info in placement DB.\n" );
	exit(0);
    }
    
}

double CPlaceDB::GetFixBlockArea( const double& left, const double& bottom, 
			const double& right, const double& top )
{
    double a = 0;
    //int id;
    if( right <= left )
    {
	printf( "WARNING! right=%f left=%f\n", right, left );
    }
    assert( right > left );
    assert( top > bottom );
    //cout << "Fixed modules= " << m_partitions[partId].fixModuleList.size() << endl;
    for( int i=0; i<(int)m_modules.size(); i++ )
    {
	if( m_modules[i].m_isFixed )
	{
	    a += getOverlapArea( left, bottom, right, top, m_modules[i].m_x, m_modules[i].m_y,
		    m_modules[i].m_x+m_modules[i].m_width, m_modules[i].m_y+m_modules[i].m_height );
	}
    }

    return a;
    
    //double rgn_area = (top - bottom) * (right-left);
    //double b = 0;
    //b = rgn_area - a ;
    //b *= EXPECT_DENSITY;
    //return rgn_area - b;
}


void CPlaceDB::PrintModules()
{
    int size = (int)m_modules.size();
    for( int i=0; i<size; i++ )
    {
	    // modified by indark for missalianment 
	    printf( "Block %-15s at lb(%5g, %5g) missalign:%d center(%5g, %5g) w= %-5g h= %-5g pin # = %d\n", 
		m_modules[i].GetName().c_str(),
	        m_modules[i].GetX(),
	        m_modules[i].GetY(),
		(int)(m_modules[i].GetX()) % (int)(m_sites[0].m_step) ,	
		m_modules[i].m_cx,
		m_modules[i].m_cy,
		m_modules[i].GetWidth(), 
		m_modules[i].GetHeight(), 
	        m_modules[i].m_pinsId.size() );
	for( int j=0; j<(int)m_modules[i].m_pinsId.size(); j++ )
	{
	    int pinId = m_modules[i].m_pinsId[j];
	    printf( "   Pin rel(%5g, %5g) abs(%5g, %5g)  module_size(%5g,%5g)\n", 
		    m_pins[pinId].xOff, m_pins[pinId].yOff,
		    m_pins[pinId].absX, m_pins[pinId].absY,
		m_modules[i].GetWidth(),m_modules[i].GetHeight()
		  );
	    assert( fabs( m_pins[pinId].xOff ) <= m_modules[i].GetWidth() * 0.5 );
	    assert( fabs( m_pins[pinId].yOff ) <= m_modules[i].GetHeight() * 0.5 );
	    assert( m_pins[pinId].absX >= m_modules[i].GetX() );
	    assert( m_pins[pinId].absY >= m_modules[i].GetY() );
	}
    }
    printf( "Block # = %d\n", size );
}

int CPlaceDB::CreateModuleNameMap()
{

    // TODO: try to make it faster ("sorting")?
    
    int id = 0;
    int size = static_cast<int>(m_modules.size());
    for( int i=0; i<size; ++i )
    {
	// 2005-12-18
	if( false == (m_moduleMapNameId.insert( pair<string,int>( m_modules[i].GetName(), id )).second ) )
	{
	    cerr << "Error, block name duplicate: " << m_modules[i].GetName() << endl;
	    return -1;
	}
		
	/*if( m_moduleMapNameId.find( m_modules[i].GetName() ) 
		!= m_moduleMapNameId.end() )
	{
	    cerr << "Error, block name duplicate: " << m_modules[i].GetName() << endl;
	    return -1;
	}
	m_moduleMapNameId[name] = id;*/
	
	id++;
    }
    return 0;
}

void CPlaceDB::ClearModuleNameMap()
{
    m_moduleMapNameId.clear();
}

void CPlaceDB::PrintNets()
{
    int size = (int)m_nets.size();
    for( int i=0; i<size; i++ )
    {
	printf( "Net %d: ", i );
	for( int j=0; j<(int)m_nets[i].size(); j++ )
	{
	    int pinId = m_nets[i][j];
#if 1
	    if( pinId >= (int)m_pins.size() )
	    {
		printf( "pin id = %d out of bound (%d)\n", pinId, m_pins.size() );
		exit(0);
	    }
#endif
	    int moduleId = m_pins[pinId].moduleId;
#if 1
	    if( moduleId >= (int)m_modules.size() )
	    {
		printf( "macro id = %d out of bound (%d)\n", moduleId, m_modules.size() );
		exit(0);
	    }
#endif
	    printf( "%s ", m_modules[moduleId].GetName().c_str() );
	}
	printf( "\n" );
    }
    printf( "Net # = %d\n", size );
}

double CPlaceDB::GetNetLength( int netId )
{
    assert( netId < (int)m_nets.size() );

    if( m_nets[netId].size() <= 1 )
	return 0.0;

    int pid = m_nets[netId][0];
    double cx, cy;
    GetPinLocation( pid, cx, cy );
    double maxX = cx;
    double minX = cx;
    double minY = cy;
    double maxY = cy;
    for( int j=1; j<(int)m_nets[netId].size(); j++ )
    {
	pid = m_nets[netId][j];
	GetPinLocation( pid, cx, cy );
	minX = min( minX, cx );
	maxX = max( maxX, cx );
	minY = min( minY, cy );
	maxY = max( maxY, cy );
    }
    return ( (maxX-minX) + (maxY-minY) );
}



double CPlaceDB::CalcHPWL()
{
    //for( int i=0; i<(int)m_modules.size(); i++ )
    //{
    //	m_modules[i].CalcCenter();
    //}

    double minX, minY, maxX, maxY;
    minX = minY = maxX = maxY = 0;
    double HPWL = 0;
    double cx, cy;
    int pid;
    bool firstPin = false;
    for( int i=0; i<(int)m_nets.size(); i++ )
    {
	firstPin = true;
	for( int j=0; j<(int)m_nets[i].size(); j++ )
	{

	    pid = m_nets[i][j];
	    if( pid >= (int)m_pins.size() )
	    {
		printf( "ERROR net[%d][%d] pid=%d pin.size()=%d\n net#=%d", i, j, pid, m_pins.size(), m_nets.size() );
	    }
	    assert( pid < (int)m_pins.size() );
	    assert( m_pins[pid].moduleId < (int)m_modules.size() );
	    cx = m_modules[ m_pins[pid].moduleId ].m_cx;	// block center
	    cy = m_modules[ m_pins[pid].moduleId ].m_cy;	// block center

	    //cout << "CalcHPWL(): BlockCenter: pid=" << pid 
	    //	<< " moduleId=" << m_pins[pid].moduleId 
	    //	<< " x=" << cx << " y=" << cy << endl;

	    if( firstPin )
	    {
		minX = maxX = cx;
		minY = maxY = cy;
		firstPin = false;
	    }
	    else
	    {
		minX = min( minX, cx );
		maxX = max( maxX, cx );
		minY = min( minY, cy );
		maxY = max( maxY, cy );
	    }
	}

	//test code
	//		if( maxX < minX )
	//		{
	//			printf("Waring: (maxX, minX): (%f, %f)", maxX, minX );
	//			flush(cout);
	//		}
	//@test code
	assert( maxX >= minX );
	assert( maxY >= minY );

	HPWL += ( (maxX - minX) + (maxY - minY) );

    }
    m_HPWL = HPWL;	// center 2 center WL

    // Pin-to-pin WL
    HPWL = 0;
    firstPin = false;
    for( int i=0; i<(int)m_nets.size(); i++ )
    {
	HPWL += GetNetLength( i );
	//firstPin = true;
	//for( int j=0; j<(int)m_nets[i].size(); j++ )
	//{
	//	pid = m_nets[i][j];
	//	GetPinLocation( pid, cx, cy );

	//	//if( pid % 1000 == 1 )
	//	//cout << "CalcHPWL(): PinLocation: pid=" << pid << " x=" << cx << " y=" << cy << endl;

	//	if( firstPin )
	//	{
	//		minX = maxX = cx;
	//		minY = maxY = cy;
	//		firstPin = false;
	//	}
	//	else
	//	{
	//		minX = min( minX, cx );
	//		maxX = max( maxX, cx );
	//		minY = min( minY, cy );
	//		maxY = max( maxY, cy );
	//	}
	//}
	//assert( maxX >= minX );
	//assert( maxY >= minY );
	//
	//HPWL += ( (maxX - minX) + (maxY - minY) );

    }
    m_HPWLp2p = HPWL;	// pin2pinWL

	//test code
	//cout << "In CalcHPWL()" << endl;
	//cout << "m_HPWLp2p: " << m_HPWLp2p << endl;
	//cout << "HPWL: " << HPWL << endl;
	//flush(cout);
	//@test code
	
    return m_HPWLp2p;
}


/*
//void CPlaceDB::SetBBox( const int& w, const int& h )
//{
//	m_bboxWidth = w;
//	m_bboxHeight = h;
//	cout << "   BBox Width: " << m_bboxWidth << endl;
//	cout << "  BBox height: " << m_bboxHeight << endl;
//
//	// Create the top-level partition.
//	// The range is the entire bounding box.
//	// All modules are in the partition.
//	m_partitions.push_back( Partition( -1, m_bboxHeight, 0, 0, m_bboxWidth, V_CUT ) );
//	
//    m_partitions[0].peerPartitionId = 0;
//
//	// Add blocks into the top-level partition.
//	// Don't add terminals (pads)
//	for( int i=0; i<(int)m_modules.size(); i++ )
//		m_partitions[0].moduleList.push_back( i );
//
//}*/




/*
////----------------------------------------------------------------------------
//// UpdatePartition
////----------------------------------------------------------------------------
//void CPlaceDB::UpdatePartition( const int& partId )
//{
//    double totalMovableArea = 0;
//    //double totalFixedArea = m_partitions[partId].totalFixedArea;
//    double left   = m_partitions[partId].left;
//    double right  = m_partitions[partId].right;
//    double top    = m_partitions[partId].top;
//    double bottom = m_partitions[partId].bottom;
//    double cx = (left+right)*0.5;
//    double cy = (top+bottom)*0.5;
//
//
// 	set<int> setNetId;		
//    set<int> setId;
//    for( int i=0; i<(int)m_partitions[partId].moduleList.size(); i++ )
//    {
//        int id = m_partitions[partId].moduleList[i];
//        MoveModuleCenter( id, cx, cy );
//        totalMovableArea += m_modules[id].m_area;
//        setId.insert( id );
//        for( int j=0; j<(int)m_modules[id].m_netsId.size(); j++ )
//        {
//            setNetId.insert( m_modules[id].m_netsId[j] );
//        }
//    }
//
//    // Remove from "set" to "vector"
//    m_partitions[partId].netList.clear();
//    set<int>::const_iterator ite;
//    for( ite=setNetId.begin(); ite!=setNetId.end(); ite++ )
//    {
//        m_partitions[partId].netList.push_back( *ite );
//    }
//
//    m_partitions[partId].totalMovableArea = totalMovableArea;
//    //m_partitions[partId].usedArea = totalMovableArea + totalFixedArea;
//    //m_partitions[partId].freeArea = m_partitions[partId].area - totalFixedArea;
//    //m_partitions[partId].utilization = (totalMovableArea+totalFixedArea) / m_partitions[partId].area;
//
//	//if( m_partitions[partId].utilization > 1.0 )
//	//{
//	//	cout << "** Util= " << m_partitions[partId].utilization << "**\n";
//	//}
//
//
//    // TEST 0215: Find number of fixed/movable blocks outside of the region
//    CUTDIR cutDir = GetCutDirForPart( partId );
//    double mid0, mid, mid1;
//    if( cutDir == V_CUT )
//    {
//        mid = (left+right) / 2.0;
//        mid0 = (left+mid) / 2.0;
//        mid1 = (mid+right) / 2.0;
//    }
//    else
//    {
//        mid = (top+bottom) / 2.0;
//        mid0 = (bottom+mid) / 2.0;
//        mid1 = (top+mid) / 2.0;
//    }
//
//    //cout << mid0 << " " << mid << " " << mid1 << endl;
//
//    int outMove = 0;
//    int outFix = 0;
//    int unsureLine = 0;
//    bool bUnsureLine = false;
//	for( int i=0; i<(int)m_partitions[partId].netList.size(); i++ )
//	{
//        bUnsureLine = false;
//		int netId = m_partitions[partId].netList[i];
//		for( int j=0; j<(int)m_nets[netId].size(); j++ )
//		{
//			if( setId.find( m_pins[ m_nets[netId][j] ].moduleId ) == setId.end() ) 
//			{
//                // The terminal is outside of the region. 
//                if( m_modules[ m_pins[ m_nets[netId][j] ].moduleId ].m_isFixed )
//                {
//                    outFix++;
//                }
//                else 
//                {
//                    if( cutDir == V_CUT )   
//                    {
//                        // Check x-coordinate
//                        if( m_modules[ m_pins[ m_nets[netId][j] ].moduleId ].m_cx <= mid0 || 
//                            m_modules[ m_pins[ m_nets[netId][j] ].moduleId ].m_cx >= mid1 )
//                        {
//                            outFix++;
//                        }
//                        else
//                        {
//                            bUnsureLine = true;
//                            outMove++;
//                        }
//                    }
//                    else
//                    {
//                        // Check y-coordinate
//                        if( m_modules[ m_pins[ m_nets[netId][j] ].moduleId ].m_cy <= mid0 || 
//                            m_modules[ m_pins[ m_nets[netId][j] ].moduleId ].m_cy >= mid1 )
//                        {
//                            outFix++;
//                        }
//                        else
//                        {
//                            bUnsureLine = true;
//                            outMove++;         
//                        }
//                    }
//                }
//			}
//		} // For each net terminal
//
//        if( bUnsureLine == true )
//            unsureLine++;
//
//	} // For each net in the parent partition
//
//    //cout << "OutFix= " << outFix << "  outMove= " << outMove << " Ratio= " << (double)outFix/outMove << endl;
//    //cout << "ID= " << partId << "  outMove= " << outMove << " outNet= " << unsureLine 
//    //    << " totalNet= " << m_partitions[partId].netList.size() 
//    //    << " Ratio= " << (double)unsureLine/m_partitions[partId].netList.size() << endl;
//
//    //m_partitions[partId].priority = (double)unsureLine;
//    m_partitions[partId].priority = (double)unsureLine/m_partitions[partId].netList.size();
//    //m_partitions[partId].priority = (double)outMove;///m_partitions[partId].netList.size();
//
//
//}*/


//---------------------------------------------------
// Partition one level
//---------------------------------------------------

/*
//// 2005/2/21



//// 2005/2/20
//bool CPlaceDB::RefineRegions( int regionId0, int regionId1 )
//{
//    if( regionId0 == regionId1 )
//        return false;
//
//    if( regionId0 == 0 )
//        return false;
//
//    if( regionId1 == 0 )
//        return false;
//
//#if 0
//    cout << "RefineRegions: " << regionId0 << " " << regionId1 << endl;
//#endif
//
//    int part0 = regionId0;
//    int part1 = regionId1;
//    
//    double top0    = m_partitions[part0].top;
//	double bottom0 = m_partitions[part0].bottom;
//	double left0   = m_partitions[part0].left;
//	double right0  = m_partitions[part0].right;
//    double cx0     = (left0 + right0) * 0.5;
//    double cy0     = (top0 + bottom0) * 0.5;
//
//	double top1    = m_partitions[part1].top;
//	double bottom1 = m_partitions[part1].bottom;
//	double left1   = m_partitions[part1].left;
//	double right1  = m_partitions[part1].right;
//    double cx1     = (left1 + right1) * 0.5;
//    double cy1     = (top1 + bottom1) * 0.5;
//
//    double xDistance = fabs( cx1 - cx0 );
//    double yDistance = fabs( cy1 - cy0 );
//    double centerDistance = xDistance + yDistance;
//
//	//////////////////////////////////////////////////////////////
//	// prepare data strucutre
//	//////////////////////////////////////////////////////////////
//	
//    const double extraWeight = 1.0 / m_rowHeight;
//
//    //==================================================
//	// 1. create module map and initial partition
//    //===================================================
// 	map<int, int> mapId;		// moduleId -> hMetis_vetex_id
//	map<int, int> reverseMapId;	// hMetis_vetex_id -> moduleId
//
//    // total area
//	double totalBlockArea = m_partitions[part1].totalMovableArea + m_partitions[part0].totalMovableArea;
//
//#if 0
//    cout << "totalBlockArea= " << totalBlockArea << endl;
//#endif
//
//    // block #
//    int p0ModuleNumber = (int)m_partitions[part0].moduleList.size();
//    int moduleNumber = (int)m_partitions[part1].moduleList.size() +  (int)m_partitions[part0].moduleList.size();
//
//    // create the map
//	int moduleId;
//    int moduleCounter = 0;
//    for( int i=0; i<moduleNumber; i++ )
//	{
//        // mappings: vetex_id <--> moduleId
//		if( i < p0ModuleNumber )
//            moduleId = m_partitions[part0].moduleList[i]; 
//        else
//            moduleId = m_partitions[part1].moduleList[i-p0ModuleNumber]; 
//
//        mapId[ moduleId ] = i;
//		reverseMapId[ i ] = moduleId;
//		part[i] = -1;	// movable block
//        moduleWeight[i] = (int)round(m_modules[moduleId].m_area * extraWeight );
//        moduleCounter++;
//	}
//    double freeArea0 = m_partitions[part0].GetFreeArea();
//    double freeArea1 = m_partitions[part1].GetFreeArea();
//    if( freeArea0 == 0 && freeArea1 == 0 )
//    {
//        cout << "\n RefineRegions: Both freeArea are 0\n"; 
//        exit(0);
//    }
//
//	// Add two fixed nodes to each partition.
//	moduleCounter += 2;
//    assert( moduleCounter-1 < m_nModules+2 );
//	part[moduleCounter-2] = 0;	// node0 is fixed to part0
//	part[moduleCounter-1] = 1;	// node1 is fixed to part1
//    moduleWeight[moduleCounter-2] = 0;
//	moduleWeight[moduleCounter-1] = 0;
//    double dummyArea = 0;
//	if( freeArea0 > freeArea1 )
//	{
//		dummyArea = totalBlockArea * ( (freeArea0 - freeArea1) / (freeArea0 + freeArea1) );
//		moduleWeight[moduleCounter-1] = (int)round( dummyArea * extraWeight );
//	}
//	else if( freeArea0 < freeArea1 )
//	{
//		dummyArea = totalBlockArea * ( (freeArea1 - freeArea0) / (freeArea0 + freeArea1) );
//		moduleWeight[moduleCounter-2] = (int)round( dummyArea * extraWeight );
//	}
//
//#if 0
//    cout << "total nodes: " << moduleCounter << endl;
//#endif
//
//    // Calculate "unbalance" factor 
//    // CHOICE 1: The following values are (likely) better for legalizer
//    //double ub0 = g_ubfactor * 100 * (freeArea0/totalBlockArea - 0.5);
//    //double ub1 = g_ubfactor * 100 * (freeArea1/totalBlockArea - 0.5);
//    // CHOICE 2: The "real" values (seems bad for legalizer... strange...)
//    double ub0 = param.ubfactor * 100 * (freeArea0/(totalBlockArea+dummyArea) - 0.5);
//    double ub1 = param.ubfactor * 100 * (freeArea1/(totalBlockArea+dummyArea) - 0.5);
//    //cout << "ub0= " << ub0 << endl;
//    //cout << "ub1= " << ub1 << endl;    
//    int ubfactor = (int)min( ub0, ub1 );
//    if( ubfactor > 49 ) ubfactor = 49;
//    if( ubfactor < 1 )  ubfactor = 1;
//
//
//    //============================================
//	// 2. create net array
//	//============================================
//    // The max # of nets is (left_part_net_# + right_part_net_#)
//    int totalNetNumber = (int)m_partitions[part0].netList.size() + (int)m_partitions[part1].netList.size();
//	//double extraNetWeight =    500000000.0 / centerDistance / totalNetNumber;
//	double extraNetWeight =        5000000.0 / centerDistance / totalNetNumber;
//	if( extraNetWeight >= 1.0 )
//		extraNetWeight = 1.0;
//
//	int netCounter = 0;
//	int pinCounter = 0;
//	eptr[netCounter] = 0;
//	netCounter++;
//	bool insidePart;			// whole net inside the partition
//	bool outsidePart;			// whole net outside the partition
//	vector<bool> externalPin;	// Save the infomation of external/internal 
//								// pin to reduce the map lookup time
//	int netId;
//
// 	set<int> setNetId;		// For checking duplication nets.
//    int p0NetNumber = (int)m_partitions[part0].netList.size();
//	for( int i=0; i<totalNetNumber; i++ )
//	{
//        if( i < p0NetNumber )
//    		netId = m_partitions[part0].netList[i];
//        else
//    		netId = m_partitions[part1].netList[i-p0NetNumber];
//
//        if( setNetId.find( netId ) != setNetId.end() )
//            continue;
//        setNetId.insert( netId );
//
//    	externalPin.resize( m_nets[netId].size() );
//
//        insidePart = true;
//		outsidePart = true;
//		for( int j=0; j<(int)m_nets[netId].size(); j++ )
//		{
//			if( mapId.find( m_pins[ m_nets[netId][j] ].moduleId ) != mapId.end() ) 
//			{
//				// Found in the partition, so the whole net is impossible
//				// outside the partition
//				outsidePart = false;
//				externalPin[j] = false;
//			}
//			else	
//			{
//				// Not found in the partition, so the whole net is impossible
//				// inside the partition
//				insidePart = false;
//				externalPin[j] = true;
//			}
//		}
//
//		// CASE 1:
//		// If all terminals of the net is not in the partition,
//		// we discard it.
//		if( outsidePart == true )	
//        {
//            cout << "RefineRegions: outsidePart!\n";
//            //exit(0);
//			continue;	// discard the net
//        }
//
//		// CASE 2:
//		// If all terminals of the net is in the partition,
//		// we simply take it. (No connections to fixed nodes)
//		if( insidePart == true )
//		{
//#if 0
//            cout << "Case2 ";
//#endif
//			for( int j=0; j<(int)m_nets[netId].size(); j++ )
//			{
//#if 1
//                if( pinCounter >= m_nPins )
//                {
//                    cout << "Error\nnet# = " << netCounter << endl;
//                    cout << "pin# = " << pinCounter << endl;
//                    exit(0);
//                }
//#endif
//                //assert( pinCounter < m_nPins );
//				eind[pinCounter] = mapId[ m_pins[ m_nets[netId][j] ].moduleId ];
//				pinCounter++;
//			}
//			eptr[netCounter] = pinCounter;
//			netWeight[netCounter-1] = (int)round( extraNetWeight * centerDistance );
//            netCounter++;
//			continue;
//		}
//
//
//		// CASE 3: 
//		// Part of the net is inside the partition.
//#if 0
//            cout << "Case3 ";
//#endif
//
//        // Compute "segment" range: [min_value max_value]
//        double x_max_value = INT_MIN, x_min_value = INT_MAX;
//        double y_max_value = INT_MIN, y_min_value = INT_MAX;
//        for( int j=0; j<(int)m_nets[netId].size(); j++ )
//        {
//			if( externalPin[j] == false )
//				continue;	// "free" modules
//            
//			double x, y;                   // Physical coordinate
//			GetPinLocation( m_nets[netId][j], x, y );
//
//            x_max_value = max( x_max_value, x );
//            x_min_value = min( x_min_value, x );
//            y_max_value = max( y_max_value, y );
//            y_min_value = min( y_min_value, y );
//        }
//
//        int weight;
//        if( (x_max_value-x_min_value) >= xDistance && (y_max_value-y_min_value) >= yDistance )   
//        {                                           // across section , dont't care
//            continue;
//        }
//        else
//        {
//            double xDis0 = distance( cx0, x_min_value, x_max_value );
//            double yDis0 = distance( cy0, y_min_value, y_max_value );
//            double xDis1 = distance( cx1, x_min_value, x_max_value );
//            double yDis1 = distance( cy1, y_min_value, y_max_value );
//            weight = (int)round( extraNetWeight * ( (xDis1+yDis1) - (xDis0+yDis0) ) );
//			if( weight == 0 )
//				continue;
//
//            // Add the fixed-node at proper partition
//            if( abs(weight) > extraNetWeight * ( centerDistance * 2.0 * param.ubfactor * 0.01 ) )
//            {
//                // add pin
//                assert( pinCounter < m_nPins );
//				if( weight >= 0 )
//                {
//	    			eind[pinCounter] = moduleCounter-2;	// left fixed node
//                }
//				else
//                {
//	    			eind[pinCounter] = moduleCounter-1;	// right fixed node
//                    weight = -weight;   // take positive value
//                }
//	    		pinCounter++;
//            }
//            else
//            {
//                // The distance from two center to the segment L is
//                // similar. We don't connect the net to any fixed node.
//                // However, the net still need to be add to the graph.
//                //saveNets++;
//                weight = (int)round( extraNetWeight * ( (xDis1+yDis1) + (xDis0+yDis0) ) * 0.5) ;   // The cost is the distance.
//				if( weight == 0 )
//					continue;
//			}
//
//            //// Test range
//            //cout << "<" << mid0 << " " << mid  << " " << mid1 << "> ";
//            //cout << "[" << min_value << " " << max_value << "] ";
//            //cout << "weight = " << weight << endl;
//        }
//
//		for( int j=0; j<(int)m_nets[netId].size(); j++ )
//		{
//			if( externalPin[j] == true )
//				continue;
//
//            assert( pinCounter < m_nPins );
//			eind[pinCounter] = mapId[ m_pins[ m_nets[netId][j] ].moduleId ];
//			pinCounter++;
//
//		} // For each pin
//		eptr[netCounter] = pinCounter;
//        assert( weight >= 0 );
//    	netWeight[netCounter-1] = weight;
//
//        netCounter++;
//
//	} // For each net
//	netCounter--;
//
//#if 0
//    cout << "Complete data preparation.\n";
//#endif
//
//
//#if 0
//	cout << "\n\t Nodes: " << moduleCounter ;
//    cout << "\t Edges: " << netCounter << " ("<< saveNets <<")\t";
//    cout << "\t Pins: " << pinCounter << endl;
//#endif
//
//
//	////====================================
//	//// Check part_weight/cutsize overflow 
//	//// (TURN OFF TO SPEEDUP)
// //   //====================================
// //   int maxNodeWeight = 0;
//	//int lastMaxNodeWeight = 0;
//	//int maxCutSize = 0;
//	//int lastMaxCutSize = 0;
//	//for( int i=0; i<moduleCounter; i++ )
//	//{
//	//	maxNodeWeight += moduleWeight[i];
//	//	if( maxNodeWeight < 0 && lastMaxNodeWeight >= 0 )
// //       {
//	//		cout << "\n\n**** MakePartition(): maxNodeWeight MAY OVERFLOW!! ****\n\n";
// //           cout << "moduleCounter = " << moduleCounter << endl;
// //           cout << "freeArea0 = " << freeArea0 << endl;
// //           cout << "freeArea1 = " << freeArea1 << endl;
// //           cout << "dummyArea = " << dummyArea << endl;
// //           for( int j=0; j<moduleCounter; j++ )
// //           {
// //               cout << "(" << j << ")" << moduleWeight[j] << " ";
// //           }
// //       }
//	//	lastMaxNodeWeight = maxNodeWeight;
//	//}
//	//for( int i=0; i<netCounter; i++ )
//	//{
//	//	maxCutSize += netWeight[i];
//	//	if( maxCutSize < 0 && lastMaxCutSize >= 0 )
//	//		cout << "\n\n**** MakePartition(): maxCutSize MAY OVERFLOW!! ****\n\n";
//	//	lastMaxCutSize = maxCutSize;
//	//}
//
//	////====================================
//	//// Check hMetis inputs 
//	//// (TURN OFF TO SPEEDUP)
// //   //====================================
//	//if( moduleCounter > m_nModules+2 )
//	//{
//	//	cerr << "ERR! Part mouleCounter > #modules + 2 (" << moduleCounter << ")\n";
//	//	exit(0);
//	//}
//	//if( netCounter > m_nNets )
//	//{
//	//	cerr << "ERR! Part netCounter > #nets (" << netCounter << ")\n";
//	//	exit(0);
//	//}
// //   for( int i=0; i<moduleCounter; i++ )
// //   {
// //       if( moduleWeight[i] < 0 )
// //       {
// //           cerr << "ERR! moduleWeight[" << i << "] < 0\n";
// //           exit(0);
// //       }
// //       if( netWeight[i] < 0 )
// //       {
// //           cerr << "ERR! netWeight[" << i << "] < 0\n";
// //           exit(0);
// //       }
// //   }
// //   for( int i=0; i<netCounter; i++ )
// //   {
// //       if( eptr[i] >= m_nPins )
// //       {
// //           cerr << "ERR! Overflow: eptr[i] >= m_nPins\n";
// //           exit(0);
// //       }
// //       if( eptr[i+1] > m_nPins )
// //       {
// //           cerr << "ERR! Overflow: eptr[i+1] > m_nPins\n";
// //           exit(0);
// //       }
// //       for( int j=eptr[i]; j<eptr[i+1]; j++ )
// //       {
// //           if( eind[j] >= moduleCounter )
// //           {
// //               cerr << "ERR! eind[j] > moduleCounter\n";
// //               exit(0);
// //           }
// //       }
// //   }
//
//
//    int edgecut;
//    if( true )
//    {
//        // HMETIS
//
//        // 2005/03/12 turn off the feature since we set the default param.ubfactor = 0
//        //if( (m_partitions[regionId0].utilization > 0.9 || m_partitions[regionId1].utilization > 0.9 )
//       //     && moduleNumber < 100 )
//       //     ubfactor = 1;        
//#if 0
//        ubfactor *= 0.5;
//        if( ubfactor<1 )  ubfactor = 1;
//#endif
//
//        if( param.bRefineUseMetis )
//        {
//            HMETIS_PartRecursive( moduleCounter, 
//						        netCounter, 
//						        moduleWeight, 
//						        eptr, 
//						        eind, 
//						        netWeight,
//						        2, // 2-way
//						        ubfactor, // unbalanced factor
//						        part_options, 
//						        part, 
//						        &edgecut );
//        }
//        else
//        {
//	        //Added by Jin
//	        for( int i = 0 ; i < moduleCounter ; i++ )
//	        {
//		        if( i < p0ModuleNumber ) // movable at part0
//		        {
//			        part[i] = 0;
//		        }
//		        else if( i < moduleNumber ) // movable at part1
//		        {
//			        part[i] = 1;
//		        }
//		        else if( i == moduleCounter - 2 ) // fixed at part0
//		        {
//			        part[i] = 2;
//		        }
//		        else if( i == moduleCounter -1 ) // fixed at part1
//		        {
//			        part[i] = 3;
//		        }
//	        }
//	        //@Added by Jin
//            int result;
//            result = CFMPart::Partition( moduleCounter, 
//						        netCounter, 
//						        moduleWeight, 
//						        eptr, 
//						        eind, 
//						        netWeight,
//						        2, // 2-way
//						        ubfactor, // unbalanced factor
//						        part_options, 
//						        part, 
//						        &edgecut );
//
//            if( result > 0 )
//                return false;
//
//        }
//
//	    //HMETIS_PartKway( moduleCounter, 
//	    //					netCounter, 
//	    //					moduleWeight, 
//	    //					eptr, 
//	    //					eind, 
//	    //					netWeight,
//	    //					2, 
//	    //					5, 
//	    //					options, 
//	    //					part, 
//	    //					&edgecut );
//    }
//    else
//    {   
//        //// PaToH
//        //PaToH_Parameters args;
//        //int partWeight[2];
//        ////cout << "Init" << endl;
//        //PaToH_Initialize_Parameters( &args, PATOH_CUTPART, PATOH_SUGPARAM_QUALITY );
//
//        //// MISC
//        //args._k = 2;    // 2-way
//        ////args.outputdetail = PATOH_OD_MEDIUM;
//        //args.seed = rand_seed;
//
//        //// COARSENING
//        //args.crs_VisitOrder = 3;    //g_part_run;
//        //args.crs_alg = 6;   //g_part_run;
//
//        //// INIT
//        ////args.nofinstances = 10;
//        //args.initp_alg = 13;       //g_part_run;
//        ////args.initp_runno = 10;
//        //args.initp_refalg = 2;
//
//        //// UNCOARSENING
//        ////args.final_imbal = 0.01;
//        //args.final_imbal = ubfactor / 100.0;
//        //args.ref_alg = 2;   //g_part_run;
//
//        ////cout << "Check" << endl;
//        ////PaToH_Check_User_Parameters( &args, 63 );
//        ////cout << "Part" << endl;
//        //PaToH_Partition_with_FixCells( &args, moduleCounter, netCounter, moduleWeight, netWeight, 
//        //    eptr, eind, part, partWeight, &edgecut );
//    }
//
//    //cout << "cut= " << edgecut << endl;
//    //flush( cout );
//
//	///////////////////////////////////////////////////////////////
//	// The partition result is in "part[]"
//	///////////////////////////////////////////////////////////////
//
//	///////////////////////////////////////////////////////////////
//	// Check area overflow
//	///////////////////////////////////////////////////////////////
//    double p0BlockArea = 0;
//    double p1BlockArea = 0;    // Total movable area
//    double p0Counter = 0;
//    double p1Counter = 1;
//	for( int i=0; i<moduleCounter-2; i++ )  // Do not include the fixed nodes
//	{
//        int id = reverseMapId[i];
//#if 0
//        cout << id << " ";
//#endif
//        assert( id < (int)m_modules.size() );
//        if( part[i] == 0 )  
//        {
//            p0BlockArea += m_modules[id].m_area;
//            p0Counter++;
//        }
//		else if( part[i] == 1 ) 
//        {
//            p1BlockArea += m_modules[id].m_area;
//            p1Counter++;
//        }
//	}
//    //if( p0BlockArea > part0Area )
//    //{
//    //    cout << "s";
//    //    //cout << "Refinement overflow!\n";
//    //    //cout << "# " << p0Counter << " " << p1Counter << endl;
//    //    //cout << "p0 " << p0BlockArea << " (" << part0Area << ")\n";
//    //    //cout << "p1 " << p1BlockArea << " (" << part1Area << ")\n";
//    //    return false;
//    //}
//    //if( p1BlockArea > part1Area )
//    //{
//    //    cout << "s";
//    //    //cout << "Refinement overflow!\n";
//    //    //cout << "# " << p0Counter << " " << p1Counter << endl;
//    //    //cout << "p0 " << p0BlockArea << " (" << part0Area << ")\n";
//    //    //cout << "p1 " << p1BlockArea << " (" << part1Area << ")\n";
//    //    return false;
//    //}
//        //cout << "p0 " << p0BlockArea << " (" << part0Area << ")\n";
//        //cout << "p1 " << p1BlockArea << " (" << part1Area << ")\n";
//
//
//    //==========================
//    //CalcHPWL();     // FIX ME!
//    //=========================
//
//
//    vector<int> part0BlockList;
//    vector<int> part1BlockList;
//
//    //double preWL = GetHPWLp2p();
//    set<int> nets1;
//    for( int i=0; i<(int)m_partitions[regionId0].netList.size(); i++ )
//    {
//        nets1.insert( m_partitions[regionId0].netList[i] );
//    }
//    for( int i=0; i<(int)m_partitions[regionId1].netList.size(); i++ )
//    {
//        nets1.insert( m_partitions[regionId1].netList[i] );
//    }
//    double preWL = 0;
//    set<int>::const_iterator ite1;
//    for( ite1=nets1.begin(); ite1!=nets1.end(); ite1++ )
//        preWL += GetNetLength( *ite1 );
//
//    //=======================================================================
//    // Apply HMETIS results
//    //=======================================================================
//    for( int i=0; i<moduleNumber ; i++ )        // NOTE: moduleNumber = moduleCounter-2
//	{
//		int id = reverseMapId[i];
//		assert( id < (int)m_modules.size() );
//
//		// Move the module to the partition
//		if( part[i] == 0 )
//		{
//			MoveModuleCenter( id, cx0, cy0 );
//            part0BlockList.push_back( id );
//		}
//		else if( part[i] == 1 )
//		{
//			MoveModuleCenter( id, cx1, cy1 );
//            part1BlockList.push_back( id );
//		}
//		else
//		{
//			cout << "Error, no partition result for module " << id << endl;
//		}
//	}
//
//
//    //======================
//    //CalcHPWL();     // FIX ME!!
//    //======================
//
//
//    bool better = false;
//    //double afterWL = GetHPWLp2p();
//
//    // Since the blockList of the partition is not updated yet, we cannot use "GetPartHPWL".
//    double afterWL = 0;
//    set<int> nets;
//    for( int i=0; i<(int)part0BlockList.size(); i++ )
//    {
//        int id = part0BlockList[i];
//        for( int j=0; j<(int)m_modules[id].m_netsId.size(); j++ )
//        {
//            nets.insert( m_modules[id].m_netsId[j] );
//        }
//    }
//    for( int i=0; i<(int)part1BlockList.size(); i++ )
//    {
//        int id = part1BlockList[i];
//        for( int j=0; j<(int)m_modules[id].m_netsId.size(); j++ )
//        {
//            nets.insert( m_modules[id].m_netsId[j] );
//        }
//    }
//    set<int>::const_iterator ite;
//    for( ite=nets.begin(); ite!=nets.end(); ite++ )
//        afterWL += GetNetLength( *ite );
//
//    //cout << "old_net_size= " << nets1.size() << endl;
//    //cout << "new_net_size= " << nets.size() << endl;
//
//    //double afterWL = GetPartHPWL( regionId0 ) + GetPartHPWL( regionId1 );
//    if( afterWL < preWL )   // "Real" apply the result
//    {
//        better = true;
//        m_partitions[part0].moduleList.clear();
//        m_partitions[part1].moduleList.clear();
//        for( int i=0; i<moduleNumber ; i++ )        // NOTE: moduleNumber = moduleCounter-2
//	    {
//		    int id = reverseMapId[i];
//		    assert( id < (int)m_modules.size() );
//
//		    // Move the module to the partition
//		    if( part[i] == 0 )
//		    {
//                m_partitions[part0].moduleList.push_back( id );
//		    }
//		    else if( part[i] == 1 )
//		    {
//                m_partitions[part1].moduleList.push_back( id );
//		    }
//		    else
//		    {
//			    cout << "Error, no partition result for module " << id << endl;
//		    }
//	    }
//    }
//    //else
//    //{
//    //    cout << "bad, restore\n";
//    //}
//
//
//    //m_partitions[part0].Print();
//    UpdatePartition( part0 );
//    UpdatePartition( part1 );
//    //m_partitions[part0].Print();
//
//    if( better )
//    {
//        //cout << "wl1= " << preWL << " wl2= " << afterWL << endl;
//        cout << "-";
//        flush( cout );
//    }
//    return better;     // Refinement complete!
//}
*/



void CPlaceDB::MoveBlockToLeft( double factor )
{
    double left = m_coreRgn.left;
    for( int i=0; i<(int)m_modules.size(); i++ )
    {
        double x, y;
        if( m_modules[i].m_isFixed == false )
        {
            x = m_modules[i].m_x;
            y = m_modules[i].m_y;
            x = (x-left) * factor + left;
            SetModuleLocation( i, x, y );
        }
    }
}

void CPlaceDB::MoveBlockToBottom( double factor )
{
    double bottom = m_coreRgn.bottom;
    for( int i=0; i<(int)m_modules.size(); i++ )
    {
        double x, y;
        if( m_modules[i].m_isFixed == false )
        {
            x = m_modules[i].m_x;
            y = m_modules[i].m_y;
            y = (y-bottom) * factor + bottom;
            SetModuleLocation( i, x, y );
        }
    }
}

void CPlaceDB::MoveBlockToCenter( double factor )
{
    double center = 0.5 * (m_coreRgn.left + m_coreRgn.right);
    for( int i=0; i<(int)m_modules.size(); i++ )
    {
        double x, y;
        if( m_modules[i].m_isFixed == false )
        {
            x = m_modules[i].m_x;
            y = m_modules[i].m_y;
            x = (x-center) * factor + center;
            SetModuleLocation( i, x, y );
        }
    }
}

/*
//void CPlaceDB::CreateBBox( const double &ar, const double &ws )
//{
//	cout << "Create bounding box\n";
//	
//	// calc total area
//    double maxModuleLength = 0;
//	double totalArea = 0;
//	for( int i=0; i<(int)m_modules.size(); i++ )
//	{
//		totalArea += m_modules[i].GetArea();
//        maxModuleLength = max ( maxModuleLength, m_modules[i].m_width );
//        maxModuleLength = max ( maxModuleLength, m_modules[i].m_height );
//	}
//
//    if( ar > 0 )
//    {
//	    // Igor
//	    m_bboxWidth = (int) sqrt( (1.0 + ws) * (double)totalArea * ar );
//	    m_bboxHeight = (int) sqrt( (1.0 + ws) * (double)totalArea / ar );
//    	
//	    // Mine
//	    //m_bboxWidth = (int) sqrt( (double)totalArea * ar / (1.0 - ws) );
//	    //m_bboxHeight = (int) sqrt( (double)totalArea / ar / (1.0 - ws) );
//    }
//
//    if( m_bboxHeight < maxModuleLength )
//    {
//        //m_bboxHeight = maxModuleLength;// + 1;
//        //m_bboxWidth = int( (1.0 + ws) * totalArea / m_bboxHeight );
//        m_bboxWidth = maxModuleLength;// + 1;
//        m_bboxHeight = int( (1.0 + ws) * totalArea / m_bboxWidth );
//    }
//	
//	cout << "   Total area: " << totalArea << endl;
//	cout << " Aspect ratio: " << ar << endl;
//	cout << "       Max WS: " << ws << endl;
//	cout << "   BBox Width: " << m_bboxWidth << endl;
//	cout << "  BBox height: " << m_bboxHeight << endl;
//	
//	// Create the top-level partition.
//	// The range is the entire bounding box.
//	// All modules are in the partition.
//	m_partitions.push_back( Partition( -1, m_bboxHeight, 0, 0, m_bboxWidth, V_CUT ) );
//	
//    m_partitions[0].peerPartitionId = 0;
//
//    // Add blocks into the top-level partition.
//	// Don't add terminals (pads)
//	for( int i=0; i<(int)m_modules.size(); i++ )
//		m_partitions[0].moduleList.push_back( i );
//}*/

//--------------------------------------------------
//
// Get cut-direction for the partition partId.
//
//--------------------------------------------------

//bool CPlaceDB::PlacePartition( int partId )
//{
//
//    if( m_partitions[partId].childPart1 > 0 && 
//        m_partitions[partId].childPart2 > 0 &&
//        m_partitions[ m_partitions[partId].childPart1 ].placed && 
//		m_partitions[ m_partitions[partId].childPart2 ].placed )
//	{
//		m_partitions[partId].placed = true;	
//		cout << "\tAlready legalized.\n";
//		return true;
//	}
//
//    if( m_partitions[ m_partitions[partId].peerPartitionId ].failLegalize )
//    {
//        cout << "Merging partitions, skip\n";
//        return false;
//    }
//
//	if( m_partitions[ partId ].moduleList.size() == 1 )
//	{
//		cout << "\tOnly 1 module, skip.\n";
//		if( m_modules[ m_partitions[ partId ].moduleList[0] ].m_x >= m_partitions[partId].left &&
//			m_modules[ m_partitions[ partId ].moduleList[0] ].m_x + m_modules[ m_partitions[ partId ].moduleList[0] ].m_width <= m_partitions[partId].right &&
//			m_modules[ m_partitions[ partId ].moduleList[0] ].m_y >= m_partitions[partId].bottom &&
//			m_modules[ m_partitions[ partId ].moduleList[0] ].m_y + m_modules[ m_partitions[ partId ].moduleList[0] ].m_height <= m_partitions[partId].top )
//		{
//			m_partitions[partId].placed = true;
//		}
//		return false;
//	}
//
//	// Place the blocks in the m_partitions[partId].
//	// The placement range: 
//	//		int m_partitions[partId].top
//	//		int m_partitions[partId].bottom
//	//		int m_partitions[partId].left 
//	//		int m_partitions[partId].right
//	// The blocks:
//	//      vector<int> m_partitions[partId].moduleList
//
//	// 2004/9/8
//	// TODO: Modify B_Tree class.
//
//	B_Tree btree;
//
//	map<int, int> mapId;	// real id --> btree id
//	int moduleCounter = 0;
//
//	// Prepare blocks
//	for( int i=0; i<(int)m_partitions[partId].moduleList.size(); i++ )
//	{
//		int id = m_partitions[partId].moduleList[i];
//		mapId[ id ] = i;
//		btree.addModule( m_modules[id].m_name.c_str(),
//						m_modules[id].m_x,
//						m_modules[id].m_y,
//						m_modules[id].m_width,
//						m_modules[id].m_height );
//		moduleCounter++;
//	}
//
//	// Prepare nets
//	bool insidePart, outsidePart;
//	for( int i=0; i<(int)m_nets.size(); i++ )
//	{
//		// pin m_nets[m][n]
//
//		insidePart = true;
//		outsidePart = true;
//		for( int j=0; j<(int)m_nets[i].size(); j++ )
//		{
//			if( mapId.find( m_nets[i][j] ) != mapId.end() ) 
//			{
//				// Found in the partition, so the whole net is impossible
//				// outside the partition
//				outsidePart = false;
//			}
//			else	
//			{
//				// Not found in the partition, so the whole net is impossible
//				// inside the partition
//				insidePart = false;
//			}
//			if( insidePart == false && outsidePart == false )
//				break;
//		}
//
//		// Case 1:
//		// If all terminals of the net is not in the partition,
//		// we discard it.
//		if( outsidePart == true )	
//			continue;	// discard the net
//
//		// Case 2:
//		// If all terminals of the net is in the partition,
//		// we simply take it.
//		if( insidePart == true )
//		{
//	        vector<int> dummyNet;
//			btree.m_nets.push_back( dummyNet );
//			vector<int>& net = btree.m_nets.back();
//			for( int j=0; j<(int)m_nets[i].size(); j++ )
//			{
//				net.push_back( mapId[m_nets[i][j]] );
//			}
//			continue;
//		}
//
//
//		// Case 3:
//		// Part of the net is inside the partition.
//        vector<int> dummyNet;
//		btree.m_nets.push_back( dummyNet );
//		vector<int>& net = btree.m_nets.back();
//		for( int j=0; j<(int)m_nets[i].size(); j++ )
//		{
//			// If the terminal is not in the partition, 
//			// add it into the map and 
//			// set to the default partition (tapping point)
//			if( mapId.find( m_nets[i][j] ) == mapId.end() )
//			{	
//				int moduleId = m_nets[i][j];
//				mapId[ moduleId ] = moduleCounter;
//
//				// Set to one fixed partition according to the
//				// physical coordinate. (Tapping points)
//				int x, y;
//
//				if( moduleId >= (int)m_modules.size() )
//				{
//					// pads
//					x = m_terminals[moduleId-(int)m_modules.size()].m_x;
//					y = m_terminals[moduleId-(int)m_modules.size()].m_y;
//					btree.addModule( m_terminals[moduleId-(int)m_modules.size()].m_name.c_str(),
//									x,
//									y,
//									0,
//									0 );
//				}
//				else
//				{
//					// blocks
//					// We don't care about the fixed blocks' "rotatation."
//					// But we need to set the correct width/height.
//					int w, h;
//					if( !m_modules[moduleId].rotate )
//					{
//						w = m_modules[moduleId].m_width;
//						h = m_modules[moduleId].m_height;
//					}
//					else
//					{
//						h = m_modules[moduleId].m_width;
//						w = m_modules[moduleId].m_height;
//					}
//
//					btree.addModule( m_modules[moduleId].m_name.c_str(),
//									m_modules[moduleId].m_x,
//									m_modules[moduleId].m_y,
//									w,
//									h );
//
//				}
//				moduleCounter++;
//	
//			}
//			net.push_back( mapId[m_nets[i][j]] );
//		} // end case 3
//
//	}
//
//	// Initial floorplan
//	// We only need to floorplan m_partitions[partId].moduleList.size()
//	// modules, not all modules. (Some of them are fixed.)
//	btree.initFPlan( (int)m_partitions[partId].moduleList.size() );
//	//btree.show_modules();
//
//	// Initial B*-tree
//	cout << "\tInit b*-tree\n";
//	btree.init();
//	btree.m_bboxWidth = m_partitions[partId].right - m_partitions[partId].left;
//	btree.m_bboxHeight = m_partitions[partId].top - m_partitions[partId].bottom;
//	btree.fixoutline_ratio = btree.m_bboxHeight / btree.m_bboxWidth;
//
//	cout << "\tw= " << btree.m_bboxWidth << "  h= " << btree.m_bboxHeight 
//		<< "\tN= " << btree.modules_N << "/" << (int)btree.modules.size() 
//		<< "\tNet#= " << (int)btree.m_nets.size() << endl;
//
//	bool feasible = false;      // got feasible floorplan?
//	int best_wire = INT_MAX;    // current best wire
//	int worst_wire = -1;
//
//    // SA parameters
//    int times = g_sa_times;
//	int local = 7;
//	double term_temp = 0.1;
//
//    // Do alpha=1 floorplanning to increase the convergence rate
//    if( g_cost_alpha < 1.0 )    
//    {
//        btree.cost_alpha = 1.0;
//    	btree.normalize_cost( 300 );
//	    cout << "\tIntitial floorplan...";
//
//        //==== NOTE ====
//        // If "times" is too small when cost_alpha=1.0,
//        // it cannot obtain feasible solution effectively.
//        times = 50; 
//        //==============
//        int counter = 0;
//        while( true )
//        {
//        
//     		btree.init();
//            SA_Floorplan_TW( btree, times, local, term_temp);
//
//            if( btree.feasible() )
//            {
//                printf( "*" );
//    	        btree.calcWireLength();           
//                if( btree.getWireLength() < best_wire )
//		        {
//                    feasible = true;
//			        printf( "!" );
//    		        best_wire = btree.getWireLength();
//			        btree.keep_best_2();
//                }
//		        if( btree.getWireLength() > worst_wire )
//		        {
//			        worst_wire = btree.getWireLength();
//		        }
//            }
//		    else
//		    {
//                //printf( "-(w%d h%d)", (int)btree.getWidth(), (int)btree.getHeight() );
//                printf( "-" );
//		    }
//
//            counter++;
//            if( counter > 10 && feasible )
//                break;
//            if( counter > 20 )
//                break;
//        } // three rounds
//
//    }
//
//    times = g_sa_times;
//
//	// debug ==================
//	//if( !btree.legal() )
//	//{
//	//	cout << "The tree is not legal!!\n";
//	//	exit(-1);
//	//}
//	// ========================
//
//	btree.cost_alpha = g_cost_alpha;
//	cout << "\n\tCalc norm cost\n";
//	btree.normalize_cost( 300 );
//
//    //cout << "=== list information ===\n";
//	//btree.show_modules();
//	//btree.list_information();
//
//
//	// Start floorplanning
//
//	int loop = 0;
//	cout << "\tStart SA...";
//	int loopMax = btree.modules_N;
//	if( loopMax > g_loopMax )	loopMax = g_loopMax;
//	while( true )
//	{
//
//		loop++;
//
//		if( loop > loopMax && feasible == true )	// enough
//		{
//			cout << "\n";
//			break;
//		}
//
//		if( loop > loopMax+g_loopMaxExtra )	        // cannot fit, more tries (2004/11/4)
//		{
//			cout << "\n\tw= " << (int)btree.getWidth() 
//				<< "  h= " << (int)btree.getHeight() << endl;
//			break;
//		}
//
//
//		btree.init();
//		//btree.normalize_cost( 300 );
//		SA_Floorplan_TW( btree, times, local, term_temp);
//
//		if( btree.feasible() )
//		{
//			feasible = true;
//			printf( "*" );
//		}
//		else
//		{
//            //printf( "-(w%d h%d)", (int)btree.getWidth(), (int)btree.getHeight() );
//            printf( "-" );
//			continue;
//		}
//
//		//cout << "\tTry " << loop << ": wire= " << btree.getWireLength();
//        // only accept feasible solution
//        if( btree.feasible() )
//        {
//    		btree.calcWireLength();
//            if( btree.getWireLength() < best_wire )
//		    {
//			    // As long as we got a feasible solution,
//			    // we can only accept feasible solution.
//			    //if( !feasible || (feasible && btree.feasible() ) )
//			    //{
//				    best_wire = btree.getWireLength();
//				    btree.keep_best_2();
//			    //}
//			    printf( "!" );
//		    }
//        }
//
//		if( btree.getWireLength() > worst_wire )
//		{
//			worst_wire = btree.getWireLength();
//		}
//
//	}
//
//	if( !feasible )
//    {
//        cout << "Fail to legalize. Merge part " 
//            << m_partitions[partId].peerPartitionId << " to parent partition "
//            << m_partitions[partId].parentPart << ".\n";
//        m_partitions[ partId ].failLegalize = true;
//        return false;
//    }
//    m_partitions[partId].placed = true;
//
//    btree.recover_best_2();
//	btree.packing();
//    
//    cout << "\tTake " << btree.calcWireLength() << endl;
//	cout << "\tWorst-best= " << worst_wire - best_wire << endl;
//    g_test_place_worst_best_sum += worst_wire - best_wire;  // for test
//
//	// Save floorplan result back to CPlaceDB
//	int xx = m_partitions[partId].left;
//	int yy = m_partitions[partId].bottom;
//	for( int i=0; i<(int)m_partitions[partId].moduleList.size(); i++ )
//	{
//		int id = m_partitions[partId].moduleList[i];
//		assert( m_modules[id].m_name == (string)btree.modules[i].name );
//		m_modules[id].m_x = xx + btree.modules_info[i].x;
//		m_modules[id].m_y = yy + btree.modules_info[i].y;
//		m_modules[id].rotate = btree.nodes[i].rotate;
//	}
//	return true;
//
//
//}

//void CPlaceDB::CalcChipWH()
//{
//	double width = 0;
//	double height = 0;
//	for( int i=0; i<(int)m_modules.size(); i++ )
//	{
//		if( !m_modules[i].rotate )
//		{
//			width = max( width, (m_modules[i].m_x+m_modules[i].m_width) );
//			height = max( height, (m_modules[i].m_y+m_modules[i].m_height) );
//		}
//		else
//		{
//			width = max( width, (m_modules[i].m_x+m_modules[i].m_height) );
//			height = max( height, (m_modules[i].m_y+m_modules[i].m_width) );
//		}
//	}
//	m_width = width;
//	m_height = height;
//}*/



//void CPlaceDB::ShowInformation()
//{
//	//CalcChipWH();
//	//m_totalModuleArea = 0;
//	//for( int i=0; i<(int)m_modules.size(); i++ )
//	//{
//	//	m_totalModuleArea += m_modules[i].m_area;
//	//}
//
//	//cout << "Width= " << m_width << endl;
//	//cout << "Height= " << m_height << endl;
//	//cout << "Dead space: " << GetDeadspace() << "%\n";
//}


//void CPlaceDB::OutputGSRC( const char* file )
//{
//	char filename[200];
//
//	// out "blocks"
//	sprintf( filename, "%s.blocks", file );
//	FILE *out;
//	out = fopen( filename, "w" );
//	fprintf( out, "UCSC blocks 1.0\n\n" );
//
//	fprintf( out, "NumSoftRectangularBlocks : 0\n" );
//	fprintf( out, "NumHardRectilinearBlocks : %d\n", m_modules.size() );
//	fprintf( out, "NumTerminals : %d\n\n", m_terminals.size() );
//	for( int i=0; i<(int)m_modules.size(); i++ )
//	{
//		fprintf( out, "%s hardrectilinear 4 (0, 0) (0, %d) (%d, %d) (%d, 0)\n", 
//			m_modules[i].m_name.c_str(), 
//			m_modules[i].m_height, 
//			m_modules[i].m_width, m_modules[i].m_height,
//			m_modules[i].m_width );
//	}
//	fprintf( out, "\n" );
//	for( int i=0; i<(int)m_terminals.size(); i++ )
//	{
//		fprintf( out, "%s terminal\n", m_terminals[i].m_name.c_str() );
//	}
//	fprintf( out, "\n\n" );
//	fclose( out );
//	cout << "Save " << filename << endl;
//
//
//	// out "nets"
//	sprintf( filename, "%s.nets", file );
//	out = fopen( filename, "w" );
//	fprintf( out, "UCLA nets 1.0\n\n" );
//
//	fprintf( out, "NumNets : %d\n", m_nNets );
//	fprintf( out, "NumPins : %d\n", m_nPins );
//	for( int i=0; i<(int)m_nets.size(); i++ )
//	{
//		fprintf( out, "NetDegree : %d\n", m_nets[i].size() );
//		for( int j=0; j<(int)m_nets[i].size(); j++ )
//		{
//			if( m_nets[i][j] < (int)m_modules.size() )
//				fprintf( out, "%s B\n", m_modules[m_nets[i][j]].m_name.c_str() );
//			else
//				fprintf( out, "%s B\n", m_terminals[m_nets[i][j]-m_modules.size()].m_name.c_str() );
//		}
//	}
//	fprintf( out, "\n\n" );
//	fclose( out );
//	cout << "Save " << filename << endl;
//
//
//	// out "pl"
//	sprintf( filename, "%s.pl", file );
//	out = fopen( filename, "w" );
//	fprintf( out, "UCLA pl 1.0\n\n" );
//
//	for( int i=0; i<(int)m_modules.size(); i++ )
//	{
//        fprintf( out, "%s\t%d\t%d : N\n", 
//			m_modules[i].m_name.c_str(), 
//			m_modules[i].m_x, m_modules[i].m_y );
//	}
//	fprintf( out, "\n" );
//	for( int i=0; i<(int)m_terminals.size(); i++ )
//	{
//        fprintf( out, "%s\t%d\t%d : N\n", 
//			m_terminals[i].m_name.c_str(), 
//			m_terminals[i].m_x, m_terminals[i].m_y );
//	}
//	fprintf( out, "\n\n" );
//	fclose( out );
//	cout << "Save " << filename << endl;
//
//}


void CPlaceDB::OutputPL( const char* file, bool setOutOrientN )
{
    printf( "Output PL file: %s\n", file );

    // out "pl"
    FILE *out = fopen( file, "w" );
    fprintf( out, "UCLA pl 1.0\n\n" );

    char* orientN = "N";

    for( int i=0; i<(int)m_modules.size(); i++ )
    {
	if( m_modules[i].m_name.substr( 0, 2 ) == "__" )
	    continue;	// skip dummy blocks

	if( strcmp( orientStr( m_modules[i].m_orient ), "BOGUS" ) == 0 )
	{
	    printf( "OutputPL: BOGUS %d ", m_modules[i].m_orient );
	} 

	char* orient;
	if( setOutOrientN )
	    orient = orientN;
	else 
	    orient = orientStr( m_modules[i].m_orient );

	fprintf( out, "%25s %15.5f %15.5f : %s\n", 
		m_modules[i].m_name.c_str(), 
		m_modules[i].m_x, 
		m_modules[i].m_y, 
		orient );
    }
    fprintf( out, "\n\n" );
    fclose( out );
}


//void CPlaceDB::UpdatePartNeighbor( int partId, double distance )
//{
//    m_partitions[partId].neighborList.clear();
//    int parentId = m_partitions[partId].parentPart;
//
//    double x1, y1, x2, y2;
//    int id, childId;
//    x1 = (m_partitions[partId].left + m_partitions[partId].right) * 0.5;
//    y1 = (m_partitions[partId].top + m_partitions[partId].bottom) * 0.5;
//
//    // Handle peerId
//    id = m_partitions[partId].peerPartitionId;
//    if( id > 0 )
//    {
//        x2 = (m_partitions[id].left + m_partitions[id].right) * 0.5;
//        y2 = (m_partitions[id].top + m_partitions[id].bottom) * 0.5;
//        if( (abs(x2-x1) + abs(y2-y1)) > distance )
//        {
//            cout << "WARNING: distance( part, peer part ) > distance\n";
//        }
//        m_partitions[partId].neighborList.push_back( id );
//    }
//
//    // Handle parent-neightbors' children
//    for( int i=0; i<(int)m_partitions[parentId].neighborList.size(); i++ )
//    {
//        id = m_partitions[parentId].neighborList[i];
//        if( m_partitions[id].childPart0 > 0 )
//        {
//            childId = m_partitions[id].childPart0;
//            x2 = (m_partitions[childId].left + m_partitions[childId].right) * 0.5;
//            y2 = (m_partitions[childId].top + m_partitions[childId].bottom) * 0.5;
//            if( (abs(x2-x1) + abs(y2-y1)) <= distance )
//                m_partitions[partId].neighborList.push_back( childId );
//        }
//        if( m_partitions[id].childPart1 > 0 )
//        {
//            childId = m_partitions[id].childPart1;
//            x2 = (m_partitions[childId].left + m_partitions[childId].right) * 0.5;
//            y2 = (m_partitions[childId].top + m_partitions[childId].bottom) * 0.5;
//            if( (abs(x2-x1) + abs(y2-y1)) <= distance )
//                m_partitions[partId].neighborList.push_back( childId );
//        }
//    }
//}

vector<CSiteRow>::iterator CPlaceDB::GetRow( double y )
{
    CSiteRow r;
    r.m_bottom = y;
    vector<CSiteRow>::iterator ite = lower_bound( m_sites.begin(), m_sites.end(), r, CSiteRow::Lesser );
    if( ite->m_bottom == y )
        return ite;
    else
        return m_sites.end();
}



void CPlaceDB::RestoreCoreRgn(void)
{
    m_coreRgn = m_coreRgn2;
}





//void CPlaceDB::GetRegionWeightedCenter(double left, double bottom, double right, double top, double& x, double& y)
//{
//    //=============================================
//    // FIX ME: Calculate weighted center by sites.
//    //=============================================
//    x = 0.5 * (left+right);
//    y = 0.5 * (top+bottom);
//}



CPoint CPlaceDB::GetRegionWeightedCenter( const double &left, const double &right,
										    const double &bottom, const double &top )
{
	//FIXME
	//row height may be discrete or unequal
	
	//return CPoint( 0.5*(left+right), 0.5*(top+bottom) ); 
	
	double site_bottom = m_sites.begin()->m_bottom;
	double unit_height = m_sites.begin()->m_height;
	
	if( (top <= site_bottom) || (bottom >= m_sites[ m_sites.size() - 1 ].m_bottom + unit_height) )
	{
		return CPoint(0, 0);
	} 
	
	int bottom_index =  static_cast<int>( floor( ( bottom - site_bottom ) / unit_height ) );
	bottom_index = static_cast<int>(max( -1, bottom_index ));
	//bottom_index = static_cast<int>(min(bottom_index, m_sites.size()-1) );
	int top_index = (int)ceil( ( top - site_bottom ) / unit_height);
	top_index = static_cast<int>(min( m_sites.size() + 1, top_index ));
	//top_index = static_cast<int>(max( 0, top_index ));
	

	
	
	double totalArea = 0;
	double sumX = 0;
	double sumY = 0;
	
	for( int i = bottom_index + 1  ; i < top_index - 1 ; i++ )
	{
		vector<double> &currentInterval = m_sites[i].m_interval;
		double effective_height = m_sites[i].m_height;
		
		
		double centerY = m_sites[i].m_bottom + ( m_sites[i].m_height / 2.0 );
		
		for( int j = 0 ; j < (signed)currentInterval.size() ; j = j+2 )
		{
			double interval_left = currentInterval[j];
			double interval_right = currentInterval[j+1];
			
			if( interval_left >= right ||
					interval_right <= left )
					continue;	
			
			double effective_left = 0;
			double effective_right = 0;
			double centerX = 0;
			
			if( interval_left >= left && interval_right <= right )
			{
				effective_left = interval_left;
				effective_right = interval_right;
			}
			else if( interval_right > right && interval_left >= left )
			{
				effective_left = interval_left;
				effective_right = right;
			}
			else if( interval_left < left && interval_right <= right )
			{
				effective_left = left;
				effective_right = interval_right;
			}
			else if( interval_left < left && interval_right > right )
			{
				effective_left = left;
				effective_right = right;
			}
			else
			{
				cerr << "Warning: Undetermined Interval Status\n";
			}			
			
			centerX = ( effective_left + effective_right ) / 2.0;
			double effective_area = ( effective_right - effective_left ) * effective_height;
			
			totalArea += effective_area;
			sumX += centerX * effective_area;
			sumY += centerY * effective_area;	
		}
		
	}
	
	// add reminder top and bottom sites
	//double bottom_reminder = m_sites[bottom_index+1].m_bottom  - bottom;
    // BUG: 2005/03/16
    double bottom_reminder = m_sites[bottom_index].m_bottom + unit_height  - bottom;

	//test code
	//printf("bottom_reminder: %f\n", bottom_reminder );
	//@test code

	// top and bottom are in different site rows
	if( top_index - bottom_index > 1 )
	{
		if( bottom_reminder > 0 && bottom_index >= 0 )
		{
			//test code
			//cout << "Have Bottom Reminder\n";
			//@test code		
			
			int i = bottom_index;
			vector<double> &currentInterval = m_sites[i].m_interval;
			double effective_height = bottom_reminder;
			
			
			double centerY = m_sites[i].m_bottom + unit_height - ( bottom_reminder / 2.0 );
	
			
			for( int j = 0 ; j < (signed)currentInterval.size() ; j = j+2 )
			{
				double interval_left = currentInterval[j];
				double interval_right = currentInterval[j+1];
				
				if( interval_left >= right ||
						interval_right <= left )
						continue;	
				
				double effective_left = 0;
				double effective_right = 0;
				double centerX = 0;
				
				if( interval_left >= left && interval_right <= right )
				{
					effective_left = interval_left;
					effective_right = interval_right;
				}
				else if( interval_right > right && interval_left >= left )
				{
					effective_left = interval_left;
					effective_right = right;
				}
				else if( interval_left < left && interval_right <= right )
				{
					effective_left = left;
					effective_right = interval_right;
				}
				else if( interval_left < left && interval_right > right )
				{
					effective_left = left;
					effective_right = right;
				}
				else
				{
					cerr << "Warning: Undetermined Interval Status\n";
				}			
				
				centerX = ( effective_left + effective_right ) / 2.0;
				double effective_area = ( effective_right - effective_left ) * effective_height;
				
				totalArea += effective_area;
				
				//test code
				//printf("totalArea: %f\n", totalArea);
				//@test code
				
				sumX += centerX * effective_area;
				sumY += centerY * effective_area;	
			}
			
			
		}
		else if( bottom_reminder < 0 )
		{
			
			cerr << "Warning: Wrong bottom_reminder: " << bottom_reminder << endl;
			cerr << "bottom: " << bottom << " bottom_index: " << bottom_index 
						<< " m_sites[bottom_index].m_bottom: " << m_sites[bottom_index].m_bottom << endl;
			cerr << "unit_height: " << unit_height << " m_sites.size(): " << (int)m_sites.size() << endl;
						
		}
		
		double top_reminder = top - m_sites[top_index-1].m_bottom ;
		
		//test code
		//printf("top_reminder: %f\n", top_reminder );
		//@test code
		
		//test code
		//printf("top_reminder: %f\n", top_reminder );
		//@test code
		
		if( top_reminder > 0 && top_index - 1 < static_cast<int>(m_sites.size()) )
		{
			//test code
			//cout << "Have Top Reminder\n";
			//@test code
			int i = top_index - 1;
			vector<double> &currentInterval = m_sites[i].m_interval;
			double effective_height = top_reminder;
			
			
                                                    // 2005/03/11: BUG FIX
			double centerY = m_sites[i].m_bottom + ( top_reminder / 2.0 );
			
			for( int j = 0 ; j < (signed)currentInterval.size() ; j = j+2 )
			{
				double interval_left = currentInterval[j];
				double interval_right = currentInterval[j+1];
				
				if( interval_left >= right ||
						interval_right <= left )
						continue;	
				
				double effective_left = 0;
				double effective_right = 0;
				double centerX = 0;
				
				if( interval_left >= left && interval_right <= right )
				{
					effective_left = interval_left;
					effective_right = interval_right;
				}
				else if( interval_right > right && interval_left >= left )
				{
					effective_left = interval_left;
					effective_right = right;
				}
				else if( interval_left < left && interval_right <= right )
				{
					effective_left = left;
					effective_right = interval_right;
				}
				else if( interval_left < left && interval_right > right )
				{
					effective_left = left;
					effective_right = right;
				}
				else
				{
					cerr << "Warning: Undetermined Interval Status\n";
				}			
				
				centerX = ( effective_left + effective_right ) / 2.0;
				double effective_area = ( effective_right - effective_left ) * effective_height;
				
				totalArea += effective_area;
				
				//test code
				//printf("totalArea: %f\n", totalArea);
				//@test code
				
				sumX += centerX * effective_area;
				sumY += centerY * effective_area;	
			}		
		}
		else if( top_reminder < 0 )
		{
			cerr << "unit_height: " << unit_height << " m_sites.size(): " << (int)m_sites.size() << endl;
			cerr << "Warning: Wrong top_reminder" << top_reminder <<  "\n";
			cerr << "top: " << top << " top_index: " << top_index 
				<< " m_sites[top_index].m_bottom: " << m_sites[top_index].m_bottom << endl; 
		}
	
	}//@top and bottom are in different site rows
	
	// top and bottom are in the same site row
	else if( top_index == bottom_index + 1 )
	{
			int i = bottom_index;

			vector<double> &currentInterval = m_sites[i].m_interval;
			double effective_height = top - bottom;
			
			if( effective_height < 0 )
			{
				cerr << "Warning: Error Effective Height\n";
			}
			
			double centerY = (top+bottom)/2.0;
			
			for( int j = 0 ; j < (signed)currentInterval.size() ; j = j+2 )
			{
				double interval_left = currentInterval[j];
				double interval_right = currentInterval[j+1];
				
				if( interval_left >= right ||
						interval_right <= left )
						continue;	
				
				double effective_left = 0;
				double effective_right = 0;
				double centerX = 0;
				
				if( interval_left >= left && interval_right <= right )
				{
					effective_left = interval_left;
					effective_right = interval_right;
				}
				else if( interval_right > right && interval_left >= left )
				{
					effective_left = interval_left;
					effective_right = right;
				}
				else if( interval_left < left && interval_right <= right )
				{
					effective_left = left;
					effective_right = interval_right;
				}
				else if( interval_left < left && interval_right > right )
				{
					effective_left = left;
					effective_right = right;
				}
				else
				{
					cerr << "Warning: Undetermined Interval Status\n";
				}			
				
				centerX = ( effective_left + effective_right ) / 2.0;
				double effective_area = ( effective_right - effective_left ) * effective_height;
				
				totalArea += effective_area;
				sumX += centerX * effective_area;
				sumY += centerY * effective_area;	
			}				
	}//@top and bottom are in the same site row
	
	else
	{
		cerr << "Warning: Undetermined top and bottom status\n";
	}
	
	double centerX, centerY;
	if( totalArea > 0 )
	{
		centerX = ( sumX / totalArea );
		centerY = ( sumY / totalArea );
	}
	else
	{
		centerX = (left + right) / 2.0;
		centerY = (top + bottom) / 2.0;	
	}
		
	//test code
//	if( (int)((top-bottom)*(right-left)) != (int)totalArea )
//	{
//		printf("*******************Begin*************\n");
//		printf("given area: %f\n", (top-bottom)*(right-left) );
//		printf("(%f, %f)\ttotalArea: %f\n", centerX, centerY, totalArea );
//		printf("bottom: (%d, %f) top: (%d, %f)\n", bottom_index, m_sites[bottom_index].m_bottom, 
//																								top_index, m_sites[top_index].m_bottom );
//		
//		printf("left: %f, right: %f, bottom: %f, top: %f\n", left, right, bottom, top );
//		printf("unit_height: %f, sites size: %d\n", unit_height, m_sites.size() );
//		printf("sites bottom: %f, sites top: %f\n", m_sites[0].m_bottom, m_sites[m_sites.size()-1].m_bottom );
//		printf("*******************End*************\n");
//		flush( cout );
//	}
	//@test code
	
	return CPoint( centerX, centerY );
		
}

CPoint CPlaceDB::GetCoreCenter( void )
{
    double sumX = 0, sumY = 0;
    int nFixedPin = 0;

    for( int i = 0 ; i < static_cast<int> (m_modules.size()) ; i++ )
    {
	if( m_modules[i].m_isFixed )
	{
	    nFixedPin += (int)m_modules[i].m_pinsId.size();
	    for( int j = 0 ; j < static_cast<int> ( m_modules[i].m_pinsId.size() ) ; j++ )
	    {
		sumX = sumX + m_modules[i].m_cx + m_pins[ m_modules[i].m_pinsId[j] ].xOff;
		sumY = sumY + m_modules[i].m_cy + m_pins[ m_modules[i].m_pinsId[j] ].yOff;
	    }
	}
    }

    if( nFixedPin == 0 )    // no pads
	return CPoint( 0.5*( m_coreRgn.top + m_coreRgn.bottom ), 0.5*( m_coreRgn.left + m_coreRgn.right ) );
    else
	return CPoint( sumX / static_cast<double>(nFixedPin), sumY / static_cast<double>(nFixedPin) );

}




void CPlaceDB::SetCoreRegion()    // Set core region according to the m_sites.
{
    // Scan m_sites to create core region bounding box
    m_coreRgn.bottom = m_sites.front().m_bottom;
    m_coreRgn.top    = m_sites.back().m_bottom + m_sites.back().m_height;
    m_coreRgn.left   = m_sites.front().m_interval.front();
    m_coreRgn.right  = m_sites.front().m_interval.back();
    //printf( "right= %g\n", m_coreRgn.right );
    for( int i=1; i<(int)m_sites.size(); i++ )
    {
	m_coreRgn.left  = min( m_coreRgn.left, m_sites[i].m_interval.front() );
	m_coreRgn.right = max( m_coreRgn.right, m_sites[i].m_interval.back() );
	//printf( "right= %g\n", m_coreRgn.right );
    }
    cout << "Set core region from site info: ";
    m_coreRgn.Print();

    // Add fixed blocks to fill "non-sites"
    //int number = CreateDummyFixedBlock();
    //cout << "Add " << number << " fixed blocks\n";

    //number = CreateDummyBlock();
    //cout << "Add " << number << " dummy blocks\n";

    // 2005/03/04
    m_coreRgn2 = m_coreRgn;
    m_coreRgnShrink = m_coreRgn;
}


void CPlaceDB::CalcModuleCenter( const int& id )
    {
	m_modules[id].m_cx = m_modules[id].m_x + m_modules[id].GetWidth() * 0.5;
	m_modules[id].m_cy = m_modules[id].m_y + m_modules[id].GetHeight() * 0.5;
    }

void CPlaceDB::GetModuleCenter( const int& id, double& x, double& y )
{
    assert( id < (int)m_modules.size() );
    x = m_modules[id].m_cx;
    y = m_modules[id].m_cy;
}

void CPlaceDB::SetModuleLocation( const int& id, float x, float y)
{
    assert( id < (int)m_modules.size() );
    m_modules[id].m_x = x;
    m_modules[id].m_y = y;
    m_modules[id].m_cx = x + m_modules[id].m_width * (float)0.5;
    m_modules[id].m_cy = y + m_modules[id].m_height * (float)0.5;
    //m_modules[id].rotate = rotate;
    for( int i=0; i<(int)m_modules[id].m_pinsId.size(); i++ )
    {
	CalcPinLocation( m_modules[id].m_pinsId[i], m_modules[id].m_cx, m_modules[id].m_cy );
    }
}

bool CPlaceDB::MoveModuleCenter( const int& id, float cx, float cy )
{
    if( m_modules[id].m_isFixed )
	return false;
    m_modules[id].m_x = cx-m_modules[id].m_width * (float)0.5;
    m_modules[id].m_y = cy-m_modules[id].m_height * (float)0.5;
    m_modules[id].m_cx = cx;
    m_modules[id].m_cy = cy;
    for( int i=0; i<(int)m_modules[id].m_pinsId.size(); i++ )
    {
	CalcPinLocation( m_modules[id].m_pinsId[i], cx, cy );
    }
    return true;
}

void CPlaceDB::ShowRows()
{
    printf( "Row information:\n" );
    for( int i=0; i<(int)m_sites.size(); i++ )
    {
	printf( "   ROW %d y %10.0f    x %10.0f -- %10.0f -- %10.0f \n", 
		i, m_sites[i].m_bottom, m_sites[i].m_interval[0], m_sites[i].m_interval[1],m_sites[i].m_height );
    }
}
    

void CPlaceDB::OutputMatlabFigure( const char* filename )
{

	FILE* out = fopen( filename, "w" );
	if( !out )
	{
		cerr << "Error, cannot open output file: " << filename << endl;
		return;
	}

	printf( "Output matlab figure: %s\n", filename );

    // output title
    fprintf( out, "\ntitle('%s, block= %d, net= %d, HPWL= %.0f')\n",
                    filename, (int)m_modules.size(), m_nNets, GetHPWL() );

	// output Dead space
	//CalcChipWH();
	//fprintf( out, "\n%% black background (deadspace)\n" ); 
    //fprintf( out, "rectangle( 'Position', [0,0,%d,%d], 'FaceColor', [0 0 0] )\n",
	//					m_width, m_height );

	// output Core region
	fprintf( out, "\n%% core region\n" ); 
	fprintf( out, "%%rectangle( 'Position', [%.2f, %.2f, %.2f, %.2f], 'LineStyle', ':' )\n",
		m_coreRgn.left, 
		m_coreRgn.bottom, 
		m_coreRgn.right-m_coreRgn.left, 
		m_coreRgn.top-m_coreRgn.bottom );

	//fprintf( out, "rectangle( 'Position', [%d,%d,%d,%d], 'FaceColor', [0.5 0.5 0.5] )\n",
	//					0, 0, m_bboxWidth, m_bboxHeight );
	//fprintf( out, "rectangle( 'Position', [%d,%d,%d,%d] )\n",
	//					0, 0, m_bboxWidth, m_bboxHeight );


	// output modules
	fprintf( out, "\n%% modules\n" ); 
	double w, h;
	for( int i=0; i<(int)m_modules.size(); i++ )
	{
		if( m_modules[i].m_name.substr( 0, 2 ) == "__" )
			continue;


		//if( !m_modules[i].rotate )
		//{
			w = m_modules[i].GetWidth();
			h = m_modules[i].GetHeight();
		//}
		//else
		//{
		//	w = m_modules[i].GetHeight();
		//	h = m_modules[i].GetWidth();
		//}
 
        if( (int)m_modules.size() < 100000 || m_modules[i].m_isFixed )
        {
            // draw movable blocks only when total module # < 100k
		    fprintf( out, "rectangle( 'Position', [%.2f, %.2f, %.2f, %.2f] )\n",
						    m_modules[i].GetX(), 
						    m_modules[i].GetY(),
						    w,
						    h );
		    fprintf( out, "line( [%.2f, %.2f], [%.2f, %.2f], 'Color', 'Black'  )\n",
							    m_modules[i].GetX() + w*0.75, 
							    m_modules[i].GetX() + w, 
							    m_modules[i].GetY() + h, 
							    m_modules[i].GetY() + h*0.5);

			//========================================================================
			// Output pin location -- unmark these if you want to output pin locations
			//------------------------------------------------------------------------
			//for( int j=0; j<(int)m_modules[i].m_pinsId.size(); j++ )
			//{
			//	double x, y;
			//	GetPinLocation( m_modules[i].m_pinsId[j], x, y );
			//	fprintf( out, "rectangle( 'Position', [%.2f, %.2f, %.2f, %.2f] )\n",
			//					x - 1.0, 
			//					y - 1.0, 
			//					2.0, 
			//					2.0 );
			//}
			//========================================================================


        }

		if( m_modules[i].m_isFixed )
		{
            double border = w < h ? w * 0.1 : h * 0.1;
			fprintf( out, "rectangle( 'Position', [%.2f, %.2f, %.2f, %.2f] )\n",
							m_modules[i].GetX() + border, 
							m_modules[i].GetY() + border,
							w - border*2,
							h - border*2 );
		}

	}

	////// output modules names
	//for( int i=0; i<(int)m_modules.size(); i++ )
	//{
	//	m_modules[i].CalcCenter();
	//	fprintf( out, "text( %f, %f, '%s', 'HorizontalAlignment', 'center', 'HorizontalAlignment', 'center' )\n",
	//					m_modules[i].m_cx, 
	//					m_modules[i].m_cy,
	//					m_modules[i].GetName().c_str() );

	//}
	
	// output terminals (pads)
	//fprintf( out, "\n%% pads (terminals)\n" ); 
	//for( int i=0; i<(int)m_terminals.size(); i++ )
	//{
	//	double x = m_terminals[i].m_x;
	//	int y = m_terminals[i].m_y;
	//	//if( x == 0 )	x = -1;
	//	//if( y == 0 )	y = -1;

	//	//fprintf( out, "rectangle( 'Position', [%d,%d,%d,%d], 'FaceColor', [1 1 1] )\n",
	//	//				x, 
	//	//				y,
	//	//				1,
	//	//				1 );

	//	//fprintf( out, "rectangle( 'Position', [%d,%d,%d,%d] )\n",
	//	//				x, 
	//	//				y,
	//	//				1,
	//	//				1 );

	//	//fprintf( out, "rectangle( 'Position', [%.1f,%.1f,%.1f,%.1f] )\n",
	//	//				x+0.1, 
	//	//				y+0.1,
	//	//				0.8,
	//	//				0.8 );


	//	//fprintf( out, "rectangle( 'Position', [%.1f,%.1f,%.1f,%.1f] )\n",
	//	//				x-0.1, 
	//	//				y-0.1,
	//	//				0.2,
	//	//				0.2 );

	//	fprintf( out, "rectangle( 'Position', [%.2f, %.2f, %.2f, %.2f] )\n",
	//					m_terminals[i].GetX(), 
	//					m_terminals[i].GetY(),
	//					m_terminals[i].GetWidth(),
	//					m_terminals[i].GetHeight() );
	//	fprintf( out, "rectangle( 'Position', [%.2f, %.2f, %.2f, %.2f] )\n",
	//					m_terminals[i].GetX()+0.1, 
	//					m_terminals[i].GetY()+0.1,
	//					m_terminals[i].GetWidth()-0.2,
	//					m_terminals[i].GetHeight()-0.2 );

	//	//fprintf( out, "line( [%d,%d], [%d,%d], 'Color', 'Black'  )\n",
	//	//					x, x+1, y, y+1 );
	//	//fprintf( out, "line( [%d,%d], [%d,%d], 'Color', 'Black'  )\n",
	//	//					x+1, x, y, y+1 );


	//}

    // 2004/11/15 output nets
	if( (int)m_nets.size() < 2000 )
	{
	    fprintf( out, "\n%% nets\n" ); 
	    for( int i=0; i<(int)m_nets.size(); i++ )
	    {
		double x1, x2, y1, y2;
		if( (int)m_nets[i].size() >= 2 )
		{
		    GetPinLocation( m_nets[i][0], x1, y1 );
		    for( int j=1; j<(int)m_nets[i].size(); j++ )
		    {
			GetPinLocation( m_nets[i][j], x2, y2 );
			fprintf( out, "line( [%.2f,%.2f], [%.2f,%.2f] )\n",
				x1, x2, y1, y2 );

		    }
		}
	    }
	}

// 	// output partitions
// 	if( (int)m_partitions.size() < 5000 )
// 	{
// 		fprintf( out, "\n%% partitons\n" ); 
// 		for( int i=0; i<(int)m_partitions.size(); i++ )
// 		{
// 
// 		    fprintf( out, "rectangle( 'Position', [%.2f, %.2f, %.2f, %.2f], 'LineStyle', ':' )\n",
// 						    m_partitions[i].left, 
// 						    m_partitions[i].bottom,
// 						    m_partitions[i].right-m_partitions[i].left,
// 						    m_partitions[i].top-m_partitions[i].bottom );
// 
// 
//    //         if( m_partitions[i].childPart1 == 0 && m_partitions[i].childPart0 == 0)
// 			//	continue;
// 
// 			//double x1, x2, y1, y2;
//    //         x1 = x2 = y1 = y2 = 0;
// 			//if( m_partitions[i].cutDir == V_CUT )
// 			//{
// 			//	y1 = m_partitions[i].top;
// 			//	y2 = m_partitions[i].bottom;
//    //             if( m_partitions[i].childPart0 != 0 )
//    //             {
// 			//	    x1 = x2 = m_partitions[ m_partitions[i].childPart0 ].right;
//    //             }
//    //             else if( m_partitions[i].childPart1 != 0 )
//    //             {
// 			//	    x1 = x2 = m_partitions[ m_partitions[i].childPart1 ].left;
//    //             }
// 
// 			//}
// 			//else if( m_partitions[i].cutDir == H_CUT )
// 			//{
// 			//	x1 = m_partitions[i].left;
// 			//	x2 = m_partitions[i].right;
//    //             if( m_partitions[i].childPart0 != 0 )
//    //             {
// 			//	    y1 = y2 = m_partitions[m_partitions[i].childPart0].top;
//    //             }
//    //             else if( m_partitions[i].childPart1 != 0 )
//    //             {
// 			//	    y1 = y2 = m_partitions[m_partitions[i].childPart1].bottom;
//    //             }
// 			//}
// 			//
// 			////                    x1 x2    y1 y2
// 			//fprintf( out, "line( [%.2f, %.2f], [%.2f, %.2f], 'LineStyle', ':' )\n",
// 			//					x1, x2, y1, y2 );
// 		}
// 	}


	fclose( out );

}


void CPlaceDB::OutputGnuplotFigureWithZoom( const char* prefix, bool withCellMove, bool showMsg, bool withZoom )
{

    CalcHPWL();
    string plt_filename,net_filename,fixed_filename,module_filename,move_filename, dummy_filename;
    FILE* pPlt;
    FILE* pNet;
    FILE* pFixed;
    FILE* pMod;
    FILE* pMove;
    FILE* pDummy;   // donnie 2006-03-02
    if(withZoom){
    	plt_filename = net_filename = fixed_filename = module_filename= move_filename = dummy_filename = prefix ;
    	plt_filename    += ".plt";
    	net_filename    += "_net.dat";
    	fixed_filename  += "_fixed.dat";
    	module_filename += "_mod.dat";
    	move_filename   += "_move.dat";
	dummy_filename  += "_dummy.dat";
    	pPlt   = fopen( plt_filename.c_str(), "w" );
    	pNet   = fopen( net_filename.c_str(), "w" );
    	pFixed = fopen( fixed_filename.c_str(), "w" );
    	pMod   = fopen( module_filename.c_str(), "w" );
    	pMove  = fopen( move_filename.c_str(), "w" );
	pDummy = fopen( dummy_filename.c_str(), "w" );
    	
    }else{
    	pPlt = fopen( prefix, "w" );
    	pNet = pFixed = pMod = pMove = pDummy = pPlt;
    	
    }

    if( !(pPlt && pNet && pFixed && pMod && pMove && pDummy) )
    {
	cerr << "Error, cannot open output file: " << prefix << endl;
	return;
    }
    

    if( showMsg )
    	if (withZoom)
    		printf( "Output gnuplot figure with prefix: %s\n", prefix );
    	else
		printf( "Output gnuplot figure: %s\n", prefix );

    // output title
    fprintf( pPlt, "\nset title \" %s, block= %d, net= %d, HPWL= %.0f \" font \"Times, 22\"\n\n",
	    prefix, (int)m_modules.size(), m_nNets, GetHPWLp2p() );

    fprintf( pPlt, "set size ratio 1\n" );
    
    if(!withZoom)
    	fprintf( pPlt, "set nokey\n\n" ); 

    //if( withCellMove && (int)m_nets.size() < 2000 )
    if (withZoom)
    	fprintf( pPlt, "plot[:][:] '%s' w l 4, '%s' w l 3, '%s' w l 1, '%s' w l 7, '%s' w l 5\n\n" ,
	   fixed_filename.c_str(), module_filename.c_str(), move_filename.c_str(),
	    net_filename.c_str(), dummy_filename.c_str() ); 
    else
	fprintf( pPlt, "plot[:][:] '-' w l 4, '-' w l 3, '-' w l 1, '-' w l 7, '-' w l 5\n\n" ); 

    // output Core region
    fprintf( pFixed, "\n# core region\n" ); 
    fprintf( pFixed, "%12.3f, %12.3f\n", m_coreRgn.left, m_coreRgn.bottom );
    fprintf( pFixed, "%12.3f, %12.3f\n", m_coreRgn.right, m_coreRgn.bottom );
    fprintf( pFixed, "%12.3f, %12.3f\n", m_coreRgn.right, m_coreRgn.top );
    fprintf( pFixed, "%12.3f, %12.3f\n", m_coreRgn.left, m_coreRgn.top ); 
    fprintf( pFixed, "%12.3f, %12.3f\n\n", m_coreRgn.left, m_coreRgn.bottom );
    fprintf( pFixed, "\n# die area\n" );
    fprintf( pFixed, "%12.3f, %12.3f\n", m_dieArea.left,  m_dieArea.bottom );
    fprintf( pFixed, "%12.3f, %12.3f\n", m_dieArea.right, m_dieArea.bottom );
    fprintf( pFixed, "%12.3f, %12.3f\n", m_dieArea.right, m_dieArea.top );
    fprintf( pFixed, "%12.3f, %12.3f\n", m_dieArea.left,  m_dieArea.top );
    fprintf( pFixed, "%12.3f, %12.3f\n\n", m_dieArea.left, m_dieArea.bottom );


    // output fixed modules
    fprintf( pFixed, "\n# fixed blocks\n" ); 
    fprintf( pFixed, "0, 0\n\n" ); 
    double x, y, w, h;
    for( int i=0; i<(int)m_modules.size(); i++ )
    {
	x = m_modules[i].GetX();
	y = m_modules[i].GetY();
	w = m_modules[i].GetWidth();
	h = m_modules[i].GetHeight();

	if( m_modules[i].m_isFixed && !m_modules[i].m_isDummy )
	{
	    fprintf( pFixed, "%12.3f, %12.3f\n", x, y );
	    fprintf( pFixed, "%12.3f, %12.3f\n", x+w, y );
	    fprintf( pFixed, "%12.3f, %12.3f\n", x+w, y+h );
	    fprintf( pFixed, "%12.3f, %12.3f\n", x, y+h ); 
	    fprintf( pFixed, "%12.3f, %12.3f\n\n", x, y );

	    fprintf( pFixed, "%12.3f, %12.3f\n", x+w*0.75, y+h );
	    fprintf( pFixed, "%12.3f, %12.3f\n\n", x+w,      y+h*0.5 );
	}
    }
    if(!withZoom)
    	fprintf( pPlt, "\nEOF\n\n" );

    // output movable modules
    fprintf( pMod, "\n# blocks\n" ); 
    fprintf( pMod, "0, 0\n\n" ); 
    for( int i=0; i<(int)m_modules.size(); i++ )
    {
	x = m_modules[i].GetX();
	y = m_modules[i].GetY();
	w = m_modules[i].GetWidth();
	h = m_modules[i].GetHeight();

	if( !m_modules[i].m_isFixed && !m_modules[i].m_isDummy )
	{
	    if( (int)m_modules.size()< 50000 || m_modules[i].m_height > m_rowHeight )
	    {
		// draw blocks
		fprintf( pMod, "%12.3f, %12.3f\n", x, y );
		fprintf( pMod, "%12.3f, %12.3f\n", x+w, y );
		fprintf( pMod, "%12.3f, %12.3f\n", x+w, y+h );
		fprintf( pMod, "%12.3f, %12.3f\n", x, y+h ); 
		fprintf( pMod, "%12.3f, %12.3f\n\n", x, y );

 		fprintf( pMod, "%12.3f, %12.3f\n", x+w*0.75, y+h );
 		fprintf( pMod, "%12.3f, %12.3f\n\n", x+w,      y+h*0.5 );
	    }
	    else
	    {
		// draw line
		fprintf( pMod, "%12.3f, %12.3f\n", x, y+h/2 );
		fprintf( pMod, "%12.3f, %12.3f\n\n", x+w, y+h/2 );
	    }
	}
    }
    if(!withZoom)
    	fprintf( pPlt, "\nEOF\n\n" );

    fprintf( pMove, "\n# cell move\n" ); 
    fprintf( pMove, "0, 0\n\n" ); 
    if( withCellMove )
    {
	// Output legalization cell shifting
	double x1, y1, x2, y2;
	for( int i=0; i<(int)m_modules.size(); i++ )
	{
	    x1 = m_modules_bak[i].m_cx;
	    y1 = m_modules_bak[i].m_cy;
	    x2 = m_modules[i].m_cx;
	    y2 = m_modules[i].m_cy;

	    if( (int)m_modules.size() < 100000 || m_modules[i].m_isFixed )
	    {
		// draw movable blocks only when total module # < 100k
		fprintf( pMove, "%12.3f, %12.3f\n", x1, y1 );
		fprintf( pMove, "%12.3f, %12.3f\n\n", x2, y2 );
	    }

	}
    }
    if(!withZoom)
    	fprintf( pPlt, "\nEOF\n\n" );

    // 2005/03/11 output nets
    fprintf( pNet, "\n# nets\n" );
    fprintf( pNet, "0, 0\n\n" ); 
    if( (int)m_nets.size() < 2000 && (int)m_nets.size() != 0 )
    {
	for( int i=0; i<(int)m_nets.size(); i++ )
	{
	    double x1, x2, y1, y2;
	    if( (int)m_nets[i].size() >= 2 )
	    {
		GetPinLocation( m_nets[i][0], x1, y1 );
		for( int j=1; j<(int)m_nets[i].size(); j++ )
		{
		    GetPinLocation( m_nets[i][j], x2, y2 );
		    fprintf( pNet, "%12.3f, %12.3f\n", x1, y1 );
		    fprintf( pNet, "%12.3f, %12.3f\n\n", x2, y2 );
		}
	    }
	}
    }
    if(!withZoom){
    	fprintf( pPlt, "\nEOF\n\n" );
    }
    

    // output dummy modules
    fprintf( pDummy, "\n# dummy modules\n" ); 
    fprintf( pDummy, "0, 0\n\n" ); 
    for( int i=0; i<(int)m_modules.size(); i++ )
    {
	x = m_modules[i].GetX();
	y = m_modules[i].GetY();
	w = m_modules[i].GetWidth();
	h = m_modules[i].GetHeight();

	if( m_modules[i].m_isDummy )
	{
	    if( (int)m_modules.size()< 50000 || m_modules[i].m_height > m_rowHeight )
	    {
		// draw blocks
		fprintf( pMod, "%12.3f, %12.3f\n", x, y );
		fprintf( pMod, "%12.3f, %12.3f\n", x+w, y );
		fprintf( pMod, "%12.3f, %12.3f\n", x+w, y+h );
		fprintf( pMod, "%12.3f, %12.3f\n", x, y+h ); 
		fprintf( pMod, "%12.3f, %12.3f\n\n", x, y );

 		fprintf( pMod, "%12.3f, %12.3f\n", x+w*0.75, y+h );
 		fprintf( pMod, "%12.3f, %12.3f\n\n", x+w,      y+h*0.5 );
	    }
	    else
	    {
		// draw line
		fprintf( pMod, "%12.3f, %12.3f\n", x, y+h/2 );
		fprintf( pMod, "%12.3f, %12.3f\n\n", x+w, y+h/2 );
	    }
	}
    }
    if(!withZoom){
    	fprintf( pPlt, "\nEOF\n\n" );
    }

    fprintf( pPlt, "pause -1 'Press any key'" );
    if(withZoom){
    	fclose( pPlt );
    	fclose( pMod );
	fclose( pFixed );
    	fclose( pMove );
	fclose( pNet );
	fclose( pDummy );
    }else{
    	fclose( pPlt );
    }

}



void CPlaceDB::OutputGnuplotFigureWithMacroPin( const char* filename, bool withCellMove )
{
    
    CalcHPWL();
    
    //double pin_width = 0.005 *( m_coreRgn.right - m_coreRgn.left);
    double pin_width = m_rowHeight * 0.5;

    FILE* out = fopen( filename, "w" );
    if( !out )
    {
	cerr << "Error, cannot open output file: " << filename << endl;
	return;
    }

    printf( "Output gnuplot figure: %s\n", filename );

    // output title
    fprintf( out, "\nset title \" %s, block= %d, net= %d, HPWL= %.0f \" font \"Times, 22\"\n\n",
	    filename, (int)m_modules.size(), m_nNets, GetHPWLp2p() );

    fprintf( out, "set nokey\n\n" ); 
    fprintf( out, "set size ratio 1\n" );

    if( withCellMove && (int)m_nets.size() < 2000 )
	fprintf( out, "plot[:][:] '-' w l 4, '-' w l 3, '-' w l 1, '-' w l 7, '-' w l 2\n\n" ); 
    else if( withCellMove )
	fprintf( out, "plot[:][:] '-' w l 4, '-' w l 3, '-' w l 1, '-' w l 2\n\n" ); 
    else if( (int)m_nets.size() < 2000 && (int)m_nets.size() != 0 )
	fprintf( out, "plot[:][:] '-' w l 4, '-' w l 3, '-' w l 7, '-' w l 2\n\n" ); 
    else
	fprintf( out, "plot[:][:] '-' w l 4, '-' w l 3, '-' w l 2\n\n" ); 

    // output Core region
    fprintf( out, "\n# core region\n" ); 
    fprintf( out, "%12.3f, %12.3f\n", m_coreRgn.left, m_coreRgn.bottom );
    fprintf( out, "%12.3f, %12.3f\n", m_coreRgn.right, m_coreRgn.bottom );
    fprintf( out, "%12.3f, %12.3f\n", m_coreRgn.right, m_coreRgn.top );
    fprintf( out, "%12.3f, %12.3f\n", m_coreRgn.left, m_coreRgn.top ); 
    fprintf( out, "%12.3f, %12.3f\n\n", m_coreRgn.left, m_coreRgn.bottom );
    fprintf( out, "\n# die area\n" );
    fprintf( out, "%12.3f, %12.3f\n", m_dieArea.left,  m_dieArea.bottom );
    fprintf( out, "%12.3f, %12.3f\n", m_dieArea.right, m_dieArea.bottom );
    fprintf( out, "%12.3f, %12.3f\n", m_dieArea.right, m_dieArea.top );
    fprintf( out, "%12.3f, %12.3f\n", m_dieArea.left,  m_dieArea.top );
    fprintf( out, "%12.3f, %12.3f\n\n", m_dieArea.left, m_dieArea.bottom );


    // output fixed modules
    fprintf( out, "\n# fixed blocks\n" ); 
    double x, y, w, h;
    double pin_x,pin_y;
    for( int i=0; i<(int)m_modules.size(); i++ )
    {
// 	if( m_modules[i].m_name.substr( 0, 2 ) == "__" )
// 	    continue;

	x = m_modules[i].GetX();
	y = m_modules[i].GetY();
	w = m_modules[i].GetWidth();
	h = m_modules[i].GetHeight();

	if( m_modules[i].m_isFixed )
	{
	    fprintf( out, "%12.3f, %12.3f\n", x, y );
	    fprintf( out, "%12.3f, %12.3f\n", x+w, y );
	    fprintf( out, "%12.3f, %12.3f\n", x+w, y+h );
	    fprintf( out, "%12.3f, %12.3f\n", x, y+h ); 
	    fprintf( out, "%12.3f, %12.3f\n\n", x, y );
	    /*for (int j = 0 ; j < m_modules[i].m_pinsId.size(); j++)
	    {
		int pin_id =  m_modules[i].m_pinsId[j];
		pin_x = this->m_pins[pin_id].absX;
		pin_y = this->m_pins[pin_id].absY;
		fprintf( out, "%12.3f, %12.3f\n",   pin_x - pin_width*0.5, pin_y - pin_width*0.5 );
		fprintf( out, "%12.3f, %12.3f\n",   pin_x - pin_width*0.5, pin_y + pin_width*0.5 );
		fprintf( out, "%12.3f, %12.3f\n",   pin_x + pin_width*0.5, pin_y + pin_width*0.5 );
		fprintf( out, "%12.3f, %12.3f\n",   pin_x + pin_width*0.5, pin_y - pin_width*0.5 );
		fprintf( out, "%12.3f, %12.3f\n\n", pin_x - pin_width*0.5, pin_y - pin_width*0.5 );
	    }*/
	}

    }
    fprintf( out, "\nEOF\n\n" );

    // output movable modules
    fprintf( out, "\n# blocks\n" ); 
    for( int i=0; i<(int)m_modules.size(); i++ )
    {
	if( m_modules[i].m_name.substr( 0, 2 ) == "__" )
	    continue;

	x = m_modules[i].GetX();
	y = m_modules[i].GetY();
	w = m_modules[i].GetWidth();
	h = m_modules[i].GetHeight();

	if( !m_modules[i].m_isFixed )
	{
	    if( (int)m_modules.size()< 50000 || m_modules[i].m_height > m_rowHeight )
	    {
		// draw blocks
		fprintf( out, "%12.3f, %12.3f\n", x, y );
		fprintf( out, "%12.3f, %12.3f\n", x+w, y );
		fprintf( out, "%12.3f, %12.3f\n", x+w, y+h );
		fprintf( out, "%12.3f, %12.3f\n", x, y+h ); 
		fprintf( out, "%12.3f, %12.3f\n\n", x, y );

		//double pin_width = 10000;

		/*if( m_modules[i].m_height > m_rowHeight * 3 )
		    for (int j = 0 ; j < m_modules[i].m_pinsId.size(); j++)
		    {
			int pin_id =  m_modules[i].m_pinsId[j];
			pin_x = this->m_pins[pin_id].absX;
			pin_y = this->m_pins[pin_id].absY;
			fprintf( out, "%12.3f, %12.3f\n",   pin_x - pin_width*0.5, pin_y - pin_width*0.5 );
			fprintf( out, "%12.3f, %12.3f\n",   pin_x - pin_width*0.5, pin_y + pin_width*0.5 );
			fprintf( out, "%12.3f, %12.3f\n",   pin_x + pin_width*0.5, pin_y + pin_width*0.5 );
			fprintf( out, "%12.3f, %12.3f\n",   pin_x + pin_width*0.5, pin_y - pin_width*0.5 );
			fprintf( out, "%12.3f, %12.3f\n\n", pin_x - pin_width*0.5, pin_y - pin_width*0.5 );
		    }*/
	    }
	    else
	    {
		// draw line
		if( true )
		{
		    fprintf( out, "%12.3f, %12.3f\n", x, y+h/2 );
		    fprintf( out, "%12.3f, %12.3f\n\n", x+w, y+h/2 );
		}
	    }
	}
    }
    fprintf( out, "\nEOF\n\n" );

    if( withCellMove )
    {
	// Output legalization cell shifting
	double x1, y1, x2, y2;
	//int mCount = 0;
	for( int i=0; i<(int)m_modules.size(); i++ )
	{
	    if( m_modules[i].m_name.substr( 0, 2 ) == "__" )
		continue;

	    x1 = m_modules_bak[i].m_cx;
	    y1 = m_modules_bak[i].m_cy;
	    x2 = m_modules[i].m_cx;
	    y2 = m_modules[i].m_cy;

	    //if( (abs(x2-x1) + abs(y2-y1)) > m_rowHeight*2 )
	    //{
	    //    mCount++;
	    //}

	    if( (int)m_modules.size() < 100000 || m_modules[i].m_isFixed )
	    {
		// draw movable blocks only when total module # < 100k
		fprintf( out, "%12.3f, %12.3f\n", x1, y1 );
		fprintf( out, "%12.3f, %12.3f\n\n", x2, y2 );
	    }

	}
	//cout << "-- move over 2 row height cell#= " << mCount << endl;
	fprintf( out, "\nEOF\n\n" );
    }


    // 2005/03/11 output nets
    if( (int)m_nets.size() < 2000 && (int)m_nets.size() != 0 )
    {
	fprintf( out, "\n# nets\n" ); 
	for( int i=0; i<(int)m_nets.size(); i++ )
	{
	    double x1, x2, y1, y2;
	    if( (int)m_nets[i].size() >= 2 )
	    {
		GetPinLocation( m_nets[i][0], x1, y1 );
		for( int j=1; j<(int)m_nets[i].size(); j++ )
		{
		    GetPinLocation( m_nets[i][j], x2, y2 );
		    fprintf( out, "%12.3f, %12.3f\n", x1, y1 );
		    fprintf( out, "%12.3f, %12.3f\n\n", x2, y2 );
		}
	    }
	}
	fprintf( out, "\nEOF\n\n" );
    }

    // 2005-12-07 output pins
    fprintf( out, "\n# pins\n" );
    fprintf( out, "\t0, 0\n\n" );
    for( int i=0; i<(int)m_modules.size(); i++ )
    {
	x = m_modules[i].GetX();
	y = m_modules[i].GetY();
	w = m_modules[i].GetWidth();
	h = m_modules[i].GetHeight();

	if( h > m_rowHeight * 2 && w <= m_rowHeight )
	{
	    cout << "WARN: " << m_modules[i].GetName() << " orient= " 
		<< orientStr( m_modules[i].m_orient ) << endl;
	}
	
	if( m_modules[i].m_isFixed || h > m_rowHeight * 2 )
	{
	    fprintf( out, "     # pin for block %s\n", m_modules[i].GetName().c_str() );
	    for (int j = 0 ; j < (int)m_modules[i].m_pinsId.size(); j++)
	    {
		int pin_id =  m_modules[i].m_pinsId[j];
		pin_x = this->m_pins[pin_id].absX;
		pin_y = this->m_pins[pin_id].absY;
		fprintf( out, "%12.3f, %12.3f\n",   pin_x - pin_width*0.5, pin_y - pin_width*0.5 );
		fprintf( out, "%12.3f, %12.3f\n",   pin_x - pin_width*0.5, pin_y + pin_width*0.5 );
		fprintf( out, "%12.3f, %12.3f\n",   pin_x + pin_width*0.5, pin_y + pin_width*0.5 );
		fprintf( out, "%12.3f, %12.3f\n",   pin_x + pin_width*0.5, pin_y - pin_width*0.5 );
		fprintf( out, "%12.3f, %12.3f\n\n", pin_x - pin_width*0.5, pin_y - pin_width*0.5 );
	    }
	}
    }
    fprintf( out, "\nEOF\n\n" );


    fprintf( out, "pause -1 'Press any key'" );
    fclose( out );

}


/*!
    \fn CPlaceDB::RemoveFixedBlockSite
 */
void CPlaceDB::RemoveFixedBlockSite()
{
    printf( "Remove core sites under fixed blocks\n" );
    
    /// @todo implement me
    double row_top =  m_sites.back().m_bottom +  m_sites.back().m_height;
    double row_bottom =  m_sites.front().m_bottom;

    for( vector<Module>::iterator iteModule =  m_modules.begin() ;
	    iteModule <  m_modules.end() ;
	    iteModule++ )
    {
	double module_top = iteModule->m_y + iteModule->m_height;
	double module_bottom = iteModule->m_y;
	double module_left = iteModule->m_x;
	double module_right = iteModule->m_x + iteModule->m_width;

	if( iteModule->m_isFixed && // Find a fixed module
		module_bottom < row_top && // Confirm that the module is overlapped with the rows in y coordinates
		module_top > row_bottom ) 
	{
	    vector<CSiteRow>::iterator iteBeginRow, iteEndRow;


	    // find the begin row
	    for( iteBeginRow =  m_sites.begin() ; 
		    iteBeginRow <  m_sites.end() ; 
		    iteBeginRow++ )
	    {

		if( iteBeginRow->m_bottom + iteBeginRow->m_height > module_bottom )
		{
		    break;
		}
		//if( iteBeginRow->m_bottom == iteModule->m_y )
		//{
		//	break;
		//}
		//else if( iteBeginRow->m_bottom > iteModule->m_y )
		//{
		//	if( iteBeginRow >  m_sites.begin() )
		//	{
		//		iteBeginRow--;
		//	}

		//	break;
		//}
	    }




	    for( iteEndRow = iteBeginRow ;
		    iteEndRow <  m_sites.end() ;
		    iteEndRow++ )
	    {
		if( iteEndRow->m_bottom + iteEndRow->m_height >= module_top )
		{
		    break;
		}
	    }

	    if( iteEndRow ==  m_sites.end() )
		iteEndRow--;


	    assert( iteBeginRow !=  m_sites.end() );

	    for( vector<CSiteRow>::iterator iteRow = iteBeginRow ;
		    iteRow <= iteEndRow ;
		    iteRow++ )
	    {
		double interval[2];
		for( int i = 0 ; i < (signed)iteRow->m_interval.size() ; i++ )
		{
		    interval[ i % 2 ] = iteRow->m_interval[i];
		    if( ( i % 2 ) == 1 ) // Get two terminals of the interval
		    {
			if( interval[0] >= module_right || interval[1] <= module_left )  // screen unnecessary checks
			    continue;

			if( interval[0] >= module_left && interval[1] <= module_right )
			{
			    iteRow->m_interval.erase( vector<double>::iterator(&(iteRow->m_interval[i])) );
			    iteRow->m_interval.erase( vector<double>::iterator(&(iteRow->m_interval[i-1])) );
			    i = i - 2;
			}
			else if( interval[1] > module_right && interval[0] >= module_left )
			{
			    iteRow->m_interval[i-1] = module_right;
			}
			else if( interval[0] < module_left && interval[1] <= module_right )
			{
			    iteRow->m_interval[i] = module_left;
			}
			else if( interval[0] < module_left && interval[1] > module_right )
			{
			    iteRow->m_interval[i] = module_left;
			    iteRow->m_interval.insert( vector<double>::iterator(&(iteRow->m_interval[i+1])), interval[1] );
			    iteRow->m_interval.insert( vector<double>::iterator(&(iteRow->m_interval[i+1])), module_right );
			    i = i + 2;

			}
			else
			{
			    printf("Warning: Module Romoving Error\n");
			    //exit(-1);
			}

		    }
		}
	    }

	    //@remove the occupied sites

	}
    }

#if 0 
	//test code
	ofstream sitefile( "sites.log" );
	for( vector<CSiteRow>::iterator iteRow = m_sites.begin() ; 
			iteRow != m_sites.end() ; iteRow++ )
	{
		sitefile << "Row bottom: " << iteRow->m_bottom << " ";

		for( unsigned int iInterval = 0 ; 
				iInterval != iteRow->m_interval.size() ; iInterval=iInterval+2 )
		{
			sitefile << "(" << iteRow->m_interval[iInterval] 
				<< "," << iteRow->m_interval[iInterval+1]  << ") ";
		}
		sitefile << endl;
	}

	sitefile.close();
	cout << "Dump site.log" << endl;
	//@test code
#endif

	
}

void CPlaceDB::AdjustCoordinate( )		//by indark
{
		
	double min_x,min_y;
	min_x = 0.0;
	min_y = 0.0;
	
	for (int i = 0 ; i < (int)m_sites.size() ; i++ ){
		if (min_y  > m_sites[i].m_bottom )
			min_y = m_sites[i].m_bottom;
		for(int j = 0 ; j < (int)m_sites[i].m_interval.size() ; j++ ){
			if (min_x > m_sites[i].m_interval[j] )
				min_x = m_sites[i].m_interval[j];
		}
				
	}
	    
	
	min_x = -min_x;
	min_y = -min_y;
	
	cout << "Adjust X:" << min_x << "Y:" << min_y << endl;
	
	for (int i = 0 ; i < (int)m_sites.size() ; i++ ){
		m_sites[i].m_bottom += min_y;
		for(int j = 0 ; j < (int)m_sites[i].m_interval.size() ; j++ ){
			m_sites[i].m_interval[j] += min_x;
		}
	}
	
	for (int i = 0 ; i < (int)m_modules.size() ; i++ ){
		m_modules[i].m_x += min_x;
		m_modules[i].m_y += min_y;
		m_modules[i].m_cx += min_x;
		m_modules[i].m_cy += min_y;
	}
	for (int i = 0 ; i < (int)m_pins.size() ; i++ ){
		m_pins[i].absX += min_x;
		m_pins[i].absY += min_y;
		
	}
	m_dieArea.bottom += min_y;
	m_dieArea.top += min_y;
	m_dieArea.left += min_x;
	m_dieArea.right += min_x;
	SetCoreRegion();

}

void CPlaceDB::CheckRowHeight(double row_height)		//by indark
{
	for (unsigned int i = 0 ; i < m_sites.size() ; i ++ ){
		if (m_sites[i].m_height == 0)
			m_sites[i].m_height = row_height;
	}
}

//Modified by Jin 20060323
void CPlaceDB::Align( )
{
	m_modules_bak = m_modules;
	double site_step = m_sites[0].m_step;
	double n_slot;
	double aligned_locx;
	for (unsigned int i = 0 ; i < m_modules.size() ; i++ ){
		const Module& curModule = m_modules[i];
		if( !curModule.m_isFixed )
		{
			n_slot = floor ( ( curModule.m_x+(site_step/2.0) / site_step) );
			aligned_locx = n_slot * site_step;
			if ( aligned_locx != curModule.m_x ){
				SetModuleLocation( i, aligned_locx, curModule.m_y );	
			}
		}
				
	}
	

//Original codes of indark
#if 0	
	m_modules_bak = m_modules;
	int site_step = (int)(m_sites[0].m_step);
	int n_slot;
	int align_step;
	double move_amount;
	bool move_result;
	for (unsigned int i = 0 ; i < m_modules.size() ; i++ ){
		n_slot = (int)floor (m_modules[i].m_x / ((double)site_step));
		align_step = n_slot * site_step;
		if ( align_step != m_modules[i].m_x ){
			move_amount = m_modules[i].m_x - align_step;
			move_result= MoveModuleCenter(i,m_modules[i].m_cx-move_amount, m_modules[i].m_cy );
			assert(move_result = true);
			
		}
				
	}
#endif

}

void CPlaceDB::SetModuleType( const int & moduleId, const int & type )
{    
	m_modules[moduleId].m_type = type;
    
}

void CPlaceDB::SetModuleOrientation( const int & moduleId, const int & orient )
{
    int _orient = m_modules[moduleId].m_orient;
    int _start,_end,_count;
    //if (m_modules[moduleId].m_pinsId.size() != 0)
    {

	if ( (_orient %2 )!= (orient %2 )  )
	    swap(m_modules[moduleId].m_width,m_modules[moduleId].m_height);

	/*if((orient / 4 )!= (_orient / 4 )){
	    for(int i = 0 ; i < m_modules[moduleId].m_pinsId.size() ; i++){
		m_pins[m_modules[moduleId].m_pinsId[i]].xOff =
		    -m_pins[m_modules[moduleId].m_pinsId[i]].xOff;

	    }

	}*/
	if( _orient >= 4 )
	{
	    // flip back
	    for(int i = 0 ; i < (int)m_modules[moduleId].m_pinsId.size() ; i++){
		m_pins[m_modules[moduleId].m_pinsId[i]].xOff =
		    -m_pins[m_modules[moduleId].m_pinsId[i]].xOff;
	    }
	}

	_start = _orient %  4;
	_end = (orient %  4) + 4;
	_count = (_end - _start) % 4;
	for (int j = 0 ; j < _count ; j++ ){

	    /*
	    // clockwise 90 degree
	    for(int i = 0 ; i < m_modules[moduleId].m_pinsId.size() ; i++){
		swap(m_pins[m_modules[moduleId].m_pinsId[i]].xOff ,
			m_pins[m_modules[moduleId].m_pinsId[i]].yOff);
		m_pins[m_modules[moduleId].m_pinsId[i]].yOff =
		    -m_pins[m_modules[moduleId].m_pinsId[i]].yOff;
	    }*/
	    // COUNTER-clockwise 90 degree
	    for(int i = 0 ; i < (int)m_modules[moduleId].m_pinsId.size() ; i++){
		swap(m_pins[m_modules[moduleId].m_pinsId[i]].xOff ,
			m_pins[m_modules[moduleId].m_pinsId[i]].yOff);
		m_pins[m_modules[moduleId].m_pinsId[i]].xOff =
		    -m_pins[m_modules[moduleId].m_pinsId[i]].xOff;
	    }

	}

	if( orient >= 4 )
	{
	    // flip new
	    for(int i = 0 ; i < (int)m_modules[moduleId].m_pinsId.size() ; i++){
		m_pins[m_modules[moduleId].m_pinsId[i]].xOff =
		    -m_pins[m_modules[moduleId].m_pinsId[i]].xOff;

	    }
	}
	
	CalcModuleCenter(moduleId);
	for(int i = 0 ; i < (int)m_modules[moduleId].m_pinsId.size() ; i++){
	    CalcPinLocation(m_modules[moduleId].m_pinsId[i]);
	}


    }


    m_modules[moduleId].m_orient = orient;

}


// donnie
int CPlaceDB::CalculateFixedModuleNumber()
{
    int num = 0;
    for( int i=0; i<(int)m_modules.size(); i++ )
    {
	if( m_modules[i].m_isFixed )
	    num++;
    }
    return num;
}

// donnie
int CPlaceDB::GetUsedPinNum()
{
    int num = 0;
    for( int i=0; i<(int)m_nets.size(); i++ )
	num += m_nets[i].size();
    return num;
}

// donnie
bool CPlaceDB::ModuleInCore( const int& i )
{
    if( !m_modules[i].m_isFixed )
	return true;
    
    // check only left-bottom
    double x = m_modules[i].m_x;
    double y = m_modules[i].m_y;
    if( x >= m_coreRgn.left && x <= m_coreRgn.right && 
	    y >= m_coreRgn.bottom && y <= m_coreRgn.top )
	return true;
    else
	return false;
}

bool CPlaceDB::CheckStdCellOrient()
{
    bool ret = true;
    double w, h;
    for( int i=0; i<(int)m_modules.size(); i++ )
    {
	if( m_modules[i].m_isFixed )
	    continue;

	w = m_modules[i].GetWidth();
	h = m_modules[i].GetHeight();
	if( h != m_rowHeight && w <= m_rowHeight )
	{
	    cout << "WARN: std-cell " << m_modules[i].GetName() 
		<< " orient " << orientStr( m_modules[i].m_orient )
		<< " height " << h 
		<< endl;
	   ret = false; 
	}
    }
    return ret;
}


// 2006-02-16
int CPlaceDB::GetMovableBlockNumber()
{
    int n = 0;
    for( int i=0; i<(int)m_modules.size(); ++i )
    {
	if( false == m_modules[i].m_isFixed )
	{
	    n++;
	}
    }
    return n;
}

//2005-12-17
void CPlaceDB::ShowDBInfo()
{

    printf( "\n<<<< DATABASE SUMMARIES >>>>\n\n" );
    
    double coreArea = (m_coreRgn.right-m_coreRgn.left)*(m_coreRgn.top-m_coreRgn.bottom);

    // calc total area, cell area, macro area
    double cellArea = 0;
    double macroArea = 0;
    double moveArea = 0;
    double fixedArea = 0;
    int nCell = 0;
    int nMacro = 0;
    int nFixed = 0;
    //int smallBlock = 0;
    for( int i=0; i<(int)m_modules.size(); ++i )
    {
	//if( m_modules[i].m_isOutCore )
	//    continue;

	if( m_modules[i].m_height == m_rowHeight )
	{
	    nCell++;
	    cellArea += m_modules[i].m_area;
	}
	else if( m_modules[i].m_height > m_rowHeight )
	{
	    nMacro++;
	    macroArea += m_modules[i].m_area;
	}

	if( m_modules[i].m_isFixed )
	{
	    nFixed++;
	    fixedArea += m_modules[i].m_area;
	}
	else
	{
	    moveArea += m_modules[i].m_area;
	}
    }

    double fixedAreaInCore = GetFixBlockArea( m_coreRgn.left, m_coreRgn.bottom,  m_coreRgn.right, m_coreRgn.top );
    
    printf( "         Core region: ");
    m_coreRgn.Print();
    printf( "          Row Height: %.0f\n", m_rowHeight );
    printf( "          Row Number: %d\n", static_cast<int>((m_coreRgn.top-m_coreRgn.bottom)/m_rowHeight) );
    printf( "           Core Area: %.0f (%g)\n", coreArea, coreArea ); 
    printf( "           Cell Area: %.0f (%.2f%%)\n", cellArea, 100.0*cellArea/coreArea );
    printf( "          Macro Area: %.0f (%.2f%%)\n", macroArea, 100.0*macroArea/coreArea );
    printf( "  Macro/(Macro+Cell): %.2f%%\n", 100.0*macroArea/(macroArea+cellArea) );
    printf( "        Movable Area: %.0f (%.2f%%)\n", moveArea, 100.0*moveArea/coreArea );
    printf( "          Fixed Area: %.0f (%.2f%%)\n", fixedArea, 100.0*fixedArea/coreArea );
    printf( "  Fixed Area in Core: %.0f (%.2f%%)\n", fixedAreaInCore, 100.0*fixedAreaInCore/coreArea );
    //printf( "   (Macro+Cell)/Core: %.2f%%\n", 100.0*(macroArea+cellArea)/coreArea );
    printf( "     Placement Util.: %.2f%% (=move/freeSites)\n", 100.0*moveArea/(coreArea-fixedAreaInCore) );
    printf( "        Core Density: %.2f%% (=usedArea/core)\n", 100.0*(moveArea+fixedAreaInCore)/coreArea );
    
    if( nCell > 1000 )
    	printf( "              Cell #: %d (=%dk)\n", nCell , (nCell/1000) );
    else
    	printf( "              Cell #: %d\n", nCell );
    printf( "             Macro #: %d\n", nMacro );

    if( nMacro < 20 )
    {
	for( int i=0; i<(int)m_modules.size(); ++i )
	{
	    if( m_modules[i].m_height > m_rowHeight )
	    {
		printf( "  Name: %s\n", m_modules[i].GetName().c_str() );
	    }
	}

    }
    printf( "             Block #: %d\n", static_cast<int>( m_modules.size() ) );
    printf( "         Fixed Obj #: %d\n", nFixed );
    printf( "               Net #: %d\n", static_cast<int>( m_nets.size() ) );
    printf( "               Pin #: %d\n", static_cast<int>( m_pins.size() ) );
    double wire = CalcHPWL();
    printf( "     Pin-to-Pin HPWL: %.0f (%g)\n", wire, wire );
   
    ShowDensityInfo();
    
    printf( "\n" );
}

void CPlaceDB::ShowDensityInfo()
{
    double wire = CalcHPWL();

    CPlaceBin placeBin( *this );
    placeBin.CreateGrid( m_rowHeight * 10.0 );

    double penalty = placeBin.GetPenalty( 0.9 );
    printf( "  Density Penalty 0.90 = %.2f \t(HPWL = %.0f)\n", penalty, wire*(1+penalty/100.0) );

    penalty = placeBin.GetPenalty( 0.85 );
    printf( "  Density Penalty 0.85 = %.2f \t(HPWL = %.0f)\n", penalty, wire*(1+penalty/100.0) );

    penalty = placeBin.GetPenalty( 0.80 );
    printf( "  Density Penalty 0.80 = %.2f \t(HPWL = %.0f)\n", penalty, wire*(1+penalty/100.0) );

    penalty = placeBin.GetPenalty( 0.75 );
    printf( "  Density Penalty 0.75 = %.2f \t(HPWL = %.0f)\n", penalty, wire*(1+penalty/100.0) );

    //penalty = placeBin.GetPenalty( 0.70 );
    //printf( "  Density Penalty 0.70 = %.2f \t(HPWL = %.0f)\n", penalty, wire*(1+penalty/100.0) );
    
    //penalty = placeBin.GetPenalty( 0.65 );
    //printf( "  Density Penalty 0.65 = %.2f \t(HPWL = %.0f)\n", penalty, wire*(1+penalty/100.0) );
    
    //penalty = placeBin.GetPenalty( 0.60 );
    //printf( "  Density Penalty 0.60 = %.2f \t(HPWL = %.0f)\n", penalty, wire*(1+penalty/100.0) );
    
    //placeBin.ShowInfo( 0.9 );
    //placeBin.ShowInfo( 0.8 );
}

void CPlaceDB::AddNet( Net n )
{
    m_nets.push_back( n );
}

void CPlaceDB::AddNet( set<int> n )
{
    Net dummy;
    m_nets.push_back( dummy );
    Net& net = m_nets[ m_nets.size()-1 ];
    net.reserve( n.size() );
    set<int>::const_iterator ite;
    for( ite=n.begin(); ite!=n.end(); ite++ )
    {
	net.push_back( *ite );
    }
}

int CPlaceDB::AddPin( const int& moduleId, const float& xOff, const float& yOff )
{
    //cout << "AddPin( " << xOff << ", " << yOff << ")\n";
    m_pins.push_back( Pin( xOff, yOff ) );
    int pid = (int)m_pins.size() - 1;
    m_modules[moduleId].m_pinsId.push_back( pid );
    m_pins[pid].moduleId = moduleId;
    return pid;
}

int CPlaceDB::AddPin( const int& moduleId, const string& pinName, 
                       const float& xOff, const float& yOff )	// 2005-08-29
{
    m_pins.push_back( Pin( pinName, xOff, yOff ) );
    int pid = (int)m_pins.size() - 1;
    m_modules[moduleId].m_pinsId.push_back( pid );
    m_pins[pid].moduleId = moduleId;
    return pid;
}


// Memory allocation
void CPlaceDB::ReserveModuleMemory( const int& n )
{
    m_modules.reserve( n );
}
void CPlaceDB::ReserveNetMemory( const int& n )
{
    CPlaceDB::m_nets.reserve( n );
}
void CPlaceDB::ReservePinMemory( const int& n )
{
    m_pins.reserve( n );
}


void CPlaceDB::RemoveDummyFixedBlock()
{
    m_modules.resize( realModuleNumber );
}

// Create dummy blocks for the non-placing sites
int CPlaceDB::CreateDummyFixedBlock()
{
    double currentX;
    char name[1000];
    int counter = 0;
    realModuleNumber = m_modules.size();
    for( int i=0; i<(int)m_sites.size(); i++ )
    {
	currentX = m_coreRgn.left;
	for( int j=0; j<(int)m_sites[i].m_interval.size(); j+=2 )
	{
	    if( m_sites[i].m_interval[j] > currentX )
	    {
		sprintf( name, "__%d", counter );
		AddModule( name, float(m_sites[i].m_interval[j]-currentX), m_sites[i].m_height, true );
		SetModuleLocation( (int)m_modules.size()-1, currentX, m_sites[i].m_bottom );
		counter++;
	    }
	    currentX = m_sites[i].m_interval[j+1];
	}
	if( currentX < m_coreRgn.right )
	{
	    sprintf( name, "__%d", counter );
	    AddModule( name, m_coreRgn.right-currentX, m_sites[i].m_height, true );
	    SetModuleLocation( (int)m_modules.size()-1, currentX, m_sites[i].m_bottom );
	    counter++;
	}

    } // for m_site
    return counter;
}

//Added by Jin 20060228
double CPlaceDB::GetModuleTotalNetLength( const int& mid )
{
	double result = 0.0;
	const Module& curModule = m_modules[mid];

	for( vector<int>::const_iterator iteNetId = curModule.m_netsId.begin() ;
			iteNetId != curModule.m_netsId.end() ; iteNetId++ )
	{
		result += GetNetLength( *iteNetId );
	}

	return result;
}
//@Added by Jin 20060228

//Added by Jin 20060302
void CPlaceDB::RemoveMacroSite()
{
	
	printf( "Remove core sites under Macros\n" );

	/// @todo implement me
	double row_top =  m_sites.back().m_bottom +  m_sites.back().m_height;
	double row_bottom =  m_sites.front().m_bottom;

	for( vector<Module>::iterator iteModule =  m_modules.begin() ;
			iteModule <  m_modules.end() ;
			iteModule++ )
	{
		double module_top = iteModule->m_y + iteModule->m_height;
		double module_bottom = iteModule->m_y;
		double module_left = iteModule->m_x;
		double module_right = iteModule->m_x + iteModule->m_width;
		if( iteModule->m_height > m_rowHeight && 
				module_bottom < row_top && // Confirm that the module is overlapped 
				// with the rows in y coordinates
				module_top > row_bottom )

		{
			vector<CSiteRow>::iterator iteBeginRow, iteEndRow;

			// find the begin row
			for( iteBeginRow =  m_sites.begin() ; 
					iteBeginRow <  m_sites.end() ; 
					iteBeginRow++ )
			{

				if( iteBeginRow->m_bottom + iteBeginRow->m_height > module_bottom )
				{
					break;
				}
				//if( iteBeginRow->m_bottom == iteModule->m_y )
				//{
				//	break;
				//}
				//else if( iteBeginRow->m_bottom > iteModule->m_y )
				//{
				//	if( iteBeginRow >  m_sites.begin() )
				//	{
				//		iteBeginRow--;
				//	}

				//	break;
				//}
			}




			for( iteEndRow = iteBeginRow ;
					iteEndRow <  m_sites.end() ;
					iteEndRow++ )
			{
				if( iteEndRow->m_bottom + iteEndRow->m_height >= module_top )
				{
					break;
				}
			}

			if( iteEndRow ==  m_sites.end() )
				iteEndRow--;


			assert( iteBeginRow !=  m_sites.end() );

			for( vector<CSiteRow>::iterator iteRow = iteBeginRow ;
					iteRow <= iteEndRow ;
					iteRow++ )
			{
				double interval[2];
				for( int i = 0 ; i < (signed)iteRow->m_interval.size() ; i++ )
				{
					interval[ i % 2 ] = iteRow->m_interval[i];
					if( ( i % 2 ) == 1 ) // Get two terminals of the interval
					{
						if( interval[0] >= module_right || interval[1] <= module_left )  // screen unnecessary checks
							continue;

						if( interval[0] >= module_left && interval[1] <= module_right )
						{
							iteRow->m_interval.erase( vector<double>::iterator(&(iteRow->m_interval[i])) );
							iteRow->m_interval.erase( vector<double>::iterator(&(iteRow->m_interval[i-1])) );
							i = i - 2;
						}
						else if( interval[1] > module_right && interval[0] >= module_left )
						{
							iteRow->m_interval[i-1] = module_right;
						}
						else if( interval[0] < module_left && interval[1] <= module_right )
						{
							iteRow->m_interval[i] = module_left;
						}
						else if( interval[0] < module_left && interval[1] > module_right )
						{
							iteRow->m_interval[i] = module_left;
							iteRow->m_interval.insert( vector<double>::iterator(&(iteRow->m_interval[i+1])), interval[1] );
							iteRow->m_interval.insert( vector<double>::iterator(&(iteRow->m_interval[i+1])), module_right );
							i = i + 2;

						}
						else
						{
							printf("Warning: Module Romoving Error\n");
							//exit(-1);
						}

					}
				}
			}

			//@remove the occupied sites

		}
	}
#if 0 
	//test code
	ofstream sitefile( "sites.log" );
	for( vector<CSiteRow>::iterator iteRow = m_sites.begin() ; 
			iteRow != m_sites.end() ; iteRow++ )
	{
		sitefile << "Row bottom: " << iteRow->m_bottom << " ";

		for( unsigned int iInterval = 0 ; 
				iInterval != iteRow->m_interval.size() ; iInterval=iInterval+2 )
		{
			sitefile << "(" << iteRow->m_interval[iInterval] 
				<< "," << iteRow->m_interval[iInterval+1]  << ") ";
		}
		sitefile << endl;
	}

	sitefile.close();
	cout << "Dump site.log" << endl;
	//@test code
#endif
}

//@Added by Jin 20060302
    

double CPlaceDB::GetHPWLdensity( double util )
{
    CalcHPWL();

    if( util < 0 )
	return m_HPWLp2p;
    
    CPlaceBin placeBin( *this );
    placeBin.CreateGrid( m_rowHeight * 10.0 );

    double penalty = placeBin.GetPenalty( util );
    return m_HPWLp2p*(1+penalty/100.0);
}

//2006-03-06
void CPlaceDB::SetAllBlockMovable()
{
    
    printf( "Set all block movable... " );
    int count=0;
    for( unsigned int i=0; i<m_modules.size(); i++ )
    {
	if( m_modules[i].m_isOutCore == false )
	{
	    if( m_modules[i].m_isFixed )
	    {
		count++;
		m_modules[i].m_isFixed = false;
	    }
	}
    }
    printf( "%d blocks\n", count ); 
}
