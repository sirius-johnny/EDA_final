#include <cmath>
#include <set>
#include <vector>
#include <algorithm>
#include <cfloat>
#include <iostream>
#include <fstream>
using namespace std;

#include "placedb.h"
#include "MyNLP.h"
#include "smooth.h"
#include "TetrisLegal.h"
#include "placebin.h"
#include "ParamPlacement.h"

//double stepScaling = 0.2;
double truncationFactor = 1.0;
//double truncationFactor = 1.25;
//double truncationFactor = 0.5;

double time_grad_f = 0;
double time_f = 0;
double density;
double totalWL;

double time_wl = 0;
double time_update_grid = 0;
double time_grad_wl = 0;
double time_grad_potential = 0;
double time_update_bin = 0;
double time_up_potential = 0;
double time_exp = 0;
double time_sum_exp = 0;
double time_log = 0;

// for Euler method
double gradWL;
double stepSize;   

/*
void MyNLP::CreateExpTable()
{
    _expTablePrecision = 20000000;            // 20M * 8byte = 160M table
    _expTable.resize( _expTablePrecision+1 );    

    double minValue = 1e10;
    double maxValue = -1e10;
    for( unsigned int i=0; i<m_pDB->m_modules.size(); i++ )	
    {
	if( m_pDB->m_modules[i].m_cx > maxValue )     maxValue =  m_pDB->m_modules[i].m_cx;
	if( m_pDB->m_modules[i].m_cy > maxValue )     maxValue =  m_pDB->m_modules[i].m_cy;
	if( m_pDB->m_modules[i].m_cx < minValue )     minValue =  m_pDB->m_modules[i].m_cx;
	if( m_pDB->m_modules[i].m_cy < minValue )     minValue =  m_pDB->m_modules[i].m_cy;
    }
    if( m_pDB->m_coreRgn.left   < minValue )    minValue = m_pDB->m_coreRgn.left;
    if( m_pDB->m_coreRgn.bottom < minValue )    minValue = m_pDB->m_coreRgn.bottom;
    if( m_pDB->m_coreRgn.top    > maxValue )    maxValue = m_pDB->m_coreRgn.top;
    if( m_pDB->m_coreRgn.right  > maxValue )    maxValue = m_pDB->m_coreRgn.right;
  
    maxValue /= _alpha;
    minValue /= _alpha;

    
    // TODO: check boundary
//    if( minValue < 0.0 )
//        minValue = 0.0;
   
    // TODO: maxValue < 0
    
    double step = ( maxValue - minValue ) / (_expTablePrecision);
    printf( " exp table: min %f(*alpha=%f)  max %f(*alpha=%f)  step %f\n", 
	    minValue, minValue*_alpha,
	    maxValue, maxValue*_alpha, 
	    step );
    for( int i=0; i<_expTablePrecision+1; i++ )
    {
	_expTable[i] = exp( minValue + step * i ); 
    }
    _expTableMin = minValue;
    _expTableMax = maxValue;
    _expTableStep = step;
    //printf( "table[100] = %.10f\n", _expTable[100] );
    //printf( "exp(minValue+100*step) = %.10f\n", exp(minValue+step*100 ) );

}
*/

/*
double MyNLP::expTable( const double& value )
{
    double t_start = seconds();

    // TEST PRECISION && RUNTIME
    double res = exp(value);
    time_exp += seconds() - t_start; 
    return res;



    
//    double absValue = fabs( value );
    double absValue = value;
    
    // BOUNDARY CHECK START =====================
    if( absValue < _expTableMin )
    {
	printf( "ERROR! value= %f (%f) min= %f\n", value, value*_alpha, _expTableMin );
    }
    if( absValue > _expTableMax )
    {
//	printf( "WARNING! value= %f (%f) max= %f\n", value, value*_alpha, this->_expTableMax );
	return exp( value );
    }
    //assert( absValue >= _expTableMin );
    //assert( absValue <= _expTableMax );
    // BOUNDARY CHECK END =======================
    
    
    // BACK LOOKUP 
    //int index1 = static_cast<int>( ceil( (value - _expTableMin) / _expTableStep ) );
    //return _expTable[index1];		// 1% err

    // INTERPOLATION
    int index1 = static_cast<int>( floor( (absValue - _expTableMin) / _expTableStep ) );
    int index2 = index1 + 1 ;
    //assert( value >= _expTableMin + index1 * _expTableStep );
    //assert( value <  _expTableMin + index2 * _expTableStep );

    double diff = absValue - ( _expTableMin + index1 * _expTableStep );
    //assert( diff >= 0 );
    //assert( diff - _expTableStep <= 0 );
     
    double pos = _expTable[index1] + 
	   (_expTable[index2] - _expTable[index1]) * (diff) / _expTableStep;

    time_exp += seconds() - t_start; 
//    if( value > 0 )
	return pos;
//    else
//	return 1.0 / pos;
}
*/

/*
void MyNLP::CheckExpTablePrecision()
{
    double maxValue = 1500 / _alpha;
    double step = 0.0000001;
    double diff = 0.0;
    for( double i=-maxValue; i<maxValue; i+=step )
    {
	double e1 = expTable( i );
	double e2 = exp(i);
	//printf( "e2 = %f   e1 = %f   diff = %f\n", e2, e1, e2-e1 );
	diff += (e2-e1)*(e2-e1);
    }
    printf( "CheckExpTablePrecision %f-%f %f err= %g\n", -maxValue, maxValue, step, diff );
    
    double t1 = seconds();
    double e1;
    for( double i=-maxValue; i<maxValue; i+=step )
	e1 = expTable( i );
    t1 = seconds() - t1;
    double t2 = seconds();
    for( double i=-maxValue; i<maxValue; i+=step )
	e1 = exp( i );
    t2 = seconds() - t2;
    printf( "time_compare = %f %f\n", t1, t2 );
}
*/


/* Constructor. */
MyNLP::MyNLP( CPlaceDB& db )
    : _potentialGridR( 2 ),
      m_potentialGridSize( -1 ),
      m_targetUtil( 0.9 )
{
    m_lookAheadLegalization = false;
    m_earlyStop = false;
    m_topLevel = false;
    m_lastNLP = false;
    m_useBellPotentialForPreplaced = true;
    
    m_weightWire = 4.0;
    m_incFactor = 2.0;
    m_smoothR = 5;	// Gaussian smooth R
    m_smoothDelta = 1;

    target_nnb = 1.1;	// UNUSE
   
    //TODO: compute target density  (evenly distribute?)
    
    m_pDB = &db;
    InitModuleNetPinId();	    
    
    //alpha = 3; //m_pDB->m_rowHeight * 20;
    //_weightDensity = 0.01;
    //_weightWire    = 1000.0;
    //m_potentialGridSize      = 1000;

    // scale between 0 to 10
    const double range = 10.0;
    if( m_pDB->m_coreRgn.right > m_pDB->m_coreRgn.top )
    {
        //printf( "right > top\n" );
        m_posScale = range / m_pDB->m_coreRgn.right;
    }
    else
    {
        //printf( "right < top\n" );
        m_posScale = range / m_pDB->m_coreRgn.top;
    }

    _cellPotentialNorm.resize( m_pDB->m_modules.size() );

    x        = new double [ 2 * m_pDB->m_modules.size() ];
    xBest    = new double [ 2 * m_pDB->m_modules.size() ];
    //xBak     = new double [ 2 * m_pDB->m_modules.size() ];	// for stop checking (UNUSE NOW)
    //xBak2    = new double [ 2 * m_pDB->m_modules.size() ];	// for line search  (UNUSE NOW)
    _expX    = new double [ 2 * m_pDB->m_modules.size() ];
    _expPins = new double [ 2 * m_pDB->m_pins.size() ];
    x_l      = new double [ 2 * m_pDB->m_modules.size() ];
    x_u      = new double [ 2 * m_pDB->m_modules.size() ];
    
    m_usePin.resize( m_pDB->m_modules.size() );
    SetUsePin();
  
    m_nets_sum_exp_xi_over_alpha.resize( m_pDB->m_nets.size(), 0 );
    m_nets_sum_exp_yi_over_alpha.resize( m_pDB->m_nets.size(), 0 );
    m_nets_sum_exp_inv_xi_over_alpha.resize( m_pDB->m_nets.size(), 0 );
    m_nets_sum_exp_inv_yi_over_alpha.resize( m_pDB->m_nets.size(), 0 );

    m_nets_sum_p_x_pos.resize( m_pDB->m_nets.size(), 0 );
    m_nets_sum_p_y_pos.resize( m_pDB->m_nets.size(), 0 );
    m_nets_sum_p_inv_x_pos.resize( m_pDB->m_nets.size(), 0 );
    m_nets_sum_p_inv_y_pos.resize( m_pDB->m_nets.size(), 0 );
    m_nets_sum_p_x_neg.resize( m_pDB->m_nets.size(), 0 );
    m_nets_sum_p_y_neg.resize( m_pDB->m_nets.size(), 0 );
    m_nets_sum_p_inv_x_neg.resize( m_pDB->m_nets.size(), 0 );
    m_nets_sum_p_inv_y_neg.resize( m_pDB->m_nets.size(), 0 );

    grad_wire.resize( 2 * m_pDB->m_modules.size(), 0.0 );
    grad_potential.resize( 2 * m_pDB->m_modules.size(), 0.0 );

    /*
    m_totalMovableModuleArea = 0;
    m_totalFixedModuleArea = 0;
    for( unsigned int i=0; i<m_pDB->m_modules.size(); i++ )
    {
	if( m_pDB->m_modules[i].m_isFixed == false )
	    m_totalMovableModuleArea += m_pDB->m_modules[i].m_area;
	else
	{
	    if( m_pDB->m_modules[i].m_isOutCore == false )
	    {
		m_totalFixedModuleArea += 
		    getOverlap( m_pDB->m_coreRgn.left, m_pDB->m_coreRgn.right,
			    m_pDB->m_modules[i].m_x, m_pDB->m_modules[i].m_x + m_pDB->m_modules[i].m_width ) *
		    getOverlap( m_pDB->m_coreRgn.bottom, m_pDB->m_coreRgn.top,
			    m_pDB->m_modules[i].m_y, m_pDB->m_modules[i].m_y + m_pDB->m_modules[i].m_height ); 
	    }	
	}
    }
    if( param.bShow )
	printf( "Total movable area %.0f, fixed area %.0f\n", 
		m_totalMovableModuleArea, m_totalFixedModuleArea );
    */
    
}

MyNLP::~MyNLP()
{
    delete [] x;
    delete [] xBest;
    //delete [] xBak;
    //delete [] xBak2;
    delete [] _expX;
    delete [] _expPins;
    delete [] x_l;
    delete [] x_u;
}

void MyNLP::SetUsePin()
{
    //printf( "row height = %f\n", m_pDB->m_rowHeight );
    int effectivePinCount = 0;
    for( unsigned int i=0; i<m_pDB->m_modules.size(); i++ )
    {
	bool usePin = false;
	for( unsigned int p=0; p<m_pDB->m_modules[i].m_pinsId.size(); p++ )
	{
	    int pinId = m_pDB->m_modules[i].m_pinsId[p];
	    
	    if( m_pDB->m_pins[pinId].xOff != 0.0 || m_pDB->m_pins[pinId].yOff != 0.0 )
	    {
		usePin = true;
		break;
	    }
	}
	if( usePin )
	    effectivePinCount++;
	m_usePin[i] = usePin;
    }
    if( param.bShow )
	printf( "Effective Pin # = %d\n", effectivePinCount );
}



