#ifndef LEFDEF_H
#define LEFDEF_H

#include <vector>
#include <cstdio>
#include <map>
using std::FILE;
using std::map;
using std::vector;

#include "placedb.h"

class CParserLEFDEF
{
    public:
	CParserLEFDEF();
	int ReadLEF( char* file1, CPlaceDB& fplan );	// in lefrw.cpp
	int ReadDEF( char* file1, CPlaceDB& fplan );	// in defrw.cpp
        int WriteDEF( const char* oldFile, const char* newFile, CPlaceDB& fplan );    // def out (donnie, 2005-10-20)
	
	int AddPin( int moduleId, string pinName, float xOff, float yOff ); 
	void PrintMacros( bool showPins );

	//private:
	vector<Module>  m_modules;
	vector<Pin>     m_pins;
	map<string,int> m_moduleNameMap;
	double		m_minBlockHeight;   // 2005-12-09
	double          m_coreSiteWidth;
	double	        m_coreSiteHeight;
	double		m_lefUnit;
	double		m_defUnit;
	int             m_nComponents;	    // # components in DEF

    private:
	void CreateModuleNameMap();
	void WriteComponents( ostream& out, CPlaceDB& fplan );    // for def out (donnie, 2005-10-20)
};

extern FILE* fout;
extern int userData;
extern CPlaceDB* g_fplan;  // Pointer to the fplan object to read DEF

void dataError();
void* mallocCB(int size); 
void* reallocCB(void* name, int size);
void freeCB(void* name);
void lineNumberCB(int lineNo);
char* orientStr(int orient);
int   orientInt(char* orient);

#endif
