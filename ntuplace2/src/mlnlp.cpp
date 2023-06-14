#include "placedb.h"
#include "MyNLP.h"
#include "cluster.h"
#include "PlaceDBQP.h"	    // SolveQP
#include "ParamPlacement.h"

#include <cstdio>
using namespace std;


#include "TetrisLegal.h"
using namespace Jin;

extern bool bOutInterPL;
extern bool allMacroMove;

bool multilevel_nlp( CPlaceDB& placedb, string outFilePrefix,
                     int ctype,	
	             int targetBlock, // target cluster block #
	             double ratio,    // clustering ratio
		     double final_target_nnb,      // top-level target nnb
		     double final_target_density,  // top-level target density
		     double incFactor,	// multiplier inc rate
		     double wWire,	// wireWeight
		     int maxLevel,
		     double weightLevelDecreaingRate,
		     double target_utilization
		   )
{
    double mlnlp_start = seconds();
    //double target_utilization = 1.0;

    if( param.bShow )
    {
	printf( "   targetBlock   = %d\n", targetBlock );
	printf( "   cluster ratio = %f\n", ratio );
	//printf( "   final_nnb     = %f\n", final_target_nnb );
	//printf( "   final_density = %f\n", final_target_density );
	printf( "   inc factor    = %f\n", incFactor );
	printf( "   wWire         = %f\n", wWire );
	printf( "   wWire L Decrease = %f\n", weightLevelDecreaingRate );
	printf( "   target util   = %f\n", target_utilization );
    }
    
    printf( "Set all module orient N\n" );
    for( unsigned int i=0; i< placedb.m_modules.size(); i++ )
	if( placedb.m_modules[i].m_isFixed == false )
	    placedb.SetModuleOrientation( i, 0 );
   
    int levels = 0;

    // construct a hierarchy of clusters 
    vector<CPlaceDB> placedb_clustered;
    vector<CClustering> clusters;
    //CPlaceDB dummyDB;
    
    CPlaceDB* currentDB = &placedb;
    //int currentBlock = currentDB->m_modules.size();
    
    //currentDB->OutputGnuplotFigure( "init_1.plt", false );
 
    //============================================================
    // add dummy blocks
    int fillNumber = 0;  
    currentDB->Init();	// calculate movable block area	/ free space
    double dummyArea = ( currentDB->m_totalMovableModuleArea / currentDB->m_totalMovableModuleNumber );
    dummyArea *= 25; // make it larger
    double dummyWidth = sqrt( dummyArea );
    double fillUtil = 0.8;
    double fillArea = ( currentDB->m_totalFreeSpace * fillUtil - currentDB->m_totalMovableModuleArea );
    fillNumber = static_cast<int>( fillArea / dummyArea );

    fillNumber = 0; // do not fill dummy blocks
    
    for( int i=0; i<fillNumber; i++ )
    {
	string name = "dummy";
	currentDB->AddModule( name, dummyWidth, dummyWidth, false );
	int id = (int)currentDB->m_modules.size() - 1;
	currentDB->m_modules[id].m_isDummy = true;
	
	double w = currentDB->m_coreRgn.right - currentDB->m_coreRgn.left;
	double h = currentDB->m_coreRgn.top - currentDB->m_coreRgn.bottom;
	w *= 0.8;
	h *= 0.8;
	currentDB->MoveModuleCenter( id, 
		currentDB->m_coreRgn.left + w*0.1 + rand() % (int)w, 
		currentDB->m_coreRgn.bottom + h*0.1 + rand() % (int)h ); 
    }
    if( param.bShow )
	printf( "Fill number = %d\n", fillNumber );
    currentDB->Init();
    //============================================================

    double clustering_start = seconds();
    int currentBlock = currentDB->GetMovableBlockNumber();
    int expLevel = (int)ceil( log( static_cast<double>(currentBlock-fillNumber) / targetBlock ) / log( ratio ) ) + 1;
    if( expLevel < 0 )
	expLevel = 0;
    if( param.bShow )
    {
	printf( "Expect level # = %d\n", expLevel );
	if( maxLevel < 200 )
	    printf( "Max level # = %d\n", maxLevel );
    }
    
    placedb_clustered.resize( expLevel+1 );
    while( currentBlock-fillNumber > targetBlock && levels < maxLevel-1 )
    {
	levels++;
	//placedb_clustered.resize( levels+1 );
	
	printf( "level %d, block # %d, movable block # %d > %d, do clustering...", 
		levels, currentDB->m_modules.size(), currentBlock, targetBlock );
	fflush( stdout );
	
	// segmentaion fault, BUG??
	//CPlaceDB dummyDB;
	//placedb_clustered.push_back( dummyDB );

	CClustering dummyCluster;	
	clusters.push_back( dummyCluster );

	clusters[levels-1].clustering( *currentDB, placedb_clustered[levels-1], (int)((currentBlock-fillNumber)/ratio), 1.5, ctype );
	
	printf( " done\n" );
	fflush( stdout );
	
        currentDB = &placedb_clustered[levels-1];
	currentBlock = currentDB->GetMovableBlockNumber();	
    }
    printf( "block # %d, movable block # %d\n", currentDB->m_modules.size(), currentBlock );
    printf( "clustering time: %.2f\n", seconds() - clustering_start );
    
    //currentDB->OutputGnuplotFigure( "init_2.plt", false );  // show physical clustering positions

   
    // generate level parameters 
    int totalLevels = levels + 1;
    double start_density = 1.10;
    //double start_density = 1.4;
    double startSmoothDelta = 4.0;
    //double startSmoothDelta = 5.0;
    final_target_density = 1.01;    // test
    double finalSmoothDelta = 1.0;
    vector<double> levelSmoothDelta;
    vector<double> levelTargetOver;
    levelSmoothDelta.resize( totalLevels );
    levelTargetOver.resize( totalLevels );
    printf( "\n" ); 
    for( int i=0; i<levels; i++ )
    {
	int currentLevel = i+1;
	levelTargetOver[i] = start_density - ( start_density - final_target_density ) * (currentLevel) / (totalLevels);
	//levelTargetOver[i] = final_target_density + 0.02;
	//levelSmoothDelta[i] = 2.0 + 6.0 * (totalLevels-currentLevel-1) / (totalLevels-1);	// minimum = 2
	levelSmoothDelta[i] = startSmoothDelta - ( startSmoothDelta - finalSmoothDelta ) * (currentLevel) / (totalLevels); 

	if( param.bShow )
	    printf( "Level %d\tBlock %d\tPin %d\tDelta %.3f\tOver %.3f\n", 
		    i+1, placedb_clustered[levels-i-1].m_modules.size(), 
		    placedb_clustered[levels-i-1].m_pins.size(), levelSmoothDelta[i], levelTargetOver[i] );	
    }
    if( param.bShow )
	printf( "Level %d\tBlock %d\tPin %d\tDelta %.3f\tOver %.3f\n\n", 
		levels+1, placedb.m_modules.size(), 
		placedb.m_pins.size(),
		finalSmoothDelta, final_target_density );
    
    
    // init solution
    printf( "Solve QP\n" );
    CPlaceDBQPPlacer* pqplace = new CPlaceDBQPPlacer( *currentDB );
    pqplace->QPplace();
    delete pqplace;
    if( bOutInterPL )
    {
	string file = outFilePrefix + "_global_qp.plt";
	currentDB->OutputGnuplotFigure( file.c_str(), false );
    }
  
    if( param.bShow ) 
    {
	printf( "\n block = %d, net = %d, pin = %d\n", currentDB->m_modules.size(),
		currentDB->m_nets.size(), currentDB->m_pins.size() );
	currentDB->OutputGnuplotFigure( "init_qp.plt", false );
    }

    // spread dummy cells
    if( fillNumber > 0 )
    {
	for( unsigned int i=0; i<currentDB->m_modules.size(); i++ )
	{
	    if( currentDB->m_modules[i].m_isDummy == true )
	    {
		double w = currentDB->m_coreRgn.right - currentDB->m_coreRgn.left;
		double h = currentDB->m_coreRgn.top - currentDB->m_coreRgn.bottom;
		w *= 0.8;
		h *= 0.8;
		currentDB->MoveModuleCenter( i, 
			currentDB->m_coreRgn.left + w*0.1 + rand() % (int)w, 
			currentDB->m_coreRgn.bottom + h*0.1 + rand() % (int)h ); 
	    }
	}
    }

    

    // placement on the clsutered placedb
    int currentLevel = 0;
    double weightWire = wWire;
    
    while( levels > 0 )
    {
	currentLevel ++;

	// TEST for > 2 levels
	if( currentLevel >= 2 )
	    weightWire = weightWire / weightLevelDecreaingRate;

	MyNLP* mynlp = new MyNLP( *currentDB );
	
	bool noRelaxSmooth = false;
  
	mynlp->m_smoothDelta = levelSmoothDelta[currentLevel-1];
	mynlp->m_useBellPotentialForPreplaced = true;
        mynlp->m_earlyStop = true;	
	mynlp->m_lookAheadLegalization = false;
	mynlp->m_incFactor  = incFactor;
	mynlp->m_targetUtil = target_utilization;
	if( param.bShow )
	{
	    printf( "NLP LEVEL %d of %d (target overflow %f, smooth delta %f)\n", 
		    currentLevel, totalLevels, levelTargetOver[currentLevel-1], levelSmoothDelta[currentLevel-1] );	
	    printf( "[block = %d, net = %d, pin = %d]\n", currentDB->m_modules.size(),
		    currentDB->m_nets.size(), currentDB->m_pins.size() );
	}
	
	// 2006-03-26
	/*if( currentLevel == 1 )
	{
	    double oldStep = param.step;
	    param.step = 0.1;
	    mynlp->MySolve( weightWire, levelTargetOver[currentLevel-1], currentLevel, noRelaxSmooth );
	    param.step = oldStep;
	}
	else	*/
	    mynlp->MySolve( weightWire, levelTargetOver[currentLevel-1], currentLevel, noRelaxSmooth );
	delete mynlp;

	
	// Macro shifter
	double avgHeight = 0;
	int count = 0;
	for( unsigned int i=0; i<currentDB->m_modules.size(); i++ )
	{
	    if( currentDB->m_modules[i].m_isFixed )
		continue;
	    count++;
	    avgHeight += currentDB->m_modules[i].m_height;
	}
	avgHeight /= count;
	CTetrisLegal legal( *currentDB );
	bool bMacroShifter = legal.MacroShifter( avgHeight * 4 / currentDB->m_rowHeight, false );
	if( param.bShow )
	{
	    if( false == bMacroShifter )
		printf( "MACRO SHIFTER FAILED!\n" );
	    else
		printf( "MACRO SHIFTER SUCCEEDED!\n" );
	}

	// TODO: remove dummy blocks

	
	if( bOutInterPL )
	{
	    char file[200];
	    sprintf( file, "%s_global_level_%d.plt", outFilePrefix.c_str(), currentLevel );
	    currentDB->OutputGnuplotFigure( file, false );
	}

	double tUsed = seconds() - mlnlp_start;
	printf( "\n########### [TotalTime = %.2f sec = %.2f min] ###########\n\n", tUsed, tUsed / 60.0 );
	if( levels > 1 )
	{
	    clusters[levels-1].declustering( placedb_clustered[levels-1], placedb_clustered[levels-2] );
	    currentDB = &placedb_clustered[levels-2];
	    //printf( "\n ************************\n" );
	}
	else	// == 1
	{
	    // to top level
	    clusters[levels-1].declustering( placedb_clustered[levels-1], placedb );
	    //printf( "\n ******* top level ******\n" );
	    break;
	}	    
	levels--;
    }


    if( param.bShow )
    {
	printf( "NLP LEVEL %d of %d (target density %f, smooth delta %d)\n", 
		currentLevel+1, totalLevels, final_target_density, 1 );	
    }
    
    if( param.topDblStep )  // 2006-03-25
	param.step *= 2.0;

    MyNLP* mynlp = new MyNLP( placedb );
    mynlp->m_useBellPotentialForPreplaced = true;
    mynlp->m_topLevel = true;
    mynlp->m_lookAheadLegalization = true;
    mynlp->m_incFactor  = incFactor;
    mynlp->m_targetUtil = target_utilization;
    mynlp->m_smoothDelta = finalSmoothDelta;
    bool isLegal = mynlp->MySolve( weightWire, final_target_density, currentLevel+1, true );
    delete mynlp;

    if( fillNumber > 0 )
    {
	placedb.m_modules.resize( placedb.m_modules.size() - fillNumber );
    }
    
    return isLegal;
}
