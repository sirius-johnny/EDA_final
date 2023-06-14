#include <list>
#include <vector>
#include <utility>

#include "mincutplacer.h"
#include "Partition.h"
#include "libhmetis.h"	

#include "PlaceDBQP.h"	// qp placement (donnie) 2006-02-04

int gCurrentLevel;

CMinCutPlacer::CMinCutPlacer(CPlaceDB& db)
{
    fplan = &db;	
}


CMinCutPlacer::~CMinCutPlacer()
{
}
 
/*
// Create dummy blocks for the non-placing sites
int CMinCutPlacer::CreateDummyFixedBlock()  // TODO: move out of mincutplacer.cpp
{

    double currentX;
    char name[50];
    int counter = 0;
    for( int i=0; i<(int)fplan->m_sites.size(); i++ )
    {

	currentX = fplan->m_coreRgn.left;
	for( int j=0; j<(int)fplan->m_sites[i].m_interval.size(); j+=2 )
	{
	    if( fplan->m_sites[i].m_interval[j] > currentX )
	    {
		sprintf( name, "__%d", counter );
		fplan->AddModule( name, float(fplan->m_sites[i].m_interval[j]-currentX), fplan->m_sites[i].m_height, true );
		fplan->SetModuleLocation( (int)fplan->m_modules.size()-1, currentX, fplan->m_sites[i].m_bottom );
		counter++;
	    }
	    currentX = fplan->m_sites[i].m_interval[j+1];
	}
	if( currentX < fplan->m_coreRgn.right )
	{
	    sprintf( name, "__%d", counter );
	    fplan->AddModule( name, fplan->m_coreRgn.right-currentX, fplan->m_sites[i].m_height, true );
	    fplan->SetModuleLocation( (int)fplan->m_modules.size()-1, currentX, fplan->m_sites[i].m_bottom );
	    counter++;
	}

    } // for m_site

    return counter;
}
*/


// Create dummy blocks for distributing whitespace
int CMinCutPlacer::CreateDummyBlock()
{
    return 0;
    double targetUtil = 0.8;


    double coreArea = (fplan->m_coreRgn.right-fplan->m_coreRgn.left)*(fplan->m_coreRgn.top-fplan->m_coreRgn.bottom);
    double fixedArea = 0;
    double movableArea = 0;
    for( int i=0; i<(int)fplan->m_modules.size(); i++ )
    {
	if( fplan->m_modules[i].m_isFixed )
	{
	    fixedArea += getOverlapArea( fplan->m_coreRgn.left, fplan->m_coreRgn.bottom, fplan->m_coreRgn.right, fplan->m_coreRgn.top,
		    fplan->m_modules[i].m_x, fplan->m_modules[i].m_y, fplan->m_modules[i].m_x+fplan->m_modules[i].m_width, fplan->m_modules[i].m_y+fplan->m_modules[i].m_height );
	}
	else
	{
	    movableArea += fplan->m_modules[i].m_area;
	}
    }
    double util = (fixedArea+movableArea)/coreArea;
    cout << "Utilization= " << util << endl;

    int blockNumber = 0;
    if( util < targetUtil )
    {
	// dummy blocks (1:4)
	blockNumber = int(( targetUtil * coreArea - fixedArea - movableArea ) / fplan->m_rowHeight / fplan->m_rowHeight / 4.0 ) ;

	char name[50];

	for( int i=0; i<blockNumber; i++ )
	{
	    sprintf( name, "___%d", i );
	    fplan->AddModule( name, fplan->m_rowHeight * 4.0, fplan->m_rowHeight, false );
	}

    }
    return blockNumber;
}

void CMinCutPlacer::RestoreCoreRgnShrink(void)
{
    fplan->m_coreRgn = fplan->m_coreRgnShrink;
}

void CMinCutPlacer::ShrinkCoreUtil( double targetUtil )
{
    if( param.bShow )
        cout << "Target core utilization= " << targetUtil << endl;
    
    double factorUp = 2.0;
    double factorLow = 0.2;
    double factor = 0.5;

    //fplan->m_coreRgn.Print();
    
    double movArea = 0;
    for( int i=0; i<(int)fplan->m_modules.size(); i++ )
    {
        if( !fplan->m_modules[i].m_isFixed )
            movArea += fplan->m_modules[i].m_area;
    }
    
    double coreArea = (fplan->m_coreRgn.top - fplan->m_coreRgn.bottom) * 
	              (fplan->m_coreRgn.right - fplan->m_coreRgn.left);
    double fixArea  = fplan->GetFixBlockArea( fplan->m_coreRgn.left, fplan->m_coreRgn.bottom,  
	                                      fplan->m_coreRgn.right, fplan->m_coreRgn.top );
    double util = movArea / (coreArea-fixArea);
    //printf( "Total movable macro/cell area = %g\n", movArea );
    //printf( "Initial utilization = %g\n", util );
    while( true )
    {
        fplan->m_coreRgn = fplan->m_coreRgn2;
        ShrinkCoreRgn( factor );
        fixArea = fplan->GetFixBlockArea( fplan->m_coreRgn.left, fplan->m_coreRgn.bottom,  fplan->m_coreRgn.right, fplan->m_coreRgn.top );
        coreArea = (fplan->m_coreRgn.top - fplan->m_coreRgn.bottom)*(fplan->m_coreRgn.right - fplan->m_coreRgn.left);
        
	util = movArea / (coreArea-fixArea);    // "density" for ISPD2006 contest

        //printf( "coreArea= %g    fixArea= %g    util= %g\n", coreArea, fixArea, util );

        if( fabs( util - targetUtil ) < 0.001 )
            break;
        
        if( (factorUp - factorLow) < 0.0005 )
        {
            cout << "Sorry, wrong utilization.";
            exit(0);
        }

        if( util > targetUtil )
        {
            factorLow = factor;
            factor = 0.5 * (factor + factorUp);
        }
        else
        {
            factorUp = factor;
            factor = 0.5 * (factor + factorLow);
        }   
    }

    param.coreShrinkFactor = factor;
    if( param.bShow )
    {
        cout << "\nShrink factor= " << factor << endl;
        cout << "Utilization= " << util << endl;
        cout << "Core= ";
        fplan->m_coreRgn.Print();
    }
}

void CMinCutPlacer::ShrinkCoreRgn(double factor)
{
    //cout << "Core shrink factor= " << factor << endl;

    double height = (fplan->m_coreRgn.top - fplan->m_coreRgn.bottom) * 0.5 * factor;
    double width  = (fplan->m_coreRgn.right - fplan->m_coreRgn.left) * 0.5 * factor;

    /*
    // 2006-02-05 (donnie) use QP to determine the core center
    static bool firstTime = true;
    static CPoint center;
    if( firstTime )
    {
	CPlaceDBQPPlacer* pqplace = new CPlaceDBQPPlacer( *fplan );
	pqplace->QPplace();
	delete pqplace;

	center.x = 0;
	center.y = 0;
	double totalArea = 0;
	for( unsigned int id=0; id<fplan->m_modules.size(); id++ )
	{
	    if( fplan->m_modules[id].m_isFixed )
		continue;
	    center.x += fplan->m_modules[id].m_cx * fplan->m_modules[id].m_area;
	    center.y += fplan->m_modules[id].m_cy * fplan->m_modules[id].m_area;
	    totalArea += fplan->m_modules[id].m_area;
	}
	center.x /= totalArea;
	center.y /= totalArea;

	firstTime = false;
    }
    */
    CPoint center = fplan->GetCoreCenter();
    
    double newTop    = center.y + height;
    double newBottom = center.y - height;
    double newLeft   = center.x - width;
    double newRight  = center.x + height;

    if( !param.bFractionalCut )
    {
        // align rows
	newTop = round( (newTop - fplan->m_coreRgn.bottom) / fplan->m_rowHeight ) * 
	         fplan->m_rowHeight + fplan->m_coreRgn.bottom;
	newBottom = round( (newBottom - fplan->m_coreRgn.bottom) / fplan->m_rowHeight ) * 
	            fplan->m_rowHeight + fplan->m_coreRgn.bottom;
    }

    if( newTop < fplan->m_coreRgn.top )        fplan->m_coreRgn.top    = newTop;
    if( newBottom > fplan->m_coreRgn.bottom )  fplan->m_coreRgn.bottom = newBottom;
    if( newLeft > fplan->m_coreRgn.left )      fplan->m_coreRgn.left   = newLeft;
    if( newRight < fplan->m_coreRgn.right )    fplan->m_coreRgn.right  = newRight;

    //cout << "Core center: (" << cx << "," << cy << ") --> (" << center.x << "," << center.y << ")\n"; 
    //cout << "Core region: "; 
    //fplan->m_coreRgn.Print();
}

void CMinCutPlacer::ShrinkCoreWidth(double factor)
{
    cout << "Core width shrink factor= " << factor << endl;

    double width  = (fplan->m_coreRgn.right - fplan->m_coreRgn.left) * factor;
    fplan->m_coreRgn.right  = fplan->m_coreRgn.left + width;   

    cout << "Core region: "; 
    fplan->m_coreRgn.Print();
}

void CMinCutPlacer::ShrinkCore(void)
{
    if( param.coreUtil > 0 )
        ShrinkCoreUtil( param.coreUtil );
    else if( param.coreShrinkFactor != 1.00 )
        ShrinkCoreRgn( param.coreShrinkFactor );
    else if( param.coreShrinkWidthFactor != 1.00 )
        ShrinkCoreWidth( param.coreShrinkWidthFactor );

    fplan->m_coreRgnShrink = fplan->m_coreRgn;
}


