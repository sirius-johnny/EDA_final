#include "TetrisLegal.h"
#include <algorithm>
#include <cmath>
#include <limits>
#include <list>
#include "placebin.h"
#include "ParamPlacement.h"
#include "macrolegal.h"

//test code
//#include "cellmoving.h"
//@test code

#include "ParamPlacement.h"

using namespace std;
using namespace Jin;

//#define _LEGALLOG_

CPlaceDB* LessXCoor::m_placedb = 0;
CPlaceDB* LessXCoorMacroFirst::m_placedb = 0;
CPlaceDB* LessXCoorMacroPrior::m_placedb = 0;
double LessXCoor::m_macro_factor = 0;

bool CTetrisLegal::DoLeftRightUntil( const double& util, const double& stop_prelegal_factor )
{
    const double search_step = 0.01;
    //const double search_bound = 0.99;
    const double search_bound = stop_prelegal_factor;

    double prelegal_factor = m_prelegal_factor + search_step;

    bool bFinish = false;
    bool bLegalLeft = false;
    
    SaveOrig();
    
    //Create CPlaceBin to compute density penalty
    CPlaceBin* pBin;
    pBin = new CPlaceBin( m_placedb );
    pBin->CreateGrid( m_placedb.m_rowHeight * 10.0 );


    if( param.bShow )
    {
	cout << "Legalize to left" << endl;	
    }

    while( (!bFinish) && (prelegal_factor > search_bound) )
    {
	prelegal_factor = prelegal_factor - search_step;
	//cout << "Legalization factor: " << prelegal_factor << endl; // by donnie
	if( param.bShow )
	    cout << "Legalization factor: " << prelegal_factor << " ";

	double t = seconds();

	RestoreOrig();
	SetLeftFreeSites();

	bool result = DoLeft( prelegal_factor );
	//bool result = DoLeftMacroFirst( prelegal_factor );

	if( result )
	{
	    double wirelength = m_placedb.CalcHPWL();

	    double cost = wirelength;	

	    if( param.bShow )
	    {
		printf(" oHPWL %.0f ", wirelength);
	    }

	    if( util > 0 )
	    {
		pBin->UpdateBinUsage();
		double penalty = pBin->GetPenalty( util );
		cost = cost * (1.0+(penalty/100.0));
		if( param.bShow )
		{
		    printf( " (p = %.2f) ", penalty );
		}
	    }


	    if( param.bShow )   // by donnie
	    {
		cout << "Success " << cost << " ";
		printf("runtime %.2f secs\n", seconds()-t);
	    }
	    else
	    {
		printf( "*" );
	    }
	    flush(cout);
	    if( cost < m_best_cost )	
	    {
		SaveBest( prelegal_factor, cost );
	    }
	    //Used to be legalized before,
	    //and find a worse cost:
	    //the search is terminated 
	    else if( bLegalLeft )
	    {
		bFinish = true;
	    }
	}
	else
	{
	    if( param.bShow )
	    {
		cout << "Fail at "; 
		const int unlegal_cellindex = m_cell_order.size() - m_unlegal_count;
		printf("(%d)th module %d (%.2f,%.2f) width: %.2f height: %.2f ",
			unlegal_cellindex,
			m_cell_order[unlegal_cellindex], 
			m_placedb.m_modules[m_cell_order[unlegal_cellindex]].m_x,
			m_placedb.m_modules[m_cell_order[unlegal_cellindex]].m_y,
			m_placedb.m_modules[m_cell_order[unlegal_cellindex]].m_width,
			m_placedb.m_modules[m_cell_order[unlegal_cellindex]].m_height );
		printf("runtime %.2f secs\n", seconds()-t);
	    }
	    else
	    {
		printf( "-" );
	    }
	    flush(cout);

	    //Used to be legalized before,
	    //and find a unlegalized solution:
	    //the search is terminated
	    if( bLegalLeft )
	    {
		bFinish = true;
	    }
	}

	bLegalLeft = bLegalLeft | result;

    }

    
    //Record the best solution for left
    if(bLegalLeft )
    {
	m_best_prelegal_factor_left = m_best_prelegal_factor;
	m_best_cost_left = m_best_cost;
	m_bestLocations_left = m_bestLocations;
    }

    //test code
    //m_placedb.OutputGnuplotFigureWithZoom( "left", false, true, true );
    //@test code
    
    RestoreOrig();
    ReversePlacement();
    ReverseLegalizationData();
    SaveOrig();
    delete pBin;
    pBin = new CPlaceBin( m_placedb );
    pBin->CreateGrid( m_placedb.m_rowHeight * 10.0 );

    //test code
    //ofile = fopen( "reverse_site.log", "w" );
    //for( unsigned int i = 0 ; i < m_placedb.m_sites.size() ; i++ )
    //{
    //    const CSiteRow& curRow = m_placedb.m_sites[i];
    //    
    //    fprintf(ofile,"bottom: %.2f ", curRow.m_bottom );
    //    for( unsigned int j = 0 ; j < curRow.m_interval.size() ; j=j+2 )
    //    {
    //	fprintf(ofile, "(%.2f,%.2f) ", curRow.m_interval[j], curRow.m_interval[j+1] );
    //    }
    //    fprintf(ofile, "\n");
    //}
    //fclose( ofile );
    //@test code


    bFinish = false;	
    bool bLegalRight = false;
    prelegal_factor = m_prelegal_factor + search_step;
    m_best_cost = numeric_limits<double>::max();
    if( param.bShow )
	cout << "Legalize to right" << endl;	
    while( (!bFinish) && (prelegal_factor > search_bound) )
    {
	prelegal_factor = prelegal_factor - search_step;
	//cout << "Legalization factor: " << prelegal_factor << endl; // by donnie
	if( param.bShow )
	    cout << "Legalization factor: " << prelegal_factor << " ";

	double t = seconds();

	RestoreOrig();
	SetRightFreeSites();

	bool result = DoRight( prelegal_factor );

	if( result )
	{
	    double wirelength = m_placedb.CalcHPWL();

	    double cost = wirelength;

	    if( param.bShow )
	    {
		printf(" oHPWL %.0f ", wirelength);
	    }


	    if( util > 0 )
	    {
		pBin->UpdateBinUsage();
		double penalty = pBin->GetPenalty( util );
		cost = cost * (1.0+(penalty/100.0));

		if( param.bShow)
		{
		    printf( " (p = %.2f) ", penalty );
		}
	    }

	    if( param.bShow )
	    {
		cout << "Success " << cost << " ";
		printf("runtime %.2f secs\n", seconds()-t);
	    }
	    else
	    {
		printf( "*" );
	    }
	    flush(cout);
	    if( cost < m_best_cost )	
	    {
		SaveBest( prelegal_factor, cost );
	    }
	    //Used to be legalized before,
	    //and find a worse cost:
	    //the search is terminated 
	    else if( bLegalRight )
	    {
		bFinish = true;
	    }
	}
	else
	{
	    if( param.bShow )
	    {
		cout << "Fail at "; 
		const int unlegal_cellindex = m_cell_order.size() - m_unlegal_count;
		printf("(%d)th module %d (%.2f,%.2f) width: %.2f height: %.2f ",
			unlegal_cellindex,
			m_cell_order[unlegal_cellindex], 
			m_placedb.m_modules[m_cell_order[unlegal_cellindex]].m_x,
			m_placedb.m_modules[m_cell_order[unlegal_cellindex]].m_y,
			m_placedb.m_modules[m_cell_order[unlegal_cellindex]].m_width,
			m_placedb.m_modules[m_cell_order[unlegal_cellindex]].m_height );
		printf("runtime %.2f secs\n", seconds()-t);
	    }
	    else
	    {
		printf( "-" );
	    }
	    flush(cout);


	    //Used to be legalized before,
	    //and find a unlegalized solution:
	    //the search is terminated
	    if( bLegalRight )
	    {
		bFinish = true;
	    }
	}

	bLegalRight = bLegalRight | result;

    }

    //test code
    //m_placedb.OutputGnuplotFigureWithZoom( "right", false, true, true );
    //@test code

    bool bFinalLegal = bLegalLeft | bLegalRight;

    //Restore m_coreRgn
    ReverseLegalizationData();

    //Left is better
    if( (bLegalLeft && !bLegalRight) ||
	    (bLegalLeft && bLegalRight && m_best_cost_left < m_best_cost ) )
    {
	m_best_prelegal_factor = m_best_prelegal_factor_left;
	m_best_cost = m_best_cost_left;
	m_bestLocations = m_bestLocations_left;

	if( param.bShow )
	{
	    cout << "Best factor: left " << m_best_prelegal_factor << endl;
	}
	RestoreBest();
	//m_placedb.RemoveMacroSite();

    }
    //Right is better
    else if( (!bLegalLeft && bLegalRight) ||
	    (bLegalLeft && bLegalRight && m_best_cost_left >= m_best_cost ) ) 
    {
	if( param.bShow )
	{
	    cout << "Best factor: Right " << m_best_prelegal_factor << endl;
	}
	RestoreBest();
	ReversePlacement();
	//m_placedb.RemoveMacroSite();

    }
    else
    {
	if( param.bShow )
	{
	    cout << "Warning: general legalization fail" << endl;
	    ReversePlacement();
	    string prefix = param.outFilePrefix + "_legal_fail";
	    m_placedb.OutputGnuplotFigureWithZoom( prefix.c_str(), false, true, true );
	}

	RestoreOrig();
	ReversePlacement();

    }

    delete pBin;
    pBin = NULL;

    return bFinalLegal;
}

void CTetrisLegal::SetNonMacroProcessList( const vector<int>& macro_ids )
{
    set<int> macro_sets;

    for( unsigned int i = 0 ; i < macro_ids.size() ; i++ )
    {
	macro_sets.insert( macro_ids[i] );
    }

    m_process_list.clear();

    set<int>::iterator iteSet = macro_sets.begin();

    for( unsigned int i = 0 ; i < m_placedb.m_modules.size() ; i++ )
    {
	//This module is in macro_ids
	//Do not add into the process list
	if( iteSet != macro_sets.end() && static_cast<int>(i) == *iteSet )
	{
	    iteSet++;
	    continue;
	}	    

	const Module& curModule = m_placedb.m_modules[i];

	//Don't add fixed module
	if( curModule.m_isFixed )
	{
	    continue;
	}
	
	m_process_list.push_back(i);
    }
}

