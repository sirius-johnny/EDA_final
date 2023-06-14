#include "fccluster.h"	
#include <iomanip>

struct bestAffinityCompare
{
  bool operator()(const double& s1, const double& s2) const
  {
    return s1>s2;
  }
};
void showDBdata(CPlaceDB * m_pDB)
{
	return;
	int movablePin=0;
	for(unsigned int i=0; i<m_pDB->m_pins.size(); ++i)
	{
		if(m_pDB->m_modules[m_pDB->m_pins[i].moduleId].m_isFixed==false)
		{
			++movablePin;
		}
	}


	double tarea=0;
	int mvCount=0;
	for(int i=0; i<(int)m_pDB->m_modules.size(); i++)
	{
		if(m_pDB->m_modules[i].m_isFixed==false)
		{
			tarea+=m_pDB->m_modules[i].m_area;
			++mvCount;
		}
	}
	double avg_movArea=tarea/mvCount;
//	cout<<"\nAvg area:"<<avg_movArea;
	double sigma;
	for(int i=0; i<(int)m_pDB->m_modules.size(); i++)
	{
		if(m_pDB->m_modules[i].m_isFixed==false)
		{
			sigma+=pow((m_pDB->m_modules[i].m_area-avg_movArea),2);
		//	cout<<" "<<m_pDB->m_modules[i].m_area-avg_movArea;
		}
	}
	sigma=sqrt(sigma/mvCount);
	cout<<"\n***********DB statis:***********";
	cout<<"\nMovalbe Modules#:"<<mvCount;
	cout<<"\nMovalbe Pins#:"<<movablePin;
	cout<<"\nAvg area:"<<avg_movArea;
	cout<<"\nSD:"<<sigma;
	cout<<"\n******end DBdata***";
	

}

bool CClusterDB_ModuleCompare(const CClusterDB_Module &a, const CClusterDB_Module &b) 
{
	if(a.m_netIDs.size()!=b.m_netIDs.size())
	{
		return a.m_netIDs.size() > b.m_netIDs.size();
	}
	else
	{
		return a.m_area < b.m_area;
	}
}
void CFCClustering::declustering(CPlaceDB& dbsmall, CPlaceDB& dblarge,bool isPertab )
{
	int clusterMove=0;
	int declusterMove=0;
	for(int i=0; i<(int)dbsmall.m_modules.size(); i++)
	{
		if(dbsmall.m_modules[i].m_isFixed==false)
		{
			++clusterMove;
			for(unsigned int j=0; j<this->m_hierarchy[i].size(); j++)
			{
				assert(dblarge.m_modules[m_hierarchy[i][j]].m_isFixed==false);
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
					dblarge.MoveModuleCenter(m_hierarchy[i][j],rdx,rdy);	// debug by donnie
					++declusterMove;
				}
			}
		}
	}
	cout<<"\n LargeDB move#:"<<clusterMove<<" SmallDB move#:"<<declusterMove<<"\n";

}

//main use!!
void CFCClustering::clustering(const CPlaceDB& dblarge, CPlaceDB& dbsmall, vector< vector<int> >& hierarchy ,int targetClusterNumber, double areaRatio)
{
	this->showMsg=false;
	if(showMsg)
	cout<<"\nStart FC clustering";
	double timestart=seconds();
	dbsmall=dblarge;

	m_pDB=&dbsmall;
	showDBdata(m_pDB);
	m_isNetExist.clear();
	m_isPinExist.clear();
	m_isModuleExist.clear();
	m_hierarchy.clear();

	m_isNetExist.resize(m_pDB->m_nets.size(),true);
	m_isPinExist.resize(m_pDB->m_pins.size(),true);
	m_isModuleExist.resize(m_pDB->m_modules.size(),true);
	m_hierarchy.resize(m_pDB->m_modules.size());
//	m_netSets.resize(m_pDB->m_modules.size());

	for(int i=0; i<(int)m_hierarchy.size(); i++)
	{
		m_hierarchy[i].push_back(i);

	}



	//find maxClusterArea 
	double tarea=0;
	int count=0;
	for(int i=0; i<(int)m_pDB->m_modules.size(); i++)
	{
		if(m_pDB->m_modules[i].m_isFixed==false)
		{
			tarea+=m_pDB->m_modules[i].m_area;
			++count;
			//for(int j=0; j<(int)m_pDB->m_modules[i].m_netsId.size(); j++)
			//{
			//	m_netSets[i].insert(m_pDB->m_modules[i].m_netsId[j]);
			//}
		}

	}

	count=targetClusterNumber;
//	cout<<"\nTarea:"<<tarea<<" targetN:"<<targetClusterNumber<<" aratio:"<<areaRatio;
	double maxClusterArea=(tarea/count)*areaRatio;

	cout<<"\nStart UP:"<<seconds()-timestart;
	cout<<" target#:"<<targetClusterNumber<<" Max area:"<<maxClusterArea;

	clustering(targetClusterNumber,  maxClusterArea);
	m_isNetExist.clear();
	m_isPinExist.clear();
	m_isModuleExist.clear();
	hierarchy=m_hierarchy;
	m_hierarchy.clear();
	showDBdata(m_pDB);
	cout<<"\nFinish :"<<seconds()-timestart;

}
void CFCClustering::physicalclustering(const CPlaceDB& dblarge, CPlaceDB& dbsmall, vector< vector<int> >& hierarchy ,int targetClusterNumber, double areaRatio)
{
	dbsmall=dblarge;
	m_pDB=&dbsmall;
	m_isNetExist.clear();
	m_isPinExist.clear();
	m_isModuleExist.clear();
	m_hierarchy.clear();

	m_isNetExist.resize(m_pDB->m_nets.size(),true);
	m_isPinExist.resize(m_pDB->m_pins.size(),true);
	m_isModuleExist.resize(m_pDB->m_modules.size(),true);
	m_hierarchy.resize(m_pDB->m_modules.size());
	for(int i=0; i<(int)m_hierarchy.size(); i++)
	{
		m_hierarchy[i].push_back(i);

	}


	//find maxClusterArea
	double tarea=0;
	int count=0;
	for(int i=0; i<(int)m_pDB->m_modules.size(); i++)
	{
		if(m_pDB->m_modules[i].m_isFixed==false)
		{
			tarea+=m_pDB->m_modules[i].m_area;
			++count;

		}

	}
	count=targetClusterNumber;
	double maxClusterArea=(tarea/count)*areaRatio;
	clustering(targetClusterNumber,  maxClusterArea, true); //bool physical==true
	m_isNetExist.clear();
	m_isPinExist.clear();
	m_isModuleExist.clear();
	hierarchy=m_hierarchy;
	m_hierarchy.clear();


}

///////////////////////////////////////////////
//no use!!
//void CFCClustering::clustering(CPlaceDB& dblarge, CPlaceDB& dbsmall ,int targetClusterNumber, double areaRatio)
//{
//	dbsmall=dblarge;
//	m_pDB=&dbsmall;
//	m_isNetExist.clear();
//	m_isPinExist.clear();
//	m_isModuleExist.clear();
//	m_hierarchy.clear();
//
//	m_isNetExist.resize(m_pDB->m_nets.size(),true);
//	m_isPinExist.resize(m_pDB->m_pins.size(),true);
//	m_isModuleExist.resize(m_pDB->m_modules.size(),true);
//	m_hierarchy.resize(m_pDB->m_modules.size());
//	for(int i=0; i<(int)m_hierarchy.size(); i++)
//	{
//		m_hierarchy[i].push_back(i);
//
//	}
//
//
//	//find maxClusterArea
//	double tarea=0;
//	int count=0;
//	for(int i=0; i<(int)m_pDB->m_modules.size(); i++)
//	{
//		if(m_pDB->m_modules[i].m_isFixed==false)
//		{
//			tarea+=m_pDB->m_modules[i].m_area;
//			++count;
//
//		}
//
//	}
//	count=targetClusterNumber;
//	double maxClusterArea=(tarea/count)*areaRatio;
//	clustering(targetClusterNumber,  maxClusterArea);
//	m_isNetExist.clear();
//	m_isPinExist.clear();
//	m_isModuleExist.clear();
//}
/////////////////////////////////////////////////////////////

