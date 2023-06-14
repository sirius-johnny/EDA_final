#include "PlaceDBQP.h"
#include <iomanip>

void checkMatrix(CQP& qp, vector<double>& vec_B)
{
	for(unsigned int i=0; i<qp.mapA.size(); i++)
	{
		
		map<int,double>::iterator iter;
		iter=qp.mapA[i].find(i);
		assert(iter!=qp.mapA[i].end());
		assert(iter->second>0);

		double dio=iter->second;
	//	cout<<"\n i:"<<iter->first<<" dio:"<<dio;

		double count=0;

		for(iter=qp.mapA[i].begin(); iter!=qp.mapA[i].end(); iter++)
		{
			assert(iter->first>=0);
			assert((unsigned int)iter->first<qp.mapA.size());
			double v2;
			qp.getValue(iter->first,i,v2);
			assert(v2==iter->second);

			count+=iter->second;
			
		}
		//if(count<0)
		//{
		//	cout<<"\n i:"<<i<<" count:"<<count<<" dio:"<<dio;
		//}
		//assert(count<0);
	//	cout<<" count:"<<count<<"\n";
		if( (dio+count)!=0 )
		{
		//	assert(vec_B[i]!=0);
		}
	}
}
void printMatrix(CQP& qp)
{
	unsigned int size=qp.mapA.size();
	cout<<" Matrix:size:"<<qp.mapA.size()<<"\n";

	for(unsigned int i=0; i<size; i++)
	{
		vector<double> vec;
		vec.resize(size);
		for(unsigned int j=0; j<size; j++)
		{
			vec[j]=0;
		}
		map<int,double>::iterator iter;
//		int count=0;
		for(iter=qp.mapA[i].begin(); iter!=qp.mapA[i].end(); iter++)
		{
			vec[iter->first]=iter->second;
			//cout<<"i:"<<i<<" j:"<<iter->first<<" value:"<<iter->second<<"\n";
		}
		for(unsigned int j=0; j<size; j++)
		{
			cout<<setw(6)<<setprecision(3)<<vec[j]<<"|";
			//if(vec[j]!=-2733.97)
			//	cout<<vec[j]<<" | ";
			//else
			//	cout<<"   | ";
		}
		cout<<"\n";
	}

}




CPlaceDBQPLevel::CPlaceDBQPLevel(void)
{
	m_levelID=-1;
}

CPlaceDBQPLevel::~CPlaceDBQPLevel(void)
{
}


CPlaceDBQPRegionNet::CPlaceDBQPRegionNet(int netID)
{
	this->m_netID=netID;

	this->m_nLeftModules=0;
	this->m_nBottomModules=0;
	this->m_nRightModules=0;
	this->m_nTopModules=0;
	m_nCLeftModules=0; 
	m_nCRightModules=0;
	m_nCTopModules=0;
	m_nCBottomModules=0;

	this->m_isGlobalX=false;
	this->m_isGlobalY=false;

	this->m_localModules.clear();

	this->m_fixedPinX.clear();
	this->m_fixedPinY.clear();
}
void CPlaceDBQPRegionNet::reset()
{
	this->m_nLeftModules=0;
	this->m_nBottomModules=0;
	this->m_nRightModules=0;
	this->m_nTopModules=0;
	m_nCLeftModules=0; 
	m_nCRightModules=0;
	m_nCTopModules=0;
	m_nCBottomModules=0;

	this->m_isGlobalX=false;
	this->m_isGlobalY=false;
	this->m_fixedPinX.clear();
	this->m_fixedPinY.clear();
}
CPlaceDBQPRegionNet::~CPlaceDBQPRegionNet(void)
{
}

CPlaceDBQPRegion::CPlaceDBQPRegion(void)
{

}

CPlaceDBQPRegion::CPlaceDBQPRegion(CPlaceDB& db,CPlaceDBQPPlacer& paqp)
{
	m_pDB = &db;
	m_pPqap = &paqp;

}

CPlaceDBQPRegion::~CPlaceDBQPRegion(void)
{
}


//only run QP on X or Y 


void CPlaceDBQPRegion::setRgn(double top, double bottom, double left, double right)
{
	this->m_Rgn.top=top;
	this->m_Rgn.bottom=bottom;
	this->m_Rgn.left=left;
	this->m_Rgn.right=right;
}