void CTetrisLegal::SetProcessList(void)
{
    m_process_list.clear();
    
    for( unsigned int i = 0 ; i < m_placedb.m_modules.size() ; i++ )
    {
	if( !m_placedb.m_modules[i].m_isFixed )
	    m_process_list.push_back(i);
    }

}

void CTetrisLegal::MacroShifterSaveOrigPosition(void)
{
    m_macro_shifter_orig_positions.clear();
    m_macro_shifter_orig_positions.reserve( m_macro_ids.size() );
    for( unsigned int i = 0 ; i < m_macro_ids.size() ; i++ )
    {
	const Module& curModule = m_placedb.m_modules[ m_macro_ids[i] ];
	m_macro_shifter_orig_positions.push_back( CPoint( curModule.m_x, curModule.m_y ) );
    }
}

void CTetrisLegal::MacroShifterRestoreOrigPosition(void)
{
    for( unsigned int i = 0 ; i < m_macro_ids.size() ; i++ )
    {
	m_placedb.SetModuleLocation( m_macro_ids[i], 
		m_macro_shifter_orig_positions[i].x, 
		m_macro_shifter_orig_positions[i].y );
    }

}

void CTetrisLegal::MacroShifterSaveBestPosition(void)
{
    m_macro_shifter_best_positions.clear();
    m_macro_shifter_best_positions.reserve( m_macro_ids.size() );
    for( unsigned int i = 0 ; i < m_macro_ids.size() ; i++ )
    {
	const Module& curModule = m_placedb.m_modules[ m_macro_ids[i] ];
	m_macro_shifter_best_positions.push_back( CPoint( curModule.m_x, curModule.m_y ) );
    }

}

void CTetrisLegal::MacroShifterRestoreBestPosition(void)
{
    for( unsigned int i = 0 ; i < m_macro_ids.size() ; i++ )
    {
	m_placedb.SetModuleLocation( m_macro_ids[i], 
		m_macro_shifter_best_positions[i].x, 
		m_macro_shifter_best_positions[i].y );
    }

}

bool CTetrisLegal::AggressiveCellLegalLocationsSearch( const int& cellid,
	vector<CLegalLocation>& legalLocations )
{
    const Module& curModule = m_placedb.m_modules[cellid];

    if( curModule.m_height > m_placedb.m_rowHeight )
    {
	fprintf(stderr, "Warning: module %d is a macro and should not be processed "
		"by AggressiveCellLegalLocationsSearch()\n", cellid );
    }

    int row_limit;
    if( m_row_limit < 1 )
    {
	row_limit = static_cast<int>(ceil(curModule.m_height/m_placedb.m_rowHeight)*m_row_factor);
    }
    else
    {
	row_limit = m_row_limit;	
    }

    double xbound = ceil( curModule.m_x - (m_left_factor*m_average_cell_width) );

    int upward_row_start, upward_row_end;
    int downward_row_start, downward_row_end;

    //Compute the cell located row index
    int cell_row_index = GetSiteIndex( curModule.m_y );

    if( 0 == cell_row_index )
    {
	upward_row_start = cell_row_index + 1;
	downward_row_start = cell_row_index;
    }
    else
    {
	upward_row_start = cell_row_index;
	downward_row_start = cell_row_index - 1;
    }

    const int max_site_index = static_cast<int>(m_free_sites.size())-1;
    upward_row_end = min( max_site_index, upward_row_start + row_limit );
    downward_row_end = max( 0, downward_row_start - row_limit );

    bool bContinue1;
    bool bContinue2;

    double min_shift = numeric_limits<double>::max();

    do
    {
	//Upward Search
	if( upward_row_start <= max_site_index )
	{
	    //Expand the search row end
	    upward_row_end = min( max_site_index, upward_row_start + row_limit );
	    vector<CLegalLocation> search_results;

	    GetCellLegalLocationsTowardLeft( cellid,
		    upward_row_start,
		    upward_row_end,
		    search_results,
		    xbound );

	    if( !search_results.empty() )
	    {
		min_shift = min( min_shift, ReturnMinimumShift( cellid, search_results ) );
		legalLocations.insert( legalLocations.end(), 
			search_results.begin(),
			search_results.end() );
	    }
	}

	//Downward Search
	if( downward_row_start >= 0 )
	{	
	    //Expand the search row end
	    downward_row_end = max( 0, downward_row_start - row_limit );
	    vector<CLegalLocation> search_results;

	    GetCellLegalLocationsTowardLeft( cellid,
		    downward_row_start,
		    downward_row_end,
		    search_results,
		    xbound );

	    if( !search_results.empty() )
	    {
		min_shift = min( min_shift, ReturnMinimumShift( cellid, search_results ) );
		legalLocations.insert( legalLocations.end(), 
			search_results.begin(),
			search_results.end() );
	    }
	}


	//Update the start row of next searching
	upward_row_start = upward_row_end + 1;
	downward_row_start = downward_row_end - 1;

	//Continue status 1: min_shift > max_search_range
	int max_search_row_number = max( upward_row_end - cell_row_index, 
		cell_row_index - downward_row_end );
	double max_search_range = 
	    static_cast<double>(max_search_row_number)*(m_placedb.m_rowHeight);
	bContinue1 = (min_shift > max_search_range);

	//Continue status 2: there are unsearched rows
	bContinue2 = (upward_row_start <= max_site_index) || (downward_row_start >= 0);

    }
    while(bContinue1 && bContinue2);

    if( legalLocations.empty() )
	return false;
    else
	return true;	
}

double CTetrisLegal::ReturnMinimumShift( const int& cellid,
	const vector<CLegalLocation>& locations )
{
    if( locations.empty() )
    {
	cerr << "Warning: 'locations' are empty in ReturnMinimumShift()" << endl;
	return -1;
    }

    double min_shift = numeric_limits<double>::max();
    CPoint p1( m_placedb.m_modules[cellid].m_x, m_placedb.m_modules[cellid].m_y );

    for( vector<CLegalLocation>::const_iterator iteLoc = locations.begin();
	    iteLoc != locations.end() ; iteLoc++ )
    {
	CPoint p2( iteLoc->m_xcoor, m_free_sites[iteLoc->m_site_index].m_bottom );
	min_shift = min( min_shift, Distance(p1,p2) );
    }

    return min_shift;
}

void CTetrisLegal::GetCellLegalLocationsTowardLeft( const int& cellid,
	int start_site_index, 
	int end_site_index,
	vector<CLegalLocation>& legalLocations,
	const double& left_bound )
{
    if( start_site_index > end_site_index )
	swap( start_site_index, end_site_index );

    if( start_site_index < 0 || 
	    end_site_index >= static_cast<int>(m_free_sites.size()) ) 
    {
	fprintf( stderr, "Illegal start_site_index and end_site_index in "
		"GetCellLegalLocationsTowardLeft() (%d,%d)\n", 
		start_site_index, end_site_index );
    }

    start_site_index = max(0, start_site_index);
    end_site_index = min( static_cast<int>(m_free_sites.size())-1, end_site_index );

    const Module& curModule = m_placedb.m_modules[cellid];

    if( curModule.m_height > m_placedb.m_rowHeight )
    {
	fprintf( stderr, "Module %d is a macro and should not be processed by "
		"GetCellLegalLocationsTowardLeft()\n", cellid );
    }

    legalLocations.clear();


    for( int iRow = start_site_index ; iRow <= end_site_index ; iRow++ )
    {
	const CSiteRow& curRow = m_free_sites[iRow];	

	for( unsigned int iInterval = 0 ; 
		iInterval < curRow.m_interval.size() ; 
		iInterval = iInterval + 2 )
	{
	    double xstart = curRow.m_interval[iInterval];
	    double xend = curRow.m_interval[iInterval+1];

	    //Discard illegal intervals
	    if( xend < left_bound )
		continue;

	    xstart = max( xstart, left_bound );
	    //Check if this interval has enough width
	    if( xend - xstart >= curModule.m_width )
	    {
		legalLocations.push_back( CLegalLocation( iRow, xstart ) );
		break;
	    }	
	}	
    }
}

bool Jin::LessXCoorMacroPrior::BL( const int& mid1, const int& mid2 )
{
    const Module& m1 = m_placedb->m_modules[mid1];
    const Module& m2 = m_placedb->m_modules[mid2];

    double cost1 = m1.m_cx + m1.m_cy - m1.m_width - m1.m_height;
    double cost2 = m2.m_cx + m2.m_cy - m2.m_width - m2.m_height;

    return cost1 < cost2;

}

bool Jin::LessXCoorMacroPrior::BR( const int& mid1, const int& mid2 )
{
    const Module& m1 = m_placedb->m_modules[mid1];
    const Module& m2 = m_placedb->m_modules[mid2];

    double cost1 = (-m1.m_cx) + m1.m_cy - m1.m_width - m1.m_height;
    double cost2 = (-m2.m_cx) + m2.m_cy - m2.m_width - m2.m_height;

    return cost1 < cost2;

}

bool Jin::LessXCoorMacroPrior::TL( const int& mid1, const int& mid2 )
{
    const Module& m1 = m_placedb->m_modules[mid1];
    const Module& m2 = m_placedb->m_modules[mid2];

    double cost1 = m1.m_cx + (-m1.m_cy) - m1.m_width - m1.m_height;
    double cost2 = m2.m_cx + (-m2.m_cy) - m2.m_width - m2.m_height;

    return cost1 < cost2;

}

bool Jin::LessXCoorMacroPrior::TR( const int& mid1, const int& mid2 )
{
    const Module& m1 = m_placedb->m_modules[mid1];
    const Module& m2 = m_placedb->m_modules[mid2];

    double cost1 = -(m1.m_cx + m1.m_cy) - m1.m_width - m1.m_height;
    double cost2 = -(m2.m_cx + m2.m_cy) - m2.m_width - m2.m_height;

    return cost1 < cost2;

}

bool Jin::LessXCoorMacroPrior::operator()( const int& mid1, const int& mid2 )
{
    const Module& m1 = m_placedb->m_modules[mid1];
    const Module& m2 = m_placedb->m_modules[mid2];

    double cost1 = m1.m_cx + m1.m_cy - m1.m_width - m1.m_height;
    double cost2 = m2.m_cx + m2.m_cy - m2.m_width - m2.m_height;

    return cost1 < cost2;

}

