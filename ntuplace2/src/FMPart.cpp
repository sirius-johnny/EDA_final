#include "FMPart.h"
#include <cmath>
#include <fstream>

using namespace std;


const int FM_LOOP_COUNT = 3;

const int INCREMENT = 0;
const int DECREMENT = 1;

const int ONE_NODE_MODE = 0;
const int ALL_NODE_MODE = 1;

//test code
//ofstream testfile("test.txt");
//@test code

void CFMPart::ChangeGain( const int &change,
														 const int &mode,
														 const int &net_id, 
														 const int &side,
														 const int *eptr,
														 const int *eind,
														 const int *hewgts,
														 vector<FMNode> &nodes,
														 list<ListNode> &gain_list )
{	
	for( int i = eptr[net_id] ; i < eptr[net_id+1] ; i++ )
	{
		//free node
		if( nodes[ eind[i] ].m_iMoveStep == INT_MAX && 
				nodes[ eind[i] ].m_bFixed != true &&
				( mode == ALL_NODE_MODE || 
				  ( mode == ONE_NODE_MODE && 
				    nodes[ eind[i] ].m_iPart == side 
				  ) 
				)
			)
		{
			if( change == INCREMENT )
				nodes[ eind[i] ].m_iGain += hewgts[net_id];
			else if( change == DECREMENT )
				nodes[ eind[i] ].m_iGain -= hewgts[net_id];
			else
			{
				cerr << "Warning: Undefine Change Method\n";
				return;
			}
			
			if( nodes[ eind[i] ].m_iteListNode != gain_list.end() )
			{
				nodes[ eind[i] ].m_iteListNode->m_iGain = nodes[ eind[i] ].m_iGain;
				
				//find the insertion position
				list<ListNode>::iterator iteFindInsertPosition;
				if( change == INCREMENT )
				{
					for( iteFindInsertPosition = nodes[ eind[i] ].m_iteListNode ;
								iteFindInsertPosition != gain_list.end() ; 
								iteFindInsertPosition++ )
					{
						if( iteFindInsertPosition->m_iGain <= nodes[ eind[i] ].m_iGain )
							break;
					}
				}
				else
				{
					for( iteFindInsertPosition = nodes[ eind[i] ].m_iteListNode ;
								iteFindInsertPosition != gain_list.begin() ;
								iteFindInsertPosition-- )
					{
						if( iteFindInsertPosition->m_iGain >= nodes[ eind[i] ].m_iGain )
						{
							break;
						}
					}
					
					if( iteFindInsertPosition == gain_list.begin() )
					{
						if( iteFindInsertPosition->m_iGain >= nodes[ eind[i] ].m_iGain )
							iteFindInsertPosition--;
					}
					else
					{
						iteFindInsertPosition--;
					}
				}
				
				list<ListNode>::iterator iteTemp = nodes[ eind[i] ].m_iteListNode;
				nodes[ eind[i] ].m_iteListNode = gain_list.insert( iteFindInsertPosition, *iteTemp );


				
				gain_list.erase( iteTemp );
				
			}
			else
			{
				cerr << "Warning: Error m_iteListNode Condition\n";
			}
			
			if( mode == ONE_NODE_MODE )
			{
				return;
			}
		}
	}		
	

														
}