void CMinCutPlacer::Init()
{
    
    // Set all blocks to the center of the core region
    double cx = 0.5* (fplan->m_coreRgn.left + fplan->m_coreRgn.right );
    double cy = 0.5* (fplan->m_coreRgn.bottom + fplan->m_coreRgn.top );
    for( int i=0; i<(int)fplan->m_modules.size(); i++ )
    {
	fplan->MoveModuleCenter( i, cx, cy );
    }

    /* 
    // Add the first partition
    AddPartition( -1, 
	    fplan->m_coreRgn.left, 
	    fplan->m_coreRgn.bottom, 
	    fplan->m_coreRgn.right, 
	    fplan->m_coreRgn.top );
    */

    int sliceX = 1;
    int sliceY = 1;
    int binNumber = sliceX * sliceY;
    printf( "Initial grid %d * %d\n", sliceX, sliceY );
    
    double width = fplan->m_coreRgn.right - fplan->m_coreRgn.left;
    double height = fplan->m_coreRgn.top - fplan->m_coreRgn.bottom;
    width /= sliceX;
    height /= sliceY;
    
    for( int i=0; i<sliceX; i++ )
    {
	for( int j=0; j<sliceY; j++ )
	{
	    AddPartition( -1, 
		    fplan->m_coreRgn.left + width*(i), fplan->m_coreRgn.bottom + height*(j), 
		    fplan->m_coreRgn.left + width*(i+1), fplan->m_coreRgn.bottom + height*(j+1) );
	}
    }
  
    // move to center 
    for( int r=0; r<binNumber; r++ )
    {
	set<int> nets;
	for( int m=0; m<(int)m_partitions[r].moduleList.size(); m++ )
	{
	    double x = ( m_partitions[r].left + m_partitions[r].right ) * 0.5;
	    double y = ( m_partitions[r].bottom + m_partitions[r].top ) * 0.5;
	    fplan->MoveModuleCenter( m_partitions[r].moduleList[m], x, y );
	}
	
	// QP init partition
	//QPPartition( r );
    }
    printf( "WL = %g\n", fplan->CalcHPWL() );
    

    
#   if 0
    // Calc cutsize
    int cut = 0;
    int soed = 0;
    vector<int> groupId;
    groupId.resize( fplan->m_modules.size() );
    for( int i=0; i<(int)groupId.size(); i++ )
	groupId[i] = i;
    for( int r=0; r<binNumber; r++ )
    {
	if( m_partitions[r].moduleList.size() >= 2 )
	{
	    for( int m=1; m<(int)m_partitions[r].moduleList.size(); m++ )            
		groupId[ m_partitions[r].moduleList[m] ] = m_partitions[r].moduleList[0];
	    for( int m=0; m<(int)m_partitions[r].fixModuleList.size(); m++ )            
		groupId[ m_partitions[r].fixModuleList[m] ] = m_partitions[r].moduleList[0];
	}
	soed += m_partitions[r].netList.size();
    }
    for( int n=0; n<(int)fplan->m_nets.size(); n++ )
    {
	set<int> groups;
	for( int t=0; t<(int)fplan->m_nets[n].size(); t++ )
	{
	    int id = fplan->m_pins[ fplan->m_nets[n][t] ].moduleId;
	    groups.insert( groupId[id] );
	}
	if( groups.size() > 1 )
	    cut++;
    }
    printf( "cut= %d  SOED= %d\n", cut, soed );
    exit(0);
#endif
    
    if( binNumber == 1 )
    {	
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
}

void CMinCutPlacer::RecursivePartition()
{
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

    ///////////////
    double startTime = seconds();
    int level = 1;
    bool stop = false;
    while( true )
    {
	fplan->SaveBlockLocation();
	if( false == LevelPartition() ) 
	    stop = true;	// No more levels

	if( stop )
	    break;

	level++;

	if( param.bPlot )
	{
	    char filename[200];
	    sprintf( filename, "%s_glevel-%03d.plt", param.outFilePrefix.c_str(),level );
	    
	    fplan->OutputGnuplotFigure( filename, false );
    	    sprintf( filename, "%s_glevel-%03d.pl", param.outFilePrefix.c_str(),level );
    	    fplan->OutputPL( filename );

	}
    }
    cout << "PartTime= " << (seconds()-startTime)  
	 << " s (total= " << (seconds()-startTime)/60.0 << " m)" << endl;  

    /////////////////

    m_partitions.clear();
    DeletePartMem();
}

bool CMinCutPlacer::LevelPartition()
{
    //static int memoryCollectCounter = 0;
    static int level = 0;
    static int startPartId = 0;     // The partitionId starts from 0.
    level++;

    gCurrentLevel = level;
    
    // 2005/3/9
    if( level > 100 )
        return false;

    double levelStartTime = seconds();

    CUTDIR cutDir = NONE_CUT;

    int endPartId = GetPartitionNumber();
    int totalPart = endPartId - startPartId;
    int edgecut, new1, new2;

    cout << "\n%%% LEVEL " << level << " (" << totalPart << " parts) %%% ";
    flush( cout );

    // Create the sequence of the partitioning
    int partId;
    list<DoubleInt> partSeq;
    DoubleInt di;

    // sort from large partition to small partition
    for( partId = startPartId; partId < endPartId; partId++ )
    {
	if( m_partitions[partId].bDone == false )
	{
	    CalcPartPriority( partId );
	    partSeq.push_back( di );
	    partSeq.back().d = m_partitions[partId].priority;
	    partSeq.back().i = partId;
	}
    }
    partSeq.sort( DoubleInt::Greater );
    
    /////TEST (2006-01-10) donnie
    printf( "%d Parts, Front/Back = %.0f %.0f (%.2f)\n", 
	    partSeq.size(), partSeq.front().d, partSeq.back().d, partSeq.front().d/partSeq.back().d );

    flush( cout );
    
    list<DoubleInt>::const_iterator ite;
 
    // TODO: check if it works
    if( param.bPrePartitioning )
    {
        int nPrepart = 1;
        for( int i=0; i<nPrepart; i++ )
        {
            double moveRatio = 0.5;
            if( level > 1 )
            {
                for( ite=partSeq.begin(); ite!=partSeq.end(); ite++ )
                {
                    partId = ite->i; 

		    // Move blocks only
                    MakePartition( partId, 
			    edgecut, 
			    new1, 
			    new2, 
			    cutDir, 
			    true,	// noSubRegion  
			    true,	// considerMovable   
			    true,	// moveHalf
			    moveRatio ); 
                }
                fplan->CalcHPWL();
                cout << " (PreWL= " << this->fplan->GetHPWLp2p() << ") ";
                flush( cout );
            }
        }
    }


    double largestArea = 0;
    
    set<int> doneParts; // a set storing parted regions
    int partCounter = 0;
    const double maxAreaRatio = 0.5;
    for( ite = partSeq.begin(); ite != partSeq.end(); ite++ )
    {
	partId = ite->i;
	if( doneParts.find( partId ) != doneParts.end() )
	    continue;

	//assert( partId < m_partitions.size() );
	if( m_partitions[partId].area < largestArea * maxAreaRatio )	// area is too small. stop this level
	{
	    //printf( " %f < %f, break\n", m_partitions[partId].area, largestArea );
	    break; 
	}
	
	MakePartition( partId, edgecut, new1, new2, cutDir );
	m_partitions[partId].bDone = true;  // 2006-01-10
	doneParts.insert( partId );

	partCounter++;

	if( new1 > 0 && m_partitions[new1].area > largestArea ) 
	    largestArea = m_partitions[new1].area;
	
	if( new2 > 0 && m_partitions[new2].area > largestArea ) 
	    largestArea = m_partitions[new2].area;
	
#if 0
	if( new1 > 0 && level < 15 )
	{
	    QPPartition( new1 );
	}

	if( new2 > 0 && level < 15 )
	{
	    QPPartition( new2 );
	}
#endif	

	/*
	// LR-repartition
	if( param.bRefineParts && m_partitions[partId].peerPartitionId > 0 )
	{
	    int p0 = partId;
	    int p1 = m_partitions[partId].peerPartitionId;
	    int parent = m_partitions[partId].parentPart;

	    MakePartition( p1, edgecut, new1, new2, cutDir );
	    doneParts.insert( p1 );

	    double wire, last_wire, best_wire;
	    best_wire = last_wire = GetPartHPWL( parent );
	    SavePartBlockLocation( parent );
	    SavePartBlockList( p0 );
	    SavePartBlockList( p1 );

	    int badCount = 0;
	    int loopCounter = 0;
	    while( loopCounter<100 )   
	    {
		loopCounter++;
		MakePartition( p0, edgecut, new1, new2, cutDir );
		MakePartition( p1, edgecut, new1, new2, cutDir );
		wire = GetPartHPWL( parent );
		if( wire < last_wire )
		{
		    if( wire < best_wire )
		    {
			//if( param.bShow )
			//{
			//    cout << "-";
			//    flush( cout );
			//}
			if( badCount > 0 )
			{
			    cout << "i";
			    flush( cout );
			    badCount = 0;
			}
			best_wire = wire;
			SavePartBlockLocation( parent );
			SavePartBlockList( p0 );
			SavePartBlockList( p1 );
		    }
		}
		else
		{
		    badCount++;
		    if( badCount >= param.n_repart )
		    {
			break;
		    }
		}
		last_wire = wire;
	    } // while
	    RestorePartBlockLocation( parent );
	    RestorePartBlockList( p0 );
	    RestorePartBlockList( p1 );
	}
	*/
	
	////partCounter++;
	////if( totalPart<100 )
	////{
	////    cout << ".";
	////}
	////else if( totalPart<10000 )
	////{
	////    if( partCounter % 100 == 0 )
	////        cout << "o";
	////}
	////else
	////{
	////    if( partCounter % 10000 == 0 )
	////        cout << "O";
	////}
	////flush( cout );
    }
    //cout << endl;
    printf( "SKIP %.2f\n", 1.0 - static_cast<double>(partCounter) / partSeq.size() );
    flush( cout );
    
//
//if( param.bRefineParts && level > 1 )
//{
//    fplan->CalcHPWL();
//    double preWL = fplan->GetHPWLp2p();
//    cout << "wire= " << preWL << endl;
//
//    set<int> doneParts; // a set storing parted regions
//    int partCounter = 0;
//    //double totalRefine = 0;
//    for( int i=startPartId; i<endPartId; i++ )
//    //for( ite = partSeq.begin(); ite != partSeq.end(); ite++ )
//    {
//        //partId = ite->i;
//        partId = i;
//
//        //cout << " " << partId << ":" << ite->d << endl;
//        if( doneParts.find( partId ) != doneParts.end() )
//            continue;
//
//        //m_partitions[partId].Print();
//
//        //cout << "part " << partId << endl;
//		//MakePartition( partId, edgecut, new1, new2, cutDir );
//        doneParts.insert( partId );
//
//        if( param.bRefineParts && m_partitions[partId].peerPartitionId > 0 )
//        {
//            int p0 = partId;
//            int p1 = m_partitions[partId].peerPartitionId;
//            int parent = m_partitions[partId].parentPart;
//
//		    //MakePartition( p1, edgecut, new1, new2, cutDir );
//            doneParts.insert( p1 );
//
//            double wire, last_wire, best_wire;
//            best_wire = last_wire = GetPartHPWL( parent );
//            SavePartBlockLocation( parent );
//            SavePartBlockList( p0 );
//            SavePartBlockList( p1 );
//
//            int badCount = 0;
//            int loopCounter = 0;
//            while( loopCounter<100 )   
//            //while( loopCounter<param.n_repart )   
//            {
//                loopCounter++;
//                MakePartition( p0, edgecut, new1, new2, cutDir );
//                MakePartition( p1, edgecut, new1, new2, cutDir );
//                wire = GetPartHPWL( parent );
//                if( wire < last_wire )
//                {
//                    if( wire < best_wire )
//                    {
//                        if( badCount > 0 )
//                        {
//                            cout << "i";
//                            flush( cout );
//                            badCount = 0;
//                        }
//                        best_wire = wire;
//                        SavePartBlockLocation( parent );
//                        SavePartBlockList( p0 );
//                        SavePartBlockList( p1 );
//                    }
//                }
//                else
//                {
//                    badCount++;
//                    if( badCount >= param.n_repart )
//                    {
//                        break;
//                    }
//                }
//                last_wire = wire;
//            } // while
//            RestorePartBlockLocation( parent );
//            RestorePartBlockList( p0 );
//            RestorePartBlockList( p1 );
//        }
//    }
//
//    fplan->CalcHPWL();
//    double afterWL = fplan->GetHPWLp2p();
//    cout << "wire= " << afterWL << " (" << (afterWL-preWL)/preWL*100.0 << "%)\n";
//
//}


    // Re-bipartition all bins (placement feedback)
    if( param.bRefineParts && level > 1 /*&& level < 10*/ )
    {
	fplan->CalcHPWL();
	cout << "       wire= " << fplan->GetHPWLp2p() << endl;
	double beforeWire = fplan->GetHPWLp2p();
	double bestWire = beforeWire;

	for( ite = partSeq.begin(); ite != partSeq.end(); ++ite )
	{
	    SavePartBlockLocation( ite->i );
	    SavePartBlockList( ite->i );
	}

	for( int i=0; i<param.n_repart; i++ )
	{
	    for( ite = partSeq.begin(); ite != partSeq.end(); ++ite )
	    {
		partId = ite->i;
		if( m_partitions[partId].bDone )	// only repart those parted
		{
		    MakePartition( partId, edgecut, new1, new2, cutDir );
		}
	    }
	    fplan->CalcHPWL();
	    cout << "Repart wire= " << fplan->GetHPWLp2p() << endl;
	    if( fplan->GetHPWLp2p() < bestWire )
	    {
		bestWire = fplan->GetHPWLp2p();
		for( ite = partSeq.begin(); ite != partSeq.end(); ++ite )
		{
		    SavePartBlockLocation( ite->i );       
		    SavePartBlockList( ite->i );	
		}
	    }
	}
	for( ite = partSeq.begin(); ite != partSeq.end(); ++ite )
	{
	    RestorePartBlockLocation( ite-> i);
	    RestorePartBlockList( ite-> i);	
	}
	printf( "Improv= %.2f%%\n", (beforeWire-bestWire) / beforeWire * 100.0 );
    }


    static double neighborDistance = 
	(fplan->m_coreRgn.right - fplan->m_coreRgn.left) + 
	(fplan->m_coreRgn.top - fplan->m_coreRgn.bottom);
    
    // Refine neighbor parts
    double before = -1, after = -1;
    if( true == param.bRefinePartsAll && level > 2 /*&& level < 8*/ )
    {
	
	int nextEndPartId = GetPartitionNumber();
	for( int i=endPartId; i<nextEndPartId; i++ )
	{
	    UpdatePartNeighbor( i, neighborDistance );
	}
	cout << "    N-Distance= " << neighborDistance << endl;

	// show neghborhood size
	/*for( int i=endPartId; i<nextEndPartId; i++ )
	  {
	  cout << "part" << i << ": ";
	  for( int j=0; j<(int)m_partitions[i].neighborList.size(); j++ )
	  {
	  cout << m_partitions[i].neighborList[j] << " ";
	  }
	  cout << endl;
	  }*/

	neighborDistance *= 0.63;

	fplan->CalcHPWL();
	before = fplan->GetHPWLp2p();
	cout << "    BeforeWL= " << before << endl;
	
	bool better = true;
	//int noBetterCount = 0;
	for( int i=0; i<param.n_repart; i++ )
	{
	    better = false;
	    for( int i=endPartId; i<nextEndPartId; i++ )
	    {
		if( RefineNeighbors( i ) )
		{
		    better = true;
		}
	    }
	    //if( better )
		//cout << "*";
	    /*else
	      {
	      break;
	      noBetterCount++;
	      }*/
	    fplan->CalcHPWL();
	    after = fplan->GetHPWLp2p();
	    cout << "    AfterWL = " << after << endl;
	}
	printf( "    Improv  = %.2f%%\n", (100.0 - 100.0 * after / before) );
    }
    else
        fplan->CalcHPWL();



    //// 2005/2/23: Refine four regions
    //if( level > 1 )//&& level < 8 )
    //{
    //    if( true == param.bRefineParts )
    //    {

    //        fplan->CalcHPWL();
    //        before = fplan->GetHPWLp2p();
    //        cout << " L" << level << ": Pin-to-pin HPWL= " << before << endl;
    //        set<int> refined;
    //        for( ite = partSeq.begin(); ite != partSeq.end(); ite++ )
    //        {
    //            partId = ite->i;
    //            if( refined.find( partId ) == refined.end() )
    //            {
    //                int peerId = m_partitions[partId].peerPartitionId;
    //                if( /*level < 8 &&*/ peerId > 0 && m_partitions[peerId].childPart0 > 0 && m_partitions[peerId].childPart1 > 0 )
    //                {
    //                    RefineFourRegions( m_partitions[partId].childPart0, m_partitions[partId].childPart1,
    //                                        m_partitions[peerId].childPart0, m_partitions[peerId].childPart1 );
    //                }
    //                refined.insert( partId );
    //                refined.insert( peerId );
    //            }
    //        }
    //        fplan->CalcHPWL();

    //        after = fplan->GetHPWLp2p();
    //        cout << "\n  Improv= " << 100.0 - 100.0 * after / before << "%" << endl;
    //    }
    //}
    //else
        fplan->CalcHPWL();


    // Collect some infomation for the next level
    double maxPartArea = 0;
    int    maxBlockCount = 0;
    double maxUtil = 0;
    double avgUtil = 0;
    double maxDensity = 0;
    double avgOverUtil = 0;
    int    overfillCounter = 0;    
    int    nextEndPartId = GetPartitionNumber();
    int    count = 0;
    for( int i=startPartId; i<nextEndPartId; i++ )
    {
	if( m_partitions[i].bDone == false )
	{
	    float util = m_partitions[i].GetUtilization();
	    float density = m_partitions[i].GetDensity();

	    if( m_partitions[i].area > maxPartArea )
		maxPartArea = m_partitions[i].area;
	    if( (int)m_partitions[i].moduleList.size() > maxBlockCount )
		maxBlockCount = (int)m_partitions[i].moduleList.size();
	    if( util > maxUtil )
		maxUtil = util;
	    if( util > 1.0 )
	    {
		avgOverUtil += util;
		overfillCounter++;
	    }
	    if( density > maxDensity )
		maxDensity = density;

	    avgUtil += util;
	    count++;
	}
    }
    if( count > 0 )
    	avgUtil /= count;

    if( param.bShow )
    {
	cout << " L" << level << ": Memory= " << GetPeakMemoryUsage() << "MB\n";
        //cout << " L" << level << ": MaxArea= " << maxPartArea << endl;
        cout << " L" << level << ": MaxDensity= " << maxDensity << endl;
        cout << " L" << level << ": MaxUtil= " << maxUtil << endl;
	cout << " L" << level << ": AvgUtil= " << avgUtil << endl;
        //cout << " L" << level << ": AvgOverUtil= " << avgOverUtil/overfillCounter << endl;
        cout << " L" << level << ": OverUtil= " << overfillCounter << " of " << totalPart << endl;
        cout << " L" << level << ": MaxBlockCount= " << maxBlockCount << endl;
    }
    cout << " L" << level << ": Time= " << (seconds()-levelStartTime)  
	 << " s (total= " << (seconds()-programStartTime)/60.0 << " m)" << endl;
    cout << " L" << level << ": Pin-to-pin HPWL= " << fplan->GetHPWLp2p() << endl;

/*
    // Collect memory
    for( int i=memoryCollectCounter; i<startPartId; i++ )
    {
        // L1: none
        // L2: clean L1
        // L3: clean L2
	if( m_partitions[i].bDone )  // 2006-01-10 more safer
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
*/
    //startPartId = endPartId;    // For the next level...
    
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

    //LevelRefinement( startPartId, GetPartitionNumber()-1 );
    //fplan->CalcHPWL();
    //cout << " L" << level << ": Pin-to-pin HPWL= " << fplan->GetHPWLp2p() << endl;

    /*if( endPartId == GetPartitionNumber() )
        return false;   // no more partitions
    else
        return true;*/
}



void CMinCutPlacer::UpdatePartNeighbor( int partId, double distance )
{
    m_partitions[partId].neighborList.clear();
    int parentId = m_partitions[partId].parentPart;

    double x1, y1, x2, y2;
    int id, childId;
    x1 = (m_partitions[partId].left + m_partitions[partId].right) * 0.5;
    y1 = (m_partitions[partId].top + m_partitions[partId].bottom) * 0.5;

    // Handle peerId
    id = m_partitions[partId].peerPartitionId;
    if( id > 0 )
    {
        x2 = (m_partitions[id].left + m_partitions[id].right) * 0.5;
        y2 = (m_partitions[id].top + m_partitions[id].bottom) * 0.5;
        /*if( (abs(x2-x1) + abs(y2-y1)) > distance )
        {
            cout << "WARNING: distance( part, peer part ) > distance\n";
        }*/
        m_partitions[partId].neighborList.push_back( id );
    }

    // Handle parent-neightbors' children
    for( int i=0; i<(int)m_partitions[parentId].neighborList.size(); i++ )
    {
        id = m_partitions[parentId].neighborList[i];
        if( m_partitions[id].childPart0 > 0 )
        {
            childId = m_partitions[id].childPart0;
            x2 = (m_partitions[childId].left + m_partitions[childId].right) * 0.5;
            y2 = (m_partitions[childId].top + m_partitions[childId].bottom) * 0.5;
            if( (abs(x2-x1) + abs(y2-y1)) <= distance )
                m_partitions[partId].neighborList.push_back( childId );
        }
        if( m_partitions[id].childPart1 > 0 )
        {
            childId = m_partitions[id].childPart1;
            x2 = (m_partitions[childId].left + m_partitions[childId].right) * 0.5;
            y2 = (m_partitions[childId].top + m_partitions[childId].bottom) * 0.5;
            if( (abs(x2-x1) + abs(y2-y1)) <= distance )
                m_partitions[partId].neighborList.push_back( childId );
        }
    }

}


bool CMinCutPlacer::RefineNeighbors( int partId )
{
    //bool better = false;
    bool hasBetter = false;
    //int noBetterCount = 0;
    //while( true )
    //{
        //hasBetter = false;
        for( int i=0; i<(int)m_partitions[partId].neighborList.size(); i++ )
        {
            if( RefineRegions( partId, m_partitions[partId].neighborList[i] ) )
            {
                //cout << "(" << partId << " " << m_partitions[partId].neighborList[i] << ")";
                //better = true;
                hasBetter = true;
            }
        }

      /*  if( !hasBetter )
            break;
        else
            cout << "|";
    }*/
    //return better;
    return hasBetter;
}


// First version: 2005/2/20
// Clean code: 2005/12/03
bool CMinCutPlacer::RefineRegions( int regionId0, int regionId1 )
{
    if( regionId0 == regionId1 )
	return false;

    if( regionId0 == 0 )
	return false;

    if( regionId1 == 0 )
	return false;

#if 0
    cout << "RefineRegions: " << regionId0 << " " << regionId1 << endl;
#endif

    int part0 = regionId0;
    int part1 = regionId1;

    double top0    = m_partitions[part0].top;
    double bottom0 = m_partitions[part0].bottom;
    double left0   = m_partitions[part0].left;
    double right0  = m_partitions[part0].right;
    double cx0     = (left0 + right0) * 0.5;
    double cy0     = (top0 + bottom0) * 0.5;

    double top1    = m_partitions[part1].top;
    double bottom1 = m_partitions[part1].bottom;
    double left1   = m_partitions[part1].left;
    double right1  = m_partitions[part1].right;
    double cx1     = (left1 + right1) * 0.5;
    double cy1     = (top1 + bottom1) * 0.5;

    double xDistance = fabs( cx1 - cx0 );
    double yDistance = fabs( cy1 - cy0 );
    double centerDistance = xDistance + yDistance;

    //////////////////////////////////////////////////////////////
    // prepare data strucutre
    //////////////////////////////////////////////////////////////

    const double extraWeight = 1.0 / fplan->m_rowHeight;

    //==================================================
    // 1. create module map and initial partition
    //===================================================
    map<int, int> mapId;		// moduleId -> hMetis_vetex_id
    map<int, int> reverseMapId;	// hMetis_vetex_id -> moduleId

    // total area
    double totalBlockArea = m_partitions[part1].totalMovableArea + m_partitions[part0].totalMovableArea;

#if 0
    cout << "totalBlockArea= " << totalBlockArea << endl;
#endif

    // block #
    int p0ModuleNumber = (int)m_partitions[part0].moduleList.size();
    int moduleNumber = (int)m_partitions[part1].moduleList.size() +  (int)m_partitions[part0].moduleList.size();

    // create the map
    int moduleId;
    int moduleCounter = 0;
    for( int i=0; i<moduleNumber; i++ )
    {
	// mappings: vetex_id <--> moduleId
	if( i < p0ModuleNumber )
	    moduleId = m_partitions[part0].moduleList[i]; 
	else
	    moduleId = m_partitions[part1].moduleList[i-p0ModuleNumber]; 

	mapId[ moduleId ] = i;
	reverseMapId[ i ] = moduleId;
	part[i] = -1;	// movable block
	moduleWeight[i] = (int)round( fplan->m_modules[moduleId].m_area * extraWeight );
	moduleCounter++;
    }
    double freeArea0 = m_partitions[part0].GetFreeArea();
    double freeArea1 = m_partitions[part1].GetFreeArea();
    if( freeArea0 == 0 && freeArea1 == 0 )
    {
	cout << "\n RefineRegions: Both freeArea are 0\n"; 
	exit(0);
    }

    // Add two fixed nodes to each partition.
    moduleCounter += 2;
    part[moduleCounter-2] = 0;	// node0 is fixed to part0
    part[moduleCounter-1] = 1;	// node1 is fixed to part1
    moduleWeight[moduleCounter-2] = 0;
    moduleWeight[moduleCounter-1] = 0;
    double dummyArea = 0;
    if( freeArea0 > freeArea1 )
    {
	dummyArea = totalBlockArea * ( (freeArea0 - freeArea1) / (freeArea0 + freeArea1) );
	moduleWeight[moduleCounter-1] = (int)round( dummyArea * extraWeight );
    }
    else if( freeArea0 < freeArea1 )
    {
	dummyArea = totalBlockArea * ( (freeArea1 - freeArea0) / (freeArea0 + freeArea1) );
	moduleWeight[moduleCounter-2] = (int)round( dummyArea * extraWeight );
    }

#if 0
    cout << "total nodes: " << moduleCounter << endl;
#endif

    // Calculate "unbalance" factor 
    // The "real" values (seems bad for legalizer... strange...)
/*    double ub0 = param.ubfactor * 100 * (freeArea0/(totalBlockArea+dummyArea) - 0.5);
    double ub1 = param.ubfactor * 100 * (freeArea1/(totalBlockArea+dummyArea) - 0.5);
    //cout << "ub0= " << ub0 << endl;
    //cout << "ub1= " << ub1 << endl;    
    int ubfactor = (int)min( ub0, ub1 );
    if( ubfactor > 49 ) ubfactor = 49;
    if( ubfactor < 1 )  ubfactor = 1;
*/
    int ubfactor = 1;

    //============================================
    // 2. create net array
    //============================================
    // The max # of nets is (left_part_net_# + right_part_net_#)
    int totalNetNumber = (int)m_partitions[part0].netList.size() + (int)m_partitions[part1].netList.size();
    double extraNetWeight =        5000000.0 / centerDistance / totalNetNumber;
    if( extraNetWeight >= 1.0 )
	extraNetWeight = 1.0;

    int netCounter = 0;
    int pinCounter = 0;
    eptr[netCounter] = 0;
    netCounter++;
    bool insidePart;			// whole net inside the partition
    bool outsidePart;			// whole net outside the partition
    vector<bool> externalPin;	// Save the infomation of external/internal 
    // pin to reduce the map lookup time
    int netId;

    set<int> setNetId;		// For checking duplication nets.
    int p0NetNumber = (int)m_partitions[part0].netList.size();
    for( int i=0; i<totalNetNumber; i++ )
    {
	if( i < p0NetNumber )
	    netId = m_partitions[part0].netList[i];
	else
	    netId = m_partitions[part1].netList[i-p0NetNumber];

	if( setNetId.find( netId ) != setNetId.end() )
	    continue;
	setNetId.insert( netId );

	externalPin.resize( fplan->m_nets[netId].size() );

	insidePart = true;
	outsidePart = true;
	for( int j=0; j<(int)fplan->m_nets[netId].size(); j++ )
	{
	    if( mapId.find( fplan->m_pins[ fplan->m_nets[netId][j] ].moduleId ) != mapId.end() ) 
	    {
		// Found in the partition, so the whole net is impossible
		// outside the partition
		outsidePart = false;
		externalPin[j] = false;
	    }
	    else	
	    {
		// Not found in the partition, so the whole net is impossible
		// inside the partition
		insidePart = false;
		externalPin[j] = true;
	    }
	}

	// CASE 1:
	// If all terminals of the net is not in the partition,
	// we discard it.
	if( outsidePart == true )	
	{
	    cout << "RefineRegions: outsidePart!\n";
	    //exit(0);
	    continue;	// discard the net
	}

	// CASE 2:
	// If all terminals of the net is in the partition,
	// we simply take it. (No connections to fixed nodes)
	if( insidePart == true )
	{
#if 0
	    cout << "Case2 ";
#endif
	    for( int j=0; j<(int)fplan->m_nets[netId].size(); j++ )
	    {
#if 1
		if( pinCounter >= fplan->m_nPins )
		{
		    cout << "Error\nnet# = " << netCounter << endl;
		    cout << "pin# = " << pinCounter << endl;
		    exit(0);
		}
#endif
		//assert( pinCounter < m_nPins );
		eind[pinCounter] = mapId[ fplan->m_pins[ fplan->m_nets[netId][j] ].moduleId ];
		pinCounter++;
	    }
	    eptr[netCounter] = pinCounter;
	    netWeight[netCounter-1] = (int)round( extraNetWeight * centerDistance );
	    netCounter++;
	    continue;
	}


	// CASE 3: 
	// Part of the net is inside the partition.
#if 0
	cout << "Case3 ";
#endif

	// Compute "segment" range: [min_value max_value]
	double x_max_value = INT_MIN, x_min_value = INT_MAX;
	double y_max_value = INT_MIN, y_min_value = INT_MAX;
	for( int j=0; j<(int)fplan->m_nets[netId].size(); j++ )
	{
	    if( externalPin[j] == false )
		continue;	// "free" modules

	    double x, y;                   // Physical coordinate
	    fplan->GetPinLocation( fplan->m_nets[netId][j], x, y );

	    x_max_value = max( x_max_value, x );
	    x_min_value = min( x_min_value, x );
	    y_max_value = max( y_max_value, y );
	    y_min_value = min( y_min_value, y );
	}

	int weight;
	if( (x_max_value-x_min_value) >= xDistance && (y_max_value-y_min_value) >= yDistance )   
	{                                           // across section , dont't care
	    continue;
	}
	else
	{
	    double xDis0 = distance( cx0, x_min_value, x_max_value );
	    double yDis0 = distance( cy0, y_min_value, y_max_value );
	    double xDis1 = distance( cx1, x_min_value, x_max_value );
	    double yDis1 = distance( cy1, y_min_value, y_max_value );
	    weight = (int)round( extraNetWeight * ( (xDis1+yDis1) - (xDis0+yDis0) ) );
	    if( weight == 0 )
		continue;

	    // Add the fixed-node at proper partition
	    if( abs(weight) > extraNetWeight * ( centerDistance * 2.0 * param.ubfactor * 0.01 ) )
	    {
		// add pin
		assert( pinCounter < fplan->m_nPins );
		if( weight >= 0 )
		{
		    eind[pinCounter] = moduleCounter-2;	// left fixed node
		}
		else
		{
		    eind[pinCounter] = moduleCounter-1;	// right fixed node
		    weight = -weight;   // take positive value
		}
		pinCounter++;
	    }
	    else
	    {
		// The distance from two center to the segment L is
		// similar. We don't connect the net to any fixed node.
		// However, the net still need to be add to the graph.
		//saveNets++;
		weight = (int)round( extraNetWeight * ( (xDis1+yDis1) + (xDis0+yDis0) ) * 0.5) ;   // The cost is the distance.
		if( weight == 0 )
		    continue;
	    }

	    //// Test range
	    //cout << "<" << mid0 << " " << mid  << " " << mid1 << "> ";
	    //cout << "[" << min_value << " " << max_value << "] ";
	    //cout << "weight = " << weight << endl;
	}

	for( int j=0; j<(int)fplan->m_nets[netId].size(); j++ )
	{
	    if( externalPin[j] == true )
		continue;

	    assert( pinCounter < fplan->m_nPins );
	    eind[pinCounter] = mapId[ fplan->m_pins[ fplan->m_nets[netId][j] ].moduleId ];
	    pinCounter++;

	} // For each pin
	eptr[netCounter] = pinCounter;
	assert( weight >= 0 );
	netWeight[netCounter-1] = weight;

	netCounter++;

    } // For each net
    netCounter--;

#if 0
    cout << "Complete data preparation.\n";
#endif


#if 0
    cout << "\n\t Nodes: " << moduleCounter ;
    cout << "\t Edges: " << netCounter << " ("<< saveNets <<")\t";
    cout << "\t Pins: " << pinCounter << endl;
#endif
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
    int edgecut;
    if( true )
    {
	// HMETIS

	// 2005/03/12 turn off the feature since we set the default param.ubfactor = 0
	//if( (m_partitions[regionId0].utilization > 0.9 || m_partitions[regionId1].utilization > 0.9 )
	//     && moduleNumber < 100 )
	//     ubfactor = 1;        
	//#if 0
	//        ubfactor *= 0.5;
	//        if( ubfactor<1 )  ubfactor = 1;
	//#endif

	//if( param.bRefineUseMetis )
	{
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
	}
	//      else
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
	//    }
}

//    //cout << "cut= " << edgecut << endl;
//    //flush( cout );
//
	///////////////////////////////////////////////////////////////
	// The partition result is in "part[]"
	///////////////////////////////////////////////////////////////

	///////////////////////////////////////////////////////////////
	// Check area overflow
	///////////////////////////////////////////////////////////////
    double p0BlockArea = 0;
    double p1BlockArea = 0;    // Total movable area
    double p0Counter = 0;
    double p1Counter = 1;
    for( int i=0; i<moduleCounter-2; i++ )  // Do not include the fixed nodes
    {
	int id = reverseMapId[i];
	//#if 0
	//        cout << id << " ";
	//#endif
	assert( id < (int)fplan->m_modules.size() );
	if( part[i] == 0 )  
	{
	    p0BlockArea += fplan->m_modules[id].m_area;
	    p0Counter++;
	}
	else if( part[i] == 1 ) 
	{
	    p1BlockArea += fplan->m_modules[id].m_area;
	    p1Counter++;
	}
    }
//if( p0BlockArea > part0Area )
//{
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

    // Calc pre-WL
    vector<int> part0BlockList;
    vector<int> part1BlockList;
    set<int> nets1;
    for( int i=0; i<(int)m_partitions[regionId0].netList.size(); i++ )
	nets1.insert( m_partitions[regionId0].netList[i] );
    for( int i=0; i<(int)m_partitions[regionId1].netList.size(); i++ )
	nets1.insert( m_partitions[regionId1].netList[i] );
    double preWL = 0;
    set<int>::const_iterator ite1;
    for( ite1=nets1.begin(); ite1!=nets1.end(); ++ite1 )
	preWL += fplan->GetNetLength( *ite1 );

    //=======================================================================
    // Apply HMETIS results
    //=======================================================================
    for( int i=0; i<moduleNumber ; i++ )        // NOTE: moduleNumber = moduleCounter-2
    {
	int id = reverseMapId[i];
	assert( id < (int)fplan->m_modules.size() );

	// Move the module to the partition
	if( part[i] == 0 )
	{
	    fplan->MoveModuleCenter( id, cx0, cy0 );
	    part0BlockList.push_back( id );
	}
	else if( part[i] == 1 )
	{
	    fplan->MoveModuleCenter( id, cx1, cy1 );
	    part1BlockList.push_back( id );
	}
	else
	{
	    cout << "Error, no partition result for module " << id << endl;
	}
    }


    // Calc after-WL
    // Since the blockList of the partition is not updated yet, we cannot use "GetPartHPWL".
    double afterWL = 0;
    set<int> nets;
    for( int i=0; i<(int)part0BlockList.size(); i++ )
    {
	int id = part0BlockList[i];
	for( int j=0; j<(int)fplan->m_modules[id].m_netsId.size(); j++ )
	{
	    nets.insert( fplan->m_modules[id].m_netsId[j] );
	}
    }
    for( int i=0; i<(int)part1BlockList.size(); i++ )
    {
	int id = part1BlockList[i];
	for( int j=0; j<(int)fplan->m_modules[id].m_netsId.size(); j++ )
	{
	    nets.insert( fplan->m_modules[id].m_netsId[j] );
	}
    }
    set<int>::const_iterator ite;
    for( ite=nets.begin(); ite!=nets.end(); ite++ )
	afterWL += fplan->GetNetLength( *ite );

    bool better = false;
    if( afterWL < preWL )   // "Real" apply the result
    {
	better = true;
	m_partitions[part0].moduleList = part0BlockList;
	m_partitions[part1].moduleList = part1BlockList;
    }

    UpdatePartition( part0 );
    UpdatePartition( part1 );
    /*if( better )
    {
	//cout << "wl1= " << preWL << " wl2= " << afterWL << endl;
	cout << "-";
	flush( cout );
    }*/
    return better;     // Refinement complete!

}


//----------------------------------------------------------------------------
// UpdatePartition
// Clear code: 2005-12-03
//----------------------------------------------------------------------------
void CMinCutPlacer::UpdatePartition( const int& partId )
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

/*
bool CMinCutPlacer::LevelRefinement( int startId, int endId )      // From startId to endId.
//{
//    if( startId == endId )
//        return false;
//
//
//    int badCount = 0;
//    int range = (endId-startId+1);
//
//#if 1
//    cout << "range= " << range << endl;
//#endif
//
//    int id1, id2;
//    int maxTimes;
//    if( range < 16 )
//        maxTimes = range*range;
//    else if( range < 256 )
//        maxTimes = range;
//    else
//        maxTimes = range / 10;
//    if( maxTimes > 500 )
//        maxTimes = 500;
//
//    while( badCount < maxTimes )
//    {
//
//        id1 = rand() % range;
//        do
//        {
//            id2 = rand() % range;
//        } while( id2 == id1 );
//        id1 += startId;
//        id2 += startId;
//        if( false == RefineRegions( id1, id2 ) )
//        {
//            badCount++;
//#if 1
//            if( badCount % 20 == 0 )
//                cout << badCount << " ";
//            flush( cout );
//#endif 
//        }
//        else
//        {
//            cout << " (" << id1 << " " << id2 << ") ";
//            badCount = 0;
//        }
//    }
//    return false;
//}*/


/*
//// 2005/2/22
//bool CFloorplan::RefineFourRegions( int id1, int id2, int id3, int id4 )
//{                                       
//    //  2  |  4
//    //-----|----
//    //  1  |  3
//    
//#if 0
//    cout << "Refine: " << id1 << " " << id2 << " " << id3 << " " << id4 << endl;
//#endif
//
//    bool better = false;
//    bool hasBetter = false;
//    int badCount = 0;
//    while( badCount < 3 )
//    {
//        if( RefineRegions( id2, id4 ) )
//        {
//            cout << "2";
//            hasBetter = true;
//            better = true;
//        }
//        if( RefineRegions( id1, id3 ) )
//        {
//            cout << "2";
//            hasBetter = true;
//            better = true;
//        }
//        //if( RefineRegions( id2, id3 ) )
//        //{
//        //    cout << "3";
//        //    hasBetter = true;
//        //    better = true;
//        //}
//        //if( RefineRegions( id1, id4 ) )
//        //{
//        //    cout << "3";
//        //    hasBetter = true;
//        //    better = true;
//        //}
//        if( RefineRegions( id1, id2 ) )
//        {
//            cout << "1";
//            hasBetter = true;
//            better = true;
//        }
//        if( RefineRegions( id3, id4 ) )
//        {
//            cout << "1";
//            hasBetter = true;
//            better = true;
//        }
//
//        if( false == better )
//        {
//            badCount++;
//            //break;
//        }
//        better = false;
//    }
//    return hasBetter;
//}
*/

//----------------------------------------------------------------------------
// AddPartition
//
// Create a new partition which has the parent partition id = parentPartId.
// Find all movable modules in the region and put into the new partition.
// Return new partition id
//----------------------------------------------------------------------------
int CMinCutPlacer::AddPartition( const int& parentPartId, 
	double left, double bottom, 
	double right, double top, int partId )
{
    // If partId != -1, we need to use the exist part id.
 //Start:=====================(indark)==========================

//End:=====================(indark)==========================
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

	set<int> nets;
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
	    
	    if( fplan->m_modules[i].m_cx >= left && fplan->m_modules[i].m_cx < right &&
	        fplan->m_modules[i].m_cy >= bottom && fplan->m_modules[i].m_cy < top )
	    {
		//setId.insert( i );
		m_partitions[partId].moduleList.push_back( i );
		totalMovableArea += fplan->m_modules[i].m_area;

		// add nets
		for( int n=0; n<(int)fplan->m_modules[i].m_netsId.size(); n++ )
		    nets.insert( fplan->m_modules[i].m_netsId[n] );
	    }

	} // for each block

	// Handle nets
	m_partitions[partId].netList.clear();
	copy( nets.begin(), nets.end(), back_inserter( m_partitions[partId].netList ) );
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
    m_partitions[partId].totalFixedArea   = totalFixedArea;

    //float util = m_partitions[partId].GetUtilization();
    //if( util > 3.0 && (int)m_partitions[partId].moduleList.size() > 3 )
    //{
    //	cout << "** Util= " << util << " (" << (int)m_partitions[partId].moduleList.size() << " blocks)**\n";
    //}

    return partId;

}