bool MyNLP::MySolve( double wWire, 
		     double target_density, 
                     int currentLevel,	// for plotting
		     bool noRelaxSmooth
                   )
{

    double time_start = seconds();    
    //printf( "Start Optimization \n" );
    assert( _potentialGridR > 0 );
   
    
    double avgCellSize = 0;
    int count = 0;
    for( unsigned int i=0; i<m_pDB->m_modules.size(); i++ )
    {
	if( m_pDB->m_modules[i].m_isFixed == false )
	{
	    avgCellSize += sqrt( m_pDB->m_modules[i].m_area );
	    count++;
	}
    }
    avgCellSize /= count;
    //int maxGridSize = static_cast<int>( (m_pDB->m_coreRgn.right - m_pDB->m_coreRgn.left) / avgCellSize * 0.95 );
    int maxGridSize = static_cast<int>( (m_pDB->m_coreRgn.right - m_pDB->m_coreRgn.left) / avgCellSize );
    
    
    if( m_potentialGridSize <= 0 )
	//m_potentialGridSize = static_cast<int>( sqrt( m_pDB->m_modules.size() ) * 0.8 );
	//m_potentialGridSize = static_cast<int>( sqrt(static_cast<double>( m_pDB->m_modules.size()) ) * 0.85 );
	m_potentialGridSize = static_cast<int>( sqrt(static_cast<double>( m_pDB->m_modules.size()) ) * 0.8 );


    //TEST
    //m_potentialGridSize = maxGridSize;
    
    //printf( "       AvgCellWidth  = %f (max grid size = %d)\n", avgCellSize, maxGridSize );
    //printf( "       targetUtil    = %f\n", m_targetUtil );
    //printf( "       potentialR    = %d\n", _potentialGridR );
    //printf( "       incFactor     = %g\n", m_incFactor );

    if( param.bShow )
	printf( "step scale = %f\n", param.step );
    
    int n, m, nnz_jac_g, nnz_h_lag;
    get_nlp_info( n, m, nnz_jac_g, nnz_h_lag );
    get_bounds_info( n, x_l, x_u, m, NULL, NULL );
    get_starting_point( n, true, x, false, NULL, NULL, m, false, NULL );
    BoundX( n, x, x_l, x_u );

    m_ite = 0;
    bool isLegal = false;
    assert( param.dLpNorm_P > 0 );
    while( true )
    {
	//_alpha = 0.5 * m_potentialGridWidth; // according to APlace ispd04
	//_alpha = ( m_pDB->m_coreRgn.right - m_pDB->m_coreRgn.left ) * 0.005;	// as small as possible
	_alpha = param.dLpNorm_P;
	//printf( "GRID = %d (alpha = %f) width = %.2f\n", m_potentialGridSize, _alpha, ( m_pDB->m_coreRgn.right - m_pDB->m_coreRgn.left )/m_potentialGridSize );
	if( param.bShow )
	    printf( "GRID = %d  (width = %.2f)\n", m_potentialGridSize, ( m_pDB->m_coreRgn.right - m_pDB->m_coreRgn.left )/m_potentialGridSize );

	if( m_topLevel )
	    m_lastNLP = true;
	else
	    m_lastNLP = false;
	
	isLegal = GoSolve( wWire, target_density, currentLevel );

	break;
	
	if( !m_topLevel )
	    break;
	
	m_potentialGridSize *= 2;	    // finner the grid when top level
	
	if( m_potentialGridSize > maxGridSize )
	    break;
    }

    if( param.bShow )
    {
	printf( "HPWL = %.0f\n", m_pDB->CalcHPWL() );
	printf( "\nLevel Time        = %.2f sec = %.2f min\n", 
		double(seconds()-time_start), double(seconds()-time_start)/60.0  );
	printf( "Time Sum-Exp      = %.1f sec\n", time_sum_exp );
	printf( "Time up potential = %.1f sec\n", time_up_potential );
	printf( "Time eval_f       = %.2f sec = (WL) %.2f + (P)%.2f\n", time_f, time_wl, time_update_grid );
	printf( "Time eval_grad_f  = %.1f sec = (gradWL) %.1f + (gradP) %.1f\n", 
		time_grad_f, time_grad_wl, time_grad_potential );
    }

    return isLegal;
}


