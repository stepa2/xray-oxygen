/*
 * LWSDK Header File: Panel Services
 *
 *---------------------------------------------------------------------------
 * Copyright � 2015 NewTek, Inc. and its licensors. All rights reserved.
 *
 * This file contains confidential and proprietary information of
 * NewTek, Inc., and is subject to the terms of the LightWave End User
 * License Agreement (EULA).
 */
#ifndef LWPANEL_H
#define LWPANEL_H

#include <lwserver.h>
#include <lwxpanel.h>
#define PANEL_SERVICES_NAME     "LWPanelServices 2"
#define LWPANELFUNCS_GLOBAL     "LWPanelServices 2"
#define LWRASTERFUNCS_GLOBAL    "RasterServices 2"
#define LWCONTEXTMENU_GLOBAL    "ContextMenuServices"
#define LWPANELMENU_GLOBAL      "PanelMenuServices 1"
#define LWPANELS_API_VERSION    21

typedef void *LWControlID;
typedef void *LWPanelID;
typedef void *LWContextMenuID;
typedef void *LWPanelMenuID;

typedef struct st_display_Metrics {
    int              width, height;
    int              pixX, pixY;
    int              maxColors, depth;
    int              textHeight;
    int              textAscent;
} display_Metrics;

#define LWDisplayMetrics display_Metrics

/* Values are exchanged with controls by means of these wrappers  */
typedef int             LWType;

#define LWT_NULL         0
#define LWT_STRING       1
#define LWT_INTEGER      2
#define LWT_FLOAT        3
#define LWT_DISTANCE     4
#define LWT_VINT         5
#define LWT_VFLOAT       6
#define LWT_VDIST        7
#define LWT_BOOLEAN      8
#define LWT_CHOICE       9
#define LWT_SURFACE      10
#define LWT_FONT         11
#define LWT_TEXT         12
#define LWT_LAYERS       13
#define LWT_CUSTOM       14
#define LWT_RANGE        15
#define LWT_LWITEM       16
#define LWT_PERCENT      17
#define LWT_POPUP        18
#define LWT_AREA         19
#define LWT_XPANEL       20
#define LWT_TREE         21
#define LWT_MLIST        22
#define LWT_POINTER      23
#define LWT__LAST        LWT_POINTER

typedef struct st_LWValString {
    LWType         type;
    char            *buf /* language encoded */;
    int              bufLen;
} LWValString;

typedef struct st_LWValInt {
    LWType         type;
    int              value;
    int              defVal;
} LWValInt;

typedef struct st_LWValFloat {
    LWType         type;
    double           value;
    double           defVal;
} LWValFloat;

typedef struct st_LWValIVector {
    LWType         type;
    int              val[3];
    int              defVal;
} LWValIVector;

typedef struct st_LWValFVector {
    LWType         type;
    double           val[3];
    double           defVal;
} LWValFVector;

typedef struct st_LWValCustom {
    LWType         type;
    int              val[4];
} LWValCustom;

typedef struct st_LWValPointer {
    LWType         type;
    void           *ptr;
} LWValPointer;

typedef struct st_LWPanStringDesc {
    LWType       type;
    int          width;
} LWPanStringDesc;

typedef struct st_LWPanChoiceDesc {
    LWType       type;
    const char   **items /* language encoded */;
    int          vertical;
} LWPanChoiceDesc;

typedef struct st_LWPanTextDesc {
    LWType       type;
    const char   **text /* language encoded */;
} LWPanTextDesc;

typedef struct st_LWPanRangeDesc {
    LWType          type;
    int                     width;
    int                     min;
    int                     max;
} LWPanRangeDesc;

typedef struct st_LWPanAreaDesc {
    LWType          type;
    int                     width;
    int                     height;
} LWPanAreaDesc;

typedef struct st_LWPanLWItemDesc {
    LWType           type;
    void            *global;
    int              itemType;
    char            **list /* language encoded */;
    int              count;
    int              width;
} LWPanLWItemDesc;

typedef struct st_LWPanPopupDesc {
    LWType           type;
    int              width;
    int              (*countFn)(void *);
    char            * /* language encoded */(*nameFn)(void *, int);
} LWPanPopupDesc;

typedef struct st_LWPanListBoxDesc {
    LWType                   type;
    int                      width,visItems, top;  // width in pixels, height in items, top visible item
    int                      (*countFn)(void *);
    char                    * /* language encoded */(*nameFn)(void *, int);
} LWPanListBoxDesc;

typedef struct st_LWPanMultiListBoxDesc {
    LWType                   type;
    int                      width,visItems, top;  // width in pixels, height in items, top visible item
    int                      (*countFn)(void *);
    char                    * /* language encoded */(*nameFn)(void *, int item, int column);
    int                      (*colWidth)(void *, int i);  // pixel width of column i, up to 8 columns terminate with 0
} LWPanMultiListBoxDesc;

typedef struct st_LWPanXPanDesc {
    LWType                   type;
    int                      width, height;
    void                    *xpan;
} LWPanXPanDesc;

typedef struct st_LWPanTreeDesc {
    LWType                   type;
    int                      width, height;
    int                      (*countFn)(void *data, void *node); /* return number of leafs under given node, NULL node is root */
    void                    *(*leafFn)(void *data, void *node, int i); /* return leaf i of node, NULL node is root */
    char                    * /* language encoded */ (*infoFn)(void *data, void *node, int *nodeFlags); /* return "name" for node, fill 0 flags, or save non-zero flags */
    void                     (*moveFn)(void *data, void *node, void *parentNode, int i); /* NULL prevents moves, or called when user moves node, with new parent, position */
} LWPanTreeDesc;

typedef struct st_LWPanMultiTreeDesc {
    LWType                   type;
    int                      width, height;
    int                      (*countFn)(void *data, void *node); /* return number of leafs under given node, NULL node is root */
    void                    *(*leafFn)(void *data, void *node, int i); /* return leaf i of node, NULL node is root */
    char                    * /* language encoded */ (*infoFn)(void *data, void *node, int column, int *nodeFlags); /* return "name" for node, fill 0 flags, or save non-zero flags */
    void                     (*moveFn)(void *data, void *node, void *parentNode, int i); /* NULL prevents moves, or called when user moves node, with new parent, position */
    int                      (*colWidth)(void *, int i);  // pixel width of column i, up to 8 columns terminate with 0
} LWPanMultiTreeDesc;

typedef enum {   /* Tags to get/set panel attributes */
    PAN_X, PAN_Y, PAN_W, PAN_H, PAN_MOUSEX, PAN_MOUSEY,
    PAN_FLAGS, PAN_USERDRAW, PAN_USERACTIVATE,
    PAN_USEROPEN, PAN_USERCLOSE, PAN_TITLE,PAN_USERDATA, PAN_USERKEYS,
    PAN_LWINSTANCE, PAN_MOUSEBUTTON, PAN_MOUSEMOVE  ,PAN_PANFUN, PAN_VERSION, PAN_RESULT,
    PAN_HOSTDISPLAY, PAN_USERKEYUPS, PAN_QUALIFIERS, PAN_TO_FRONT,
    PAN_MOUSEWHEEL, PAN_USERRESIZE, PAN_MIN_W, PAN_MIN_H, PAN_MAX_W, PAN_MAX_H, PAN_CONFIGTAG
} pTag;

