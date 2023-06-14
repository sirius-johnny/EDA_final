#ifndef RANDOMPLACE_H
#define RANDOMPLACE_H
#include "placedb.h"
 
/**
@author Indark
*/
class CRandomPlace{
public:
	CPlaceDB*  m_db;
	void place( double );
    CRandomPlace(CPlaceDB& db);

    ~CRandomPlace();

};

#endif