bool MyNLP::GoSolve( double wWire, 
		     double target_density, 
                     int currentLevel	// for plotting
                   )
{
    double givenTargetUtil = m_targetUtil; // for look ahead legalization
    m_currentStep = param.step;
    
    m_targetUtil += 0.05;
    if( m_targetUtil > 1.0 )
	m_targetUtil = 1.0;
    
    
    double time_start = seconds();    
    char filename[100];	    // for gnuplot
    

    int n = 2 * m_pDB->m_modules.size();
    
    double designUtil = m_pDB->m_totalMovableModuleArea / m_pDB->m_totalFreeSpace;
    if( param.bShow )
	printf( "INFO: Design utilization: %f\n", designUtil );
    if( m_targetUtil > 0 )  // has user-defined target utilization
    {
	// This part is very important for ISPD-06 Placement Contest.
	double lowest = designUtil + 0.05;
	if( m_targetUtil < lowest )
	{
	    if( param.bShow )
	    {
		printf( "WARNING: Target utilization (%f) is too low\n", m_targetUtil );
		printf( "         Set target utilization to %f\n", lowest );
	    }
	    m_targetUtil = lowest;
	}
    }
    else // no given utilization
    {
	printf( "No given target utilization. Distribute blocks evenly.\n" );
	m_targetUtil = designUtil + 0.05;
	if( m_targetUtil > 1.0 )
	    m_targetUtil = 1.0;	    
    }
    if( param.bShow )
	printf( "DBIN: Target utilization: %f\n", m_targetUtil );
    
    double* grad_f = new double [ 2 * m_pDB->m_modules.size() ];
    double* last_grad_f = new double [ 2 * m_pDB->m_modules.size() ]; // for computing CG-direction;
    memset( grad_f, 0, sizeof(double)*2*m_pDB->m_modules.size() );
    memset( last_grad_f, 0, sizeof(double)*2*m_pDB->m_modules.size() );

    /*for( unsigned int i=0; i<2*m_pDB->m_modules.size(); i++ )
    {
	grad_f[i] = 0.0;
	last_grad_f[i] = 0.0;
    }*/

    CreatePotentialGrid();   // create potential grid according to "m_potentialGridSize"
    int densityGridSize = 10;	// 1% chip area
    //int densityGridSize = m_potentialGridSize / 3;	    // not good in big3
    CreateDensityGrid( densityGridSize );	// real density: use 1% area
    UpdateDensityGridSpace( n, x );
    UpdatePotentialGridBase( x );		// init exp potential for each bin, also update ExpBin
    
#if 1
    // gaussian smoothing for base potential
    GaussianSmooth smooth;
    int r = m_smoothR;
    smooth.Gaussian2D( r, 6*r+1 );
    smooth.Smooth( m_basePotential );
    m_basePotentialOri = m_basePotential;
    sprintf( filename, "gbase%d.dat", currentLevel );
    OutputPotentialGrid( filename );
#endif 


    // TEST
    if( m_smoothDelta == 1 )
    {
	if( param.bShow )
	{
	    sprintf( filename, "gbase%d-more.dat", currentLevel );
	    printf( "generate %s...\n", filename );
	    fflush( stdout );
	}
	
	vector< vector< double > > moreSmooth = m_basePotential;
	r = m_smoothR * 6;
	int kernel_size = 5*r;
	if( kernel_size % 2 == 0 )
	    kernel_size++;
	smooth.Gaussian2D( r, kernel_size );
	smooth.Smooth( moreSmooth );

	if( param.bShow )
	{
	    swap( moreSmooth, m_basePotential );
	    OutputPotentialGrid( filename );
	    swap( moreSmooth, m_basePotential );
	}

	// merge base and moreSmooth
	double binArea = m_potentialGridWidth * m_potentialGridHeight;
	double halfBinArea = binArea / 2;
	int changeCount = 0;
	for( unsigned int i=0; i<moreSmooth.size(); i++ )
	{
	    for( unsigned int j=0; j<moreSmooth[i].size(); j++ )
	    {
		double free = binArea - m_basePotential[i][j];
		if( free < 1e-4 )	// no space
		{
		    if( moreSmooth[i][j] > halfBinArea )
		    {
			m_basePotential[i][j] += moreSmooth[i][j] - halfBinArea;
			changeCount++;
		    }	
		}
	    }
	}

	if( param.bShow )
	{
	    printf( "change %d\n", changeCount );
	    sprintf( filename, "gbase%d-more-merge.dat", currentLevel );
	    OutputPotentialGrid( filename );
	}
    }


    if( m_smoothDelta > 1.0 )
	SmoothPotentialBase( double(m_smoothDelta) );   // also update ExpBin
    
    UpdateExpBinPotential( m_targetUtil );
   
#if 1 
    if( param.bShow )
    {
	sprintf( filename, "base%d.dat", currentLevel );
	OutputPotentialGrid( filename );
	// TEST
	/*for( int delta=1; delta<=10; delta++ )
	  {
	  SmoothPotentialBase( (double)delta );
	  sprintf( filename, "base%d-%d.dat", currentLevel, delta );
	  OutputPotentialGrid( filename );
	  }*/
    }
#endif


    assert( m_targetUtil > 0 );
    
    // wirelength 
    UpdateExpValueForEachCell( n, x, _expX, _alpha );
    UpdateExpValueForEachPin( n, x, _expPins, _alpha );
    UpdateNetsSumExp( x, _expX );
    totalWL = GetWL( n, x, _expX, _alpha );

    // density
    UpdatePotentialGrid( x );
    UpdateDensityGrid( n, x );
    density = GetDensityPanelty();

    // 2006-02-22 weight (APlace ICCAD05)
    _weightWire = 1.0;    
    eval_grad_f( n, x, _expX, true, grad_f );
    double totalWireGradient = 0;
    double totalPotentialGradient = 0;

    // TODO: truncation?
    AdjustForce( n, x, grad_wire, grad_potential );
		
    for( int i=0; i<n; i++ )
    {
	totalWireGradient      += fabs( grad_wire[i] );
	totalPotentialGradient += fabs( grad_potential[i] );
	//totalWireGradient      += grad_wire[i] * grad_wire[i];
	//totalPotentialGradient += grad_potential[i] * grad_potential[i];
    }
    //totalWireGradient = sqrt( totalWireGradient );
    //totalPotentialGradient = sqrt( totalPotentialGradient );

    /*
    for( unsigned int i=0; i<m_pDB->m_modules.size(); i++ )
    {
	totalWireGradient += sqrt( grad_wire[2*i] * grad_wire[2*i] + grad_wire[2*i+1] * grad_wire[2*i+1] );
	totalPotentialGradient += sqrt( grad_potential[2*i] * grad_potential[2*i] + grad_potential[2*i+1] * grad_potential[2*i+1] );
    }
    */
    
    //_weightDensity = 1.0 / ( 0.5 * totalPotentialGradient / totalWireGradient );  // APlace ICCAD-05
    //_weightDensity = 1.0 / ( wWire * totalPotentialGradient / totalWireGradient );
    
    _weightDensity = 1.0;
    _weightWire = wWire * totalPotentialGradient / totalWireGradient;
    
//    printf( " INIT: LogSumExp WL= %.0f, gradWL= %.0f\n", totalWL, totalWireGradient );
//    printf( " INIT: DensityPenalty= %.0f, gradPenalty= %.0f\n", density, totalPotentialGradient ); 
    
    int maxIte = 50;	// max outerIte	

//    printf( "       DenGridSize   = %d\n", densityGridSize );
//    printf( "       GridSize      = %d * %d (w %.3f h %.3f)\n", m_potentialGridSize, m_potentialGridSize, m_potentialGridWidth, m_potentialGridHeight );
//    printf( "       GridLen       = w %.3f h %.3f\n", m_potentialGridWidth, m_potentialGridHeight );
//    printf( "       alpha         = %f\n", _alpha );
//    printf( "       maxOuterIter  = %d\n", maxIte ); 
//    printf( "       weightDensity = %g\n", _weightDensity );
//    printf( "       weightWL      = %g\n", _weightWire );
//    printf( "       targetNNB     = %f\n", target_nnb );
//    printf( "       stopStepSize  = %f\n", stopStepSize );
    
   
    
    bool newDir = true;
    double obj_value;
    double beta;	// determined by CG
    eval_f( n, x, _expX, true, obj_value );
    eval_grad_f( n, x, _expX, true, grad_f );

    double nnb_real = GetNonZeroDensityGridPercent();
    UpdateDensityGrid( n, x );
    double maxDen = GetMaxDensity();
    double totalOverDen = GetTotalOverDensity();
    double totalOverDenLB = GetTotalOverDensityLB();
    double totalOverPotential = GetTotalOverPotential();
    
    //printf( "INIT f = %f\n", obj_value );
   
    if( param.bShow )
    {	
	printf( " %d-%2d HPWL= %.0f\tDen= %.2f %.2f %.2f %.2f NNB= %.2f Dcost= %4.1f%%  WireW= %.0f ", 
		currentLevel, m_ite, m_pDB->CalcHPWL(), 
		maxDen, totalOverDen, totalOverDenLB, totalOverPotential,
		nnb_real, 
		density * _weightDensity / obj_value * 100.0, _weightWire  
	      );
    }
    else
    {
	printf( " %d-%2d HPWL= %.0f \t", 
		currentLevel, m_ite, m_pDB->CalcHPWL() 
	      );
    }
    fflush( stdout );
    if( param.bShow )
    {
	sprintf( filename, "fig%d-%d.plt", currentLevel, m_ite );
	m_pDB->OutputGnuplotFigure( filename, false, false );
    }

    double lastTotalOver = 0;
    double lastTotalOverPotential = DBL_MAX;
    double over = totalOverDen;
    int totalIte = 0;
   
    bool hasBestLegalSol = false;
    double bestLegalWL = DBL_MAX;
    int lookAheadLegalCount = 0;
    double totalLegalTime = 0.0;
    //double norm_move = 0.0;
    
    bool startDecreasing = false;
    
    int checkStep = 5; 
    int outStep = 25;
    if( param.bShow == false )
	outStep = INT_MAX;

    int LALnoGoodCount = 0;
    
    for( int ite=0; ite<maxIte; ite++ )
    {
	m_ite++;
	int innerIte = 0;
	double old_obj = DBL_MAX;
	double last_obj_value = DBL_MAX;

	m_currentStep = param.step;
	
	newDir = true;
	while( true )	// inner loop, minimize "f" 
	{
	    innerIte++;
	    swap( last_grad_f, grad_f );    // save for computing the congujate gradient direction
	    eval_grad_f( n, x, _expX, true, grad_f );
	    AdjustForce( n, x, grad_f );

	    if( innerIte % checkStep == 0 )
	    {
		old_obj = last_obj_value;    // backup the old value
		eval_f( n, x, _expX, true, obj_value );
		last_obj_value = obj_value;
	    }
	    
#if 1
	    // Output solving progress
	    if( innerIte % outStep == 0 && innerIte != 0 )
	    {
		if( innerIte % checkStep != 0 )
		    eval_f( n, x, _expX, true, obj_value );
		printf( "\n\t  (%4d): f= %.10g\tstep= %.6f \t %.1fm ", 
			innerIte, 
			obj_value, 
			stepSize,
			double(seconds()-time_start)/60.0
		      );
		/*printf( "\n\t  (%4d): f= %.8g\tstep= %.6f %.2f \t %.1fm ", 
			innerIte, 
			obj_value, 
			stepSize,
			sqrt(norm_move/n),
			double(seconds()-time_start)/60.0
		      );*/
		/*UpdateBlockPosition( x );   // update to placeDB
		  UpdateDensityGrid( n, x );
		  double nnb_real = GetNonZeroDensityGridPercent();
		  maxDen = GetMaxDensity();
		  totalOverDen = GetTotalOverDensity();
		  printf( "\n %4d HPWL= %0.4g\tDen= %.3f %.3f NNB= %.3f LTime= %.1fm ", 
		  innerIte, m_pDB->CalcHPWL(), 
		  maxDen, totalOverDen,
		  nnb_real, 
		  double(seconds()-time_start)/60.0  );*/
		fflush( stdout );
	    }
#endif

	    if( innerIte % checkStep == 0 )
	    {
		printf( "." );
		fflush( stdout );

		if( innerIte % 2 * checkStep == 0 )
		{
		    UpdateBlockPosition( x );   // update to placeDB
		    if( m_pDB->CalcHPWL() > bestLegalWL )   // gWL > LAL-WL
		    {
			printf( "X\n" );
			fflush( stdout );
			break;	
		    }
		}

		UpdateDensityGrid( n, x );  // find the exact bin density
		totalOverDen = GetTotalOverDensity();
		totalOverDenLB = GetTotalOverDensityLB();
		totalOverPotential = GetTotalOverPotential();

		lastTotalOver = over;
		over = min( totalOverPotential, totalOverDen ); // TEST

		if( !startDecreasing
			&& over < lastTotalOver 
			&& ite >= 1 
			&& innerIte >= 6 )
		{
		    printf( ">>" );
		    fflush( stdout );
		    startDecreasing = true;
		}

		// 2005-03-11: meet the constraint 
		if( startDecreasing && over < target_density )
		    break;

		// Cannot further improve 
		if( obj_value >= param.precision * old_obj )   
		{
		    break;
#if 0
		    if( m_currentStep < 0.2 )
			break;
		    else
		    {
			//m_currentStep *= 0.618;
			m_currentStep *= 0.6666;
			printf( "*" );
			newDir = true;
			printf( "\n\t  (%4d): f= %.10g\tstep= %.6f \t %.1fm ", 
				innerIte, 
				obj_value, 
				stepSize,
				double(seconds()-time_start)/60.0
			      );
		    }
#endif
		}
	    }


	    // Calculate d_k (conjugate gradient method)
	    if( newDir == true )	
	    {
		// gradient direction
		newDir = false;
		for( int i=0; i<n; i++ )
		    grad_f[i] = -grad_f[i];
	    }
	    else
	    {
		// conjugate gradient direction
		FindBeta( n, grad_f, last_grad_f, beta );
		for( int i=0; i<n; i++ )
		    grad_f[i] = -grad_f[i] + beta * last_grad_f[i];
	    }


	    // Calculate a_k (step size)
	    LineSearch( n, x, grad_f, stepSize );

	    // Update X. (x_{k+1} = x_{k} + \alpha_k * d_k)
	    double move;
	    for( int i=0; i<n; i++ )
	    {
		move = grad_f[i] * stepSize;
		x[i] += move;
	    }
	   
	    /* 
	    norm_move = 0.0;
	    for( int i=0; i<n; i++ )
	    {
		double move = grad_f[i] * stepSize;
		norm_move += move * move;
	    }
	    */
	    
	    BoundX( n, x, x_l, x_u );
	    double time_used = seconds();
	    UpdateExpValueForEachCell( n, x, _expX, _alpha );
	    UpdateExpValueForEachPin( n, x, _expPins, _alpha );
	    UpdateNetsSumExp( x, _expX );
	    time_grad_wl += seconds() - time_used;
	    UpdatePotentialGrid( x );

	}// inner loop

	if( param.bShow )
	{
	    printf( "%d\n", innerIte );
	    fflush( stdout );
	}
	else
	    printf( "\n" );
	totalIte += innerIte;

	UpdateDensityGrid( n, x );
	double nnb_real = GetNonZeroDensityGridPercent();
	maxDen = GetMaxDensity();
	totalOverDen = GetTotalOverDensity();
	totalOverDenLB = GetTotalOverDensityLB();
	totalOverPotential = GetTotalOverPotential();
	
	UpdateBlockPosition( x );   // update to placeDB

#if 1
	if( param.bShow )
	{
	    // output figures
	    sprintf( filename, "fig%d-%d.plt", currentLevel, m_ite );
	    m_pDB->OutputGnuplotFigure( filename, false, false );	// it has "CalcHPWL()"
	    if( m_topLevel ) // debugging
	    {
		sprintf( filename, "fig%d-%d.pl", currentLevel, m_ite );
		m_pDB->OutputPL( filename );	
	    }
	    //if( m_potentialGridSize < 30 )  PrintPotentialGrid();

	    sprintf( filename, "grid%d-%d.dat", currentLevel, m_ite );
	    OutputPotentialGrid( filename );
	    sprintf( filename, "den%d-%d.dat", currentLevel, m_ite );
	    OutputDensityGrid( filename );

	    sprintf( filename, "util%d-%d.dat", currentLevel, m_ite );
	    CPlaceBin placeBin( *m_pDB );
	    placeBin.CreateGrid( m_pDB->m_rowHeight * 10.0 );
	    placeBin.OutputBinUtil( filename );
	}
#endif

	if( param.bShow )
	{
	    printf( " %d-%2d HPWL= %.0f\tDen= %.2f %.4f %.4f %.4f NNB= %.2f LTime= %.1fm Dcost= %4.1f%% WireW= %.0f ", 
		    currentLevel, m_ite, m_pDB->CalcHPWL(), 
		    maxDen, totalOverDen, totalOverDenLB, totalOverPotential,
		    nnb_real, 
		    double(seconds()-time_start)/60.0, 
		    density*_weightDensity /obj_value * 100.0, 
		    _weightWire
		  );
	}
	else
	{
	    printf( " %d-%2d HPWL= %.f\tLTime= %.1fm ", 
		    currentLevel, m_ite, m_pDB->CalcHPWL(), 
		    double(seconds()-time_start)/60.0 
		  );
	}
	fflush( stdout );

	
#if 1
	// 2006-03-06 (CAUTION! Do not use look-ahead legalization when dummy block exists.
	// TODO: check if there is dummy block (m_modules[].m_isDummy)
	if( m_ite >= 2 && m_lookAheadLegalization && over < target_density+0.10 )
	//if( startDecreasing && m_lookAheadLegalization ) // test
	{
	    UpdateBlockPosition( x );   // update to placeDB
	    double hpwl = m_pDB->CalcHPWL();
	    if( hpwl > bestLegalWL )
	    {
		printf( "Stop. Good enough.\n" );
		break;	
	    }

	    lookAheadLegalCount++;
	    double oldWL = hpwl;
	    CTetrisLegal legal(*m_pDB);

	    //bool bMacroShifter = legal.MacroShifter( 10, false );
	    //if( false == bMacroShifter )
	    //	printf( "MACRO SHIFTER FAILED!\n" );

	    double scale = 0.85;
	    if( givenTargetUtil < 1.0 && givenTargetUtil > 0 )
		scale = 0.9;

	    double legalStart = seconds();
	    bool bLegal = legal.Solve( givenTargetUtil, false, false, scale );
	    double legalTime = seconds() - legalStart;
	    totalLegalTime += legalTime;
	    if( param.bShow )
		printf( "LAL Time: %.2f\n", legalTime );
	    if( bLegal )
	    {
		double WL = m_pDB->GetHPWLdensity( givenTargetUtil );
		if( param.bShow )
		    m_pDB->ShowDensityInfo();
		if( WL < bestLegalWL )
		{
		    // record the best legal solution
		    LALnoGoodCount = 0;
		    if( param.bShow )
			printf( "SAVE BEST! (HPWL=%.0f)(dHPWL= %.0f)(%.2f%%)\n", 
				m_pDB->GetHPWLp2p(), WL, (WL-oldWL)/oldWL*100 );
		    bestLegalWL = WL;
		    hasBestLegalSol = true;
		    for( int i=0; i<(int)m_pDB->m_modules.size(); i++ )
		    {
			xBest[2*i] = m_pDB->m_modules[i].m_cx;
			xBest[2*i+1] = m_pDB->m_modules[i].m_cy;
		    }	    
		}
		else
		{
		    if( param.bShow )
			printf( "(HPWL=%.0f)(dHPWL= %.0f)(%.2f%%)\n", m_pDB->GetHPWLp2p(), WL, (WL-oldWL)/oldWL*100 );
		    if( (WL-oldWL)/oldWL < 0.075 )
		    {
			if( param.bShow )
			    printf( "Stop. Good enough\n" ); 
			break;
		    }
		    LALnoGoodCount++;
		    if( LALnoGoodCount >= 2 )
			break;
		}
	    }
	}
#endif	

	if( ite >= 2 )
	{
	    if( startDecreasing && over < target_density )
	    {
		printf( "Meet constraint!\n" );
		break;
	    }

	    // cannot reduce totalOverPotential
	    if( ite > 3 && totalOverPotential > lastTotalOverPotential &&
		    totalOverPotential < 1.4 )
	    {
		printf( "Cannot further reduce!\n" );
		break;
	    }
	}

	_weightWire /= m_incFactor;
	lastTotalOverPotential = totalOverPotential;
    
    }// outer loop


    // 2006-03-06 (donnie)
    if( hasBestLegalSol )
	memcpy( x, xBest, sizeof(double)*n );
    UpdateBlockPosition( x );
   
    if( lookAheadLegalCount > 0 && param.bShow )
    {
	printf( "LAL: Total Count: %d\n", lookAheadLegalCount );
	printf( "LAL: Total CPU: %.2f\n", totalLegalTime );
	sprintf( filename, "util-global.dat" );
	CPlaceBin placeBin( *m_pDB );
	placeBin.CreateGrid( m_pDB->m_rowHeight * 10.0 );
	placeBin.OutputBinUtil( filename );
    }
    
    static int allTotalIte = 0;
    allTotalIte += totalIte;

    if( param.bShow )
    {
	m_pDB->ShowDensityInfo();
	printf( "\nLevel Ite %d   Total Ite %d\n", totalIte, allTotalIte );
    }
    
    delete [] grad_f;
    delete [] last_grad_f;	// for computing conjugate gradient direction
    
    return hasBestLegalSol;
}


void MyNLP::FindBeta( const int& n, const double* grad_f, const double* last_grad_f, double& beta )
{
    // Polak-Ribiere foumula from APlace journal paper
    // NOTE:
    //   g_{k-1} = -last_grad_f
    //   g_k     = grad_f

    double l2norm = 0;
    for( int i=0; i<n; i++ )
	l2norm += last_grad_f[i] * last_grad_f[i];

    double product = 0;
    for( int i=0; i<n; i++ )
	product += grad_f[i] * ( grad_f[i] + last_grad_f[i] );	// g_k^T ( g_k - g_{k-1} )
    beta = product / l2norm;
}


void MyNLP::BoundX( const int& n, double* x, double* x_l, double* x_h, const int& i )
{
    if( x[i] < x_l[i] )      x[i] = x_l[i];
    else if( x[i] > x_h[i] )	x[i] = x_h[i];
}


void MyNLP::BoundX( const int& n, double* x, double* x_l, double* x_h )
{
    for( int i=0; i<n; i++ )
    {
	if( x[i] < x_l[i] )             x[i] = x_l[i];
        else if( x[i] > x_h[i] )	x[i] = x_h[i];
    } 
}

