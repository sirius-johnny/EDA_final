#include "DPlace.h"
//Set the default parameter
CDetailPlacerParam::CDetailPlacerParam(void) :
			bRunBBCellSwap(false),
			bbWindowSize(3),
			bbOverlapSize(2),
			bbIteration(1)
		{}

ostream& operator<< (ostream& ostr, const CDetailPlacerParam& dpParam )
{
	ostr << "********Contents of CDetailPlacerParam*******" << endl;
	ostr << "bRunBBCellSwap: " << dpParam.bRunBBCellSwap << endl;
	ostr << "bbWindowSize: " << dpParam.bbWindowSize << endl;
	ostr << "bbOverlapSize: " << dpParam.bbOverlapSize << endl;
	ostr << "bbIteration: " << dpParam.bbIteration << endl;
	return ostr;
}


void CDetailPlacer::InitForBBCellSwap(void)
{
	//Clear data in each segment
	for( vector<CSegment>::iterator iteSegment = m_segments.begin();
			iteSegment != m_segments.end() ; iteSegment++ )
	{
		iteSegment->m_module_ids.clear();
	}

	//Add all id's of non-Macro modules into m_segments
	for( unsigned int iModule = 0 ; iModule < m_placedb.m_modules.size() ; iModule++ )
	{
		const Module& curModule = m_placedb.m_modules[iModule];
		//Skip Macro modules
		if( curModule.m_height > m_placedb.m_rowHeight || curModule.m_isFixed )
			continue;

		//Find the corresponding segment of this module,
		//and insert the module id into the segment
		vector<CSegment>::iterator iteFindSegment;
		for( iteFindSegment = m_segments.begin() ;
				iteFindSegment != m_segments.end() ; iteFindSegment++ )
		{
			if( iteFindSegment->m_bottom == curModule.m_y &&
					iteFindSegment->m_left <= curModule.m_x &&
					iteFindSegment->m_right >= curModule.m_x )
			{
				iteFindSegment->AddModuleId( iModule );
				break;
			}
		}

		//Warning: this module is not on any segments
		if( iteFindSegment == m_segments.end() )
		{
			cerr << "Warning: Module " << iModule << " is not on any segments" << endl;
		}
	}
	
	//Sort the module id's by their x coordinates
	for( vector<CSegment>::iterator iteSegment = m_segments.begin();
			iteSegment != m_segments.end() ; iteSegment++ )
	{
		sort( iteSegment->m_module_ids.begin(),
				iteSegment->m_module_ids.end(),
				CompareModuleById::CompareXCoor );
	}

}

CDetailPlacer::CDetailPlacer( CPlaceDB& set_placedb, const CParamPlacement& param,
	   const CDetailPlacerParam& dpParam ) : 
	m_placedb( set_placedb ), m_param( param ), m_dpParam( dpParam )
{
	//Maintain site structure
	m_placedb.RemoveMacroSite();

	
	//Initialize class CompareModuleById
	CompareModuleById::m_pDB = &m_placedb;

	//Initialize m_segments
	for( vector<CSiteRow>::const_iterator iteRow = m_placedb.m_sites.begin() ;
			iteRow != m_placedb.m_sites.end() ; iteRow++ )
	{
		for( unsigned int iInterval = 0 ; iInterval < iteRow->m_interval.size() ; iInterval=iInterval+2 )
		{
			m_segments.push_back( CSegment( iteRow->m_bottom,
						iteRow->m_interval[iInterval],
						iteRow->m_interval[iInterval+1] ) );
		}
	}

	//Add all id's of non-Macro modules into m_segments
	for( unsigned int iModule = 0 ; iModule < m_placedb.m_modules.size() ; iModule++ )
	{
		const Module& curModule = m_placedb.m_modules[iModule];
		//Skip Macro modules
		if( curModule.m_height > m_placedb.m_rowHeight || curModule.m_isFixed )
			continue;

		//Find the corresponding segment of this module,
		//and insert the module id into the segment
		vector<CSegment>::iterator iteFindSegment;
		for( iteFindSegment = m_segments.begin() ;
				iteFindSegment != m_segments.end() ; iteFindSegment++ )
		{
			if( iteFindSegment->m_bottom == curModule.m_y &&
					iteFindSegment->m_left <= curModule.m_x &&
					iteFindSegment->m_right >= curModule.m_x )
			{
				iteFindSegment->AddModuleId( iModule );
				break;
			}
		}

		//Warning: this module is not on any segments
		if( iteFindSegment == m_segments.end() )
		{
			cerr << "Warning: Module " << iModule << " is not on any segments" << endl;
		}
	}

	//Sort the module id's by their x coordinates
	for( vector<CSegment>::iterator iteSegment = m_segments.begin();
			iteSegment != m_segments.end() ; iteSegment++ )
	{
		sort( iteSegment->m_module_ids.begin(),
				iteSegment->m_module_ids.end(),
				CompareModuleById::CompareXCoor );
	}


}