void CPlaceDBQPRegion::initRegionNet()
{
    //----------------------------------------------------
    // Modify log: (tellux) 
    // 2005/11/09: Change the outside region movabel module to movable pin (improve accuracy of large module)
    // 2005/11/09: Add m_regionNets[i].m_nCLeftModules... to give data to hMetis partitioner
    //----------------------------------------------------

    if( m_moduleIDs.size() == 0 ) //if there is no movable module inside
	return;

    set<int> netSet;
    localModuleIDSet.clear();
    realModuleID2regionModuleID.clear();

    //fill netsID into netSet from modules in this region
    for( unsigned int i=0; i<m_moduleIDs.size(); i++ )
    {
	int id = m_moduleIDs[i];
	this->safeMoveCenter( id, m_pDB->m_modules[id].m_cx, m_pDB->m_modules[id].m_cy );
	realModuleID2regionModuleID[id] = i;
	localModuleIDSet.insert( id );

	for( unsigned int j=0; j<m_pDB->m_modules[id].m_netsId.size(); j++ )
	{
	    netSet.insert( m_pDB->m_modules[id].m_netsId[j] );
	}
	//double cx=0.5*(m_Rgn.left+m_Rgn.right);
	//double cy=0.5*(m_Rgn.top+m_Rgn.bottom);
	//m_pDB->SetModuleLocation(m_moduleIDs[i],cx,cy);
    }


    //fill into m_regionNets
    m_regionNets.reserve( netSet.size() );
    set<int>::iterator it;
    for( it=netSet.begin(); it!=netSet.end(); ++it )
    {
	CPlaceDBQPRegionNet rgNet(*it);

	//only consider net degree >1 
	if( m_pDB->m_nets[*it].size() > 1 )
	{
	    m_regionNets.push_back(rgNet);
	}
    }

    //------
    //establish pins and m_localModules
    //------
    double region_centerX = 0.5 * (m_Rgn.left + m_Rgn.right);
    double region_centerY = 0.5 * (m_Rgn.top  + m_Rgn.bottom);

    //for all region nets
    for( unsigned int i=0; i<m_regionNets.size(); i++ )
    {
	//for all cells in the net
	for( unsigned int j=0; j<m_pDB->m_nets[m_regionNets[i].m_netID].size(); j++ )
	{
	    int pinID    = m_pDB->m_nets[m_regionNets[i].m_netID][j];
	    int moduleID = m_pDB->m_pins[pinID].moduleId;

	    //for movable module
	    if( m_pDB->m_modules[moduleID].m_isFixed == false )
	    {
		set<int>::iterator it;
		it = localModuleIDSet.find( moduleID );

		if( it == localModuleIDSet.end() ) //the module is not in the region (use pin positions)
		{
		    double pinX = m_pDB->m_pins[pinID].absX;
		    double pinY = m_pDB->m_pins[pinID].absY;

		    if( pinX <= m_Rgn.left ) //module in the left
		    {
			++m_regionNets[i].m_nLeftModules;					
		    }
		    else if( pinX <= region_centerX )
		    {
			m_regionNets[i].m_fixedPinX.push_back( pinX );
			++m_regionNets[i].m_nCLeftModules;
		    }
		    else if( pinX < m_Rgn.right )
		    {
			m_regionNets[i].m_fixedPinX.push_back( pinX );
			++m_regionNets[i].m_nCRightModules;
		    }
		    else //module in the right
		    {
			++m_regionNets[i].m_nRightModules;					
		    }


		    if( pinY <= m_Rgn.bottom ) //module in the left
		    {
			++m_regionNets[i].m_nBottomModules;					
		    }
		    else if( pinY <= region_centerY )
		    {
			m_regionNets[i].m_fixedPinY.push_back( pinY );
			++m_regionNets[i].m_nCBottomModules;
		    }
		    else if( pinY < m_Rgn.top )
		    {
			m_regionNets[i].m_fixedPinY.push_back( pinY );
			++m_regionNets[i].m_nCTopModules;
		    }
		    else //module in the right
		    {
			++m_regionNets[i].m_nTopModules;					
		    }
		}
		else // the module is inside the region
		{

		    m_regionNets[i].m_localModules.push_back( moduleID );
		    m_regionNets[i].m_localModulesPinID.push_back( pinID );

		}
	    }
	    else //for fixed modules
	    {

		//m_regionNets[i].m_fixedPinX.push_back(m_pDB->m_pins[pinID].absX);
		//m_regionNets[i].m_fixedPinY.push_back(m_pDB->m_pins[pinID].absY);
		double pinX = m_pDB->m_pins[pinID].absX;
		double pinY = m_pDB->m_pins[pinID].absY;

		if( pinX <= m_Rgn.left ) //module in the left
		{
		    ++m_regionNets[i].m_nLeftModules;					
		}
		else if( pinX <= region_centerX )
		{
		    m_regionNets[i].m_fixedPinX.push_back( pinX );
		    ++m_regionNets[i].m_nCLeftModules;
		}
		else if( pinX < m_Rgn.right )
		{
		    m_regionNets[i].m_fixedPinX.push_back( pinX );
		    ++m_regionNets[i].m_nCRightModules;
		}
		else //module in the right
		{
		    ++m_regionNets[i].m_nRightModules;					
		}


		if(pinY<=m_Rgn.bottom) //module in the left
		{
		    ++m_regionNets[i].m_nBottomModules;					
		}
		else if(pinY<=region_centerY)
		{
		    m_regionNets[i].m_fixedPinY.push_back(pinY);
		    ++m_regionNets[i].m_nCBottomModules;
		}
		else if(pinY<m_Rgn.top)
		{
		    m_regionNets[i].m_fixedPinY.push_back(pinY);
		    ++m_regionNets[i].m_nCTopModules;
		}
		else //module in the right
		{
		    ++m_regionNets[i].m_nTopModules;					
		}
	    }
	}

	//determine if the net is global
	if( m_regionNets[i].m_nLeftModules>0 && m_regionNets[i].m_nRightModules>0 )
	{
	    m_regionNets[i].m_isGlobalX = true;
	}
	if( m_regionNets[i].m_nBottomModules>0 && m_regionNets[i].m_nTopModules>0 )
	{
	    m_regionNets[i].m_isGlobalY = true;
	}
    }

    //check
    for(int i=0; i<(int)this->m_regionNets.size(); i++)
    {
	if( m_regionNets[i].m_localModules.size() <= 0 )
	{
	    cout<<"\n=======================ERROR===============NET ID:"<<m_regionNets[i].m_netID<<" \n";
	    cout<<"\n pinSize:"<<m_pDB->m_nets[m_regionNets[i].m_netID].size();
	    for(unsigned int j=0; j<m_pDB->m_nets[m_regionNets[i].m_netID].size(); j++)
	    {
		int pinID=m_pDB->m_nets[m_regionNets[i].m_netID][j];
		int moduleID=m_pDB->m_pins[pinID].moduleId;
		cout<<" mod::"<<moduleID<<" mname:"<<m_pDB->m_modules[moduleID].m_name;
		set<int>::iterator it;
		it=localModuleIDSet.find(moduleID);

		if( it!=localModuleIDSet.end() )
		{
		    cout<<" in set";
		}
		else
		    cout<<" NOT in set";
		cout<<"\n";
	    }
	    cout<<"\n";

	    for(unsigned int k=0; k<m_moduleIDs.size(); k++)
	    {
		for(unsigned int j=0; j<m_pDB->m_modules[m_moduleIDs[k]].m_netsId.size(); j++)
		{
		    if(m_pDB->m_modules[m_moduleIDs[k]].m_netsId[j]==m_regionNets[i].m_netID)
			cout<<" FIND IN NET mid:"<<m_moduleIDs[k]<<" ";
		}
	    }

	    bool find=false;
	    for(unsigned int k=0; k<m_moduleIDs.size(); k++)
	    {

		//if((m_pDB->m_modules[m_moduleIDs[i]].m_cx<m_Rgn.left))
		//	m_pDB->

		for(unsigned int j=0; j<m_pDB->m_modules[m_moduleIDs[k]].m_netsId.size(); j++)
		{
		    if(m_regionNets[i].m_netID==m_pDB->m_modules[m_moduleIDs[k]].m_netsId[j])
		    {
			find=true;
			cout<<"\nFind!!";

			cout << " failRid:" << m_regionNets[i].m_netID 
			    << " rightRid:"<< m_regionNets[i].m_netID
			    << " module:"  << m_moduleIDs[k]
			    << " mname"    << m_pDB->m_modules[m_moduleIDs[k]].m_name;

			set<int>::iterator it;
			it=localModuleIDSet.find(m_moduleIDs[k]);

			if( it!=localModuleIDSet.end() )
			{
			    cout<<" in set";
			}
			break;
		    }
		}

	    }

	    cout<<"\n rss:"<<m_regionNets.size();
	    cout<<"\n i:"<<i<<" rs:"<<m_regionNets[i].m_localModules.size()<<"\n";
	}
	assert(m_regionNets[i].m_localModules.size()>0);
    }

}