/*
void CMinCutPlacer::QPPrePartition( int partId )
{
    // TODO: check if QP is done before
    QPPartition( partId );
    CUTDIR cutDir = GetCutDirForPart( partId )
    
    const double largeWeight = 0.8;
    const double smallWeight = 0.2;
	
    vector< pair<double, int> > blockSeq;
    double coordinateBound0;
    double coordinateBound1;
    if( cutDir == V_CUT )
    {
	coordinateBound0 = m_partitions[partId].left * largeWeight + m_partitions[partId].right * smallWeight;
	coordinateBound1 = m_partitions[partId].left * smallWeight + m_partitions[partId].right * largeWeight;
	for( unsigned int i=0; i<m_partitions[partId].moduleList.size(); i++ )
	{
	    int id = m_partitions[partId].moduleList[i];
	    blockSeq.push_back( make_pair( fplan->m_modules[id].m_cx, id ) );    // < x, id >
	}
    }
    else // H_CUT
    {
	coordinateBound0 = m_partitions[partId].bottom * largeWeight + m_partitions[partId].top * smallWeight;
	coordinateBound1 = m_partitions[partId].bottom * smallWeight + m_partitions[partId].top * largeWeight;
	for( unsigned int i=0; i<m_partitions[partId].moduleList.size(); i++ )
	{
	    int id = m_partitions[partId].moduleList[i];
	    blockSeq.push_back( make_pair( fplan->m_modules[id].m_cy, id ) );    // < y, id >
	}
    }
    // sort from min to max
    sort( blockSeq.begin(), blockSeq.end() );   // TODO, sort using only "first"

    int fixedNodes = 0;

    // set fixed node in part0
    const double areaRatioBound = 0.20;
    double areaUsed = 0;
    for( unsigned int i=0; i<blockSeq.size(); i++ )
    {
	if( areaUsed > freeArea0 * areaRatioBound )
	{
	    //printf( "." );
	    break;
	}
	if( blockSeq[i].first > coordinateBound0 )
	{
	    //printf( "-" );
	    break;
	}
	assert( (int)blockSeq[i].second < fplan->m_modules.size() );
	assert( mapId[blockSeq[i].second] < (int)m_partitions[partId].moduleList.size() + 2 );
	areaUsed += fplan->m_modules[ blockSeq[i].second ].m_area;
	part[ mapId[blockSeq[i].second] ] = 0;    // fixed at the part0
	fixedNodes++;
    }

    // set fixed node in part1
    areaUsed = 0;
    if( blockSeq.size() > 0 )
	for( int i=(int)blockSeq.size()-1; i>=0; i-- )
	{
	    if( areaUsed > freeArea1 * areaRatioBound )
	    {
		//printf( "." );
		break;
	    }
	    if( blockSeq[i].first < coordinateBound1 )
	    {
		//printf( "-" );
		break;
	    }
	    if( blockSeq[i].second >= (int)fplan->m_modules.size() )
		printf( "i= %d of %d   %f %d\n", i, blockSeq.size(), blockSeq[i].first, blockSeq[i].second );
	    assert( blockSeq[i].second < fplan->m_modules.size() );
	    assert( mapId[blockSeq[i].second] < m_partitions[partId].moduleList.size() + 2 );
	    areaUsed += fplan->m_modules[ blockSeq[i].second ].m_area;
	    part[ mapId[blockSeq[i].second] ] = 1;    // fixed at the part1
	    fixedNodes++;
	}

    //printf( "fix = %d (%.1f)\n", fixedNodes, 100.0 * fixedNodes / m_partitions[partId].moduleList.size() );

}
*/