void CDetailPlacer::DetailPlace(void)
{
    
	double orig_wirelength, aft_wirelength;
	CWSDistribution* wsdist;
	CellSwap* cswap;
	
	// add one BB swapping before tellux's detailed placer (2006-03-14, donnie)
	if(true)
	{
		cout << "BB Cell Swapping" << endl;
		m_placedb.m_modules_bak = m_placedb.m_modules;
		double t = seconds();
		orig_wirelength = m_placedb.CalcHPWL();
		InitForBBCellSwap();
		cswap = new CellSwap( m_placedb, m_segments );
		//cswap->Solve(4, 3, 1);
		cswap->Solve(3, 2, 1);	// (2006-03-21) donnie  use 3-2-1
		aft_wirelength = m_placedb.CalcHPWL();
		printf( "  HPWL: %.0f (%3.2f%%)\n", 
			aft_wirelength, ((orig_wirelength - aft_wirelength)/orig_wirelength)*100.0);
		cswap->Solve(3, 2, 1);	// (2006-03-21) donnie  use 3-2-1
		//cswap->Solve(m_dpParam.bbWindowSize, m_dpParam.bbOverlapSize, m_dpParam.bbIteration);
		//cswap->Solve(m_dpParam.bbWindowSize+1, m_dpParam.bbOverlapSize+1, m_dpParam.bbIteration);
		aft_wirelength = m_placedb.CalcHPWL();
		printf( "  HPWL: %.0f (%3.2f%%)\n", 
			aft_wirelength, ((orig_wirelength - aft_wirelength)/orig_wirelength)*100.0);
		printf( "  Runtime: %.2f secs\n", seconds() - t );
		delete cswap;
		//m_placedb.OutputGnuplotFigureWithZoom( "out_bb", true, true, true );
	}

	//Detailed placer from TCHsu
	deRunDetail detail;
	detail.runDetail( m_param, m_placedb, 5, 1 );	    // add paramters 2006-03-22 (donnie)

		
	//if( m_dpParam.bRunBBCellSwap )
	if(true)
	{
		cout << "BBCell swapping stage:" << endl;
		m_placedb.m_modules_bak = m_placedb.m_modules;
		double t = seconds();
		orig_wirelength = m_placedb.CalcHPWL();
		InitForBBCellSwap();
		cswap = new CellSwap( m_placedb, m_segments );
		cswap->Solve(3, 2, 1);
		aft_wirelength = m_placedb.CalcHPWL();
		cout << "  Updated WL: " << aft_wirelength << endl;
		printf( "  Improve: %3.2f%%\n", ((orig_wirelength - aft_wirelength)/orig_wirelength)*100.0);
		cswap->Solve(3, 2, 1);
		//cswap->Solve(4, 3, 1);    // (2006-03-21) donnie, turn off 4-3-1
		//cswap->Solve(m_dpParam.bbWindowSize+1, m_dpParam.bbOverlapSize+1, m_dpParam.bbIteration);
		//cswap->Solve(m_dpParam.bbWindowSize, m_dpParam.bbOverlapSize, m_dpParam.bbIteration);
		aft_wirelength = m_placedb.CalcHPWL();
		cout << "  Updated WL: " << aft_wirelength << endl;
		printf( "  Improve: %3.2f%%\n", ((orig_wirelength - aft_wirelength)/orig_wirelength)*100.0);
		printf( "  Runtime: %.2f secs\n", seconds() - t );
		delete cswap;
		//m_placedb.OutputGnuplotFigureWithZoom( "out_bb", true, true, true );
	}
	
}

