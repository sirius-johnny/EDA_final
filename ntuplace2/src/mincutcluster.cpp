#include <list>
#include <vector>
#include <algorithm>
using namespace std;

#include "mincutcluster.h"
#include "Partition.h"
#include "libhmetis.h"	


CMinCutCluster::CMinCutCluster(CPlaceDB& db)
{
	fplan = &db;	
}


CMinCutCluster::~CMinCutCluster()
{
}


void CMinCutCluster::CreateCluster( const CPlaceDB& inDB, CPlaceDB& outDB )  
{
    Init();

    NewPartMem();

    fplan->part_options[0] = 1;	// Use the following options. (0=default)
    fplan->part_options[1] = param.hmetis_run;
    fplan->part_options[2] = param.hmetis_ctype;
    fplan->part_options[3] = param.hmetis_rtype;
    fplan->part_options[4] = param.hmetis_vcycle;
    fplan->part_options[5] = 0; 	// Reconst  -- Useless for 2-way bisection
    fplan->part_options[6] = 1;	// Use fix vertices
    fplan->part_options[7] = param.seed;  // Seed (-1: random)
    fplan->part_options[8] = param.hmetis_debug;	// debug level

    printf( "OLD: movable block # = %d\n", inDB.m_modules.size() );
    int targetCount = inDB.m_modules.size() / 10;
    printf( "target block # = %d\n", targetCount );

    int partCount = 0;
    do
    {
	LevelPartition();

	double totalArea = 0;
	partCount = 0;
	for( int i=0; i<(int)m_partitions.size(); i++ )
	{
	    if( m_partitions[i].bDone == false )
	    {
		partCount++;
		totalArea += m_partitions[i].area;
	    }
	}
	//printf( "area = %.0f\n", totalArea );

    } while( partCount < targetCount );
   

    // Convert m_partitions to placedb
    vector< vector<int> > clustersInfo;
    vector<int> cluster;
    for( int i=0; i<(int)m_partitions.size(); i++ )
    {
	if( m_partitions[i].bDone == false )
	{
	    clustersInfo.push_back( cluster );

	    for( unsigned int j=0; j<m_partitions[i].moduleList.size(); j++ )
		clustersInfo.back().push_back( m_partitions[i].moduleList[j] );
	}
    }
    assert( (int)clustersInfo.size() == partCount );
    
    int nFixedBlock = 0;
    for( int i=0; i<(int)inDB.m_modules.size(); i++ )
    {
	if( inDB.m_modules[i].m_isFixed )
	    nFixedBlock++;
    }
    
    // prepare map from old id to new id
    vector<int> newBlockId;
    newBlockId.resize( inDB.m_modules.size() );
    fill( newBlockId.begin(), newBlockId.end(), -1 );

    // handle movable block
    for( int i=0; i<(int)clustersInfo.size(); i++ )
       for( int j=0; j<(int)clustersInfo[i].size(); j++ )
       {
	   newBlockId[ clustersInfo[i][j] ] = i;
       }	   

    // handle fixed block 
    int blockId = clustersInfo.size();
    for( int i=0; i<(int)inDB.m_modules.size(); i++ )
    {
	if( inDB.m_modules[i].m_isFixed )
	{
	    assert( newBlockId[i] == -1 );
	    newBlockId[i] = blockId;
	    blockId++;
	}
    }
     
    // check the map
    for( int i=0; i<(int)newBlockId.size(); i++ )
    {
	assert( newBlockId[i] != -1 );
    }    
   
    // add blocks to outDB
    
    // add nets to outDB
    
    m_partitions.clear();
    DeletePartMem();
}


void CMinCutCluster::Init()
{
    // Set all blocks to the center of the core region
    double cx = 0.5* (fplan->m_coreRgn.left + fplan->m_coreRgn.right );
    double cy = 0.5* (fplan->m_coreRgn.bottom + fplan->m_coreRgn.top );
    for( int i=0; i<(int)fplan->m_modules.size(); i++ )
    {
	fplan->MoveModuleCenter( i, cx, cy );
    }

    // Add the first partition
    AddPartition( -1, 
	    fplan->m_coreRgn.left, 
	    fplan->m_coreRgn.bottom, 
	    fplan->m_coreRgn.right, 
	    fplan->m_coreRgn.top );

    float density = m_partitions[0].GetDensity();

    if( param.bShow )
    {
	cout << "    Core density: " << density << "\n";
	cout << "Core utilization: " << m_partitions[0].GetUtilization() << "\n";
    }

    if( density > 1.0 )
    {
	cout << "Sorry, core density > 1.0\n";
	exit(0);
    }

}


bool CMinCutCluster::LevelPartition()
{
    static int memoryCollectCounter = 0;
    static int level = 0;
    static int startPartId = 0;     // The partitionId starts from 0.
    level++;

    // 2005/3/9
    if( level > 50 )
        return false;

    //double levelStartTime = seconds();

    CUTDIR cutDir = NONE_CUT;

    int endPartId = GetPartitionNumber();
    //int totalPart = endPartId - startPartId;
    int edgecut, new1, new2;

    // Create the sequence of the partitioning
    int partId;
    list<DoubleInt> partSeq;
    DoubleInt di;

    for( partId = startPartId; partId < endPartId; partId++ )
    {
	if( m_partitions[partId].bDone == false && m_partitions[partId].moduleList.size() > 1 )
	{
	    partSeq.push_back( di );
	    partSeq.back().d = m_partitions[partId].area;
	    partSeq.back().i = partId;
	}
    }
    partSeq.sort( DoubleInt::Greater );	    // from large to small

    list<DoubleInt>::const_iterator ite;
 
    double largestArea = 0;
    set<int> doneParts; // a set storing parted regions
    for( ite = partSeq.begin(); ite != partSeq.end(); ite++ )
    {
	partId = ite->i;

	if( m_partitions[partId].area < largestArea )
	    break;
	
	MakePartition( partId, edgecut, new1, new2, cutDir );
	doneParts.insert( partId );
	m_partitions[partId].bDone = true;

	if( new1 > 0 && m_partitions[new1].area > largestArea )
	    largestArea = m_partitions[new1].area;
	if( new2 > 0 && m_partitions[new2].area > largestArea )
	    largestArea = m_partitions[new2].area;
    }

    // Collect some infomation for the next level
    double maxPartArea = 0;
    double minPartArea = 1e20;
    int    maxBlockCount = 0;
    int    nextEndPartId = GetPartitionNumber();
    int	   parts = 0;
    for( int i=startPartId; i<nextEndPartId; i++ )
    {
	if( m_partitions[i].bDone == false )
	{
	    parts++;
	    if( m_partitions[i].area > maxPartArea )
		maxPartArea = m_partitions[i].area;
	    if( m_partitions[i].area < minPartArea )
		minPartArea = m_partitions[i].area;
	    if( (int)m_partitions[i].moduleList.size() > maxBlockCount )
		maxBlockCount = (int)m_partitions[i].moduleList.size();
	}
    }
    printf( "  Part# = %5d   MaxArea/MinArea= %.1f   MaxBlock #= %d\n",
	    parts, maxPartArea/minPartArea, maxBlockCount );
    
    // Collect memory
    for( int i=memoryCollectCounter; i<startPartId; i++ )
    {
        // L1: none
        // L2: clean L1
        // L3: clean L2
	if( m_partitions[i].bDone )
	{
	    m_partitions[i].blockPosList.clear();
	    m_partitions[i].netList.clear();
	    m_partitions[i].moduleList.clear();
	    m_partitions[i].fixModuleList.clear();
	    m_partitions[i].netListBak.clear();
	    m_partitions[i].moduleListBak.clear();        
	    memoryCollectCounter = i;
	}
    }

    for( int i=startPartId; i<endPartId; i++ )
    {
	if( m_partitions[i].bDone )
	    startPartId++;
	else
	    break;
    }

    //printf( "next startPartId= %d / %d\n", startPartId, GetPartitionNumber() );
    if( startPartId < GetPartitionNumber() )
	return true;
    else
	return false;
}



//----------------------------------------------------------------------------
// UpdatePartition
// Clear code: 2005-12-03
//----------------------------------------------------------------------------
void CMinCutCluster::UpdatePartition( const int& partId )
{
    double totalMovableArea = 0;
    //double totalFixedArea = m_partitions[partId].totalFixedArea;
    double left   = m_partitions[partId].left;
    double right  = m_partitions[partId].right;
    double top    = m_partitions[partId].top;
    double bottom = m_partitions[partId].bottom;
    CPoint center = fplan->GetRegionWeightedCenter( left, right, bottom , top );

    // Move blocks to the region center
    // Calculate total movable area
    // Collect nets coorelated to this region
    set<int> setNetId;	    // net id set
    for( int i=0; i<(int)m_partitions[partId].moduleList.size(); i++ )
    {
	int moduleId = m_partitions[partId].moduleList[i];

	fplan->MoveModuleCenter( moduleId, center.x, center.y );

	totalMovableArea += fplan->m_modules[moduleId].m_area;

	for( int j=0; j<(int)fplan->m_modules[moduleId].m_netsId.size(); j++ )
	    setNetId.insert( fplan->m_modules[moduleId].m_netsId[j] );
    }

    // Move from "set" to "vector"
    m_partitions[partId].netList.clear();
    set<int>::const_iterator ite;
    for( ite=setNetId.begin(); ite!=setNetId.end(); ite++ )
    {
	m_partitions[partId].netList.push_back( *ite );
    }

    m_partitions[partId].totalMovableArea = totalMovableArea;
    //m_partitions[partId].usedArea = totalMovableArea + totalFixedArea;
    //m_partitions[partId].freeArea = m_partitions[partId].area - totalFixedArea;
    //m_partitions[partId].utilization = (totalMovableArea+totalFixedArea) / m_partitions[partId].area;

    //if( m_partitions[partId].utilization > 1.0 )
    //{
    //	cout << "** Util= " << m_partitions[partId].utilization << "**\n";
    //}
}


