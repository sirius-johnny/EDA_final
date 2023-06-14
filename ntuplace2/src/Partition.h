#ifndef PARTITION_H
#define PARTITION_H


#include <iostream>
#include <vector>
using namespace std;


enum CUTDIR { NONE_CUT, V_CUT, H_CUT };

class CPartition
{
    public:

	struct BlockPosition    // Struct for saving block positions
	{
	    int id;     // block id
	    float x;   // x
	    float y;   // y
	};

	CPartition( int parentId, float top, float bottom, float left, float right, CUTDIR defaultCutDir );

	bool Check();   // Check region 
	void Print();   // Print the region infomation

	float GetFreeArea()    { return (area - totalFixedArea);                    }
	float GetUsedArea()    { return (totalMovableArea + totalFixedArea);        }
	float GetUtilization() { return (totalMovableArea/(area-totalFixedArea));   }
	float GetDensity()     { return (totalMovableArea + totalFixedArea)/area;   }

	float left, right, top, bottom;
	float area;
	//float usedArea;
	//float freeArea;
	float totalMovableArea;
	float totalFixedArea;
	//float utilization;     // movable / freeArea
	//float density;         // (movable+fixed) / area
	float priority;        // 2005/02/16 The priority of the partitioning.
	vector<int> moduleList;
	vector<int> fixModuleList;
	vector<int> netList;
	vector<int> neighborList;
	vector<BlockPosition> blockPosList;
	vector<int> moduleListBak;
	vector<int> netListBak;

	//CUTDIR cutDir;
	int parentPart;
	int childPart0;
	int childPart1;
	//bool placed;
	int peerPartitionId;

	int bDone;	// 2006-01-10
	
};
#endif