void MyNLP::AdjustForce( const int& n, const double* x, double* grad_f )
{

    double totalGrad = 0;
    int size = n/2;
    for( int i=0; i<size; i++ )
    {
	double value = grad_f[2*i] * grad_f[2*i] + grad_f[2*i+1] * grad_f[2*i+1];
	totalGrad += value;
    }
    double avgGrad = sqrt( totalGrad / size );
 
    // Do truncation
    double expMaxGrad = avgGrad * truncationFactor;	// x + y
    double expMaxGradSquare = expMaxGrad * expMaxGrad;
    for( int i=0; i<size; i++ )
    {
	double valueSquare = ( grad_f[2*i]*grad_f[2*i] + grad_f[2*i+1]*grad_f[2*i+1] );
	if( valueSquare > expMaxGradSquare )
	{
	    double value = sqrt( valueSquare );
	    grad_f[2*i]   = grad_f[2*i]   * expMaxGrad / value;
	    grad_f[2*i+1] = grad_f[2*i+1] * expMaxGrad / value;
	}
    }
}


void MyNLP::AdjustForce( const int& n, const double* x, vector<double> grad_wl, vector<double> grad_potential )
{
    double totalGrad = 0;
    int size = n/2;
    for( int i=0; i<size; i++ )
    {
	double value = 
	    (grad_wl[2*i] + grad_potential[2*i]) * (grad_wl[2*i] + grad_potential[2*i]) + 
	    (grad_wl[2*i+1] + grad_potential[2*i+1]) * (grad_wl[2*i+1] + grad_potential[2*i+1]); 
	totalGrad += value;
    }
    double avgGrad = sqrt( totalGrad / size );
 
    // Do truncation
    double expMaxGrad = avgGrad * truncationFactor;	// x + y
    double expMaxGradSquare = expMaxGrad * expMaxGrad;
    for( int i=0; i<size; i++ )
    {
	double valueSquare = 
	    (grad_wl[2*i] + grad_potential[2*i]) * (grad_wl[2*i] + grad_potential[2*i]) + 
	    (grad_wl[2*i+1] + grad_potential[2*i+1]) * (grad_wl[2*i+1] + grad_potential[2*i+1]); 
	if( valueSquare > expMaxGradSquare )
	{
	    double value = sqrt( valueSquare );
	    grad_wl[2*i]   = grad_wl[2*i]   * expMaxGrad / value;
	    grad_wl[2*i+1] = grad_wl[2*i+1] * expMaxGrad / value;
	    grad_potential[2*i]   = grad_potential[2*i]   * expMaxGrad / value;
	    grad_potential[2*i+1] = grad_potential[2*i+1] * expMaxGrad / value;
	}
    }
}


void MyNLP::LineSearch( const int& n, /*const*/ double* x, double* grad_f, double& stepSize )
{
    int size = n / 2;
    double totalGrad = 0;
    for( int i=0; i<n; i++ )
	totalGrad += grad_f[i] * grad_f[i];
    double avgGrad = sqrt( totalGrad / size );
    stepSize = m_potentialGridWidth / avgGrad * m_currentStep;	
    
    return;
}

bool MyNLP::get_nlp_info(int& n, int& m, int& nnz_jac_g, 
			 int& nnz_h_lag/*, IndexStyleEnum& index_style*/)
{
    //printf( "*** get_nlp_info() ***\n" );

    n = m_pDB->m_modules.size() * 2;
    
    //printf( "alpha = %f, wireWeigtht = %f, densityWeight = %f\n", 
	//    alpha, _weightWire, _weightDensity );
    
    m = 0;	    // no constraint
    nnz_jac_g = 0;  // 0 nonzeros in the jacobian since no constraint

    /*
    // calculate nnz_h
    set< pair<int,int> > cell_pair;
    for( unsigned int i=0; i<m_pDB->m_nets.size(); i++ )
    {
	if( m_pDB->m_nets[i].size() <= 1 )
	    continue;
	
	for( unsigned int first=0; first<m_pDB->m_nets[i].size()-1; first++ )
	{
	    for( unsigned int second=1; second<m_pDB->m_nets[i].size(); second++ )
	    {
		if( m_pDB->m_nets[i][first] == m_pDB->m_nets[i][second] )
		    continue;	

		assert( m_pDB->m_nets[i][first] < (int)m_pDB->m_pins.size() );
		assert( m_pDB->m_nets[i][second] < (int)m_pDB->m_pins.size() );

		int blockFirst = m_pDB->m_pins[ m_pDB->m_nets[i][first] ].moduleId;
		int blockSecond = m_pDB->m_pins[ m_pDB->m_nets[i][second] ].moduleId;
		
		if( blockFirst >= blockSecond )
		    cell_pair.insert( pair<int,int>( blockFirst, blockSecond ) );
		else
		    cell_pair.insert( pair<int,int>( blockSecond, blockFirst ) );
	    }
	}
    }	
    for( unsigned int i=0; i<m_pDB->m_modules.size(); i++ )
    {
	if( m_pDB->m_modules[i].m_isFixed == false )
	{
	    // diagonal terms
	    cell_pair.insert( pair<int,int>( i, i ) );
	}
    }
    _cellPair.reserve( cell_pair.size() );
    copy( cell_pair.begin(), cell_pair.end(), back_inserter( _cellPair ) ); 
    nnz_h_lag = _cellPair.size() * 2;	    // dx1dx2 & dy1dy2
    */
    //printf( "    nnz_h_lag = %d\n", nnz_h_lag );

    // We use the standard fortran index style for row/col entries
    //index_style = TNLP::C_STYLE;

    return true;
}

bool MyNLP::get_bounds_info(int n, double* x_l, double* x_u,
                            int m, double* g_l, double* g_u)
{
  // here, the n and m we gave IPOPT in get_nlp_info are passed back to us.
  // If desired, we could assert to make sure they are what we think they are.
  assert(n == (int)m_pDB->m_modules.size() * 2);
  assert(m == 0);

  //printf( "get_bounds_info\n" );
  
  for( unsigned int i=0; i<m_pDB->m_modules.size(); i++ )
  {
      if( m_pDB->m_modules[i].m_isFixed )
      {
	  x_l[2*i] = m_pDB->m_modules[i].m_cx;
	  x_u[2*i] = m_pDB->m_modules[i].m_cx;
	  x_l[2*i+1] = m_pDB->m_modules[i].m_cy;
	  x_u[2*i+1] = m_pDB->m_modules[i].m_cy;
      }
      else
      {
	  x_l[2*i]   = m_pDB->m_coreRgn.left   + m_pDB->m_modules[i].m_width  * 0.5;  
	  x_u[2*i]   = m_pDB->m_coreRgn.right  - m_pDB->m_modules[i].m_width  * 0.5;  
	  x_l[2*i+1] = m_pDB->m_coreRgn.bottom + m_pDB->m_modules[i].m_height * 0.5;
	  x_u[2*i+1] = m_pDB->m_coreRgn.top    - m_pDB->m_modules[i].m_height * 0.5;
      }
  }
  
  return true;
}

bool MyNLP::get_starting_point(int n, bool init_x, double* x,
                               bool init_z, double* z_L, double* z_U,
                               int m, bool init_lambda,
                               double* lambda)
{
  assert(init_x == true);
  assert(init_z == false);
  assert(init_lambda == false);

  //printf( "get_starting_point\n" );
  
  for( unsigned int i=0; i<m_pDB->m_modules.size(); i++ )
  {
	x[2*i]   = m_pDB->m_modules[i].m_cx;
	x[2*i+1] = m_pDB->m_modules[i].m_cy;
  }

  return true;
}

void MyNLP::UpdateExpValueForEachCell( const int& n, const double* x, double* expX, const double& inAlpha )
{
    for( int i=0; i<n; i++ )
    {
	expX[i] = pow( x[i] * m_posScale, inAlpha );
	//expX[i] = expTable( x[i] / inAlpha );
	/*if( expX[i] == 0 )
	{
	    printf( "ERR x[i] %f alpha %f \n", x[i], inAlpha );
	}
	assert( expX[i] != 0 );*/
    }
}

void MyNLP::UpdateExpValueForEachPin( const int& n, const double* x, double* expPins, const double& inAlpha )
{
    for( unsigned int pinId=0; pinId<m_pDB->m_pins.size(); pinId++ )
    {
	int blockId = m_pDB->m_pins[pinId].moduleId;

	// 2006-02-20
	if( m_usePin[blockId] == false )
	    continue;	// save time
	
	double xx = x[ 2*blockId ]   + m_pDB->m_pins[ pinId ].xOff;
	double yy = x[ 2*blockId+1 ] + m_pDB->m_pins[ pinId ].yOff;
	expPins[2*pinId]   = pow( xx * m_posScale, inAlpha );
	expPins[2*pinId+1] = pow( yy * m_posScale, inAlpha );
        assert( expPins[2*pinId] != 0 );
        assert( expPins[2*pinId+1] != 0 );
	//expPins[2*pinId]   = expTable( xx / inAlpha );
	//expPins[2*pinId+1] = expTable( yy / inAlpha );
    }
}

void MyNLP::UpdateNetsSumExp( const double* x, const double* expX )
{
    double sum_exp_xi_over_alpha;
    double sum_exp_inv_xi_over_alpha;
    double sum_exp_yi_over_alpha;
    double sum_exp_inv_yi_over_alpha;
    for( unsigned int n=0; n<m_pDB->m_nets.size(); n++ )
    {
	if( m_pDB->m_nets[n].size() == 0 )
	    continue;
	calc_sum_exp_using_pin(
		m_pDB->m_nets[n].begin(), m_pDB->m_nets[n].end(), x, expX,
		sum_exp_xi_over_alpha, sum_exp_inv_xi_over_alpha,
		sum_exp_yi_over_alpha, sum_exp_inv_yi_over_alpha );
	
	m_nets_sum_exp_xi_over_alpha[n]     = sum_exp_xi_over_alpha;
	m_nets_sum_exp_yi_over_alpha[n]     = sum_exp_yi_over_alpha;
	m_nets_sum_exp_inv_xi_over_alpha[n] = sum_exp_inv_xi_over_alpha;
	m_nets_sum_exp_inv_yi_over_alpha[n] = sum_exp_inv_yi_over_alpha;
    }

    for( unsigned int n=0; n<m_pDB->m_nets.size(); n++ )
    {
        if( m_pDB->m_nets[n].size() == 0 )
            continue;
        m_nets_sum_p_x_pos[n]     = pow( m_nets_sum_exp_xi_over_alpha[n], 1/_alpha-1 );
        m_nets_sum_p_y_pos[n]     = pow( m_nets_sum_exp_yi_over_alpha[n], 1/_alpha-1 );
        m_nets_sum_p_inv_x_pos[n] = pow( m_nets_sum_exp_inv_xi_over_alpha[n], 1/_alpha-1 );
        m_nets_sum_p_inv_y_pos[n] = pow( m_nets_sum_exp_inv_yi_over_alpha[n], 1/_alpha-1 );
        m_nets_sum_p_x_neg[n]     = pow( m_nets_sum_exp_xi_over_alpha[n], -1/_alpha-1 );
        m_nets_sum_p_y_neg[n]     = pow( m_nets_sum_exp_yi_over_alpha[n], -1/_alpha-1 );
        m_nets_sum_p_inv_x_neg[n] = pow( m_nets_sum_exp_inv_xi_over_alpha[n], -1/_alpha-1 );
        m_nets_sum_p_inv_y_neg[n] = pow( m_nets_sum_exp_inv_yi_over_alpha[n], -1/_alpha-1 );

    }

}

double MyNLP::GetWL( const int& n, const double* x, const double* expX, const double& alpha )
{
    totalWL = 0;
    for( unsigned int n=0; n<m_pDB->m_nets.size(); n++ )	// for each net
    {
	if( m_pDB->m_nets[n].size() == 0 )
	    continue;
	
	double invAlpha = 1.0 / alpha;
        totalWL +=
            pow( m_nets_sum_exp_xi_over_alpha[n], invAlpha ) -
            pow( m_nets_sum_exp_inv_xi_over_alpha[n], -invAlpha ) +
            pow( m_nets_sum_exp_yi_over_alpha[n], invAlpha ) -
            pow( m_nets_sum_exp_inv_yi_over_alpha[n], -invAlpha );
    }
    return totalWL / m_posScale;
}
    

bool MyNLP::eval_f(int n, const double* x, const double* expX, bool new_x, double& obj_value)
{
    double time_start = seconds();
    
    totalWL = GetWL( n, x, expX, _alpha );
    time_wl += seconds() - time_start;
    
    double time_start_2 = seconds();
    density = GetDensityPanelty();
    time_update_grid += seconds() - time_start_2;

    obj_value = (totalWL * _weightWire) + (density * _weightDensity);
    
    time_f += seconds() - time_start;    
    return true;
}

bool MyNLP::eval_f_HPWL(int n, const double* x, const double* expX, bool new_x, double& obj_value)
{
    double time_start = seconds();
    
    UpdateBlockPosition( x );
    totalWL = m_pDB->CalcHPWL();
    time_wl += seconds() - time_start;
    
    double time_start_2 = seconds();
    density = GetDensityPanelty();
    time_update_grid += seconds() - time_start_2;

    obj_value = (totalWL * _weightWire) + (density * _weightDensity);
    
    time_f += seconds() - time_start;    
    return true;
}

