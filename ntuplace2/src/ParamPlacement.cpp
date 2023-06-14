#include <string>
#include <iostream>
#include <cstdio>
using namespace std;

#include "ParamPlacement.h"

CParamPlacement param;	// global variable (2006-03-22) donnie

CParamPlacement::CParamPlacement()
{
    hmetis_ubfactor   = -1;	// (2006-02-13) -1 means nouse
    hmetis_run        = 1;     // hMETIS: run #
    hmetis_ctype      = 2;     // hMETIS: coarsening type
    hmetis_rtype      = 2;     // hMETIS: refinement type
    hmetis_vcycle     = 3;     // hMETIS: v-cycle (change default=3 2006-02-02)
    hmetis_debug      = 0;     // hMETIS: debug level
    minBlocks = 1;             // minimum blocks in a region
    ubfactor  = 0.95;	       // The unbalanced factor of NTUplace.   (UNUSE)
    coreShrinkFactor = 1.00;
    coreShrinkWidthFactor = 1.00;
    preLegalFactor  = 0.995;
    bFractionalCut  = true;
    bPrePartitioning = false;
    bRefineParts    = false;       // Bisection/Level refinement
    bRefinePartsAll = false;
    bRefineUseMetis = true;        // hMETIS or FM

    n_repart = 1;

    // stats
    n_ctype_count[1] = 0;
    n_ctype_count[2] = 0;
    n_ctype_count[3] = 0;
    n_ctype_count[4] = 0;
    n_ctype_count[5] = 0;

    aspectRatio = 1.0;

    scaleType = SCALE_TO_MIDLINE;
    //coreUtil = 0.96;                // The miracle number: 0.96.
    coreUtil = -1;                  // for NLP 

    bShow = false;                 // Show the parameters
    bLog = false;                  // Log the result
    bPlot = false;
    outFilePrefix = "out";         // out_global.pl & out_legal.pl

    // input type

    useAUX = true;
    plFilename = "";
    nodesFilename = "";
    
    // Assignment detailed placer
    de_MW = 20;
    de_MM = 30;
    de_btime = true;
    de_time_constrain = 28800;	// 8 hours
    de_window = 45;

    // 2005-12-15
    setOutOrientN = false;	// false: keep original orient
    //	2006-01-16 indark
    n_MacroRowHeight = 1;

    // NLP
    step = 0.3;
    precision = 0.99999;
    topDblStep = false;

    dLpNorm_P = 150;
}

void CParamPlacement::Print()
{
    cout << "Parameters:\n";
    cout << "  seed        = " << seed << endl;
    cout << "  part_run    = " << hmetis_run << endl;
    cout << "  part_ctype  = " << hmetis_ctype << endl;
    cout << "  part_rtype  = " << hmetis_rtype << endl;
    cout << "  part_vcycle = " << hmetis_vcycle << endl;
    //cout << "  ubfactor    = " << ubfactor << endl;
    cout << "  ubfactor    = " << hmetis_ubfactor << endl;
    //cout << "  core_shrink       = " << coreShrinkFactor << endl;
    //cout << "  core_shrink_width = " << coreShrinkWidthFactor << endl;
    cout << "  pre_legal_factor  = " << preLegalFactor << endl;
    cout << "  fractional_cut    = " << bFractionalCut << endl;
    cout << "  refine_parts      = " << bRefineParts << endl;
    cout << "  n_repart          = " << n_repart << endl;
    //cout << "  refine_use_hmetis = " << bRefineUseMetis << endl;
    //cout << "  prelegal_type     = " << scaleType << endl;
    cout << "  aspect_ratio      = " << aspectRatio << endl;
    cout << "  target_util       = " << coreUtil << endl;
    cout << "  out_file_prefix   = " << outFilePrefix << endl;
    cout << "  MacroRowHeight    = " << n_MacroRowHeight << endl;
    cout << "NLP Parameters:\n";
    cout << "  step size         = " << step << endl;
    cout << "  precision         = " << precision << endl;
    cout << "  topdblstep        = " << topDblStep << endl;
    cout << "\n";
}

string CParamPlacement::GetParamString( bool runMincut )
{
	
    char s[500];
    //sprintf( s, "seed=%u run=%d ctype=%d rtype=%d vcycle=%d ubfactor=%.2f util=%.2f shrink=%.2f %.2f %.2f, fracCut=%d prepart=%d refine=%d hmetis=%d scale=%d ar=%.2f",
    //            seed, 
    //            hmetis_run, hmetis_ctype, hmetis_rtype, hmetis_vcycle, 
    //            ubfactor, coreUtil, coreShrinkFactor, coreShrinkWidthFactor, preLegalFactor, 
    //            bFractionalCut, bPrePartitioning, bRefineParts, bRefineUseMetis, scaleType, aspectRatio );
    
    //sprintf( s, "see=%u run=%d c=%d r=%d v=%d util=%.2f prel=%.2f frac=%d prep=%d ref=%d scal=%d ar=%.2f",
    //            seed, 
    //            hmetis_run, hmetis_ctype, hmetis_rtype, hmetis_vcycle, 
    //            coreUtil, preLegalFactor, 
    //            bFractionalCut, bPrePartitioning, bRefineParts, scaleType, aspectRatio );

    if( runMincut )
    {
	sprintf( s, "sd%d run%d ct%d util%.2f prel%.3f fr%d ref%d:%d st%d ub%d",
		(int)seed, 
		(int)hmetis_run, (int)hmetis_ctype, 
		coreUtil, preLegalFactor, 
		bFractionalCut, bRefineParts, n_repart,
		(int)scaleType,
		hmetis_ubfactor );
    }
    else
    {
	sprintf( s, "util%.2f %.2f", coreUtil, step );
    }

    return string(s);
}

