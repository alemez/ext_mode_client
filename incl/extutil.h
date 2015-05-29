#ifndef __EXTUTIL__
#define __EXTUTIL__

/*
 * Copyright 1994-2014 The MathWorks, Inc.
 *
 * File    : extutil.h
 * Abstract:
 *  Utilities such as assert.
 */

/*
 * Set ASSERTS_ON to 1 turn asserts on, 0 otherwise.
 */
#define ASSERTS_ON (1)

/*------------------------------------------------------------------------------
 * Do not modify below this line.
 *----------------------------------------------------------------------------*/
#if ASSERTS_ON
#include <assert.h>
#else
#define assert(dum) /* do nothing */
#endif


#include "mex.h"
#include "ext_share.h"
#include "extsim.h"
#include "rtiostream_utils.h"

#if defined (EXTMODE_SERIAL_TRANSPORT) && defined (EXTMODE_TCPIP_TRANSPORT)
    #error Cannot have both EXTMODE_SERIAL_TRANSPORT and EXTMODE_TCPIP_TRANSPORT defined
#endif

#if defined(EXTMODE_TCPIP_TRANSPORT)
   #define ARGC_MAX 6 /* -hostname, -port, -client */
   #define ARGV_INIT {NULL, NULL, NULL, NULL, NULL, NULL}
#endif

#if defined(EXTMODE_SERIAL_TRANSPORT)
#if defined(EXTMODE_HOST_RTS_DTR_ENABLED)
   #define ARGC_MAX 6 /* -port, -baud -rtsdtrenable*/
   #define ARGV_INIT {NULL, NULL, NULL, NULL, NULL, NULL}
#else
   #define ARGC_MAX 4 /* -port, -baud */
   #define ARGV_INIT {NULL, NULL, NULL, NULL}
#endif
#endif

#define PENDING_DATA_CACHE_SIZE 8

typedef struct RtIOStreamData_tag {
    int streamID; /* id returned by rtIOStreamOpen */
    int sharedLibLoaded; /* has rtiostream shared library been loaded */
    libH_type libH; /* rtiostream shared library handle */
    char pendingRecvdData[PENDING_DATA_CACHE_SIZE]; /* pending receive data buffer */
    int numPending; /* amount of data in the pending receive buffer */
    size_t startIdxPending; /* current position in the pending receive buffer */
} RtIOStreamData;

/* ExtUtilCreateRtIOStreamArgs: Create argc, argv for rtiostream */
PUBLIC int ExtUtilCreateRtIOStreamArgs(ExternalSim   *ES, 
                                   int nrhs, 
                                   const mxArray * prhs[], 
                                   int * argc, 
                                   const char * argv[]);

/* ExtUtilLoadSharedLib: Load the rtiostream library. */
PUBLIC int ExtUtilLoadSharedLib(RtIOStreamData * rtiostreamData);

/* ExtUtilUnloadSharedLib: Unload the rtiostream library. */
PUBLIC int ExtUtilUnloadSharedLib(RtIOStreamData * rtiostreamData);


#endif /* __EXTUTIL__ */