//----------------------------------------------------------------------------
// AddPartition
//
// Create a new partition which has the parent partition id = parentPartId.
// Find all movable modules in the region and put into the new partition.
// Return new partition id
//----------------------------------------------------------------------------
int CMinCutCluster::AddPartition( const int& parentPartId, 
	double left, double bottom, 
	double right, double top, int partId )
{
    // If partId != -1, we need to use the exist part id.
    if( partId > 0 )
    {
        // Use existing part
        m_partitions[partId].left   = left;
        m_partitions[partId].bottom = bottom;
        m_partitions[partId].right  = right;
        m_partitions[partId].top    = top;
        m_partitions[partId].area   = (right-left)*(top-bottom);
        m_partitions[partId].moduleList.clear();
        m_partitions[partId].netList.clear();
        m_partitions[partId].fixModuleList.clear();
    }
    else
    {
        // Create a new part
	    m_partitions.push_back( CPartition( parentPartId, top, bottom, left, right, NONE_CUT ) );
	    partId = (int)m_partitions.size() - 1;
    }

 	set<int> setId;		// Save the moduleId in the new partition

    double totalMovableArea = 0;
    double totalFixedArea = 0;
    if( parentPartId == -1 )	// The first partition
    {
	for( int i=0; i<(int)fplan->m_modules.size(); i++ )
	{
	    if( fplan->m_modules[i].m_isFixed )
	    {
		double a = getOverlapArea( left, bottom, right, top, 
			fplan->m_modules[i].m_x, fplan->m_modules[i].m_y, 
			fplan->m_modules[i].m_x+fplan->m_modules[i].m_width, 
			fplan->m_modules[i].m_y+fplan->m_modules[i].m_height );
		if( a > 0 )
		{
		    m_partitions[partId].fixModuleList.push_back( i );
		    totalFixedArea += a;
		}
		continue;
	    }
	    if( fplan->m_modules[i].m_cx > left && 
		    fplan->m_modules[i].m_cx < right &&
		    fplan->m_modules[i].m_cy > bottom && 
		    fplan->m_modules[i].m_cy < top )
	    {
		setId.insert( i );
		m_partitions[partId].moduleList.push_back( i );
		totalMovableArea += fplan->m_modules[i].m_area;
	    }
	}

	// Handle nets
	bool outsidePart;
	for( int i=0; i<fplan->m_nNets; i++ )
	{
	    outsidePart = true;
	    for( int j=0; j<(int)fplan->m_nets[i].size(); j++ )
	    {
		if( setId.find( fplan->m_pins[ fplan->m_nets[i][j] ].moduleId ) != setId.end() ) 
		{
		    // Found a pin in the partition, so the whole net is impossible outside the partition.
		    outsidePart = false;
		    break;
		}
	    }
	    // If all terminals of the net is not in the partition, we discard it.
	    if( outsidePart == true )	
		continue;	// discard the net

	    m_partitions[0].netList.push_back( i );		// The net is belongs to the partition
	} // For each net		
    }
    else 
    {
	// Not the first partition. (Has parent)

	int i;
	// handle movable blocks
	for( int j=0; j<(int)m_partitions[parentPartId].moduleList.size(); j++ )
	{
	    i = m_partitions[parentPartId].moduleList[j];	// get real module id
	    assert( fplan->m_modules[i].m_isFixed == false );
	    if( fplan->m_modules[i].m_cx > left && fplan->m_modules[i].m_cx < right &&
		    fplan->m_modules[i].m_cy > bottom && fplan->m_modules[i].m_cy < top )
	    {
		setId.insert( i );
		m_partitions[partId].moduleList.push_back( i );
		totalMovableArea += fplan->m_modules[i].m_area;
	    }
	}

	// handle fixed blocks
	for( int j=0; j<(int)m_partitions[parentPartId].fixModuleList.size(); j++ )
	{
	    i = m_partitions[parentPartId].fixModuleList[j];	// get real module id
	    assert( fplan->m_modules[i].m_isFixed == true );
	    double a = getOverlapArea( left, bottom, right, top, 
		    fplan->m_modules[i].m_x, 
		    fplan->m_modules[i].m_y, 
		    fplan->m_modules[i].m_x+fplan->m_modules[i].m_width, 
		    fplan->m_modules[i].m_y+fplan->m_modules[i].m_height );
	    if( a > 0 )
	    {
		m_partitions[partId].fixModuleList.push_back( i );
		totalFixedArea += a;
	    }
	}

	// handle nets
	//cout << "set size= " << setId.size() << endl;
	int netId;
	bool outsidePart;
	for( int i=0; i<(int)m_partitions[parentPartId].netList.size(); i++ )
	{
	    netId = m_partitions[parentPartId].netList[i];
	    outsidePart = true;
	    for( int j=0; j<(int)fplan->m_nets[netId].size(); j++ )
	    {
		if( setId.find( fplan->m_pins[ fplan->m_nets[netId][j] ].moduleId ) != setId.end() ) 
		{
		    // Found a pin in the partition, so the whole net is impossible outside the partition.
		    outsidePart = false;
		    break;
		}
	    }
	    // If all terminals of the net is not in the partition, we discard it.
	    if( outsidePart == true )	
		continue;	// discard the net

	    m_partitions[partId].netList.push_back( netId );		// The net is belongs to the partition
	} // For each net in the parent partition 


    }

    m_partitions[partId].totalMovableArea = totalMovableArea;
    m_partitions[partId].totalFixedArea = totalFixedArea;

    float util = m_partitions[partId].GetUtilization();
    if( util > 3.0 && (int)m_partitions[partId].moduleList.size() > 3 )
    {
	cout << "** Util= " << util << " (" << (int)m_partitions[partId].moduleList.size() << " blocks)**\n";
    }

    return partId;

}