typedef enum { DR_ERASE, DR_RENDER, DR_GHOST, DR_REFRESH } DrMode;

typedef enum {           /* Tags to get/set panel attributes */
    CTL_X, CTL_Y, CTL_W, CTL_H, CTL_VALUE,
    CTL_HOTX, CTL_HOTY, CTL_HOTW, CTL_HOTH, CTL_LABELWIDTH,
    CTL_MOUSEX, CTL_MOUSEY,
    CTL_FLAGS, CTL_USERDRAW, CTL_USEREVENT, CTL_LABEL, CTL_USERDATA,
    CTL_PANEL, CTL_PANFUN,      /* use these 2 to get your panelID and functions! */
    CTL_RANGEMIN, CTL_RANGEMAX, /* get/set slider range params  API 10 and later */
    CTL_ACTIVATE,               /* set only, to actvate/enter string controls   API 10 and later */
    CTL_MOUSEBUTTON,            /* get the mouse button pressed, read only API 20 and later */
    CTL_SELECTEVENT,            /* set a callback for selection events */
    CTL_MOUSECLICKCNT,          /* check if double clicked             */
    CTL_MENUFLAGSFUNC,          /* set a callback for menu item flags  */
    CTL_COLORSPACE              /* color space for color squair.       */
} cTag;

typedef enum {           /* Tags to get mouse buttons for controls */
    CTL_MOUSELEFT=1, CTL_MOUSECENTER, CTL_MOUSERIGHT
} mBut;

typedef enum { MX_HCHOICE, MX_VCHOICE ,MX_POPUP } MxType;

typedef void (*LWPanHook)(LWPanelID, void *);
typedef int  (*LWPanKeyHook)(LWPanelID, void *, LWDualKey);
typedef void (*LWPanMouseHook)(LWPanelID, void *, int, int, int);  /* see input qualifier bits     */
typedef void (*LWPanDrawHook)(LWPanelID, void *, DrMode);
typedef void (*LWPanResizeHook)(LWPanelID, void *, int w, int h);
typedef void (*LWCtlDrawHook)(LWControlID, void *,DrMode);
typedef void (*LWCtlEventHook)(LWControlID, void *);

// LWCtlSelectHook prototype for use with CON_SETSELECT()
//  'data' is the custom data attached to the control via the CON_SETSELECT() macro.
//  'index' is the item number that was clicked on. If index is -1, the list is being cleared.
//  'selecting' is 0 for de-selecting the indexed item, and 1 for selecting it.
typedef int  (*LWCtlSelectHook)(LWControlID ctl, void* data, int index, int selecting);

typedef unsigned int  (*LWCtlMenuFlagsFunc)(LWControlID, void *, int index);


#define MENUFLAG_SELECTED      (1<<0)
#define MENUFLAG_UNSELECTED    (1<<1)
#define MENUFLAG_HIGHLIGHT     (1<<2)
#define MENUFLAG_DISABLED      (1<<3)
#define MENUFLAG_GROUP_MAX     0xF
#define MENUFLAG_GROUP_SHIFT   24
#define MENUFLAG_GROUP(g)      (((g) & MENUFLAG_GROUP_MAX) << MENUFLAG_GROUP_SHIFT)


typedef union un_LWValue {
    LWType         type;
    LWValString      str;
    LWValInt         intv;
    LWValFloat       flt;
    LWValIVector     ivec;
    LWValFVector     fvec;
    LWValPointer     ptr;
    LWValCustom      cust;
} LWValue;

typedef union un_LWPanControlDesc {
    LWType                  type;
    LWPanStringDesc         string;
    LWPanChoiceDesc         choice;
    LWPanTextDesc           text;
    LWPanRangeDesc          range;
    LWPanAreaDesc           area;
    LWPanLWItemDesc         lwitem;
    LWPanPopupDesc          popup;
    LWPanListBoxDesc        listbox;
    LWPanXPanDesc           xpanel;
    LWPanTreeDesc           tree;
    LWPanMultiListBoxDesc   multiList;
    LWPanMultiTreeDesc      multiTree;
} LWPanControlDesc;

typedef LWPanControlDesc                ControlDesc;
typedef struct st_DrawFuncs {
    void    (*drawPixel)(LWPanelID,int,int,int);                    /* color,x,y    */
    void    (*drawRGBPixel)(LWPanelID,int,int,int,int,int);                 /* r,g,b,x,y    */
    void    (*drawLine)(LWPanelID,int,int,int,int,int);             /* color,x,y,x2,y2      */
    void    (*drawBox)(LWPanelID,int,int,int,int,int);              /* color,x,y,w,h        */
    void    (*drawRGBBox)(LWPanelID,int,int,int,int,int,int,int);   /* r,g,b,x,y,w,h        */
    void    (*drawBorder)(LWPanelID,int,int,int,int,int);           /* indent,x,y,w,h       (h==0 -> divider)       */
    int     (*textWidth)(LWPanelID,char *text /* language encoded */);             /* text */
    void    (*drawText)(LWPanelID,char *text /* language encoded */,int,int,int);  /* text,color,x,y */
    const LWDisplayMetrics   *(*dispMetrics)();
} DrawFuncs;

typedef struct st_LWControl {
    void            (*draw)(LWControlID, DrMode);
    void            (*get)(LWControlID, cTag, LWValue *);
    void            (*set)(LWControlID, cTag, LWValue *);
    void            *priv_data;
} LWControl;

typedef struct st_LWPanelFuncs {
    LWPanelID               (*create)(char *title /* language encoded */, void *);
    void                    (*destroy)(LWPanelID);
    int                     (*open)(LWPanelID,int);  /* flags int (see flag bits  PANF_ etc.) */
    int                     (*handle)(LWPanelID, int);       /*  process input manually after non-blocking open, use EVNT_ bits to process async. or synchronously */
    void                    (*draw)(LWPanelID, DrMode);
    void                    (*close)(LWPanelID);
    void                    (*get)(LWPanelID, pTag, void *);
    void                    (*set)(LWPanelID, pTag, void *);
    LWControl              *(*addControl)(LWPanelID pan, char *type /* ascii encoded */, ControlDesc *data, const char *label /* language encoded */);
    LWControl              *(*nextControl)(LWPanelID pan, LWControlID ctl);
    DrawFuncs               *drawFuncs;
    void                    *user_data;      /* do whatcha like */
    GlobalFunc              *globalFun;      /* please set this to your GlobalFunction pointer.. at least for file req. */
    void                    (*destroyControl)(LWPanelID pane, LWControlID ctl);
} LWPanelFuncs;

typedef void    *LWRasterID;

