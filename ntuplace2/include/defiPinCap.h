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

#ifndef defiPinCap_h
#define defiPinCap_h
#include "defiKRDefs.h"
#include <stdio.h>

/*
 * pin num
 */

/*
 * capacitance
 */

typedef struct defiPinCap_s {
  int             pin_;
  double          cap_;
} defiPinCap;

EXTERN void
defiPinCap_setPin
  PROTO_PARAMS(( defiPinCap * this,
                 int p ));

EXTERN void
defiPinCap_setCap
  PROTO_PARAMS(( defiPinCap * this,
                 double d ));

EXTERN int
defiPinCap_pin
  PROTO_PARAMS(( const defiPinCap * this ));

EXTERN double
defiPinCap_cap
  PROTO_PARAMS(( const defiPinCap * this ));

EXTERN void
defiPinCap_print
  PROTO_PARAMS(( const defiPinCap * this,
                 FILE * f ));

/*
 * 5.5
 */

/*
 * ANTENNAPINGATEAREA
 */

/*
 * ANTENNAPINMAXAREACAR
 */

/*
 * ANTENNAPINMAXSIDEAREACAR
 */

/*
 * ANTENNAPINMAXCUTCAR
 */

/*
 * 5.4
 */

/*
 * 5.4 AntennaPinGateArea
 */

/*
 * 5.4 Layer
 */

/*
 * 5.4
 */

/*
 * 5.4 AntennaPinMaxAreaCar
 */

/*
 * 5.4 Layer
 */

/*
 * 5.4
 */

/*
 * 5.4 AntennaPinMaxSideAreaCar
 */

/*
 * 5.4 Layer
 */

/*
 * 5.4
 */

/*
 * 5.4 AntennaPinMaxCutCar
 */

/*
 * 5.4 Layer
 */

typedef struct defiPinAntennaModel_s {
  char           *oxide_;
  int             numAPinGateArea_;
  int             APinGateAreaAllocated_;
  int            *APinGateArea_;
  char          **APinGateAreaLayer_;
  int             numAPinMaxAreaCar_;
  int             APinMaxAreaCarAllocated_;
  int            *APinMaxAreaCar_;
  char          **APinMaxAreaCarLayer_;
  int             numAPinMaxSideAreaCar_;
  int             APinMaxSideAreaCarAllocated_;
  int            *APinMaxSideAreaCar_;
  char          **APinMaxSideAreaCarLayer_;
  int             numAPinMaxCutCar_;
  int             APinMaxCutCarAllocated_;
  int            *APinMaxCutCar_;
  char          **APinMaxCutCarLayer_;
} defiPinAntennaModel;

EXTERN defiPinAntennaModel *
defiPinAntennaModel_Create
  PROTO_PARAMS((  ));

EXTERN void
defiPinAntennaModel_Init
  PROTO_PARAMS(( defiPinAntennaModel * this ));

EXTERN void
defiPinAntennaModel_Delete
  PROTO_PARAMS(( defiPinAntennaModel * this ));

EXTERN void
defiPinAntennaModel_clear
  PROTO_PARAMS(( defiPinAntennaModel * this ));

EXTERN void
defiPinAntennaModel_Destroy
  PROTO_PARAMS(( defiPinAntennaModel * this ));

EXTERN void
defiPinAntennaModel_setAntennaModel
  PROTO_PARAMS(( defiPinAntennaModel * this,
                 int oxide ));

EXTERN void
defiPinAntennaModel_addAPinGateArea
  PROTO_PARAMS(( defiPinAntennaModel * this,
                 int value,
                 const char *layer ));

EXTERN void
defiPinAntennaModel_addAPinMaxAreaCar
  PROTO_PARAMS(( defiPinAntennaModel * this,
                 int value,
                 const char *layer ));

EXTERN void
defiPinAntennaModel_addAPinMaxSideAreaCar
  PROTO_PARAMS(( defiPinAntennaModel * this,
                 int value,
                 const char *layer ));

EXTERN void
defiPinAntennaModel_addAPinMaxCutCar
  PROTO_PARAMS(( defiPinAntennaModel * this,
                 int value,
                 const char *layer ));

EXTERN char *
defiPinAntennaModel_antennaOxide
  PROTO_PARAMS(( const defiPinAntennaModel * this ));

EXTERN int
defiPinAntennaModel_hasAPinGateArea
  PROTO_PARAMS(( const defiPinAntennaModel * this ));

EXTERN int
defiPinAntennaModel_numAPinGateArea
  PROTO_PARAMS(( const defiPinAntennaModel * this ));