void CFCClustering::clustering(int targetClusterNumber, double maxClusterArea,bool physical)
{
	if(showMsg)
		cout<<"\nStart FC clustering";
	double timestart=seconds();	

	int numOfClusters=this->countMoveModules();
	if(showMsg)
		cout<<"\n# of Moveable:"<<numOfClusters;
	bool stop=false;
	int noc=numOfClusters;
	while(numOfClusters>targetClusterNumber && stop==false)
	{
		bool mergeFlag=false;
		//sort by vertex degree (same as mPL5's procedure)

		double findnodes=seconds();
		vector<int> nodes;
		nodes.reserve(m_pDB->m_modules.size());
		for(int i=0; i<(int)m_pDB->m_modules.size(); ++i)
		{
			if(m_pDB->m_modules[i].m_isFixed==false)
			{
				if(m_pDB->m_modules[i].m_area<maxClusterArea)
				{
					nodes.push_back(i);
				}
			}
		}
		sort(nodes.begin(),nodes.end(),CFCClusteringVertexCompare(*m_pDB));
		if(showMsg)
		{
			cout<<"\nsort by degree:"<<seconds()-findnodes;
		}
		findnodes=seconds();
		double chooseMaxAtime=0;
		double margeTime=0;
		//for all movable module, merge with heightest affinity module
		for(int i=0; i<(int)nodes.size(); ++i)
		{
			assert(m_pDB->m_modules[nodes[i]].m_isFixed==false && m_pDB->m_modules[nodes[i]].m_area<maxClusterArea);
			if( m_isModuleExist[nodes[i]]==true)
			{
				double temptime=seconds();
				int mergeID=chooseMaxAffinity(nodes[i],maxClusterArea,physical);
				chooseMaxAtime+=seconds()-temptime;

				temptime=seconds();
				if(mergeID!=-1)
				{
					this->mergeModule(nodes[i],mergeID);
					mergeFlag=true;
					--numOfClusters;
					if(numOfClusters==targetClusterNumber)
					{
						break;
					}
				}
				margeTime+=seconds()-temptime;

			}
		}
		int numAfterMerge=countMoveModules();
		if(showMsg)
		{
			cout<<"\nlook around:"<<seconds()-findnodes<<" , choose:"<<chooseMaxAtime<<" , merge:"<<margeTime<<", #ofMove:"<<numAfterMerge;
		}
		if(mergeFlag==false)
		{
			stop=true;
		}
		double decreasingRate=1-(double)numAfterMerge/(double)noc;
		noc=numAfterMerge;
		if(decreasingRate<0.05)
		{
			stop=true;
		}
	}
	if(showMsg)
	{
		cout<<"\nFinish Cluster:"<<seconds()-timestart;
	}
	removeExcessModulePin();
	if(showMsg)
	{
		cout<<"\nTotal Finish:"<<seconds()-timestart;
	}
}



void CFCClustering::removeExcessModulePin()
{
	//--------------------------------------------------------------------
	//remove excess pin,module from placeDB and update ids
	//--------------------------------------------------------------------

	//---------------
	//1. pins
	//---------------
	vector<int> pinIDmap;
	pinIDmap.resize(m_pDB->m_pins.size(),-1);
	vector<Pin> newPins;
	int newPinCount=0;
	for(int i=0; i<(int)this->m_isPinExist.size(); i++)
	{
		if(m_isPinExist[i]==true)
			++newPinCount;
	}
	newPins.reserve(newPinCount);
	for(int i=0; i<(int)m_pDB->m_pins.size(); i++)
	{
		if(m_isPinExist[i]==true)
		{
			pinIDmap[i]=(int)newPins.size();
			newPins.push_back(m_pDB->m_pins[i]);
		}
	}
	m_pDB->m_pins=newPins;

	//update nets, map the old pinID to new pinID
	for(int i=0; i<(int)m_pDB->m_nets.size(); i++)
	{
		for(int j=0; j<(int)m_pDB->m_nets[i].size(); j++)
		{
			assert(this->m_isPinExist[m_pDB->m_nets[i][j]]==true);
			assert(pinIDmap[m_pDB->m_nets[i][j]]!=-1);
			m_pDB->m_nets[i][j]=pinIDmap[m_pDB->m_nets[i][j]];
		}
	}
	//update modules , map the old pinID to new pinID
	for(int i=0; i<(int)m_pDB->m_modules.size(); i++)
	{
		if(m_isModuleExist[i]==true)
		{
			for(int j=0; j<(int)m_pDB->m_modules[i].m_pinsId.size(); j++)
			{
				assert(this->m_isPinExist[m_pDB->m_modules[i].m_pinsId[j]]==true);
				assert(pinIDmap[m_pDB->m_modules[i].m_pinsId[j]]!=-1);
				m_pDB->m_modules[i].m_pinsId[j]=pinIDmap[m_pDB->m_modules[i].m_pinsId[j]];
			}
		}
	}
	//---------------
	//2. modules
	//---------------
	vector<Module> newModules;
	vector< vector<int> > newHierarchy;
	int newModuleCount=0;
	//for debug
//	int hiCount=0;
	for(int i=0; i<(int)this->m_isModuleExist.size(); i++)
	{
		if(m_isModuleExist[i]==true)
		{
			++newModuleCount;
//			hiCount+=this->m_hierarchy[i].size();
		}
	}
//	cout<<"\n # of Hierarchy before trim module:"<<hiCount<<"\n";
	newModules.reserve(newModuleCount);
	newHierarchy.reserve(newModuleCount);
	for(int i=0; i<(int)m_pDB->m_modules.size(); i++)
	{
		if(m_isModuleExist[i]==true)
		{
			int newModuleID=(int)newModules.size();

			//update moduleId of the pin belongs to this module
			for(int j=0; j<(int)m_pDB->m_modules[i].m_pinsId.size(); j++)
			{
				assert(m_pDB->m_modules[i].m_pinsId[j]<(int)m_pDB->m_pins.size());
				m_pDB->m_pins[m_pDB->m_modules[i].m_pinsId[j]].moduleId=newModuleID;
			}
			newModules.push_back(m_pDB->m_modules[i]);
			newHierarchy.push_back(m_hierarchy[i]);

		}
	}
	this->m_hierarchy=newHierarchy;
	m_pDB->m_modules=newModules;
	m_pDB->m_nModules=(int)m_pDB->m_modules.size();
	m_pDB->m_nPins=(int)m_pDB->m_pins.size();

	//hiCount=0;
	//for(int i=0; i<(int)m_hierarchy.size(); i++)
	//{
	//	hiCount+=this->m_hierarchy[i].size();

	//}
	//cout<<"\n # of Hierarchy after trim module:"<<hiCount<<"\n";
}
void CFCClustering::mergeModule(int mID1, int mID2)
{
	m_pDB->m_modules[mID1].m_isCluster=true;
	//1.move pins of module2 to module1
	for(int i=0; i<(int)m_pDB->m_modules[mID2].m_pinsId.size(); i++)
	{
		//m_pDB->m_modules[mID1].m_pinsId.push_back(m_pDB->m_modules[mID2].m_pinsId[i]);
		m_pDB->m_pins[m_pDB->m_modules[mID2].m_pinsId[i]].moduleId=mID1;
		//m_pDB->m_pins[m_pDB->m_modules[mID2].m_pinsId[i]].xOff=0;
		//m_pDB->m_pins[m_pDB->m_modules[mID2].m_pinsId[i]].yOff=0;		
	}

	//2.recalc widht,height,area of module 1
	double a1=m_pDB->m_modules[mID1].m_area;
	double a2=m_pDB->m_modules[mID2].m_area; 
	double ar1=m_pDB->m_modules[mID1].m_width/m_pDB->m_modules[mID1].m_height; //find aspect ratio
	double ar2=m_pDB->m_modules[mID2].m_width/m_pDB->m_modules[mID2].m_height;
	double newar=(a1*ar1+a2*ar2)/(a1+a2);

	// TEST by donnie
	newar = 1.0;	// square
	

	m_pDB->m_modules[mID1].m_area=a1+a2;
	m_pDB->m_modules[mID1].m_width=sqrt(newar*m_pDB->m_modules[mID1].m_area);
	m_pDB->m_modules[mID1].m_height=m_pDB->m_modules[mID1].m_area/m_pDB->m_modules[mID1].m_width;

	//determine new loaction of mID1 (center of gravity of mID1 and mID2 )
	double new_cx=(m_pDB->m_modules[mID1].m_cx*a1+m_pDB->m_modules[mID2].m_cx*a2)/(a1+a2);
	double new_cy=(m_pDB->m_modules[mID1].m_cy*a1+m_pDB->m_modules[mID2].m_cy*a2)/(a1+a2);	
	m_pDB->MoveModuleCenter(mID1, new_cx, new_cy);

	//3.build the hierarchy
	int tempsize=m_hierarchy[mID1].size();
	m_hierarchy[mID1].resize(tempsize+m_hierarchy[mID2].size());
	for(int i=0; i<(int)m_hierarchy[mID2].size(); i++)
	{
		m_hierarchy[mID1][tempsize+i]=m_hierarchy[mID2][i];		
	}
	m_isModuleExist[mID2]=false;

	//4.rebuild netsID and pinsID of moudle 1
	//m_netSets[mID1].insert(m_netSets[mID2].begin(),m_netSets[mID2].end());
	//m_netSets[mID2].clear();

//	set<int> netSet=m_netSets[mID1];
	set<int> netSet;

	for(int i=0; i<(int)m_pDB->m_modules[mID1].m_netsId.size(); i++)
	{
		netSet.insert(m_pDB->m_modules[mID1].m_netsId[i]);
	}
	for(int i=0; i<(int)m_pDB->m_modules[mID2].m_netsId.size(); i++)
	{
		netSet.insert(m_pDB->m_modules[mID2].m_netsId[i]);
	}

	int netsum=(int)m_pDB->m_modules[mID1].m_netsId.size()+(int)m_pDB->m_modules[mID2].m_netsId.size();
	int pinsum=(int)m_pDB->m_modules[mID1].m_pinsId.size()+(int)m_pDB->m_modules[mID2].m_pinsId.size();

	m_pDB->m_modules[mID1].m_netsId.clear();
	m_pDB->m_modules[mID1].m_netsId.reserve(netsum);

	m_pDB->m_modules[mID1].m_pinsId.clear();
	m_pDB->m_modules[mID1].m_pinsId.reserve(pinsum);

	set<int>::iterator it;
	for(it=netSet.begin(); it!=netSet.end(); ++it)
	{
		int netsize=this->getNetSize(*it);

		if(netsize==1) // inside net
		{
			this->m_isNetExist[*it]=false;
			for(int i=0; i<(int)m_pDB->m_nets[*it].size(); ++i)
			{
				this->m_isPinExist[m_pDB->m_nets[*it][i]]=false;
				assert(m_pDB->m_pins[m_pDB->m_nets[*it][i]].moduleId==mID1);
			}
			m_pDB->m_nets[*it].clear(); //clear the pinsID vector of the net
		}
		else //remove excess pin, if any
		{
			m_pDB->m_modules[mID1].m_netsId.push_back(*it);

			bool appear=false;
			vector<int> newNet;
			newNet.reserve(m_pDB->m_nets[*it].size());
			for(int i=0; i<(int)m_pDB->m_nets[*it].size(); ++i)
			{
				if(m_pDB->m_pins[m_pDB->m_nets[*it][i]].moduleId==mID1)
				{
					if(appear==false)
					{
						newNet.push_back(m_pDB->m_nets[*it][i]);
						m_pDB->m_modules[mID1].m_pinsId.push_back(m_pDB->m_nets[*it][i]);
						m_pDB->m_pins[m_pDB->m_nets[*it][i]].xOff=0;
						m_pDB->m_pins[m_pDB->m_nets[*it][i]].yOff=0;
						m_pDB->m_pins[m_pDB->m_nets[*it][i]].absX=m_pDB->m_modules[mID1].m_cx;
						m_pDB->m_pins[m_pDB->m_nets[*it][i]].absY=m_pDB->m_modules[mID1].m_cy;
						appear=true;
					}
					else  //appear is true => already 1 pin in module 1 (the new clustered module) =>remove excess pin
					{
						this->m_isPinExist[m_pDB->m_nets[*it][i]]=false;
					}
				}
				else
				{
					newNet.push_back(m_pDB->m_nets[*it][i]);
				}
			}
			m_pDB->m_nets[*it]=newNet;

		}
	
	}


}
void CFCClustering::getModuleNeighbors(int mID, set<int>& neighborSet)
{
	neighborSet.clear();
	//for all nets contain this module
	for(int j=0; j<(int)m_pDB->m_modules[mID].m_netsId.size(); j++)
	{
		int netid=m_pDB->m_modules[mID].m_netsId[j];

		//for all pins in this net
		for(int k=0; k<(int)m_pDB->m_nets[netid].size(); k++)
		{
			int pinid=m_pDB->m_nets[netid][k];

			if(m_pDB->m_pins[pinid].moduleId!=mID)
			{
				if(m_pDB->m_modules[m_pDB->m_pins[pinid].moduleId].m_isFixed==false)
				{
					neighborSet.insert(m_pDB->m_pins[pinid].moduleId);

				}
			}
		}
	}	
}