typedef struct st_LWRasterFuncs {
    void            (*drawPixel)( LWRasterID raster, int color, int x, int y);
    void            (*drawRGBPixel)( LWRasterID raster, int r, int g, int b, int x, int y);
    void            (*drawLine)( LWRasterID raster, int color, int x, int y, int x2, int y2);
    void            (*drawBox)( LWRasterID raster, int color, int x, int y, int w, int h);
    void            (*drawRGBBox)(LWRasterID raster, int r, int g, int b, int x, int y, int w, int h);
    void            (*eraseBox)( LWRasterID raster, int x, int y,int w, int h);
    void            (*drawBorder)( LWRasterID raster, int indent, int x, int y, int w, int h); /* (h==0 -> divider) */

    /* text is no longer treated as ANSI */
    /* text may begin with a rich text control code */
    void            (*drawText)( LWRasterID raster, char *text /* utf-8 */, int color, int x, int y);

    LWRasterID      (*create)( int w, int h, int flags);
    void            (*destroy)( LWRasterID raster);
    void            (*blitPanel)( LWRasterID src, int srcx, int srcy, LWPanelID dest, int destx, int desty, int w, int h);
    void            (*blitRaster)( LWRasterID src, int srcx, int srcy, LWRasterID dest, int destx, int desty, int w, int h);

    /* obtain text width in pixels */
    int             (*textWidth)( LWRasterID raster, char *text /* utf-8 */);

} LWRasterFuncs;

typedef struct st_ContextMenuFuncs {
    LWContextMenuID (*cmenuCreate)(LWPanPopupDesc *desc, void *userdata);
    int             (*cmenuDeploy)(LWContextMenuID menu, LWPanelID pan, int item); // item==-1 for no choice initially and/or on return
    void            (*cmenuDestroy)(LWContextMenuID menu);
} ContextMenuFuncs;


#define PANELMENUFLAG_SELECTED   (MENUFLAG_SELECTED)
#define PANELMENUFLAG_UNSELECTED (MENUFLAG_UNSELECTED)
#define PANELMENUFLAG_HIGHLIGHT  (MENUFLAG_HIGHLIGHT)
#define PANELMENUFLAG_DISABLED   (MENUFLAG_DISABLED)

typedef struct st_LWPanMenuDesc {
    int            (*countFn)(void *);
        // returns number of items available
    char           * /* language encoded */(*nameFn)(void *, int);
        // returns the display string for a particular item
    void           *(*submenuFn)(void *, int); // added in version 2
        // returns user data for the submenu (use it to identify which submenu being processed)
    int             (*flagsFn)(void *, int); // added in version 2
        // returns (PANEL)MENUFLAG_ flags for a particular item
        // Setting both the selected and unselected bits imply a mixed selection state
    int            *(*separatorFn)(void*); // added in version 2
        // returns an array of integers, where 0 is a menu item, and !0 is a menu item with a trailing separator
} LWPanMenuDesc;

typedef struct st_LWPanMenuFuncs {
    LWPanelMenuID   (*menuCreate)(LWPanMenuDesc *desc, void *userdata);
    int             (*menuDeploy)(LWPanelMenuID menu, LWPanelID pan, int item); // item==-1 for no choice initially and/or on return
    void            (*menuDestroy)(LWPanelMenuID menu);
    void           *(*menuSelection)(LWPanelMenuID menu); // user data of selection when deploy returns >= 0.
} LWPanelMenuFuncs;

#ifndef RGB_
#define RGB_(r,g,b)     ((unsigned int)(((r) << 16) | ((b) << 8) | (g) | 0x1000000))
#endif
#define PANF_BLOCKING           (1 << 0) /* open waits for close, or returns immediately */
#define PANF_CANCEL             (1 << 1) /* Continue or Continue/Cancel*/
#define PANF_FRAME              (1 << 2) /* Windowing System Title bar */
#define PANF_MOUSETRAP          (1 << 3) /* Please track mouse input and call mouse callbacks */
#define PANF_PASSALLKEYS        (1 << 4) /* Pass Enter/Esc keys to panel, instead of closing with OK/Cancel */
#define PANF_ABORT              (1 << 5) /* Abort instead of Continue, usw w/ PANF_CANCEL!! */
#define PANF_NOBUTT             (1 << 6) /* No Continue or Continue/Cancel*/
#define PANF_RESIZE             (1 << 7) /* Allow resizing of window. Clients should reposition/recreate controls */
#define PANF_MOUSETRACK         (1 << 8) /* Allow passive mouse motion to be included in mouse move callbacks */

#define CTLF_DISABLE            0x100    // Read-Only ,i.e. check if button has been drawn w/DR_GHOST , or erase
#define CTLF_INVISIBLE          0x200 // Read-Only        ... erased
#define CTLF_GHOST              CTLF_DISABLE

#define LWI_IMAGE               LWI_BONE+1
#define LWI_ANY                 LWI_IMAGE+1
#define NODE_FLAG_EXPND         1       /* TreeControl item expanded state*/
#define NODE_FLAG_WRITE         1024    /* TreeControl flag write-mode, allows clearing of other bits*/

/* w/EVNT_BLOCKING handle() won't return 'til panel is closed , w/out EVNT_BLOCKING, EVNT_ALL is irrelevant */
#define EVNT_BLOCKING           1 /* 1  << 0  i.e. bit flags  handle() method waits for input, or gives immediate return */
#define EVNT_ALL                2 /* 1 << 1  if set, handle() method will poll entire queue of events,  */

#define TRUECOLOR(dm)   (dm->depth ? 1:0)
#define PALETTESIZE(dm) (dm->depth ? 1<<24:1<<dm->depth)
#define PAN_CREATE(pfunc,tit) ((*pfunc->create)(tit,NULL))
#define PAN_POST(pfunc,pan) ((*pfunc->open)(pan,PANF_BLOCKING|PANF_CANCEL|PANF_FRAME))
#define PAN_KILL(pfunc,pan) ((*pfunc->destroy)(pan))

#define HCHOICE_CTL(pfunc,pan,tit,ls) (desc.type=LWT_CHOICE,desc.choice.vertical=MX_HCHOICE,\
              desc.choice.items=ls,\
              (*pfunc->addControl) (pan, "ChoiceControl", &desc, tit))
#define VCHOICE_CTL(pfunc,pan,tit,ls) (desc.type=LWT_CHOICE,desc.choice.vertical=MX_VCHOICE,\
              desc.choice.items=ls,\
              (*pfunc->addControl) (pan, "ChoiceControl", &desc, tit))
#define POPUP_CTL(pfunc,pan,tit,ls) (desc.type=LWT_CHOICE,desc.choice.vertical=0, desc.choice.items=ls,\
              (*pfunc->addControl) (pan, "PopupControl", &desc, tit))
#define WPOPUP_CTL(pfunc,pan,tit,ls,w) (desc.type=LWT_CHOICE,desc.choice.vertical=w, desc.choice.items=ls,\
              (*pfunc->addControl) (pan, "PopupControl", &desc, tit))
#define FLOAT_CTL(pfunc,pan,tit)  (desc.type=LWT_FLOAT,\
                                    (*pfunc->addControl) (pan, "FloatControl", &desc, tit))
/* RO means 'read-only', uneditable text gadget */
#define FLOATRO_CTL(pfunc,pan,tit)  (desc.type=LWT_FLOAT,\
                                    (*pfunc->addControl) (pan, "FloatInfoControl", &desc, tit))
#define FVEC_CTL(pfunc,pan,tit)         (desc.type=LWT_VFLOAT,\
                                    (*pfunc->addControl) (pan, "FVecControl", &desc, tit))
#define FVECRO_CTL(pfunc,pan,tit)       (desc.type=LWT_VFLOAT,\
                                    (*pfunc->addControl) (pan, "FVecInfoControl", &desc, tit))
#define DVEC_CTL(pfunc,pan,tit)         (desc.type=LWT_VDIST,\
                                    (*pfunc->addControl) (pan, "FVecControl", &desc, tit))