EXTERN int
defiPinAntennaModel_APinGateArea
  PROTO_PARAMS(( const defiPinAntennaModel * this,
                 int index ));

EXTERN const char *
defiPinAntennaModel_APinGateAreaLayer
  PROTO_PARAMS(( const defiPinAntennaModel * this,
                 int index ));

EXTERN int
defiPinAntennaModel_hasAPinMaxAreaCar
  PROTO_PARAMS(( const defiPinAntennaModel * this ));

EXTERN int
defiPinAntennaModel_numAPinMaxAreaCar
  PROTO_PARAMS(( const defiPinAntennaModel * this ));

EXTERN int
defiPinAntennaModel_APinMaxAreaCar
  PROTO_PARAMS(( const defiPinAntennaModel * this,
                 int index ));

EXTERN const char *
defiPinAntennaModel_APinMaxAreaCarLayer
  PROTO_PARAMS(( const defiPinAntennaModel * this,
                 int index ));

EXTERN int
defiPinAntennaModel_hasAPinMaxSideAreaCar
  PROTO_PARAMS(( const defiPinAntennaModel * this ));

EXTERN int
defiPinAntennaModel_numAPinMaxSideAreaCar
  PROTO_PARAMS(( const defiPinAntennaModel * this ));

EXTERN int
defiPinAntennaModel_APinMaxSideAreaCar
  PROTO_PARAMS(( const defiPinAntennaModel * this,
                 int index ));

EXTERN const char *
defiPinAntennaModel_APinMaxSideAreaCarLayer
  PROTO_PARAMS(( const defiPinAntennaModel * this,
                 int index ));

EXTERN int
defiPinAntennaModel_hasAPinMaxCutCar
  PROTO_PARAMS(( const defiPinAntennaModel * this ));

EXTERN int
defiPinAntennaModel_numAPinMaxCutCar
  PROTO_PARAMS(( const defiPinAntennaModel * this ));

EXTERN int
defiPinAntennaModel_APinMaxCutCar
  PROTO_PARAMS(( const defiPinAntennaModel * this,
                 int index ));

EXTERN const char *
defiPinAntennaModel_APinMaxCutCarLayer
  PROTO_PARAMS(( const defiPinAntennaModel * this,
                 int index ));

/*
 * 5.5
 */

/*
 * optional parts
 */

/*
 * 5.4
 */

/*
 * ANTENNAPINPARTIALMETALAREA
 */

/*
 * ANTENNAPINPARTIALMETALSIDEAREA
 */

/*
 * ANTENNAPINDIFFAREA
 */

/*
 * ANTENNAPINPARTIALCUTAREA
 */

/*
 * 5.5
 */

/*
 * allocated size of pin name
 */

/*
 * allocated size of net name
 */

/*
 * orient 0-7
 */

/*
 * allocated size of length
 */

/*
 * allocated size of direction
 */

/*
 * allocated size of layer
 */

/*
 * placement
 */

/*
 * 5.5 AntennaModel
 */

/*
 * 5.4
 */

/*
 * 5.4 AntennaPinPartialMetalArea
 */

/*
 * 5.4 Layer
 */

/*
 * 5.4
 */

/*
 * 5.4 AntennaPinPartialMetalSideArea
 */

/*
 * 5.4 Layer
 */

/*
 * 5.4
 */

/*
 * 5.4 AntennaPinDiffArea
 */

/*
 * 5.4 Layer
 */

/*
 * 5.4
 */

/*
 * 5.4 AntennaPinPartialCutArea
 */

/*
 * 5.4 Layer
 */

typedef struct defiPin_s {
  int             pinNameLength_;
  char           *pinName_;
  int             netNameLength_;
  char           *netName_;
  char            hasDirection_;
  char            hasUse_;
  char            hasLayer_;
  char            placeType_;
  char            orient_;
  int             useLength_;
  char           *use_;
  int             directionLength_;
  char           *direction_;
  int             layerLength_;
  char           *layer_;
  int             xl_, yl_, xh_, yh_;
  int             x_, y_;
  int             hasSpecial_;
  int             numAntennaModel_;
  int             antennaModelAllocated_;
  defiPinAntennaModel **antennaModel_;
  int             numAPinPartialMetalArea_;
  int             APinPartialMetalAreaAllocated_;
  int            *APinPartialMetalArea_;
  char          **APinPartialMetalAreaLayer_;
  int             numAPinPartialMetalSideArea_;
  int             APinPartialMetalSideAreaAllocated_;
  int            *APinPartialMetalSideArea_;
  char          **APinPartialMetalSideAreaLayer_;
  int             numAPinDiffArea_;
  int             APinDiffAreaAllocated_;
  int            *APinDiffArea_;
  char          **APinDiffAreaLayer_;
  int             numAPinPartialCutArea_;
  int             APinPartialCutAreaAllocated_;
  int            *APinPartialCutArea_;
  char          **APinPartialCutAreaLayer_;
} defiPin;