//void CFMPart::ChangeAllGain( const int &change,
//														 const int &net_id, 
//														 const int *eptr,
//														 const int *eind,
//														 vector<FMNode> &nodes,
//														 list<ListNode> &gain_list )
//{	
//	for( int i = eptr[net_id] ; i < eptr[net_id+1] ; i++ )
//	{
//		if( nodes[ eind[i] ].m_iMoveStep == INT_MAX )
//		{
//			if( change == INCREMENT )
//				nodes[ eind[i] ].m_iGain++;
//			else if( change == DECREMENT )
//				nodes[ eind[i] ].m_iGain--;
//			else
//			{
//				cerr << "Warning: Undefine Change Method\n";
//				return;
//			}
//			
//			if( nodes[ eind[i] ].m_iteListNode != gain_list.end() )
//			{
//				nodes[ eind[i] ].m_iteListNode->m_iGain = nodes[ eind[i] ].m_iGain;
//				
//				//find the insertion position
//				list<ListNode>::iterator iteFindInsertPosition;
//				if( change == INCREMENT )
//				{
//					for( iteFindInsertPosition = nodes[ eind[i] ].m_iteListNode ;
//								iteFindInsertPosition != gain_list.end() ; 
//								iteFindInsertPosition++ )
//					{
//						if( iteFindInsertPosition->m_iGain <= nodes[ eind[i] ].m_iGain )
//							break;
//					}
//				}
//				else
//				{
//					for( iteFindInsertPosition = nodes[ eind[i] ].m_iteListNode ;
//								iteFindInsertPosition != gain_list.begin() ;
//								iteFindInsertPosition-- )
//					{
//						if( iteFindInsertPosition->m_iGain >= nodes[ eind[i] ].m_iGain )
//						{
//							break;
//						}
//					}
//					
//					if( iteFindInsertPosition == gain_list.begin() )
//					{
//						if( iteFindInsertPosition->m_iGain >= nodes[ eind[i] ].m_iGain )
//							iteFindInsertPosition--;
//					}
//					else
//					{
//						iteFindInsertPosition--;
//					}
//				}
//				
//				list<ListNode>::iterator iteTemp = nodes[ eind[i] ].m_iteListNode;
//				nodes[ eind[i] ].m_iteListNode = gain_list.insert( iteFindInsertPosition, *iteTemp );
//				gain_list.erase( iteTemp );
//				
//			}
//			else
//			{
//				cerr << "Warning: Error m_iteListNode Condition\n";
//			}
//			
//		}
//	}		
//	
//														
//}														
//
//void CFMPart::ChangeOneGain( const int &change,
//														 const int &net_id, 
//														 const int &side,
//														 const int *eptr,
//														 const int *eind,
//														 vector<FMNode> &nodes,
//														 list<ListNode> &gain_list )
//{	
//	for( int i = eptr[net_id] ; i < eptr[net_id+1] ; i++ )
//	{
//		if( nodes[ eind[i] ].m_iMoveStep == INT_MAX && nodes[ eind[i] ].m_iPart == side )
//		{
//			if( change == INCREMENT )
//				nodes[ eind[i] ].m_iGain++;
//			else if( change == DECREMENT )
//				nodes[ eind[i] ].m_iGain--;
//			else
//			{
//				cerr << "Warning: Undefine Change Method\n";
//				return;
//			}
//			
//			if( nodes[ eind[i] ].m_iteListNode != gain_list.end() )
//			{
//				nodes[ eind[i] ].m_iteListNode->m_iGain = nodes[ eind[i] ].m_iGain;
//				
//				//find the insertion position
//				list<ListNode>::iterator iteFindInsertPosition;
//				if( change == INCREMENT )
//				{
//					for( iteFindInsertPosition = nodes[ eind[i] ].m_iteListNode ;
//								iteFindInsertPosition != gain_list.end() ; 
//								iteFindInsertPosition++ )
//					{
//						if( iteFindInsertPosition->m_iGain <= nodes[ eind[i] ].m_iGain )
//							break;
//					}
//				}
//				else
//				{
//					for( iteFindInsertPosition = nodes[ eind[i] ].m_iteListNode ;
//								iteFindInsertPosition != gain_list.begin() ;
//								iteFindInsertPosition-- )
//					{
//						if( iteFindInsertPosition->m_iGain >= nodes[ eind[i] ].m_iGain )
//						{
//							break;
//						}
//					}
//					
//					if( iteFindInsertPosition == gain_list.begin() )
//					{
//						if( iteFindInsertPosition->m_iGain >= nodes[ eind[i] ].m_iGain )
//							iteFindInsertPosition--;
//					}
//					else
//					{
//						iteFindInsertPosition--;
//					}
//				}
//				
//				list<ListNode>::iterator iteTemp = nodes[ eind[i] ].m_iteListNode;
//				nodes[ eind[i] ].m_iteListNode = gain_list.insert( iteFindInsertPosition, *iteTemp );
//				gain_list.erase( iteTemp );
//				
//			}
//			else
//			{
//				cerr << "Warning: Error m_iteListNode Condition\n";
//			}
//			
//			return;
//		}
//	}		
//	
//														
//}


CFMPart::CFMPart(void)
{
}

CFMPart::~CFMPart(void)
{
}