#define DIST_CTL(pfunc,pan,tit)   (desc.type=LWT_DISTANCE,\
                                    (*pfunc->addControl) (pan, "FloatControl", &desc, tit))

/* a popup list of LW items (i.e. lights, objects, bones, ..search 'LWI_' ) */
#define ITEM_CTL(pfunc,pan,tit,glb,typ)   (desc.type=LWT_LWITEM, desc.lwitem.global=glb, desc.lwitem.itemType=typ,\
                                    desc.lwitem.list=NULL, desc.lwitem.count=0,desc.lwitem.width=0, \
                                    (*pfunc->addControl) (pan, "LWListControl", &desc, tit) )
/* a popup list of LW items (i.e. lights, objects, bones, ..search 'LWI_' ) */
#define WITEM_CTL(pfunc,pan,tit,glb,typ,w)   (desc.type=LWT_LWITEM,desc.lwitem.global=glb, desc.lwitem.itemType=typ,\
                                    desc.lwitem.list=NULL,desc.lwitem.count=0,desc.lwitem.width=w,  \
                                    (*pfunc->addControl) (pan, "LWListControl", &desc, tit) )
#define PIKITEM_CTL(pfunc,pan,tit,glb,typ,w)   (desc.type=LWT_LWITEM,desc.lwitem.global=glb, desc.lwitem.itemType=typ,\
                                    desc.lwitem.list=NULL,desc.lwitem.count=0,desc.lwitem.width=w,  \
                                    (*pfunc->addControl) (pan, "LWPickListControl", &desc, tit) )
/* a 'do something now' button */
#define BUTTON_CTL(pfunc,pan,tit)   (desc.type=LWT_INTEGER,\
                                    (*pfunc->addControl) (pan, "ButtonControl", &desc, tit))
#define WBUTTON_CTL(pfunc,pan,tit,w)   (desc.type=LWT_AREA,desc.area.width=w,desc.area.height=0,\
                                    (*pfunc->addControl) (pan, "ButtonControl", &desc, tit))
#define LINK_CTL(pfunc,pan,tit)     (desc.type=LWT_INTEGER,\
                                    (*pfunc->addControl) (pan, "LinkControl", &desc, tit))
/* a checkbox */
#define BOOL_CTL(pfunc,pan,tit)   (desc.type=LWT_BOOLEAN,\
                                    (*pfunc->addControl) (pan, "BoolControl", &desc, tit))
/* a selectable text button */
#define BOOLBUTTON_CTL(pfunc,pan,tit)   (desc.type=LWT_BOOLEAN,\
                                    (*pfunc->addControl) (pan, "BoolButtonControl", &desc, tit))
#define WBOOLBUTTON_CTL(pfunc,pan,tit,w)   (desc.type=LWT_AREA,desc.area.width=w,desc.area.height=0,\
                                    (*pfunc->addControl) (pan, "BoolButtonControl", &desc, tit))
#define INT_CTL(pfunc,pan,tit)    (desc.type=LWT_INTEGER,\
                                    (*pfunc->addControl) (pan, "NumControl", &desc, tit))
#define INTRO_CTL(pfunc,pan,tit)    (desc.type=LWT_INTEGER,\
                                    (*pfunc->addControl) (pan, "NumInfoControl", &desc, tit))

#define SLIDER_CTL(pfunc,pan,tit,w,mn,mx)    (desc.type=LWT_RANGE,desc.range.width=w,\
                                    desc.range.min=mn,desc.range.max=mx,\
                                    (*pfunc->addControl)(pan, "SliderControl", &desc, tit))

#define VSLIDER_CTL(pfunc,pan,tit,w,mn,mx)    (desc.type=LWT_RANGE,desc.range.width=w,\
                                    desc.range.min=mn,desc.range.max=mx,\
                                    (*pfunc->addControl)(pan, "VSlideControl", &desc, tit))

#define HSLIDER_CTL(pfunc,pan,tit,w,mn,mx)    (desc.type=LWT_RANGE,desc.range.width=w,\
                                    desc.range.min=mn,desc.range.max=mx,\
                                    (*pfunc->addControl)(pan, "HSlideControl", &desc, tit))

#define UNSLIDER_CTL(pfunc,pan,tit,w,mn,mx)    (desc.type=LWT_RANGE,desc.range.width=w,\
                                    desc.range.min=mn,desc.range.max=mx,\
                                    (*pfunc->addControl)(pan, "UnboundSlideControl", &desc, tit))

#define MINISLIDER_CTL(pfunc,pan,tit,w,mn,mx)    (desc.type=LWT_RANGE,desc.range.width=w,\
                                    desc.range.min=mn,desc.range.max=mx,\
                                    (*pfunc->addControl)(pan, "MiniSliderControl", &desc, tit))

#define IVEC_CTL(pfunc,pan,tit)    (desc.type=LWT_VINT,\
                                    (*pfunc->addControl) (pan, "VecControl", &desc, tit))
#define IVECRO_CTL(pfunc,pan,tit)    (desc.type=LWT_VINT,\
                                    (*pfunc->addControl) (pan, "VecInfoControl", &desc, tit))
#define RGB_CTL(pfunc,pan,tit)    (desc.type=LWT_VINT,\
                                    (*pfunc->addControl) (pan, "RGBControl", &desc, tit))
#define MINIRGB_CTL(pfunc,pan,tit)    (desc.type=LWT_VINT,\
                                    (*pfunc->addControl) (pan, "MiniRGBControl", &desc, tit))
#define MINIHSV_CTL(pfunc,pan,tit)    (desc.type=LWT_VINT,\
                                    (*pfunc->addControl) (pan, "MiniHSVControl", &desc, tit))
#define RGBVEC_CTL(pfunc,pan,tit)    (desc.type=LWT_VINT,\
                                    (*pfunc->addControl) (pan, "RGBVecControl", &desc, tit))
#define STR_CTL(pfunc,pan,tit,w)        (desc.type=LWT_STRING,desc.string.width=w,\
              (*pfunc->addControl) (pan, "EditControl", &desc, tit))
#define STRRO_CTL(pfunc,pan,tit,w)      (desc.type=LWT_STRING,desc.string.width=w,\
              (*pfunc->addControl) (pan, "InfoControl", &desc, tit))
/* file name string + file-requester ! */
#define FILE_CTL(pfunc,pan,tit,w)       (desc.type=LWT_STRING,desc.string.width=w,\
              (*pfunc->addControl) (pan, "FileControl", &desc, tit))
/* width in pixels not char.s here */
#define FILEBUTTON_CTL(pfunc,pan,tit,w)         (desc.type=LWT_STRING,desc.string.width=w,\
              (*pfunc->addControl) (pan, "FileButtonControl", &desc, tit))
#define LOAD_CTL(pfunc,pan,tit,w)       (desc.type=LWT_STRING,desc.string.width=w,\
              (*pfunc->addControl) (pan, "LoadControl", &desc, tit))
#define LOADBUTTON_CTL(pfunc,pan,tit,w)         (desc.type=LWT_STRING,desc.string.width=w,\
              (*pfunc->addControl) (pan, "LoadButtonControl", &desc, tit))
#define SAVE_CTL(pfunc,pan,tit,w)       (desc.type=LWT_STRING,desc.string.width=w,\
              (*pfunc->addControl) (pan, "SaveControl", &desc, tit))