bool CTetrisLegal::MacroShifter( const double& macroFactor, const bool& makeFixed )
{
    //test code
    //if( param.bShow )
    //{
    //	m_placedb.m_modules_bak = m_placedb.m_modules;
    //	m_placedb.OutputGnuplotFigureWithZoom( "macro_shifter_before", false, true, true );
    //}
    //@test code

    const double thresholdHeight = macroFactor * m_placedb.m_rowHeight;

    for( unsigned int iMod = 0 ; iMod < m_placedb.m_modules.size() ; iMod++ )
    {
	const Module& curModule = m_placedb.m_modules[iMod];
	if( !curModule.m_isFixed && curModule.m_height >= thresholdHeight )
	{
	    m_macro_ids.push_back( iMod );
	}	
    }

    double min_shifting = numeric_limits<double>::max();

    bool (*function_array[4])( const int&, const int& );
    function_array[0] = LessXCoorMacroPrior::BL;
    function_array[1] = LessXCoorMacroPrior::BR;
    function_array[2] = LessXCoorMacroPrior::TL;
    function_array[3] = LessXCoorMacroPrior::TR;

    LessXCoorMacroPrior::m_placedb = &m_placedb;

    bool bSuccess = false;
    int bestType = -1;
    MacroShifterSaveOrigPosition();

    for( int iFunction = 0 ; iFunction < 4 ; iFunction++ )
    {
	vector<int> macro_order = m_macro_ids;
	//sort( macro_order.begin(), macro_order.end(), LessXCoorMacroPrior() );
	sort( macro_order.begin(), macro_order.end(), function_array[iFunction] );

	bool result = true;
	MacroShifterRestoreOrigPosition();
	RestoreFreeSite();
	double total_shifting = 0;

	for( unsigned int iOrder = 0 ; iOrder < macro_order.size() && result ; iOrder++ )
	{
	    const int& cellid = macro_order[iOrder];
	    vector<CLegalLocation> legalLocations;

	    if( true == AggressiveMacroDiamondSearch( cellid, legalLocations ) )
	    {
		int bestIndex = ReturnBestLocationIndex( cellid, legalLocations );
		const CLegalLocation& bestLocation = legalLocations[bestIndex];

		const Module& curModule = m_placedb.m_modules[cellid];
		CPoint p1( curModule.m_x, curModule.m_y );
		CPoint p2( bestLocation.m_xcoor, m_free_sites[bestLocation.m_site_index].m_bottom );

		total_shifting += Distance(p1,p2);

		const double bestx = bestLocation.m_xcoor;
		const double besty = m_free_sites[bestLocation.m_site_index].m_bottom;
		m_placedb.SetModuleLocation( cellid, bestx, besty );

		UpdateFreeSite( cellid );		

		//if( makeFixed )
		//{
		//	m_placedb.m_modules[cellid].m_isFixed = true;
		//}
	    }
	    else
	    {
		result = false;
	    }

	}

	//test code
	//if( result )
	//printf("%d Success\n", iFunction);
	//else
	//printf("%d Fail\n", iFunction);
	//@test code

	if( result && total_shifting < min_shifting )
	{
	    bestType = iFunction;
	    min_shifting = total_shifting;
	    MacroShifterSaveBestPosition();
	    bSuccess = true;	
	}

    }	

    if( bSuccess )
    {
	MacroShifterRestoreBestPosition();

	if( 0 == bestType )
	{
	    printf("Best direction is BL\n");
	}
	else if( 1 == bestType )
	{
	    printf("Best direction is BR\n");
	}
	else if( 2 == bestType )
	{
	    printf("Best direction is TL\n");
	}
	else if( 3 == bestType )
	{
	    printf("Best direction is TR\n");
	}
	else
	{
	    fprintf(stderr, "Warning: unknown bestType %d\n", bestType );
	}

	//Fix macros if 'makeFixed' is true
	if( makeFixed )
	{
	    for( unsigned int i = 0 ; i < m_macro_ids.size() ; i++ )
	    {
		const int moduleIndex = m_macro_ids[i];
		m_placedb.m_modules[moduleIndex].m_isFixed;
	    }
	}
    }
    else
    {
	MacroShifterRestoreOrigPosition();
    }

    //test code
    //if( param.bShow )
    //{
    //	m_placedb.OutputGnuplotFigureWithZoom( "macro_shifter", true, true, true );
    //}
    //@test code

    return bSuccess;
}

bool CTetrisLegal::AggressiveMacroDiamondSearch( const int& cellid,
	vector<CLegalLocation>& legalLocations )
{
    const Module& curModule = m_placedb.m_modules[cellid];

    if( curModule.m_height <= m_placedb.m_rowHeight )
    {
	fprintf( stderr, "Module %d is a standard cell and should not be processed by "
		"AggressiveMacroDiamondSearch()\n", cellid );
    }

    double max_horizontal_dist = max( 
	    fabs(curModule.m_x - m_placedb.m_coreRgn.left),
	    fabs(curModule.m_x - m_placedb.m_coreRgn.right) );
    double max_vertical_dist = max(
	    fabs(curModule.m_y - m_placedb.m_coreRgn.bottom ),
	    fabs(curModule.m_y - m_placedb.m_coreRgn.top ) );

   
    legalLocations.clear();

    double radius_step = max( m_max_module_height, m_max_module_width );	
    double radius = ( curModule.m_width + curModule.m_height ) * 2.0 / 3.0; 
    const CPoint module_center = CPoint( curModule.m_cx, curModule.m_cy );

    //Diamond search stop when all the chip region is covered
    double max_radius = max_horizontal_dist + max_vertical_dist + radius_step;

    //Search continues until at least one legal location is found or 
    //no legal locations can be found
    while( radius < max_radius && legalLocations.empty() )
    {
	vector<CSiteRow> sites;
	GetDiamondSiteRows( module_center,
		radius, sites );
	GetMacroLegalLocationsTowardOrig( cellid,
		sites, legalLocations );

	radius += radius_step;
    }

    //No legal locations are found
    if( legalLocations.empty() )
    {
	//test code
	if( param.bShow )
	    printf("Final try radius %.2f of module %d height %.2f width %.2f (%.2f,%.2f)\n",
		    radius, cellid, curModule.m_height, curModule.m_width,
		    curModule.m_x, curModule.m_y );
	//@test code
	return false;
    }
    else
    {
	return true;
    }
}


void CTetrisLegal::UpdateFreeSite( const int& cellid )
{
    const Module& curModule = m_placedb.m_modules[cellid];
    int start_row_index = GetSiteIndex( curModule.m_y );

    if( curModule.m_height <= m_placedb.m_rowHeight )
    {
	UpdateFreeSite( start_row_index, 
		curModule.m_x, 
		curModule.m_width ); 
    }
    else
    {
	//Update free site for a macro
	int needed_row_count = static_cast<int>( ceil(curModule.m_height/m_site_height) );
	for( int iRow = start_row_index; 
		iRow < start_row_index + needed_row_count ;
		iRow++ )
	{
	    UpdateFreeSite( iRow, curModule.m_x, curModule.m_width );
	}
    }

}

int CTetrisLegal::ReturnBestLocationIndex( const int& cellid, 
	std::vector<Jin::CLegalLocation>& legalLocations )
{
    if( legalLocations.empty() )
    {
	cerr << "Warning: legalLocations are empty, and thus -1 is returned" << endl;
	return -1;
    }
    else if( 1 == legalLocations.size() )
    {
	return 0;
    }

    //Set up the cost for each legalLocations
    const Module& curModule = m_placedb.m_modules[cellid];

    CPoint p1(curModule.m_x,curModule.m_y);

    for( unsigned int i = 0 ; i < legalLocations.size() ; i++ )
    {
	CLegalLocation& curLoc = legalLocations[i];
	double locx = curLoc.m_xcoor;
	double locy = m_free_sites[ curLoc.m_site_index ].m_bottom;

	CPoint p2( locx, locy );

	curLoc.m_shift = Distance(p1, p2);
    }

    double min_shift = legalLocations.front().m_shift;
    list<int> min_shift_indexes(1,0);

    for( unsigned int iLoc = 1 ; iLoc < legalLocations.size() ; iLoc++ )
    {
	CLegalLocation& curLoc = legalLocations[iLoc];
	if( curLoc.m_shift < min_shift )
	{
	    min_shift = curLoc.m_shift;
	    min_shift_indexes.resize(1);
	    min_shift_indexes.front() = iLoc;
	}
	else if( curLoc.m_shift == min_shift )
	{
	    min_shift_indexes.push_back( iLoc );
	}	
    }

    //Only one location has the minimum shift
    //Just return the location	
    if( 1 == min_shift_indexes.size() )
    {
	return min_shift_indexes.front();
    }
    //More than one locations have minimum shift
    //Return the location with minimum wirelength
    else if( 1 < min_shift_indexes.size() )
    {
	//Set up wirelength
	for( list<int>::iterator iteLocIndex = min_shift_indexes.begin() ;
		iteLocIndex != min_shift_indexes.end() ; iteLocIndex++ )
	{
	    CLegalLocation& curLoc = legalLocations[*iteLocIndex];

	    double locx = curLoc.m_xcoor;
	    double locy = m_free_sites[ curLoc.m_site_index ].m_bottom;

	    m_placedb.SetModuleLocation( cellid, locx, locy );
	    curLoc.m_wirelength = m_placedb.GetModuleTotalNetLength( cellid );
	}

	double min_wirelength = numeric_limits<double>::max();
	int min_wirelength_location_index = -1;

	for( list<int>::iterator iteLocIndex = min_shift_indexes.begin() ;
		iteLocIndex != min_shift_indexes.end() ; iteLocIndex++ )
	{
	    CLegalLocation& curLoc = legalLocations[*iteLocIndex];

	    if( curLoc.m_wirelength < min_wirelength )
	    {
		min_wirelength_location_index = *iteLocIndex;
		min_wirelength = curLoc.m_wirelength;
	    }
	}

	if( min_wirelength_location_index < 0 )
	{
	    cerr << "Warning: illegal min_wirelength_location_index " 
		<< min_wirelength_location_index << ", and thus reutrn 0" << endl;
	    return 0;
	}
	else
	{
	    return min_wirelength_location_index;
	}

    }
    else
    {
	cerr << "Warning: incorrect min_shift_indexes size " << min_shift_indexes.size() << 
	    ", and thus return 0" << endl;
	return 0;
    }
}