bool CMinCutPlacer::MakePartition( int partId, 
	int& edgecut, 
	int& new1, 
	int& new2, 
	CUTDIR cutDir/*=NONE_CUT*/,
	bool noSubRegion/*=false*/, 
	bool considerMovable/*=true*/, 
	bool moveHalf/*=false*/,  
	double moveRatio/*=0.5*/ )
{

#   if 0
    // Show part info
    printf( "In MakePartition(%d)\n", partId );
    cout << "Partition " << partId << " # = " << m_partitions[partId].moduleList.size()
        << " (" << m_partitions[partId].left << "," << m_partitions[partId].bottom << ")-(" 
		<< m_partitions[partId].right << "," << m_partitions[partId].top << ")" << endl;
#    endif
    
    //const double wsAllocUtil = 0.9; // util < wsAllocUtil, do whitespace allocation
    const double bUse2D = true;

    if( (int)m_partitions[partId].moduleList.size() <= param.minBlocks )	// no more cut
    {
	new1 = new2 = -1;
	return false;
    }

    if( m_partitions[partId].area < fplan->m_rowHeight )   // less than a site
    {
        cout << " Bin is too small! (m# =" << (int)m_partitions[partId].moduleList.size() << ")\n";
	new1 = new2 = -1;
        return false;
    }

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
	    mid = round( (top-bottom) / fplan->m_rowHeight / 2.0 ) * fplan->m_rowHeight + bottom; // align rows
	
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
#       if 0
	    exit(0);
#       endif
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
	return false;
    }
    if( freeArea1 < fplan->m_rowHeight && freeArea0 > totalMovableArea * 0.1 )    
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
	return false;
    }

    // 3. Calc dummy area.
    double dummyArea = 0;
    if( freeArea0 > freeArea1 )
    {
	dummyArea = totalMovableArea * ( (freeArea0 - freeArea1) / (freeArea0 + freeArea1) );
	moduleWeight[1] = (int)round( dummyArea * extraWeight );
	moduleWeight[0] = 0;
    }
    else if( freeArea0 < freeArea1 )
    {
	dummyArea = totalMovableArea * ( (freeArea1 - freeArea0) / (freeArea0 + freeArea1) );
	moduleWeight[1] = 0;
	moduleWeight[0] = (int)round( dummyArea * extraWeight );
    }

    bool doWSAlloc = false;