#define SAVEBUTTON_CTL(pfunc,pan,tit,w)         (desc.type=LWT_STRING,desc.string.width=w,\
              (*pfunc->addControl) (pan, "SaveButtonControl", &desc, tit))
#define DIR_CTL(pfunc,pan,tit,w)       (desc.type=LWT_STRING,desc.string.width=w,\
              (*pfunc->addControl) (pan, "DirControl", &desc, tit))
#define DIRBUTTON_CTL(pfunc,pan,tit,w)         (desc.type=LWT_STRING,desc.string.width=w,\
              (*pfunc->addControl) (pan, "DirButtonControl", &desc, tit))

#define TEXT_CTL(pfunc,pan,tit,sts)     (desc.type=LWT_TEXT,desc.text.text=sts,\
                                    (*pfunc->addControl) (pan, "TextControl", &desc, tit))
#define AREA_CTL(pfunc,pan,tit,w,h)    (desc.type=LWT_AREA,desc.area.width=w,desc.area.height=h,\
                                    (*pfunc->addControl)(pan, "AreaControl", &desc, tit))
#define PALETTE_CTL(pfunc,pan,tit,w,h)    (desc.type=LWT_RANGE,desc.area.width=w,desc.area.height=h,\
                                    (*pfunc->addControl)(pan, "PaletteControl", &desc, tit))
#define OPENGL_CTL(pfunc,pan,tit,w,h)    (desc.type=LWT_AREA,desc.area.width=w,desc.area.height=h,\
                                    (*pfunc->addControl)(pan, "OpenGLControl", &desc, tit))
#define CUSTPOPUP_CTL(pfunc,pan,tit,w,name,num)    (desc.type=LWT_POPUP,desc.popup.width=w,desc.popup.nameFn=name,\
                                    desc.popup.countFn=num,\
                                    (*pfunc->addControl)(pan, "CustomPopupControl", &desc, tit))

#define CUSTPOPDOWN_CTL(pfunc,pan,tit,w,name,num)    (desc.type=LWT_POPUP,desc.popup.width=w,desc.popup.nameFn=name,\
                                    desc.popup.countFn=num,\
                                    (*pfunc->addControl)(pan, "CustomPopdownControl", &desc, tit))

#define CANVAS_CTL(pfunc,pan,tit,w,h)    (desc.type=LWT_AREA,desc.area.width=w,desc.area.height=h,\
                                    (*pfunc->addControl)(pan, "CanvasControl", &desc, tit))

#define POPDOWN_CTL(pfunc,pan,tit,ls)     (desc.type=LWT_CHOICE,desc.choice.vertical=MX_POPUP,\
              desc.choice.items=ls,\
              (*pfunc->addControl) (pan, "PopDownControl", &desc, tit))
/* get intvector w/(dx,dy,count)  */
#define DRAGBUT_CTL(pfunc,pan,tit,w,h)    (desc.type=LWT_AREA,desc.area.width=w,desc.area.height=h,(*pfunc->addControl)(pan, "DragButtonControl", &desc, tit))
#define VDRAGBUT_CTL(pfunc,pan,tit)    (desc.type=LWT_AREA,desc.area.width=0,desc.area.height=-1,(*pfunc->addControl)(pan, "DragButtonControl", &desc, tit))
#define HDRAGBUT_CTL(pfunc,pan,tit)    (desc.type=LWT_AREA,desc.area.width=-1,desc.area.height=0,(*pfunc->addControl)(pan, "DragButtonControl", &desc, tit))
#define DRAGAREA_CTL(pfunc,pan,tit,w,h)    (desc.type=LWT_AREA,desc.area.width=w,desc.area.height=h,(*pfunc->addControl)(pan, "DragAreaControl", &desc, tit))
#define LISTBOX_CTL(pfunc,pan,tit,w,vis,name,num)    (desc.type=LWT_POPUP,desc.listbox.width=w,desc.listbox.nameFn=name,\
                                    desc.listbox.countFn=num, desc.listbox.visItems=vis,\
                                    (*pfunc->addControl)(pan, "ListBoxControl", &desc, tit))

#define MULTILIST_CTL(pfunc,pan,tit,w,vis,name,num,colw)    (desc.type=LWT_MLIST,desc.multiList.width=w,desc.multiList.nameFn=name,\
                                    desc.multiList.countFn=num,desc.multiList.visItems=vis,desc.multiList.colWidth=colw,\
                                    (*pfunc->addControl)(pan, "MultiListBoxControl", &desc, tit))

#define PERCENT_CTL(pfunc,pan,tit)    (desc.type=LWT_RANGE,\
                                    desc.range.min=0,desc.range.max=100,\
                                    (*pfunc->addControl)(pan, "PercentControl", &desc, tit))

#define ANGLE_CTL(pfunc,pan,tit)    (desc.type=LWT_RANGE,\
                                    desc.range.min=0,desc.range.max=360,\
                                    (*pfunc->addControl)(pan, "AngleControl", &desc, tit))

#define DISTSLIDER_CTL(pfunc,pan,tit)    (desc.type=LWT_RANGE,\
                                    desc.range.min=0,desc.range.max=100,\
                                    (*pfunc->addControl)(pan, "MiniDistanceControl", &desc, tit))


#define BORDER_CTL(pfunc,pan,tit,w,h)    (desc.type=LWT_AREA,desc.area.width=w,desc.area.height=h,\
                                    (*pfunc->addControl)(pan, "BorderControl", &desc, tit))

#define TABCHOICE_CTL(pfunc,pan,tit,ls) (desc.type=LWT_CHOICE,desc.choice.vertical=MX_HCHOICE,\
              desc.choice.items=ls,\
              (*pfunc->addControl) (pan, "TabChoiceControl", &desc, tit))

#define CHANNEL_CTL(pfunc,pan,tit,w,h) (desc.type=LWT_AREA,desc.area.width=w,desc.area.height=h,\
              (*pfunc->addControl) (pan, "ChannelControl", &desc, tit))

#define XPANEL_CTL(pfunc,pan,tit,xp) (desc.type=LWT_XPANEL, desc.xpanel.xpan=xp,\
              (*pfunc->addControl) (pan, "XPanelControl", &desc, tit))

#define TREE_CTL(pfunc,pan,tit,w,h,info,num,node)    (desc.type=LWT_TREE,desc.tree.width=w,desc.tree.height=h,desc.tree.infoFn=info,\
                                    desc.tree.countFn=num,desc.tree.leafFn=node,desc.tree.moveFn=NULL,\
                                    (*pfunc->addControl)(pan, "TreeControl", &desc, tit))

#define MULTITREE_CTL(pfunc,pan,tit,w,h,info,num,node,colw) (desc.type=LWT_TREE,desc.multiTree.width=w,desc.multiTree.height=h,desc.multiTree.infoFn=info,\
                                    desc.multiTree.countFn=num,desc.multiTree.leafFn=node,desc.multiTree.colWidth=colw,desc.multiTree.moveFn=NULL,\
                                    (*pfunc->addControl)(pan, "MultiTreeControl", &desc, tit))

/*
  The folowing macros require a few global LWValue variables initialized as
  appropriate types.
- - - - - - CUT HERE - - - - - */

