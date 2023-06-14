#include "BinPacking.h"
#include "placedb.h"


//namespace 
//{
	struct Interval
	{
	public:
		Interval(void);
		~Interval(void);
		Interval(const double& set_x, const double& set_y, const double& set_width );
		double m_x, m_y; //left-bottom coordinate
		double m_width;  //available width

	};

	struct TmpModule
	{
	public:
		TmpModule(void);
		~TmpModule(void);
		TmpModule( const double &set_width, const int &set_ID ) : m_width(set_width), m_ID(set_ID){};
		double m_x, m_y;
		double m_width;
		int m_ID;
	};

	class LessThan
	{
	public:
		bool operator()( const TmpModule &m1, const TmpModule &m2 );
		bool operator()( const Interval &i1, const Interval &i2 );
	};
//}

#include <cstdio>
#include <iostream>
#include <queue>
#include <list>

using namespace std;



bool CBinPacking::Packing(CPlaceDB& fplan, 
						  const double &left, 
						  const double &bottom, 
						  const double &right, 
						  const double &top, 
						  const std::vector<int> block_list )
{
	priority_queue<TmpModule, vector<TmpModule>, LessThan > heapModule;
	priority_queue<Interval, vector<Interval>, LessThan > heapInterval;

	// Read all modules in the partition
	//Partition &part = fplan.m_partitions[ PartID ];

	// calculate total module width
	double totalModuleWidth = 0;
	
	for( vector<int>::const_iterator ite = block_list.begin() ;
		ite < block_list.end() ;
		ite++ )
	{
		Module &m = fplan.m_modules[ *ite ];

        if( m.m_name.substr( 0, 2 ) == "__" )
            continue;

		heapModule.push( TmpModule( m.m_width, *ite ) );
		totalModuleWidth += m.m_width;
	}


	//test code
	//printf("heapModule size %d\n", heapModule.size() );
	//@test code

	//const double part_left = part.left;
	//const double part_bottom = part.bottom;
	//const double part_right = part.right;
	//const double part_top = part.top;

	vector<CSiteRow>::iterator iteBeginRow, iteEndRow;

	// find the begin row
	for( iteBeginRow = fplan.m_sites.begin() ; 
		iteBeginRow < fplan.m_sites.end() ; 
		iteBeginRow++ )
	{

		if( iteBeginRow->m_bottom + iteBeginRow->m_height > bottom )
		{
			break;
		}

	}

	// find the end row
	for( iteEndRow = iteBeginRow ;
		iteEndRow < fplan.m_sites.end() ;
		iteEndRow++ )
	{
		if( iteEndRow->m_bottom + iteEndRow->m_height >= top )
		{
			break;
		}
	}

	if( iteEndRow == fplan.m_sites.end() )
		iteEndRow--;

	if( iteBeginRow == fplan.m_sites.end() )
	{
		printf("\n\n\nWarning: no sites in the given partition\n\n\n" );
		return false;
	}
	//assert( iteBeginRow != fplan.m_sites.end() );

	// read all intervals in partition
	double totalIntervalWidth = 0; // calculate total interval width;
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
				if( interval[0] >= right || interval[1] <= left )  // screen unnecessary checks
					continue;

				if( interval[0] >= left && interval[1] <= right )
				{
					heapInterval.push( Interval( interval[0], iteRow->m_bottom, interval[1] - interval[0] ) );
					////test code
					//printf("push interval x: %f y: %f width: %f\n", interval[0], iteRow->m_bottom, interval[1] - interval[0] );
					////@test code
					totalIntervalWidth += interval[1] - interval[0];
					assert( interval[1] - interval[0] > 0 );
				}
				else if( interval[1] > right && interval[0] >= left )
				{
					heapInterval.push( Interval( interval[0], iteRow->m_bottom, right - interval[0] ) );
					////test code
					//printf("push interval x: %f y: %f width: %f\n", interval[0], iteRow->m_bottom, right - interval[0] );
					////@test code
					totalIntervalWidth += right - interval[0];
					assert( right - interval[0] > 0 );
				}
				else if( interval[0] < left && interval[1] <= right )
				{
					heapInterval.push( Interval( left, iteRow->m_bottom, interval[1] - left ) );
					////test code
					//printf("push interval x: %f y: %f width: %f\n", part.left, iteRow->m_bottom, interval[1] - part.left );
					////@test code
					totalIntervalWidth += interval[1] - left;
					assert( interval[1] - left > 0 );
				}
				else if( interval[0] < left && interval[1] > right )
				{
					heapInterval.push( Interval( left, iteRow->m_bottom, right - left ) );
					////test code
					//printf("push interval x: %f y: %f width: %f\n", left, iteRow->m_bottom, part.right - part.left );
					////@test code
					totalIntervalWidth += right - left;
					assert( right - left > 0 );
				}
				else
				{
					printf("Jin: Read Intervals Error In CBinPacking\n");
					exit(-1);
				}

			}
		}
	}//@read all intervals in partition



	// check for available solution
	if( totalModuleWidth > totalIntervalWidth )
	{
		////test code
		//printf("totalModuleWidth %f > totalIntervalWidth %f\n", totalModuleWidth, totalIntervalWidth );
		////@test code
		return false;
	}

	list<TmpModule> listPackedModule;
	while( !heapInterval.empty() && !heapModule.empty() )
	{
		if( heapModule.top().m_width > heapInterval.top().m_width )
			break;

		((TmpModule&)heapModule.top()).m_x = heapInterval.top().m_x;
		((TmpModule&)heapModule.top()).m_y = heapInterval.top().m_y;

		if( heapInterval.top().m_width > heapModule.top().m_width )
		{
			heapInterval.push( Interval( heapInterval.top().m_x + heapModule.top().m_width, 
				heapInterval.top().m_y, 
				heapInterval.top().m_width - heapModule.top().m_width ) );
		}

		listPackedModule.push_back( heapModule.top() );

		heapModule.pop();
		heapInterval.pop();
	}

	if( !heapModule.empty() )
	{
		////test code
		//for( list<TmpModule>::iterator ite = listPackedModule.begin() ;
		//	ite != listPackedModule.end() ;
		//	ite++ )
		//{
		//	fplan.m_modules[ ite->m_ID].m_x = ite->m_x;
		//	fplan.m_modules[ ite->m_ID].m_y = ite->m_y;
		//}
		//printf("tatalModuleWidth %f, totalIntervalWidth %f, number of modules %d\n", 
		//	totalModuleWidth, totalIntervalWidth, heapModule.size() + listPackedModule.size() );
		////@test code
		return false;
	}
	else
	{
		for( list<TmpModule>::iterator ite = listPackedModule.begin() ;
			ite != listPackedModule.end() ;
			ite++ )
		{
            fplan.SetModuleLocation( ite->m_ID, ite->m_x, ite->m_y );
			//fplan.m_modules[ ite->m_ID].m_x = ite->m_x;
			//fplan.m_modules[ ite->m_ID].m_y = ite->m_y;
            //fplan.m_modules[ ite->m_ID].CalcCenter();
			////test code
			//printf("module height %f\n", fplan.m_modules[ ite->m_ID ].m_height );
			////@test code
		}

		////test code
		//printf("tatalModuleWidth %f, totalIntervalWidth %f, number of modules %d\n", 
		//	totalModuleWidth, totalIntervalWidth, heapModule.size() + listPackedModule.size() );
		////@test code
		return true;
	}


}