void MyNLP::PrintPotentialGrid()
{
    for( int i=(int)m_gridPotential.size()-1; i>=0; i-- )
    {
	for( unsigned int j=0; j<m_gridPotential[i].size(); j++ )
	{
	    printf( "%4.1f ", (m_gridPotential[i][j]-m_expBinPotential[i][j])/m_expBinPotential[i][j] );
	}
	printf( "\n" );
    }
    printf( "\n\n" );
}


double MyNLP::GetDensityPanelty()
{
    double density = 0;
    for( unsigned int i=0; i<m_gridPotential.size(); i++ )
    {
	for( unsigned int j=0; j<m_gridPotential[i].size(); j++ )
	{
	    density += ( m_gridPotential[i][j] - m_expBinPotential[i][j] ) *
		( m_gridPotential[i][j] - m_expBinPotential[i][j] );
	}
    }
    return density;
}

void MyNLP::InitModuleNetPinId()
{
    //printf( "Init module-net-pin id\n" );
    m_moduleNetPinId.resize( m_pDB->m_modules.size() );
    for( unsigned int i=0; i<m_pDB->m_modules.size(); i++ )
    {
	m_moduleNetPinId[i].resize( m_pDB->m_modules[i].m_netsId.size() );
	for( unsigned int j=0; j<m_pDB->m_modules[i].m_netsId.size(); j++ )
	{
	    int netId = m_pDB->m_modules[i].m_netsId[j];
	    int pinId = -1;
	    for( unsigned int p=0; p<m_pDB->m_nets[netId].size(); p++ )
	    {
		if( m_pDB->m_pins[ m_pDB->m_nets[netId][p] ].moduleId == (int)i )
		{
		    pinId = m_pDB->m_nets[netId][p];
		    break;
		}
	    }
	    assert( pinId != -1 );  // floating pin? (impossible for bookshelf format)
	    m_moduleNetPinId[i][j] = pinId;
	} // each net to the module
    } // each module
}

bool MyNLP::eval_grad_f(int n, const double* x, const double* expX, bool new_x, double* grad_f)
{
    double time_used = seconds();
   
    // grad WL
if( _weightWire > 0 )	//TEST
    for( unsigned int i=0; i<m_pDB->m_modules.size(); i++ )	// for each block
    {
	if( m_pDB->m_modules[i].m_isFixed || m_pDB->m_modules[i].m_netsId.size() == 0 )
	    continue;


	grad_wire[ 2*i ] = 0;
	grad_wire[ 2*i+1 ] = 0;

	for( unsigned int j=0; j<m_pDB->m_modules[i].m_netsId.size(); j++ )
	{
	    // for each net connecting to the block
	    int netId = m_pDB->m_modules[i].m_netsId[j];
	    if( m_pDB->m_nets[netId].size() == 0 ) // floating-module
		continue;

	    // TODO: modification for LEF/DEF input
	    // no floating pin for bookshelf format
	    //if( m_pDB->m_nets[netId].size() == 0 )
	    //	continue;

	    int selfPinId = m_moduleNetPinId[i][j];
	    
	    if( m_usePin[i] )
	    {
		assert( selfPinId != -1 );
                double xx = x[ 2*i ]   + m_pDB->m_pins[ selfPinId ].xOff;
                double yy = x[ 2*i+1 ] + m_pDB->m_pins[ selfPinId ].yOff;
                xx *= m_posScale;
                yy *= m_posScale;

                grad_wire[ 2*i ] +=
                    m_nets_sum_p_x_pos[netId]     * _expPins[2*selfPinId] / xx -
                    m_nets_sum_p_inv_x_neg[netId] / _expPins[2*selfPinId] / xx;
                grad_wire[ 2*i+1 ] +=
                    m_nets_sum_p_y_pos[netId]     * _expPins[2*selfPinId+1] / yy -
                    m_nets_sum_p_inv_y_neg[netId] / _expPins[2*selfPinId+1] / yy;
	    }
	    else
	    {
                double xx = x[ 2*i ];
                double yy = x[ 2*i+1 ];
                xx *= m_posScale;
                yy *= m_posScale;

                grad_wire[ 2*i ] +=
                    m_nets_sum_p_x_pos[netId]     * _expX[2*i] / xx  -
                    m_nets_sum_p_inv_x_neg[netId] / _expX[2*i] / xx;
                grad_wire[ 2*i+1 ] +=
                    m_nets_sum_p_y_pos[netId]     * _expX[2*i+1] / yy -
                    m_nets_sum_p_inv_y_neg[netId] / _expX[2*i+1] / yy;
	    }

	} // for each pin in the module
    } // for each module
    time_grad_wl += seconds() - time_used;
   


    // grad Density
    double time_start_2 = seconds();
#if 0
    UpdateBinForce();	// diffusion
#endif
    
    double gradDensityX;
    double gradDensityY;
    for( int i=0; i<(int)m_pDB->m_modules.size(); i++ )	    // for each cell
    {
	if( m_pDB->m_modules[i].m_isFixed )
	    continue;
#if 0
	GetDiffusionGrad( x, i, gradDensityX, gradDensityY );	    // diffusion	
	grad_f[2*i]   = -(2 * gradDensityX);
	grad_f[2*i+1] = -(2 * gradDensityY);
#endif
#if 1	
	GetPotentialGrad( x, i, gradDensityX, gradDensityY );	    // bell-shaped potential
	grad_potential[2*i]   = /*2 **/ gradDensityX;
	grad_potential[2*i+1] = /*2 **/ gradDensityY;
#endif	
    } // for each cell
    time_grad_potential += seconds() - time_start_2;
    
    // compute total fouce
    for( int i =0; i<n; i++ )
	grad_f[i] = _weightDensity * grad_potential[i] + grad_wire[i] * _weightWire;
    
    time_grad_f += seconds()-time_used;
    return true;
}

/*
void MyNLP::GetDensityGrad( const double* x, const int& b, double& gradX, double& gradY )
{
	double w  = m_pDB->m_modules[b].m_width;
	double h  = m_pDB->m_modules[b].m_height;

	// bottom-left 
	double left   = x[b*2]   - w * 0.5;
	double bottom = x[b*2+1] - h * 0.5;
	double right  = left   + w;
	double top    = bottom + h;

	// find nearest gird
	int gLeft   = static_cast<int>( floor( (left - m_pDB->m_coreRgn.left) / m_gridDensityWidth ) );
	int gBottom = static_cast<int>( floor( (bottom - m_pDB->m_coreRgn.bottom) / m_gridDensityHeight ) );
	int gRight  = static_cast<int>( floor( (right - m_pDB->m_coreRgn.left) / m_gridDensityWidth ) );
	int gTop    = static_cast<int>( floor( (top - m_pDB->m_coreRgn.bottom) / m_gridDensityHeight ) );

	gradX = 0;
	gradY = 0;	
	for( int xOff = gLeft; xOff < gRight; xOff++ )
	{
	    for( int yOff = gBottom; yOff < gTop; yOff++ )
	    {
		gradX += ( m_gridDensity[xOff+1][yOff] - m_gridDensity[xOff][yOff] ) / m_gridDensityWidth;
		gradY += ( m_gridDensity[xOff][yOff+1] - m_gridDensity[xOff][yOff] ) / m_gridDensityHeight;
	    }
	}
	gradX /= m_gridDensityTarget;
	gradY /= m_gridDensityTarget;
}*/


void MyNLP::GetDiffusionGrad( const double* x, const int& i, double& gradX, double& gradY )
{
	double cellX = x[i*2];
	double cellY = x[i*2+1];
	
	int gx, gy;	// left bottom grid
	GetClosestGrid( cellX, cellY, gx, gy );
	
	double xx, yy;	// left bottom grid center coordiante (x, y)
	xx = GetXGrid( gx );
	yy = GetYGrid( gy );
	if( xx > cellX )
	{
	    assert( gx > 0 );
	    gx--;
	    xx -= m_potentialGridWidth;
	}
	if( yy > cellY )
	{
	    assert( gy > 0 );	// TODO boundary
	    gy--;
	    yy -= m_potentialGridHeight;
	}

	// interpolation
	double alpha = ( cellX - xx ) / m_potentialGridWidth;	// x-direction
	double beta  = ( cellY - yy ) / m_potentialGridHeight;	// y-direction
	gradX = m_binForceX[gx][gy] + 
	        alpha * (m_binForceX[gx+1][gy] - m_binForceX[gx][gy]) +
		beta  * (m_binForceX[gx][gy+1] - m_binForceX[gx][gy]) +
		alpha * beta * (m_binForceX[gx][gy] + m_binForceX[gx+1][gy+1] -
			        m_binForceX[gx+1][gy] - m_binForceX[gx][gy+1] );
	gradY = m_binForceY[gx][gy] + 
	        alpha * (m_binForceY[gx+1][gy] - m_binForceY[gx][gy]) +
		beta  * (m_binForceY[gx][gy+1] - m_binForceY[gx][gy]) +
		alpha * beta * (m_binForceY[gx][gy] + m_binForceY[gx+1][gy+1] -
			        m_binForceY[gx+1][gy] - m_binForceY[gx][gy+1] );
}


void MyNLP::GetPotentialGrad( const double* x, const int& i, double& gradX, double& gradY )
{
    double cellX = x[i*2];
    double cellY = x[i*2+1];

    double width  = m_pDB->m_modules[i].m_width;
    double height = m_pDB->m_modules[i].m_height;
    double left   = cellX - width  * 0.5 - _potentialRX;
    double bottom = cellY - height * 0.5 - _potentialRY;
    double right  = cellX + ( cellX - left );
    double top    = cellY + ( cellY - bottom );
    if( left   < m_pDB->m_coreRgn.left )	left   = m_pDB->m_coreRgn.left;
    if( bottom < m_pDB->m_coreRgn.bottom )	bottom = m_pDB->m_coreRgn.bottom;
    if( right  > m_pDB->m_coreRgn.right )	right  = m_pDB->m_coreRgn.right;
    if( top    > m_pDB->m_coreRgn.top )	top    = m_pDB->m_coreRgn.top;
    int gx, gy;
    GetClosestGrid( left, bottom, gx, gy );

    if( gx < 0 )	gx = 0;
    if( gy < 0 )	gy = 0;

    int gxx, gyy;
    double xx, yy;
    gradX = 0.0;	
    gradY = 0.0;

    //// TEST (std-cell)
    if( height < m_potentialGridHeight && width < m_potentialGridWidth )
	width = height = 0;

    for( gxx = gx, xx = GetXGrid( gx ); 
	    xx <= right && gx < (int)m_gridPotential.size(); 
	    gxx++, xx += m_potentialGridWidth )
    {

	for( gyy = gy, yy = GetYGrid( gy ); 
		yy <= top && gy < (int)m_gridPotential.size() ; 
		gyy++, yy += m_potentialGridHeight )
	{

	    double gX = 0;
	    double gY = 0;
	    // TEST
	    //if( m_gridPotential[ gxx ][ gyy ] > m_expBinPotential[gxx][gyy] )  // TEST for ispd05
	    {
		gX = ( m_gridPotential[gxx][gyy] - m_expBinPotential[gxx][gyy] ) *
		    _cellPotentialNorm[i] *
		    GetGradPotential( cellX, xx, _potentialRX, width ) *
		    GetPotential(     cellY, yy, _potentialRY, height );
		gY =  ( m_gridPotential[gxx][gyy] - m_expBinPotential[gxx][gyy] ) *
		    _cellPotentialNorm[i] *
		    GetPotential(     cellX, xx, _potentialRX, width  ) *
		    GetGradPotential( cellY, yy, _potentialRY, height );
	    }

	    gradX += gX;
	    gradY += gY;
	}
    } // for each grid
}