EXTERN defiPin *
defiPin_Create
  PROTO_PARAMS((  ));

EXTERN void
defiPin_Init
  PROTO_PARAMS(( defiPin * this ));

EXTERN void
defiPin_Delete
  PROTO_PARAMS(( defiPin * this ));

EXTERN void
defiPin_Destroy
  PROTO_PARAMS(( defiPin * this ));

EXTERN void
defiPin_Setup
  PROTO_PARAMS(( defiPin * this,
                 const char *pinName,
                 const char *netName ));

EXTERN void
defiPin_setDirection
  PROTO_PARAMS(( defiPin * this,
                 const char *dir ));

EXTERN void
defiPin_setUse
  PROTO_PARAMS(( defiPin * this,
                 const char *use ));

EXTERN void
defiPin_setLayer
  PROTO_PARAMS(( defiPin * this,
                 const char *layer,
                 int xl,
                 int yl,
                 int xh,
                 int yh ));

EXTERN void
defiPin_setPlacement
  PROTO_PARAMS(( defiPin * this,
                 int typ,
                 int x,
                 int y,
                 int orient ));

EXTERN void
defiPin_setSpecial
  PROTO_PARAMS(( defiPin * this ));

EXTERN void
defiPin_addAntennaModel
  PROTO_PARAMS(( defiPin * this,
                 int oxide ));

EXTERN void
defiPin_addAPinPartialMetalArea
  PROTO_PARAMS(( defiPin * this,
                 int value,
                 const char *layer ));

EXTERN void
defiPin_addAPinPartialMetalSideArea
  PROTO_PARAMS(( defiPin * this,
                 int value,
                 const char *layer ));

EXTERN void
defiPin_addAPinGateArea
  PROTO_PARAMS(( defiPin * this,
                 int value,
                 const char *layer ));

EXTERN void
defiPin_addAPinDiffArea
  PROTO_PARAMS(( defiPin * this,
                 int value,
                 const char *layer ));

EXTERN void
defiPin_addAPinMaxAreaCar
  PROTO_PARAMS(( defiPin * this,
                 int value,
                 const char *layer ));

EXTERN void
defiPin_addAPinMaxSideAreaCar
  PROTO_PARAMS(( defiPin * this,
                 int value,
                 const char *layer ));

EXTERN void
defiPin_addAPinPartialCutArea
  PROTO_PARAMS(( defiPin * this,
                 int value,
                 const char *layer ));

EXTERN void
defiPin_addAPinMaxCutCar
  PROTO_PARAMS(( defiPin * this,
                 int value,
                 const char *layer ));

EXTERN void
defiPin_addPinPort
  PROTO_PARAMS(( defiPin * this,
                 const char *name ));

EXTERN void
defiPin_setPinPortConnect
  PROTO_PARAMS(( defiPin * this,
                 int type,
                 int x,
                 int y,
                 int orient ));

EXTERN void
defiPin_setPinPortLayer
  PROTO_PARAMS(( defiPin * this,
                 const char *layerName,
                 int xl,
                 int yl,
                 int xh,
                 int yh ));

EXTERN void
defiPin_addPinPortAPortPartialMetalArea
  PROTO_PARAMS(( defiPin * this,
                 int value,
                 const char *layer ));

EXTERN void
defiPin_addPinPortAPortPartialMetalSideArea
  PROTO_PARAMS(( defiPin * this,
                 int value,
                 const char *layer ));

EXTERN void
defiPin_addPinPortAPortGateArea
  PROTO_PARAMS(( defiPin * this,
                 int value,
                 const char *layer ));

EXTERN void
defiPin_addPinPortAPortDiffArea
  PROTO_PARAMS(( defiPin * this,
                 int value,
                 const char *layer ));

EXTERN void
defiPin_addPinPortAPortMaxAreaCar
  PROTO_PARAMS(( defiPin * this,
                 int value,
                 const char *layer ));

EXTERN void
defiPin_addPinPortAPortMaxSideAreaCar
  PROTO_PARAMS(( defiPin * this,
                 int value,
                 const char *layer ));

EXTERN void
defiPin_addPinPortAPortPartialCutArea
  PROTO_PARAMS(( defiPin * this,
                 int value,
                 const char *layer ));

EXTERN void
defiPin_addPinPortAPortMaxCutCar
  PROTO_PARAMS(( defiPin * this,
                 int value,
                 const char *layer ));

