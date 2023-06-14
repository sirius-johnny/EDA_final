#ifndef _FMPART_
#define _FMPART_

#include <climits>
#include <string>
#include <vector>
#include <iostream>
#include <cstdio>
#include <list>

class ListNode;	
	
class FMNode
{
public:
	//int m_iIndex;
	int m_iPart;
	bool m_bFixed;
	int m_iMoveStep;
	int m_iGain;
	std::list<ListNode>::iterator m_iteListNode;
	std::vector<int> m_listEdges;
	FMNode(void){};
	~FMNode(void){};
	void InitNode( //const int &set_iIndex,
		const int &set_iPart,
		const bool &set_bFixed ) 
	{
	 	m_iPart = set_iPart;
	 	m_bFixed = set_bFixed;
	 	m_iMoveStep = INT_MAX;
	 	m_iGain = 0;
	};
};
	
class ListNode
{
public:
	int m_iIndex;
	int m_iGain;
	FMNode* m_ptrFMNode;
	ListNode( const int &set_iIndex,
						const int &set_iGain ) :
						m_iIndex( set_iIndex ),
						m_iGain( set_iGain ),
						m_ptrFMNode( NULL ) {};
	~ListNode( void ) {};
};
	
class Compare
{
public:
	bool operator()( const ListNode &h1, const ListNode &h2 )
	{
		return h1.m_iGain > h2.m_iGain;
	};
};
	

class CFMPart
{
private:
//	static void ChangeAllGain( const int &change,
//										 const int &net_id, 
//										 const int *eptr,
//										 const int *eind,
//										 std::vector<FMNode> &nodes,
//										 std::list<ListNode> &gain_list );	
//	static void ChangeOneGain( const int &change,
//										 const int &net_id, 
//										 const int &side,
//										 const int *eptr,
//										 const int *eind,
//										 std::vector<FMNode> &nodes,
//										 std::list<ListNode> &gain_list );
	static void ChangeGain( const int &change,
														 const int &mode,
														 const int &net_id,
														 const int &side,
														 const int *eptr,
														 const int *eind,
														 const int *hewgts,
														 std::vector<FMNode> &nodes,
														 std::list<ListNode> &gain_list );
	static int CalculateCut( const int &nvtxs,
													const int &nhedges,
													const int *eptr,
													const int *eind,
													const int *hewgts,
													const int *part );
													
													
public:
	CFMPart(void);
	~CFMPart(void);
	static int Partition( int nvtxs,	// number of vertices
			int nhedges,	// number of hyperedges
			int *vwgts,	// array of vetex weights (size=nvtxs)
			int *eptr,	// i: eind[eptr[i]] ~ eind[eptr[i+1]]
			int *eind,
			int *hewgts,	// array of hyperedge weights (size=nhedges)
			int nparts,	// number of desired partitions
			int ubfactor,	// imbalance factor
			int *options,	
			int *part,	// (RETURN) (size=nvtxs)
			int *edgecut );
};

#endif