void MyNLP::calc_sum_exp_using_pin( 
	const vector<int>::const_iterator& begin, const vector<int>::const_iterator& end,
	const double* x, const double* expX,
	double& sum_exp_xi_over_alpha, double& sum_exp_inv_xi_over_alpha,
	double& sum_exp_yi_over_alpha, double& sum_exp_inv_yi_over_alpha, int id )
{
    double t_start = seconds();
    
    sum_exp_xi_over_alpha = 0;
    sum_exp_inv_xi_over_alpha = 0;
    sum_exp_yi_over_alpha = 0;
    sum_exp_inv_yi_over_alpha = 0;

    vector<int>::const_iterator ite;
    int pinId;
    int blockId;
    for( ite=begin; ite!=end; ++ite )
    {
	// for each pin of the net
	pinId   = *ite;
	blockId = m_pDB->m_pins[ pinId ].moduleId;
	
	/*sum_exp_xi_over_alpha     += expX[2*blockId];
	sum_exp_inv_xi_over_alpha += 1.0 / expX[2*blockId];
	sum_exp_yi_over_alpha     += expX[2*blockId+1];
	sum_exp_inv_yi_over_alpha += 1.0 / expX[2*blockId+1];
	*/
#if 1
	if( m_usePin[blockId] /*&& blockId != id*/ )	// macro or self pin
	//if( blockId != id )	
	{
	    // handle pins
	    sum_exp_xi_over_alpha     += _expPins[ 2*pinId ];
	    sum_exp_inv_xi_over_alpha += 1.0 / _expPins[ 2*pinId ];
	    sum_exp_yi_over_alpha     += _expPins[ 2*pinId+1 ];
	    sum_exp_inv_yi_over_alpha += 1.0 / _expPins[ 2*pinId+1 ];
	}
	else
	{
	    // use block center
	    //assert( expX[2*blockId] != 0);
	    //assert( expX[2*blockId+1] != 0 );
	    sum_exp_xi_over_alpha     += expX[2*blockId];
	    sum_exp_inv_xi_over_alpha += 1.0 / expX[2*blockId];
	    sum_exp_yi_over_alpha     += expX[2*blockId+1];
	    sum_exp_inv_yi_over_alpha += 1.0 / expX[2*blockId+1];
	}
#endif
    }
    time_sum_exp += seconds() - t_start;
} 
/*
void MyNLP::finalize_solution(SolverReturn status,
                              Index n, const double* x, const double* z_L, const double* z_U,
                              Index m, const double* g, const double* lambda,
                              double obj_value)
{
  // here is where we would store the solution to variables, or write to a file, etc
  // so we could use the solution. Since the solution is displayed to the console,
  // we currently do nothing here.
  

    UpdateBlockPosition( x );
    printf( "HPWL= %f\n", m_pDB->CalcHPWL() );
    m_pDB->OutputGnuplotFigure( "fig_final.plt", false );
  
}*/

void MyNLP::UpdateBlockPosition( const double* x )
{
   for( int i=0; i<(int)m_pDB->m_modules.size(); i++ )
   {
       if( m_pDB->m_modules[i].m_isFixed == false )
       {
	    m_pDB->MoveModuleCenter( i, x[i*2], x[i*2+1] ); 
       }
   }
}

void MyNLP::CreatePotentialGrid()
{
    //printf( "Create Potential Grid\n" );
    m_gridPotential.clear(); // remove old values
    
    int realGridSize = m_potentialGridSize;
   
    m_gridPotential.resize( realGridSize );
    m_basePotential.resize( realGridSize );
    for( unsigned int i=0; i<m_gridPotential.size(); i++ )
    {
	m_basePotential[i].resize( realGridSize, 0 );
	m_gridPotential[i].resize( realGridSize, 0 );
    }
    
    m_potentialGridWidth  = ( m_pDB->m_coreRgn.right - m_pDB->m_coreRgn.left ) / m_potentialGridSize;
    m_potentialGridHeight = ( m_pDB->m_coreRgn.top   - m_pDB->m_coreRgn.bottom ) / m_potentialGridSize;
    _potentialRX = m_potentialGridWidth  * _potentialGridR;
    _potentialRY = m_potentialGridHeight * _potentialGridR;

}


void MyNLP::ClearPotentialGrid()
{
    for( int gx=0; gx<(int)m_gridPotential.size(); gx++ )
	fill( m_gridPotential[gx].begin(), m_gridPotential[gx].end(), 0.0 );
}

#if 0
void MyNLP::UpdateBinForce()	// 2006-02-21
{
    for( unsigned int i=0; i<m_gridPotential.size(); i++ )
	for( unsigned int j=0; j<m_gridPotential[i].size(); j++ )
	{
	    if( j == 0 || j == m_gridPotential.size()-1 )    // left and right
		m_binForceY[i][j] = 0;
	    else
		m_binForceY[i][j] = - ( m_gridPotential[i][j+1] - m_gridPotential[i][j-1] ) / 
		                        m_gridPotential[i][j] / 2;
	    
	    if( i == 0 || i == m_gridPotential.size()-1 )    // bottom and top
		m_binForceX[i][j] = 0;
	    else
		m_binForceX[i][j] = - ( m_gridPotential[i+1][j] - m_gridPotential[i-1][j] ) / 
		                        m_gridPotential[i][j] / 2;
	}
}
#endif


/*
void MyNLP::UpdateGridPotentialByGrad()
	//double xx = x[ 2*blockId ]   + m_pDB->m_pins[ pinId ].xOff;
	//double yy = x[ 2*blockId+1 ] + m_pDB->m_pins[ pinId ].yOff;
{
    for( unsigned int i=0; i<m_gridPotential.size(); i++ )
	for( unsigned int j=0; j<m_gridPotential[i].size(); j++ )
	    m_gridPotential[i][j] -= _gridGradPotential[i][j] * stepSize * _weightDensity;
}
*/

void MyNLP::UpdateExpBinPotential( double util )
{
    double binArea = m_potentialGridWidth * m_potentialGridHeight;

    if( util < 0 )
	util = 1.0; // use all space

    double totalFree = 0;
    int zeroSpaceBin = 0;
    m_expBinPotential.resize( m_basePotential.size() );
    for( unsigned int i=0; i<m_basePotential.size(); i++ )
    {
	m_expBinPotential[i].resize( m_basePotential[i].size() );
	for( unsigned int j=0; j<m_basePotential[i].size(); j++ )
	{
	    double base = m_basePotential[i][j];
	    double free = binArea - base;
	    if( free > 1e-4 )
	    {
		m_expBinPotential[i][j] = free * util;
		totalFree += m_expBinPotential[i][j];
	    }
	    else
	    {
		m_expBinPotential[i][j] = 0.0;
		zeroSpaceBin++;
	    }
	} 
    }

    if( param.bShow )
    {
	printf( "PBIN: Expect bin potential utilization: %f\n", util );
	printf( "PBIN: Zero space bin # = %d\n", zeroSpaceBin );
	printf( "PBIN: Total free potential = %.0f (%.5f)\n", totalFree, m_pDB->m_totalMovableModuleArea / totalFree );
    }

    // TODO: scaling?
    //assert( m_pDB->m_totalMovableModuleArea / totalFree <= 1.000001 );
    double alwaysOver = 0.0;
    if( m_targetUtil > 0.0 && m_targetUtil < 1.0 )
    {
	for( unsigned int i=0; i<m_pDB->m_modules.size(); i++ )
	{
	    if( m_pDB->m_modules[i].m_isFixed )
		continue;
	    if( m_pDB->m_modules[i].m_width >= 2 * m_potentialGridWidth && 
		    m_pDB->m_modules[i].m_height >= 2 * m_potentialGridHeight )
	    {
		alwaysOver += 
		    (m_pDB->m_modules[i].m_width - m_potentialGridWidth ) * 
		    (m_pDB->m_modules[i].m_height - m_potentialGridHeight ) * 
		    (1.0 - m_targetUtil );
	    }
	}
	if( param.bShow )
	    printf( "PBIN: Always over: %.0f (%.1f%%)\n", alwaysOver, alwaysOver/m_pDB->m_totalMovableModuleArea*100.0 );
    }
    m_alwaysOverPotential = alwaysOver;
}

void MyNLP::SmoothPotentialBase( const double& delta )
{
    
    // find the max potential (TODO: comnpute one time is enough)
    double maxPotential = 0;
    double avgPotential = 0;
    double totalPotential = 0;
    for( unsigned int i=0; i<m_basePotentialOri.size(); i++ )
	for( unsigned int j=0; j<m_basePotentialOri[i].size(); j++ )
	{
	    totalPotential += m_basePotentialOri[i][j];
	    if( m_basePotentialOri[i][j] > maxPotential )
		maxPotential = m_basePotentialOri[i][j];
	}
    avgPotential = totalPotential / (m_basePotentialOri.size() * m_basePotentialOri.size() );

    if( totalPotential == 0 )
	return; // no preplaced
    
    // apply TSP-style smoothing
    double newTotalPotential = 0;
    for( unsigned int i=0; i<m_basePotential.size(); i++ )
	for( unsigned int j=0; j<m_basePotential[i].size(); j++ )
	{
	    if( m_basePotentialOri[i][j] >= avgPotential )
	    {
		m_basePotential[i][j] = 
		    avgPotential + 
		    pow( ( m_basePotentialOri[i][j] - avgPotential ) / maxPotential, delta ) * maxPotential;
	    }
	    else
	    {
		m_basePotential[i][j] = 
		    avgPotential - 
		    pow( ( avgPotential - m_basePotentialOri[i][j] ) / maxPotential, delta ) * maxPotential;
	    }
	    newTotalPotential += m_basePotential[i][j];
	}
    
    // normalization
    double ratio = totalPotential / newTotalPotential;
    for( unsigned int i=0; i<m_basePotential.size(); i++ )
	for( unsigned int j=0; j<m_basePotential[i].size(); j++ )
	    m_basePotential[i][j] = m_basePotential[i][j] * ratio;

    //printf( "Smooth %.0f (%.0f->%.0f)\n", delta, totalPotential, newTotalPotential );
}

void MyNLP::UpdatePotentialGridBase( const double* x )
{
    double time_start = seconds();

    double binArea = m_potentialGridWidth * m_potentialGridHeight;
    m_binFreeSpace.resize( m_basePotential.size() );
    for( unsigned int i=0; i<m_basePotential.size(); i++ )
    {
	fill( m_basePotential[i].begin(), m_basePotential[i].end(), 0.0 );
	m_binFreeSpace[i].resize( m_basePotential[i].size() );
	fill( m_binFreeSpace[i].begin(), m_binFreeSpace[i].end(), binArea );
    }

    for( int i=0; i<(int)m_pDB->m_modules.size(); i++ )
    {
	// for each cell. cell ci coordinate is ( x[i*2], x[i*2+1] )

	if( m_pDB->m_modules[i].m_isFixed == false )
	    continue;

	// TODO: BUG when shrinking core?
	if( m_pDB->m_modules[i].m_isOutCore )
	    continue;	// pads?
	
	int gx, gy;
	double cellX = x[i*2];
	double cellY = x[i*2+1];
	double width  = m_pDB->m_modules[i].m_width;
	double height = m_pDB->m_modules[i].m_height;

	double potentialRX = _potentialRX;
	double potentialRY = _potentialRY;
	//double left   = cellX - width * 0.5  - potentialRX;
	//double bottom = cellY - height * 0.5 - potentialRY;
	double left   = cellX - width * 0.5;  // for gaussian smoothing
	double bottom = cellY - height * 0.5; // for gaussian smoothing
	double right  = cellX + (cellX - left);
	double top    = cellY + (cellY - bottom);
	if( left   < m_pDB->m_coreRgn.left )     left   = m_pDB->m_coreRgn.left;
	if( bottom < m_pDB->m_coreRgn.bottom )   bottom = m_pDB->m_coreRgn.bottom;
	if( top    > m_pDB->m_coreRgn.top )      top    = m_pDB->m_coreRgn.top;
	if( right  > m_pDB->m_coreRgn.right )    right  = m_pDB->m_coreRgn.right;
	GetClosestGrid( left, bottom, gx, gy );
	if( gx < 0 )  gx = 0;
	if( gy < 0 )  gy = 0;
      
	double totalPotential = 0;
	vector< potentialStruct > potentialList;      
	int gxx, gyy;
	double xx, yy;

	//if( m_useBellPotentialForPreplaced == false )
	{
	    // "Exact density for the potential"
	    for( gxx = gx, xx = GetXGrid(gx); xx<=right ; gxx++, xx+=m_potentialGridWidth )
	    {
		for( gyy = gy, yy = GetYGrid(gy); yy<=top ; gyy++, yy+=m_potentialGridHeight )
		{
		    m_basePotential[gxx][gyy] +=
		    	getOverlap( left, right, xx, xx+m_potentialGridWidth ) * 
		    	getOverlap( bottom, top, yy, yy+m_potentialGridHeight );

		    m_binFreeSpace[gxx][gyy] -= 
		    	getOverlap( left, right, xx, xx+m_potentialGridWidth ) * 
		    	getOverlap( bottom, top, yy, yy+m_potentialGridHeight );
		}
	    }
	    continue;
	}
	
	for( gxx = gx, xx = GetXGrid(gx); xx<=right ; gxx++, xx+=m_potentialGridWidth )
	{
	    for( gyy = gy, yy = GetYGrid(gy); yy<=top ; gyy++, yy+=m_potentialGridHeight )
	    {
		double potential = GetPotential( cellX, xx, potentialRX, width ) *
		                   GetPotential( cellY, yy, potentialRY, height );
		if( potential > 0 )
		{
		    totalPotential += potential;
		    potentialList.push_back( potentialStruct( gxx, gyy, potential ) );
		}
	    }
	}

	// normalize the potential so that total potential equals the cell area
	double scale = m_pDB->m_modules[i].m_area / totalPotential;
	//printf( "totalPotential = %f\n", totalPotential );

	_cellPotentialNorm[i] = scale;	    // normalization factor for the cell i

	vector< potentialStruct >::const_iterator ite;
	for( ite=potentialList.begin(); ite!=potentialList.end(); ++ite )
	{
	    if(	ite->gx < 0 || ite->gx >= (int)m_gridPotential.size() ||
		ite->gy < 0 || ite->gy >= (int)m_gridPotential[ite->gx].size() )
		continue; // bin may be outside when core-shrinking is applied
	    else
		m_basePotential[ ite->gx ][ ite->gy ] += ite->potential * scale;	    
	}

	
    } // for each cell
    time_up_potential += seconds() - time_start;

    m_basePotentialOri = m_basePotential;   // make a copy for TSP-style smoothing

    /*
    double totalFreeSpace = 0;
    for( unsigned int i=0; i<m_binFreeSpace.size(); i++ )
    {
	for( unsigned int j=0; j<m_binFreeSpace[i].size(); j++ )
	{
	    if( m_binFreeSpace[i][j] < 0 )
		m_binFreeSpace[i][j] = 0;
	    totalFreeSpace += m_binFreeSpace[i][j];
	}
    }
    printf( "totalFreeSpace: %.0f\n", totalFreeSpace );
    */
}