/* required by macros in lwpanel.h */
/*static LWValue ival={LWT_INTEGER},ivecval={LWT_VINT},
  fval={LWT_FLOAT},fvecval={LWT_VFLOAT},sval={LWT_STRING},pval={LWT_POINTER}; */

/* - - - - - - CUT HERE - - - - - */



#define SET_STR(ctl,s,l) (sval.str.buf=s,sval.str.bufLen=l,(*ctl->set)(ctl, CTL_VALUE, &sval))
#define SET_INT(ctl,n) (ival.intv.value=n,(*ctl->set)(ctl, CTL_VALUE, &ival))
#define SET_ADDR(ctl,n) (pval.ptr.ptr=n,(*ctl->set)(ctl, CTL_VALUE, &pval))
#define SET_FLOAT(ctl,f) (fval.flt.value=f,(*ctl->set)(ctl, CTL_VALUE, &fval))
#define SET_FVEC(ctl,x,y,z) (fvecval.fvec.val[0]=x,fvecval.fvec.val[1]=y,fvecval.fvec.val[2]=z,\
    (*ctl->set)(ctl, CTL_VALUE, &fvecval))
#define SET_IVEC(ctl,x,y,z) (ivecval.ivec.val[0]=x,ivecval.ivec.val[1]=y,ivecval.ivec.val[2]=z,\
    (*ctl->set)(ctl, CTL_VALUE, &ivecval))
#define SETV_FVEC(ctl,v) (fvecval.fvec.val[0]=v[0],fvecval.fvec.val[1]=v[1],fvecval.fvec.val[2]=v[2],\
    (*ctl->set)(ctl, CTL_VALUE, &fvecval))
#define SETV_IVEC(ctl,v) (ivecval.ivec.val[0]=v[0],ivecval.ivec.val[1]=v[1],ivecval.ivec.val[2]=v[2],\
    (*ctl->set)(ctl, CTL_VALUE, &ivecval))

#define GET_STR(ctl,s,l) (sval.str.buf=s,sval.str.bufLen=l,(*ctl->get)(ctl, CTL_VALUE, &sval))
#define GET_INT(ctl,n) ((*ctl->get)(ctl, CTL_VALUE, &ival),n=ival.intv.value)
#define GET_ADDR(ctl,n) ((*ctl->get)(ctl, CTL_VALUE, &pval),n=pval.ptr.ptr)
#define GET_FLOAT(ctl,f) ((*ctl->get)(ctl, CTL_VALUE, &fval),f=fval.flt.value)
#define GET_FVEC(ctl,x,y,z) ((*ctl->get)(ctl, CTL_VALUE, &fvecval),x=fvecval.fvec.val[0],y=fvecval.fvec.val[1],z=fvecval.fvec.val[2])
#define GETV_FVEC(ctl,v) ((*ctl->get)(ctl, CTL_VALUE, &fvecval),v[0]=fvecval.fvec.val[0],v[1]=fvecval.fvec.val[1],v[2]=fvecval.fvec.val[2])
#define GET_IVEC(ctl,x,y,z) ((*ctl->get)(ctl, CTL_VALUE, &ivecval),x=ivecval.ivec.val[0],y=ivecval.ivec.val[1],z=ivecval.ivec.val[2])
#define GETV_IVEC(ctl,v) ((*ctl->get)(ctl, CTL_VALUE, &ivecval),v[0]=ivecval.ivec.val[0],v[1]=ivecval.ivec.val[1],v[2]=ivecval.ivec.val[2])

#define CON_X(ctl)      ((*ctl->get)(ctl, CTL_X, &ival),ival.intv.value)
#define CON_Y(ctl)      ((*ctl->get)(ctl, CTL_Y, &ival),ival.intv.value)
#define CON_W(ctl)      ((*ctl->get)(ctl, CTL_W, &ival),ival.intv.value)
#define CON_H(ctl)      ((*ctl->get)(ctl, CTL_H, &ival),ival.intv.value)
#define CON_LW(ctl)     ((*ctl->get)(ctl, CTL_LABELWIDTH, &ival),ival.intv.value)
#define CON_PAN(ctl)    ((*ctl->get)(ctl, CTL_PANEL, &sval),sval.ptr.ptr)
#define CON_PANFUN(ctl) ((*ctl->get)(ctl, CTL_PANFUN, &sval),sval.ptr.ptr)
#define CON_HOTX(ctl)   ((*ctl->get)(ctl, CTL_HOTX, &ival),ival.intv.value)
#define CON_HOTY(ctl)   ((*ctl->get)(ctl, CTL_HOTY, &ival),ival.intv.value)
#define CON_HOTW(ctl)   ((*ctl->get)(ctl, CTL_HOTW, &ival),ival.intv.value)
#define CON_HOTH(ctl)   ((*ctl->get)(ctl, CTL_HOTH, &ival),ival.intv.value)
#define CON_MOUSEX(ctl) ((*ctl->get)(ctl, CTL_MOUSEX, &ival),ival.intv.value)
#define CON_MOUSEY(ctl) ((*ctl->get)(ctl, CTL_MOUSEY, &ival),ival.intv.value)
#define CON_MOUSECLICKCNT(ctl) ((*ctl->get)(ctl, CTL_MOUSECLICKCNT, &ival),ival.intv.value)
#define MOVE_CON(ctl,x,y)       ( (ival.intv.value=x,(*ctl->set)(ctl, CTL_X, &ival)),(ival.intv.value=y,(*ctl->set)(ctl, CTL_Y, &ival)) )
#define CON_SETEVENT(ctl,f,d) ( (ival.ptr.ptr=(void*)d,(*ctl->set)(ctl,CTL_USERDATA,&ival)), (ival.ptr.ptr=(void*)f,(*ctl->set)(ctl,CTL_USEREVENT,&ival)) )
#define CON_SETDRAW(ctl,f,d)   ( (ival.ptr.ptr=(void*)d,(*ctl->set)(ctl,CTL_USERDATA,&ival)), (ival.ptr.ptr=(void*)f,(*ctl->set)(ctl,CTL_USERDRAW,&ival)) )
#define CON_SETSELECT(ctl,f,d) ( (ival.ptr.ptr=(void*)d,(*ctl->set)(ctl,CTL_USERDATA,&ival)), (ival.ptr.ptr=(void*)f,(*ctl->set)(ctl,CTL_SELECTEVENT,&ival)) )
#define ERASE_CON(ctl)  ((*ctl->draw)(ctl,DR_ERASE))
#define REDRAW_CON(ctl) ((*ctl->draw)(ctl,DR_REFRESH))
#define GHOST_CON(ctl)  ((*ctl->draw)(ctl,DR_GHOST))
#define RENDER_CON(ctl) ((*ctl->draw)(ctl,DR_RENDER))
#define UNGHOST_CON(ctl) ((*ctl->draw)(ctl,DR_RENDER))
#define ACTIVATE_CON(ctl)       (ival.intv.value=0, (*ctl->set)(ctl, CTL_ACTIVATE, &ival))
#define CON_SETMENUFLAGSFUNC(ctl,f)  (ival.ptr.ptr=(void*)f,(*ctl->set)(ctl,CTL_MENUFLAGSFUNC,&ival))