#if 0
    double util = m_partitions[partId].GetUtilization();
    if( util < wsAllocUtil )
    {
	// do whitespace allocation
	doWSAlloc = true;

	// QP current partition
	QPPartition( partId );

	double massX, massY;
	GetCenterOfMass( partId, massX, massY );    // todo, using 1D center is enough
	double width = sqrt( m_partitions[partId].totalMovableArea * 
		             (m_partitions[partId].right - m_partitions[partId].left) /
			     (m_partitions[partId].top   - m_partitions[partId].bottom) 
			   );	
	double height = m_partitions[partId].totalMovableArea / width;
	
	double ratio;
	if( cutDir == V_CUT )
	{
	    double center = 0.5 * (m_partitions[partId].left + m_partitions[partId].right);
	    ratio = 0.5 + ( center - massX )  / width;
	    /*printf( "V-cut ratio = %f  (util = %f, ar = %f)\n", 
		    ratio, 
		    m_partitions[partId].totalMovableArea / m_partitions[partId].area,
		   (m_partitions[partId].top   - m_partitions[partId].bottom) / 
		   (m_partitions[partId].right - m_partitions[partId].left) );*/
	}
	else // cutDir == H_CUT
	{
	    double center = 0.5 * (m_partitions[partId].bottom + m_partitions[partId].top);
	    ratio = 0.5 + ( center - massY ) / height;
	    /*printf( "H-cut ratio = %f (util = %f, ar = %f)\n", 
		    ratio,
		    m_partitions[partId].totalMovableArea / m_partitions[partId].area,
		   (m_partitions[partId].top   - m_partitions[partId].bottom) / 
		   (m_partitions[partId].right - m_partitions[partId].left) );*/
	}
	if( ratio > 1 )      ratio = 1;
	else if( ratio < 0)  ratio = 0;
	double targetArea0 = m_partitions[partId].totalMovableArea * ratio;
	double targetArea1 = m_partitions[partId].totalMovableArea - targetArea0;

	if( targetArea0 > freeArea0 )
	{
	    targetArea1 += targetArea0 - freeArea0;
	    targetArea0 = freeArea0;	    
	}
	else if( targetArea1 > freeArea1 )
	{
	    targetArea0 += targetArea1 - freeArea1;
	    targetArea1 = freeArea1;	    
	}
    
	if( targetArea0 > targetArea1 )
	{
	    dummyArea = totalMovableArea * ( (targetArea0 - targetArea1) / (targetArea0 + targetArea1) );
	    moduleWeight[0] = 0;
	    moduleWeight[1] = (int)round( dummyArea * extraWeight );
	}
	else if( targetArea0 < targetArea1 )
	{
	    dummyArea = totalMovableArea * ( (targetArea1 - targetArea0) / (targetArea0 + targetArea1) );
	    moduleWeight[0] = (int)round( dummyArea * extraWeight );
	    moduleWeight[1] = 0;
	}
    }