int CFCClustering::chooseMaxAffinity(int mID, double maxClusterArea, bool physical)
{
	map<int,double> affinityMap;

	//for all net belonging to the module, find its affinity
	for(int j=0; j<(int)m_pDB->m_modules[mID].m_netsId.size(); j++)
	{
		int netid=m_pDB->m_modules[mID].m_netsId[j];
		int netSize=getNetSize(netid);

		//for all pins in this net
		for(int k=0; k<(int)m_pDB->m_nets[netid].size(); k++)
		{
			int pinid=m_pDB->m_nets[netid][k];

			if(m_pDB->m_pins[pinid].moduleId!=mID)
			{
				if(m_pDB->m_modules[m_pDB->m_pins[pinid].moduleId].m_isFixed==false)
				{
					int mid=m_pDB->m_pins[pinid].moduleId;
					assert(netSize>1);


					//----------------
					// calc affinity
					//---------------
					double affinity=0;
					if(physical==false)
					{
						affinity=1/( (netSize-1)*(m_pDB->m_modules[mID].m_area + m_pDB->m_modules[mid].m_area));
					}
					else //physical clustering
					{
						double dist=1+sqrt( pow(m_pDB->m_modules[mID].m_cx-m_pDB->m_modules[mid].m_cx,2)+pow(m_pDB->m_modules[mID].m_cy-m_pDB->m_modules[mid].m_cy,2));
						affinity=1/( (netSize-1)*(m_pDB->m_modules[mID].m_area + m_pDB->m_modules[mid].m_area)*dist);
					}

					if(affinityMap.find(mid)!=affinityMap.end()) //the module exists in other net
					{
						affinityMap[mid]+=affinity;						
					}
					else
					{
						affinityMap.insert(pair<int,double>(mid,affinity));
					}

				}
			}
		}
	}


	//--------------------------------------------------------------------------------------------
	// find the max affinity without break the constraint
	//--------------------------------------------------------------------------------------------
	map<int,double>::iterator it;
	int target=-1;
	double maxAffinity=-1;

	for(it=affinityMap.begin(); it!=affinityMap.end(); ++it)
	{
		if(it->second>maxAffinity)
		{
			double combineArea=m_pDB->m_modules[mID].m_area + m_pDB->m_modules[it->first].m_area;
			if(combineArea<= maxClusterArea)
			{
				target=it->first;
				maxAffinity=it->second;
			}
		}

	}

	return target; //if target==-1, means all neighbor cells are larger than maxarea. Don't perform clustering on this cell

}


int CFCClustering::getNetSize(int netID)
{

	set<int> moduleSet;

	//for all pins in this net
	for(int k=0; k<(int)m_pDB->m_nets[netID].size(); k++)
	{
		moduleSet.insert(m_pDB->m_pins[m_pDB->m_nets[netID][k]].moduleId);		
	}

	return (int)moduleSet.size();
}

int CFCClustering::countMoveModules()
{
	int i=0;
	for(int j=0; j<(int)m_pDB->m_modules.size(); j++)
	{
		if(m_pDB->m_modules[j].m_isFixed==false && m_isModuleExist[j]==true)
			++i;
	}
	return i;
}

/////////////////////////////////////////////////
// CClusterDB
////////////////////////////////////////////////

void CClusterDB::placeDbIn(CPlaceDB& db)
{
	this->m_pDB=&db;
	m_movableCount=0;
	//set up modules
	m_modules.resize(db.m_modules.size());
	for(unsigned int i=0; i<db.m_modules.size(); ++i)
	{
		m_modules[i].m_mID=i;
		m_modules[i].m_isExist=true;
		m_modules[i].m_cx=db.m_modules[i].m_cx;
		m_modules[i].m_cy=db.m_modules[i].m_cy;
		m_modules[i].m_area=db.m_modules[i].m_area;
		m_modules[i].m_hierarchy.resize(1,i);
		if(db.m_modules[i].m_isFixed==false)
		{
			++m_movableCount;
		}
	}

	//set up nets
	m_nets.resize(db.m_nets.size());
	for(unsigned int i=0; i<db.m_nets.size(); ++i)
	{
		for(unsigned int j=0; j<db.m_nets[i].size(); ++j)
		{
			int mid=db.m_pins[ db.m_nets[i][j] ].moduleId;
			//if(db.m_modules[mid].m_isFixed==false)
			//{
				m_nets[i].insert( mid );
				this->m_modules[mid].m_netIDs.insert(i);
			//}
		}
	}


//	////////////////////////////////////////////////
//	//build neighborMaps
//	////////////////////////////////////////////////
////	m_neighborMaps.resize(db.m_modules.size() );
//	for(unsigned int i=0; i<m_modules.size(); ++i)
//	{
//		set<int>::iterator netIter;
//		for(netIter=m_modules[i].m_netIDs.begin(); netIter!=m_modules[i].m_netIDs.end(); ++netIter)
//		{
//			set<int>::iterator moduleIter;
//			for(moduleIter=m_nets[*netIter].begin(); moduleIter!=m_nets[*netIter].end(); ++moduleIter)
//			{
//				if(isModuleFix(*moduleIter)==false)
//				{
//					m_modules[i].m_neighborMaps[*moduleIter].insert(*netIter);	
//				}
//				
//			}
//		}
//	}
//
//	////////////////////////////////////////////////////
//	//build initial affinity maps
//	////////////////////////////////////////////////////
//	m_affinityMap.resize(m_neighborMaps.size(),0);
//	for(unsigned int i=0; i<m_affinityMap.size(); ++i)
//	{
//		if(db.m_modules[i].m_isFixed==false)
//		{
//			map<int,set<int> >::iterator it;
//			for(it=m_neighborMaps[i].begin(); it!=m_neighborMaps[i].end(); ++it)
//			{
//				if(db.m_modules[it->first].m_isFixed==false)
//				{
//					double affinity=0;
//					set<int>::iterator netIter;
//					for(netIter=it->second.begin() ; netIter!=it->second.end(); ++netIter)
//					{
//						//count affinity
//						assert(m_nets[*netIter].size()>1);
//						affinity+=1/( (m_nets[*netIter].size()-1)*(m_modules[i].m_area + m_modules[it->first].m_area));
//					}
//					m_affinityMap[i][it->first]=affinity;
//
//				}
//			}
//		}		
//	}
}
//void CClusterDB::placeDbOut( vector< vector<int> >& hierarchy)
//modify m_pDB's module data (this function is called after clustering is finished)