double CPlaceDBQPRegion::getMaxSize()
{
    double width=m_Rgn.right-m_Rgn.left;
    double height=m_Rgn.top-m_Rgn.bottom;
    assert(width*height>=0);

    if(width>height)
	return width;
    else
	return height;
}

void CPlaceDBQPRegion::safeMoveCenter(int i,double cx, double cy)
{
    //int i=moduleID;
    if( m_pDB->m_modules[i].m_isFixed == true )
	return;

    double x = cx - m_pDB->m_modules[i].m_width*(double)0.5;
    double y = cy - m_pDB->m_modules[i].m_height*(double)0.5;


    if( (x + m_pDB->m_modules[i].m_width) > m_Rgn.right)
    {
	x = m_Rgn.right - m_pDB->m_modules[i].m_width;
    }
    else if(x < m_Rgn.left)
    {
	x = m_Rgn.left;
    }
    if( (y + m_pDB->m_modules[i].m_height)>m_Rgn.top)
    {
	y = m_Rgn.top-m_pDB->m_modules[i].m_height;
    }
    else if(y<m_Rgn.bottom)
    {
	y = m_Rgn.bottom;
    }

    m_pDB->SetModuleLocation( i, x, y );

}

CPlaceDBQPPlacer::CPlaceDBQPPlacer(CPlaceDB& db)
{
	m_pDB=&db;
}