EXTERN void
defiPin_clear
  PROTO_PARAMS(( defiPin * this ));

EXTERN const char *
defiPin_pinName
  PROTO_PARAMS(( const defiPin * this ));

EXTERN const char *
defiPin_netName
  PROTO_PARAMS(( const defiPin * this ));

EXTERN int
defiPin_hasDirection
  PROTO_PARAMS(( const defiPin * this ));

EXTERN int
defiPin_hasUse
  PROTO_PARAMS(( const defiPin * this ));

EXTERN int
defiPin_hasLayer
  PROTO_PARAMS(( const defiPin * this ));

EXTERN int
defiPin_hasPlacement
  PROTO_PARAMS(( const defiPin * this ));

EXTERN int
defiPin_isUnplaced
  PROTO_PARAMS(( const defiPin * this ));

EXTERN int
defiPin_isPlaced
  PROTO_PARAMS(( const defiPin * this ));

EXTERN int
defiPin_isCover
  PROTO_PARAMS(( const defiPin * this ));

EXTERN int
defiPin_isFixed
  PROTO_PARAMS(( const defiPin * this ));

EXTERN int
defiPin_placementX
  PROTO_PARAMS(( const defiPin * this ));

EXTERN int
defiPin_placementY
  PROTO_PARAMS(( const defiPin * this ));

EXTERN const char *
defiPin_direction
  PROTO_PARAMS(( const defiPin * this ));

EXTERN const char *
defiPin_use
  PROTO_PARAMS(( const defiPin * this ));

EXTERN const char *
defiPin_layer
  PROTO_PARAMS(( const defiPin * this ));

EXTERN void
defiPin_bounds
  PROTO_PARAMS(( const defiPin * this,
                 int *xl,
                 int *yl,
                 int *xh,
                 int *yh ));

EXTERN int
defiPin_orient
  PROTO_PARAMS(( const defiPin * this ));

EXTERN const char *
defiPin_orientStr
  PROTO_PARAMS(( const defiPin * this ));

EXTERN int
defiPin_hasSpecial
  PROTO_PARAMS(( const defiPin * this ));

EXTERN int
defiPin_hasAPinPartialMetalArea
  PROTO_PARAMS(( const defiPin * this ));

EXTERN int
defiPin_numAPinPartialMetalArea
  PROTO_PARAMS(( const defiPin * this ));

EXTERN int
defiPin_APinPartialMetalArea
  PROTO_PARAMS(( const defiPin * this,
                 int index ));

EXTERN const char *
defiPin_APinPartialMetalAreaLayer
  PROTO_PARAMS(( const defiPin * this,
                 int index ));

EXTERN int
defiPin_hasAPinPartialMetalSideArea
  PROTO_PARAMS(( const defiPin * this ));

EXTERN int
defiPin_numAPinPartialMetalSideArea
  PROTO_PARAMS(( const defiPin * this ));

EXTERN int
defiPin_APinPartialMetalSideArea
  PROTO_PARAMS(( const defiPin * this,
                 int index ));

EXTERN const char *
defiPin_APinPartialMetalSideAreaLayer
  PROTO_PARAMS(( const defiPin * this,
                 int index ));

EXTERN int
defiPin_hasAPinDiffArea
  PROTO_PARAMS(( const defiPin * this ));

EXTERN int
defiPin_numAPinDiffArea
  PROTO_PARAMS(( const defiPin * this ));

EXTERN int
defiPin_APinDiffArea
  PROTO_PARAMS(( const defiPin * this,
                 int index ));

EXTERN const char *
defiPin_APinDiffAreaLayer
  PROTO_PARAMS(( const defiPin * this,
                 int index ));

EXTERN int
defiPin_hasAPinPartialCutArea
  PROTO_PARAMS(( const defiPin * this ));

EXTERN int
defiPin_numAPinPartialCutArea
  PROTO_PARAMS(( const defiPin * this ));

EXTERN int
defiPin_APinPartialCutArea
  PROTO_PARAMS(( const defiPin * this,
                 int index ));

EXTERN const char *
defiPin_APinPartialCutAreaLayer
  PROTO_PARAMS(( const defiPin * this,
                 int index ));

EXTERN int
defiPin_numAntennaModel
  PROTO_PARAMS(( const defiPin * this ));

EXTERN defiPinAntennaModel *
defiPin_antennaModel
  PROTO_PARAMS(( const defiPin * this,
                 int index ));

EXTERN int
defiPin_numPorts
  PROTO_PARAMS(( const defiPin * this ));

EXTERN void
defiPin_print
  PROTO_PARAMS(( const defiPin * this,
                 FILE * f ));

#endif