void CClusterDB::placeDbOut()
{

	m_isNetExist.resize(m_pDB->m_nets.size(),true);
	m_isPinExist.resize(m_pDB->m_pins.size(),true);
	m_isModuleExist.resize(m_pDB->m_modules.size(),true);
	m_hierarchy.clear();
	m_hierarchy.resize(m_pDB->m_modules.size());

	//clear pseude net (to bypass small fixed macros)
	this->m_nets.resize(m_pDB->m_nets.size());


	//--------------------------------------------------------------------
	// 1. update hierarchy, move pins to left module
	//--------------------------------------------------------------------
	for(int i=0; i<(int)m_modules.size(); ++i)
	{
		if(m_modules[i].m_isExist==true)
		{
			//update module width and height
			if(this->isModuleFix(i)==false && (m_modules[i].m_hierarchy.size()>1))
			{
				if(m_pDB->m_modules[i].m_area==m_modules[i].m_area)
				{
					cerr<<"\n i"<<i<<" dbArea:"<<m_pDB->m_modules[i].m_area<<" cdbArea:"<<m_modules[i].m_area<<" hiera"<<m_hierarchy[i].size();
				}
				assert(m_pDB->m_modules[i].m_area!=m_modules[i].m_area);
				m_pDB->m_modules[i].m_area=m_modules[i].m_area;
				double newar = 1.0;	// square
				m_pDB->m_modules[i].m_width=sqrt(newar*m_modules[i].m_area);
				m_pDB->m_modules[i].m_height=m_modules[i].m_area/m_pDB->m_modules[i].m_width;
			}
			m_hierarchy[i]=m_modules[i].m_hierarchy;
			for(int j=0; j<(int)m_modules[i].m_hierarchy.size(); ++j)
			{
				
				if(m_modules[i].m_hierarchy[j]!=i)
				{
					assert(this->isModuleFix(i)==false);
					for(unsigned int k=0; k<m_pDB->m_modules[m_modules[i].m_hierarchy[j]].m_pinsId.size(); ++k)
					{
						int pid=m_pDB->m_modules[m_modules[i].m_hierarchy[j]].m_pinsId[k];
						m_pDB->m_pins[pid].moduleId=i;
						m_pDB->m_pins[pid].xOff=0;
						m_pDB->m_pins[pid].yOff=0;
						m_pDB->m_pins[pid].absX=m_modules[i].m_cx;
						m_pDB->m_pins[pid].absY=m_modules[i].m_cy;

					}

				}
			}
			if(m_hierarchy[i].size()>1)
			{
				m_pDB->m_modules[i].m_isCluster=true;
				assert(this->isModuleFix(i)==false);
				for(unsigned int k=0; k<m_pDB->m_modules[i].m_pinsId.size(); ++k)
				{
					int pid=m_pDB->m_modules[i].m_pinsId[k];
					m_pDB->m_pins[pid].xOff=0;
					m_pDB->m_pins[pid].yOff=0;
					m_pDB->m_pins[pid].absX=m_modules[i].m_cx;
					m_pDB->m_pins[pid].absY=m_modules[i].m_cy;

				}

			
			}
		}
		else
		{
			m_isModuleExist[i]=false;
		}
	}

	//--------------------------------------------------------------------
	// 2. update m_pDB->m_net, remove unnecessary net and unnecessary pin
	//--------------------------------------------------------------------
	for(unsigned int i=0; i<this->m_nets.size(); ++i)
	{
		if(m_nets[i].size()<2)
		{
			m_isNetExist[i]=false;
			for(unsigned int j=0; j<m_pDB->m_nets[i].size(); ++j)
			{
				m_isPinExist[m_pDB->m_nets[i][j]]=false;  //remove pins
			}
			m_pDB->m_nets[i].clear();
		}
		else //remove redundant pin (prevent 1 net has more than 1 pin in a module)
		{
			vector<int> newPinList;
			newPinList.reserve(m_pDB->m_nets[i].size());
			set<int> moduleSet;
			set<int>::iterator it;

			//for all pins in nets[i]
			for(unsigned int j=0; j<m_pDB->m_nets[i].size(); ++j)
			{
				it=moduleSet.find(m_pDB->m_pins[m_pDB->m_nets[i][j]].moduleId);
				if(it!=moduleSet.end()) //already 1 pin in this module
				{
					m_isPinExist[m_pDB->m_nets[i][j]]=false;
				}
				else
				{
					moduleSet.insert(m_pDB->m_pins[m_pDB->m_nets[i][j]].moduleId);
					newPinList.push_back(m_pDB->m_nets[i][j]);
				}

			}
			m_pDB->m_nets[i]=newPinList;
		}
	}


	//--------------------------------------------------------------------
	//3. remove excess pin,module from placeDB and update ids
	//--------------------------------------------------------------------

	//---------------
	//3.1. pins
	//---------------
	vector<int> pinIDmap;
	pinIDmap.resize(m_pDB->m_pins.size(),-1);
	vector<Pin> newPins;
	int newPinCount=0;
	for(int i=0; i<(int)this->m_isPinExist.size(); i++)
	{
		if(m_isPinExist[i]==true)
			++newPinCount;
	}
	newPins.reserve(newPinCount);
	for(int i=0; i<(int)m_pDB->m_pins.size(); i++)
	{
		if(m_isPinExist[i]==true)
		{
			pinIDmap[i]=(int)newPins.size();
			newPins.push_back(m_pDB->m_pins[i]);
		}
	}
	m_pDB->m_pins=newPins;

	//update nets, map the old pinID to new pinID
	for(int i=0; i<(int)m_pDB->m_nets.size(); i++)
	{
		for(int j=0; j<(int)m_pDB->m_nets[i].size(); j++)
		{
			assert(this->m_isPinExist[m_pDB->m_nets[i][j]]==true);
			assert(pinIDmap[m_pDB->m_nets[i][j]]!=-1);
			m_pDB->m_nets[i][j]=pinIDmap[m_pDB->m_nets[i][j]];
		}
	}
	//--------------------------------------------------------------------
	// 3.2. update m_pDB->m_modules[mid].m_netsId/m_pinsId
	//--------------------------------------------------------------------
	for(int i=0; i<(int)m_pDB->m_modules.size(); ++i)
	{
		m_pDB->m_modules[i].m_netsId.clear();
		m_pDB->m_modules[i].m_pinsId.clear();		
	}
	for(int i=0; i<(int)m_pDB->m_nets.size(); ++i)
	{
		for(unsigned int j=0; j<m_pDB->m_nets[i].size(); ++j)
		{
			int mid=m_pDB->m_pins[m_pDB->m_nets[i][j]].moduleId;
			m_pDB->m_modules[mid].m_netsId.push_back(i);
			m_pDB->m_modules[mid].m_pinsId.push_back(m_pDB->m_nets[i][j]);	
		}
	}
	//---------------
	//3.3 clear redundant modules
	//---------------
	vector<Module> newModules;
	vector< vector<int> > newHierarchy;
	int newModuleCount=0;
	//for debug
//	int hiCount=0;
	for(int i=0; i<(int)this->m_isModuleExist.size(); i++)
	{
		if(m_isModuleExist[i]==true)
		{
			++newModuleCount;
//			hiCount+=this->m_hierarchy[i].size();
		}
	}
//	cout<<"\n # of Hierarchy before trim module:"<<hiCount<<"\n";
	newModules.reserve(newModuleCount);
	newHierarchy.reserve(newModuleCount);
	for(int i=0; i<(int)m_pDB->m_modules.size(); i++)
	{
		if(m_isModuleExist[i]==true)
		{
			int newModuleID=(int)newModules.size();

			//update moduleId of the pin belongs to this module
			for(int j=0; j<(int)m_pDB->m_modules[i].m_pinsId.size(); j++)
			{
				assert(m_pDB->m_modules[i].m_pinsId[j]<(int)m_pDB->m_pins.size());
				m_pDB->m_pins[m_pDB->m_modules[i].m_pinsId[j]].moduleId=newModuleID;
			}
			newModules.push_back(m_pDB->m_modules[i]);
			newHierarchy.push_back(m_hierarchy[i]);

		}
	}
	m_hierarchy=newHierarchy;
	m_pDB->m_modules=newModules;
	m_pDB->m_nModules=(int)m_pDB->m_modules.size();
	m_pDB->m_nPins=(int)m_pDB->m_pins.size();
}