#endif // Analytical Constraint Generation for whitespace distribution
    


#if 0

    // QP guided min-cut (TEST)
    if(	m_partitions[partId].moduleList.size() > 50 )
    {
	if( doWSAlloc == false )
	    QPPartition( partId );

	const double largeWeight = 0.8;
	const double smallWeight = 0.2;
	
	vector< pair<double, int> > blockSeq;
	double coordinateBound0;
	double coordinateBound1;
	if( cutDir == V_CUT )
	{
	    coordinateBound0 = m_partitions[partId].left * largeWeight + m_partitions[partId].right * smallWeight;
	    coordinateBound1 = m_partitions[partId].left * smallWeight + m_partitions[partId].right * largeWeight;
	    for( unsigned int i=0; i<m_partitions[partId].moduleList.size(); i++ )
	    {
		int id = m_partitions[partId].moduleList[i];
		blockSeq.push_back( make_pair( fplan->m_modules[id].m_cx, id ) );    // < x, id >
	    }
	}
	else // H_CUT
	{
	    coordinateBound0 = m_partitions[partId].bottom * largeWeight + m_partitions[partId].top * smallWeight;
	    coordinateBound1 = m_partitions[partId].bottom * smallWeight + m_partitions[partId].top * largeWeight;
	    for( unsigned int i=0; i<m_partitions[partId].moduleList.size(); i++ )
	    {
		int id = m_partitions[partId].moduleList[i];
		blockSeq.push_back( make_pair( fplan->m_modules[id].m_cy, id ) );    // < y, id >
	    }
	}
	// sort from min to max
	sort( blockSeq.begin(), blockSeq.end() );   // TODO, sort using only "first"

	int fixedNodes = 0;
	
	// set fixed node in part0
	const double areaRatioBound = 0.20;
	double areaUsed = 0;
	for( unsigned int i=0; i<blockSeq.size(); i++ )
	{
	    if( areaUsed > freeArea0 * areaRatioBound )
	    {
		//printf( "." );
		break;
	    }
	    if( blockSeq[i].first > coordinateBound0 )
	    {
		//printf( "-" );
		break;
	    }
	    assert( (int)blockSeq[i].second < fplan->m_modules.size() );
	    assert( mapId[blockSeq[i].second] < (int)m_partitions[partId].moduleList.size() + 2 );
	    areaUsed += fplan->m_modules[ blockSeq[i].second ].m_area;
	    part[ mapId[blockSeq[i].second] ] = 0;    // fixed at the part0
	    fixedNodes++;
	}

	// set fixed node in part1
	areaUsed = 0;
	if( blockSeq.size() > 0 )
	for( int i=(int)blockSeq.size()-1; i>=0; i-- )
	{
	    if( areaUsed > freeArea1 * areaRatioBound )
	    {
		//printf( "." );
		break;
	    }
	    if( blockSeq[i].first < coordinateBound1 )
	    {
		//printf( "-" );
		break;
	    }
	    if( blockSeq[i].second >= (int)fplan->m_modules.size() )
		printf( "i= %d of %d   %f %d\n", i, blockSeq.size(), blockSeq[i].first, blockSeq[i].second );
	    assert( blockSeq[i].second < fplan->m_modules.size() );
	    assert( mapId[blockSeq[i].second] < m_partitions[partId].moduleList.size() + 2 );
	    areaUsed += fplan->m_modules[ blockSeq[i].second ].m_area;
	    part[ mapId[blockSeq[i].second] ] = 1;    // fixed at the part1
	    fixedNodes++;
	}

	//printf( "fix = %d (%.1f)\n", fixedNodes, 100.0 * fixedNodes / m_partitions[partId].moduleList.size() );

    }