void CTetrisLegal::GetMacroLegalLocationsTowardOrig( 
	const int& cellid,
	const std::vector<CSiteRow>& sites,
	std::vector<Jin::CLegalLocation>& legalLocations )
{
#if 0
    //test code
    FILE* ofile = fopen( "getmacrolegallocation.log", "w" );
    for( unsigned int i = 0 ; i < sites.size() ; i++ )
    {
        const CSiteRow& curRow = sites[i];
        
        fprintf(ofile,"bottom: %.2f ", curRow.m_bottom );
        for( unsigned int j = 0 ; j < curRow.m_interval.size() ; j=j+2 )
        {
    	fprintf(ofile, "(%.2f,%.2f) ", curRow.m_interval[j], curRow.m_interval[j+1] );
	}
        fprintf(ofile, "\n");
    }
    fclose( ofile );
    //@test code
#endif

    const Module& curModule = m_placedb.m_modules[cellid];

    if( curModule.m_height <= m_placedb.m_rowHeight )
    {
	fprintf(stderr, "Module %d is a standard cell and should not been processed "
		"by GetMacroLegalLocationsTowardLeft()\n", cellid );

    }

    const double module_width = curModule.m_width;

    //Number of rows needed for this macro
    const int module_row_number = 
	static_cast<int>(ceil(curModule.m_height/m_placedb.m_rowHeight));

    //'terminal_count_array' keeps the terminal count used by scanline algorithm.
    //terminal_count_array[i] is associated with row [sites[i], site[i+module_row_number])	
    int terminal_count_array_size = sites.size() - module_row_number + 1;

    if( terminal_count_array_size <= 0 )
    {
	cerr << "Warning: input sites has no enough height to put target macro" << endl;
	return;
    }

    int* terminal_count_array = new int[terminal_count_array_size];
    for( int i = 0 ; i < terminal_count_array_size ; i++ )	
    {
	terminal_count_array[i] = 0;
    }

    //'rightmost_left_terminals' keeps the x coordinate of the rightmost left terminal of each row
    //rightmost_left_terminals[i] recordes the x coordinate of the rightmost left terminal with 
    //row [ sites[i], site[i+module_row_number] )
    double* rightmost_left_terminals = new double[terminal_count_array_size];

    //Push all terminals into an array and sort it
    vector<CTerminal> terminals;
    int precalculate_terminal_number = 0;
    for( unsigned int i = 0 ; i < sites.size() ; i++ )
    {
	precalculate_terminal_number += sites[i].m_interval.size();
    }

    terminals.reserve( precalculate_terminal_number );
    for( unsigned int iRow = 0 ; iRow < sites.size() ; iRow++ )
    {
	const CSiteRow& curRow = sites[iRow];
	for( unsigned int i = 0 ; i < curRow.m_interval.size() ; i=i+2 )
	{
	    terminals.push_back( 
		    CTerminal( curRow.m_interval[i], CTerminal::Left, iRow ) );
	    terminals.push_back(
		    CTerminal( curRow.m_interval[i+1], CTerminal::Right, iRow ) );
	}		
    }	

    //Sort terminals by their x coordinates
    sort( terminals.begin(), terminals.end(), LessXCoor() );

    //Scanline algorithm
    for( unsigned int iTerminal = 0 ; iTerminal < terminals.size() ; iTerminal++ )
    {
	const CTerminal& curTerminal = terminals[iTerminal];
	const int associated_upper_row = 
	    min( terminal_count_array_size-1, curTerminal.m_row  );
	const int associated_lower_row = 
	    max( 0, curTerminal.m_row - (module_row_number-1));

	//Increase the terminal count for associated arrays
	if( CTerminal::Left == curTerminal.m_type )
	{
	    for( int iRow = associated_lower_row ; iRow <= associated_upper_row ; iRow++ )
	    {
		terminal_count_array[iRow]++;
		rightmost_left_terminals[iRow] = curTerminal.m_xcoor;
	    }
	}
	//Check if there is free spaces for this macro
	else
	{
	    for( int iRow = associated_lower_row ; iRow <= associated_upper_row ; iRow++ )
	    {
		if( module_row_number == terminal_count_array[iRow] )
		{
		    double xleft = rightmost_left_terminals[iRow];
		    double xright = curTerminal.m_xcoor;

		    double width = xright - xleft;

		    //This interval is enough for this macro
		    if( width >= module_width )
		    {
			//Compute the feasible x interval
			double feasible_xleft = xleft;
			double feasible_xright = xright - module_width;

			double xbest;

			//Feasible interval is at left of curModule
			//The best location is the right boundary of feasible interval
			if( feasible_xright <= curModule.m_x )
			{
			    xbest = feasible_xright;
			}
			//Feasible interval is at right of curModule
			//The best location is the left boundary of feasible interval
			else if( feasible_xleft >= curModule.m_x )
			{
			    xbest = feasible_xleft;
			}
			//Feasible interval contains the original location of curModule
			//The best location is the orignal x coordinate of curModule
			else
			{
			    xbest = curModule.m_x;
			}

			legalLocations.push_back( 
				CLegalLocation( iRow, xbest ) );
		    }	

		}

		terminal_count_array[iRow]--;
	    }
	}

    }


    delete []terminal_count_array;
    delete []rightmost_left_terminals;
    terminal_count_array = NULL;
    rightmost_left_terminals = NULL;

}

void CTetrisLegal::GetMacroLegalLocationsTowardLeft( 
	const int& cellid,
	const std::vector<CSiteRow>& sites,
	std::vector<Jin::CLegalLocation>& legalLocations,
	const double& left_bound )
{
    const Module& curModule = m_placedb.m_modules[cellid];

    if( curModule.m_height <= m_placedb.m_rowHeight )
    {
	fprintf(stderr, "Module %d is a standard cell and should not been processed "
		"by GetMacroLegalLocationsTowardLeft()\n", cellid );

    }

    const double module_width = curModule.m_width;

    //Number of rows needed for this macro
    const int module_row_number = 
	static_cast<int>(ceil(curModule.m_height/m_placedb.m_rowHeight));

    //'terminal_count_array' keeps the terminal count used by scanline algorithm.
    //terminal_count_array[i] is associated with row [sites[i], site[i+module_row_number])	
    int terminal_count_array_size = sites.size() - module_row_number + 1;

    if( terminal_count_array_size <= 0 )
    {
	cerr << "Warning: input sites has no enough height to put target macro" << endl;
	return;
    }

    int* terminal_count_array = new int[terminal_count_array_size];
    for( int i = 0 ; i < terminal_count_array_size ; i++ )	
    {
	terminal_count_array[i] = 0;
    }

    //'rightmost_left_terminals' keeps the x coordinate of the rightmost left terminal of each row
    //rightmost_left_terminals[i] recordes the x coordinate of the rightmost left terminal with 
    //row [ sites[i], site[i+module_row_number] )
    double* rightmost_left_terminals = new double[terminal_count_array_size];

    //Push all terminals into an array and sort it
    vector<CTerminal> terminals;
    int precalculate_terminal_number = 0;
    for( unsigned int i = 0 ; i < sites.size() ; i++ )
    {
	precalculate_terminal_number += sites[i].m_interval.size();
    }

    terminals.reserve( precalculate_terminal_number );
    for( unsigned int iRow = 0 ; iRow < sites.size() ; iRow++ )
    {
	const CSiteRow& curRow = sites[iRow];
	for( unsigned int i = 0 ; i < curRow.m_interval.size() ; i=i+2 )
	{
	    terminals.push_back( 
		    CTerminal( curRow.m_interval[i], CTerminal::Left, iRow ) );
	    terminals.push_back(
		    CTerminal( curRow.m_interval[i+1], CTerminal::Right, iRow ) );
	}		
    }	

    //Sort terminals by their x coordinates
    sort( terminals.begin(), terminals.end(), LessXCoor() );

    //Add a stop condition when at least one point is found for each row
    //Also, one row can be skip if one point is found
    bool* bFoundLocation = new bool[terminal_count_array_size];
    for( int i = 0 ; i < terminal_count_array_size ; i++ )
    {
	bFoundLocation[i] = false;
    }
    int numFoundLocation = 0;

    //Scanline algorithm
    for( unsigned int iTerminal = 0 ; 
	    iTerminal < terminals.size() && numFoundLocation < terminal_count_array_size ; 
	    iTerminal++ )
    {
	const CTerminal& curTerminal = terminals[iTerminal];
	const int associated_upper_row = 
	    min( terminal_count_array_size-1, curTerminal.m_row  );
	const int associated_lower_row = 
	    max( 0, curTerminal.m_row - (module_row_number-1));

	//Increase the terminal count for associated arrays
	if( CTerminal::Left == curTerminal.m_type )
	{
	    for( int iRow = associated_lower_row ; iRow <= associated_upper_row ; iRow++ )
	    {
		terminal_count_array[iRow]++;
		rightmost_left_terminals[iRow] = curTerminal.m_xcoor;
	    }
	}
	//Check if there is free spaces for this macro
	else
	{
	    for( int iRow = associated_lower_row ; iRow <= associated_upper_row ; iRow++ )
	    {
		//If one location has been found for this row,
		//skip the check of this row
		if( true == bFoundLocation[iRow] )
		    continue;

		if( module_row_number == terminal_count_array[iRow] )
		{
		    double xleft = max( left_bound, rightmost_left_terminals[iRow] );
		    double xright = curTerminal.m_xcoor;

		    //May be negative
		    double width = xright - xleft;

		    if( width >= module_width )
		    {
			legalLocations.push_back( 
				CLegalLocation( iRow, xleft ) );

			//To skip unnecessary checkings
			numFoundLocation++;
			bFoundLocation[iRow] = true;
		    }	

		}

		terminal_count_array[iRow]--;
	    }
	}

    }


    delete []terminal_count_array;
    delete []rightmost_left_terminals;
    delete []bFoundLocation;
    terminal_count_array = NULL;
    rightmost_left_terminals = NULL;
    bFoundLocation = NULL;

}

bool Jin::LessXCoor::operator()( const int& mid1, const int& mid2 )
{
    const Module& m1 = m_placedb->m_modules[mid1];
    const Module& m2 = m_placedb->m_modules[mid2];

    double cost1 = m1.m_x;
    double cost2 = m2.m_x;

    //Macro have higher priority than cells
    if( m1.m_height > m_placedb->m_rowHeight )
    {
	cost1 = cost1 - m_macro_factor;
    }

    if( m2.m_height > m_placedb->m_rowHeight )
    {
	cost2 = cost2 - m_macro_factor;
    }

    if( cost1 == cost2 )
    {
	if( m1.m_width == m2.m_width )
	    return m1.m_height < m2.m_height;
	else
	    return m1.m_width < m2.m_width;
    }
    else
	return cost1 < cost2;
}