#define PAN_GETX(pfunc,pan) ((*pfunc->get)(pan,PAN_X,(void *)&(ival.intv.value)),ival.intv.value)
#define PAN_GETY(pfunc,pan) ((*pfunc->get)(pan,PAN_Y,(void *)&(ival.intv.value)),ival.intv.value)
#define PAN_GETW(pfunc,pan) ((*pfunc->get)(pan,PAN_W,(void *)&(ival.intv.value)),ival.intv.value)
#define PAN_GETH(pfunc,pan) ((*pfunc->get)(pan,PAN_H,(void *)&(ival.intv.value)),ival.intv.value)
#define PAN_GETMINW(pfunc,pan) ((*pfunc->get)(pan,PAN_MIN_W,(void *)&(ival.intv.value)),ival.intv.value)
#define PAN_GETMINH(pfunc,pan) ((*pfunc->get)(pan,PAN_MIN_H,(void *)&(ival.intv.value)),ival.intv.value)
#define PAN_GETMAXW(pfunc,pan) ((*pfunc->get)(pan,PAN_MAX_W,(void *)&(ival.intv.value)),ival.intv.value)
#define PAN_GETMAXH(pfunc,pan) ((*pfunc->get)(pan,PAN_MAX_H,(void *)&(ival.intv.value)),ival.intv.value)
#define PAN_SETH(pfunc,pan,h)   ( ival.intv.value=h, (*pfunc->set)(pan, PAN_H, (void *)&ival.intv.value) )
#define PAN_SETW(pfunc,pan,w)   ( ival.intv.value=w,(*pfunc->set)(pan, PAN_W, (void *)&ival.intv.value) )
#define PAN_SETMINH(pfunc,pan,h)   ( ival.intv.value=h, (*pfunc->set)(pan, PAN_MIN_H, (void *)&ival.intv.value) )
#define PAN_SETMINW(pfunc,pan,w)   ( ival.intv.value=w,(*pfunc->set)(pan, PAN_MIN_W, (void *)&ival.intv.value) )
#define PAN_SETMAXH(pfunc,pan,h)   ( ival.intv.value=h, (*pfunc->set)(pan, PAN_MAX_H, (void *)&ival.intv.value) )
#define PAN_SETMAXW(pfunc,pan,w)   ( ival.intv.value=w,(*pfunc->set)(pan, PAN_MAX_W, (void *)&ival.intv.value) )
#define MOVE_PAN(pfunc,pan,x,y) ( ival.intv.value=x,(*pfunc->set)(pan, PAN_X, (void *)&ival.intv.value),ival.intv.value=y,(*pfunc->set)(pan, PAN_Y, (void *)&ival.intv.value) )
#define PAN_SETDATA(pfunc,pan,d)  ( (*pfunc->set)(pan,PAN_USERDATA,(void *)d) )
#define PAN_SETDRAW(pfunc,pan,d)  ( (*pfunc->set)(pan,PAN_USERDRAW,(void *)d) )
#define PAN_SETKEYS(pfunc,pan,d)  ( (*pfunc->set)(pan,PAN_USERKEYS,(void *)d) )
#define PAN_GETVERSION(pfunc,pan) ((*pfunc->get)(pan,PAN_VERSION,(void *)&(ival.intv.value)),ival.intv.value)
#define PAN_SETCONFIGTAG(pfunc,pan,tag)   ( ival.intv.value=tag, (*pfunc->set)(pan, PAN_CONFIGTAG, (void *)&ival.intv.value) )

        /*      input qualifiers */
#define IQ_CTRL         1
#define IQ_SHIFT        2
#define IQ_ALT          4
#define IQ_CONSTRAIN    8
#define IQ_ADJUST       16
#define MOUSE_LEFT      32
#define MOUSE_MID       64
#define MOUSE_RIGHT     96
#define MOUSE_DOWN              128
#define RCLICK(code)            ( (code&MOUSE_LEFT) && (code&MOUSE_MID) ? 1:0)
#define LCLICK(code)            ( (code&MOUSE_LEFT) && (!(code&MOUSE_MID)) ? 1:0)
#define MCLICK(code)            ( (!(code&MOUSE_LEFT)) && (code&MOUSE_MID) ? 1:0)

#define PALETTESIZE(dm) (dm->depth ? 1<<24:1<<dm->depth)

        /* Input key codes */

#define LWDK_CHAR(c)      ((LWDualKey) (c))
#define LWDK_F1           ((LWDualKey) 0x1001)
#define LWDK_F2           ((LWDualKey) 0x1002)
#define LWDK_F3           ((LWDualKey) 0x1003)
#define LWDK_F4           ((LWDualKey) 0x1004)
#define LWDK_F5           ((LWDualKey) 0x1005)
#define LWDK_F6           ((LWDualKey) 0x1006)
#define LWDK_F7           ((LWDualKey) 0x1007)
#define LWDK_F8           ((LWDualKey) 0x1008)
#define LWDK_F9           ((LWDualKey) 0x1009)
#define LWDK_F10          ((LWDualKey) 0x100A)
#define LWDK_F11          ((LWDualKey) 0x100B)
#define LWDK_F12          ((LWDualKey) 0x100C)

#define LWDK_SC_LEFT      ((LWDualKey) 0x1010)
#define LWDK_SC_RIGHT     ((LWDualKey) 0x1011)
#define LWDK_SC_UP        ((LWDualKey) 0x1012)
#define LWDK_SC_DOWN      ((LWDualKey) 0x1013)

#define LWDK_DELETE       ((LWDualKey) 0x1020)
#define LWDK_HELP         ((LWDualKey) 0x1021)
#define LWDK_RETURN       ((LWDualKey) 0x1022)

#define LWDK_HOME      ((LWDualKey) 0x108F)
#define LWDK_END       ((LWDualKey) 0x1090)
#define LWDK_PAGEUP    ((LWDualKey) 0x1091)
#define LWDK_PAGEDOWN  ((LWDualKey) 0x1092)

#define LWDK_TOP_0        ((LWDualKey) 0x1030)
#define LWDK_TOP_1        ((LWDualKey) 0x1031)
#define LWDK_TOP_2        ((LWDualKey) 0x1032)
#define LWDK_TOP_3        ((LWDualKey) 0x1033)
#define LWDK_TOP_4        ((LWDualKey) 0x1034)
#define LWDK_TOP_5        ((LWDualKey) 0x1035)
#define LWDK_TOP_6        ((LWDualKey) 0x1036)
#define LWDK_TOP_7        ((LWDualKey) 0x1037)
#define LWDK_TOP_8        ((LWDualKey) 0x1038)
#define LWDK_TOP_9        ((LWDualKey) 0x1039)

#define LWDK_PAD_0        ((LWDualKey) 0x1040)
#define LWDK_PAD_1        ((LWDualKey) 0x1041)
#define LWDK_PAD_2        ((LWDualKey) 0x1042)
#define LWDK_PAD_3        ((LWDualKey) 0x1043)
#define LWDK_PAD_4        ((LWDualKey) 0x1044)
#define LWDK_PAD_5        ((LWDualKey) 0x1045)
#define LWDK_PAD_6        ((LWDualKey) 0x1046)
#define LWDK_PAD_7        ((LWDualKey) 0x1047)
#define LWDK_PAD_8        ((LWDualKey) 0x1048)
#define LWDK_PAD_9        ((LWDualKey) 0x1049)