// Check for packing solution (do not update coordinate)
bool CBinPacking::CheckSolution(CPlaceDB& fplan, 
						  const double &left, 
						  const double &bottom, 
						  const double &right, 
						  const double &top, 
						  const std::vector<int>& block_list )
{
	priority_queue<TmpModule, vector<TmpModule>, LessThan > heapModule;
	priority_queue<Interval, vector<Interval>, LessThan > heapInterval;

	// Read all modules in the partition
	//Partition &part = fplan.m_partitions[ PartID ];

	// calculate total module width
	double totalModuleWidth = 0;
	for( vector<int>::const_iterator ite = block_list.begin() ;
		ite < block_list.end() ;
		ite++ )
	{
		Module &m = fplan.m_modules[ *ite ];

        if( m.m_name.substr( 0, 2 ) == "__" )
            continue;

        heapModule.push( TmpModule( m.m_width, *ite ) );
		totalModuleWidth += m.m_width;
	}

	vector<CSiteRow>::iterator iteBeginRow, iteEndRow;

	// find the begin row
	for( iteBeginRow = fplan.m_sites.begin() ; 
		iteBeginRow < fplan.m_sites.end() ; 
		iteBeginRow++ )
	{

		if( iteBeginRow->m_bottom + iteBeginRow->m_height > bottom )
		{
			break;
		}

	}

	// find the end row
	for( iteEndRow = iteBeginRow ;
		iteEndRow < fplan.m_sites.end() ;
		iteEndRow++ )
	{
		if( iteEndRow->m_bottom + iteEndRow->m_height >= top )
		{
			break;
		}
	}

	if( iteEndRow == fplan.m_sites.end() )
		iteEndRow--;

	assert( iteBeginRow != fplan.m_sites.end() );

	// read all intervals in partition
	double totalIntervalWidth = 0; // calculate total interval width;
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
				if( interval[0] >= right || interval[1] <= left )  // screen unnecessary checks
					continue;

				if( interval[0] >= left && interval[1] <= right )
				{
					heapInterval.push( Interval( interval[0], iteRow->m_bottom, interval[1] - interval[0] ) );
					totalIntervalWidth += interval[1] - interval[0];
					assert( interval[1] - interval[0] > 0 );
				}
				else if( interval[1] > right && interval[0] >= left )
				{
					heapInterval.push( Interval( interval[0], iteRow->m_bottom, right - interval[0] ) );
					totalIntervalWidth += right - interval[0];
					//assert( left - interval[0] > 0 );
				}
				else if( interval[0] < left && interval[1] <= right )
				{
					heapInterval.push( Interval( left, iteRow->m_bottom, interval[1] - left ) );
					totalIntervalWidth += interval[1] - left;
					assert( interval[1] - left > 0 );
				}
				else if( interval[0] < left && interval[1] > right )
				{
					heapInterval.push( Interval( left, iteRow->m_bottom, right - left ) );
					totalIntervalWidth += right - left;
					assert( right - left > 0 );
				}
				else
				{
					printf("Jin: Read Intervals Error In CBinPacking\n");
					exit(-1);
				}

			}
		}
	}//@read all intervals in partition



	// check for available solution
	if( totalModuleWidth > totalIntervalWidth )
	{
		return false;
	}

	//list<TmpModule> listPackedModule;
	while( !heapInterval.empty() && !heapModule.empty() )
	{
		if( heapModule.top().m_width > heapInterval.top().m_width )
			break;

		if( heapInterval.top().m_width > heapModule.top().m_width )
		{
			heapInterval.push( Interval( heapInterval.top().m_x + heapModule.top().m_width, 
				heapInterval.top().m_y, 
				heapInterval.top().m_width - heapModule.top().m_width ) );
		}

		//listPackedModule.push_back( heapModule.top() );

		heapModule.pop();
		heapInterval.pop();
	}

	if( !heapModule.empty() )
	{
		return false;
	}
	else
	{
		//for( list<TmpModule>::iterator ite = listPackedModule.begin() ;
		//	ite != listPackedModule.end() ;
		//	ite++ )
		//{
		//	fplan.m_modules[ ite->m_ID].m_x = ite->m_x;
		//	fplan.m_modules[ ite->m_ID].m_y = ite->m_y;
		//}

		return true;
	}

}
//@Check for packing solution (do not update coordinate)