void CWSDistribution::DoIt(void)
{
	//test code
	//cout << "**********************************" << m_placedb.m_modules[5181].m_x << endl;
	//m_segments[2].m_right = 75;
	//m_segments[2].m_left  = 0;
	//m_segments[2].m_module_ids.resize(3);
	//@test code

	for( unsigned int iSeg = 0 ; iSeg < m_segments.size() ; iSeg++ )
	{
		double orig_wirelength = m_placedb.CalcHPWL();

		//test code
		cout << "Processing segment " << iSeg << endl;
		//@test code

		CDPOneSegment dpOneSegment( m_placedb, m_segments[iSeg] );
		dpOneSegment.DoIt();

		double aft_wirelength = m_placedb.CalcHPWL();
		if( orig_wirelength < aft_wirelength )
			cerr << "Warning: dynamic programming cause worse wirelength" << endl;

	}
}

CDPOneSegment::CDPOneSegment(CPlaceDB& set_placedb, 
		CSegment& set_segment,
		const int& set_dpsite_width ) :
	m_placedb( set_placedb ),
	m_segment( set_segment ),
	m_dpsite_width(set_dpsite_width),
	m_left( set_segment.m_left ),
	m_right( set_segment.m_right ),
	m_module_ids( set_segment.m_module_ids )
{
	m_dpsite_num = static_cast<int>(m_segment.m_right - m_segment.m_left)/m_dpsite_width;
	m_module_num = m_segment.m_module_ids.size();
}

