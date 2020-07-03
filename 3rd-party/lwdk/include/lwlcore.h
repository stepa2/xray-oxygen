/*
 * LWSDK Header File
 *
 * LWLCORE.H -- LightWave LCore Exports
 *
 * This header contains declarations for the public services
 * provided by the LightWave LCore subsystem.
 *
 *---------------------------------------------------------------------------
 * Copyright � 2015 NewTek, Inc. and its licensors. All rights reserved.
 *
 * This file contains confidential and proprietary information of
 * NewTek, Inc., and is subject to the terms of the LightWave End User
 * License Agreement (EULA).
 */
#ifndef LWSDK_LCORE_H
#define LWSDK_LCORE_H

#include <lwrender.h>
#include <lwglobsrv.h>

//-------------------------------------------------------------------
// The LScript Universal Requester
//
// This mechanism makes it possible for a C plug-in to employ LScript
// as its requester interface.  This makes it possible to use the
// LSIDE Interface Designer to design the requester, and LScript
// itself to test it before planting it into the C plug-in.

#define LWLSUR_GLOBAL       "LSUniversalRequester"

typedef void *LWLSURID;
typedef struct st_LSURFuncs
{
    // The compile() function takes a character string holding
    // the interface script to be compiled, and a pointer to a
    // pointer of character pointers to hold the identified
    // user-defined variables (if any) in the expression.  It
    // returns an opaque pointer that represents the compiled
    // script, or NULL of something goes horribly wrong.

    LWLSURID    (*compile)(const char **script /* ascii encoded */,char **variable_names /* ascii encoded */,int);

    // The post() function takes a compiled interface script, returned
    // by compile(), and runs it.  Variable data referenced by the
    // script should sequentially match the data provided in the second
    // argument.  Each element will be cast to the appropriate data type
    // based upon the type of control created for data exchange.

    int         (*post)(LWLSURID,void **variable_data);

    // When a host is finished with an interface script, it should
    // call the release() function with the scrpit pointer.  This
    // frees memory used by the script.

    void        (*release)(LWLSURID);
} LSURFuncs;

#endif