CPlaceDBQPPlacer::~CPlaceDBQPPlacer(void)
{
}

void CPlaceDBQPPlacer::safeMoveCenter( int i, double cx, double cy )
{
//	int i=moduleID;
	if(m_pDB->m_modules[i].m_isFixed==true)
		return;

	double x = cx - m_pDB->m_modules[i].m_width * (double)0.5;
	double y = cy - m_pDB->m_modules[i].m_height * (double)0.5;


	if( ( x + m_pDB->m_modules[i].m_width ) > m_pDB->m_coreRgn.right )
	{
		x = m_pDB->m_coreRgn.right - m_pDB->m_modules[i].m_width;
	}
	else if( x < m_pDB->m_coreRgn.left )
	{
		x = m_pDB->m_coreRgn.left;
	}
	if( ( y + m_pDB->m_modules[i].m_height ) > m_pDB->m_coreRgn.top )
	{
		y = m_pDB->m_coreRgn.top - m_pDB->m_modules[i].m_height;
	}
	else if( y < m_pDB->m_coreRgn.bottom )
	{
		y=m_pDB->m_coreRgn.bottom;
	}

	m_pDB->SetModuleLocation( i,  x, y );

}
void CPlaceDBQPPlacer::findFloatingNet()
{
	//cout<<"\nStart check floating net!!";
	m_isFloatingNets.clear();
	m_isFloatingNets.resize( m_pDB->m_nets.size() ,true);

	for(unsigned int i=0; i<m_pDB->m_nets.size(); i++)
	{
		for(unsigned int j=0; j<m_pDB->m_nets[i].size(); j++)
		{
			int pinID=m_pDB->m_nets[i][j];
			int modID=m_pDB->m_pins[pinID].moduleId;
			if(m_pDB->m_modules[modID].m_isFixed==true)
			{
				m_isFloatingNets[i]=false;
				break;
			}
		}
	}
	int fcount=0;
	int fmcount=0;
	isModule_float.resize(m_pDB->m_modules.size(),true);
	bool end=false;
	while(end==false)
	{
	    bool change=false;
	    for(unsigned int i=0; i<m_pDB->m_modules.size(); i++)
	    {
		if(isModule_float[i]==true)
		{
		    if(m_pDB->m_modules[i].m_isFixed==false)
		    {
			bool isFix=false;
			for(unsigned int j=0; j<m_pDB->m_modules[i].m_netsId.size(); j++)
			{
			    if(m_isFloatingNets[m_pDB->m_modules[i].m_netsId[j]]==false)
			    {
				isFix=true;
				break;
			    }
			}
			if(isFix==true)
			{
			    //cout<<" fix:"<<i<<" ";
			    for(unsigned int j=0; j<m_pDB->m_modules[i].m_netsId.size(); j++)
			    {
				m_isFloatingNets[m_pDB->m_modules[i].m_netsId[j]]=false;

			    }
			    isModule_float[i]=false;
			    change=true;
			}
		    }
		    else
		    {
			isModule_float[i]=false;
		    }
		}

	    }

	    if(change==false)
	    {
		end=true;
	    }
	}

	fcount=0;
	for(unsigned int i=0; i<this->m_isFloatingNets.size(); i++)
	{
		if(m_isFloatingNets[i]==true)
			++fcount;
	}
	for(unsigned int i=0; i<isModule_float.size(); i++)
	{
		if(isModule_float[i]==true)
			++fmcount;
	}

	if( fcount > 0 || fmcount > 0 )
	    cout<<"\nEnd of checking floaging Net. # of Floaging NET:"<<fcount<<" floaging module:"<<fmcount<<"\n";

}