void CDPOneSegment::DoIt(void)
{
	if( 0 == m_module_ids.size() )
		return;

#if 0
	//test code
	cout << "Number of modules: " << m_module_ids.size() << endl;
	cout << "Width of segments: " << m_right - m_left << endl;
	cout << "Left: " << m_left << " Right: " << m_right << endl;
	cout << "Processing module ids: ";
	for( unsigned int i = 0 ; i < m_module_ids.size() ; i++ )
		cout << m_module_ids[i] << "(" << m_placedb.m_modules[m_module_ids[i]].m_x << ") ";
	cout << endl;
	//@test code
#endif

	m_dpsites.resize( m_dpsite_num + 1 );

	for( int i = 0 ; i < m_dpsite_num + 1 ; i++ )
	{
		m_dpsites[i].resize( m_module_num + 1, CDPCell(numeric_limits<double>::max()) );
	}

#if 0
	//TODO
	//Record the orig module location and wirelength
	vector<double> orig_xcoors;	//The original x coordinate of each module
	orig_xcoor.reserve( m_module_ids.size() );
	for( unsigned int i = 0 ; i < m_module_ids.size() ; i++ )
	{

	}
#endif

	//Calculte the total placed width and unplaced width
	//to prune unnecessary wirelength
	double placed_width = 0;
	double unplaced_width = 0;

	//First, move all modules to the rightmost possible site
	for( vector<int>::const_iterator iteModuleId = m_module_ids.begin() ;
			iteModuleId != m_module_ids.end() ; iteModuleId++ )
	{
		const Module& curModule = m_placedb.m_modules[*iteModuleId];
		m_placedb.SetModuleLocation( *iteModuleId,
				m_right - curModule.m_width,
				curModule.m_y );

		unplaced_width += curModule.m_width;
	}

#if 0
	//test code (count net module number)
	map<int, int> net_map;
	for( unsigned int i = 0 ; i < m_module_ids.size() ; i++ )
	{
		int curModuleId = m_module_ids[i];
		const Module& curModule = m_placedb.m_modules[curModuleId];
		for( vector<int>::const_iterator iteNetId = curModule.m_netsId.begin() ;
				iteNetId != curModule.m_netsId.end() ; iteNetId++ )
			net_map[*iteNetId] = 0;
	}
	for( unsigned int i = 0 ; i < m_module_ids.size() ; i++ )
	{
		int curModuleId = m_module_ids[i];
		const Module& curModule = m_placedb.m_modules[curModuleId];
		for( vector<int>::const_iterator iteNetId = curModule.m_netsId.begin() ;
				iteNetId != curModule.m_netsId.end() ; iteNetId++ )
			net_map[*iteNetId]++;
	}
	for( map<int, int>::iterator iteMap = net_map.begin() ;
			iteMap != net_map.end() ; iteMap++ )
		printf("Net %5d: %5d\n", iteMap->first, iteMap->second );
	//@test code

	//test code
	cout << "Move to right WL: " << m_placedb.CalcHPWL() << endl; 
	//@test code
#endif

	//m_dpsites[0][0].m_cost = 0;
	for( int i = 0 ; i < m_dpsite_num ; i++ )
		m_dpsites[i][0].m_cost = 0;

	//Dynamic programming table traversing
	for( int iCell = 0 ; iCell < m_module_num + 1 ; iCell++ )
	{
		int curModuleId = 0;
		double orig_wirelength = 0;

		if( iCell != m_module_num )
		{
			curModuleId = m_module_ids[iCell];
			orig_wirelength = GetModuleTotalNetLength(curModuleId);
		}

		const Module& curModule = m_placedb.m_modules[curModuleId];

		//test code
		//printf( "iCell: %2d, width: %5f, unplaced width: %5f\n", iCell, curModule.m_width, unplaced_width );
		//@test code


		for( int iSite = static_cast<int>(ceil(placed_width)) ; 
				iSite < static_cast<int>(floor( m_dpsite_num - unplaced_width )) ; 
				iSite++ )
		{
			//Propogate to down-right dpcell
			if( iCell != m_module_num )
			{
				m_placedb.SetModuleLocation( curModuleId,
						m_left + ( iSite * m_dpsite_width ),
						m_placedb.m_modules[curModuleId].m_y);
				//test code
				//printf("Set module %4d location: (%2.0f, %2.0f)\n", curModuleId, m_left + ( iSite * m_dpsite_width ), m_placedb.m_modules[curModuleId].m_y ); 
				//@test code
				double cost = GetModuleTotalNetLength( curModuleId ) 
					- orig_wirelength
					+ m_dpsites[iSite][iCell].m_cost;
				int next_iSite = iSite + static_cast<int>(ceil( curModule.m_width ));
				double next_cost = m_dpsites[next_iSite][iCell+1].m_cost;

				if( cost < next_cost )
				{

					m_dpsites[next_iSite][iCell+1].m_cost = cost;
					m_dpsites[next_iSite][iCell+1].m_dirct = CDPCell::dp_topleft;

					//test code
					//printf("Update m_dpsites[%d][%d]: %5f topleft\n", 
					//		next_iSite, iCell+1, cost );
					//@test code
				}
			}	

			//Propagate to right dpcell
			if( iCell != 0 )
			{
				double cost = m_dpsites[iSite][iCell].m_cost;
				double next_cost = m_dpsites[iSite+1][iCell].m_cost;
				if( cost < next_cost )
				{
					m_dpsites[iSite+1][iCell].m_cost = cost;
					m_dpsites[iSite+1][iCell].m_dirct = CDPCell::dp_left;
					//test code
					//printf("Update m_dpsites[%d][%d]: %5f left\n", 
					//		iSite+1, iCell, cost );
					//@test code
				}
			}	

			//Put the placed cell to leftmost possible site
			if( iCell != m_module_num )
			{
				m_placedb.SetModuleLocation( curModuleId, m_left, curModule.m_y );
			}

		}
		placed_width += curModule.m_width;
		unplaced_width -= curModule.m_width;

	}//@Dynamic programming table traversing

	//Backtrack and update the optimal cell location
	//cout << "Backtrace:" << endl;
	int next_start_site = m_dpsite_num;
	for( int iCell = m_module_num ; iCell > 0 ; iCell-- )
	{
		int iSite = next_start_site;
		for( ; iSite > 0 && m_dpsites[iSite][iCell].m_dirct == CDPCell::dp_left ; iSite-- );

		int curModuleId = m_module_ids[iCell-1];
		const Module& curModule = m_placedb.m_modules[curModuleId];

		//test code
		//printf("<%2d> Module %5d, width %5f, x: %5.0f, y: %5.0f\n", 
		//		iCell, curModuleId, curModule.m_width, 
		//		m_left + iSite - curModule.m_width, curModule.m_y );
		//@test code

		m_placedb.SetModuleLocation( curModuleId, 
				m_left + iSite - curModule.m_width,
				curModule.m_y );

		next_start_site = static_cast<int>(iSite - curModule.m_width);
	}
}


double CDPOneSegment::GetModuleTotalNetLength( const int& module_id )
{
	const Module& curModule = m_placedb.m_modules[module_id];

	double result = 0;
	for( vector<int>::const_iterator iteNetId = curModule.m_netsId.begin() ;
			iteNetId != curModule.m_netsId.end() ; iteNetId++ )
	{
		result += m_placedb.GetNetLength( *iteNetId );
	}

	return result;
}