bool Jin::LessXCoorMacroFirst::operator()( const int& mid1, const int& mid2 )
{
    const Module& m1 = m_placedb->m_modules[mid1];
    const Module& m2 = m_placedb->m_modules[mid2];

    const bool bMacro1 = (m1.m_height > m_placedb->m_rowHeight) ? true : false;
    const bool bMacro2 = (m2.m_height > m_placedb->m_rowHeight) ? true : false;

    //Both modules are macros or cells,
    //sort them according to their x coordinates
    if( bMacro1 == bMacro2 )
    {

	if( m1.m_x == m2.m_x )
	{
	    if( m1.m_width == m2.m_width )
		return m1.m_height < m2.m_height;
	    else
		return m1.m_width > m2.m_width;
	}
	else
	    return m1.m_x < m2.m_x;
    }
    //One of these two modules are macros,
    //the macro module has higher priority
    else if( bMacro1 || bMacro2 )
    {
	if( bMacro2 )
	    return false;
	else
	    return true;
    }
    //No module is macro
    else
    {
	if( m1.m_x == m2.m_x )
	{
	    if( m1.m_width == m2.m_width )
		return m1.m_height < m2.m_height;
	    else
		return m1.m_width < m2.m_width;
	}
	else
	{
	    return m1.m_x < m2.m_x;
	}

    }
}

void CTetrisLegal::GetDiamondSiteRows( const CPoint& center,               //center of diamond
	const double& radius,               //radius of diamond
	std::vector<CSiteRow>& sites )     //put the resulting diamond sites into "sites"
{
    //Clear the returning result
    sites.clear();

    double diamond_bottom = center.y - radius;
    double diamond_top = center.y + radius;
    //double diamond_left = center.x - radius;
    //double diamond_right = center.x + radius;

    double free_site_bottom = m_free_sites.front().m_bottom;
    double free_site_height = m_free_sites.back().m_height;

    double free_site_top = m_free_sites.back().m_bottom + free_site_height;

    if( diamond_bottom > free_site_top || diamond_top < free_site_bottom )
    {
	cerr << "Warning: desired diamond region has no overlapping with free sites" << endl;
	return;
    }

    //Find the range of the diamond region in the free sites
    int bottom_site_index = max( 0, 
	    static_cast<int>( ceil( (diamond_bottom - free_site_bottom)/free_site_height)) );
    int top_site_index = min( static_cast<int>(m_free_sites.size()) - 1, 
	    static_cast<int>(floor( (diamond_top-free_site_bottom)/free_site_height ) ) - 1 );

    sites.reserve( top_site_index - bottom_site_index + 1 );

    //For each free site row in the range, add corresponding interval
    //into resulting sites
    for( int iRow = bottom_site_index ; iRow <= top_site_index ; iRow++ )
    {
	const CSiteRow& curRow = m_free_sites[iRow];

	double row_left, row_right;

	//Find the diamond left and right boundary in this row
	//curRow is at lower half of diamond
	if( curRow.m_height < center.y )
	{
	    row_left = floor( center.x - ( curRow.m_height - diamond_bottom ) );
	    row_right = ceil( center.x + ( curRow.m_height - diamond_bottom ) );	
	}
	//curRow is at higher half of diamond
	else
	{
	    row_left = floor( center.x - ( diamond_top - curRow.m_height ) );
	    row_right = ceil( center.x + ( diamond_top - curRow.m_height ) );
	}

	//Push curRow into resulting sites
	sites.push_back( CSiteRow(curRow.m_bottom, curRow.m_height, curRow.m_step) );
	//sites.back().m_bottom = curRow.m_bottom;
	//sites.back().m_height = curRow.m_height;
	//sites.back().m_step = curRow.m_step;

	sites.back().m_interval.reserve( curRow.m_interval.size() );

	//Push intervals which have overlapping with row_left and row_right
	for( unsigned int i = 0 ; i < curRow.m_interval.size() ; i = i+2 )
	{
	    double left = curRow.m_interval[i];
	    double right = curRow.m_interval[i+1];

	    if( left >= row_right || right <= row_left )
		continue;

	    sites.back().m_interval.push_back( max( left, row_left ) );
	    sites.back().m_interval.push_back( min( right, row_right ) );
	}
    }	
}

//Infeasible due the change of interface
#if 0
bool CTetrisLegal::DoVerticalLine(const double& prelegal_factor,
	const double& vertical_line_xcoor)
{
    bool bLegalLeft = false;
    bool bLegalRight = false;

    bLegalLeft = DoLeftWithVerticalLine( prelegal_factor, vertical_line_xcoor );

    if( bLegalLeft )
	bLegalRight = DoRightWithVerticalLine( prelegal_factor, vertical_line_xcoor );

    return bLegalLeft & bLegalRight;
}


bool CTetrisLegal::DoRightWithVerticalLine(const double& prelegal_factor,
	const double& vertical_line_xcoor)
{
    //ReversePlacement();
    //SetReverseSite();

    bool bLegal = DoLeftWithVerticalLine( prelegal_factor, -vertical_line_xcoor );

    //ReversePlacement();

    return bLegal;
}

bool CTetrisLegal::DoLeftWithVerticalLine(const double& prelegal_factor,
	const double& vertical_line_xcoor)
{
    CalculateNewLocation(prelegal_factor, vertical_line_xcoor);

    CalculateCellOrder(vertical_line_xcoor);
    RemoveFreeSite(vertical_line_xcoor);
    bool bLegal = LegalizeByCellOrder();

    return bLegal;
}

void CTetrisLegal::RemoveFreeSite( const double& vertical_line_xcoor )
{
    for( vector<CSiteRow>::iterator iteRow = m_free_sites.begin() ;
	    iteRow != m_free_sites.end() ; iteRow++ )
    {
	while(true)
	{
	    if( iteRow->m_interval.empty() )
		break;

	    double left = iteRow->m_interval[0];
	    double right = iteRow->m_interval[1];

	    if( left >= vertical_line_xcoor )
		break;
	    else if( right <= vertical_line_xcoor )
	    {
		iteRow->m_interval.erase( iteRow->m_interval.begin() );
		iteRow->m_interval.erase( iteRow->m_interval.begin() );
	    }
	    //left < vertical_line_xcoor and right > vertical_line_xcoor
	    else
	    {
		iteRow->m_interval[0] = vertical_line_xcoor;
		break;
	    }
	}	
    }	
}

void CTetrisLegal::CalculateCellOrder(const double& vertical_line_xcoor)
{
    m_cell_order.clear();
    m_cell_order.reserve( m_placedb.m_modules.size() );

    for( unsigned int i = 0 ; i < m_placedb.m_modules.size() ; i++ )
    {
	if( !m_placedb.m_modules[i].m_isFixed && 
		m_placedb.m_modules[i].m_cx >= vertical_line_xcoor)
	    m_cell_order.push_back(i);
    }

    LessXCoor::m_placedb = &m_placedb;

    sort( m_cell_order.begin(), m_cell_order.end(), LessXCoor() );	
}

void CTetrisLegal::CalculateNewLocation(const double& prelegal_factor,
	const double& vertical_line_xcoor)
{
    for( unsigned int i = 0 ; i < m_process_list.size() ; i++ )
    {
	Module& curModule = m_placedb.m_modules[ m_process_list[i] ];
	
	if( curModule.m_isFixed )
	{
	    fprintf(stderr, "module %d should not be processed by CalculateNewLocation()\n", m_process_list[i] );
	    continue;
	}

	//Only modules in the left side of the given vertical line is moved	
	if( curModule.m_cx >= vertical_line_xcoor )
	{
	    double newCX = (curModule.m_cx - vertical_line_xcoor)*prelegal_factor 
		+ vertical_line_xcoor;
	    double newCY = curModule.m_cy;

	    //Rounding for macros
	    if( curModule.m_height > m_placedb.m_rowHeight )
		newCX = Rounding(newCX);

	    m_placedb.MoveModuleCenter( m_process_list[i], newCX, newCY );
	}
    }
}
#endif

void CTetrisLegal::PrepareNonMacroLeftRightFreeSites(const vector<int>& macro_ids)
{
    m_free_sites = m_placedb.m_sites;
    m_right_free_sites.clear();
    
    for( unsigned int i = 0 ; i < macro_ids.size() ; i++ ) 
    {
	UpdateFreeSite( macro_ids[i] );
    }
    
    m_left_free_sites = m_free_sites;
    
    //Reverse m_free_sites
    for( unsigned int iRow = 0 ; iRow < m_left_free_sites.size() ; iRow++ )
    {
	CSiteRow& sourceRow = m_left_free_sites[iRow];
	//CSiteRow& curRow = m_free_sites[iRow];

	m_right_free_sites.push_back( CSiteRow( sourceRow.m_bottom, sourceRow.m_height, sourceRow.m_step ) );
	
	vector<double>& reverse_interval = m_right_free_sites.back().m_interval;
	reverse_interval.reserve( sourceRow.m_interval.size() );
	for( vector<double>::reverse_iterator iteInterval = sourceRow.m_interval.rbegin() ;
		iteInterval != sourceRow.m_interval.rend() ; iteInterval++ )
	{
	    //curRow.m_interval.push_back( -(*iteInterval) );
	    reverse_interval.push_back( -(*iteInterval) );	
	}	
    }


}

void CTetrisLegal::PrepareLeftRightFreeSites(void)
{
    m_left_free_sites = m_placedb.m_sites;
    m_right_free_sites.clear();
    
    //Reverse m_free_sites
    for( unsigned int iRow = 0 ; iRow < m_left_free_sites.size() ; iRow++ )
    {
	CSiteRow& sourceRow = m_left_free_sites[iRow];
	//CSiteRow& curRow = m_free_sites[iRow];

	m_right_free_sites.push_back( CSiteRow( sourceRow.m_bottom, sourceRow.m_height, sourceRow.m_step ) );
	
	vector<double>& reverse_interval = m_right_free_sites.back().m_interval;
	reverse_interval.reserve( sourceRow.m_interval.size() );
	for( vector<double>::reverse_iterator iteInterval = sourceRow.m_interval.rbegin() ;
		iteInterval != sourceRow.m_interval.rend() ; iteInterval++ )
	{
	    //curRow.m_interval.push_back( -(*iteInterval) );
	    reverse_interval.push_back( -(*iteInterval) );	
	}	
    }


}