void CClusterDB::showNetProfile()
{
	vector<int> nets;
	nets.resize(10,0);
	int netCount=0;
	for(int i=0; i<(int)this->m_nets.size(); ++i)
	{
		if(m_nets[i].size()>1)
		{
			++netCount;
			if(m_nets[i].size()<=10)
			{
				++nets[m_nets[i].size()-2];
			}
			else
			{
				++nets[9];
			}
		}
	}
	cout<<"\n========Net DATA:====== ActiveNET:"<<netCount<<"\n";
	for(unsigned int i=0; i<nets.size(); ++i)
	{
		cout<<i+2<<" degree Net:"<<nets[i]<<" || ";
		if(i==3)
			cout<<"\n";
		if(i==7)
			cout<<"\n";
	}
	cout<<"\n============\n";
}

bool CClusterDB::isModuleFix( int mID)
{
	return m_pDB->m_modules[mID].m_isFixed;

}
int CClusterDB::getMovableCount()
{
	return m_movableCount;
}

void CClusterDB::mergeModule(int mID1, int mID2)
{
	assert(m_pDB->m_modules[mID1].m_isFixed==false && m_pDB->m_modules[mID2].m_isFixed==false);
	m_modules[mID2].m_isExist=false;
	double a1=m_modules[mID1].m_area;
	double a2=m_modules[mID2].m_area; 

	//determine new loaction of mID1 (center of gravity of mID1 and mID2 )
	m_modules[mID1].m_cx=(m_modules[mID1].m_cx*a1+m_modules[mID2].m_cx*a2)/(a1+a2);
	m_modules[mID1].m_cy=(m_modules[mID1].m_cy*a1+m_modules[mID2].m_cy*a2)/(a1+a2);	

	m_modules[mID1].m_area=a1+a2;
	m_modules[mID1].m_hierarchy.insert(m_modules[mID1].m_hierarchy.end(),m_modules[mID2].m_hierarchy.begin(),m_modules[mID2].m_hierarchy.end());
	m_modules[mID1].m_netIDs.insert(m_modules[mID2].m_netIDs.begin(),m_modules[mID2].m_netIDs.end());

	//clear the ID of module2 from all its nets, add mID1 to those nets
	set<int>::iterator it;
	for(it=m_modules[mID2].m_netIDs.begin(); it!=m_modules[mID2].m_netIDs.end(); ++it)
	{
		assert(m_nets[*it].find(mID2)!=m_nets[*it].end());

		m_nets[*it].erase(mID2);
		m_nets[*it].insert(mID1);

		//remove the internal net from module 1's net set
		if(m_nets[*it].size()==1)
		{
			m_modules[mID1].m_netIDs.erase(*it);
		}
	}

	////remove the internal net from module 1's net set	
	//set<int> tempset=m_modules[mID1].m_netIDs;
	//for(it=tempset.begin(); it!=tempset.end(); ++it)
	//{
	//	if(this->m_nets[*it].size()<=1)
	//	{
	//		m_modules[mID1].m_netIDs.erase(*it);
	//	}
	//}

	--m_movableCount;


}


/////////////////////////////////////////////////
// CClusterDBFC
////////////////////////////////////////////////
void CClusterDBFC::clustering(const CPlaceDB& dblarge, CPlaceDB& dbsmall, vector< vector<int> >& hierarchy,int targetClusterNumber, double areaRatio)
{


	bool showMsg=false;
	if(showMsg)
	{
		cout<<"\nStart FC clustering:target#:"<<targetClusterNumber;
		

	}
	double timestart=seconds();
	dbsmall=dblarge;

	m_pDB=&dbsmall;
	m_ClusterDB.placeDbIn(*m_pDB);



	//find maxClusterArea 
	double maxClusterArea=m_ClusterDB.findMaxClusterArea(targetClusterNumber, areaRatio);

	//--------------------------------------------------------------------------------
	//main clustering program
	//--------------------------------------------------------------------------------


	//add bypass net to prevnet clustering saturation
	addBypassNet();
	if(showMsg)
	{
		cout<<"\nDB in, add bypass net:"<<seconds()-timestart;
		timestart=seconds();
		cout<<"\n# of Moveable:"<<m_ClusterDB.getMovableCount();
	//	m_ClusterDB.showNetProfile();
	}
	//-------------------
	// 1. merge 2-pin net
	//-------------------
	for(unsigned int i=0; i<m_ClusterDB.m_nets.size(); ++i)
	{
		if(m_ClusterDB.m_nets[i].size()==2)
		{
			set<int>::iterator it;
			it=m_ClusterDB.m_nets[i].begin();
			int mid1=*it;
			++it;
			assert(it!=m_ClusterDB.m_nets[i].end());
			int mid2=*it;
			assert((mid1<(int)m_ClusterDB.m_modules.size()) && (mid2<(int)m_ClusterDB.m_modules.size()) );
			if(m_ClusterDB.isModuleFix(mid1)==false && m_ClusterDB.isModuleFix(mid2)==false)
			{
				if( (m_ClusterDB.m_modules[mid1].m_area +m_ClusterDB.m_modules[mid2].m_area)<=(0.5*maxClusterArea) )
				{
					m_ClusterDB.mergeModule(mid1,mid2);
				}
			}
		}
		if(m_ClusterDB.getMovableCount()<=targetClusterNumber)
		{
			break;
		}
	}
	if(showMsg)
	{
		cout<<"\nMerge 2-pin net:"<<seconds()-timestart;
		timestart=seconds();
	//	m_ClusterDB.showNetProfile();
	}
	//-------------------
	// 2. first choice merge
	//-------------------

	int numOfClusters=m_ClusterDB.getMovableCount();
	if(showMsg)
	{
		cout<<"\n# of Moveable:"<<numOfClusters;
	}
	bool stop=false;
	int noc=numOfClusters;
	while(numOfClusters>targetClusterNumber && stop==false)
	{
		bool mergeFlag=false;
		//sort by vertex degree (same as mPL5's procedure)
		vector<int> nodes;
		nodes.reserve(m_ClusterDB.getMovableCount());
		for(int i=0; i<(int)m_ClusterDB.m_modules.size(); ++i)
		{
			if(m_ClusterDB.m_modules[i].m_isExist==true && m_ClusterDB.isModuleFix(i)==false &&
				m_ClusterDB.m_modules[i].m_isNoLegalNeighbor==false)
			{
				if(m_ClusterDB.m_modules[i].m_area<=(0.5*maxClusterArea))
				{
					nodes.push_back(i);
				}
			}
		}
		sort(nodes.begin(),nodes.end(),CFCDBClusteringVertexCompare(m_ClusterDB));
		if(showMsg)
		{
			cout<<"\nBuild nodes, sorting:"<<seconds()-timestart<<" Size of noes:"<<nodes.size();
			timestart=seconds();
		}
		//vector< CClusterDB_Module > modulesSort=this->m_ClusterDB.m_modules;

		//sort(modulesSort.begin(),modulesSort.end(),CClusterDB_ModuleCompare);


		//for all movable module, merge with heightest affinity module
		for(int i=0; i<(int)nodes.size(); ++i)
		{
			if( m_ClusterDB.m_modules[nodes[i]].m_isExist==true && m_ClusterDB.isModuleFix(nodes[i])==false)
			{
				int mergeID=chooseMaxAffinity( nodes[i] ,maxClusterArea );

				if(mergeID!=-1)
				{
					m_ClusterDB.mergeModule(nodes[i],mergeID);
					mergeFlag=true;
					if(m_ClusterDB.getMovableCount()<=targetClusterNumber)
					{
						stop=true;
						break;
					}
				}
				else
				{
					m_ClusterDB.m_modules[nodes[i]].m_isNoLegalNeighbor=true;
				}

			}
		}
		if(showMsg)
		{
			cout<<"\nRound :"<<seconds()-timestart;
			cout<<"\n# of Moveable:"<<m_ClusterDB.getMovableCount();
		//	m_ClusterDB.showNetProfile();
			timestart=seconds();
		}
		if(mergeFlag==false)
		{
			stop=true; //after a round of search, no modules can be merged
		}
		int numAfterMerge=m_ClusterDB.getMovableCount();

		double decreasingRate=1-(double)numAfterMerge/(double)noc;
		noc=numAfterMerge;
		if(decreasingRate<0.001)
		{
			stop=true;
		}
	}
	//if(showMsg)
	//{
	//	cout<<"\nFinish Cluster:"<<seconds()-timestart;
	//}
//	removeExcessModulePin();
	m_ClusterDB.placeDbOut();
	hierarchy=m_ClusterDB.m_hierarchy;
	if(showMsg)
	{
		cout<<"\nDB out :"<<seconds()-timestart;
		cout<<"\n# of Moveable:"<<m_ClusterDB.getMovableCount()<<"\n======================\n";
		timestart=seconds();
	}
	//if(showMsg)
	//{
	//	cout<<"\nTotal Finish:"<<seconds()-timestart;
	//}


}