#define LWDK_PAD_OPAREN   ((LWDualKey) 0x1050)
#define LWDK_PAD_CPAREN   ((LWDualKey) 0x1051)
#define LWDK_PAD_SLASH    ((LWDualKey) 0x1052)
#define LWDK_PAD_STAR     ((LWDualKey) 0x1053)
#define LWDK_PAD_DASH     ((LWDualKey) 0x1054)
#define LWDK_PAD_PLUS     ((LWDualKey) 0x1055)
#define LWDK_PAD_DECIMAL  ((LWDualKey) 0x1056)
#define LWDK_PAD_ENTER    ((LWDualKey) 0x1057)

#define LWDK_ALT          ((LWDualKey) 0x1060)
#define LWDK_SHIFT        ((LWDualKey) 0x1061)
#define LWDK_CTRL         ((LWDualKey) 0x1062)


        /* Color Definitions */
#define SYSTEM_Ic(i)    (4136 + i)
#define COLOR_BG        LWP_BG
#define COLOR_DK_YELLOW LWP_EDIT_IMG
#define COLOR_DK_GREY   LWP_SHADOW
#define COLOR_LT_YELLOW LWP_EDIT_SEL
#define COLOR_MD_GREY   LWP_B_FACE
#define COLOR_LT_GREY   LWP_GLINT
#define COLOR_BLACK     LWP_NORMAL
#define COLOR_WHITE     LWP_HEADLINE

#define LWP_NONE         SYSTEM_Ic(0)
#define LWP_INVERT       SYSTEM_Ic(1)
#define LWP_BG           SYSTEM_Ic(2)
#define LWP_NORMAL       SYSTEM_Ic(3)
#define LWP_HEADLINE     SYSTEM_Ic(4)
#define LWP_INFO_BG      SYSTEM_Ic(5)
#define LWP_INFO_IMG     SYSTEM_Ic(6)
#define LWP_INFO_DIS     SYSTEM_Ic(7)
#define LWP_EDIT_BG      SYSTEM_Ic(8)
#define LWP_EDIT_IMG     SYSTEM_Ic(9)
#define LWP_EDIT_SEL     SYSTEM_Ic(10)
#define LWP_EDIT_DIS     SYSTEM_Ic(11)

#define LWP_BLACK   SYSTEM_Ic(12)
#define LWP_SHADOW  SYSTEM_Ic(13)
#define LWP_LOWERED SYSTEM_Ic(14)
#define LWP_RAISED  SYSTEM_Ic(15)
#define LWP_HILIGHT SYSTEM_Ic(16)

#define LWP_0_STAT  SYSTEM_Ic(17)
#define LWP_1_STAT  SYSTEM_Ic(18)
#define LWP_2_STAT  SYSTEM_Ic(19)
#define LWP_3_STAT  SYSTEM_Ic(20)
#define LWP_4_STAT  SYSTEM_Ic(21)
#define LWP_5_STAT  SYSTEM_Ic(22)
#define LWP_0_DLOG  SYSTEM_Ic(23)
#define LWP_1_DLOG  SYSTEM_Ic(24)
#define LWP_2_DLOG  SYSTEM_Ic(25)
#define LWP_3_DLOG  SYSTEM_Ic(26)
#define LWP_4_DLOG  SYSTEM_Ic(27)
#define LWP_5_DLOG  SYSTEM_Ic(28)
#define LWP_0_DOIT  SYSTEM_Ic(29)
#define LWP_1_DOIT  SYSTEM_Ic(30)
#define LWP_2_DOIT  SYSTEM_Ic(31)
#define LWP_3_DOIT  SYSTEM_Ic(32)
#define LWP_4_DOIT  SYSTEM_Ic(33)
#define LWP_5_DOIT  SYSTEM_Ic(34)
#define LWP_0_DRAG  SYSTEM_Ic(35)
#define LWP_1_DRAG  SYSTEM_Ic(36)
#define LWP_2_DRAG  SYSTEM_Ic(37)
#define LWP_3_DRAG  SYSTEM_Ic(38)
#define LWP_4_DRAG  SYSTEM_Ic(39)
#define LWP_5_DRAG  SYSTEM_Ic(40)
#define LWP_1_BG    SYSTEM_Ic(41)
#define LWP_2_BG    SYSTEM_Ic(42)
#define LWP_3_BG    SYSTEM_Ic(43)

// Allow LWPanels to access new color definitions - Matt Gorner (11.15.2010)
#define LWP_LABEL                    SYSTEM_Ic(69)
#define LWP_WIN_BG                   SYSTEM_Ic(70)

#define LWP_TREE_TITLE               SYSTEM_Ic(71)
#define LWP_TREE_BODY                SYSTEM_Ic(72)

#define LWP_BUTTON_OUTLINE           SYSTEM_Ic(73)
#define LWP_BUTTON_HIGH              SYSTEM_Ic(74)
#define LWP_BUTTON_SELECTED_TEXT     SYSTEM_Ic(75)
#define LWP_BUTTON_LOW               SYSTEM_Ic(76)
#define LWP_BUTTON_SELECTED_HILITE   SYSTEM_Ic(77)
#define LWP_TREE_TEXT                SYSTEM_Ic(78)

/*  Same Palette different names (based on defaults)!*/
#define LWP_GRAY1      SYSTEM_Ic( 7)
#define LWP_GRAY2      SYSTEM_Ic(13)
#define LWP_GRAY3      SYSTEM_Ic( 1)
#define LWP_GRAY4      SYSTEM_Ic( 8)
#define LWP_GRAY5      SYSTEM_Ic( 5)
#define LWP_GRAY6      SYSTEM_Ic( 2)
#define LWP_GRAY7      SYSTEM_Ic(11)
#define LWP_GRAY8      SYSTEM_Ic( 9)
#define LWP_WHITE      SYSTEM_Ic( 4)
#define LWP_CANARY1    SYSTEM_Ic(17)
#define LWP_CANARY2    SYSTEM_Ic(18)
#define LWP_CANARY3    SYSTEM_Ic(19)
#define LWP_CANARY4    SYSTEM_Ic(20)
#define LWP_CANARY5    SYSTEM_Ic(21)
#define LWP_CANARY6    SYSTEM_Ic(22)
#define LWP_AQUA1      SYSTEM_Ic(23)
#define LWP_AQUA2      SYSTEM_Ic(24)
#define LWP_AQUA3      SYSTEM_Ic(25)
#define LWP_AQUA4      SYSTEM_Ic(26)
#define LWP_AQUA5      SYSTEM_Ic(27)
#define LWP_AQUA6      SYSTEM_Ic(28)
#define LWP_LAVENDER1  SYSTEM_Ic(29)
#define LWP_LAVENDER2  SYSTEM_Ic(30)
#define LWP_LAVENDER3  SYSTEM_Ic(31)
#define LWP_LAVENDER4  SYSTEM_Ic(32)
#define LWP_LAVENDER5  SYSTEM_Ic(33)
#define LWP_LAVENDER6  SYSTEM_Ic(34)
#define LWP_OLIVE1     SYSTEM_Ic(35)
#define LWP_OLIVE2     SYSTEM_Ic(36)
#define LWP_OLIVE3     SYSTEM_Ic(37)
#define LWP_OLIVE4     SYSTEM_Ic(38)
#define LWP_OLIVE5     SYSTEM_Ic(39)
#define LWP_OLIVE6     SYSTEM_Ic(40)

#endif