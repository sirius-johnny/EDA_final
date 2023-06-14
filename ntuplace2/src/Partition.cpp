#include "Partition.h"



CPartition::CPartition( int parentId, float top, float bottom, float left, float right, CUTDIR defaultCutDir )
{
    //cutDir = defaultCutDir;
    parentPart = parentId;
    childPart0 = -1;
	childPart1 = -1;
    peerPartitionId = -1;
    //utilization = -1;
	//placed = false;
	this->left   = left;
	this->right  = right;
	this->top    = top;
	this->bottom = bottom;
	area = (right-left) * (top-bottom);

    bDone = false;

#if 1
    Check();
#endif
}


void CPartition::Print()
{
    cout << "PartInfo: Range(" << left << "," << bottom << ")-(" << right << "," << top << ")\n";
    cout << "            H= " << top-bottom << " V= " << right-left << endl;
    cout << "         Area= " << area << endl;
    cout << "      FixArea= " << totalFixedArea << endl;
    cout << "  MovableArea= " << totalMovableArea << " (" << (int)moduleList.size() << " blocks)\n";
    cout << "     FreeArea= " << GetFreeArea() << endl;
    cout << "         Util= " << GetUtilization() << endl;
    cout << "      Density= " << GetDensity() << endl;
}

bool CPartition::Check()
{
    if( top<bottom )
    {
        cout << "REGION ERROR: top<bottom\n";
        Print();
        exit(-1);
        return false;
    }
    if( right<left )
    {
        cout << "REGION ERROR: right<left\n";
        Print();
        exit(-1);
    }
    return true;
}

