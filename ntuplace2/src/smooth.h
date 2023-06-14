#ifndef SMOOTH_H
#define SMOOTH_H

#include "util.h"

#include <vector>

class GaussianSmooth
{
    public:
	void   Smooth( vector< vector<double> >& input );
	double GaussianDiscrete2D( double theta, int x, int y );
	void   Gaussian2D( double theta, int size );
	vector< vector< double > > m_kernel;
};


#endif