int CFMPart::Partition( int nvtxs,	// number of vertices
			int nhedges,	// number of hyperedges
			int *vwgts,	// array of vetex weights (size=nvtxs)
			int *eptr,	// i: eind[eptr[i]] ~ eind[eptr[i+1]]
			int *eind,
			int *hewgts,	// array of hyperedge weights (size=nhedges)
			int nparts,	// number of desired partitions
			int ubfactor,	// imbalance factor
			int *options,	
			int *part,	// (RETURN) (size=nvtxs)
			int *edgecut )
{
//********NOTE************
//Nodes may connect to no edges
//********NOTE************

	vector<FMNode> nodes( nvtxs );
	vector<int> NodeCountInPart0( nhedges );
	vector<int> NodeCountInPart1( nhedges );

	//test code
	//testfile << "cut before: " << CalculateCut( nvtxs, 
	//							nhedges,
	//							eptr,
	//							eind,
	//							hewgts,
	//							part 
	//						) << endl;
	//@test code
	
	int Improve = 0;

	// set edges associative to nodes 
	for( int i = 0 ; i < nhedges ; i++ )
	{
		for( int j = eptr[i] ; j < eptr[i+1] ; j++ )
		{
			nodes[eind[j]].m_listEdges.push_back( i );
		}
	}

	
	int GK;
	int totalNodeWgt = 0;
	for( int i = 0 ; i < nvtxs ; i++ )
	{
		totalNodeWgt += vwgts[i];
	}
	//const double tempArea0 = (1.0+(ubfactor/100.0)) / ( 2.0+(ubfactor/100.0)) * (double)totalNodeWgt;
	const int maxArea0 = (int)ceil( (50 + ubfactor ) / 100.0 * totalNodeWgt );
	const int minArea0 = (int)floor( ( 50 - ubfactor ) / 100.0 * totalNodeWgt );
	//**************
	// Main FM Loop
	//**************
	int RedundantLoopCount = 0;
	do
	{


		for( int i = 0 ; i < nvtxs ; i++ )
		{
			if( part[i] == 0 )
			{
				nodes[i].InitNode( 0, false );
			}
			else if( part[i] == 1 )
			{
				nodes[i].InitNode( 1, false );
			}
			else if( part[i] == 2 )
			{
				nodes[i].InitNode( 0, true );
			}
			else if( part[i] == 3 )
			{
				nodes[i].InitNode( 1, true );
			}
			else
			{
				cerr << "Warning: Unknown Node Position\n";
			}
		}

		// set node count for each edge
		for( int i = 0 ; i < nhedges ; i++ )
		{
			NodeCountInPart0[i] = 0;
			NodeCountInPart1[i] = 0;
		}
		
		for( int i = 0 ; i < nhedges ; i++ )
		{
			for( int j = eptr[i] ; j < eptr[i+1] ; j++ )
			{
				if( nodes[eind[j]].m_iPart == 0 )
				{
					NodeCountInPart0[i]++;
				}
				else if( nodes[eind[j]].m_iPart == 1 )
				{
					NodeCountInPart1[i]++;
				}
				else
				{
					cerr << "Warning: Unknown Node Status\n";
				}
			}
		}
		//********************
		// Compute node gains
		//********************
		for( int i = 0 ; i < nvtxs ; i++ )
		{
			for( int j = 0 ; j < (signed)nodes[i].m_listEdges.size() ; j++ )
			{
				int edgeID = nodes[i].m_listEdges[j];
				if( nodes[i].m_iPart == 0 )
				{
					if( NodeCountInPart0[edgeID] == 1 ) // FromSide == 1
					{
						nodes[i].m_iGain += hewgts[edgeID];
					}
					if( NodeCountInPart1[edgeID] == 0 ) // TargetSide == 0
					{
						nodes[i].m_iGain -= hewgts[edgeID];
					}
				}
				else
				{
					if( NodeCountInPart1[edgeID] == 1 ) // FromSide == 1
					{
						nodes[i].m_iGain += hewgts[edgeID];
					}
					if( NodeCountInPart0[edgeID] == 0 ) // TargetSide == 0
					{
						nodes[i].m_iGain -= hewgts[edgeID];
					}
				}
			}
		}//@Compute node gains				
		
	
		//*********************
		// Data Initialization
		//*********************
		GK = 0;
		int MoveStep = 0;
		


		list< ListNode > gain_list;
		for( int i = 0 ; i < nvtxs ; i++ )
		{
			if( !nodes[i].m_bFixed )
			{
				gain_list.push_front( ListNode( i, nodes[i].m_iGain ) );
				gain_list.front().m_ptrFMNode = &nodes[i];
				nodes[i].m_iteListNode = gain_list.begin();
			}
		}


		
		gain_list.sort(Compare());		


		
		int Area0 = 0;
		for( int i = 0 ; i < nvtxs ; i++ )
		{
			if( nodes[i].m_iPart == 0 )
			{
				Area0 += vwgts[i];
			}
		}
		
		vector<int> GainRecord( gain_list.size() );
		
		while( !gain_list.empty() )
		{
			list<ListNode>::iterator iteFindNode;



			//find the maximum movable node which satisfied the area constraint
			for( iteFindNode = gain_list.begin() ; iteFindNode != gain_list.end() ; iteFindNode++ )
			{

				if( nodes[ iteFindNode->m_iIndex ].m_iPart == 0 )
				{
					if( ( Area0 - vwgts[iteFindNode->m_iIndex] ) >= minArea0 )
					{
						Area0 -= vwgts[iteFindNode->m_iIndex];
						break;
					}
				}
				else if( nodes[ iteFindNode->m_iIndex ].m_iPart == 1 )
				{
					if( ( Area0 + vwgts[iteFindNode->m_iIndex] ) <= maxArea0 )
					{
						Area0 += vwgts[iteFindNode->m_iIndex];
						break;
					}
				}
				else
				{
					cerr << "Warning: Data Setup Error. May Cause Unterminate Loop.";
				}

			}//@find the maximum movable node which satisfied the area constraint
			


			if( iteFindNode == gain_list.end() ) // Cannot Move Any Nodes More
			{
				break;
			}
			else // Find a Node to Move
			{


				FMNode &currentNode = nodes[ iteFindNode->m_iIndex ];								
				
				// calculate and record current gain
				GK += currentNode.m_iGain;
				GainRecord[MoveStep] = GK;
				

				// fix the node
				currentNode.m_iMoveStep = MoveStep++;

				//Update Gains
				for( int i = 0 ; i < (signed)currentNode.m_listEdges.size() ; i++ )
				{
					const int &currentEdgeID = currentNode.m_listEdges[i];
					
					if( currentNode.m_iPart == 0 )
					{
						// T(n) == 0
						// Increment gains of all free cells on n
						if( NodeCountInPart1[currentEdgeID] == 0 )
						{
							ChangeGain( INCREMENT,
														 ALL_NODE_MODE,
												   	 currentEdgeID,
												   	 0,
														 eptr,
														 eind,
														 hewgts,
														 nodes,
														 gain_list );
						}//@T(n) == 0
						
						
						// T(n) == 1
						// decrement gain of the only T cell on n
						else if( NodeCountInPart1[currentEdgeID] == 1 )
						{
							ChangeGain( DECREMENT,
														 ONE_NODE_MODE,
														 currentEdgeID,
														 1,
														 eptr,
														 eind,
														 hewgts,
														 nodes,
														 gain_list );
						}//@T(n) == 1
						
						NodeCountInPart0[currentEdgeID]--;
						NodeCountInPart1[currentEdgeID]++;
						
						// F(n) == 0
						// Decrement gains of all free cells on n
						if( NodeCountInPart0[currentEdgeID] == 0 )
						{
							ChangeGain( DECREMENT,
														 ALL_NODE_MODE,
														 currentEdgeID,
														 0,
														 eptr,
														 eind,
														 hewgts,
														 nodes,
														 gain_list );
						}
						// F(n) == 1
						// Increment gain of the only F cell on n
						else if( NodeCountInPart0[currentEdgeID] == 1 )
						{
							ChangeGain( INCREMENT,
														 ONE_NODE_MODE,
														 currentEdgeID,
														 0,
														 eptr,
														 eind,
														 hewgts,
														 nodes,
														 gain_list );							
						}
						
						
						
					}
					else
					{
						// T(n) == 0
						// Increment gains of all free cells on n
						if( NodeCountInPart0[currentEdgeID] == 0 )
						{
							ChangeGain( INCREMENT,
														 ALL_NODE_MODE,
												   	 currentEdgeID,
												   	 0,
														 eptr,
														 eind,
														 hewgts,
														 nodes,
														 gain_list );
						}//@T(n) == 0
						
						
						// T(n) == 1
						// decrement gain of the only T cell on n
						else if( NodeCountInPart0[currentEdgeID] == 1 )
						{
							ChangeGain( DECREMENT,
														 ONE_NODE_MODE,
														 currentEdgeID,
														 0,
														 eptr,
														 eind,
														 hewgts,
														 nodes,
														 gain_list );
						}//@T(n) == 1
						
						NodeCountInPart0[currentEdgeID]++;
						NodeCountInPart1[currentEdgeID]--;
						
						// F(n) == 0
						// Decrement gains of all free cells on n
						if( NodeCountInPart1[currentEdgeID] == 0 )
						{
							ChangeGain( DECREMENT,
														 ALL_NODE_MODE,
														 currentEdgeID,
														 0,
														 eptr,
														 eind,
														 hewgts,
														 nodes,
														 gain_list );
						}
						// F(n) == 1
						// Increment gain of the only F cell on n
						else if( NodeCountInPart1[currentEdgeID] == 1 )
						{
							ChangeGain( INCREMENT,
														 ONE_NODE_MODE,
														 currentEdgeID,
														 1,
														 eptr,
														 eind,
														 hewgts,
														 nodes,
														 gain_list );							
						}						
					}
					
				}//@Update Gains
				
				// Complement the part
				currentNode.m_iPart = currentNode.m_iPart == 0 ? 1 : 0; 
				
				// Kill the ListNode

				iteFindNode->m_ptrFMNode->m_iteListNode = gain_list.end();
				gain_list.erase( iteFindNode );

				
			}//@Find a Node to Move
									
		}//@while( !gain_list.empty() )
						
		//find max GK
		int maxGainStep = -1;
		if( MoveStep == 0 )
		{
			break;
		}
		else
		{
			int i;
			for( i = 0, GK = GainRecord[0], maxGainStep = 0 ; i < MoveStep ; i++ )
			{

				if( GainRecord[i] >= GK )
				{
					maxGainStep = i;
					GK = GainRecord[i];
				}
			}
		}//@find max GK
		




		if( GK >= 0 )
		{
			for( int i = 0 ; i < nvtxs ; i++ )
			{
				if( nodes[i].m_iMoveStep <= maxGainStep )
				{
					if( part[i] == 0 )
					{
						part[i] = 1;
					}
					else if( part[i] == 1 )
					{
						part[i] = 0;
					}
					else
					{
						cerr << "Warning: Wrong Process\n";
					}
				}
			}

		}

		if( GK > 0 )
		{
			RedundantLoopCount = 0;
			Improve += GK;
		}
		else if( GK == 0 )
		{
			RedundantLoopCount++;
		}
	
		

	}while( GK >= 0 && RedundantLoopCount <= FM_LOOP_COUNT );
	//@Main FM Loop
	
	*edgecut = CalculateCut( nvtxs, 
								nhedges,
								eptr,
								eind,
								hewgts,
								part 
							);
	//test code
	//testfile << "cut after: " << *edgecut << endl;
	//if( Improve > 0 )
	//	testfile << "$$$$$$$$$$$$$$$$$$$$$$$$$$$$:Improve: " << Improve << endl;
	//@test code
	return Improve;
	
}

