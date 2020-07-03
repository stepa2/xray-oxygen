/*
 * LWSDK Header File
 *
 * LWDOPETRACK.H -- Layout Dopetrack Interface
 *
 * This header contains declarations that allow a Tool-class plug-in
 * to take over most of the functionality of the Layout Dopetrack
 *
 *---------------------------------------------------------------------------
 * Copyright � 2015 NewTek, Inc. and its licensors. All rights reserved.
 *
 * This file contains confidential and proprietary information of
 * NewTek, Inc., and is subject to the terms of the LightWave End User
 * License Agreement (EULA).
 */
#ifndef LWSDK_DOPETRACK_H
#define LWSDK_DOPETRACK_H

#include <lwtypes.h>
#include <lwenvel.h>

#define LWDOPETRACK_GLOBAL      "Dopetrack Proxy"

#ifndef RGB_
#define RGB_(r,g,b)    (((r) << 16) | ((b) << 8) | (g))
#endif

#define TIME_MARKER         ((LWTime)99999.0)

typedef enum
{
    DTOPMODE_CHANNEL = 0,
    DTOPMODE_GLOBAL
} DTOperatingMode;

typedef enum
{
    DTEVENT_KEYFUNCTION,
    DTEVENT_EDITSTATECHANGE,
    DTEVENT_BAKEZONECHANGE,
    DTEVENT_UNDO,
    DTEVENT_REDO
} DTEventType;

typedef enum
{
    DTACTION_SELECT,
    DTACTION_SHIFT,
    DTACTION_SHIFT_COPY,
    DTACTION_CREATE_KEY,
    DTACTION_DELETE_KEY
} DTKeyAction;

typedef enum
{
    DTMOUSE_DOWN,
    DTMOUSE_MOVE,
    DTMOUSE_UP
} DTMouseEvent;

typedef struct _dtevent_mouse_st
{
    DTMouseEvent    event;
    int             context;    // 0=left margin, 1=right margin
    int             x,y;        // relative to drawing area
    int             w,h;        // dimensions of drawing area
    int             button;     // 0=left, 1=right, 2=middle
    int             count;      // click count: 1=single, 2=double, etc.
} DTMouseParam, *DTMouseParamID;

typedef struct _dtevent_keyselect_st
{
    int             count;
    LWTime          *indices;   // list of time indices effected
    LWTime          offset;     // shift offset of time indices (if DTACTION_SHIFT/DTACTION_SHIFT_COPY)
} DTKeySelect, *DTKeySelectID;

typedef struct _dtevent_bakezone_st
{
    int             count;

    // parallel arrays
    int             *start_frame,*end_frame;
    LWTime          *start_time,*end_time;
    int             *selected;
} DTBakeZone, *DTBakeZoneID;

typedef struct _dteventparam_st
{
    DTEventType event;
    int         value;
    LWTime      offset;
} DTEventParam, *DTEventParamID;

// menu management functions

typedef int             (*DTMenuCount)(int);
typedef int             (*DTMenuSub)(int,int);
typedef int        *    (*DTMenuSep)(int);
typedef int             (*DTMenuEnable)(int,int);
typedef const char * /* language encoded */ (*DTMenuItem)(int,int);
typedef void            (*DTMenuSelect)(int,int);
typedef int             (*DTMenuInitial)(void);

// events on the dopetrack

typedef void            (*DTUserEvent)(DTEventParamID);
typedef int             (*DTAllow)(DTKeyAction);
typedef void            (*DTMouse)(DTMouseParamID);

typedef struct _dtmenucallbacks
{
    DTMenuCount     menuCount;
    DTMenuSub       menuSubMenu;
    DTMenuSep       menuSep;
    DTMenuEnable    menuEnable;
    DTMenuItem      menuItem;
    DTMenuSelect    menuSelect;
    DTMenuInitial   menuInitial;
} DTMenuCallbacks, *DTMenuCallbacksID;

typedef struct _dttoolcallbacks
{
    DTUserEvent     userEvent;
    DTAllow         allow;
    DTMouse         mouseEvent;
    DTMenuCallbacks menu;
} DTToolCallbacks, *DTToolCallbacksID;

typedef struct _dtdrawcallbacks
{
    int         *(*context)(int side);       // 0=left margin, 1=right margin
    void        (*erase)(int x,int y,int w,int h);
    void        (*pixel)(int x,int y,int color);        // use RGB_() to construct color
    void        (*line)(int x1,int y1,int x2,int y2,int color);
    void        (*rectOutline)(int x,int y,int w,int h,int color);
    void        (*rectFilled)(int x,int y,int w,int h,int bcolor,int fcolor);
    void        (*border)(int x,int y,int w,int h);
    void        (*divider)(int x,int y,int w);
    void        (*text)(int x,int y,int color,const char *text /* language encoded */);
    void        (*text_box)(int x,int y,int w,int h,const char *text /* language encoded */);
    void        (*button)(int x,int y,int w,int h,int color);
    void        (*flush)(void);
} DTDrawFuncs, *DTDrawFuncsID;

typedef struct st_LWDopetrackProxy
{
    void            (*toolRegister)(DTToolCallbacksID);
    void            (*toolRelease)(void);

    void            (*exposeEnvelopes)(LWEnvelopeID *,const char **channel_names /* utf8 encoded? */,int *states);
    void            (*refreshDisplay)(void);

    DTKeySelectID   (*querySelectedKeys)(void);
    void            (*querySelection)(LWTime *,LWTime *);

    const LWTime   *(*queryMarkers)(void);
    void            (*addMarker)(LWTime);
    void            (*remMarker)(LWTime);

    DTBakeZoneID    (*queryBakeZones)(void);
    void            (*addBakeZone)(LWTime,LWTime);
    void            (*remBakeZone)(LWTime,LWTime);

    DTOperatingMode (*queryOpMode)(void);

    void            (*displayMenu)(DTMenuCallbacksID);

    int             (*visible)(void);

    DTDrawFuncs     drawfuncs;
} LWDopetrackProxy, *LWDopetrackProxyID;

#endif