#endif

	    
    // 4. Calculate the unbalance factor 
    int ubfactor = 1;
    
    if( usedFixArea0 + usedFixArea1 == 0 )
	ubfactor = 15;
    else
	ubfactor = 1;

    if( param.hmetis_ubfactor > 0 ) // user specified
	ubfactor = param.hmetis_ubfactor;
  
    if( doWSAlloc )
	ubfactor = 1;

	
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
    bool insidePart;		// whole net inside the partition
    bool outsidePart;		// whole net outside the partition
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

	    if( nodes.size() == 1 )
		continue;

	    // Put into hMETIS net data structure
	    int weight = (int)round( extraNetWeight * centerDistance );
	    if( weight <= 0 )   // the weight should > 0 for this case.
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

	    //if( considerMovable == false && 
	    //    fplan->m_modules[ fplan->m_pins[fplan->m_nets[netId][j]].moduleId ].m_isFixed == false )
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
	if( pointMin.x <= pointMid0.x && pointMax.x >= pointMid1.x &&
	    pointMin.y <= pointMid0.y && pointMax.y >= pointMid1.y )
	    continue;   // The net is across two centers of the partitions (dont't care it!)

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


	// WIRE (both at 0) -- pointMid0
	CPoint min0 = pointMin;
	CPoint max0 = pointMax;
	min0.x = min( min0.x, pointMid0.x );
	max0.x = max( max0.x, pointMid0.x );
	min0.y = min( min0.y, pointMid0.y );
	max0.y = max( max0.y, pointMid0.y );
	double wire0 = Distance( min0, max0 );

	// WIRE (both at 1) -- pointMid1
	CPoint min1 = pointMin;
	CPoint max1 = pointMax;
	min1.x = min( min1.x, pointMid1.x );
	max1.x = max( max1.x, pointMid1.x );
	min1.y = min( min1.y, pointMid1.y );
	max1.y = max( max1.y, pointMid1.y );
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
	double wire01 = Distance( min01, max01 );

	// Determine the net weight
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
	    //cout << "centerDistance= " << mid1-mid0 << " dis1= " << dis1 << " dis2= " << dis2 << endl;
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

		// Add the net 
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
	//if( netCounter > fplan->m_nNets )
	//    cout << "\nERR! netCounter > fplan->m_nNets\n";

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

    //if( true )
    //{


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

	    //Start:=====================(indark)==========================
	    assert(netCounter < fplan->m_nNets * 1.1);
	    //End:=====================(indark)==========================
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

	    /*HMETIS_PartKway( moduleCounter, 
		    netCounter, 
		    moduleWeight, 
		    eptr, 
		    eind, 
		    netWeight,
		    kway, 
		    5, 
		    fplan->part_options, 
		    part, 
		    &edgecut );*/

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

    //}
    //else
    //{   
        //// PaToH
        //PaToH_Parameters args;
        //int partWeight[2];
        ////cout << "Init" << endl;
        //PaToH_Initialize_Parameters( &args, PATOH_CUTPART, PATOH_SUGPARAM_QUALITY );

        //// MISC
        //args._k = 2;    // 2-way
        ////args.outputdetail = PATOH_OD_MEDIUM;
        //args.seed = rand_seed;

        //// COARSENING
        //args.crs_VisitOrder = 3;    //g_part_run;
        //args.crs_alg = 6;   //g_part_run;

        //// INIT
        ////args.nofinstances = 10;
        //args.initp_alg = 13;       //g_part_run;
        ////args.initp_runno = 10;
        //args.initp_refalg = 2;

        //// UNCOARSENING
        ////args.final_imbal = 0.01;
        //args.final_imbal = ubfactor / 100.0;
        //args.ref_alg = 2;   //g_part_run;

        ////cout << "Check" << endl;
        ////PaToH_Check_User_Parameters( &args, 63 );
        ////cout << "Part" << endl;
        //PaToH_CPartition_with_FixCells( &args, moduleCounter, netCounter, moduleWeight, netWeight, 
        //    eptr, eind, part, partWeight, &edgecut );
    //}

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

    double ratio = (p0Area+usedFixArea0) / (p0Area+p1Area+usedFixArea0+usedFixArea1);
    //if( debugLevel >= 5 )
    //    cout << " CPartition ratio: " << ratio << endl;


    // CREATE SUB-PARTITIONS =================================================

    double x1, y1;	        // center of partition 1
    double x2, y2;              // center of partition 2
    double cutMid;	        // the "real" cut position
    CPoint center0, center1;


    // STEP 1: Calculate new center point for the partition.
    
    // The smaller the "moveFactor" is, the larger the cutline can move.
    //const double moveFactor = 0.33;
    const double moveFactor = 0.15;    
    
    // If both regions' utilization are smaller the "utilTarget", stop moving the cutline
    //const double utilTarget = 0.96;
    
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
	if( ( usedFixArea0 + usedFixArea1 ) == 0 && ratio != 0 && ratio != 1 )
	{
	    cutMid = ( left + ratio * (right-left) );	    // No pre-placed blocks, equally spacing
	    bCutShifting = false;
	}
	else
	    cutMid = mid;
#endif

	if( doWSAlloc )
	{
	    cutMid = mid;
	    bCutShifting = false;
	}

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
	    util0 = (p0Area)/(newArea0-newFixedArea0);
	    util1 = (p1Area)/(newArea1-newFixedArea1);

	    ///// ispd06 test
	    //if( whileCounter == 1 && util0 < utilTarget && util1 < utilTarget )
	    //	break;

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

	if( doWSAlloc )
	{
	    cutMid = mid;
	    bCutShifting = false;
	}
	
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
	    util0 = (p0Area)/(newArea0-newFixedArea0);
	    util1 = (p1Area)/(newArea1-newFixedArea1);

	    //// ispd06 test, want to spread cell evenly
	    //if( whileCounter == 1 && util0 < utilTarget && util1 < utilTarget )
	    //{
	    //	bestCutMid = cutMid;
	    //	break;
	    //}

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

#if 1
	if( cutMid >= top )
	{
	    printf( "bottom = %f bottomBound = %f  cutMid = %f  topBound = %f top = %f\n", 
		    bottom, bottomBound, cutMid, topBound, top );
	}