bool CMinCutCluster::MakePartition( int partId, 
			int& edgecut, 
			int& new1, 
			int& new2, 
			CUTDIR cutDir/*=NONE_CUT*/,
                        bool noSubRegion/*=false*/, 
			bool considerMovable/*=true*/, 
			bool moveHalf/*=false*/,  
			double moveRatio/*=0.5*/ )
{

    //**debug** printf( "In MakePartition()" );
    
    double bUse2D = true;

    if( (int)m_partitions[partId].moduleList.size() <= 1 )	// no more cut
		return false;

    if( m_partitions[partId].area < fplan->m_rowHeight )   // less than a site
    {
        cout << " Bin is too small! (N=" << (int)m_partitions[partId].moduleList.size() << ")\n";
        return false;
    }

#if 0
    // Show part info
    cout << "\n\nPartition " << partId 
        << " (" << m_partitions[partId].left << "," << m_partitions[partId].bottom << ")-(" 
		<< m_partitions[partId].right << "," << m_partitions[partId].top << ")\n";
#endif
	

    if( cutDir == NONE_CUT )
    	cutDir = GetCutDirForPart( partId );
    if( ( m_partitions[partId].top - m_partitions[partId].bottom ) <= fplan->m_rowHeight )
	cutDir = V_CUT;

    //m_partitions[partId].cutDir = cutDir;

    // Calculate the partition ID to the coordinate of the partition
    // ---------     ---------      ---------
    // |       |     |   |   |      | 4 | 6 |
    // |   0   |     | 1 | 2 |      |---|---|
    // |       |     |   |   |      | 3 | 5 |
    // ---------     ---------      ---------
    double top    = m_partitions[partId].top;
    double bottom = m_partitions[partId].bottom;
    double left   = m_partitions[partId].left;
    double right  = m_partitions[partId].right;



    //////////////////////////////////////////////////////////////
    // prepare data strucutre
    //////////////////////////////////////////////////////////////

    const double extraWeight = 1.0 / fplan->m_rowHeight;

    //============================================================================
	// Prepare nodes. Only node 0 and node 1 are fixed. 
    //============================================================================
    part[0] = 0;	// node0 is fixed to part0
    part[1] = 1;	// node1 is fixed to part1
    moduleWeight[0] = 0;
    moduleWeight[1] = 0;

    // 1. create module map and initial partition
    map<int, int> mapId;	    // moduleId -> hMetis_vetex_id
    map<int, int> reverseMapId;	    // hMetis_vetex_id -> moduleId
    int moduleId;
    int moduleNumber = (int)m_partitions[partId].moduleList.size(); // # blocks in the partition
    int moduleCounter = 2 + moduleNumber;
    for( int i=0; i<moduleNumber; i++ )
    {
	// mappings: vetex_id <--> moduleId
	moduleId = m_partitions[partId].moduleList[i]; 
	mapId[ moduleId ] = i+2;
	reverseMapId[ i+2 ] = moduleId;
	//cout << moduleId << " ==> " << moduleCounter << endl;

	int nodeWeight = (int)round( fplan->m_modules[moduleId].m_area * extraWeight );

	if( nodeWeight <= 0 ) // Usually, nodeWeight > 0
	{
	   printf( "WARNING! node_weight = %d <= 0, extra_weight = %g\n", nodeWeight, extraWeight );
	}

	moduleWeight[i+2] = nodeWeight;
    }

    // 2. Find the "guessed" cut-line position ( at "mid" )
    //    Example: (V-CUT)
    //		|            |          |
    //		| part0      |  part1	|
    //		|            |          |
    //		-------------------------
    //	              |<--------->|
    //               mid0  mid   mid1

    CPoint pointMid0, pointMid1, pointMid;
    //double mid0_x, mid0_y, mid1_x, mid1_y;
    double mid, mid0, mid1;             // guessed cut-line and center of the part
    double area0, area1;                // guessed area
    double usedFixArea0, usedFixArea1;  // guessed used area

    pointMid.x = ( left + right ) * 0.5;
    pointMid.y = ( top + bottom ) * 0.5;
    if( cutDir == V_CUT )
    {	
	mid = (left + right) * 0.5;

	// 1D
	//mid0 = (left + mid) * 0.5;
	//mid1 = (mid + right) * 0.5;
	//fplan->GetRegionWeightedCenter( left, bottom,   mid, top, mid0_x, mid0_y );
	//fplan->GetRegionWeightedCenter(  mid, bottom, right, top, mid1_x, mid1_y );
	//mid0 = mid0_x;
	//mid1 = mid1_x;

	// 2D
	pointMid0 = fplan->GetRegionWeightedCenter( left,  mid,  bottom, top );
	pointMid1 = fplan->GetRegionWeightedCenter(  mid, right, bottom, top );
	mid0 = pointMid0.x;
	mid1 = pointMid1.x;

	//pointMid.y = 0.5 * (pointMid0.y + pointMid1.y); // adjust mid y-coordinate

	if( false == bUse2D )
	{
	    // 1D: Don't use y-coordinates
	    pointMid0.y = pointMid.y;
	    pointMid1.y = pointMid.y;
	}
	
	// left
	usedFixArea0 = GetFixBlockAreaInPartition( partId, left, bottom, mid, top );
	area0 = (mid-left)*(top-bottom);
	// right
	usedFixArea1 = GetFixBlockAreaInPartition( partId, mid, bottom, right, top );
	area1 = (right-mid)*(top-bottom);
    }
    else // H_CUT
    {

	
	if( param.bFractionalCut )
	    mid = (top + bottom)/2.0;   // don't align rows
	else
	    mid = round( (top-bottom) / fplan->m_rowHeight / 2.0 ) * fplan->m_rowHeight + bottom;     // align rows
	
/*	
	if( (top-bottom) > fplan->m_rowHeight * 25 )
	    mid = round( (top-bottom) / fplan->m_rowHeight / 2.0 ) * fplan->m_rowHeight + bottom;     // align rows
	else
	    mid = (top + bottom)/2.0;   // don't align rows
*/
	
	// 1D
	////mid0 = (bottom + mid) * 0.5;
	////mid1 = (mid + top) * 0.5;
	//  fplan->GetRegionWeightedCenter( left, bottom, right, mid, mid0_x, mid0_y );
	//  fplan->GetRegionWeightedCenter( left,    mid, right, top, mid1_x, mid1_y );
	//  mid0 = mid0_y;
	//  mid1 = mid1_y;
	//  if( param.bFractionalCut == false )
	//  {
	//      // align rows
	//mid0 = round( (mid0-bottom) / fplan->m_rowHeight ) * fplan->m_rowHeight + bottom;
	//mid1 = round( (mid1-bottom) / fplan->m_rowHeight ) * fplan->m_rowHeight + bottom;
	//  }

	// 2D
	pointMid0 = fplan->GetRegionWeightedCenter( left, right, bottom, mid );
	pointMid1 = fplan->GetRegionWeightedCenter( left, right,    mid, top );

	//if( mid  <0 )
	//{
	//    cout << "top= " << top << endl;
	//    cout << "bottom= " << bottom << endl;
	//    cout << "mid= " << mid << endl;
	//}
	//assert( mid > 0 );

	mid0 = pointMid0.y;
	mid1 = pointMid1.y;
	//pointMid.x = 0.5 * (pointMid0.x + pointMid1.x); // adjust mid x-coordinate

	if( false == bUse2D )
	{
	    // 1D: don't use x-coordinates
	    pointMid0.x = pointMid.x;
	    pointMid1.x = pointMid.x;
	}

	// bottom
	usedFixArea0 = GetFixBlockAreaInPartition( partId, left, bottom, right, mid );
	area0 = (mid-bottom)*(right-left);
	// top
	usedFixArea1 = GetFixBlockAreaInPartition( partId, left, mid, right, top );
	area1 = (top-mid)*(right-left);
    }  
    //cout << "mid= " << mid << endl;
    //cout << "mid0= " << mid0 << endl;
    //cout << "mid1= " << mid1 << endl;

    if( moveHalf )
    {
        //mid0 = (mid0 + mid) * 0.5;
        //mid1 = (mid1 + mid) * 0.5;
        //pointMid0.x = (pointMid0.x + pointMid.x) * 0.5;
        //pointMid0.y = (pointMid0.y + pointMid.y) * 0.5;
        //pointMid1.x = (pointMid1.x + pointMid.x) * 0.5;
        //pointMid1.y = (pointMid1.y + pointMid.y) * 0.5;

        // 2005/03/13 moveRatio
        mid0 = mid + (mid0 - mid) * moveRatio;
        mid1 = mid + (mid1 - mid) * moveRatio;
        pointMid0.x = pointMid.x + (pointMid0.x - pointMid.x) * moveRatio;
        pointMid0.y = pointMid.y + (pointMid0.y - pointMid.y) * moveRatio;
        pointMid1.x = pointMid.x + (pointMid1.x - pointMid.x) * moveRatio;
        pointMid1.y = pointMid.y + (pointMid1.y - pointMid.y) * moveRatio;
    }
    //cout << "usedFixArea0= " << usedFixArea0 << endl;         // fixed block area in partition 0
    //cout << "usedFixArea1= " << usedFixArea1 << endl;         // fixed block area in partition 1

    double centerDistance = Distance( pointMid1, pointMid0 ); 
    
    if( centerDistance < 1.0 )
    {
        cout << "WARNING! center_distance < 1.0\n";
        return false;   // centers are too close
    }

    //=============================================================
    // Consider the two special cases: freeArea0=0 and freeArea1=0
    //=============================================================
    double freeArea0 = area0 - usedFixArea0;
    double freeArea1 = area1 - usedFixArea1;
    double totalMovableArea = m_partitions[partId].totalMovableArea;

    if( freeArea0 == 0 && freeArea1 == 0 )
    {
	cout << "\nWARNING!! Both guessed freeArea are 0\n";
	//cout << "   area= " << area0 << " : " << area1 << endl;
	//cout << "   fixArea= " << usedFixArea0 << " : " << usedFixArea1 << endl;
	//cout << "   freeArea= " << freeArea0 << " : " << freeArea1 << endl;
	//cout << "   dummyArea= " << dummyArea << endl;
#if 0
	exit(0);
#endif
    }

    if( freeArea0 < fplan->m_rowHeight && freeArea1 > totalMovableArea*0.1 )    
    {
	// Put all blocks in partition 1
	for( int i=0; i<(int)m_partitions[partId].moduleList.size(); i++ )
	{
	    int id = m_partitions[partId].moduleList[i];
	    fplan->SetModuleLocation( id,  pointMid1.x, pointMid1.y );
	}
	// Create the partition
	int pid;
	if( cutDir == V_CUT ) // right
	    pid = AddPartition( partId, mid, bottom, right, top, m_partitions[partId].childPart1 );
	else // H_CUT: top
	    pid = AddPartition( partId, left, mid, right, top, m_partitions[partId].childPart1 );

	// Return values
	m_partitions[partId].childPart0 = 0;	
	m_partitions[partId].childPart1 = pid;	
	new1 = -1;
	new2 = pid;
	//cout << "r";
	return false;
    }
    if( freeArea1 < fplan->m_rowHeight && freeArea0 > totalMovableArea*0.1 )    
    {
	// Put all blocks in partition 0
	for( int i=0; i<(int)m_partitions[partId].moduleList.size(); i++ )
	{
	    int id = m_partitions[partId].moduleList[i];
	    fplan->SetModuleLocation( id,  pointMid0.x, pointMid0.y );
	}
	// Create the partition
	int pid;
	if( cutDir == V_CUT ) // left
	    pid = AddPartition( partId, left, bottom, mid, top, m_partitions[partId].childPart0 );
	else // H_CUT: bottom
	    pid = AddPartition( partId, left, bottom, right, mid, m_partitions[partId].childPart0 );

	// Return values
	m_partitions[partId].childPart0 = pid;	
	m_partitions[partId].childPart1 = 0;	
	new1 = pid;
	new2 = -1;
	//cout << "l";
	return false;
    }

    // 3. Calc dummy area.
    double dummyArea = 0;
    if( freeArea0 > freeArea1 )
    {
	dummyArea = totalMovableArea * ( (freeArea0 - freeArea1) / (freeArea0 + freeArea1) );
	moduleWeight[1] = (int)round( dummyArea * extraWeight );
    }
    else if( freeArea0 < freeArea1 )
    {
	dummyArea = totalMovableArea * ( (freeArea1 - freeArea0) / (freeArea0 + freeArea1) );
	moduleWeight[0] = (int)round( dummyArea * extraWeight );
    }

    // 4. Calculate "unbalance" factor 
    // CHOICE 1: The following values are (likely) better for legalizer
    //double ub0 = g_ubfactor * 100 * (freeArea0/totalMovableArea - 0.5);
    //double ub1 = g_ubfactor * 100 * (freeArea1/totalMovableArea - 0.5);
    // CHOICE 2: The "real" values (seems bad for legalizer... strange...)
    double ub0 = param.ubfactor * 100 * (freeArea0/(totalMovableArea+dummyArea) - 0.5);
    double ub1 = param.ubfactor * 100 * (freeArea1/(totalMovableArea+dummyArea) - 0.5);
    int ubfactor = (int)min( ub0, ub1 );
    if( ubfactor > 49 ) ubfactor = 49;
    if( ubfactor < 1 )  ubfactor = 1;

    if( param.ubfactor <= 0.01 )
        ubfactor = 1;

    //if( ubfactor != 1 )
    //{
    //    cout << "ub= " << ubfactor << endl;
    //    cout << "free= " << freeArea0 << " : " << freeArea1 << endl;
    //    cout << "totalMovableArea= " << totalMovableArea << " dummy= " << dummyArea << endl;
    //}


    // 2. create net array
    //===================================
    //double extraNetWeight = 2147483647.0 / (mid1-mid0) / (int)m_partitions[partId].netList.size();
    //double extraNetWeight =  500000000.0 / (mid1-mid0) / (int)m_partitions[partId].netList.size();    // 1D
    double extraNetWeight =    500000000.0 / centerDistance / (int)m_partitions[partId].netList.size(); // 2D
    if( extraNetWeight >= 1.0 )
	extraNetWeight = 1.0;
    //===================================
    int netCounter = 0;
    int pinCounter = 0;
    eptr[netCounter] = 0;
    netCounter++;
    bool insidePart;			// whole net inside the partition
    bool outsidePart;			// whole net outside the partition
    vector<bool> externalPin;	// Save the infomation of external/internal pin to reduce the map lookup time.
    int netId;
    for( int i=0; i<(int)m_partitions[partId].netList.size(); i++ )
    {
	netId = m_partitions[partId].netList[i];
	externalPin.resize( fplan->m_nets[netId].size() );

	insidePart = true;
	outsidePart = true;

	for( int j=0; j<(int)fplan->m_nets[netId].size(); j++ )
	{
	    if( mapId.find( fplan->m_pins[ fplan->m_nets[netId][j] ].moduleId ) != mapId.end() ) 
	    {
		// Found in the partition, so the whole net is impossible outside the partition
		outsidePart = false;
		externalPin[j] = false;
	    }
	    else	
	    {
		// Not found in the partition, so the whole net is impossible inside the partition
		insidePart = false;
		externalPin[j] = true;
	    }
	}

	// Case 1:
	// If all terminals of the net is not in the partition, we discard it.
	if( outsidePart == true )	
	    continue;	// discard the net

	// Case 2:
	// If all terminals of the net is in the partition, we simply take it. (No connections to fixed nodes)
	if( insidePart == true )
	{
	    // Collect nodes, remove duplicate nodes for a single net
	    set<int> nodes;     
	    for( int j=0; j<(int)fplan->m_nets[netId].size(); j++ )
	    {
		int pinId = fplan->m_nets[netId][j];
		nodes.insert( mapId[ fplan->m_pins[ pinId ].moduleId ] );
	    }

	    //         // Calculate pin offsets
	    //         CPoint boxP0( 0, 0 );
	    //         CPoint boxP1( 0, 0 );
	    //for( int j=0; j<(int)fplan->m_nets[netId].size(); j++ )
	    //         {
	    //             int pinId = fplan->m_nets[netId][j];
	    //             if( fplan->m_pins[ pinId ].xOff < boxP0.x )   boxP0.x = fplan->m_pins[ pinId ].xOff; 
	    //             if( fplan->m_pins[ pinId ].yOff < boxP0.y )   boxP0.y = fplan->m_pins[ pinId ].yOff; 
	    //             if( fplan->m_pins[ pinId ].xOff > boxP1.x )   boxP1.x = fplan->m_pins[ pinId ].xOff; 
	    //             if( fplan->m_pins[ pinId ].yOff > boxP1.y )   boxP1.y = fplan->m_pins[ pinId ].yOff; 
	    //         }
	    //         //boxP0.Print();
	    //         //cout << " ";
	    //         //boxP1.Print();
	    //         //cout << endl;
	    //         double offset = Distance( boxP0, boxP1 );

	    if( nodes.size() == 1 )
		continue;

	    // Put into hMETIS net data structure
	    int weight = (int)round( extraNetWeight * ( centerDistance /*+ offset*/ ) );
	    if( weight <= 0 )   // Usually, the weight should > 0 for this case.
	    {
		cout << "(WARN) Inside net weight <= 0 \n";
		m_partitions[partId].Print();
		continue;
	    }

	    set<int>::const_iterator ite;
	    for( ite=nodes.begin(); ite!=nodes.end(); ite++ )
	    {
		eind[pinCounter] = *ite;
		pinCounter++;
		//if( pinCounter > fplan->m_pins.size() )
		//    cout << "WARN: pinCounter > fplan->m_pins.size()\n";
	    }
	    eptr[netCounter] = pinCounter;


#if 0
	    if( weight <= 0 )
	    {
		cout << "mid= ";
		pointMid.Print();
		cout << "mid0= ";
		pointMid0.Print();
		cout << "mid1= ";
		pointMid1.Print();
		cout << "\ncenterDistance= " << centerDistance << endl;
		cout << "weight=" << weight << "!!" << endl;
		exit(0);
	    }
#endif 
	    netWeight[netCounter-1] = weight;

	    netCounter++;
	    //if( netCounter > fplan->m_nNets )
	    //     cout << "\nERR! netCounter > fplan->m_nNets\n";

	    continue;
	}


	// Case 3:
	// Part of the net is inside the partition.

	// Compute "segment" range: [min_value max_value]
	//double max_value = INT_MIN, min_value = INT_MAX;
	CPoint pointMax( INT_MIN, INT_MIN );
	CPoint pointMin( INT_MAX, INT_MAX );
	for( int j=0; j<(int)fplan->m_nets[netId].size(); j++ )
	{
	    if( externalPin[j] == false )
		continue;	// internal "free" modules

	    //if( considerMovable == false && fplan->m_modules[ fplan->m_pins[fplan->m_nets[netId][j]].moduleId ].m_isFixed == false )
	    //    continue;

	    double x, y;                   // Physical coordinate
	    fplan->GetPinLocation( fplan->m_nets[netId][j], x, y );

	    // 2D
	    pointMin.x = min( pointMin.x, x );
	    pointMax.x = max( pointMax.x, x );
	    pointMin.y = min( pointMin.y, y );
	    pointMax.y = max( pointMax.y, y );

	    // 1D
	    //if( cutDir == V_CUT )   // Check x
	    //         {
	    //             max_value = max( max_value, x );
	    //             min_value = min( min_value, x );
	    //         }
	    //         else    // Check y
	    //         {
	    //             max_value = max( max_value, y );
	    //             min_value = min( min_value, y );
	    //         }
	}

	//if( min_value < mid0 && max_value > mid1 )  
	if( pointMin.x <= pointMid0.x && 
		pointMin.y <= pointMid0.y &&
		pointMax.x >= pointMid1.x && 
		pointMax.y >= pointMid1.y )
	{
	    continue;   // The net is across two centers of the partitions (dont't care it!)
	}

	//if( min_value < mid0 )  min_value = mid0;
	//if( max_value > mid1 )  max_value = mid1;
	//cout << "\n min= ";
	//pointMin.Print();
	//cout << " max= " ;
	//pointMax.Print();

	//if( pointMin.x <= pointMid0.x ) pointMin.x = pointMid0.x;
	//if( pointMin.y <= pointMid0.y ) pointMin.y = pointMid0.y;
	//if( pointMin.x >= pointMid1.x ) pointMin.x = pointMid1.x;
	//if( pointMin.y >= pointMid1.y ) pointMin.y = pointMid1.y;

	//if( pointMax.x <= pointMid0.x ) pointMax.x = pointMid0.x;
	//if( pointMax.y <= pointMid0.y ) pointMax.y = pointMid0.y;
	//if( pointMax.x >= pointMid1.x ) pointMax.x = pointMid1.x;
	//if( pointMax.y >= pointMid1.y ) pointMax.y = pointMid1.y;

	// Pick the nodes inside the partition
	set<int> nodes;
	for( int j=0; j<(int)fplan->m_nets[netId].size(); j++ ) // For each pin
	{
	    if( externalPin[j] == true )
		continue;
	    nodes.insert( mapId[ fplan->m_pins[ fplan->m_nets[netId][j] ].moduleId ] );
	} 
	if( nodes.size() == 0 )
	    continue;


	//      // Calculate pin offsets
	//      CPoint boxP0( 0, 0 );   // left-bottom
	//      CPoint boxP1( 0, 0 );   // right-top
	//for( int j=0; j<(int)fplan->m_nets[netId].size(); j++ )
	//      {
	//          int pinId = fplan->m_nets[netId][j];
	//          if( fplan->m_pins[ pinId ].xOff < boxP0.x )   boxP0.x = fplan->m_pins[ pinId ].xOff; 
	//          if( fplan->m_pins[ pinId ].yOff < boxP0.y )   boxP0.y = fplan->m_pins[ pinId ].yOff; 
	//          if( fplan->m_pins[ pinId ].xOff > boxP1.x )   boxP1.x = fplan->m_pins[ pinId ].xOff; 
	//          if( fplan->m_pins[ pinId ].yOff > boxP1.y )   boxP1.y = fplan->m_pins[ pinId ].yOff; 
	//      }

	// WIRE (both at 0) -- pointMid0
	CPoint min0 = pointMin;
	CPoint max0 = pointMax;
	min0.x = min( min0.x, pointMid0.x );
	max0.x = max( max0.x, pointMid0.x );
	min0.y = min( min0.y, pointMid0.y );
	max0.y = max( max0.y, pointMid0.y );
	//	min0.x = min( min0.x, pointMid0.x-boxP0.x );
	//max0.x = max( max0.x, pointMid0.x-boxP0.y );
	//min0.y = min( min0.y, pointMid0.y+boxP1.x );
	//max0.y = max( max0.y, pointMid0.y+boxP1.y );
	double wire0 = Distance( min0, max0 );

	// WIRE (both at 1) -- pointMid1
	CPoint min1 = pointMin;
	CPoint max1 = pointMax;
	min1.x = min( min1.x, pointMid1.x );
	max1.x = max( max1.x, pointMid1.x );
	min1.y = min( min1.y, pointMid1.y );
	max1.y = max( max1.y, pointMid1.y );
	//	min1.x = min( min1.x, pointMid1.x-boxP0.x );
	//max1.x = max( max1.x, pointMid1.x-boxP0.y );
	//min1.y = min( min1.y, pointMid1.y+boxP1.x );
	//max1.y = max( max1.y, pointMid1.y+boxP1.y );
	double wire1 = Distance( min1, max1 );


	// WIRE (0 and 1) -- pointMid0 & pointMid1
	CPoint min01 = pointMin;
	CPoint max01 = pointMax;
	min01.x = min( min01.x, pointMid0.x );
	max01.x = max( max01.x, pointMid0.x );
	min01.y = min( min01.y, pointMid0.y );
	max01.y = max( max01.y, pointMid0.y );
	min01.x = min( min01.x, pointMid1.x );
	max01.x = max( max01.x, pointMid1.x );
	min01.y = min( min01.y, pointMid1.y );
	max01.y = max( max01.y, pointMid1.y );     
	//	min01.x = min( min01.x, pointMid0.x-boxP0.x );
	//max01.x = max( max01.x, pointMid0.x-boxP0.y );
	//min01.y = min( min01.y, pointMid0.y+boxP1.x );
	//max01.y = max( max01.y, pointMid0.y+boxP1.y );
	//	min01.x = min( min01.x, pointMid1.x-boxP0.x );
	//max01.x = max( max01.x, pointMid1.x-boxP0.y );
	//min01.y = min( min01.y, pointMid1.y+boxP1.x );
	//max01.y = max( max01.y, pointMid1.y+boxP1.y );   
	double wire01 = Distance( min01, max01 );

	// Determine the net weight
	//double weight;
	//double dis1_x = distance( pointMid0.x, pointMin.x, pointMax.x );
	//double dis1_y = distance( pointMid0.y, pointMin.y, pointMax.y );
	//double dis2_x = distance( pointMid1.x, pointMin.x, pointMax.x );
	//double dis2_y = distance( pointMid1.y, pointMin.y, pointMax.y );
	//double dis1 = dis1_x + dis1_y;
	//double dis2 = dis2_x + dis2_y;

	////double dis1 = distance( mid0, min_value, max_value );
	////double dis2 = distance( mid1, min_value, max_value );
	//weight = ( dis2 - dis1 );

	double weight = wire1 - wire0;

	// Add the fixed-node at proper partition
	if( abs(weight) > (mid1-mid0) * 2.0 * ubfactor * 0.01 )
	{
	    // add fixed node
	    if( weight >= 0 )
	    {
		nodes.insert(0);   // left fixed node
	    }
	    else
	    {
		nodes.insert(1);    // right fixed node
		weight = -weight;   // take the positive value
	    }
	}
	else
	{
	    weight = 0;
	}
	if( nodes.size() == 1 ) // The net only connects to 1 node. Discard it.
	    continue;

	// 2005/03/03: For a multi-terminal net, the nodes may be put into the different 
	//             partitions. So, the max cut is centerDistance instead of 
	if( nodes.size() > 2 )
	{
	    //cout << "centerDistance= " << mid1-mid0;
	    //cout << " dis1= " << dis1;
	    //cout << " dis2= " << dis2 << endl;
	    //int anotherNetWeight = (int)round( (mid1-mid0) - min(dis1, dis2) - weight - (max_value-min_value) );
	    //double minmaxDistance = Distance( pointMax,  pointMin );
	    //double anotherNetWeight = ( (centerDistance) - min(dis1, dis2) - weight - minmaxDistance );
	    double anotherNetWeight = wire01 - min( wire0, wire1 ) - weight;
#if 1
	    if( anotherNetWeight < -0.001 )
	    {
		cout << "\nWARNING!! anotherNetWeightL= " << anotherNetWeight <<  endl;
		cout << "L= " << mid1-mid0 <<  endl;
		cout << "net1= " << weight <<  endl;
		cout << "pointMid0 ";
		pointMid0.Print();
		cout << "pointMid1 ";
		pointMid1.Print();
		cout << "\npointMin ";
		pointMin.Print();
		cout << "pointMax ";
		pointMax.Print();
		anotherNetWeight = 0;
	    }
#endif

	    anotherNetWeight = round( anotherNetWeight * extraNetWeight );
	    if( anotherNetWeight > 0 )
	    {
		//cout << "nodes.size()= " << nodes.size() << " ";
		//cout << "weight= " << weight << " ";
		//cout << "anotherNetWeight= " << anotherNetWeight << endl;

		// Add the net :P
		set<int>::const_iterator ite;
		for( ite=nodes.begin(); ite!=nodes.end(); ite++ )
		{
		    if( *ite == 0 || *ite == 1 )
			continue; // Ignore fixed node

		    eind[pinCounter] = *ite;
		    pinCounter++;
		    //if( pinCounter > fplan->m_nPins )
		    //    cout << "\nERR! pinCounter > fplan->m_nPins\n";
		}
		eptr[netCounter] = pinCounter;
		netWeight[netCounter-1] = (int)anotherNetWeight;
		netCounter++;
		//if( netCounter > fplan->m_nNets )
		//    cout << "\nERR! netCounter > fplan->m_nNets\n";
	    } 
	} // extra net


	// Add the net
	if( weight < (mid1-mid0) * 2.0 * ubfactor * 0.01 )
	    continue;
	weight = round( weight * extraNetWeight );
	if( weight <= 0 )
	    continue;
	set<int>::const_iterator ite;
	for( ite=nodes.begin(); ite!=nodes.end(); ite++ )
	{
	    eind[pinCounter] = *ite;
	    pinCounter++;
	    //if( pinCounter > fplan->m_nPins )
	    //  cout << "\nERR! pinCounter > fplan->m_nPins\n";


	}
	eptr[netCounter] = pinCounter;
	netWeight[netCounter-1] = (int)weight;
	netCounter++;
	if( netCounter > fplan->m_nNets )
	    cout << "\nERR! netCounter > fplan->m_nNets\n";

    } // For each net
    netCounter--;

#if 0
		cout << "\n\t Nodes: " << moduleCounter ;
	    cout << "\t Edges: " << netCounter << " ("<< saveNets <<")\t";
	    cout << "\t Pins: " << pinCounter << endl;
#endif

    if( pinCounter > (int)fplan->m_pins.size() )
        cout << "WARN: pin#= " << (int)fplan->m_pins.size() << " pinCounter= " << pinCounter << endl;
    if( netCounter > (int)fplan->m_nets.size() )
        cout << "WARN: net#= " << (int)fplan->m_nets.size() << " netCounter= " << netCounter << endl;

	//////////////////////////////////////////////////////////////
	//// Output HGraph
	//cout << "moduleWeight:part ";
    //for( int i=0; i<moduleCounter; i++ )
    //{
	//	cout << i << "(" << moduleWeight[i] << ":" << part[i] << ") ";
    //}
    //cout << "\n";
    //cout << "moduleWeight: ";
    //for( int i=0; i<moduleCounter; i++ )
    //{
	   // cout << i << "(" << moduleWeight[i] << ") ";
    //}
    //cout << "\n";
    //cout << "eptr: \n";
    //for( int i=0; i<netCounter; i++ )
    //{
	   // cout << "net" << i << " " << eptr[i] << " [ ";
    //    for( int j=eptr[i]; j<eptr[i+1]; j++ )
    //    {
    //        cout << eind[j] << " ";
    //    }
    //    cout << "] weight= " << netWeight[i] << "\n";
    //}
    //cout << endl;


	//====================================
	// Check part_weight/cutsize overflow 
	// (TURN OFF TO SPEEDUP)
    //====================================
 //   int maxNodeWeight = 0;
	//int maxCutSize = 0;
	//for( int i=0; i<moduleCounter; i++ )
	//{
	//	maxNodeWeight += moduleWeight[i];
	//	if( (double)maxNodeWeight > pow(2.0, 30) )
 //       {
	//		cout << "\n\n**** MakeCPartition(): maxNodeWeight MAY OVERFLOW!! ****\n\n";
 //           cout << "nodeWeightSum = " << maxNodeWeight << endl;
 //           cout << "moduleCounter = " << i << "/" << moduleCounter << endl;
 //           cout << "freeArea0 = " << freeArea0 << endl;
 //           cout << "freeArea1 = " << freeArea1 << endl;
 //           cout << "dummyArea = " << dummyArea << endl;
 //           exit(0);
 //       }
	//}
	//for( int i=0; i<netCounter; i++ )
	//{
	//	maxCutSize += netWeight[i];
	//	if( (double)maxCutSize > pow(2.0, 30) )
 //       {
	//		cout << "\n\n**** MakeCPartition(): maxCutSize MAY OVERFLOW!! ****\n\n";
 //           cout << "cutSum = " << maxCutSize << endl;
 //           cout << "netCounter = " << i << "/" << netCounter << endl;
 //           exit(0);
 //       }
	//}
    //cout << "maxWeight= " << maxNodeWeight;
    //cout << "   maxCut= " << maxCutSize << endl;
    //for( int i=0; i<moduleCounter; i++ )
    //{
    //    if( moduleWeight[i] == 0 )
    //    {
    //        cout << "0";
    //        flush( cout );
    //        moduleWeight[i] = 1;
    //    }
    //}

	//====================================
	// Check hMetis inputs 
	// (TURN OFF TO SPEEDUP)
    //====================================
    //for( int i=0; i<moduleCounter; i++ )
    //{
    //    if( moduleWeight[i] < 0 )
    //    {
    //        cerr << "ERR! moduleWeight[" << i << "] < 0\n";
    //        exit(0);
    //    }
    //}
    //for( int i=0; i<netCounter; i++ )
    //{
    //    if( netWeight[i] <= 0 )
    //    {
    //        cerr << "ERR! netWeight[" << i << "] = " << netWeight[i] << "\n";
    //        exit(0);
    //    }
    //    for( int j=eptr[i]; j<eptr[i+1]; j++ )
    //    {
    //        if( eind[j] >= moduleCounter )
    //        {
    //            cerr << "ERR! eind[j] > moduleCounter\n";
    //            exit(0);
    //        }
    //    }
    //}

    /*
    if( usedFixArea0 + usedFixArea1 == 0 )
	ubfactor = 5;
    else
	ubfactor = 1;*/

    ubfactor = 1;
    
#if 0
        // Output HMETIS files
        ofstream out1( "part.hgr" );
        ofstream out2( "part.weight" );
        set<int> dup_test;
        out1 << netCounter << " " << moduleCounter << " 11\n"; 
        for( int i=0; i<netCounter; i++ )
        {
            dup_test.clear();
            out1 << netWeight[i] << " ";
            if( eptr[i] == eptr[i+1]-1 )
                cout << "Empty!!!";

            for( int j=eptr[i]; j<eptr[i+1]; j++ )
            {
                dup_test.insert( eind[j] );
                out1 << eind[j]+1 << " ";
            }
            if( dup_test.size() != (eptr[i+1] - eptr[i]) )
            {
                cout << "size !!";
            }
            out1 << endl;
        }
        for( int i=0; i<moduleCounter; i++ )
        {
            out1 << moduleWeight[i] << endl;
            out2 << part[i] << endl;
        }

#endif

        int types = param.hmetis_ctype;
        int ctype = 5;
        int min_edgecut = INT_MAX;
        int mincut_ctype = 0;
        bool lastIsTheBest = false;
        while( true )
	{
	    ctype = types % 10;
	    if( ctype == 0 )
		break;
	    assert( ctype >= 1 );
	    assert( ctype <= 5 );
	    types = types / 10;
	    fplan->part_options[2] = ctype;

	    lastIsTheBest = false;

	    // Initialize node position to run HMETIS.
	    for( int i=2; i<moduleCounter; i++ )
		part[i] = -1;

	    ////===================================
	    //// FOR TEST ONLY
	    //for( int i=2; i<moduleCounter; i++ )
	    //{
	    //    if( i< (moduleCounter-2)/2+2 )
	    //        part[i] = 0;
	    //    else
	    //        part[i] = 1;
	    //}
	    ////===================================

	    //if( partId < 10 )
	    //{
	    //	cout << "H";
	    //	flush( cout );
	    //  }
	    HMETIS_PartRecursive( moduleCounter, 
		    netCounter, 
		    moduleWeight, 
		    eptr,  
		    eind, 
		    netWeight,
		    2, // 2-way
		    ubfactor, // unbalanced factor
		    fplan->part_options, 
		    part,  
		    &edgecut );
	    //if( partId < 10 )
	    //{
	    //	cout << "h";
	    //	flush( cout );
	    //  }
/*
	    int result;
	    int edgecut2;
	    result = CFMPart::CPartition( moduleCounter, 
		    netCounter, 
		    moduleWeight, 
		    eptr, 
		    eind, 
		    netWeight,
		    2, // 2-way
		    ubfactor, // unbalanced factor
		    fplan->part_options, 
		    part, 
		    &edgecut2 );				    

	    
	    printf( "  %d --> %d   (%d)\n", edgecut, edgecut2, edgecut-edgecut2 );
	   */ 

	    //HMETIS_PartKway( moduleCounter, 
	    //					netCounter, 
	    //					moduleWeight, 
	    //					eptr, 
	    //					eind, 
	    //					netWeight,
	    //					2, 
	    //					5, 
	    //					fplan->part_options, 
	    //					part, 
	    //					&edgecut );

	    //cout << edgecut << " ";
	    if( edgecut < min_edgecut )
	    {
		memcpy( part_best, part,  sizeof(int)*moduleCounter );
		min_edgecut = edgecut;
		mincut_ctype = ctype;
		lastIsTheBest = true;
	    }


	    if( moduleCounter < 100 )
		break;  // no multi-level, we don't have to check other CType.

	} // while-ctype-loop

        // If the result in "part" is not the best, we need to copy from "part_best".
        if( !lastIsTheBest )
            memcpy( part, part_best,  sizeof(int)*moduleCounter );

        param.n_ctype_count[ mincut_ctype ] ++;     // CType stats

    //cout << "cut= " << edgecut << endl;
    //flush( cout );

	///////////////////////////////////////////////////////////////
	// The partition result is in "part[]"
	///////////////////////////////////////////////////////////////


	///////////////////////////////////////////////////////////////
	// Calculate area ratio
	///////////////////////////////////////////////////////////////
	double p0Area=0, p1Area=0;  // area of movable blocks in the partition
	double newArea0 = 0, newArea1 = 0;              // area of the partition
	double util0 = -1, util1 = -1;                    // utilization of the partition
	double newFixedArea0 = 0, newFixedArea1 = 0;    // area of fixed blocks in the partition
	vector<int> p0BlockList;
	vector<int> p1BlockList;
	for( int i=2; i<moduleCounter; i++ )  // Do not include the fixed nodes
	{
	    int id = reverseMapId[i];
	    assert( id < (int)fplan->m_modules.size() );
	    if( part[i] == 0 )  
	    {
		p0BlockList.push_back( id );
		p0Area += fplan->m_modules[id].m_area;
	    }
	    else if( part[i] == 1 ) 
	    {
		p1BlockList.push_back( id );
		p1Area += fplan->m_modules[id].m_area;
	    }
	}

	double ratio = (p0Area+usedFixArea0)/(p0Area+p1Area+usedFixArea0+usedFixArea1);
	//if( debugLevel >= 5 )
	//    cout << " CPartition ratio: " << ratio << endl;


    //double overfill;
    //if( m_partitions[partId].GetUtilization() > 1.0 )
    //    overfill = true;
    //else
    //    overfill = false;

    // CREATE SUB-PARTITIONS =================================================

    double x1, y1;	        // center of partition 1
    double x2, y2;              // center of partition 2
    double cutMid;	        // the "real" cut position
    CPoint center0, center1;



    //double maxCutlineShift = (mid1-mid0) * 2.0 * ubfactor * 0.01;

    double den0, den1;

    // STEP 1: Calculate new center point for the partition.
    
    // The smaller the "moveFactor" is, the larger the cutline can move.
    const double moveFactor = 0.33;     
    
    // If both regions' utilization are smaller the "utilTarget", stop moving the cutline
    const double utilTarget = 0.96;
    
    if( cutDir == V_CUT )
    {
	// V_CUT ()

	// Equally spacing for high utilization small regions
	//     if( m_partitions[partId].utilization>0.5 && moduleNumber<100 )
	//cutMid = ( left + ratio * (right-left) );	    // Equally spacing
	//     else
	//         cutMid = mid;   // Controlled by the unbalanced factor

	cutMid = mid;   // Controlled by the unbalanced factor

	bool bCutShifting = true; 

#if 1
	if( ( usedFixArea0 + usedFixArea1 ) == 0 && 
	      ratio != 0 && ratio != 1 )
	{
	    cutMid = ( left + ratio * (right-left) );	    // No pre-placed blocks, equally spacing
	    bCutShifting = false;
	}
	else
	    cutMid = mid;
#endif

	int whileCounter = 0;
	double leftBound = left + (right-left) * moveFactor;
	double rightBound = right - (right-left) * moveFactor;

	while( bCutShifting )
	{
	    whileCounter++;

	    newArea0 = (cutMid-left)*(top-bottom);
	    newArea1 = (right-cutMid)*(top-bottom);
	    newFixedArea0 = GetFixBlockAreaInPartition( partId, left, bottom, cutMid, top );
	    newFixedArea1 = GetFixBlockAreaInPartition( partId, cutMid, bottom, right, top );
	    den0 = (p0Area+newFixedArea0)/newArea0; // density
	    den1 = (p1Area+newFixedArea1)/newArea1; // density
	    util0 = (p0Area)/(newArea0-newFixedArea0);
	    util1 = (p1Area)/(newArea1-newFixedArea1);

	    if( whileCounter == 1 && util0 < utilTarget && util1 < utilTarget )
		break;

	    if( ( rightBound - leftBound ) < 0.01 )
		break;

	    if( fabs( util0 - util1 ) < 0.001 )
		break;

	    if( util0 > util1 )
	    {
		leftBound = cutMid;
		cutMid = 0.5 * ( rightBound + cutMid );
	    }
	    else
	    {
		rightBound = cutMid;
		cutMid = 0.5 * ( leftBound + cutMid );
	    }

	} // loop for finding V-cut
	if( whileCounter > 30 )
	{
	    cout << "WARNING: V-CUT whileCounter= " << whileCounter << endl;
	}
	//cutMid = bestCutMid;

	//assert( cutMid >= left );
	//assert( cutMid <= right );
	if( cutMid == left || cutMid == right || cutMid < left || cutMid > right )
	{
	    printf( "WARNING: ratio = %g, block # (%d %d), cut( %g %g %g )\n", 
		    ratio, p0BlockList.size(), p1BlockList.size(), left, cutMid, right );
	}


	pointMid.x = cutMid; // new mid point
	center0 = fplan->GetRegionWeightedCenter( left, cutMid, bottom, top );    // left
	center1 = fplan->GetRegionWeightedCenter( cutMid, right, bottom, top );   // right

	assert( center0.x > left );
	assert( center0.x < cutMid );
	assert( center0.y < top );
	assert( center0.y > bottom );
	assert( center1.x > cutMid );
	assert( center1.x < right );
	assert( center1.y < top );
	assert( center1.y > bottom );

	//pointMid.y = 0.5 * ( center0.y + center1.y ); 

	if( false == bUse2D )
	{
	    // 1D: Don't use y-coordinate for V-cut
	    center0.y = pointMid.y;
	    center1.y = pointMid.y;
	}

	// weighted-center
	x1 = center0.x;
	y1 = center0.y;
	x2 = center1.x;
	y2 = center1.y;
	
	/*
	// real center (TEST)
	x1 = (left + cutMid) * 0.5;
	x2 = (cutMid + right) * 0.5;
	y1 = y2 = (top + bottom) * 0.5;
	*/

	if( true == moveHalf )
	{
	    x1 = cutMid + (x1 - cutMid) * moveRatio;
	    x2 = cutMid + (x2 - cutMid) * moveRatio;
	    y1 = pointMid.y + (y1 - pointMid.y) * moveRatio;
	    y2 = pointMid.y + (y2 - pointMid.y) * moveRatio;

	    //x1 = (x1 + cutMid) * 0.5;
	    //x2 = (x2 + cutMid) * 0.5;
	    //y1 = (y1 + pointMid.y) * 0.5;
	    //y2 = (y2 + pointMid.y) * 0.5;
	}

    }
    else
    {
	// H_CUT
	cutMid = mid;
	// Align row (should we re-calculate the cutMid?)
	//cutMid = round( (top-bottom)/fplan->m_rowHeight * ratio ) * fplan->m_rowHeight + bottom;  

	//if( m_partitions[partId].utilization>0.5 && param.bFractionalCut )
	//    cutMid = ( bottom + ratio * (top-bottom) );	    // Equally spacing

	bool bCutShifting = true;
#if 1
	if( (usedFixArea0 + usedFixArea1) == 0 && param.bFractionalCut )
	{
	    cutMid = ( bottom + ratio * (top-bottom) );	    // No preplaced blocks, equally spacing
	    bCutShifting = false;
	}
	else
	    cutMid = mid;
#endif

	if( param.bFractionalCut == false )
	    cutMid = mid;

	int dir = 0;
	double bestCutMid = cutMid;
	double smallestOver = 1e7;
	int whileCounter = 0;
	double bottomBound = bottom + (top-bottom) * moveFactor;
	double topBound = top - (top-bottom) * moveFactor;
	while( bCutShifting )
	{
	    whileCounter++;

	    newArea0 = (right-left)*(cutMid-bottom);
	    newArea1 = (right-left)*(top-cutMid);
	    newFixedArea0 = GetFixBlockAreaInPartition( partId, left, bottom, right, cutMid );
	    newFixedArea1 = GetFixBlockAreaInPartition( partId, left, cutMid, right, top );
	    den0 = (p0Area+newFixedArea0)/newArea0; // density
	    den1 = (p1Area+newFixedArea1)/newArea1; // density
	    util0 = (p0Area)/(newArea0-newFixedArea0);
	    util1 = (p1Area)/(newArea1-newFixedArea1);

	    if( whileCounter == 1 && util0 < utilTarget && util1 < utilTarget )
	    {
		bestCutMid = cutMid;
		break;
	    }

	    if( param.bFractionalCut ) 
	    {
		if( ( topBound - bottomBound ) < 0.01 )
		{
		    bestCutMid = cutMid;
		    break;
		}

		if( fabs( util0 - util1 ) < 0.001 )
		{
		    bestCutMid = cutMid;
		    break;
		}

		if( util0 > util1 )
		{
		    bottomBound = cutMid;
		    cutMid = 0.5 * ( topBound + cutMid );
		}
		else
		{
		    topBound = cutMid;
		    cutMid = 0.5 * ( bottomBound + cutMid );
		}
	    }
	    else
	    {

		// Move the cut-line
		if( dir == 0 )
		{
		    if( util0 > util1 )
		    {
			//printf( "UP: %f %f\n", util0, util1 );
			dir = 1;    // Up
		    }
		    else 
		    {
			//printf( "DOWN: %f %f\n", util0, util1 );
			dir = 2;    // Down
		    }
		}

		// minimize the utilization difference
		double over = 0;
		over = fabs(util0 - util1);
		over *= over;

		if( over < smallestOver )
		{
		    bestCutMid = cutMid;
		    smallestOver = over;
		    if( dir == 1 )      
			cutMid += fplan->m_rowHeight;  // Move up
		    else             
			cutMid -= fplan->m_rowHeight;  // Move down
		}
		else
		{
		    break;
		}
		if( cutMid >= top || cutMid <= bottom )
		    break;

	    }

	} // while loop for find h-cut
	if( whileCounter > 30 )
	{
	    cout << "WARNING: H-CUT whileCounter= " << whileCounter << endl;
	}
	cutMid = bestCutMid;

	assert( cutMid > bottom );
	assert( cutMid < top );

	pointMid.y = cutMid; // new mid point

	// weighted center
	center0 = fplan->GetRegionWeightedCenter( left, right, bottom, cutMid );
	center1 = fplan->GetRegionWeightedCenter( left, right, cutMid, top );
	
	//assert( ! (center0.x == 0 && center0.y == 0) );
	//assert( ! (center1.x == 0 && center1.y == 0) );

	assert( center0.x > left );
	if( center0.x >= right )
	{
	    cout << left << " " << right << " " << bottom << " " << top << endl;
	}
	assert( center0.x < right );
	assert( center0.y < cutMid );
	assert( center0.y > bottom );
	assert( center1.x > left );
	assert( center1.x < right );
	assert( center1.y < top );
	assert( center1.y > cutMid );

// 	assert( cutMid > 0 );

	//pointMid.x = 0.5 * ( center0.x + center1.x );

	if( false == bUse2D )
	{
	    // 1D: Don't use x-coordinate for H-cut
	    center0.x = pointMid.x;
	    center1.x = pointMid.x;
	}

	// weighted center
	x1 = center0.x;
	y1 = center0.y;
	x2 = center1.x;
	y2 = center1.y;

	/*
	// real center (TEST)
	y1 = (bottom + cutMid) * 0.5;
	y2 = (top + cutMid) * 0.5;
	x1 = x2 = (left + right) * 0.5;
	*/
	
	if( true == moveHalf )
	{
	    x1 = pointMid.x + (x1 - pointMid.x) * moveRatio;
	    x2 = pointMid.x + (x2 - pointMid.x) * moveRatio;
	    y1 = cutMid + (y1 - cutMid) * moveRatio;
	    y2 = cutMid + (y2 - cutMid) * moveRatio;

	    //x1 = (x1 + pointMid.x) * 0.5;
	    //x2 = (x2 + pointMid.x) * 0.5;
	    //y1 = (y1 + cutMid) * 0.5;
	    //y2 = (y2 + cutMid) * 0.5;
	}

    }


    //=======================================================================
    // STEP 2: Move the module to the center of the partition (center align)
    //=======================================================================
    // Since we moved the cutline, we need to check if there is free space 
    // to put the blocks in the sub-region.
    bool allLeft = false;
    bool allRight = false;
    double newFreeArea0, newFreeArea1;
    if( cutDir == V_CUT )
    {
        newFixedArea0 = GetFixBlockAreaInPartition( partId, left, bottom, cutMid, top );
        newFixedArea1 = GetFixBlockAreaInPartition( partId, cutMid, bottom, right, top );
        newArea0 = (cutMid-left)*(top-bottom);
        newArea1 = (right-cutMid)*(top-bottom);
        newFreeArea0 = newArea0 - newFixedArea0;
        newFreeArea1 = newArea1 - newFixedArea1;
        if( newFreeArea0 < 0.001 )
            allRight = true;
        else if( newFreeArea1 < 0.001 )
            allLeft = true;
    }
    else // H_CUT
    {
        newFixedArea0 = GetFixBlockAreaInPartition( partId, left, bottom, right, cutMid );
        newFixedArea1 = GetFixBlockAreaInPartition( partId, left, cutMid, right, top );
        newArea0 = (cutMid-bottom)*(right-left);
        newArea1 = (top-cutMid)*(right-left);
        newFreeArea0 = newArea0 - newFixedArea0;
        newFreeArea1 = newArea1 - newFixedArea1;
        if( newFreeArea0 < 0.001 )
            allRight = true;
        else if( newFreeArea1 < 0.001 )
            allLeft = true;
    }
    assert( ! (allLeft==true && allRight==true) );

    //if( allLeft )
    //    cout << "L";
    //if( allRight )
    //    cout << "R";

    // Move blocks
    int part0BlockCount = 0;
    int part1BlockCount = 0;
    for( int i=2; i<moduleCounter; i++ )           // NOTE: moduleNumber = moduleCounter - 2
    {
	assert( reverseMapId.find(i) != reverseMapId.end() );
	int id = reverseMapId[i];
	assert( id < (int)fplan->m_modules.size() );

	// Move the module to the partition
	int pos = -1;
	if( part[i] == 0 )
	    pos = 0;
	if( part[i] == 1 )
	    pos = 1;
	if( allLeft )
	    pos = 0;
	if( allRight )
	    pos = 1;
	assert( pos == 0 || pos == 1 );
	if( pos == 0 )
	{
	    fplan->MoveModuleCenter( id, x1, y1 );
	    part0BlockCount++;
	}
	else if( pos == 1 )
	{
	    fplan->MoveModuleCenter( id, x2, y2 );
	    part1BlockCount++;
	}
	else
	{
	    cout << "Error, no partition result for module " << id << endl;
	}
    }

    //if( part0BlockCount == 0 )
    //    cout << "p0# = 0\n";
    //if( part0BlockCount == 0 )
    //    cout << "p1# = 0\n";


    if( noSubRegion == true )   // Don't create sub-partitions, move blocks only
        return false;


    //=======================================================================
	// STEP 3: Create partitions
    //=======================================================================
    int part0Id = -1;
    int part1Id = -1;
    if( cutDir == V_CUT )
    {
	// left
	if( m_partitions[partId].childPart0 > 0 )
	{
	    part0Id = AddPartition( partId, left, bottom, cutMid, top, m_partitions[partId].childPart0 );
	}
	else
	{
	    part0Id = AddPartition( partId, left, bottom, cutMid, top );
	    m_partitions[partId].childPart0 = part0Id;
	}
	// right
	if( m_partitions[partId].childPart1 > 0 )
	{
	    part1Id = AddPartition( partId, cutMid, bottom, right, top, m_partitions[partId].childPart1 );
	}
	else
	{
	    part1Id = AddPartition( partId, cutMid, bottom, right, top );
	    m_partitions[partId].childPart1 = part1Id;
	}
    }
    else // H_CUT
    {

	// bottom 
	if( m_partitions[partId].childPart0 > 0 )
	{
	    part0Id = AddPartition( partId, left, bottom, right, cutMid, m_partitions[partId].childPart0 );
	}
	else
	{
		part0Id = AddPartition( partId, left, bottom, right, cutMid );		
		m_partitions[partId].childPart0 = part0Id;
	}
	// top
	if( m_partitions[partId].childPart1 > 0 )
	{
	    part1Id = AddPartition( partId, left, cutMid, right, top, m_partitions[partId].childPart1 );
	}
	else
	{
		part1Id = AddPartition( partId, left, cutMid, right, top );
		m_partitions[partId].childPart1 = part1Id;
	}
    }

    // Add peer partition id
    if( part0Id > 0 && part1Id > 0 )
    {
        m_partitions[part0Id].peerPartitionId = part1Id;
        m_partitions[part1Id].peerPartitionId = part0Id;
    }

    // Check freeArea
    //m_partitions[part0Id].Print();
    //m_partitions[part1Id].Print();
    if( (!allRight && part0Id > 0 && m_partitions[part0Id].GetFreeArea() == 0) || 
        (!allLeft && part1Id > 0 && m_partitions[part1Id].GetFreeArea() == 0 ) )
    {
        cout << "WARNING! freeArea of the new part is zero\n";
        if( part0Id > 0 )
        {
            cout << "PART " << part0Id;
            m_partitions[part0Id].Print();
        }
        if( part1Id > 0 )
        {
            cout << "PART " << part1Id;
            m_partitions[part1Id].Print();
        }
        cout << "newArea= " << newArea0 << ":" << newArea1 << endl;
        cout << "newFixedArea= " << newFixedArea0 << ":" << newFixedArea1 << endl;
        cout << "util= " << util0 << ":" << util1 << endl;
        //exit(0);
    }

    ///////////////////////////////////////////////////////////////
	// Return values
    new1 = (part0Id == 0) ? -1 : part0Id;
    new2 = (part1Id == 0) ? -1 : part1Id;

    //cout << new1 << " " << new2 ;

    if( new1 > 0 || new2 > 0 )
    	return true;
    else
        return false;

}

