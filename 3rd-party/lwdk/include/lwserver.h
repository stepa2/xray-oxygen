/*
 * LWSDK Header File
 *
 * LWSERVER.H -- LightWave Plug-in Server
 *
 * This header contains the basic declarations need to define the
 * simplest LightWave plug-in server.
 *
 *---------------------------------------------------------------------------
 * Copyright � 2015 NewTek, Inc. and its licensors. All rights reserved.
 *
 * This file contains confidential and proprietary information of
 * NewTek, Inc., and is subject to the terms of the LightWave End User
 * License Agreement (EULA).
 */
#ifndef LWSDK_SERVER_H
#define LWSDK_SERVER_H

#include    <lwtypes.h>

/*
 * External Entry Points.  All entry points which may be called from the
 * host must be declared as XCALL types.  The obsolete XCALL_INIT is
 * still present but defined as nothing.
 */
#ifdef _MSWIN
    #define XCALL_(t)   t
#endif
#ifdef _LINUX
    #define XCALL_(t)   t
#endif
#ifdef _MACOS
    #define XCALL_(t)   t
#endif

#define XCALL_INIT


/*
 * Global Function.  The global function is a callback provided by the
 * host taking the following form.  A pointer for the given service name
 * is returned for the given useage.  The use codes are for services
 * that will be used once or that will be used many times.
 */
typedef void *  GlobalFunc (const char *serviceName /* ascii encoded */, int useMode);

#define GFUSE_TRANSIENT         0
#define GFUSE_ACQUIRE           1
#define GFUSE_RELEASE           2


/*
 * Activation Function.  The server entry points are all functions of
 * this kind, taking a version number, global function pointer, local
 * (class-specific) data and module data.  The return value indicates
 * if the service could run and the reason for failure if it couldn't.
 */
typedef int ActivateFunc (  int          version,
                            GlobalFunc  *global,
                            void        *local,
                            void        *serverData );

#define AFUNC_OK                0
#define AFUNC_BADVERSION        1
#define AFUNC_BADGLOBAL         2
#define AFUNC_BADLOCAL          3
#define AFUNC_BADAPP            4
#define AFUNC_BADAPP_SILENT     5


/*
 * This is a selection of some of the more useful language ID's,
 * although this list is far from complete.  Remaining codes can be
 * found by searching the Microsoft Windows documentation for OLE
 * LANGID codes.  Japanese strings should be encoded as JIS on Windows
 * and EUC on Unix.
 */
#define LANGID_GERMAN               0x0407
#define LANGID_USENGLISH            0x0409
#define LANGID_UKENGLISH            0x0809
#define LANGID_SPANISH              0x040a
#define LANGID_FRENCH               0x040c
#define LANGID_ITALIAN              0x0410
#define LANGID_JAPANESE             0x0411
#define LANGID_KOREAN               0x0412
#define LANGID_RUSSIAN              0x0419
#define LANGID_SWEDISH              0x041D
#define LANGID_CHINESE_SIMPLIFIED   0x0804

/*
 * Each server can contain a list of tagged strings which declare
 * some of the static information about that server.  Tags are a bitwise
 * OR of a SRVTAG code which selects the type of information, and a
 * language ID word which selects the language for which the string is
 * valid.
 */
#define SRVTAG_USERNAME     0x00000
#define SRVTAG_BUTTONNAME   0x10000
#define SRVTAG_CMDGROUP     0x20000
#define SRVTAG_MENU         0x30000
#define SRVTAG_DESCRIPTION  0x40000
#define SRVTAG_ENABLE       0x50000
#define SRVTAG_SELECTCMD    0x60000
#define SRVTAG_NODEGROUP    0x70000
#define SRVTAG_NODECONTEXT  0x80000
#define SRVTAG_TOGGLE       0x90000

typedef struct st_ServerTagInfo
{
    const char      *string; /* ascii encoded? */
    unsigned int     tag;

} ServerTagInfo;

/*
 * Server Definition Record.  Each server record describes a single
 * activation entry point in the plug-in module.  Each has a class,
 * a name, an activation function, and an array of tag info structs
 * terminated with a zero tag code.
 */
typedef struct st_ServerRecord
{
    const char      *className; /* ascii encoded */
    const char      *name; /* ascii encoded? */
    ActivateFunc    *activate;
    ServerTagInfo   *tagInfo;

} ServerRecord;

#define ServerUserName ServerTagInfo


#define LWSERVERINFO_GLOBAL "LW Server Info"

typedef struct st_LWServerInfo
{
    const char* /* language encoded */ (*path)        (const char* server_class /* ascii encoded */,const char* server_name /* ascii encoded? */);
    const char** /* ascii encoded? */  (*list)        (const char* server_class /* ascii encoded */);
} LWServerInfo;

#endif