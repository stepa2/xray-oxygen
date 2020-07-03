/*
 * LWSDK Header File
 *
 * LWOBJREP.H -- LightWave Object Replacement
 *
 *---------------------------------------------------------------------------
 * Copyright � 2015 NewTek, Inc. and its licensors. All rights reserved.
 *
 * This file contains confidential and proprietary information of
 * NewTek, Inc., and is subject to the terms of the LightWave End User
 * License Agreement (EULA).
 */
#ifndef LWSDK_OBJREP_H
#define LWSDK_OBJREP_H

#include <lwrender.h>
#include <lwcmdseq.h>

#define LWOBJREPLACEMENT_HCLASS     "ObjReplacementHandler"
#define LWOBJREPLACEMENT_ICLASS     "ObjReplacementInterface"
#define LWOBJREPLACEMENT_GCLASS     "ObjReplacementGizmo"
#define LWOBJREPLACEMENT_VERSION    4


typedef struct st_LWObjReplacementAccess {
    LWItemID     objectID;
    LWFrame      curFrame, newFrame;
    LWTime       curTime,  newTime;
    int      curType,  newType;
    const char  *curFilename;
    const char  *newFilename;
    int         force;

    // Providing a modeling callback routine allows mesh manipulation right after full object replacement
    // This routine is only valid once after it has been specified.
    // return 1 if the mesh was edited, else 0.
    // The primary foreground layer is the mesh of the object being replaced.
    // There are no background layers.  Available LW9.6.1+
    int         (*editMesh)( LWModCommand *modeling_command);
} LWObjReplacementAccess;

#define LWOBJREP_NONE    0
#define LWOBJREP_PREVIEW 1
#define LWOBJREP_RENDER  2
#define LWOBJREP_INTERACTIVE 3

typedef struct st_LWObjReplacementHandler {
    LWInstanceFuncs  *inst;
    LWItemFuncs  *item;
    void        (*evaluate) (LWInstance, LWObjReplacementAccess *);
} LWObjReplacementHandler;


#endif