double CMinCutCluster::GetFixBlockAreaInPartition( const int partId, 
	  const double& left, const double& bottom, 
	  const double& right, const double& top )
{
    double a = 0;
    int id;
    assert( right > left );
    assert( top > bottom );
    //cout << "Fixed modules= " << m_partitions[partId].fixModuleList.size() << endl;
    for( int i=0; i<(int)m_partitions[partId].fixModuleList.size(); i++ )
    {
	id = m_partitions[partId].fixModuleList[i];
	a += getOverlapArea( left, bottom, right, top, fplan->m_modules[id].m_x, fplan->m_modules[id].m_y,
		fplan->m_modules[id].m_x+fplan->m_modules[id].m_width, fplan->m_modules[id].m_y+fplan->m_modules[id].m_height );
    }
    return a;
}


double CMinCutCluster::GetPartHPWL( int partId )
{
    double HPWL = 0;
    for( int i=0; i<(int)m_partitions[partId].netList.size(); i++ )
    {
        HPWL += fplan->GetNetLength( m_partitions[partId].netList[i] );
    }
    return HPWL;
}


CUTDIR CMinCutCluster::GetCutDirForPart( int partId )
{
	CUTDIR cutDir;
	// The curDir depends on the module width/height.
	double long_side, short_side;
	double max_width = 0.0;
	double max_module_length = 0.0;
	for( int i=0; i<(int)m_partitions[partId].moduleList.size(); i++ )
	{
		if( fplan->m_modules[ m_partitions[partId].moduleList[i] ].m_width > 
			fplan->m_modules[ m_partitions[partId].moduleList[i] ].m_height )
		{
			long_side = fplan->m_modules[ m_partitions[partId].moduleList[i] ].m_width;
			short_side = fplan->m_modules[ m_partitions[partId].moduleList[i] ].m_height;
		}
		else
		{
			short_side = fplan->m_modules[ m_partitions[partId].moduleList[i] ].m_width;
			long_side = fplan->m_modules[ m_partitions[partId].moduleList[i] ].m_height;
		}

		if( long_side > max_module_length )
			max_module_length = long_side;
		
		if( fplan->m_modules[ m_partitions[partId].moduleList[i] ].m_width > max_width )
		    max_width = fplan->m_modules[ m_partitions[partId].moduleList[i] ].m_width;

	}
	
	double partW = m_partitions[partId].right-m_partitions[partId].left;
	double partH = m_partitions[partId].top-m_partitions[partId].bottom;
	//if( max_module_length < partW &&
	//	max_module_length < partH )	// any dir
	//{
	//	if( partW >= partH )
	//		cutDir = V_CUT; 
	//	else
	//		cutDir = H_CUT;
	//}
	//else if( /*max_module_length < partW &&*/ max_module_length > 0.7 * partW ) // horizontal
	//{
	//	cutDir = H_CUT;
	//}
	//else if( /*max_module_length < partH &&*/ max_module_length > 0.7 * partH )	// vertical
	//{
	//	cutDir = V_CUT;
	//}
	//else
	//{
 //       //cout << "Cannot fit all modules!\n";
	//	if( partW >= partH )
	//		cutDir = V_CUT; 
	//	else
	//		cutDir = H_CUT;
	//}

    //----------------------------------------------

    // 2005/02/17

    //double   restrictAR = 1.0;
    double restrictAR = param.aspectRatio;

    if( max_module_length > partH * 0.1 )
        restrictAR = 1.5;
    //else 
    //    restrictAR = 1.5;

    double ar = (double)partW/partH;
    if( ar > restrictAR )
        cutDir = V_CUT;
    else
        cutDir = H_CUT;


    // 2005-12-03 (for long std-cells)
    if( max_width > partW * 0.75 )
	cutDir = H_CUT;
    
    if( partH <= fplan->m_rowHeight )	// cannot smaller than a row
	    cutDir = V_CUT;

    return cutDir;
}