void CPlaceDBQPPlacer::QPplace()
{
    double top    = m_pDB->m_coreRgn.top;
    double left   = m_pDB->m_coreRgn.left;
    double bottom = m_pDB->m_coreRgn.bottom;
    double right  = m_pDB->m_coreRgn.right;
    
    //init something
    //isModuleTempFix.resize(m_pDB->m_modules.size(),false);
    //for(unsigned int i=0; i<m_pDB->m_modules.size(); i++)
    //{
    //	if(m_pDB->m_modules[i].m_isFixed==true)
    //	{
    //		isModuleTempFix[i]=true;
    //	}
    //}

    //------------------------------------------------
    // Find floating net
    //------------------------------------------------
    //	cout<<"\nStart Finding floating net:";
    findFloatingNet();
    //	cout<<"\nFinish finding floating net";
    
    //------------------------------------------------
    // Initialize the top level region
    //------------------------------------------------
    //	level_now.m_levelID=0;
    //	level_now.m_regions.resize(1);
    
    CPlaceDBQPRegion topRegion( *m_pDB, *this );

    //	level_now.m_regions[0]=topRegion;
    topRegion.m_moduleIDs.reserve( m_pDB->m_modules.size() );
    
    //insert all movable modules into the top region
    for( unsigned int i=0; i<m_pDB->m_modules.size(); i++ )
    {
    	if( m_pDB->m_modules[i].m_isFixed == false )
    	{
    	    topRegion.m_moduleIDs.push_back( i );
    	}
    	else
    	{
    	    topRegion.m_fixModuleIDs.push_back( i );
    	}
    }
    topRegion.setRgn( top, bottom, left, right );

    // (donnie) 2006-02-04 remove msg
    //m_pDB->CalcHPWL();
    //double wl=m_pDB->GetHPWLp2p();
    //cout<<"\nInitial WL:"<<wl;
    
    QPplaceCore( topRegion );
    //m_pDB->CalcHPWL();
}

// (donnie) 2006-02-04
void CPlaceDBQPPlacer::QPplace( const double& left, const double& bottom, const double& right, const double& top, 
	                        const vector<int>& movModules, const vector<int>& fixModules )
{
    findFloatingNet();

    CPlaceDBQPRegion topRegion( *m_pDB, *this );

    topRegion.m_moduleIDs = movModules;
    topRegion.m_fixModuleIDs = fixModules;
    topRegion.setRgn( top, bottom, left, right );

    QPplaceCore( topRegion );
}

void CPlaceDBQPPlacer::QPplaceCore( CPlaceDBQPRegion& topRegion )
{
    topRegion.initRegionNet();
    //cout << "\nBuild matrix...";
    topRegion.buildMatrix();
    //cout << " Done!!\nStart QP...";
    topRegion.pureQP();
    //cout << " Done!!";
}