void CTetrisLegal::ReverseLegalizationData(void)
{
    //Reverse the chip core region
    double new_left = -(m_placedb.m_coreRgn.right);
    double new_right = -(m_placedb.m_coreRgn.left);
    m_placedb.m_coreRgn.left = new_left;
    m_placedb.m_coreRgn.right = new_right;

    //Reverse module pin offsets
    for( unsigned int iPin = 0 ; iPin < m_placedb.m_pins.size() ; iPin++ )
    {
	m_placedb.m_pins[iPin].xOff = -m_placedb.m_pins[iPin].xOff;
    }

}

void CTetrisLegal::ReversePlacement(void)
{
    //Reverse module centers
    for( unsigned int iModule = 0 ; iModule < m_placedb.m_modules.size() ; iModule++ )
    {
	const Module& curModule = m_placedb.m_modules[iModule];
	double new_locx = (-curModule.m_cx) - (curModule.m_width/2.0);
	m_placedb.SetModuleLocation( iModule, new_locx, curModule.m_y );
    }

}

//void CTetrisLegal::SetReverseSite(void)
//{
//	for( unsigned int iRow = 0 ; iRow < m_placedb.m_sites.size() ; iRow++ )
//	{
//		const CSiteRow& sourceRow = m_placedb.m_sites[iRow];
//		CSiteRow& curRow = m_free_sites[iRow];
//
//		curRow.m_interval.clear();
//		curRow.m_interval.reserve( sourceRow.m_interval.size() );
//
//		for( vector<double>::const_reverse_iterator iteInterval = sourceRow.m_interval.rbegin() ;
//				iteInterval != sourceRow.m_interval.rend() ; iteInterval++ )
//		{
//			curRow.m_interval.push_back( -(*iteInterval) );
//		}	
//	}
//}

bool CTetrisLegal::DoRight(const double& prelegal_factor)
{
    //ReversePlacement();
    //SetReverseSite();
    //Change m_chip_left_bound
    m_chip_left_bound = m_placedb.m_coreRgn.left;

    bool bLegal = DoLeft( prelegal_factor );

    //ReversePlacement();
    //restore m_chip_left_bound
    m_chip_left_bound = m_placedb.m_coreRgn.left;

    return bLegal;
}

void CTetrisLegal::RestoreGlobalResult(void)
{
    for( unsigned int iModule = 0 ; iModule < m_placedb.m_modules.size() ; iModule++ )
    {
	m_placedb.SetModuleLocation( iModule,
		m_globalLocations[iModule].x, 
		m_globalLocations[iModule].y);
    }
}


void CTetrisLegal::RestoreOrig(void)
{
    for( unsigned int iModule = 0 ; iModule < m_placedb.m_modules.size() ; iModule++ )
    {
	m_placedb.SetModuleLocation( iModule,
		m_origLocations[iModule].x, 
		m_origLocations[iModule].y);
    }
}

void CTetrisLegal::RestoreBest(void)
{
    for( unsigned int iModule = 0 ; iModule < m_placedb.m_modules.size() ; iModule++ )
    {
	m_placedb.SetModuleLocation( iModule,
		m_bestLocations[iModule].x, 
		m_bestLocations[iModule].y);
    }
}

void CTetrisLegal::SaveGlobalResult(void)
{
    for( unsigned int iModule = 0 ; iModule < m_placedb.m_modules.size() ; iModule++ )
    {
	const Module& curModule = m_placedb.m_modules[iModule];
	m_globalLocations[iModule] = CPoint( curModule.m_x, curModule.m_y );	
    }
}

void CTetrisLegal::SaveOrig(void)
{
    for( unsigned int iModule = 0 ; iModule < m_placedb.m_modules.size() ; iModule++ )
    {
	const Module& curModule = m_placedb.m_modules[iModule];
	m_origLocations[iModule] = CPoint( curModule.m_x, curModule.m_y );	
    }
}

void CTetrisLegal::SaveBest( const double& best_prelegal_factor, const double& best_cost )
{
    m_best_prelegal_factor = best_prelegal_factor;
    //m_best_cost = m_placedb.CalcHPWL();
    m_best_cost = best_cost;

    for( unsigned int iModule = 0 ; iModule < m_placedb.m_modules.size() ; iModule++ )
    {
	const Module& curModule = m_placedb.m_modules[iModule];
	m_bestLocations[iModule] = CPoint( curModule.m_x, curModule.m_y );	
    }
}


int CTetrisLegal::GetSiteIndex( const double& ycoor )
{
    if( ycoor >= m_free_sites.back().m_bottom )
	return m_free_sites.size() - 1;
    else if( ycoor < m_site_bottom + m_site_height )
	return 0;
    else
    {
	return static_cast<int>(floor( (ycoor-m_site_bottom)/m_site_height));
    }
}

CTetrisLegal::CTetrisLegal( CPlaceDB& placedb ) :
    m_placedb( placedb ),
    m_width_factor(1.0),
    m_left_factor(1.0),
    m_row_limit(10),
    m_unlegal_count(0),
    m_prelegal_factor(1.00),
    m_best_prelegal_factor( m_prelegal_factor ),
    m_best_cost( numeric_limits<double>::max() ),
    m_row_factor(10.0),
    m_bMacroLegalized(false)
{
    //Compute average cell width
    int cell_count = 0;
    double total_width = 0;
    //double max_height = 0.0;
    m_max_module_height = 0.0;
    m_max_module_width = 0.0;
    for( unsigned int i = 0 ; i < m_placedb.m_modules.size() ; i++ )
    {
	const Module& curModule = m_placedb.m_modules[i];

	m_max_module_height = max( m_max_module_height, curModule.m_height );
	m_max_module_width = max( m_max_module_width, curModule.m_width );
	//Do not include fixed cells and macros
	if( curModule.m_isFixed || curModule.m_height > m_placedb.m_rowHeight )
	    continue;

	cell_count++;
	total_width += curModule.m_width;
    }

    double search_range_factor = 0.05;
    //ifstream parFile( "parameter" );
    //if( parFile )
    //{
    //	parFile >> search_range_factor;
    //}
    //parFile.close();

    m_row_limit = max( m_row_limit, 
	    static_cast<int>(ceil(m_max_module_height/m_placedb.m_rowHeight)*search_range_factor));
    //test code
    if( param.bShow )
	printf("\nsearch_range_factor: %.2f m_row_limit: %d\n", search_range_factor, m_row_limit );
    //@test code
    //m_row_limit = max( m_row_limit, 
    //		static_cast<int>(ceil(m_max_module_height/m_placedb.m_rowHeight)*0.1));
    //m_row_limit = max( m_row_limit, 
    //		static_cast<int>(ceil(m_max_module_height/m_placedb.m_rowHeight)*0.25));

    //cout << "Search row limit: " << m_row_limit << endl;
    m_average_cell_width = total_width / cell_count;
    m_macro_factor = m_average_cell_width * 5.0;
    LessXCoor::m_macro_factor = m_macro_factor;

    m_free_sites = m_placedb.m_sites;
    m_site_bottom = m_free_sites.front().m_bottom;
    m_site_height = m_free_sites.front().m_height;

    m_placedb.m_modules_bak = m_placedb.m_modules;

    //initalize m_origLocations and m_bestLocations
    m_origLocations.resize( m_placedb.m_modules.size() );
    m_bestLocations.resize( m_placedb.m_modules.size() );
    m_globalLocations.resize( m_placedb.m_modules.size() );
    
    m_chip_left_bound = m_placedb.m_coreRgn.left;

}