void MyNLP::UpdatePotentialGrid( const double* x )
{
    double time_start = seconds();
    ClearPotentialGrid();
    for( int i=0; i<(int)m_pDB->m_modules.size(); i++ )
    {
	// for each cell. cell ci coordinate is ( x[i*2], x[i*2+1] )

	if( m_pDB->m_modules[i].m_isOutCore )
	    continue;

	// preplaced blocks are stored in m_basePotential
	if( m_pDB->m_modules[i].m_isFixed )
	    continue;
	
	int gx, gy;
	double cellX = x[i*2];
	double cellY = x[i*2+1];
	double potentialRX = _potentialRX;
	double potentialRY = _potentialRY;
	double width  = m_pDB->m_modules[i].m_width;
	double height = m_pDB->m_modules[i].m_height;
	double left   = cellX - width * 0.5  - potentialRX;
	double bottom = cellY - height * 0.5 - potentialRY;
	double right  = cellX + (cellX - left);
	double top    = cellY + (cellY - bottom);
	if( left   < m_pDB->m_coreRgn.left )     left   = m_pDB->m_coreRgn.left;
	if( bottom < m_pDB->m_coreRgn.bottom )   bottom = m_pDB->m_coreRgn.bottom;
	if( top    > m_pDB->m_coreRgn.top )      top    = m_pDB->m_coreRgn.top;
	if( right  > m_pDB->m_coreRgn.right )    right  = m_pDB->m_coreRgn.right;
	GetClosestGrid( left, bottom, gx, gy );
       	
	double totalPotential = 0;
	vector< potentialStruct > potentialList;      
	int gxx, gyy;
	double xx, yy;

	//// TEST (convert to std-cell)
	if( height < m_potentialGridHeight && width < m_potentialGridWidth )
	    width = height = 0;

	for( gxx = gx, xx = GetXGrid(gx); xx<=right ; gxx++, xx+=m_potentialGridWidth )
	{
	    for( gyy = gy, yy = GetYGrid(gy); yy<=top ; gyy++, yy+=m_potentialGridHeight )
	    {
		double potential = GetPotential( cellX, xx, potentialRX, width ) *
		                   GetPotential( cellY, yy, potentialRY, height );
		if( potential > 0 )
		{
		    totalPotential += potential;
		    potentialList.push_back( potentialStruct( gxx, gyy, potential ) );
		}
	    }
	}

	// normalize the potential so that total potential equals the cell area
	double scale = m_pDB->m_modules[i].m_area / totalPotential;
	//printf( "totalPotential = %f\n", totalPotential );

	_cellPotentialNorm[i] = scale;	    // normalization factor for the cell i
	vector< potentialStruct >::const_iterator ite;
	for( ite=potentialList.begin(); ite!=potentialList.end(); ++ite )
	{
#if 0	    
	    assert( ite->gx <=  (int)m_gridPotential.size() );
	    assert( ite->gy <=  (int)m_gridPotential.size() );
	    assert( ite->gx >= 0 );
	    assert( ite->gy >= 0 );
#endif
	    m_gridPotential[ ite->gx ][ ite->gy ] += ite->potential * scale;	    
	}
	
    } // for each cell
    time_up_potential += seconds() - time_start;

}

/*double MyNLP::GetGridWidth()
{
    return m_potentialGridWidth;
}*/

/*double MyNLP::GetPotential( const double& x1, const double& x2, const double& r )
{
    double d = fabs( x1 - x2 );

    if( d <= r * 0.5 )
	return 1.0 - 2 * d * d / ( r * r );
    else if( d <= r )
	return 2 * ( d - r ) * ( d - r ) / ( r * r );
    else
	return 0;
}*/

double MyNLP::GetPotential( const double& x1, const double& x2, const double& r, const double& w )
{
    double d = fabs( x1 - x2 );
    double a = 4.0 / ( w + r ) / ( w + 2 * r );
    double b = 4.0 / r / ( w + 2.0 * r );
    
    if( d <= w * 0.5 + r * 0.5 )
	return 1.0 - a * d * d;
    else if( d <= w * 0.5 + r )
	return b * ( d - r - w * 0.5 ) * ( d - r - w * 0.5);
    else
	return 0.0;
}
/*
double MyNLP::GetGradPotential( const double& x1, const double& x2, const double& r )
{
    double d;
    if( x1 >= x2 )  // right half
    {
	d = x1 - x2;	// d >= 0
	if( d <= r * 0.5 )
	    return -4.0 * d / ( r * r );
	else if( d <= r )
	    return +4.0 * ( d - r ) / ( r * r );
	else
	    return 0;
    }
    else    // left half
    {
	d = x2 - x1;	// d >= 0	
	if( d <= r * 0.5 )
	    return +4.0 * d / ( r * r );
	else if( d <= r )
	    return -4.0 * ( d - r ) / ( r * r );
	else
	    return 0;
    }
}*/

double MyNLP::GetGradPotential( const double& x1, const double& x2, const double& r, const double& w )
{
    //double w = 0;
    double d;
    double a = 4.0 / ( w + r ) / ( w + 2.0 * r );
    double b = 4.0 / r / ( w + 2.0 * r );

    if( x1 >= x2 )  // right half
    {
	d = x1 - x2;	// d >= 0
	if( d <= w * 0.5 + r * 0.5 )
	    return -2.0 * a * d;
	else if( d <= w * 0.5 + r )
	    return +2.0 * b * ( d - r - w * 0.5);
	else
	    return 0;
    }
    else    // left half
    {
	d = x2 - x1;	// d >= 0	
	if( d <= w * 0.5 + r * 0.5 )
	    return +2.0 * a * d;
	else if( d <= w * 0.5 + r )
	    return -2.0 * b * ( d - r - w * 0.5);
	else
	    return 0;
    }
}

/*double MyNLP::GetGradGradPotential( const double& x1, const double& x2, const double& r )
{
    double d = fabs( x1 - x2 );

    if( d <= r * 0.5 )
	return -4.0 / ( r * r );
    else if( d <= r )
	return +4.0 / ( r * r );
    else
	return 0;
}*/
	    
/*void   MyNLP::GetGridCenter( const int& gx, const int& gy, double& x1, double& y1 )
{
    assert( gx <= m_potentialGridSize );
    assert( gy <= m_potentialGridSize );
    assert( gx >= 0 );
    assert( gy >= 0 );
    
    x1 = m_pDB->m_coreRgn.left   + gx * m_potentialGridWidth  + 0.5 * m_potentialGridWidth;
    y1 = m_pDB->m_coreRgn.bottom + gy * m_potentialGridHeight + 0.5 * m_potentialGridHeight; 
}*/

double MyNLP::GetXGrid( const int& gx )
{
    return m_pDB->m_coreRgn.left + gx * m_potentialGridWidth + 0.5 * m_potentialGridWidth;
}

double MyNLP::GetYGrid( const int& gy )
{
    return  m_pDB->m_coreRgn.bottom + gy * m_potentialGridHeight + 0.5 * m_potentialGridHeight;
}

/*double MyNLP::GetPotentialToGrid( const double& x1, const int& gx, const bool& useX )
{
    if( gx < 0 || gx >= m_potentialGridSize )
	return 0.0;
    double x2, y2;
    GetGridCenter( gx, 0, x2, y2 ); // we only use "x2"
    return GetPotential( x1, x2 );
}

double MyNLP::GetGradPotentialToGrid( const double& x1, const int& gx )
{
    if( gx < 0 || gx >= m_potentialGridSize )
	return 0.0;
    double x2, y2;
    GetGridCenter( gx, 0, x2, y2 ); // we only use "x2"
    return GetGradPotential( x1, x2 );
}*/

void MyNLP::GetClosestGrid( const double& x1, const double& y1, int& gx, int& gy ) 
{
    gx = static_cast<int>( floor( ( x1 - m_pDB->m_coreRgn.left ) / m_potentialGridWidth ) );
    gy = static_cast<int>( floor( ( y1 - m_pDB->m_coreRgn.bottom ) / m_potentialGridHeight ) );

    // DEBUG
    /*if( gy >= m_gridPotential.size() || gy < 0 )
    {
	printf( "gridHeight= %f, x1= %f, y1= %f, bottom= %f, top= %f, gy= %d\n", 
		m_potentialGridHeight, x1, y1, m_pDB->m_coreRgn.bottom, m_pDB->m_coreRgn.top , gy );
    }
    if( gx >= m_gridPotential.size() || gx < 0)
    {
	printf( "gridWidth = %f, y1 = %f, x1 = %f, left = %f, right = %f, gx = %d\n", 
		m_potentialGridWidth, y1, x1, m_pDB->m_coreRgn.left, m_pDB->m_coreRgn.right, gx );
    }*/
    
#if 0    
    assert( gx >= 0 );
    assert( gy >= 0 );
    assert( gx < (int)m_gridPotential.size() );
    assert( gy < (int)m_gridPotential.size() );
#endif
}

void MyNLP::ClearDensityGrid()
{
    for( unsigned int i=0; i<m_gridDensity.size(); i++ )
	for( unsigned int j=0; j<m_gridDensity[i].size(); j++ )
	    m_gridDensity[i][j] = 0.0;
}


void MyNLP::UpdateDensityGridSpace( const int& n, const double* x )
{
    double allSpace = m_gridDensityWidth * m_gridDensityHeight;
    for( unsigned int i=0; i<m_gridDensity.size(); i++ )
	for( unsigned int j=0; j<m_gridDensity[i].size(); j++ )
	    m_gridDensitySpace[i][j] = allSpace;
   
    // TEST
    //return;
    
    // for each cell b, update the corresponding bin area
    for( int b=0; b<(int)m_pDB->m_modules.size(); b++ )
    {
	if( false == m_pDB->m_modules[b].m_isFixed )
	    continue;

	double w  = m_pDB->m_modules[b].m_width;
	double h  = m_pDB->m_modules[b].m_height;
	double left   = x[b*2]   - w * 0.5;
	double bottom = x[b*2+1] - h * 0.5;
	double right  = left   + w;
	double top    = bottom + h;

	if( w == 0 || h == 0 )
	    continue;
	
	// find nearest bottom-left gird
	int gx = static_cast<int>( floor( (left   - m_pDB->m_coreRgn.left)   / m_gridDensityWidth ) );
	int gy = static_cast<int>( floor( (bottom - m_pDB->m_coreRgn.bottom) / m_gridDensityHeight ) );

	if( gx < 0 )  gx = 0;
	if( gy < 0 )  gy = 0;
	
	for( int xOff = gx; xOff < (int)m_gridDensity.size(); xOff++ )
	{
	    double binLeft  = m_pDB->m_coreRgn.left + xOff * m_gridDensityWidth;
	    double binRight = binLeft + m_gridDensityWidth;
	    if( binLeft >= right )
		break;
	    
	    for( int yOff = gy; yOff < (int)m_gridDensity[xOff].size(); yOff ++ )
	    {
		double binBottom = m_pDB->m_coreRgn.bottom + yOff * m_gridDensityHeight;
		double binTop    = binBottom + m_gridDensityHeight;
		if( binBottom >= top )
		    break;

		m_gridDensitySpace[xOff][yOff] -= 
		    getOverlap( left, right, binLeft, binRight ) * 
		    getOverlap( bottom, top, binBottom, binTop );
	    }
	}

    } // each module

    int zeroSpaceCount = 0;
    m_totalFreeSpace = 0;
    for( unsigned int i=0; i<m_gridDensity.size(); i++ )
	for( unsigned int j=0; j<m_gridDensity[i].size(); j++ )
	{
	    if( m_gridDensitySpace[i][j] < 1e-5 )
	    {
		m_gridDensitySpace[i][j] = 0.0;
		zeroSpaceCount ++;
	    }
	    m_totalFreeSpace += m_gridDensitySpace[i][j];
	}
    if( param.bShow )
	printf( "DBIN: zero space bins: %d\n", zeroSpaceCount );
}