void CPlaceDBQPRegion::buildMatrix()
{

    //1.establish move points (add star-node)
    //2.fill into matrix
    solverX.mapA.clear();
    solverY.mapA.clear();
    vec_Bx.clear();
    vec_By.clear();
    vec_mvY.clear();
    vec_mvX.clear();	
    vec_Kx.clear();
    vec_Ky.clear();
    vec_SPx.clear();
    vec_SPy.clear();
    vec_Kx.resize(m_moduleIDs.size(),0);
    vec_Ky.resize(m_moduleIDs.size(),0);
    vec_SPx.resize(m_moduleIDs.size(),0);
    vec_SPy.resize(m_moduleIDs.size(),0);

    if(m_moduleIDs.size()==0) //if there is no movable module inside
	return;

    double fc = 0.01;
    double globalWeight = 1;
    vector<int> rgnNetDegree;
    vector<int> rgnNet2StarPos;

    //----------------------------------------------------------------------
    // Find the # of multi-terminal net
    //----------------------------------------------------------------------

    rgnNetDegree.resize(m_regionNets.size());
    rgnNet2StarPos.resize(m_regionNets.size());
    int multinet_count=0;
    for(unsigned int i=0; i<m_regionNets.size(); i++)
    {
	rgnNetDegree[i]=m_regionNets[i].m_localModules.size() + 
	    m_regionNets[i].m_nLeftModules  + 
	    m_regionNets[i].m_nRightModules +
	    m_regionNets[i].m_fixedPinX.size();
	assert(m_regionNets[i].m_localModules.size()>0);
	if( rgnNetDegree[i]>2)
	{
	    rgnNet2StarPos[i] = multinet_count;
	    ++multinet_count;
	}
	else
	{
	    rgnNet2StarPos[i] = -1;
	}
    }


    //----------------------------------------------------------------------
    //build matrix A and vector Bx, By and initial X,Y vector
    //----------------------------------------------------------------------

    //initial X
    int size_of_matrix = m_moduleIDs.size() + multinet_count;

    double region_centerX = (m_Rgn.left + m_Rgn.right) / 2;
    vec_mvX.resize( size_of_matrix, region_centerX );
    vec_Bx.resize( size_of_matrix, 0 );

    for(int i=0; i<size_of_matrix; i++)
    {
	solverX.setValue( i, i, 0 );
    }

    //for all regionNets, fill data into matrix A and vector B

    for(unsigned int i=0; i<m_regionNets.size(); i++)
    {
	if( rgnNetDegree[i]>2) // 3--k terminal net
	{

	    assert(rgnNet2StarPos[i]!=-1);

	    double netWeight=1;
	    double netWeightX=netWeight;

	    //calc net weight
	    if( m_regionNets[i].m_isGlobalX == true )
	    {
		netWeightX = globalWeight * netWeight;
	    }

	    netWeightX *= ( (double)rgnNetDegree[i] / ((double)rgnNetDegree[i] - 1) );


	    //add value to matrix A (w.r.t star node)
	    int starPos = m_moduleIDs.size() + rgnNet2StarPos[i];
	    assert( starPos < size_of_matrix );
	    solverX.setValue( starPos, starPos, netWeightX * rgnNetDegree[i] );

	    double fixSumX=0;

	    for( unsigned int j=0; j<m_regionNets[i].m_fixedPinX.size(); j++ )
	    {
		fixSumX += m_regionNets[i].m_fixedPinX[j];
	    }


	    //cout<<"\n FX:"<<fixSumX<<" FY:"<<fixSumY;
	    double bx_vec_sum = m_Rgn.left * m_regionNets[i].m_nLeftModules +
		                m_Rgn.right * m_regionNets[i].m_nRightModules + fixSumX;


	    //calc value of vector B (the information of fixed pin)
	    vec_Bx[starPos] = netWeightX * bx_vec_sum;

	    for( unsigned int j=0; j<m_regionNets[i].m_localModules.size(); j++ )
	    {
		//module position in the vector X,Y and matrix A
		int modulePos = realModuleID2regionModuleID[ m_regionNets[i].m_localModules[j] ];

		//x
		solverX.increment( modulePos, modulePos, netWeightX );
		solverX.decrement( modulePos, starPos,netWeightX );
		solverX.decrement( starPos, modulePos,netWeightX );

		vec_Bx[starPos]   += netWeightX * (m_pDB->m_pins[m_regionNets[i].m_localModulesPinID[j]].xOff);
		vec_Bx[modulePos] -= netWeightX * (m_pDB->m_pins[m_regionNets[i].m_localModulesPinID[j]].xOff);
	    }

	}
	else if(rgnNetDegree[i]>1) // 2-terminal net, may be movable-movable or fix-movable
	{
	    //calc net weight
	    double netWeight=1;
	    double netWeightX=netWeight;
	    if(m_regionNets[i].m_isGlobalX==true)
	    {
		netWeightX = globalWeight * netWeight;
	    }
	    //movable-movable
	    if( m_regionNets[i].m_localModules.size() == 2 ) 
	    {
		int pos0=realModuleID2regionModuleID[m_regionNets[i].m_localModules[0]];
		int pos1=realModuleID2regionModuleID[m_regionNets[i].m_localModules[1]];

		vec_Bx[pos0] += netWeightX * 
		                ( m_pDB->m_pins[m_regionNets[i].m_localModulesPinID[1]].xOff - 
				  m_pDB->m_pins[m_regionNets[i].m_localModulesPinID[0]].xOff );
		vec_Bx[pos1] += netWeightX * 
		                ( m_pDB->m_pins[m_regionNets[i].m_localModulesPinID[0]].xOff - 
				  m_pDB->m_pins[m_regionNets[i].m_localModulesPinID[1]].xOff );

		solverX.increment( pos0, pos0, netWeightX );
		solverX.increment( pos1, pos1, netWeightX );
		solverX.decrement( pos0, pos1, netWeightX );
		solverX.decrement( pos1, pos0, netWeightX );
	    }
	    else //fixed Pin - movable module
	    {
		int pos0 = realModuleID2regionModuleID[m_regionNets[i].m_localModules[0]];
		double fixSumX=0;
		for(unsigned int j=0; j< m_regionNets[i].m_fixedPinX.size(); j++)
		{
		    fixSumX+=m_regionNets[i].m_fixedPinX[j];
		}

		//assertion: exactly 1 fixed pin
		assert( (m_regionNets[i].m_nLeftModules+m_regionNets[i].m_nRightModules +
			 m_regionNets[i].m_fixedPinX.size() ) == 1 );

		double bx_vec_sum = m_Rgn.left * m_regionNets[i].m_nLeftModules +
		                    m_Rgn.right * m_regionNets[i].m_nRightModules + fixSumX;

		vec_Bx[pos0] += netWeightX * 
		                ( bx_vec_sum - m_pDB->m_pins[m_regionNets[i].m_localModulesPinID[0]].xOff);
		solverX.increment( pos0, pos0, netWeightX );
	    }

	}
	else
	{
	    //1 terminal net !? Don't care.
	}
    }

    //for all floating module, add a pseudo fix pin to region center
    for( unsigned int i=0; i<vec_Bx.size(); i++ )
    {
	//assertion: starNode shouldn't be floating
	double val=0;
	solverX.getValue(i,i,val);
	if(val==0)
	{
	    solverX.setValue(i,i,fc);
	    vec_Bx[i]=fc*region_centerX;
	}
	else if(i<m_moduleIDs.size())
	{
	    if(m_pPqap->isModule_float[m_moduleIDs[i]]==true) //floating module
	    {
		solverX.increment(i,i,fc);
		vec_Bx[i]+=fc*region_centerX;
	    }
	}
    }


    //	checkMatrix(solverX, vec_Bx);



    //------------------------------------------
    // QP on y
    //------------------------------------------


    //----------------------------------------------------------------------
    // Find the # of multi-terminal net
    //----------------------------------------------------------------------
    rgnNetDegree.clear();
    rgnNet2StarPos.clear();
    rgnNetDegree.resize(m_regionNets.size());
    rgnNet2StarPos.resize(m_regionNets.size());
    multinet_count=0;
    for( unsigned int i=0; i<m_regionNets.size(); i++ )
    {

	rgnNetDegree[i] = m_regionNets[i].m_localModules.size() + 
	                  m_regionNets[i].m_nBottomModules  + 
	                  m_regionNets[i].m_nTopModules +
	                  m_regionNets[i].m_fixedPinY.size()
	                  ;
	assert( m_regionNets[i].m_localModules.size() > 0 );
	if( rgnNetDegree[i] > 2 )
	{
	    rgnNet2StarPos[i] = multinet_count;
	    ++multinet_count;
	}
	else
	{
	    rgnNet2StarPos[i] = -1;
	}
    }

    //----------------------------------------------------------------------
    //build matrix A and vector Bx, By and initial X,Y vector
    //----------------------------------------------------------------------

    //initial Y
    size_of_matrix = m_moduleIDs.size() + multinet_count;
    double region_centerY = (m_Rgn.top+m_Rgn.bottom)/2;
    vec_mvY.resize(size_of_matrix,region_centerY);
    vec_By.resize(size_of_matrix,0);
    for( int i=0; i<size_of_matrix; i++ )
    {
	solverY.setValue(i,i,0);
    }

    //for all regionNets, fill data into matrix A and vector B

    for(unsigned int i=0; i<m_regionNets.size(); i++)
    {
	if( rgnNetDegree[i]>2) //3-k terminal net
	{

	    assert(rgnNet2StarPos[i]!=-1);

	    double netWeight=1;
	    double netWeightY=netWeight;

	    //calc net weight
	    if(m_regionNets[i].m_isGlobalY==true)
	    {
		netWeightY=globalWeight*netWeight;
	    }
	    netWeightY*=( (double)rgnNetDegree[i]/((double)rgnNetDegree[i]-1) );

	    //add value to matrix A (w.r.t star node)
	    int starPos=m_moduleIDs.size()+rgnNet2StarPos[i];
	    assert(starPos<size_of_matrix);
	    solverY.setValue(starPos,starPos,netWeightY*rgnNetDegree[i]);			

	    double fixSumY=0;
	    for(unsigned int j=0; j< m_regionNets[i].m_fixedPinY.size(); j++)
	    {
		fixSumY+=m_regionNets[i].m_fixedPinY[j];
	    }

	    double by_vec_sum=m_Rgn.bottom*m_regionNets[i].m_nBottomModules+m_Rgn.top*m_regionNets[i].m_nTopModules+fixSumY;

	    //calc value of vector B (the information of fixed pin)
	    vec_By[starPos]=netWeightY *by_vec_sum;
	    //			cout<<"\n FbX:"<<vec_Bx[starPos]<<" FbY:"<<vec_By[starPos]<<" starPos:"<<starPos;


	    for(unsigned int j=0; j< m_regionNets[i].m_localModules.size(); j++)
	    {
		//module position in the vector X,Y and matrix A
		int modulePos=realModuleID2regionModuleID[m_regionNets[i].m_localModules[j]];

		//y
		solverY.increment(modulePos,modulePos,netWeightY);
		solverY.decrement(modulePos,starPos,netWeightY);
		solverY.decrement(starPos,modulePos,netWeightY);
		vec_By[starPos]+=netWeightY * (m_pDB->m_pins[m_regionNets[i].m_localModulesPinID[j]].yOff);
		vec_By[modulePos]-=netWeightY * (m_pDB->m_pins[m_regionNets[i].m_localModulesPinID[j]].yOff);
	    }

	}
	else if(rgnNetDegree[i]>1) // 2-terminal net, may be movable-movable or fix-movable
	{
	    //calc net weight
	    double netWeight=1;
	    double netWeightY=netWeight;
	    if(m_regionNets[i].m_isGlobalY==true)
	    {
		netWeightY=globalWeight*netWeight;
	    }

	    //movable-movable
	    if(m_regionNets[i].m_localModules.size()==2) 
	    {
		int pos0=realModuleID2regionModuleID[m_regionNets[i].m_localModules[0]];
		int pos1=realModuleID2regionModuleID[m_regionNets[i].m_localModules[1]];

		vec_By[pos0]+=netWeightY * (m_pDB->m_pins[m_regionNets[i].m_localModulesPinID[1]].yOff - m_pDB->m_pins[m_regionNets[i].m_localModulesPinID[0]].yOff);
		vec_By[pos1]+=netWeightY * (m_pDB->m_pins[m_regionNets[i].m_localModulesPinID[0]].yOff - m_pDB->m_pins[m_regionNets[i].m_localModulesPinID[1]].yOff);

		solverY.increment(pos0,pos0,netWeightY);
		solverY.increment(pos1,pos1,netWeightY);
		solverY.decrement(pos0,pos1,netWeightY);
		solverY.decrement(pos1,pos0,netWeightY);
	    }
	    else //fixed Pin - movable module
	    {
		int pos0=realModuleID2regionModuleID[m_regionNets[i].m_localModules[0]];
		double fixSumY=0;
		for(unsigned int j=0; j< m_regionNets[i].m_fixedPinY.size(); j++)
		{
		    fixSumY+=m_regionNets[i].m_fixedPinY[j];
		}
		//assertion: exactly 1 fixed pin
		assert( (m_regionNets[i].m_nBottomModules+m_regionNets[i].m_nTopModules+m_regionNets[i].m_fixedPinY.size())==1 );
		double by_vec_sum=m_Rgn.bottom*m_regionNets[i].m_nBottomModules+m_Rgn.top*m_regionNets[i].m_nTopModules+fixSumY;
		vec_By[pos0]+=netWeightY * ( by_vec_sum - m_pDB->m_pins[m_regionNets[i].m_localModulesPinID[0]].yOff);
		solverY.increment(pos0,pos0,netWeightY);
	    }
	}
	else
	{
	    //1 terminal net !? Don't care.
	}
    }

    //for all floating module, add a pseudo fix pin to region center
    for(unsigned int i=0; i<vec_By.size(); i++)
    {
	//assertion: starNode shouldn't be floating

	double val=0;
	solverY.getValue(i,i,val);
	if(val==0)
	{
	    solverY.setValue(i,i,fc);
	    vec_By[i]=fc*region_centerY;
	}
	else if(i<m_moduleIDs.size())
	{
	    if(m_pPqap->isModule_float[m_moduleIDs[i]]==true) //floating module
	    {
		solverY.increment(i,i,fc);
		vec_By[i]+=fc*region_centerY;
	    }
	}
    }


    /* (???)
    //solve the AX=B
    solverY.solverQP(10000,vec_By,vec_mvY);

    //move modules in this region
    for(unsigned int i=0; i<m_moduleIDs.size(); i++)
    {
	//float cx=m_pDB->m_modules[m_moduleIDs[i]].m_x;
	//float cy=vec_mvY[i]-m_pDB->m_modules[m_moduleIDs[i]].m_height*(float)0.5;
	//	cout<<" Module:"<<m_moduleIDs[i]<<" vx:"<<vec_mvX[i]<<" vy:"<<vec_mvY[i]<<"  ";
	assert(m_pDB->m_modules[m_moduleIDs[i]].m_isFixed==false);
	//m_pDB->SetModuleLocation(m_moduleIDs[i],cx,cy);
	safeMoveCenter(m_moduleIDs[i],m_pDB->m_modules[m_moduleIDs[i]].m_cx,vec_mvY[i]);
    }
    */


}

void CPlaceDBQPRegion::pureQP()
{
    //solve the AX=B
    solverX.solverQP(10000,vec_Bx,vec_mvX);
    //solve the AX=B
    solverY.solverQP(10000,vec_By,vec_mvY);

    for(unsigned int i=0; i<m_moduleIDs.size(); i++)
    {
	//assert(m_pDB->m_modules[m_moduleIDs[i]].m_isFixed==false);
	safeMoveCenter(m_moduleIDs[i],vec_mvX[i],vec_mvY[i]);
	//assert(m_pDB->m_modules[m_moduleIDs[i]].m_cy>m_Rgn.bottom);
    }
}

//use QP to calc new module location according to the diffusion position


//-------------------------------------------------
// Log: 2005.11.28
// Finish the diffusion-QP code, but still very, very buggy :(
//