bool CTetrisLegal::Solve( const double& util,
	const bool& bMacroLegal,
	const bool& bRobust,
	const double& stop_prelegal_factor	)
{
    //test code
    //FILE* ofile = fopen( "site.log", "w" );
    //for( unsigned int i = 0 ; i < m_placedb.m_sites.size() ; i++ )
    //{
    //    const CSiteRow& curRow = m_placedb.m_sites[i];
    //    
    //    fprintf(ofile,"bottom: %.2f ", curRow.m_bottom );
    //    for( unsigned int j = 0 ; j < curRow.m_interval.size() ; j=j+2 )
    //    {
    //	fprintf(ofile, "(%.2f,%.2f) ", curRow.m_interval[j], curRow.m_interval[j+1] );
    //    }
    //    fprintf(ofile, "\n");
    //}
    //
    //fclose( ofile );
    //@test code

    m_bMacroLegalized = bMacroLegal;

    SaveGlobalResult();
    PrepareLeftRightFreeSites();
    
#if 0 
    //test code: test for legalization from a vertical line
    //Find the column with highest density
    int BinHNum = pBin->GetBinNumberH();
    int BinWNum = pBin->GetBinNumberW();
    vector<double> column_density;
    column_density.resize( BinWNum, 0.0 );
    for( int iH = 0 ; iH < BinHNum ; iH++ )
    {
	for( int iW = 0 ; iW < BinWNum ; iW++ )
	{
	    column_density[iW] += pBin->m_binUsage[iW][iH];
	}
    }

    int maxW = 0;
    double max_density = column_density[0];

    for( int iW = 1 ; iW < BinWNum ; iW++ )
    {
	if( column_density[iW] > max_density )
	{
	    maxW = iW;
	    max_density = column_density[iW];
	}
    }

    double mid_line_xcoor = pBin->GetBinX(maxW) + (pBin->GetBinWidth()/2.0);
    cout << "The coordinate of vertical line with maximum density: " << mid_line_xcoor << endl;

    //Fixed macros crossed by the mid_line
    for( unsigned int iMod = 0 ; iMod < m_placedb.m_modules.size() ; iMod++ )
    {
	Module& curModule = m_placedb.m_modules[iMod];
	if( curModule.m_height > m_placedb.m_rowHeight && 
		curModule.m_x < mid_line_xcoor &&
		curModule.m_x + curModule.m_width > mid_line_xcoor )
	    curModule.m_isFixed = true;
    }
    m_placedb.RemoveFixedBlockSite();

    bool bLegalVertical = false;

    cout << "Legalize to vertical line" << endl;	
    while( (!bFinish) && (prelegal_factor > search_bound) )
    {
	prelegal_factor = prelegal_factor - search_step;
	cout << "Legalization factor: " << prelegal_factor << endl;

	RestoreOrig();

	bool result = DoVerticalLine( prelegal_factor, mid_line_xcoor );
	//m_placedb.OutputGnuplotFigureWithZoom( "vertical_line", false, true, true );
	//exit(0);
	if( result )
	{
	    double wirelength = m_placedb.CalcHPWL();

	    double cost = wirelength;	

	    if( util > 0 )
	    {
		pBin->UpdateBinUsage();
		double penalty = pBin->GetPenalty( util );
		cost = cost * (1.0+(penalty/100.0));
		printf( " (p = %.2f) ", penalty );
	    }


	    cout << "Success " << cost << endl;
	    if( cost < m_best_cost )	
	    {
		SaveBest( prelegal_factor, cost );
	    }
	    //Used to be legalized before,
	    //and find a worse cost:
	    //the search is terminated 
	    else if( bLegalVertical )
	    {
		bFinish = true;
	    }
	}
	else
	{
	    cout << "Fail at "; 
	    const int unlegal_cellindex = m_cell_order.size() - m_unlegal_count;
	    printf("(%d)th module %d (%.2f,%.2f) width: %.2f height: %.2f\n",
		    unlegal_cellindex,
		    m_cell_order[unlegal_cellindex], 
		    m_placedb.m_modules[m_cell_order[unlegal_cellindex]].m_x,
		    m_placedb.m_modules[m_cell_order[unlegal_cellindex]].m_y,
		    m_placedb.m_modules[m_cell_order[unlegal_cellindex]].m_width,
		    m_placedb.m_modules[m_cell_order[unlegal_cellindex]].m_height );
	    flush(cout);

	    //Used to be legalized before,
	    //and find a unlegalized solution:
	    //the search is terminated
	    if( bLegalVertical )
	    {
		bFinish = true;
	    }
	}

	bLegalVertical = bLegalVertical | result;

    }

    if( bLegalVertical )
	RestoreBest();

    return bLegalVertical;
    //@test code: test for legalization from a vertical line
#endif
 
    bool bFinalLegal = false; 
    SetProcessList();
    bFinalLegal = DoLeftRightUntil( util, stop_prelegal_factor );

    if( !bRobust || bFinalLegal )
    {
	return bFinalLegal;
    }

    //*********************
    //* Roubust Legalizer *
    //*********************
#if 0 
    RestoreGlobalResult();
    
    //Run for macro legal
    if( true )
    {
	CMacroLegal mLegal(m_placedb, param.n_MacroRowHeight,40);
	vector<int> macro_ids;
	bool bMLegal = mLegal.Legalize(macro_ids);

	if( bMLegal )
	{
	    if( param.bShow )
	    {
		printf("Macro legal: Success\n");
	    }
	    SetNonMacroProcessList( macro_ids ); 
	    PrepareNonMacroLeftRightFreeSites( macro_ids );
	    bool final  = DoLeftRightUntil( util, stop_prelegal_factor ); 

	    if( final )
	    {
		return final;
	    }
	}
	else
	{
	    if( param.bShow )
	    {
		printf("Macro legal: Fail\n");
	    }
	}
    }
#endif

#if 1 
    //Run for macro shifter
    RestoreGlobalResult();
    if( true )
    {
	bool bMSLegal = MacroShifter(10,false);

	if( bMSLegal )
	{
	    if( param.bShow )
	    {
		printf("Macro shifter: Success\n");
	    }
	    SetNonMacroProcessList( m_macro_ids ); 
	    PrepareNonMacroLeftRightFreeSites( m_macro_ids );
	    bool final = DoLeftRightUntil( util, stop_prelegal_factor ); 

	    if( final )
	    {
		return final;
	    }
	}
	else
	{
	    if( param.bShow )
	    {
		printf("Macro shifter: Fail\n");
	    }
	}
    }
#endif

    //Final robust legalizer
    SetProcessList();
    PrepareLeftRightFreeSites();
    RestoreGlobalResult();
    SaveOrig();

    if( true )
    {	
	
	if( param.bShow )
	{
	    cout << "Robust legalizer: " << endl;
	}

	double start = 1.0;
	double step = 0.05;
	double stop = 0.05;
	    
	while( start >= stop )
	{
	    double t = seconds();
	    
	    SetLeftFreeSites();
	    RestoreOrig();

	    if( param.bShow )
	    {
                cout << "Legalization factor: " << start << " ";
	    }
	    
	    bool final = DoLeftMacroFirst(start);

	    if( param.bShow )
	    {
		if( final )
		    cout << "Success ";
		else
		    cout << "Fail ";
		printf(" runtime %.2f secs\n", seconds()-t );
		flush(cout);
	    }
	    else
	    {
		cout << "*";
	    }

	    if( final )
	    {
		return final;
	    }
	    else
	    {
		start = start - step;
	    }
	}
    }
    
    //test code
    //m_placedb.OutputGnuplotFigureWithZoom( "robust", false, true, true );
    //@test code

    return bFinalLegal;

}

bool CTetrisLegal::DoLeft(const double& prelegal_factor)
{
#if 0
    cout << "Search row limit: ";
    if( m_row_limit < 1 )
    {
	cout << m_row_factor << " times of cell height " << endl;
    }
    else
    {
	cout << m_row_limit << " row heights " << endl;
    }
    flush(cout);
#endif
    CalculateNewLocation(prelegal_factor);
    //test code
    CalculateCellOrder();
    //CalculateCellOrderMacroFirst();
    //@test code
    bool bLegal = LegalizeByCellOrder();

    return bLegal;
}

bool CTetrisLegal::DoLeftMacroFirst(const double& prelegal_factor)
{
    CalculateNewLocation(prelegal_factor);
    CalculateCellOrderMacroFirst();
    bool bLegal = LegalizeByCellOrder();

    return bLegal;

}

void CTetrisLegal::CalculateNewLocation(const double& prelegal_factor)
{
    for( unsigned int i = 0 ; i < m_process_list.size() ; i++ )
    {
	Module& curModule = m_placedb.m_modules[ m_process_list[i] ];
	if( curModule.m_isFixed )
	{
	    fprintf(stderr, "module %d should not be processed by CalculateNewLocation()\n", m_process_list[i] );
	    continue;
	}

	double newX = (curModule.m_x - m_chip_left_bound)*prelegal_factor + m_chip_left_bound;
	double newY = curModule.m_y;

	//Rounding for macros
	if( curModule.m_height > m_placedb.m_rowHeight )
	    newX = Rounding(newX);

	m_placedb.SetModuleLocation( m_process_list[i], newX, newY );
    }
}

void CTetrisLegal::CalculateCellOrderMacroFirst(void)
{
#if 0
    m_cell_order.clear();
    m_cell_order.reserve( m_placedb.m_modules.size() );

    for( unsigned int i = 0 ; i < m_placedb.m_modules.size() ; i++ )
    {
	if( !m_placedb.m_modules[i].m_isFixed )
	    m_cell_order.push_back(i);
    }
#endif
    m_cell_order = m_process_list;

    LessXCoorMacroFirst::m_placedb = &m_placedb;

    //Legal the macros first
    sort( m_cell_order.begin(), m_cell_order.end(), LessXCoorMacroFirst() );

}

void CTetrisLegal::CalculateCellOrder(void)
{
    m_cell_order = m_process_list;
    
    LessXCoor::m_placedb = &m_placedb;

    sort( m_cell_order.begin(), m_cell_order.end(), LessXCoor() );	
    //Legal the macros first
    //sort( m_cell_order.begin(), m_cell_order.end(), LessXCoorMacroFirst() );
}

bool CTetrisLegal::LegalizeByCellOrder(void)
{
#ifdef _LEGALLOG_
    //test code
    cout << "# of modules: " << m_cell_order.size() << endl;
    //@test code
#endif
    for( unsigned int i = 0 ; i < m_cell_order.size() ; i++ )
    {
#ifdef _LEGALLOG_
	//test code
	printf("Processing (%d)th module %d (%.2f,%.2f) width: %.2f height: %.2f\n",
		i,
		m_cell_order[i], 
		m_placedb.m_modules[m_cell_order[i]].m_x,
		m_placedb.m_modules[m_cell_order[i]].m_y,
		m_placedb.m_modules[m_cell_order[i]].m_width,
		m_placedb.m_modules[m_cell_order[i]].m_height );
	flush(cout);
	//@test code
#endif
	if( false == PlaceCellToLegalLocation( m_cell_order[i] ) )
	{
	    m_unlegal_count = m_cell_order.size() - i;
	    return false;
	}
    }
    return true;
}


