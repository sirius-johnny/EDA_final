#ifndef PLACEDBQP_H
#define PLACEDBQP_H
#pragma once
#include "placedb.h"
#include "solverQP.h"
#include "util.h"

//--------------------------------------------------------------------------------------------------------------------------------------------
//  CPlaceDBQP: do simple QP placement on a given placeDB
// 
//  USAGE:
//	CPlaceDB	placedb;
//	CPlaceDBQPPlacer QPplacer(placedb);
//  QPplacer.QPplace();
//	
//
//--------------------------------------------------------------------------------------------------------------------------------------------


class CPlaceDBQPPlacer;


class CPlaceDBQPRegionNet
{
	//----------------------------------------------------
	// Modify log: 
	// 11/09: add member function "reset()" to reset the data about fixed pin (terminal propagation)
	// 11/09: Add m_nCLeftModules... to give data to hMetis partitioner
	//----------------------------------------------------
public:
	CPlaceDBQPRegionNet(int netID);
	~CPlaceDBQPRegionNet(void);

	void reset();//reset all fix pin information

	bool m_isGlobalX;
	bool m_isGlobalY;

	int m_netID;

	int m_nLeftModules;
	int m_nRightModules;
	int m_nTopModules;
	int m_nBottomModules;

	int m_nCLeftModules; //# of pins between the left edge and the center of the region
	int m_nCRightModules;
	int m_nCTopModules;
	int m_nCBottomModules;


	vector<double> m_fixedPinX;
	vector<double> m_fixedPinY;

	vector<int> m_localModules;
	vector<int> m_localModulesPinID;
	void print()
	{
		cout<<"\nnetID:"<<m_netID;
		cout<<"\n Modules:";
		for(unsigned int i=0; i<m_localModules.size(); i++)
		{
			cout<<m_localModules[i]<<", ";
		}
		cout<<"\nnOfTop:"<<m_nTopModules<<" nOfBottom:"<<m_nBottomModules<<" nOfLeft:"<<m_nLeftModules<<" nOfRight:"<<m_nRightModules;
		cout<<"\nFixPinX:";
		for(unsigned int i=0; i<m_fixedPinX.size(); i++)
		{
			cout<<m_fixedPinX[i]<<", ";
		}
		cout<<"\nFixPinY:";
		for(unsigned int i=0; i<m_fixedPinY.size(); i++)
		{
			cout<<m_fixedPinY[i]<<", ";
		}
		cout<<"\n";
	}
};


class CPlaceDBQPRegion
{
public:
	CPlaceDBQPRegion(void);
	CPlaceDBQPRegion(CPlaceDB& db,CPlaceDBQPPlacer& paqp);
	~CPlaceDBQPRegion(void);


	CPlaceDB * m_pDB;
	CQP solverX; //qp matrix X
	CQP solverY; //qp matrix Y
	vector<double> vec_Bx;
	vector<double> vec_By;
	vector<double> vec_mvY;
	vector<double> vec_mvX;
	vector<double> vec_Kx;
	vector<double> vec_Ky;
	vector<double> vec_SPx;
	vector<double> vec_SPy;


	double density;

	vector<int> m_moduleIDs; //modules inside the region
	vector<int> m_fixModuleIDs; //fixed modules inside the region
	map<int,int> realModuleID2regionModuleID; // map the real moduleID to position in m_moduleIDs

	void buildMatrix();
	void pureQP();


	void safeMoveCenter(int i,double cx, double cy);

	void initRegionNet(); //after insert modules, build the m_regionNets vector

	void setRgn(double top, double bottom, double left, double right); //set region boundary
	double getWidth()
	{
		return m_Rgn.right-m_Rgn.left;
	}
	double getHeight()
	{
		return m_Rgn.top-m_Rgn.bottom;
	}
	double getArea()
	{
		return (m_Rgn.right-m_Rgn.left)*(m_Rgn.top-m_Rgn.bottom);

	}


	double getMaxSize();


	CRect  m_Rgn; //region coordinate
	set<int> localModuleIDSet;

private:
	vector<CPlaceDBQPRegionNet> m_regionNets;
	CPlaceDBQPPlacer * m_pPqap;

