// 2006-02-27 Tung-Chieh Chen
// The same metric in ISPD 2006 placement contest

#include "placedb.h"
#include "placebin.h"

#include <vector>
using namespace std;

CPlaceBin::CPlaceBin( CPlaceDB& db )
{
    m_pDB = &db;
    m_totalMovableArea = 0;
    for( unsigned int i=0; i<m_pDB->m_modules.size(); i++ )
	if( m_pDB->m_modules[i].m_isFixed == false )
	    m_totalMovableArea += m_pDB->m_modules[i].m_area;
}

void CPlaceBin::CreateGrid( int nGrid )
{
    m_binWidth  = ( m_pDB->m_coreRgn.right - m_pDB->m_coreRgn.left )   / nGrid;
    m_binHeight = ( m_pDB->m_coreRgn.top   - m_pDB->m_coreRgn.bottom ) / nGrid;
    m_binNumberH = nGrid;
    m_binNumberW = nGrid;
    CreateGrid();    
    UpdateBinFreeSpace(); 
    UpdateBinUsage();
}

void CPlaceBin::CreateGrid( double size )
{
    m_binWidth   = size;
    m_binHeight  = size;
    m_binNumberW = static_cast<int>(ceil( ( m_pDB->m_coreRgn.right - m_pDB->m_coreRgn.left ) / size ));
    m_binNumberH = static_cast<int>(ceil( ( m_pDB->m_coreRgn.top   - m_pDB->m_coreRgn.bottom ) / size ));

    CreateGrid();     
    UpdateBinFreeSpace(); 
    UpdateBinUsage();
}

void CPlaceBin::CreateGrid()
{
    m_binSpace.resize( m_binNumberW );
    m_binUsage.resize( m_binNumberW );
    for( int i=0; i<m_binNumberW; i++ )
    {
	m_binSpace[i].resize( m_binNumberH );
	m_binUsage[i].resize( m_binNumberH );
    }
}

void CPlaceBin::ClearBinUsage()
{
    for( int i=0; i<m_binNumberW; i++ )
    {
	for( int j=0; j<m_binNumberH; j++ )
	{
	    m_binUsage[i][j] = 0.0;
	}
    }
}

void CPlaceBin::UpdateBinUsage()
{
    ClearBinUsage();
    for( unsigned int b=0; b<m_pDB->m_modules.size(); b++ )
    {
		if( m_pDB->m_modules[b].m_isOutCore || m_pDB->m_modules[b].m_isFixed )
			continue;

		double debug_area = 0;
		
		double w = m_pDB->m_modules[b].m_width;
		double h = m_pDB->m_modules[b].m_height;

		// bottom-left
			double left   = m_pDB->m_modules[b].m_x;
			double bottom = m_pDB->m_modules[b].m_y;
			double right  = left   + w;
			double top    = bottom + h;

		// find nearest gird
		int binX = static_cast<int>( floor( (left   - m_pDB->m_coreRgn.left)   / m_binWidth ) );
		int binY = static_cast<int>( floor( (bottom - m_pDB->m_coreRgn.bottom) / m_binHeight ) );

		for( int xOff=binX; xOff<m_binNumberW; xOff++ )
		{
			if( xOff<0 )
			continue;
		    
			double binX = GetBinX( xOff );
			if( binX >= right )
			break;

			for( int yOff=binY; yOff<m_binNumberH; yOff++ )
			{
			if( yOff < 0 )
				continue;

			double binY = GetBinY( yOff );
			if( binY >= top )
				break;

			double binXright = binX + m_binWidth;
			double binYtop = binY + m_binHeight;
			if( binXright > m_pDB->m_coreRgn.right )
				binXright = m_pDB->m_coreRgn.right;
			if( binYtop > m_pDB->m_coreRgn.top )
				binYtop = m_pDB->m_coreRgn.top;

			double common_area = 
				getOverlap( left, right, binX, binXright ) *
				getOverlap( bottom, top, binY, binYtop );
			
			m_binUsage[xOff][yOff] += common_area;
			
			debug_area += common_area;
			}
		    
		} // for each bin

		if( debug_area != m_pDB->m_modules[b].m_area )
		{
		    // cell may outside the region
		    //printf( "error in area\n" );
		}	    

    } // for each block
}


