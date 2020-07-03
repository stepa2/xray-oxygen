/*
 * LWSDK Header File
 *
 * LWDYNA.H -- LightWave DynaTypes
 *
 * This header defines the types and macros for simple DynaTypes.
 *
 *---------------------------------------------------------------------------
 * Copyright � 2015 NewTek, Inc. and its licensors. All rights reserved.
 *
 * This file contains confidential and proprietary information of
 * NewTek, Inc., and is subject to the terms of the LightWave End User
 * License Agreement (EULA).
 */
#ifndef LWSDK_DYNA_H
#define LWSDK_DYNA_H

#include <stddef.h>
#include <lwmonitor.h>
#include <lwxpanel.h>

/*
 * DynaType codes.
 */
typedef int             DynaType;

#define DY_NULL         0
#define DY_STRING       1
#define DY_INTEGER      2
#define DY_FLOAT        3
#define DY_DISTANCE     4
#define DY_VINT         5
#define DY_VFLOAT       6
#define DY_VDIST        7
#define DY_BOOLEAN      8
#define DY_CHOICE       9
#define DY_SURFACE      10
#define DY_FONT         11
#define DY_TEXT         12
#define DY_LAYERS       13
#define DY_CUSTOM       14
#define DY_RANGE        15
#define DY_LWITEM       16
#define DY_PERCENT      17
#define DY_POPUP        18
#define DY_AREA         19
#define DY_XPANEL       20
#define DY_TREE         21
#define DY_MLIST        22
#define DY_POINTER      23
#define DY_ENVELOPE     24
#define DY_TEXTURE      25
#define DY__LAST        DY_TEXTURE


/*
 * DynaValue union.
 */
typedef struct st_DyValString {
    DynaType         type;
    char            *buf /* language encoded */;
    int              bufLen;
} DyValString;

typedef struct st_DyValInt {
    DynaType         type;
    int              value;
    int              defVal;
} DyValInt;

typedef struct st_DyValFloat {
    DynaType         type;
    double           value;
    double           defVal;
} DyValFloat;

typedef struct st_DyValIVector {
    DynaType         type;
    int              val[3];
    int              defVal;
} DyValIVector;

typedef struct st_DyValFVector {
    DynaType         type;
    double           val[3];
    double           defVal;
} DyValFVector;

typedef struct st_DyValCustom {
    DynaType         type;
    size_t           val[4];
} DyValCustom;

typedef struct st_DyValPointer {
    DynaType       type;
    void           *ptr;
} DyValPointer;

typedef union un_DynaValue {
    DynaType         type;
    DyValString      str;
    DyValInt         intv;
    DyValFloat       flt;
    DyValIVector     ivec;
    DyValFVector     fvec;
    DyValPointer     ptr;
    DyValCustom      cust;
} DynaValue;


/*
 * Conversion hints.
 */
typedef struct st_DyChoiceHint {
    const char      *item /* language encoded */;
    int              value;
} DyChoiceHint;

typedef struct st_DyBitfieldHint {
    char             code;
    int              bitval;
} DyBitfieldHint;

typedef struct st_DynaStringHint {
    DyChoiceHint    *chc;
    DyBitfieldHint  *bits;
} DynaStringHint;


/*
 * Dynamic Requester types.
 */
typedef struct st_DynaRequest   *DynaRequestID;

typedef struct st_DyReqStringDesc {
    DynaType         type;
    int              width;
} DyReqStringDesc;

typedef struct st_DyReqChoiceDesc {
    DynaType         type;
    const char     **items /* language encoded */;
    int              vertical;
} DyReqChoiceDesc;

typedef struct st_DyReqTextDesc {
    DynaType         type;
    const char     **text /* language encoded */;
} DyReqTextDesc;

typedef union un_DyReqControlDesc {
    DynaType         type;
    DyReqStringDesc  string;
    DyReqChoiceDesc  choice;
    DyReqTextDesc    text;
} DyReqControlDesc;



/*
 * DynaType and DynaValue error codes.
 */
#define DYERR_NONE                0
#define DYERR_MEMORY            (-1)
#define DYERR_BADTYPE           (-2)
#define DYERR_BADSEQ            (-3)
#define DYERR_BADCTRLID         (-4)
#define DYERR_TOOMANYCTRL       (-5)
#define DYERR_INTERNAL          (-6)


/*
 * DynaValue conversion global service.
 */
#define LWDYNACONVERTFUNC_GLOBAL    "LWM: Dynamic Conversion"

typedef int DynaConvertFunc (const DynaValue *, DynaValue *,
                 const DynaStringHint *);


/*
 * Dynamic requester service.
 */
#define LWDYNAREQFUNCS_GLOBAL       "LWM: Dynamic Request 2"

typedef struct st_DynaReqFuncs {
    DynaRequestID   (*create)   (const char *title /* language encoded */);
    int             (*addCtrl)  (DynaRequestID, const char *label /* language encoded */, DyReqControlDesc *);
    DynaType        (*ctrlType) (DynaRequestID, int);
    int             (*valueSet) (DynaRequestID, int, DynaValue *);
    int             (*valueGet) (DynaRequestID, int, DynaValue *);
    int             (*post)     (DynaRequestID);
    void            (*destroy)  (DynaRequestID);
    LWXPanelID      (*xpanel)   (DynaRequestID);
} DynaReqFuncs;


/*
 * Global monitor.
 */
#define LWDYNAMONITORFUNCS_GLOBAL   "LWM: Dynamic Monitor"

typedef struct st_DynaMonitorFuncs {
    LWMonitor * (*create)  (const char *title /* language encoded */, const char *text /* language encoded */);
    void        (*destroy) (LWMonitor *monitor);
} DynaMonitorFuncs;


#endif