void CTetrisLegal::GetLegalLocations( const int& cellid, vector<CLegalLocation>& legalLocations )
{
    legalLocations.clear();

    const Module& curModule = m_placedb.m_modules[cellid];
    //double xbound = floor( curModule.m_x - (m_left_factor*m_average_cell_width) );


    //const int& row_limit = m_row_limit;
    //Decide the row_limit acoording to cell's width
    int row_limit;
    if( m_row_limit < 1 )
    {
	row_limit = static_cast<int>(ceil(curModule.m_height/m_placedb.m_rowHeight)*m_row_factor);
    }
    else
    {
	row_limit = m_row_limit;	
    }

    //test code
    //Set the row_limit to full chip height
    //row_limit = m_placedb.m_sites.size();
    //@test code


    //Integration with macro legalizer
    //if( m_bMacroLegalized && curModule.m_height > m_placedb.m_rowHeight )
    //	row_limit = 0;

    //test code
    //cout << "row limit: " << row_limit << endl;
    //@test code

    //***********************************************************************
    //* For each row within the row_limit,                                *
    //* add the leftmost possible x location (>=xbound) into legalLocations *
    //***********************************************************************
    //For standard cells
    if( curModule.m_height <= m_placedb.m_rowHeight )
    {
#ifdef _LEGALLOG_
	//test code
	cout << "(stdcell)" << endl;
	flush(cout);
	//@test code
#endif
	//int median_site_index = GetSiteIndex( curModule.m_y );
	//int low_site_index = max( 0, median_site_index - row_limit );
	//int high_site_index = min( static_cast<int>(m_free_sites.size())-1, 
	//		median_site_index + row_limit );

	//Enable/Disable GetCellLegalLocationsTowardLeft()
#if 1 
	AggressiveCellLegalLocationsSearch( cellid,
		legalLocations );
	//GetCellLegalLocationsTowardLeft( cellid,
	//		low_site_index,
	//		high_site_index,
	//		legalLocations,
	//		xbound );
#else
	for( int iRow = low_site_index ; iRow <= high_site_index ; iRow++ )
	{
	    const CSiteRow& curRow = m_free_sites[iRow];	

	    for( unsigned int iInterval = 0 ; 
		    iInterval < curRow.m_interval.size() ; 
		    iInterval = iInterval + 2 )
	    {
		double xstart = curRow.m_interval[iInterval];
		double xend = curRow.m_interval[iInterval+1];

		//Discard illegal intervals
		if( xend < xbound )
		    continue;

		xstart = max( xstart, xbound );
		//Check if this interval has enough width
		if( xend - xstart >= curModule.m_width )
		{
		    legalLocations.push_back( CLegalLocation( iRow, xstart ) );
		    break;
		}	
	    }	
	}
#endif		
    }
    //For macros (occupy more than one row)
    else
    {
#ifdef _LEGALLOG_
	//test code
	cout << "(macro)" << endl;
	flush(cout);
	//@test code
#endif
	//int median_site_index = GetSiteIndex( curModule.m_y );
	//# of row occupied by this macro
	//int needed_row_count = static_cast<int>( ceil(curModule.m_height/m_site_height) );
	//int low_site_index = max( 0, median_site_index - row_limit );
	//int high_site_index = min( static_cast<int>(m_free_sites.size())-needed_row_count,
	//		median_site_index + row_limit );

	AggressiveMacroDiamondSearch( cellid, legalLocations );

#if 0
	if (true)
	{
	    vector<CSiteRow> sites;

	    const int module_row_number = static_cast<int>(ceil(curModule.m_height/m_placedb.m_rowHeight));

	    int highest_associated_row_index = 
		min( static_cast<int>(m_free_sites.size())-1, high_site_index+module_row_number-1 );

	    for( int iRow = low_site_index ; iRow <= highest_associated_row_index ; iRow++ )
	    {
		sites.push_back( m_free_sites[iRow] );
	    }

	    GetMacroLegalLocationsTowardLeft( cellid, sites, legalLocations, xbound );
	    //GetMacroLegalLocationsTowardOrig( cellid, sites, legalLocations);

	    for( unsigned int iLoc = 0 ; iLoc < legalLocations.size() ; iLoc++ )
	    {
		legalLocations[iLoc].m_site_index += low_site_index;
	    }

	    //test code
	    //cout << "legalLocations: ";
	    //for( unsigned int iLoc = 0 ; iLoc < legalLocations.size() ; iLoc++ )
	    //{
	    //	printf("(%d,%.2f) ", legalLocations[iLoc].m_site_index, legalLocations[iLoc].m_xcoor );
	    //}
	    //cout << endl;
	    //flush(cout);
	    //@test code
	}
	else
	{
	    //For each need-for-searching row, find if there is a legal location
	    for( int iRow = low_site_index ; iRow <= high_site_index ; iRow++ )
	    {
		//Find the leftest interval with the upon 'needed rows' which all have 
		//free space for this cell
		//"Scanline algorithm"
		vector<CTerminal> terminals;

		//Push all terminals into an array
#ifdef _LEGALLOG_
		//test code
		cout << "Row Content: " << endl;
		//@test code
#endif
		for( int iPushRow = iRow ; iPushRow < iRow+needed_row_count ; iPushRow++ )
		{
#ifdef _LEGALLOG_
		    //test code
		    printf("Row %d: ", iPushRow );
		    //@test code
#endif
		    const CSiteRow& pushRow = m_free_sites[iPushRow];
		    for( unsigned int i = 0 ; i < pushRow.m_interval.size() ; i=i+2 )
		    {
#ifdef _LEGALLOG_
			//test code
			printf("(%.2f,%.2f,%.2f) ", 
				pushRow.m_interval[i], 
				pushRow.m_interval[i+1],
				pushRow.m_interval[i+1]-pushRow.m_interval[i]);
			//@test code
#endif
			terminals.push_back( 
				CTerminal( pushRow.m_interval[i], CTerminal::Left, iRow ) );
			terminals.push_back( 
				CTerminal( pushRow.m_interval[i+1], CTerminal::Right, iRow ) );	
		    }
#ifdef _LEGALLOG_
		    //test code
		    printf("\n");
		    //@test code
#endif
		}	

		//Sort terminals by their x coordinates			
		sort( terminals.begin(), terminals.end(), LessXCoor() );

		//To find a legal location,
		//we have to find overlapped segments with width >= curModule.m_width 
		int overlap_count = 0;
		for( unsigned int i = 0 ; i < terminals.size() ; i++ )
		{
		    const CTerminal& curTerminal = terminals[i];

		    if( curTerminal.m_type == CTerminal::Left )
		    {
			overlap_count++;	
		    }
		    else
		    {
			//Current segment is overlapped by all needed rows
			//Check if the width enough for this Macro
			if( overlap_count == needed_row_count )
			{
			    double xleft = max( xbound, terminals[i-1].m_xcoor );
			    double xright = curTerminal.m_xcoor;
			    double width = xright - xleft;

			    if( width >= curModule.m_width )
			    {
				legalLocations.push_back( 
					CLegalLocation( iRow, xleft ) );
				break;
			    }	
			}

			overlap_count--;
		    }	
		}
	    }//@For each need-for-searching row, find if there is a legal location
	}
#endif

    }
}

bool CTetrisLegal::PlaceCellToLegalLocation( const int& cellid )
{
    vector<CLegalLocation> legalLocations;
    GetLegalLocations( cellid, legalLocations );

    //Check if no legal locations are found
    if( legalLocations.empty() )
	return false;

    //const Module& curModule = m_placedb.m_modules[cellid];

    //Old method to find the best location in legalLocations
#if 0
    //*****************************************************
    //*Place the cell to the location with lowest cost and*
    //*update m_free_sites                                *
    //*****************************************************
    //Determine a best location among multiple legal locations
    //(put to the from of legalLocations)
    if( legalLocations.size() > 1 )	
    {

	//Calculate shift for each location
	for( vector<CLegalLocation>::iterator iteLoc = legalLocations.begin() ;
		iteLoc != legalLocations.end() ; iteLoc++ )
	{
	    CPoint p1( curModule.m_x, curModule.m_y );
	    CPoint p2( iteLoc->m_xcoor, m_free_sites[iteLoc->m_site_index].m_bottom );
	    iteLoc->m_shift = Distance( p1, p2 ); 
	}

	sort( legalLocations.begin(), legalLocations.end(), LessShift() );

	//There are at least two locations has the lowest shift
	//Remove locations with higher shift and sort by wirelength
	if( legalLocations[0].m_shift == legalLocations[1].m_shift )
	{
	    unsigned int new_size;
	    double min_shift = legalLocations[0].m_shift;

	    //Determine the new size
	    for( new_size = 2 ; 
		    new_size < legalLocations.size() && 
		    legalLocations[new_size].m_shift == min_shift ;
		    new_size++ );

	    legalLocations.resize( new_size, CLegalLocation(-1,-1) );

	    //Calculate wirelength for each location
	    for( vector<CLegalLocation>::iterator iteLoc = legalLocations.begin() ;
		    iteLoc != legalLocations.end() ; iteLoc++ )
	    {
		double xcoor = iteLoc->m_xcoor;
		double ycoor = m_free_sites[iteLoc->m_site_index].m_bottom;
		m_placedb.SetModuleLocation( cellid, xcoor, ycoor );
		iteLoc->m_wirelength = m_placedb.GetModuleTotalNetLength( cellid );
	    }

	    sort( legalLocations.begin(), legalLocations.end(), LessWirelength() );

	}

    }

    CLegalLocation bestLocation = legalLocations.front();
#endif

    int bestIndex = ReturnBestLocationIndex( cellid, legalLocations );
    CLegalLocation bestLocation = legalLocations[bestIndex];

    double xcoor = bestLocation.m_xcoor;
    double ycoor = m_free_sites[bestLocation.m_site_index].m_bottom;
    m_placedb.SetModuleLocation( cellid, xcoor, ycoor );

#ifdef _LEGALLOG_
    //test code
    printf("Move module %d to (%.2f,%.2f) width: %.2f\n",
	    cellid, xcoor, ycoor, curModule.m_width );
    flush(cout);
    //@test code
#endif

    //Old update free sites method
#if 0
    if( curModule.m_height <= m_placedb.m_rowHeight )
    {
	UpdateFreeSite( bestLocation.m_site_index, 
		xcoor, 
		curModule.m_width ); 
    }
    else
    {
	//Update free site for a macro
	int needed_row_count = static_cast<int>( ceil(curModule.m_height/m_site_height) );
	for( int iRow = bestLocation.m_site_index ; 
		iRow < bestLocation.m_site_index + needed_row_count ;
		iRow++ )
	{
	    UpdateFreeSite( iRow, xcoor, curModule.m_width );
	}
    }
#endif

    UpdateFreeSite( cellid );

    return true;

}

void CTetrisLegal::UpdateFreeSite( const int& rowId, const double& xstart, const double& width )
{
    CSiteRow& curRow = m_free_sites[rowId];

    const double xleft = xstart;
    const double xright = xstart + width;

    for( unsigned int i = 0 ; i < curRow.m_interval.size() ; i=i+2 )
    {
	double interval_left = curRow.m_interval[i];
	double interval_right = curRow.m_interval[i+1];

	//Avoid unneccessary check
	if( interval_right <= xleft )
	    continue;	

	//No further check is needed
	if( interval_left >= xright )
	    break;

	//Update free sites
	if( interval_left == xleft && interval_right == xright )
	{
	    //This situation is incorrect for Tetris legalizaer
	    //only left == left and right == right stands
	    //cerr <<	"Warning: incorrect module location in UpdateFreeSite()" << endl;
	    //cerr << "Row " << rowId << " module: (" << xleft << "," << xright << ")" 
	    //	<< " interval: (" << interval_left << "," << interval_right << )" << endl;
	    curRow.m_interval.erase( curRow.m_interval.begin()+i );
	    curRow.m_interval.erase( curRow.m_interval.begin()+i );
	    i = i-2;
	}
	else if( interval_right > xright && interval_left >= xleft )
	{
	    curRow.m_interval[i] = xright;
	}
	else if( interval_left < xleft && interval_right <= xright )
	{
	    curRow.m_interval[i+1] = xleft;	
	}
	else if( interval_left < xleft && interval_right > xright )
	{
	    curRow.m_interval[i+1] = xleft;
	    curRow.m_interval.insert( curRow.m_interval.begin()+(i+2), interval_right );
	    curRow.m_interval.insert( curRow.m_interval.begin()+(i+2), xright );
	    i = i+2;
	}
	else
	{
	    cerr << "Warning: UpdateFreeSite() Error" << endl;
	}
    }
}