//indark: 2005-10-25 partion based  should not included in placedb
// bool CBinPacking::Packing(CFloorplan& fplan, const int& PartID)
// {
// 	return Packing( fplan, 
// 			fplan.m_partitions[ PartID ].left, 
// 			fplan.m_partitions[ PartID ].bottom,
// 			fplan.m_partitions[ PartID ].right,
// 			fplan.m_partitions[ PartID ].top,
// 			fplan.m_partitions[ PartID ].moduleList );
// }
// 
// // Check for packing solution (do not update coordinate)
// bool CBinPacking::CheckSolution(CFloorplan& fplan, const int& PartID)
// {
// 	return CheckSolution( fplan, 
// 			fplan.m_partitions[ PartID ].left, 
// 			fplan.m_partitions[ PartID ].bottom,
// 			fplan.m_partitions[ PartID ].right,
// 			fplan.m_partitions[ PartID ].top,
// 			fplan.m_partitions[ PartID ].moduleList );
// 	
// }
// //@Check for packing solution (do not update coordinate)



Interval::Interval(void)
{
}

Interval::~Interval(void)
{
}

Interval::Interval( const double& set_x, const double& set_y, const double& set_width )
{
	m_x = set_x;
	m_y = set_y;
	m_width = set_width;
}

TmpModule::TmpModule(void)
{
}

TmpModule::~TmpModule(void)
{
}

//TmpModule::TmpModule( const double & set_x, const double &set_y, const double &set_width, const int &set_ID )
//{
//	m_x = set_x; m_y = set_y; m_width = set_width; m_ID = set_ID;
//}

bool LessThan::operator()( const TmpModule &m1, const TmpModule &m2 )
{
	return m1.m_width < m2.m_width;
}

bool LessThan::operator ()( const Interval &i1, const Interval &i2 )
{
	return i1.m_width < i2.m_width;
}	
