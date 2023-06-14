#ifndef PARAMPLACEMENT_H
#define PARAMPLACEMENT_H
#include <string>

enum SCALE_TYPE
{
SCALE_TO_LEFT,
SCALE_TO_RIGHT,
SCALE_TO_MIDLINE,
SCALE_TO_LEFT_BETWEEN_MACRO,
SCALE_TO_RIGHT_BETWEEN_MACRO,
SCALE_TO_MIDLINE_BETWEEN_MACRO
};


class CParamPlacement
{
public:
    CParamPlacement();
    void Print();
    string GetParamString( bool runMincut );    // Get parameter string for logging

    unsigned long int seed;

    int hmetis_run;            // hMETIS: run #
    int hmetis_ctype;          // hMETIS: coarsening type
    int hmetis_rtype;          // hMETIS: refinement type
    int hmetis_vcycle;         // hMETIS: v-cycle
    int hmetis_debug;          // hMETIS: debug level

    int minBlocks;             // minimum blocks in a region
    double ubfactor;	       // The unbalanced factor of NTUplace.   (UNUSE)

    int hmetis_ubfactor;	// 2006-02-13 (donnie) ubfactor for hmetis
    
    double coreShrinkFactor;
    double coreShrinkWidthFactor;
    double preLegalFactor;
    double coreUtil;            // target core utilization

    bool bFractionalCut;
    bool bPrePartitioning;
    bool bRefineParts;          // placement feedback
    bool bRefinePartsAll;	// neighborhood refinement
    bool bRefineUseMetis;       // hMETIS or FM

    int n_repart;               // repartition bad count
    int n_ctype_count[6];

    double aspectRatio;         // The partition aspect ratio

    enum SCALE_TYPE scaleType;
	

    bool bShow;                 // Show the parameters
    bool bLog;                  // Log the result
    bool bPlot;                 // Out the gnuplot figure
    string outFilePrefix;

    // detailed placer
    int de_MW;
    int de_MM;
    bool de_btime;
    int de_time_constrain;
    int de_window;
	
    bool useAUX;		// true: aux; false: lefdef
    string plFilename;
    string nodesFilename;


    // 2005-12-15
    bool setOutOrientN;		// modify output for setting all blocks orient N
   
    // NLP
    double step;
    double precision;
    bool topDblStep;
    double dLpNorm_P;
    
    //indark:060116
    int n_MacroRowHeight;	// minimum # of row height a macro should be
};

extern CParamPlacement param;	// global variable

#endif
