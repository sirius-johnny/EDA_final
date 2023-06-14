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

#ifndef defiSite_h
#define defiSite_h
#include "defiKRDefs.h"
#include <stdio.h>

/*
 * Struct holds the data for one site. It is also used for a canplace and
 * cannotoccupy.
 */

/*
 * Name of this.
 */

/*
 * allocated size of siteName_
 */

/*
 * Origin
 */

/*
 * Array step size.
 */

/*
 * orientation
 */

typedef struct defiSite_s {
  char           *siteName_;
  int             nameSize_;
  double          x_orig_, y_orig_;
  double          x_step_, y_step_;
  double          x_num_, y_num_;
  int             orient_;
} defiSite;

EXTERN defiSite *
defiSite_Create
  PROTO_PARAMS((  ));

EXTERN void
defiSite_Init
  PROTO_PARAMS(( defiSite * this ));

EXTERN void
defiSite_Delete
  PROTO_PARAMS(( defiSite * this ));

EXTERN void
defiSite_Destroy
  PROTO_PARAMS(( defiSite * this ));

EXTERN void
defiSite_clear
  PROTO_PARAMS(( defiSite * this ));

EXTERN void
defiSite_setName
  PROTO_PARAMS(( defiSite * this,
                 const char *name ));

EXTERN void
defiSite_setLocation
  PROTO_PARAMS(( defiSite * this,
                 double xorg,
                 double yorg ));

EXTERN void
defiSite_setOrient
  PROTO_PARAMS(( defiSite * this,
                 int orient ));

EXTERN void
defiSite_setDo
  PROTO_PARAMS(( defiSite * this,
                 double x_num,
                 double y_num,
                 double x_step,
                 double y_step ));

EXTERN double
defiSite_x_num
  PROTO_PARAMS(( const defiSite * this ));

EXTERN double
defiSite_y_num
  PROTO_PARAMS(( const defiSite * this ));

EXTERN double
defiSite_x_step
  PROTO_PARAMS(( const defiSite * this ));

EXTERN double
defiSite_y_step
  PROTO_PARAMS(( const defiSite * this ));

EXTERN double
defiSite_x_orig
  PROTO_PARAMS(( const defiSite * this ));

EXTERN double
defiSite_y_orig
  PROTO_PARAMS(( const defiSite * this ));

EXTERN int
defiSite_orient
  PROTO_PARAMS(( const defiSite * this ));

EXTERN const char *
defiSite_orientStr
  PROTO_PARAMS(( const defiSite * this ));

EXTERN const char *
defiSite_name
  PROTO_PARAMS(( const defiSite * this ));

EXTERN void
defiSite_print
  PROTO_PARAMS(( const defiSite * this,
                 FILE * f ));

EXTERN void
defiSite_bumpName
  PROTO_PARAMS(( defiSite * this,
                 int size ));

/* Struct holds the data for a Box */

/*
 * Use the default destructor and constructor.
 */

typedef struct defiBox_s {
  int             xl_, yl_;
  int             xh_, yh_;
} defiBox;

EXTERN void
defiBox_setLowerLeft
  PROTO_PARAMS(( defiBox * this,
                 int x,
                 int y ));

EXTERN void
defiBox_setUpperRight
  PROTO_PARAMS(( defiBox * this,
                 int x,
                 int y ));

EXTERN int
defiBox_xl
  PROTO_PARAMS(( const defiBox * this ));

EXTERN int
defiBox_yl
  PROTO_PARAMS(( const defiBox * this ));

EXTERN int
defiBox_xh
  PROTO_PARAMS(( const defiBox * this ));

EXTERN int
defiBox_yh
  PROTO_PARAMS(( const defiBox * this ));

EXTERN void
defiBox_print
  PROTO_PARAMS(( const defiBox * this,
                 FILE * f ));

#endif