void CMinCutCluster::SavePartBlockList( int partId )
{
    m_partitions[partId].moduleListBak = m_partitions[partId].moduleList;
    m_partitions[partId].netListBak = m_partitions[partId].netList;
}

void CMinCutCluster::SavePartBlockLocation( int partId )
{
    CPartition::BlockPosition blockPos;
    m_partitions[partId].blockPosList.clear();
    for( int i=0; i<(int)m_partitions[partId].moduleList.size(); i++ )
    {
        blockPos.id = m_partitions[partId].moduleList[i];
        blockPos.x  = fplan->m_modules[blockPos.id].m_x;
        blockPos.y  = fplan->m_modules[blockPos.id].m_y;
        m_partitions[partId].blockPosList.push_back( blockPos );
    }
    m_partitions[partId].blockPosList.resize( m_partitions[partId].blockPosList.size() );
}


void CMinCutCluster::RestorePartBlockList( int partId )
{
    m_partitions[partId].moduleList = m_partitions[partId].moduleListBak;
    m_partitions[partId].netList = m_partitions[partId].netListBak;
}

void CMinCutCluster::RestorePartBlockLocation( int partId )
{
    for( int i=0; i<(int)m_partitions[partId].blockPosList.size(); i++ )
    {
        CPartition::BlockPosition& blockPos = m_partitions[partId].blockPosList[i];
        fplan->SetModuleLocation( blockPos.id, blockPos.x, blockPos.y ); 
    }
}


bool CMinCutCluster::NewPartMem(void)
{
    // Allocate memory for partitioning.
    // Our program will not release memory until the end of the program.
    part         = new int[fplan->m_nModules+2];	// 2 fixed node
    part_best    = new int[fplan->m_nModules+2];	// 2 fixed node
    moduleWeight = new int[fplan->m_nModules+2];	// 2 fixed node
    netWeight    = new int[int(fplan->m_nNets*1.10)];       // multi 1.10 to avoid too much Exact Net Weight
    eptr         = new int [int(fplan->m_nNets*1.10)+1];
    eind         = new int [int(fplan->m_nPins*1.10)];
    if( part == NULL || moduleWeight == NULL || netWeight == NULL || eptr == NULL || eind == NULL )
    {
	cout << "Out of memory\n";
	exit(-1);
    }
    return true;
}

void CMinCutCluster::DeletePartMem(void)
{
    delete [] part;
    delete [] moduleWeight;
    delete [] netWeight;
    delete [] eptr;
    delete [] eind;
}