	//static bool sortX(const int& i, const int& j)//sort modules according to X 
	//{
	//	return m_pDB->m_modules[i].m_x<m_pDB->m_modules[j].m_x;
	//}
	//static bool sortY(const int& i, const int& j)//sort modules according to X 
	//{
	//	return m_pDB->m_modules[i].m_y<m_pDB->m_modules[j].m_y;
	//}

};

class CPlaceDBQPLevel
{
public:
	CPlaceDBQPLevel(void);
	~CPlaceDBQPLevel(void);

	int m_levelID;
	vector<CPlaceDBQPRegion> m_regions;		//index for region ID
	vector<int> m_moduleID2regionID;	// index: moduleID in placeDB, value:regionID in PaqpLevel

	void levelQP();


};
class CPlaceDBQPPlacer
{
public:
	CPlaceDBQPPlacer( CPlaceDB& db);
	~CPlaceDBQPPlacer(void);


	// (donnie) 2006-02-04 change the arguments of QPplace()
	void QPplace();
	void QPplace( const double& left, const double& bottom, const double& right, const double& top, 
	              const vector<int>& movModules, const vector<int>& fixModules );

private:
	void QPplaceCore( CPlaceDBQPRegion& topRegion );
	
public:
	vector<CPlaceDBQPLevel> m_levels;

	CPlaceDBQPLevel level_now;
	CPlaceDBQPLevel level_next;



	void findFloatingNet();

	void safeMoveCenter(int i,double cx, double cy);
	void safeMove(int i,double x, double y);

//	double globalWeight;
	//CParamPlacement param;   // store parameters

//	void roughPlace();//a test function, still buggy and no use 

	vector<bool> m_isFloatingNets;
	vector<bool> isModule_float;
	vector<bool> isModuleTempFix;		//fixed by Paqp.db





private:

//	void clearFixMarco(CPlaceDBQPLevel& level);
//	CPlaceDBQPLevel mergeRegion(CPlaceDBQPLevel level);
    CPlaceDB * m_pDB;
	bool moduleInNet(int NetId,int ModuleId)
	{
		//bool inNet=false;
		for(int i=0; i<(int)m_pDB->m_nets[NetId].size(); i++)
		{
			if(m_pDB->m_pins[m_pDB->m_nets[NetId][i]].moduleId==ModuleId)
				return true;
		}
		return false;
	}
	//void checkLegal()
	//{
	//	cout<<"\n=========Begin Check Legal!!========\n";
	//	for(unsigned int i=0; i<m_pDB->m_modules.size(); i++)
	//	{
	//		for( unsigned int j=0; j<m_pDB->m_modules[i].m_netsId.size(); j++)
	//		{
	//			if(moduleInNet(m_pDB->m_modules[i].m_netsId[j],i)==false)
	//			{
	//				cout<<"\nFail: module:"<<i<<" module name"<<m_pDB->m_modules[i].m_name<<" net:"<<m_pDB->m_modules[i].m_netsId[j];
	//			}
	//		}
	//	}
	//	cout<<"\n======Finish Check Legal!!============\n";
	//}
	//void dbDiff(CPlaceDB& db1,CPlaceDB& db2)
	//{
	//	cout<<"\n=========Begin Check Diff!!========\n";
	//	if(db1.m_modules.size()!=db2.m_modules.size())
	//	{
	//		cout<<"\nModule Size Difference db1:"<<db1.m_modules.size()<<" db2:"<<db2.m_modules.size();
	//	}
	//	else
	//	{
	//		for(unsigned int i=0; i<db1.m_modules.size(); i++)
	//		{
	//			if(db1.m_modules[i].m_netsId!=db2.m_modules[i].m_netsId)
	//			{
	//				cout<<"\nFail! module#:"<<i;
	//			}
	//		}
	//	}
	//	cout<<"\n======Finish Check Diff!!============\n";
	//}
};

/*-----------------------------------------------------------------
Coding Log

2005.11.06
DONE: partition
TODO: algorithms to handle marco (legal when fix?), re-partition

2005.11.10
DONE: add hMetis partition
DONE: Modify legaizer (CRow::insert_blockage,legalizeSubset(int WINDOW, const vector<bool>& isLegal, const vector<CRect>& preFix) ) to move out macros
DONE: remove CPlaceDBQPPlacer::m_levels to save memory 

------------------------------------------------------------------*/
#endif
