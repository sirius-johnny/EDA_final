#ifndef MINCUTPLACER_H
#define MINCUTPLACER_H

#include <vector>
#include <string>
#include <map>
#include <iostream>
#include <cassert>
#include <set>
using namespace std;

// ========= 051022:indark ================
#include "ParamPlacement.h"
#include "Partition.h"
#include "placedb.h"


/**
@author Indark
*/
class CMinCutPlacer
{

    public:
      void CreateCluster( const CPlaceDB& in, CPlaceDB& out );
    
    public:
	
	CMinCutPlacer(CPlaceDB& db);
	
	~CMinCutPlacer();
	
		//Partiton
	
	void Init();
	void RecursivePartition();
	bool LevelPartition();
	
	bool RefineNeighbors( int partId );
	bool RefineRegions( int regionId1, int regionId2 );             // return true for the better result
	//bool RefineFourRegions( int id1, int id2, int id3, int id4 );   // return true for the better result
	
	bool LevelRefinement( int startId, int endId );
	void UpdatePartNeighbor( int partId, double distance );
	
	
	int AddPartition( const int& parentPartId, double left, double bottom, double right, double top, int oldPartId=-1 );
	void UpdatePartition( const int& partId );  // called in "RegineRegions"
	
	bool MakePartition( int partId, int& edgecut, int& new1, int& new2, CUTDIR cutDir=NONE_CUT,
		bool noSubRegion=false, bool considerMovable=true, bool moveHalf=false,  double moveRatio=0.5 );
	void QPPartition( const int& partId );	// 2006-02-05 (donnie)
	void GetCenterOfMass( const int& partId, double& x, double& y ); // 2006-02-05 (donnie)
	
	bool PlacePartition( int partId );
	double GetFixBlockAreaInPartition( const int partId, const double& left, const double& bottom, 
		const double& right, const double& top );
	double GetPartHPWL( int partId );   // Get the HPWL of a single partition.
	CUTDIR GetCutDirForPart( int partId );  // Get the cut-direction for the partition
	void SavePartBlockLocation( int partId );
	void RestorePartBlockLocation( int partId );
	void SavePartBlockList( int partId );
	void RestorePartBlockList( int partId );
	int GetPartitionNumber()    {	return (int)m_partitions.size();    }
	void CalcPartPriority(int partId);
	
	
	
		
		//Dummy Block
	int CreateDummyFixedBlock();    // Create dummy blocks for the 
	// non-placing sites
	int CreateDummyBlock();			// Create dummy blocks to distribute the whitespace
	
	
	
		//Core Shink
	void RestoreCoreRgnShrink(void);
	void ShrinkCoreRgn(double factor);
	void ShrinkCoreWidth(double factor);
	void ShrinkCoreUtil(double util);
	void ShrinkCore(void);

	
	
	// Memory storing partitioning information
	int *part;						// nodes
	int *part_best;					// nodes
	int *moduleWeight;				// node weights
	int *netWeight;					// net weights
	int *eptr;						// edge pointers
	int *eind;						// edge index
	
	vector<CPartition> m_partitions;
	bool NewPartMem(void);
	void DeletePartMem(void);
	
	CParamPlacement param;   // store parameters
	CPlaceDB*	fplan;	//Placement DB


	
	
	

};

#endif
