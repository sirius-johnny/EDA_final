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

#ifndef defiBLOCKAGES_h
#define defiBLOCKAGES_h
#include <stdio.h>
#include "defiKRDefs.h"

typedef struct defiBlockage_s {
  int             hasLayer_;
  char           *layerName_;
  int             layerNameLength_;
  char           *placementName_;
  int             hasPlacement_;
  int             hasComponent_;
  char           *componentName_;
  int             componentNameLength_;
  int             hasSlots_;
  int             hasFills_;
  int             hasPushdown_;
  int             numRectangles_;
  int             rectsAllocated_;
  int            *xl_;
  int            *yl_;
  int            *xh_;
  int            *yh_;
} defiBlockage;

EXTERN defiBlockage *
defiBlockage_Create
  PROTO_PARAMS((  ));

EXTERN void
defiBlockage_Init
  PROTO_PARAMS(( defiBlockage * this ));

EXTERN void
defiBlockage_Destroy
  PROTO_PARAMS(( defiBlockage * this ));

EXTERN void
defiBlockage_Delete
  PROTO_PARAMS(( defiBlockage * this ));

EXTERN void
defiBlockage_clear
  PROTO_PARAMS(( defiBlockage * this ));

EXTERN void
defiBlockage_setLayer
  PROTO_PARAMS(( defiBlockage * this,
                 const char *name ));

EXTERN void
defiBlockage_setPlacement
  PROTO_PARAMS(( defiBlockage * this ));

EXTERN void
defiBlockage_setComponent
  PROTO_PARAMS(( defiBlockage * this,
                 const char *name ));

EXTERN void
defiBlockage_setSlots
  PROTO_PARAMS(( defiBlockage * this ));

EXTERN void
defiBlockage_setFills
  PROTO_PARAMS(( defiBlockage * this ));

EXTERN void
defiBlockage_setPushdown
  PROTO_PARAMS(( defiBlockage * this ));

EXTERN void
defiBlockage_addRect
  PROTO_PARAMS(( defiBlockage * this,
                 int xl,
                 int yl,
                 int xh,
                 int yh ));

EXTERN int
defiBlockage_hasLayer
  PROTO_PARAMS(( const defiBlockage * this ));

EXTERN int
defiBlockage_hasPlacement
  PROTO_PARAMS(( const defiBlockage * this ));

EXTERN int
defiBlockage_hasComponent
  PROTO_PARAMS(( const defiBlockage * this ));

EXTERN int
defiBlockage_hasSlots
  PROTO_PARAMS(( const defiBlockage * this ));

EXTERN int
defiBlockage_hasFills
  PROTO_PARAMS(( const defiBlockage * this ));

EXTERN int
defiBlockage_hasPushdown
  PROTO_PARAMS(( const defiBlockage * this ));

EXTERN const char *
defiBlockage_layerName
  PROTO_PARAMS(( const defiBlockage * this ));

EXTERN const char *
defiBlockage_layerComponentName
  PROTO_PARAMS(( const defiBlockage * this ));

EXTERN const char *
defiBlockage_placementComponentName
  PROTO_PARAMS(( const defiBlockage * this ));

EXTERN int
defiBlockage_numRectangles
  PROTO_PARAMS(( const defiBlockage * this ));

EXTERN int
defiBlockage_xl
  PROTO_PARAMS(( const defiBlockage * this,
                 int index ));

EXTERN int
defiBlockage_yl
  PROTO_PARAMS(( const defiBlockage * this,
                 int index ));

EXTERN int
defiBlockage_xh
  PROTO_PARAMS(( const defiBlockage * this,
                 int index ));

EXTERN int
defiBlockage_yh
  PROTO_PARAMS(( const defiBlockage * this,
                 int index ));

EXTERN void
defiBlockage_print
  PROTO_PARAMS(( const defiBlockage * this,
                 FILE * f ));

#endif
