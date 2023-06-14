// 2006-01-10 (donnie)

#ifndef MINCUTCLUSTER_H
#define MINCUTCLUSTER_H

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
class CMinCutCluster
{

    public:
      void CreateCluster( const CPlaceDB& in, CPlaceDB& out );
    
    public:
	
	CMinCutCluster(CPlaceDB& db);
	~CMinCutCluster();
	
	//Partiton
	
	void Init();
	void RecursivePartition();
	bool LevelPartition();
	
	bool LevelRefinement( int startId, int endId );
	void UpdatePartNeighbor( int partId, double distance );
	
	int AddPartition( const int& parentPartId, double left, double bottom, double right, double top, int oldPartId=-1 );
	void UpdatePartition( const int& partId );  // called in "RegineRegions"
	
	bool MakePartition( int partId, int& edgecut, int& new1, int& new2, CUTDIR cutDir=NONE_CUT,
		bool noSubRegion=false, bool considerMovable=true, bool moveHalf=false,  double moveRatio=0.5 );	
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