#endif
	
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
	    if( part0BlockCount > 1 )
	    {
		part0Id = AddPartition( partId, left, bottom, cutMid, top );
		m_partitions[partId].childPart0 = part0Id;
	    }
	}
	// right
	if( m_partitions[partId].childPart1 > 0 )
	{
	    part1Id = AddPartition( partId, cutMid, bottom, right, top, m_partitions[partId].childPart1 );
	}
	else
	{
	    if( part1BlockCount > 1 )
	    {
		part1Id = AddPartition( partId, cutMid, bottom, right, top );
		m_partitions[partId].childPart1 = part1Id;
	    }
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
	    if( part0BlockCount > 1 )
	    {
		part0Id = AddPartition( partId, left, bottom, right, cutMid );		
		m_partitions[partId].childPart0 = part0Id;
	    }
	}
	// top
	if( m_partitions[partId].childPart1 > 0 )
	{
	    part1Id = AddPartition( partId, left, cutMid, right, top, m_partitions[partId].childPart1 );
	}
	else
	{
	    if( part1BlockCount > 1 )
	    {
		part1Id = AddPartition( partId, left, cutMid, right, top );
		m_partitions[partId].childPart1 = part1Id;
	    }
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

#if 0
    //=======================================================================
    //// STEP 4: Check utilization
    //=======================================================================
    if( m_partitions[partId].GetUtilization() <= 1.0 )
    {
        if( (m_partitions[part0Id].utilization > 1.0 || m_partitions[part1Id].utilization > 1.0 ))
        {
            if( part0Id >0 && part1Id> 0 )
            {
            cout << " ** U(" << m_partitions[part0Id].utilization 
				<< ":" << m_partitions[part1Id].utilization
				//<< ") u(" << util0 
				//<< ":" << util1 
				<< ") A(" << p0Area 
				<< ":" << p1Area 
				<< ") Am(" << m_partitions[part0Id].totalMovableArea 
				<< ":" << m_partitions[part1Id].totalMovableArea 
				<< ") Af(" << m_partitions[part0Id].totalFixedArea 
				<< ":" << m_partitions[part1Id].totalFixedArea 
				<< ") N(" << (int)m_partitions[part0Id].moduleList.size() 
				<< ":" << (int)m_partitions[part1Id].moduleList.size() << ") ";
            }
            else if( part0Id >0 )
            {
            cout << " ** U(" << m_partitions[part0Id].utilization 
				<< ":_" 
				//<< ") u(" << util0 
				//<< ":X"  
				<< ") A(" << p0Area 
				<< ":_"  
				//<< ") Am(" << m_partitions[part0Id].totalMovableArea 
				//<< ":X"  
				//<< ") Af(" << m_partitions[part0Id].totalFixedArea 
				//<< ":X" 
				<< ") N(" << (int)m_partitions[part0Id].moduleList.size() 
				<< ":_) ";
            }
            else
            {
            cout << " ** U(_" 
				<< ":" << m_partitions[part1Id].utilization
				//<< ") u(X" 
				//<< ":" << util1 
				<< ") A(_" 
				<< ":" << p1Area 
				//<< ") Am(X" 
				//<< ":" << m_partitions[part1Id].totalMovableArea 
				//<< ") Af(X" 
				//<< ":" << m_partitions[part1Id].totalFixedArea 
				<< ") N(_" 
				<< ":" << (int)m_partitions[part1Id].moduleList.size() << ") ";
            }

			if( cutDir == V_CUT )
			{
				cout << "V-CUT (H= " << (top-bottom) << ") ";
				cout << (cutMid-left) << ":" << (right-cutMid) << " VVVVVVVVVVV\n";
           	    cout << "  ratio =" <<  (p0Area+usedFixArea0)/(p0Area+p1Area+usedFixArea0+usedFixArea1) << "\n\n";
			}
			else
			{
				cout << "H-CUT (W= " << (right-left) << ") ";	
				cout << (cutMid-bottom)/fplan->m_rowHeight << ":" << (top-cutMid)/fplan->m_rowHeight << "\n";
			}
        }
    }
#endif

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


void CMinCutPlacer::QPPartition( const int& partId )	// 2006-02-05 (donnie)
{
    CPlaceDBQPPlacer* pqplace = new CPlaceDBQPPlacer( *fplan );
    double left   = m_partitions[partId].left;
    double right  = m_partitions[partId].right;
    double top    = m_partitions[partId].top;
    double bottom = m_partitions[partId].bottom;
    pqplace->QPplace( left, bottom, right, top, 
	    m_partitions[partId].moduleList, 
	    m_partitions[partId].fixModuleList );
    delete pqplace;
}


void CMinCutPlacer::GetCenterOfMass( const int& partId, double& x, double& y ) // 2006-02-05 (donnie)
{
    x = 0;
    y = 0;
    double totalArea = 0;
    for( unsigned int i=0; i<m_partitions[partId].moduleList.size(); i++ )
    {
	int id = m_partitions[partId].moduleList[i];
	x += fplan->m_modules[id].m_cx * fplan->m_modules[id].m_area;
	y += fplan->m_modules[id].m_cy * fplan->m_modules[id].m_area;
	totalArea += fplan->m_modules[id].m_area;
    }
    x /= totalArea;
    y /= totalArea;
}


double CMinCutPlacer::GetFixBlockAreaInPartition( const int partId, 
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


double CMinCutPlacer::GetPartHPWL( int partId )
{
    double HPWL = 0;
    for( int i=0; i<(int)m_partitions[partId].netList.size(); i++ )
    {
        HPWL += fplan->GetNetLength( m_partitions[partId].netList[i] );
    }
    return HPWL;
}


CUTDIR CMinCutPlacer::GetCutDirForPart( int partId )
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

void CMinCutPlacer::SavePartBlockList( int partId )
{
    m_partitions[partId].moduleListBak = m_partitions[partId].moduleList;
    m_partitions[partId].netListBak = m_partitions[partId].netList;
}

void CMinCutPlacer::SavePartBlockLocation( int partId )
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


void CMinCutPlacer::RestorePartBlockList( int partId )
{
    m_partitions[partId].moduleList = m_partitions[partId].moduleListBak;
    m_partitions[partId].netList = m_partitions[partId].netListBak;
}

void CMinCutPlacer::RestorePartBlockLocation( int partId )
{
    for( int i=0; i<(int)m_partitions[partId].blockPosList.size(); i++ )
    {
        CPartition::BlockPosition& blockPos = m_partitions[partId].blockPosList[i];
        fplan->SetModuleLocation( blockPos.id, blockPos.x, blockPos.y ); 
    }
}


bool CMinCutPlacer::NewPartMem(void)
{
    // Allocate memory for partitioning.
    // Our program will not release memory until the end of the program.
    part         = new int[fplan->m_nModules+2];	// 2 fixed node
    part_best    = new int[fplan->m_nModules+2];	// 2 fixed node
    moduleWeight = new int[fplan->m_nModules+2];	// 2 fixed node
    netWeight    = new int[int(fplan->m_nNets*1.2)];       // multi 1.10 to avoid too much Exact Net Weight
    eptr         = new int [int(fplan->m_nNets*1.2)+1]; // indark change to 2
    eind         = new int [int(fplan->m_nPins*1.2)];// indark change to 2
    if( part == NULL || moduleWeight == NULL || netWeight == NULL || eptr == NULL || eind == NULL )
    {
	cout << "Out of memory\n";
	exit(-1);
    }
    return true;
}

void CMinCutPlacer::DeletePartMem(void)
{
    delete [] part;
    delete [] moduleWeight;
    delete [] netWeight;
    delete [] eptr;
    delete [] eind;
}


// TODO: "mid", "mid0", and "mid1" are not weighted center
void CMinCutPlacer::CalcPartPriority(int partId)
{
    
    double top = m_partitions[partId].top;
    double bottom = m_partitions[partId].bottom;
    double left = m_partitions[partId].left;
    double right = m_partitions[partId].right;

    // TEST 2006-01-10
    m_partitions[partId].priority = (top-bottom)*(right-left);
    return;

    
    // TEST 0215: Find number of fixed/movable blocks outside of the region
    CUTDIR cutDir = GetCutDirForPart( partId );
    double mid0, mid, mid1;
    if( cutDir == V_CUT )
    {
	mid = (left+right) / 2.0;
	mid0 = (left+mid) / 2.0;
	mid1 = (mid+right) / 2.0;
    }
    else
    {
	mid = (top+bottom) / 2.0;
	mid0 = (bottom+mid) / 2.0;
	mid1 = (top+mid) / 2.0;
    }

    //cout << mid0 << " " << mid << " " << mid1 << endl;

    set<int> setId;
    int outMove = 0;
    int outFix = 0;
    int unsureLine = 0;
    bool bUnsureLine = false;
    for( int i=0; i<(int)m_partitions[partId].netList.size(); i++ )
    {
	bUnsureLine = false;
	int netId = m_partitions[partId].netList[i];
	for( int j=0; j<(int)fplan->m_nets[netId].size(); j++ )
	{
	    if( setId.find( fplan->m_pins[ fplan->m_nets[netId][j] ].moduleId ) == setId.end() ) 
	    {
		// The terminal is outside of the region. 
		if( fplan->m_modules[ fplan->m_pins[ fplan->m_nets[netId][j] ].moduleId ].m_isFixed )
		{
		    outFix++;
		}
		else 
		{
		    if( cutDir == V_CUT )   
		    {
			// Check x-coordinate
			if( fplan->m_modules[ fplan->m_pins[ fplan->m_nets[netId][j] ].moduleId ].m_cx <= mid0 || 
				fplan->m_modules[ fplan->m_pins[ fplan->m_nets[netId][j] ].moduleId ].m_cx >= mid1 )
			{
			    outFix++;
			}
			else
			{
			    bUnsureLine = true;
			    outMove++;
			}
		    }
		    else
		    {
			// Check y-coordinate
			if( fplan->m_modules[ fplan->m_pins[ fplan->m_nets[netId][j] ].moduleId ].m_cy <= mid0 || 
				fplan->m_modules[ fplan->m_pins[ fplan->m_nets[netId][j] ].moduleId ].m_cy >= mid1 )
			{
			    outFix++;
			}
			else
			{
			    bUnsureLine = true;
			    outMove++;         
			}
		    }
		}
	    }
	} // For each net terminal

	if( bUnsureLine == true )
	    unsureLine++;

    } // For each net in the parent partition

    //cout << "OutFix= " << outFix << "  outMove= " << outMove << " Ratio= " << (double)outFix/outMove << endl;
    //cout << "ID= " << partId << "  outMove= " << outMove << " outNet= " << unsureLine 
    //    << " totalNet= " << m_partitions[partId].netList.size() 
    //    << " Ratio= " << (double)unsureLine/m_partitions[partId].netList.size() << endl;

    //m_partitions[partId].priority = (double)unsureLine;
    m_partitions[partId].priority = (float)unsureLine/m_partitions[partId].netList.size();
    //m_partitions[partId].priority = (double)outMove;///m_partitions[partId].netList.size();



}



