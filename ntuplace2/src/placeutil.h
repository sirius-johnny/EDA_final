// placeutil.h
// created by donnie
// 2005-12-20


#ifndef PLACEUTIL
#define PLACEUTIL

double PlaceLegalize( CPlaceDB& fplan, 
                      CParamPlacement& param, 
		      double& wl1, double& wl2 );

class CPlaceDBScaling
{
    public:
        static void XShift( CPlaceDB&, const double& shift );
        static void XScale( CPlaceDB&, const double& scale );
        static void YShift( CPlaceDB&, const double& shift );
        static void YScale( CPlaceDB&, const double& scale );
};

void CPlaceDBScaling::YShift( CPlaceDB& db, const double& shift )
{
    //printf( "x shift = %f\n", shift );

    // core region
    db.m_coreRgn.bottom += shift;
    db.m_coreRgn.top    += shift;
    db.m_dieArea.bottom += shift;
    db.m_dieArea.top    += shift;

    // blocks
    for( unsigned int i=0; i<db.m_modules.size(); i++ )
    {
        db.m_modules[i].m_y  += shift;
        db.m_modules[i].m_cy += shift;
    }

    if( db.m_modules_bak.size() == db.m_modules.size() )
    {
        for( unsigned int i=0; i<db.m_modules.size(); i++ )
        {
            db.m_modules_bak[i].m_y  += shift;
            db.m_modules_bak[i].m_cy += shift;
        }
    }

    // pins
    for( unsigned int i=0; i<db.m_pins.size(); i++ )
        db.m_pins[i].absY += shift;

    // sites
    for( unsigned int i=0; i<db.m_sites.size(); i++ )
        db.m_sites[i].m_bottom += shift;
}

void CPlaceDBScaling::XShift( CPlaceDB& db, const double& shift )
{
    //printf( "x shift = %f\n", shift );

    // core region
    db.m_coreRgn.left  += shift;
    db.m_coreRgn.right += shift;
    db.m_dieArea.left  += shift;
    db.m_dieArea.right += shift;

    // blocks
    for( unsigned int i=0; i<db.m_modules.size(); i++ )
    {
        db.m_modules[i].m_x += shift;
        db.m_modules[i].m_cx += shift;
    }

    if( db.m_modules_bak.size() == db.m_modules.size() )
    {
        for( unsigned int i=0; i<db.m_modules.size(); i++ )
        {
            db.m_modules_bak[i].m_x += shift;
            db.m_modules_bak[i].m_cx += shift;
        }
    }

    // pins
    for( unsigned int i=0; i<db.m_pins.size(); i++ )
        db.m_pins[i].absX += shift;

    // sites
    for( unsigned int i=0; i<db.m_sites.size(); i++ )
        for( unsigned int j=0; j<db.m_sites[i].m_interval.size(); j++ )
            db.m_sites[i].m_interval[j] += shift;
}

void CPlaceDBScaling::YScale( CPlaceDB& db, const double& scale )
{
    // core region
    db.m_coreRgn.top    *= scale;
    db.m_coreRgn.bottom *= scale;
    db.m_dieArea.bottom *= scale;
    db.m_dieArea.top    *= scale;

    // blocks
    for( unsigned int i=0; i<db.m_modules.size(); i++ )
    {
        db.m_modules[i].m_y  *= scale;
        db.m_modules[i].m_cy *= scale;
        db.m_modules[i].m_height *= scale;
        db.m_modules[i].m_area   *= scale;
    }

    // pins
    for( unsigned int i=0; i<db.m_pins.size(); i++ )
    {
        db.m_pins[i].yOff *= scale;
        db.m_pins[i].absY *= scale;
    }

    // sites
    for( unsigned int i=0; i<db.m_sites.size(); i++ )
        db.m_sites[i].m_bottom *= scale;
}

void CPlaceDBScaling::XScale( CPlaceDB& db, const double& scale )
{
    //printf( "x scale = %f\n", scale );

    // core region
    db.m_coreRgn.left  *= scale;
    db.m_coreRgn.right *= scale;
    db.m_dieArea.left  *= scale;
    db.m_dieArea.right *= scale;

    // blocks
    for( unsigned int i=0; i<db.m_modules.size(); i++ )
    {
        db.m_modules[i].m_x  *= scale;
        db.m_modules[i].m_cx *= scale;
        db.m_modules[i].m_width *= scale;
        db.m_modules[i].m_area  *= scale;
    }

    // pins
    for( unsigned int i=0; i<db.m_pins.size(); i++ )
    {
        db.m_pins[i].xOff *= scale;
        db.m_pins[i].absX *= scale;
    }

    // sites
    for( unsigned int i=0; i<db.m_sites.size(); i++ )
    {
        db.m_sites[i].m_step *= scale;
        for( unsigned int j=0; j<db.m_sites[i].m_interval.size(); j++ )
            db.m_sites[i].m_interval[j] *= scale;
    }
}

#endif