int CFMPart::CalculateCut( const int &nvtxs,
                            const int &nhedges,
                            const int *eptr,
                            const int *eind,
                            const int *hewgts,
                            const int *part )
{
	vector<int> NodeCountInPart0( nhedges );
	vector<int> NodeCountInPart1( nhedges );
	
	// set node count for each edge
	for( int i = 0 ; i < nhedges ; i++ )
	{
		NodeCountInPart0[i] = 0;
		NodeCountInPart1[i] = 0;
	}
	
	for( int i = 0 ; i < nhedges ; i++ )
	{
		for( int j = eptr[i] ; j < eptr[i+1] ; j++ )
		{
			if( part[ eind[j] ] == 0 || part[ eind[j] ] == 2 )
			{
				NodeCountInPart0[i]++;
			}
			else if( part[ eind[j] ] == 1 || part[ eind[j] ] == 3 )
			{
				NodeCountInPart1[i]++;
			}
			else
			{
				cerr << "Warning: Unknown Node Status\n";
			}
		}
	}
	
	int cut_count = 0;
	for( int i = 0 ; i < nhedges ; i++ )
	{
		if( NodeCountInPart0[i] > 0 && NodeCountInPart1[i] > 0 )
			cut_count = cut_count + hewgts[i];
	}
	return cut_count;
}