void CPlaceBin::UpdateBinFreeSpace()
{
    // calculate total space in bins
    for( int i=0; i<m_binNumberW; i++ )
    {
		for( int j=0; j<m_binNumberH; j++ )
		{
			double left = GetBinX( i );
			double right = left + m_binWidth;
			double bottom = GetBinY( j );
			double top = bottom + m_binHeight;
			if( right > m_pDB->m_coreRgn.right )
			right = m_pDB->m_coreRgn.right;
			if( top > m_pDB->m_coreRgn.top )
			top = m_pDB->m_coreRgn.top;
			m_binSpace[i][j] = (right-left) * (top-bottom);
		}
    }
   
    
    for( unsigned int b=0; b<m_pDB->m_modules.size(); b++ )
    {
	
		if( false == m_pDB->m_modules[b].m_isFixed )
			continue;
		double w = m_pDB->m_modules[b].m_width;
		double h = m_pDB->m_modules[b].m_height;

		// bottom-left
			double left   = m_pDB->m_modules[b].m_x;
			double bottom = m_pDB->m_modules[b].m_y;
			double right  = left   + w;
			double top    = bottom + h;

		// find nearest gird
		int binX = static_cast<int>( floor( (left   - m_pDB->m_coreRgn.left)   / m_binWidth ) );
		int binY = static_cast<int>( floor( (bottom - m_pDB->m_coreRgn.bottom) / m_binHeight ) );

		for( int xOff=binX; xOff<m_binNumberW; xOff++ )
		{
			if( xOff<0 )
			continue;
		    
			double binX = GetBinX( xOff );
			if( binX >= right )
			break;

			for( int yOff=binY; yOff<m_binNumberH; yOff++ )
			{
				if( yOff < 0 )
					continue;

				double binY = GetBinY( yOff );
				if( binY >= top )
					break;

				double binXright = binX + m_binWidth;
				double binYtop = binY + m_binHeight;
				if( binXright > m_pDB->m_coreRgn.right )
					binXright = m_pDB->m_coreRgn.right;
				if( binYtop > m_pDB->m_coreRgn.top )
					binYtop = m_pDB->m_coreRgn.top;
				
				double common_area = 
					getOverlap( left, right, binX, binXright ) *
					getOverlap( bottom, top, binY, binYtop );
				m_binSpace[xOff][yOff] -= common_area;
			}
		    
		} // for each bin

    } // for each block
}

double CPlaceBin::GetNonZeroBinPercent()
{
    int nonZero = 0;
    for( int i=0; i<m_binNumberW; i++ )
    {
	for( int j=0; j<m_binNumberH; j++ )
	{
	    if( m_binUsage[i][j] > 0.0 )
		nonZero++;
	}
    }
    return (double)nonZero / m_binNumberW / m_binNumberH;
}

double CPlaceBin::GetTotalOverflowPercent( const double& util )
{
    double over = 0;
    for( int i=0; i<m_binNumberW; i++ )
    {
	for( int j=0; j<m_binNumberH; j++ )
	{
	    double targetUsage = m_binSpace[i][j] * util;
	    if( m_binUsage[i][j] > targetUsage )
	    {
		over += m_binUsage[i][j] - targetUsage;
	    }
	}
    }
    return over / m_totalMovableArea;
}

double CPlaceBin::GetMaxUtil()
{
    double maxUtil = 0;
    for( int i=0; i<m_binNumberW; i++ )
    {
	for( int j=0; j<m_binNumberH; j++ )
	{
	    double util = (double)m_binUsage[i][j] / m_binSpace[i][j];
	    if( util > maxUtil )
		maxUtil = util;
	}
    }
    return maxUtil;
}

double CPlaceBin::GetPenalty( const double& targetUtil )
{
    double totalOver = 0;
    for( int i=0; i<m_binNumberW; i++ )
    {
	for( int j=0; j<m_binNumberH; j++ )
	{
	    double util = m_binUsage[i][j] / m_binSpace[i][j];
	    if( util > targetUtil )
	    {
		totalOver += m_binUsage[i][j] - m_binSpace[i][j]*targetUtil;
	    }
	}
    }
    double scaled_overflow_per_bin = 
	(totalOver * m_binWidth * m_binHeight * targetUtil) / (m_totalMovableArea * 400 );
    return scaled_overflow_per_bin * scaled_overflow_per_bin;
}

