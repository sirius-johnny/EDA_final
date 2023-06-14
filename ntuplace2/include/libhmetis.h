// libmetis.h
// 2004/8/30 Tung-Chieh Chen

#ifndef LIBHMETIS_H
#define LIBHMETIS_H


    //=========================================
    // hMETIS Options 
    //=========================================
/*

CType:  This is the type of vertex grouping scheme (i.e., matching scheme) to use during 
        the coarsening phase. It is an integer parameter and the possible values are:

        1  Selects the hybrid first-choice scheme (HFC). This scheme is a combination of 
           the first-choice and greedy first-choice scheme described later. This is the 
           scheme used by shmetis.
      * 2  Selects the first-choice scheme (FC). In this scheme vertices are grouped together 
           if they are present in multiple hyperedges. Groups of vertices of arbitrary size 
 Best      are allowed to be collapsed together.
   ==>  3  Selects the greedy first-choice scheme (GFC). In this scheme vertices are grouped 
           based on the firstchoice scheme, but the grouping is biased in favor of faster 
           reduction in the number of the hyperedges that remain in the coarse hypergraphs.
        4  Selects the hyperedge scheme. In this scheme vertices are grouped together that 
           correspond to entire hyperedges. Preference is given to hyperedges that have 
           large weight.
        5  Selects the edge scheme. In this scheme pairs of vertices are grouped together 
           if they are connected by multiple hyperedges.
           
RType:    This is the type of refinement policy to use during the 
          uncoarsening phase. It is an integer parameter and the possible 
		  values are:

		  1  Selects the Fiduccia-Mattheyses (FM) refinement scheme.
        * 2  Selects the one-way Fiduccia-Mattheyses refinement scheme. 
		     In this scheme, during each iteration of the FM algorithm, 
			 vertices are allowed to move only in a single direction.
          3  Selects the early-exit FM refinement scheme. In this scheme, 
		     the FM iteration is aborted if the quality of the solution 
			 does not improve after a relatively small number of vertex moves.

Vcycle:   This parameter selects the type of V-cycle refinement to be used 
          by the algorithm. It is an integer parameter and the possible 
		  values are:

		  0  Does not perform any form of V-cycle refinement.
          1  Performs V-cycle refinement on the final solution of each 
		     bisection step. That is, only the best of the Nruns bisections 
			 are refined using V-cycles. This is the options used by shmetis.
       *  2  Performs V-cycle refinement on each intermediate solution whose 
		     quality is equally good or better than the best found so far. 
			 That is, as hmetis computes Nruns bisections, for each bisection 
			 that matches or improves the best one, it is also further 
			 refined using V-cycles.
          3  Performs V-cycle refinement on each intermediate solution. 
		     That is, each one of the Nruns bisections is also refined using 
			 V-cycles.

Reconst:  This parameter is used to select the scheme to be used in dealing with hyperedges 
          that are being cut during the recursive bisection. It is an integer parameter and 
          the possible values are:

		* 0  This scheme removes any hyperedges that were cut while constructing the two 
             smaller hypergraphs in the recursive bisection step. In other words, once a 
             hyperedge is being cut, it is removed from further consideration. Essentially 
             this scheme focuses on minimizing the number of hyperedges that are being cut.
          1  This scheme reconstructs the hyperedges that are being cut, so that each of the 
             two partitions retain the portion of the hyperedge that corresponds to its set 
             of vertices.
*/


extern "C" {

void HMETIS_PartRecursive( int nvtxs,	// number of vertices
			int nhedges,	// number of hyperedges
			int *vwgts,	// array of vetex weights (size=nvtxs)
			int *eptr,	// i: eind[eptr[i]] ~ eind[eptr[i+1]]
			int *eind,
			int *hewgts,	// array of hyperedge weights (size=nhedges)
			int nparts,	// number of desired partitions
			int ubfactor,	// imbalance factor
			int *options,	
			int *part,	// (RETURN) (size=nvtxs)
			int *edgecut );
			

void HMETIS_PartKway( int nvtxs,	// number of vertices
			int nhedges,	// number of hyperedges
			int *vwgts,	// array of vetex weights (size=nvtxs)
			int *eptr,	// i: eind[eptr[i]] ~ eind[eptr[i+1]]
			int *eind,
			int *hewgts,	// array of hyperedge weights (size=nhedges)
			int nparts,	// number of desired partitions
			int ubfactor,	// imbalance factor
			int *options,	
			int *part,	// (RETURN) (size=nvtxs)
			int *edgecut );



}

#endif
