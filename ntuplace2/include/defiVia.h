/*
 * This  file  is  part  of  the  Cadence  LEF/DEF  Open   Source
 * Distribution,  Product Version 5.5, and is subject to the Cadence LEF/DEF
 * Open Source License Agreement.   Your  continued  use  of this file
 * constitutes your acceptance of the terms of the LEF/DEF Open Source
 * License and an agreement to abide by its  terms.   If you  don't  agree
 * with  this, you must remove this and any other files which are part of the
 * distribution and  destroy any  copies made.
 * 
 * For updates, support, or to become part of the LEF/DEF Community, check
 * www.openeda.org for details.
 */

#ifndef defiVia_h
#define defiVia_h
#include "defiKRDefs.h"
#include <stdio.h>

/*
 * Struct holds the data for one property.
 */

typedef struct defiVia_s {
  char           *name_;
  int             nameLength_;
  char           *pattern_;
  int             patternLength_;
  char          **layers_;
  int            *xl_;
  int            *yl_;
  int            *xh_;
  int            *yh_;
  int             layersLength_;
  int             numLayers_;
} defiVia;

EXTERN defiVia *
defiVia_Create
  PROTO_PARAMS((  ));

EXTERN void
defiVia_Init
  PROTO_PARAMS(( defiVia * this ));

EXTERN void
defiVia_Destroy
  PROTO_PARAMS(( defiVia * this ));

EXTERN void
defiVia_Delete
  PROTO_PARAMS(( defiVia * this ));

EXTERN void
defiVia_setup
  PROTO_PARAMS(( defiVia * this,
                 const char *name ));

EXTERN void
defiVia_addPattern
  PROTO_PARAMS(( defiVia * this,
                 const char *patt ));

EXTERN void
defiVia_addLayer
  PROTO_PARAMS(( defiVia * this,
                 const char *layer,
                 int xl,
                 int yl,
                 int xh,
                 int yh ));

EXTERN const char *
defiVia_name
  PROTO_PARAMS(( const defiVia * this ));

EXTERN const char *
defiVia_pattern
  PROTO_PARAMS(( const defiVia * this ));

EXTERN int
defiVia_hasPattern
  PROTO_PARAMS(( const defiVia * this ));

EXTERN int
defiVia_numLayers
  PROTO_PARAMS(( const defiVia * this ));

EXTERN void
defiVia_layer
  PROTO_PARAMS(( const defiVia * this,
                 int index,
                 char **layer,
                 int *xl,
                 int *yl,
                 int *xh,
                 int *yh ));

EXTERN void
defiVia_print
  PROTO_PARAMS(( const defiVia * this,
                 FILE * f ));

#endif