void CPlaceBin::OutputBinUtil( string filename )
{
    FILE* out = fopen( filename.c_str(), "w" );
    //ofstream out( filename.c_str() );
    //double areaPerBin = m_pDB->m_totalMovableModuleArea / m_binNumberH / m_binNumberW;
    double areaPerBin = m_binWidth * m_binHeight;
    double maxUtil = CPlaceBin::GetMaxUtil();
    for( int j=0; j<m_binNumberH; j++ )
    {
	for( int i=0; i<m_binNumberW; i++ )
	{
	    fprintf( out, "%.2f ", m_binUsage[i][j] / areaPerBin );
	    //out << m_binUsage[i][j] / areaPerBin << " ";
	    /*
	    double util = m_binUsage[i][j] / m_binSpace[i][j];
	    if( m_binSpace[i][j] < 1e-10 )
		out << 0 << " ";
	    else
		out << util << " ";
		*/
	}
	fprintf( out, "\n" );
	//out << endl;
    }
    //out.close();
    fclose( out );
}

void CPlaceBin::ShowInfo( const double& targetUtil )
{

    printf( "Phase 0: Total %d rows are processed.\n", 
	    int( (m_pDB->m_coreRgn.top - m_pDB->m_coreRgn.bottom) / m_pDB->m_rowHeight) );
    printf( "ImageWindow=(%d %d %d %d) w/ row_height=%d\n", 
	    (int)m_pDB->m_coreRgn.left, (int)m_pDB->m_coreRgn.bottom,
	    (int)m_pDB->m_coreRgn.right, (int)m_pDB->m_coreRgn.top,
	    (int)m_pDB->m_rowHeight );
   
    int vioNum = 0; 
    double totalFreeSpace = 0;
    double totalOver = 0;
    double maxOver = 0;
    for( int i=0; i<m_binNumberW; i++ )
    {
	for( int j=0; j<m_binNumberH; j++ )
	{
	    if( m_binSpace[i][j] == 0 )
		continue;
	    
	    totalFreeSpace += m_binSpace[i][j];
	    
	    double util = m_binUsage[i][j] / m_binSpace[i][j];
	    if( util > targetUtil )
	    {
		totalOver += m_binUsage[i][j] - m_binSpace[i][j]*targetUtil;
		vioNum ++;
	    }
	    if( (util-targetUtil) > maxOver )
		maxOver = (util-targetUtil) ;

	    //if( m_binUsage[i][j] / m_binSpace[i][j] > targetUtil )
	    //	printf( "bin %d %d musage %d free %d\n", 
	    //		i, j, (int)m_binUsage[i][j], (int)m_binSpace[i][j] );
	}
    }

    int totalBinNumber = m_binNumberW*m_binNumberH;
    
    printf( "Total Row Area=%d\n", 
	    (int)((m_pDB->m_coreRgn.top-m_pDB->m_coreRgn.bottom)*(m_pDB->m_coreRgn.right-m_pDB->m_coreRgn.left)) );
    
    printf( "Phase 1: CMAP Dim: %d x %d BinSize: %d x %d Total %d bins.\n",
	    m_binNumberW, m_binNumberH, (int)m_binWidth, (int)m_binHeight,
	    totalBinNumber );
    
    printf( "Total movable area: %d\n", (int)m_totalMovableArea ); 
    
    printf( "Target density: %f\n", targetUtil );
    
    printf( "Violation num:: %d (%f)    Avg overflow: %f  Max overflow: %f\n",
	    vioNum, (double)vioNum/totalBinNumber, 
	    totalOver/vioNum, 
	    maxOver );
    
    printf( "Overflow per bin: %f       Total overflow amount: %f\n",
	    totalOver/totalBinNumber, totalOver );

    double scaled_overflow_per_bin = 
	(totalOver * m_binWidth * m_binHeight * targetUtil) / (m_totalMovableArea * 400 );
	
    printf( "Scaled Overflow per bin: %f\n", scaled_overflow_per_bin*scaled_overflow_per_bin );    
    
    /*
        Phase 0: Total 890 rows are processed.
	ImageWindow=(459 459 11151 11139) w/ row_height=12
	Total Row Area=114190560
	Phase 1: CMAP Dim: 90 x 89 BinSize: 120 x 120 Total 8010 bins.
	NumNodes: 211447 NumTerminals: 543
	Phase 2: Node file processing is done. Total 211447 objects (terminal 543)
	Total 211447 entries in ObjectDB
	Total movable area: 53307432
	Phase 3: Solution PL file processing is done.
	Total 211447 objects (terminal 543)
	Phase 4: Congestion map construction is done.
	Total 211447 objects (terminal 543)
	Phase 5: Congestion map analysis is done.
	Total 8010 (90 x 89) bins. Target density: 0.900000
	Violation num: 77 (0.009613)    Avg overflow: 0.031305  Max overflow: 0.100000
	Overflow per bin: 2.699625      Total overflow amount: 21624.000000
	Scaled Overflow per bin: 0.000173
    */

}

