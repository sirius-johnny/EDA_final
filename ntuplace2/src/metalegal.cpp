// metalegal.cpp
// 2005-12-20 
// created by donnie

#include "legal.h"
#include "mincutplacer.h"
#include "Prelegalizer.h"

double PlaceLegalize( CPlaceDB& fplan, CParamPlacement& param, double& wl1, double& wl2 )
{
	printf("Peak Memory Usage Before Legal: %f MB\n",GetPeakMemoryUsage());
	printf("Saving BlockPosition.....\n");
	
    fplan.SaveBlockLocation();
    printf("Peak Memory Usage: %f MB\n",GetPeakMemoryUsage());
    printf("Saving BestBlockPosition.....\n");
    fplan.SaveBestBlockLocation();
    printf("Peak Memory Usage: %f MB\n",GetPeakMemoryUsage());
    
    printf( "Prelegal factor = %f\n", param.preLegalFactor );
    
    //////////////////////////////////////////////
    // LEFT
    CPrelegalizer::Prelegalizer( fplan, SCALE_TO_LEFT, param.preLegalFactor );    
    fplan.RestoreCoreRgn();
    CLegalizer check(fplan);
    if( check.legalize( (int)(max( fplan.GetHeight(), fplan.GetWidth() )*1.4/fplan.m_rowHeight), SCALE_TO_LEFT ) )
    {
        fplan.CalcHPWL();
        if( param.bShow )        
            cout << "LEFT wire= " << fplan.GetHPWLp2p() << " (" << 100.0*(fplan.GetHPWLp2p()/wl1-1.0) << "%)\n";
        if( fplan.GetHPWLp2p() < wl2 )
        {
            if( param.bShow )        
                cout << "Save best!\n";
            fplan.SaveBestBlockLocation();
            wl2 = fplan.GetHPWLp2p();
            param.scaleType = SCALE_TO_LEFT;
	
        }
    }
    else
        cout << "\n\nFAIL TO LEGALIZE!!\n\n";
    cout << endl;
    //------------------------------------------
    // RIGHT
	
    fplan.RestoreBlockLocation();
	//Start:=====================(indark)==========================
    //placer.RestoreCoreRgnShrink();
    fplan.m_coreRgn = fplan.m_coreRgnShrink;
    //End:=====================(indark)==========================
    CPrelegalizer::Prelegalizer( fplan, SCALE_TO_RIGHT, param.preLegalFactor );    
    fplan.RestoreCoreRgn();
    check.Init();
    if( check.legalize( (int)(max( fplan.GetHeight(), fplan.GetWidth() )*1.4/fplan.m_rowHeight), SCALE_TO_RIGHT ) )
    {
        fplan.CalcHPWL();
        if( param.bShow )        
            cout << "RIGHT wire= " << fplan.GetHPWLp2p() << " (" << 100.0*(fplan.GetHPWLp2p()/wl1-1.0) << "%)\n";
        if( fplan.GetHPWLp2p() < wl2 )
        {
            if( param.bShow )        
                cout << "Save best!\n";
            fplan.SaveBestBlockLocation();
            wl2 = fplan.GetHPWLp2p();
            param.scaleType = SCALE_TO_RIGHT;
        }
    }
    else
        cout << "\n\nFAIL TO LEGALIZE!!\n\n";
    cout << endl;
    //------------------------------------------
    // CENTER
    fplan.RestoreBlockLocation();
	//Start:=====================(indark)==========================
    //placer.RestoreCoreRgnShrink();
    fplan.m_coreRgn = fplan.m_coreRgnShrink;
    //End:=====================(indark)==========================
    CPrelegalizer::Prelegalizer( fplan, SCALE_TO_MIDLINE, param.preLegalFactor );    
    fplan.RestoreCoreRgn();
    check.Init();
    if( check.legalize( (int)(max( fplan.GetHeight(), fplan.GetWidth() )*1.4/fplan.m_rowHeight), SCALE_TO_MIDLINE ) )
    {
        fplan.CalcHPWL();
        if( param.bShow )        
            cout << "CENTER wire= " << fplan.GetHPWLp2p() << " (" << 100.0*(fplan.GetHPWLp2p()/wl1-1.0) << "%)\n";
        if( fplan.GetHPWLp2p() < wl2 )
        {
            if( param.bShow )        
                cout << "Save best!\n";
            fplan.SaveBestBlockLocation();
            wl2 = fplan.GetHPWLp2p();
            param.scaleType = SCALE_TO_MIDLINE;
        }
    }
    else
        cout << "\n\nFAIL TO LEGALIZE!!\n\n";
    cout << endl;
    //===============================================
    fplan.RestoreBestBlockLocation();
    printf("Peak Memory Usage After Legal: %f MB\n",GetPeakMemoryUsage());
    return wl2;
}



