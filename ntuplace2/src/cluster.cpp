#include "cluster.h"
#include "fccluster.h"	
#include <iomanip>

void CClustering::declustering(const  CPlaceDB& dbsmall, CPlaceDB& dblarge,bool isPertab )
{
	int clusterMove=0;
	int declusterMove=0;
	for(int i=0; i<(int)dbsmall.m_modules.size(); i++)
	{
		//if(dbsmall.m_modules[i].m_isFixed==false)  // donnie
		{
			++clusterMove;

			if( dbsmall.m_modules[i].m_isFixed ) 
			{
			    if( this->m_hierarchy[i].size() > 1 )
				printf( "WARNING: check dbsmall for fixed terminal %d!\n", i );
			}
			
			for(int j=0; j<this->m_hierarchy[i].size(); j++)
			{
				//assert(dblarge.m_modules[m_hierarchy[i][j]].m_isFixed==false);
				dblarge.m_modules[ m_hierarchy[i][j] ].m_isFixed = dbsmall.m_modules[i].m_isFixed;  // donnie
				if(isPertab==true)
				{				
					double rdx = rand() % (int)(dbsmall.m_modules[i].m_width/2) + (int)(dbsmall.m_modules[i].m_x+dbsmall.m_modules[i].m_width/4);
					double rdy = rand() % (int)(dbsmall.m_modules[i].m_height/2) + (int)(dbsmall.m_modules[i].m_y+dbsmall.m_modules[i].m_height/4);
					//dblarge.SetModuleLocation(m_hierarchy[i][j],rdx,rdy);
					dblarge.MoveModuleCenter(m_hierarchy[i][j],rdx,rdy);
					++declusterMove;
				}
				else
				{
					double rdx=dbsmall.m_modules[i].m_cx;
					double rdy=dbsmall.m_modules[i].m_cy;
					//dblarge.SetModuleLocation(m_hierarchy[i][j],rdx,rdy);
					dblarge.MoveModuleCenter(m_hierarchy[i][j],rdx,rdy); // by donnie
					++declusterMove;
				}
			}
		}
	}
	//cout<<"\n LargeDB move#:"<<clusterMove<<" SmallDB move#:"<<declusterMove<<"\n";

}
void CClustering::clustering(const CPlaceDB& dblarge, CPlaceDB& dbsmall ,int targetClusterNumber, double areaRatio,int ctype)
{
	if(ctype==1) //first choice clustering
	{
		double start=seconds();
		CFCClustering fcc;
		fcc.clustering(dblarge, dbsmall ,m_hierarchy, targetClusterNumber, areaRatio);
		if(showMsg==true)
		{
			cout<<"\n==Type1 Cluster Time:"<<seconds()-start<<" seconds";
		}
	}
	else if(ctype==2) //first choice clustering with physical clustering
	{
		CFCClustering fcc;
		fcc.physicalclustering(dblarge, dbsmall ,m_hierarchy, targetClusterNumber, areaRatio);
	}
	else if(ctype==3)
	{
		double start=seconds();
		CClusterDBFC dbfc;
		dbfc.clustering(dblarge, dbsmall ,m_hierarchy, targetClusterNumber, areaRatio);
		if(showMsg==true)
		{
			cout<<"\n==Type3 Cluster Time:"<<seconds()-start<<" seconds";
		}
	}
	else if(ctype==4)
	{
		//first choice with DB no acc Heuristic
		double start=seconds();
		CClusterDBFC dbfc;
		dbfc.clusteringNH(dblarge, dbsmall ,m_hierarchy, targetClusterNumber, areaRatio);
		if(showMsg==true)
		{
			cout<<"\n==Type4 Cluster Time:"<<seconds()-start<<" seconds";
		}
	}
	else if(ctype==5)
	{
		//best choice clustering 
		double start=seconds();
		CClusterDBBC dbbc;
		dbbc.clustering(dblarge, dbsmall ,m_hierarchy, targetClusterNumber, areaRatio);
		if(showMsg==true)
		{
			cout<<"\n==Type5 Cluster Time:"<<seconds()-start<<" seconds";
		}
		
	}
	else
	{
		CFCClustering fcc;
		fcc.clustering(dblarge, dbsmall ,m_hierarchy, targetClusterNumber, areaRatio);
	}

}