void CClusterDBFC::clusteringNH(const CPlaceDB& dblarge, CPlaceDB& dbsmall, vector< vector<int> >& hierarchy,int targetClusterNumber, double areaRatio)
{



	bool showMsg=false;
	if(showMsg)
	{
		cout<<"\nStart FC clustering:target#:"<<targetClusterNumber;
		

	}
	double timestart=seconds();
	dbsmall=dblarge;

	m_pDB=&dbsmall;
	m_ClusterDB.placeDbIn(*m_pDB);

	showDBdata(m_pDB);

	//find maxClusterArea 
	double maxClusterArea=m_ClusterDB.findMaxClusterArea(targetClusterNumber, areaRatio);
	cout<<"\nStart FC clustering:target#:"<<targetClusterNumber<<" Max area:"<<maxClusterArea;
	//--------------------------------------------------------------------------------
	//main clustering program
	//--------------------------------------------------------------------------------


	//add bypass net to prevnet clustering saturation
	if(showMsg)
	{
		cout<<"\nDB in, add bypass net:"<<seconds()-timestart;
		timestart=seconds();
		cout<<"\n# of Moveable:"<<m_ClusterDB.getMovableCount();
	//	m_ClusterDB.showNetProfile();
	}


	//-------------------
	// 2. first choice merge
	//-------------------

	int numOfClusters=m_ClusterDB.getMovableCount();
	if(showMsg)
	{
		cout<<"\n# of Moveable:"<<numOfClusters;
	}
	bool stop=false;
	int noc=numOfClusters;
	while(numOfClusters>targetClusterNumber && stop==false)
	{
		bool mergeFlag=false;
		//sort by vertex degree (same as mPL5's procedure)
		vector<int> nodes;
		nodes.reserve(m_ClusterDB.getMovableCount());
		for(int i=0; i<(int)m_ClusterDB.m_modules.size(); ++i)
		{
			if(m_ClusterDB.m_modules[i].m_isExist==true && m_ClusterDB.isModuleFix(i)==false &&
				m_ClusterDB.m_modules[i].m_isNoLegalNeighbor==false)
			{

				nodes.push_back(i);

			}
		}
		sort(nodes.begin(),nodes.end(),CFCDBClusteringVertexCompare(m_ClusterDB));
		if(showMsg)
		{
			cout<<"\nBuild nodes, sorting:"<<seconds()-timestart<<" Size of noes:"<<nodes.size();
			timestart=seconds();
		}
		//vector< CClusterDB_Module > modulesSort=this->m_ClusterDB.m_modules;

		//sort(modulesSort.begin(),modulesSort.end(),CClusterDB_ModuleCompare);


		//for all movable module, merge with heightest affinity module
		for(int i=0; i<(int)nodes.size(); ++i)
		{
			if( m_ClusterDB.m_modules[nodes[i]].m_isExist==true && m_ClusterDB.isModuleFix(nodes[i])==false)
			{
				int mergeID=chooseMaxAffinity( nodes[i] ,maxClusterArea );

				if(mergeID!=-1)
				{
					m_ClusterDB.mergeModule(nodes[i],mergeID);
					mergeFlag=true;
					if(m_ClusterDB.getMovableCount()<=targetClusterNumber)
					{
						stop=true;
						break;
					}
				}
				else
				{
					m_ClusterDB.m_modules[nodes[i]].m_isNoLegalNeighbor=true;
				}

			}
		}
		if(showMsg)
		{
			cout<<"\nRound :"<<seconds()-timestart;
			cout<<"\n# of Moveable:"<<m_ClusterDB.getMovableCount();
		//	m_ClusterDB.showNetProfile();
			timestart=seconds();
		}
		if(mergeFlag==false)
		{
			stop=true; //after a round of search, no modules can be merged
		}
		int numAfterMerge=m_ClusterDB.getMovableCount();

		double decreasingRate=1-(double)numAfterMerge/(double)noc;
		noc=numAfterMerge;
		if(decreasingRate<0.0001)
		{
			stop=true;
		}
	}
	//if(showMsg)
	//{
	//	cout<<"\nFinish Cluster:"<<seconds()-timestart;
	//}
//	removeExcessModulePin();
	m_ClusterDB.placeDbOut();
	hierarchy=m_ClusterDB.m_hierarchy;
	if(showMsg)
	{
		cout<<"\nDB out :"<<seconds()-timestart;
		cout<<"\n# of Moveable:"<<m_ClusterDB.getMovableCount()<<"\n======================\n";
		timestart=seconds();
	}
	//if(showMsg)
	//{
	//	cout<<"\nTotal Finish:"<<seconds()-timestart;
	//}
	showDBdata(m_pDB);
	printf("\nFCClustering,  memory usage:%.2f",GetPeakMemoryUsage());
}
void CClusterDBBC::init()
{
//	vector< map<int,set<int> > > m_moduleNeighborS;
	//m_moduleNeighborS.resize(m_ClusterDB.m_modules.size());

	this->m_moduleNeighbor.resize(m_ClusterDB.m_modules.size());
	this->m_moduleAffinityMap.resize(m_ClusterDB.m_modules.size());
	this->m_moduleBestAffinity.resize(m_ClusterDB.m_modules.size(),0);
	this->m_moduleBestAffinityID.resize(m_ClusterDB.m_modules.size(),-1);

	m_updateFlag.resize(m_ClusterDB.m_modules.size(),false);
	printf("\nInit--Resize,  memory usage:%.2f",GetPeakMemoryUsage());

	////////////////////////////////////////////////
	//build neighborMaps
	////////////////////////////////////////////////
//	m_neighborMaps.resize(db.m_modules.size() );
	for(int i=0; i<(int)m_ClusterDB.m_modules.size(); ++i)
	{
		if(m_ClusterDB.isModuleFix(i)==false)
		{
			set<int>::iterator netIter;
			for(netIter=m_ClusterDB.m_modules[i].m_netIDs.begin(); netIter!=m_ClusterDB.m_modules[i].m_netIDs.end(); ++netIter)
			{
				set<int>::iterator moduleIter;
				for(moduleIter=m_ClusterDB.m_nets[*netIter].begin(); moduleIter!=m_ClusterDB.m_nets[*netIter].end(); ++moduleIter)
				{
					if(m_ClusterDB.isModuleFix(*moduleIter)==false && *moduleIter!=i)
					{
//						m_moduleNeighbor[i][*moduleIter].insert(*netIter);	
						//m_moduleNeighborS[i][*moduleIter].insert(*netIter);
						m_moduleNeighbor[i][*moduleIter].push_back(*netIter);
					}
					
				}
			}
		}
	}
	printf("\nInit--build neighbor,  memory usage:%.2f",GetPeakMemoryUsage());
	////////////////////////////////////////////////////
	//build initial affinity maps
	////////////////////////////////////////////////////
	for(unsigned int i=0; i<m_ClusterDB.m_modules.size(); ++i)
	{
		if(m_pDB->m_modules[i].m_isFixed==false)
		{
//			map<int,set<int> >::iterator it;
			map<int,vector<int> >::iterator it;
			for(it=m_moduleNeighbor[i].begin(); it!=m_moduleNeighbor[i].end(); ++it)
			{
				//if(m_moduleNeighbor[i][it->first].size()!=m_moduleNeighborS[i][it->first].size())
				//{
				//	cerr<<"\n V:"<<m_moduleNeighbor[i][it->first].size()<<" S:"<<m_moduleNeighborS[i][it->first].size();
				//}
				if(m_pDB->m_modules[it->first].m_isFixed==false)
				{
					updateAffinity(i,it->first);
				}
			}
		}		
	}
	printf("\nInit--Affinity Map,  memory usage:%.2f",GetPeakMemoryUsage());
	//find max affinity
	for(unsigned int i=0; i<m_ClusterDB.m_modules.size(); ++i)
	{
		if(m_pDB->m_modules[i].m_isFixed==false)
		{
			updateMaxAffinity(i);
		}

	}
	printf("\nInit--MaxAffinity,  memory usage:%.2f",GetPeakMemoryUsage());
}
void CClusterDBBC::init2()
{
//	vector< map<int,set<int> > > m_moduleNeighborS;
	//m_moduleNeighborS.resize(m_ClusterDB.m_modules.size());

	this->m_moduleAffinityMap.resize(m_ClusterDB.m_modules.size());
	this->m_moduleBestAffinity.resize(m_ClusterDB.m_modules.size(),0);
	this->m_moduleBestAffinityID.resize(m_ClusterDB.m_modules.size(),-1);

	m_updateFlag.resize(m_ClusterDB.m_modules.size(),false);
//	printf("\nInit--Resize,  memory usage:%.2f",GetPeakMemoryUsage());


	////////////////////////////////////////////////////
	//build initial affinity maps
	////////////////////////////////////////////////////
	for(unsigned int i=0; i<m_ClusterDB.m_modules.size(); ++i)
	{
		if(m_pDB->m_modules[i].m_isFixed==false)
		{
			updateAllAffinity(i);
		}		
	}
//	printf("\nInit--Affinity Map,  memory usage:%.2f",GetPeakMemoryUsage());
	//find max affinity
	for(unsigned int i=0; i<m_ClusterDB.m_modules.size(); ++i)
	{
		if(m_pDB->m_modules[i].m_isFixed==false)
		{
			updateMaxAffinity(i);
		}

	}
	m_updateFlag.resize(m_ClusterDB.m_modules.size(),false);
//	printf("\nInit--MaxAffinity,  memory usage:%.2f",GetPeakMemoryUsage());
}
void CClusterDBBC::updateAllAffinity(int mID)
{
	m_moduleAffinityMap[mID].clear();
	//for all net belonging to the module, find its affinity
	set<int>::iterator netIter;
	for(netIter=m_ClusterDB.m_modules[mID].m_netIDs.begin(); netIter!=m_ClusterDB.m_modules[mID].m_netIDs.end(); ++netIter)
	{
		int netid=*netIter;
		int netSize=(int)m_ClusterDB.m_nets[*netIter].size();
		if(netSize>1)
		{
			set<int>::iterator mIter; //module iterator
			for(mIter=m_ClusterDB.m_nets[*netIter].begin(); mIter!=m_ClusterDB.m_nets[*netIter].end(); ++mIter)
			{
				if(m_ClusterDB.isModuleFix(*mIter)==false && *mIter!=mID && m_ClusterDB.m_modules[*mIter].m_isExist==true )
				{
					int mid=*mIter;
					if((m_ClusterDB.m_modules[mID].m_area + m_ClusterDB.m_modules[mid].m_area)<=m_maxClusterArea)
					{
						
						//----------------
						// calc affinity
						//---------------
						double affinity=0;
						affinity=1/( (netSize-1)*(m_ClusterDB.m_modules[mID].m_area + m_ClusterDB.m_modules[mid].m_area));

						if(m_moduleAffinityMap[mID].find(mid)!=m_moduleAffinityMap[mID].end()) //the module exists in other net
						{
							m_moduleAffinityMap[mID][mid]+=affinity;						
						}
						else
						{
							m_moduleAffinityMap[mID].insert(pair<int,double>(mid,affinity));
						}
					}
					else
					{
						m_moduleAffinityMap[mID][mid]=-1;

					}
				}
			}
		}

	}
	map<int,double>::iterator it;
	for(it=m_moduleAffinityMap[mID].begin(); it!=m_moduleAffinityMap[mID].end(); ++it)
	{
		assert(m_ClusterDB.m_modules[it->first].m_isExist==true);

		m_moduleAffinityMap[it->first][mID]=it->second;
		m_updateFlag[it->first]=true;

	}
}
void CClusterDBBC::clustering(const CPlaceDB& dblarge, CPlaceDB& dbsmall, vector< vector<int> >& hierarchy,int targetClusterNumber, double areaRatio)
{

//	printf("\nBC start,  memory usage:%.2f",GetPeakMemoryUsage());
	bool showMsg=false;
	if(showMsg)
	{
		cout<<"\nStart BC clustering:target#:"<<targetClusterNumber;
	}
	double timestart=seconds();
	dbsmall=dblarge;
	m_pDB=&dbsmall;
	m_ClusterDB.placeDbIn(*m_pDB);
	showDBdata(m_pDB);

	//find maxClusterArea 
	m_maxClusterArea=m_ClusterDB.findMaxClusterArea(targetClusterNumber, areaRatio);

	//--------------------------------------------------------------------------------
	//main clustering program
	//--------------------------------------------------------------------------------

	if(showMsg)
	{
		cout<<"\nDB in, add bypass net:"<<seconds()-timestart;
		timestart=seconds();
		cout<<"\n# of Moveable:"<<m_ClusterDB.getMovableCount();
	//	m_ClusterDB.showNetProfile();
	}

	//-------------------
	// 2. Best choice merge
	//-------------------

	init2();
//	printf("\nFinish init, total %.2f seconds, memory usage:%.2f",seconds()-timestart,GetPeakMemoryUsage());
	timestart=seconds();
	//1.add all moudle to lazy queue
	multimap<double,int,bestAffinityCompare> lazyQueue;
	for(unsigned int i=0; i<m_ClusterDB.m_modules.size(); ++i)
	{
		if(m_moduleBestAffinityID[i]!=-1 && m_ClusterDB.isModuleFix(i)==false)
		{
			lazyQueue.insert(pair<double,int>(m_moduleBestAffinity[i],i));
		}
	}


	int numOfClusters=m_ClusterDB.getMovableCount();
//	printf("\nFinish Lqueue build, total %.2f seconds, memory usage:%.2f",seconds()-timestart,GetPeakMemoryUsage());
	timestart=seconds();
	if(showMsg)
	{
		cout<<"\n# of Moveable:"<<numOfClusters<<" queue size:"<<lazyQueue.size();
	}
	bool stop=false;
	while(numOfClusters>targetClusterNumber && stop==false)
	{
		multimap<double,int,bestAffinityCompare>::iterator iter=lazyQueue.begin();
		if(iter==lazyQueue.end())
		{
			stop=true;
		}
		else if(m_updateFlag[iter->second]==true) //the max affinity may changed
		{
			int mid=iter->second;
			lazyQueue.erase(iter);

			updateMaxAffinity(mid);
			if(m_moduleBestAffinityID[mid]!=-1)
			{
				lazyQueue.insert(pair<double,int>(m_moduleBestAffinity[mid],mid));
			}
		}
		else
		{
			int mid=iter->second;
			lazyQueue.erase(iter);
			assert(m_moduleBestAffinityID[mid]!=-1);
			assert(m_ClusterDB.isModuleFix(m_moduleBestAffinityID[mid])==false);
			mergeModule(m_moduleBestAffinityID[mid],mid);

		}

		numOfClusters=m_ClusterDB.getMovableCount();

	}
//	printf("\nFinish BC, total %.2f seconds, memory usage:%.2f",seconds()-timestart,GetPeakMemoryUsage());
//	timestart=seconds();
	//if(showMsg)
	//{
	//	cout<<"\nFinish Cluster:"<<seconds()-timestart;
	//}
//	removeExcessModulePin();
	m_ClusterDB.placeDbOut();
	hierarchy=m_ClusterDB.m_hierarchy;
//	printf("\nFinish DB-Out, total %.2f seconds, memory usage:%.2f",seconds()-timestart,GetPeakMemoryUsage());
	timestart=seconds();
//	showDBdata(m_pDB);
	if(showMsg)
	{
			
		cout<<"\nDB out :"<<seconds()-timestart;
		cout<<"\n# of Moveable:"<<m_ClusterDB.getMovableCount()<<"\n======================\n";
		timestart=seconds();
	}
	//if(showMsg)
	//{
	//	cout<<"\nTotal Finish:"<<seconds()-timestart;
	//}
	this->m_moduleNeighbor.clear();
	m_moduleAffinityMap.clear();


}
void CClusterDBBC::updateAffinity(int mID1,int mID2)
{
	set<int>::iterator it;
	double affinity=0;
	if( (m_ClusterDB.m_modules[mID1].m_area + m_ClusterDB.m_modules[mID2].m_area)<=m_maxClusterArea )
	{
//		for(it=m_moduleNeighbor[mID1][mID2].begin(); it!=m_moduleNeighbor[mID1][mID2].end(); ++it)
		for(unsigned int i=0; i<m_moduleNeighbor[mID1][mID2].size(); ++i)
		{
//			affinity+=1/( (m_ClusterDB.m_nets[*it].size()-1)*(m_ClusterDB.m_modules[mID1].m_area + m_ClusterDB.m_modules[mID2].m_area));
			affinity+=1/( (m_ClusterDB.m_nets[m_moduleNeighbor[mID1][mID2][i]].size()-1)*(m_ClusterDB.m_modules[mID1].m_area + m_ClusterDB.m_modules[mID2].m_area));
		}

		m_moduleAffinityMap[mID1][mID2]=affinity;
		m_moduleAffinityMap[mID2][mID1]=affinity;
	}
	else
	{
		m_moduleAffinityMap[mID1][mID2]=-1;
		m_moduleAffinityMap[mID2][mID1]=-1;
	}
	
}
void CClusterDBBC::mergeModule(int mID1,int mID2)
{
	m_ClusterDB.mergeModule(mID1,mID2);

//	//------------------------
//	//update neighbor
//	//------------------------
////	map<int,set<int> >::iterator nIt;
//	map<int,vector<int> >::iterator nIt;
//	for(nIt=m_moduleNeighbor[mID2].begin(); nIt!=m_moduleNeighbor[mID2].end(); ++nIt)
//	{
//		if(nIt->first!=mID1)
//		{
////			m_moduleNeighbor[nIt->first][mID1].insert(m_moduleNeighbor[nIt->first][mID2].begin(),m_moduleNeighbor[nIt->first][mID2].end());
//			set<int> tempNetSet;
//			tempNetSet.insert(m_moduleNeighbor[nIt->first][mID1].begin(),m_moduleNeighbor[nIt->first][mID1].end());
//			tempNetSet.insert(m_moduleNeighbor[nIt->first][mID2].begin(),m_moduleNeighbor[nIt->first][mID2].end());
//			m_moduleNeighbor[nIt->first][mID1].clear();
////			m_moduleNeighbor[nIt->first][mID1].reserve(tempNetSet.size());
////			set<int>::iterator it;
//			//for(it=tempNetSet.begin(); it!=tempNetSet.end(); ++it)
//			//{
//			//	m_moduleNeighbor[nIt->first][mID1].push_back(*it);
//			//}
//			m_moduleNeighbor[nIt->first][mID1].insert(m_moduleNeighbor[nIt->first][mID1].begin(),tempNetSet.begin(),tempNetSet.end());
//
//			m_moduleNeighbor[mID1][nIt->first]=m_moduleNeighbor[nIt->first][mID1];
//			m_moduleNeighbor[nIt->first].erase(mID2);
//			m_moduleAffinityMap[nIt->first].erase(mID2);
//		}
//	}
//	m_moduleNeighbor[mID1].erase(mID2);

	updateAllAffinity(mID1);

	m_moduleAffinityMap[mID1].erase(mID2);

	//set false flag, for all neighbors of mID1, set updateFlag=true;
	m_updateFlag[mID1]=true;
	m_updateFlag[mID2]=true;
	//for(nIt=m_moduleNeighbor[mID1].begin(); nIt!=m_moduleNeighbor[mID1].end(); ++nIt)
	//{
	//	updateAffinity(mID1,nIt->first);
	//	m_updateFlag[nIt->first]=true;
	//}
	

}
//int CClusterDBBC::findBestAffinity(double maxClusterArea, int& target)
//{
//	double maxAff=0;
//	target=-1;
//	int mid=-1;
//	for(int i=0; i<(int)m_ClusterDB.m_modules.size(); ++i)
//	{
//		if(m_ClusterDB.m_modules[i].m_isExist==true && m_ClusterDB.isModuleFix(i)==false &&
//			m_ClusterDB.m_modules[i].m_isNoLegalNeighbor==false)
//		{
//			double aff;
//			int ma=chooseMaxAffinity( i,  maxClusterArea, aff);
//			if(ma==-1)
//			{
//				m_ClusterDB.m_modules[i].m_isNoLegalNeighbor=true;
//			}
//			else
//			{
//				if(aff>maxAff)
//				{
//					mid=ma;
//					target=i;
//				}
//			}
//
//		}
//	}
//	return mid;
//}
void CClusterDBFC::addBypassNet()
{
	if(this->bypassMacroRowHeight!=0)
	{
		//for all fixed module
		for(int i=0; i<(int)m_ClusterDB.m_modules.size(); ++i)
		{
			if(m_ClusterDB.isModuleFix(i)==true && m_pDB->m_modules[i].m_height<=bypassMacroRowHeight)
			{
				int pseudoNetID=m_ClusterDB.m_nets.size();
				m_ClusterDB.m_nets.resize(pseudoNetID+1);

				//for all neighbors of the fixed module
				set<int>::iterator netIter;
				for(netIter=m_ClusterDB.m_modules[i].m_netIDs.begin(); netIter!=m_ClusterDB.m_modules[i].m_netIDs.end(); ++netIter)
				{
					set<int>::iterator mIter;
					for(mIter=m_ClusterDB.m_nets[*netIter].begin(); mIter!=m_ClusterDB.m_nets[*netIter].end(); ++mIter)
					{
						if(*mIter!=i)
						{
							m_ClusterDB.m_modules[*mIter].m_netIDs.insert(pseudoNetID);
							m_ClusterDB.m_nets[pseudoNetID].insert(*mIter);
						}

					}
				}

			}
		}
	}
}
double CClusterDB::findMaxClusterArea(const int& targetClusterNumber,const double& areaRatio)
{
	//find maxClusterArea 
	double tarea=0;

	for(int i=0; i<(int)m_pDB->m_modules.size(); i++)
	{
		if(m_pDB->m_modules[i].m_isFixed==false)
		{
			tarea+=m_pDB->m_modules[i].m_area;
			//for(int j=0; j<(int)m_pDB->m_modules[i].m_netsId.size(); j++)
			//{
			//	m_netSets[i].insert(m_pDB->m_modules[i].m_netsId[j]);
			//}
		}
	}
//	cout<<"\nTarea:"<<tarea<<" targetN:"<<targetClusterNumber<<" aratio:"<<areaRatio;
	return (tarea/targetClusterNumber)*areaRatio;
}
void CClusterDBBC::updateMaxAffinity(int mID)
{
	double max=0;
	set<int> removeSet;
	map<int,double>::iterator it;
	for(it=m_moduleAffinityMap[mID].begin(); it!=m_moduleAffinityMap[mID].end(); ++it)
	{
		if(m_ClusterDB.m_modules[it->first].m_isExist==false)
		{
			removeSet.insert(it->first);
		}
		else if(it->second>max)
		{
			m_moduleBestAffinity[mID]=it->second;
			m_moduleBestAffinityID[mID]=it->first; //if no legal max affinity module, m_moduleBestAffinityID[i]==-1 !!
			max=it->second;

		}
	}
	set<int>::iterator rit;
	for(rit=removeSet.begin(); rit!=removeSet.end(); ++rit)
	{
		m_moduleAffinityMap[mID].erase(*rit);
	}

	if(max==0)
		m_moduleBestAffinityID[mID]=-1;
	else
	{
		m_updateFlag[mID]=false;
	}

}
//int CClusterDBBC::chooseMaxAffinity(int mID, double maxClusterArea, double& aff)
//{
//	map<int,double> affinityMap;
//
//	//for all net belonging to the module, find its affinity
//	set<int>::iterator netIter;
//	for(netIter=m_ClusterDB.m_modules[mID].m_netIDs.begin(); netIter!=m_ClusterDB.m_modules[mID].m_netIDs.end(); ++netIter)
//	{
//		int netid=*netIter;
//		int netSize=(int)m_ClusterDB.m_nets[*netIter].size();
//		if(netSize>1)
//		{
//			set<int>::iterator mIter; //module iterator
//			for(mIter=m_ClusterDB.m_nets[*netIter].begin(); mIter!=m_ClusterDB.m_nets[*netIter].end(); ++mIter)
//			{
//				if(m_ClusterDB.isModuleFix(*mIter)==false && *mIter!=mID)
//				{
//					int mid=*mIter;
//					//----------------
//					// calc affinity
//					//---------------
//					double affinity=0;
//					affinity=1/( (netSize-1)*(m_ClusterDB.m_modules[mID].m_area + m_ClusterDB.m_modules[mid].m_area));
//
//					if(affinityMap.find(mid)!=affinityMap.end()) //the module exists in other net
//					{
//						affinityMap[mid]+=affinity;						
//					}
//					else
//					{
//						affinityMap.insert(pair<int,double>(mid,affinity));
//					}
//				}
//			}
//		}
//
//	}
//
//
//	//--------------------------------------------------------------------------------------------
//	// find the max affinity without break the constraint
//	//--------------------------------------------------------------------------------------------
//	map<int,double>::iterator it;
//	int target=-1;
//	double maxAffinity=-1;
//
//	for(it=affinityMap.begin(); it!=affinityMap.end(); ++it)
//	{
//		if(it->second>maxAffinity)
//		{
//			double combineArea=m_ClusterDB.m_modules[mID].m_area + m_ClusterDB.m_modules[it->first].m_area;
//			if(combineArea<= maxClusterArea)
//			{
//				target=it->first;
//				maxAffinity=it->second;
//
//			}
//		}
//
//	}
//	aff=maxAffinity;
//	return target; //if target==-1, means all neighbor cells are larger than maxarea. Don't perform clustering on this cell
//}
int CClusterDBFC::chooseMaxAffinity(int mID, double maxClusterArea)
{
	map<int,double> affinityMap;

	//for all net belonging to the module, find its affinity
	set<int>::iterator netIter;
	for(netIter=m_ClusterDB.m_modules[mID].m_netIDs.begin(); netIter!=m_ClusterDB.m_modules[mID].m_netIDs.end(); ++netIter)
	{
//		int netid=*netIter;
		int netSize=(int)m_ClusterDB.m_nets[*netIter].size();
		if(netSize>1)
		{
			set<int>::iterator mIter; //module iterator
			for(mIter=m_ClusterDB.m_nets[*netIter].begin(); mIter!=m_ClusterDB.m_nets[*netIter].end(); ++mIter)
			{
				if(m_ClusterDB.isModuleFix(*mIter)==false && *mIter!=mID)
				{
					int mid=*mIter;
					//----------------
					// calc affinity
					//---------------
					double affinity=0;
					affinity=1/( (netSize-1)*(m_ClusterDB.m_modules[mID].m_area + m_ClusterDB.m_modules[mid].m_area));

					if(affinityMap.find(mid)!=affinityMap.end()) //the module exists in other net
					{
						affinityMap[mid]+=affinity;						
					}
					else
					{
						affinityMap.insert(pair<int,double>(mid,affinity));
					}
				}
			}
		}

	}


	//--------------------------------------------------------------------------------------------
	// find the max affinity without break the constraint
	//--------------------------------------------------------------------------------------------
	map<int,double>::iterator it;
	int target=-1;
	double maxAffinity=-1;

	for(it=affinityMap.begin(); it!=affinityMap.end(); ++it)
	{
		if(it->second>maxAffinity)
		{
			double combineArea=m_ClusterDB.m_modules[mID].m_area + m_ClusterDB.m_modules[it->first].m_area;
			if(combineArea<= maxClusterArea)
			{
				target=it->first;
				maxAffinity=it->second;
			}
		}

	}

	return target; //if target==-1, means all neighbor cells are larger than maxarea. Don't perform clustering on this cell
}