void MyNLP::UpdateDensityGrid( const int& n, const double* x )
{
    ClearDensityGrid();
    
    // for each cell b, update the corresponding bin area
    for( int b=0; b<(int)m_pDB->m_modules.size(); b++ )
    {
	if(  m_pDB->m_modules[b].m_isOutCore || m_pDB->m_modules[b].m_isFixed )
	    continue;

	double w  = m_pDB->m_modules[b].m_width;
	double h  = m_pDB->m_modules[b].m_height;

	// bottom-left 
	double left   = x[b*2]   - w * 0.5;
	double bottom = x[b*2+1] - h * 0.5;
	double right  = left   + w;
	double top    = bottom + h;

	// find nearest gird
	int gx = static_cast<int>( floor( (left - m_pDB->m_coreRgn.left) / m_gridDensityWidth ) );
	int gy = static_cast<int>( floor( (bottom - m_pDB->m_coreRgn.bottom) / m_gridDensityHeight ) );
	if( gx < 0 ) gx = 0;
	if( gy < 0 ) gy = 0;

	// Block is always inside the core region. Do not have to check boundary.
	//double debug_area = 0;
	for( int xOff = gx; xOff < (int)m_gridDensity.size(); xOff++ )
	{
	    double binLeft = m_pDB->m_coreRgn.left + m_gridDensityWidth * xOff;
	    double binRight = binLeft + m_gridDensityWidth;
	    if( binLeft >= right )
		break;
	    
	    for( int yOff = gy; yOff < (int)m_gridDensity[xOff].size(); yOff++ )
	    {
		double binBottom = m_pDB->m_coreRgn.bottom + m_gridDensityHeight * yOff;
		double binTop    = binBottom + m_gridDensityHeight;
		if( binBottom >= top )
		    break;

		double area = 
		    getOverlap( left, right, binLeft, binRight ) *
		    getOverlap( bottom, top, binBottom, binTop );
		
		m_gridDensity[xOff][yOff] += area;
		//debug_area += area;
	    }
	}

	// TODO: check precision
	//printf( " module %d %f %f\n", b, m_pDB->m_modules[b].m_area, debug_area );
	
    } // each module

    /* ??? TODO: check precision
    double totalArea = 0;
    for( unsigned int i=0; i<m_gridDensity.size(); i++ )
	for( unsigned int j=0; j<m_gridDensity[i].size(); j++ )
	{
	    totalArea += m_gridDensity[i][j];
	}
    printf( "%f %f\n", totalArea, m_totalMovableModuleArea );
    assert( totalArea == m_totalMovableModuleArea );*/
}

void MyNLP::CheckDensityGrid()
{
    double totalDensity = 0;
    for( int i=0; i<(int)m_gridDensity.size(); i++ )
	for( int j=0; j<(int)m_gridDensity[i].size(); j++ )
	    totalDensity += m_gridDensity[i][j];

    double totalArea = 0;
    for( int i=0; i<(int)m_pDB->m_modules.size(); i++ )
    {
	if( m_pDB->m_modules[i].m_isOutCore == false )
	    totalArea += m_pDB->m_modules[i].m_area;
    }

    printf( " %f %f\n", totalDensity, totalArea );
}

void MyNLP::CreateDensityGrid( int nGrid )
{
    m_gridDensity.resize( nGrid );
    for( int i=0; i<nGrid; i++ )
	m_gridDensity[i].resize( nGrid );
    
    m_gridDensitySpace.resize( nGrid );
    for( int i=0; i<nGrid; i++ )
	m_gridDensitySpace[i].resize( nGrid );
    
    m_gridDensityWidth  = ( (double)m_pDB->m_coreRgn.right - m_pDB->m_coreRgn.left ) / nGrid;
    m_gridDensityHeight = ( (double)m_pDB->m_coreRgn.top   - m_pDB->m_coreRgn.bottom ) / nGrid;
    m_gridDensityTarget = m_pDB->m_totalModuleArea / ( nGrid * nGrid );
    
    //printf( "Density Target Area = %f\n", m_gridDensityTarget );
    //printf( "Design Density = %f\n", m_gridDensityTarget/m_gridDensityWidth/m_gridDensityHeight );
    // 2006-03-21 compute always overflow area
    
    double alwaysOver = 0.0;
    if( m_targetUtil > 0.0 && m_targetUtil < 1.0 )
    {
	for( unsigned int i=0; i<m_pDB->m_modules.size(); i++ )
	{
	    if( m_pDB->m_modules[i].m_isFixed )
		continue;
	    if( m_pDB->m_modules[i].m_width >= 2*m_gridDensityWidth && m_pDB->m_modules[i].m_height >= 2*m_gridDensityHeight )
		alwaysOver += 
		    (m_pDB->m_modules[i].m_width - m_gridDensityWidth ) * 
		    (m_pDB->m_modules[i].m_height - m_gridDensityHeight ) * 
		    (1.0 - m_targetUtil );
	}
	if( param.bShow )
	    printf( "DBIN: Always over: %.0f (%.1f%%)\n", alwaysOver, alwaysOver/m_pDB->m_totalMovableModuleArea*100.0 );
    }
    m_alwaysOverArea = alwaysOver;
}

/*
double MyNLP::GetDensityGridPanelty()
{
    double den = 0;
    double p;
    for( int i=0; i<(int)m_gridDensity.size(); i++ )
    {
	for( int j=0; j<(int)m_gridDensity[i].size(); j++ )
	{
	    p = ( m_gridDensity[i][j] - m_gridDensityTarget ) / m_gridDensityTarget;
	    p = p*p;
	    den += p;
	}
    }
    return den;
}*/


double MyNLP::GetMaxDensity()
{
    double maxUtilization = 0;
    double binArea = m_gridDensityWidth * m_gridDensityHeight;
    for( int i=0; i<(int)m_gridDensity.size(); i++ )
	for( int j=0; j<(int)m_gridDensity[i].size(); j++ )
	{
	    if( m_gridDensitySpace[i][j] > 1e-5 )
	    {
		//double utilization = m_gridDensity[i][j] / m_gridDensitySpace[i][j];   

		double preplacedArea = binArea - m_gridDensitySpace[i][j];
		double utilization = ( m_gridDensity[i][j] + preplacedArea ) / binArea;   
		
		// TEST
		//double utilization = m_gridDensity[i][j] / m_gridDensityWidth / m_gridDensityHeight;   
		
		if( utilization > maxUtilization )
		    maxUtilization = utilization;
	    }
	}
    return maxUtilization;
}

/*
double MyNLP::GetAvgOverDensity()
{
    //const double targetDensity = 1.0;
    double avgDensity = 0;
    int overflowCount = 0;
    
    for( unsigned int i=0; i<m_gridDensity.size(); i++ )
	for( unsigned int j=0; j<m_gridDensity.size(); j++ )
	    if( m_gridDensity[i][j] > m_gridDensityTarget )
	    {
		overflowCount++;
    		avgDensity += m_gridDensity[i][j];
	    }
    return avgDensity / overflowCount / m_gridDensityTarget;
}
*/

double MyNLP::GetTotalOverDensityLB()
{
    double over = 0;
    for( unsigned int i=0; i<m_gridDensity.size(); i++ )
	for( unsigned int j=0; j<m_gridDensity.size(); j++ )
	{
	    double targetSpace = m_gridDensitySpace[i][j] * m_targetUtil;
	    if( targetSpace > 1e-5 && m_gridDensity[i][j]  > targetSpace  )
    		over += m_gridDensity[i][j] - targetSpace;
	}

    // TODO: remove "1.0"
    return (over -m_alwaysOverArea) / (m_pDB->m_totalMovableModuleArea) + 1.0; 
}


double MyNLP::GetTotalOverDensity()
{
    double over = 0;
    for( unsigned int i=0; i<m_gridDensity.size(); i++ )
	for( unsigned int j=0; j<m_gridDensity.size(); j++ )
	{
	    double targetSpace = m_gridDensitySpace[i][j] * m_targetUtil;
	    if( m_gridDensity[i][j]  > targetSpace  )
    		over += m_gridDensity[i][j] - targetSpace;
	}

    // TODO: remove "1.0"
    return ( over - m_alwaysOverArea) / (m_pDB->m_totalMovableModuleArea) + 1.0; 
}


double MyNLP::GetTotalOverPotential()
{
    double over = 0;
    for( unsigned int i=0; i<m_gridPotential.size(); i++ )
	for( unsigned int j=0; j<m_gridPotential[i].size(); j++ )
	{
	    if( m_gridPotential[i][j]  > m_expBinPotential[i][j]  )
    		over += m_gridPotential[i][j] - m_expBinPotential[i][j];
	}

    // TODO: remove "1.0"
    return (over - m_alwaysOverPotential) / (m_pDB->m_totalMovableModuleArea) + 1.0; 
}

  
double MyNLP::GetNonZeroDensityGridPercent()
{
    double nonZero = 0;
    for( int i=0; i<(int)m_gridDensity.size(); i++ )
	for( int j=0; j<(int)m_gridDensity.size(); j++ )
	{
	    if( m_gridDensity[i][j] > 0 || 
		m_gridDensitySpace[i][j] == 0 
		//|| m_gridDensitySpace[i][j] < m_potentialGridWidth * m_potentialGridHeight 
		)
		nonZero += 1.0;
	}
    return nonZero / m_gridDensity.size() / m_gridDensity.size();
}


double MyNLP::GetNonZeroGridPercent()
{
    double nonZero = 0;
    for( int i=0; i<(int)m_gridPotential.size(); i++ )
	for( int j=0; j<(int)m_gridPotential.size(); j++ )
	    if( m_gridPotential[i][j] > 0 )
		nonZero += 1.0;
    return nonZero / m_gridPotential.size() / m_gridPotential.size();
}


double MyNLP::GetMaxPotential()
{
    double maxDensity = 0;

    for( unsigned int i=0; i<m_gridPotential.size(); i++ )
	for( unsigned int j=0; j<m_gridPotential.size(); j++ )
	    if( m_gridPotential[i][j] > maxDensity )   
		maxDensity = m_gridPotential[i][j];
    return maxDensity;
}


double MyNLP::GetAvgPotential()
{
    const double targetDensity = 1.0;
    double avgDensity = 0;
    int overflowCount = 0;
    
    for( unsigned int i=0; i<m_gridPotential.size(); i++ )
	for( unsigned int j=0; j<m_gridPotential.size(); j++ )
	    if( m_gridPotential[i][j] > targetDensity )
	    {
		overflowCount++;
    		avgDensity += m_gridPotential[i][j];
	    }
    return avgDensity / overflowCount;
}

/*
// 2006-03-01
double MyNLP::GetTotalOverPotential()
{
    double totalOver = 0; 
    for( unsigned int i=0; i<m_gridPotential.size(); i++ )
	for( unsigned int j=0; j<m_gridPotential.size(); j++ )
	{
	    // TODO: use different expPotential in different bins
	    //double targetPotential = _expPotential[i][j] * m_targetUtil;
	    double targetPotential = _expPotential * m_targetUtil;
	    if( m_gridPotential[i][j] > targetPotential )
	    {
    		totalOver += m_gridPotential[i][j] - targetPotential;
	    }
	}
    // TODO: remove 1.0
    return totalOver / m_totalMovableModuleArea + 1.0;
}
*/

// Output potential data for gnuplot
void MyNLP::OutputPotentialGrid( string filename )
{
    int stepSize = (int)m_gridPotential.size() / 100;
    if( stepSize == 0 )
	stepSize = 1;
    FILE* out = fopen( filename.c_str(), "w" );
    double binArea = m_potentialGridWidth * m_potentialGridHeight;
    for( unsigned int j=0; j<m_gridPotential.size(); j+=stepSize )
    {
	for( unsigned int i=0; i<m_gridPotential.size(); i+=stepSize )
	    fprintf( out, "%.03f ", (m_gridPotential[i][j] + m_basePotential[i][j]) / binArea );
	fprintf( out, "\n" );
    }
    fprintf( out, "\n" );
    fclose( out );
}


// Output potential data for gnuplot
void MyNLP::OutputDensityGrid( string filename )
{
    int stepSize = 1;
    FILE* out = fopen( filename.c_str(), "w" );
    for( unsigned int j=0; j<m_gridDensity.size(); j+=stepSize )
    {
	for( unsigned int i=0; i<m_gridDensity.size(); i+=stepSize )
	{
	    double targetSpace = m_gridDensitySpace[i][j] * m_targetUtil;
	    if( m_gridDensity[i][j] > targetSpace )
	    {
		// % overflow
		fprintf( out, "%.03f ", (m_gridDensity[i][j]-targetSpace) / m_pDB->m_totalMovableModuleArea * 100 );
	    }
	    else
	    {
		fprintf( out, "%.03f ", 0.0 );
	    }
	}
	fprintf( out, "\n" );
    }
    fprintf( out, "\n" );
    fclose( out );
}


