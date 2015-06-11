/*
 * Copyright 2012-2014 The MathWorks, Inc.
 *
 * File: ext_util.c
 *
 * Abstract:
 *  Host-side utility functions for External mode
 */

#include "extutil.h"
#include <string.h>
#include "rtiostream.h"
#include "version.h" /* SL_EXT */

#if defined (LEGO_NXT)
#define DEFAULT_CONNECT_TIMEOUT_SECS  30;
#else
#define DEFAULT_CONNECT_TIMEOUT_SECS  120;
#endif

#if defined(EXTMODE_SERIAL_TRANSPORT)
/* ExtUtilProcessSerialPortArg: Process the serial port argument */
PRIVATE int ExtUtilProcessSerialPortArg(ExternalSim   *ES, 
                                   const mxArray * const mat, 
                                   char ** argValue);

/* ExtUtilProcessSerialBaudArg: Process the serial baud argument */
PRIVATE int ExtUtilProcessSerialBaudArg(ExternalSim   *ES, 
                                   const mxArray * const mat, 
                                   char ** argValue);
#endif

#if defined(EXTMODE_TCPIP_TRANSPORT)
/* ExtUtilProcessTCPIPHostArg: Process the TCP/IP host name argument */
PRIVATE int ExtUtilProcessTCPIPHostArg(ExternalSim *ES, 
                                  const mxArray * const mat, 
                                  char ** argValue);

/* ExtUtilProcessTCPIPPortArg: Process the TCP/IP port argument */
PRIVATE int ExtUtilProcessTCPIPPortArg(ExternalSim *ES, 
                                  const mxArray * const mat, 
                                  char ** argValue);
#endif

#if defined(EXTMODE_TCPIP_TRANSPORT) || defined(EXTMODE_SERIAL_TRANSPORT)
/* ExtUtilProcessVerboseArg: Process the verbose argument */
PRIVATE int ExtUtilProcessVerboseArg(ExternalSim   *ES, 
                                const mxArray * const mat, 
                                int * argValue);

/* ExtUtilProcessConnectTimeoutArg: Process the connection timeout argument */
PRIVATE int ExtUtilProcessConnectTimeoutArg(ExternalSim   *ES, 
                                const mxArray * const mat, 
                                int * argValue);

#endif

#if defined(EXTMODE_TCPIP_TRANSPORT)
/* ExtUtilProcessTCPIPPortArg: Process the TCP/IP port argument */
PRIVATE int ExtUtilProcessTCPIPPortArg(ExternalSim *ES, const mxArray * const mat, char ** argValue) {
   int errorOccurred = 0;
   char * portStr = NULL;
   uint16_T port; 
   const size_t         m    = mxGetM(mat);
   const size_t         n    = mxGetN(mat);

   const char msg[] = 
      "Port argument must be a real, scalar, integer value in the range: "
      "[256-65535].";

   /* verify that we've got a real, scalar integer */
   if (!mxIsNumeric(mat) || mxIsComplex(mat) || mxIsSparse(mat) ||
         !mxIsDouble(mat) || (!(m ==1 && n ==1)) || !IS_INT(*mxGetPr(mat))) {
      esSetError(ES, msg);
      errorOccurred = 1;
      return errorOccurred;
   }

   /* check range */
   if (*mxGetPr(mat) < 256 || *mxGetPr(mat) > 65535) {
      esSetError(ES, msg);
      errorOccurred = 1;
      return errorOccurred;
   }

   port = (uint16_T) *mxGetPr(mat);

   {
#define PORT_LENGTH 5 /* port variable is up to uint16 max (65535) - 5 digits */
      const size_t len = PORT_LENGTH + 1;
      port = (uint16_T) *(mxGetPr(mat));
      portStr = mxCalloc(len, sizeof(char));
      if (portStr != NULL) {
         sprintf(portStr, "%d", port);
      }
   }
   *argValue = portStr; /* will be freed when MEX call ends */
   return errorOccurred;
}

/* ExtUtilProcessTCPIPHostArg: Process the TCP/IP host name argument */
PRIVATE int ExtUtilProcessTCPIPHostArg(ExternalSim *ES, const mxArray * const mat, char ** argValue) {
   /* host name specified */
   int errorOccurred = 0;
   char * hostName = NULL;

   if ((mxGetM(mat) != 1) || !mxIsChar(mat)) {
      esSetError(ES, "Expected 1xN string array for host name\n");
      errorOccurred = 1;
      return errorOccurred;
   }

   hostName = mxArrayToString(mat);
   if (hostName == NULL) {
      esSetError(ES, "Error converting matlab string to C string.");      
      errorOccurred = 1;
      return errorOccurred;
   }

   *argValue = hostName; /* will be freed when MEX call ends */ 
   return errorOccurred;
}

/* ExtUtilCreateRtIOStreamArgs: Process arguments required for TCP/IP communications */
PUBLIC int ExtUtilCreateRtIOStreamArgs(ExternalSim   *ES, 
                        int nrhs, 
                        const mxArray * prhs[], 
                        int * argc, 
                        const char * argv[]) {
   int errorOccurred = 0;
   /* initialize connect timeout value, may get overridden by user specified mex file fourth argument */
   int connectTimeOutSecs = DEFAULT_CONNECT_TIMEOUT_SECS;

   /* ... Argument 1 - host name */
   if (nrhs >= 1 && !mxIsEmpty(prhs[0])) {
      /* host name specified */
      char * argValue = 0;
      errorOccurred = ExtUtilProcessTCPIPHostArg(ES, prhs[0], &argValue);
      if (errorOccurred) {
    	  printf("\nError with first arg");
    	  fflush(stdout);
         return errorOccurred;
      } 
      else {
         argv[(*argc)++] = "-hostname";
         argv[(*argc)++] = argValue;
      }
   } 

   /* ... Argument 2 - verbosity */
   if(nrhs >= 2 && !mxIsEmpty(prhs[1])) {
      int argValue = 0;
      errorOccurred = ExtUtilProcessVerboseArg(ES, prhs[1], &argValue);
      if (errorOccurred) {
    	  printf("\nError with 2nd arg");
    	  fflush(stdout);
         return errorOccurred;
      } 
      else {
         esSetVerbosity(ES, argValue); /*lint !e734 loss of precision 31 bits to 8 bits*/   
      }
   }    

   /* ... Argument 3 - TCP port value */
   if (nrhs >= 3) {
      char * argValue = 0;
      errorOccurred = ExtUtilProcessTCPIPPortArg(ES, prhs[2], &argValue);
      if (errorOccurred) {
    	  printf("\nError with 3rd arg");
    	  fflush(stdout);
         return errorOccurred;
      } 
      else {
         argv[(*argc)++] = "-port";
         argv[(*argc)++] = argValue;
      }
   } 

   /* ... Argument 4 - connection timeout */   
   if(nrhs >= 4) {
       errorOccurred = ExtUtilProcessConnectTimeoutArg(ES, prhs[3], &connectTimeOutSecs);
       if (errorOccurred) {
    	   printf("\nError with 4th arg");
    	   fflush(stdout);
           return errorOccurred;
       }
   }
   esSetConnectTimeout(ES, connectTimeOutSecs); /*lint !e734 loss of precision 31 bits to 8 bits*/
      
   /* ... Argument 5 - open rtIOStream as TCP/IP client */
   argv[(*argc)++] = "-client";
   argv[(*argc)++] = "1";

   return errorOccurred;
}
#endif

#if defined(EXTMODE_SERIAL_TRANSPORT)
/* ExtUtilProcessSerialPortArg: Process the serial port argument */
PRIVATE int ExtUtilProcessSerialPortArg(ExternalSim *ES, const mxArray * const mat, char ** argValue) {
   int errorOccurred = 0;
   uint16_T      port;
   size_t         m    = mxGetM(mat);
   size_t         n    = mxGetN(mat);

   char * portStr = NULL;
   const char msg[] = 
      "Port argument must be a positive integer value or a string.";        

   /* verify that we've got a positive integer or string */
   if (!mxIsChar(mat) && (!mxIsNumeric(mat) || mxIsComplex(mat) || mxIsSparse(mat) ||
            !mxIsDouble(mat) || (!(m ==1 && n ==1)) || !IS_INT(*mxGetPr(mat))
            || (*mxGetPr(mat) < 0))) {
      esSetError(ES, msg);
      errorOccurred = 1;
      return errorOccurred;
   }

   if (mxIsChar(mat)) {
      portStr = mxArrayToString(mat);
   }
   else {
#define PORT_LENGTH 5 /* port variable is up to uint16 max (65535) - 5 digits */
      const size_t len = PORT_LENGTH + 1;
      port = (uint16_T) *(mxGetPr(mat));
      portStr = mxCalloc(len, sizeof(char));
      if (portStr != NULL) {
         sprintf(portStr, "%d", port);
      }
   }
   *argValue = portStr; /* will be freed when MEX call ends */
   return errorOccurred;
}

/* ExtUtilProcessSerialBaudArg: Process the serial baud argument */
PRIVATE int ExtUtilProcessSerialBaudArg(ExternalSim   *ES, 
                            const mxArray * const mat, 
                            char ** argValue) {
   int errorOccurred = 0;   
   const char_T *baudRatesStr =
      "Baud rate argument must be a positive integer value.";
   uint32_T      baud;
   const size_t         m    = mxGetM(mat);
   const size_t         n    = mxGetN(mat);        
   char * baudStr = NULL;

   /* verify that we've got a positive integer */
   if (!mxIsNumeric(mat) || mxIsComplex(mat) || mxIsSparse(mat) ||
         !mxIsDouble(mat) || (!(m ==1 && n ==1)) || !IS_INT(*mxGetPr(mat))
         || (*mxGetPr(mat) < 0)) {
      esSetError(ES, baudRatesStr);
      errorOccurred = 1;      
      return errorOccurred;
   }
   baud = (uint32_T) *mxGetPr(mat);       
   {
#define BAUD_LENGTH 10 /* baud variable is up to uint32 max (4294967295) - 10 digits */
      const size_t len = BAUD_LENGTH + 1;
      baudStr = mxCalloc(len, sizeof(char));
      if (baudStr != NULL) {
         sprintf(baudStr, "%d", baud);
      }
   }
   *argValue = baudStr; /* will be freed when MEX call ends */ 
   return errorOccurred;   
}

/* ExtUtilCreateRtIOStreamArgs: Process arguments required for serial communications */
PUBLIC int ExtUtilCreateRtIOStreamArgs(ExternalSim   *ES, 
                         int nrhs, 
                         const mxArray * prhs[], 
                         int * argc, 
                         const char * argv[]) {
   int errorOccurred = 0;
   /* initialize connect timeout value, may get overridden by user specified mex arg fourth argument */
   int connectTimeOutSecs = DEFAULT_CONNECT_TIMEOUT_SECS;
   
   /* ... Argument 1 - verbosity */
   if(nrhs >= 1 && !mxIsEmpty(prhs[0])) {
      int argValue = 0;
      errorOccurred = ExtUtilProcessVerboseArg(ES, prhs[0], &argValue);
      if (errorOccurred) {
         return errorOccurred;
      } 
      else {
         esSetVerbosity(ES, argValue); /*lint !e734 loss of precision 31 bits to 8 bits*/   
      }
   }    

   /* ... Argument 2 - serial comm port value */
   if (nrhs >= 2 && !mxIsEmpty(prhs[1])) {
      char * argValue = NULL;
      errorOccurred = ExtUtilProcessSerialPortArg(ES, prhs[1], &argValue);
      if (errorOccurred) {
         return errorOccurred;
      } 
      else {
         argv[(*argc)++] = "-port";
         argv[(*argc)++] = argValue;
      }
   } 

   /* ... Argument 3 - serial baud rate value */
   if (nrhs >= 3) {        
      char * argValue = NULL;
      errorOccurred = ExtUtilProcessSerialBaudArg(ES, prhs[2], &argValue);
      if (errorOccurred) {
         return errorOccurred;
      } 
      else {
         argv[(*argc)++] = "-baud";
         argv[(*argc)++] = argValue;
      }
   }
   
   /* ... Argument 4 - connection timeout */
   if(nrhs >= 4) {
       errorOccurred = ExtUtilProcessConnectTimeoutArg(ES, prhs[3], &connectTimeOutSecs);
       if (errorOccurred) {
           return errorOccurred;
       }
   }
   esSetConnectTimeout(ES, connectTimeOutSecs); /*lint !e734 loss of precision 31 bits to 8 bits*/

   #if defined(EXTMODE_HOST_RTS_DTR_ENABLED)
        argv[(*argc)++] = "-rtsDtrEnable";
        argv[(*argc)++] = "1";
   #endif

   return errorOccurred;
}
#endif

#if defined(EXTMODE_TCPIP_TRANSPORT) || defined(EXTMODE_SERIAL_TRANSPORT)
/* ExtUtilProcessVerboseArg: Process the verbose argument */
PRIVATE int ExtUtilProcessVerboseArg(ExternalSim   *ES, 
                         const mxArray * const mat, 
                         int * argValue) {
   int errorOccurred = 0;
   const size_t         m    = mxGetM(mat);
   const size_t         n    = mxGetN(mat);

   const char msg[] =
      "Verbosity argument must be a real, scalar, integer value in the "
      "range: [0-1].";

   /* verify that we've got a real, scalar integer */
   if (!mxIsNumeric(mat) || mxIsComplex(mat) || mxIsSparse(mat) ||
         !mxIsDouble(mat) || (!(m ==1 && n ==1)) || !IS_INT(*mxGetPr(mat))) {
      esSetError(ES, msg);
      errorOccurred = 1;
      return errorOccurred;
   }
   *argValue = (int) *mxGetPr(mat);

   /* verify that it's 0 or 1 */
   if ((*argValue != 0) && (*argValue != 1)) {
      esSetError(ES, msg);
      errorOccurred = 1;      
   }
   return errorOccurred;
}

/* ExtUtilProcessConnectTimeoutArg: Process the connection timeout argument */
PRIVATE int ExtUtilProcessConnectTimeoutArg(ExternalSim   *ES, 
                         const mxArray * const mat, 
                         int * argValue) {
   int errorOccurred = 0;
   const size_t         m    = mxGetM(mat);
   const size_t         n    = mxGetN(mat);

   const char msg[] =
      "Connection timeout argument must be a real, scalar, integer, positive value";

   /* verify that we've got a real, scalar integer */
   if (!mxIsNumeric(mat) || mxIsComplex(mat) || mxIsSparse(mat) ||
         !mxIsDouble(mat) || (!(m ==1 && n ==1)) || !IS_INT(*mxGetPr(mat))) {
      esSetError(ES, msg);
      errorOccurred = 1;
      return errorOccurred;
   }
   *argValue = (int) *mxGetPr(mat);

   /* verify that it's >= 0 */
   if (*argValue < 0) {
      esSetError(ES, msg);
      errorOccurred = 1;      
   }
   return errorOccurred;
}
#endif

/* Function: ExtUtilLoadSharedLib ===========================================================
 * Abstract: Load the rtiostream library.
 */
PUBLIC int ExtUtilLoadSharedLib(RtIOStreamData * rtiostreamData) {
#if defined (EXTMODE_SERIAL_TRANSPORT)
   #define RTIOSTREAM_SHARED_LIB "libmwrtiostreamserial"
#elif defined (EXTMODE_TCPIP_TRANSPORT)
   #define RTIOSTREAM_SHARED_LIB "libmwrtiostreamtcpip"
#else
   #error Unsupported external mode transport layer
#endif

    static char libName[] = RTIOSTREAM_SHARED_LIB SL_EXT ;    
    int error = RTIOSTREAM_NO_ERROR;
    
    if (!rtiostreamData->sharedLibLoaded) {
       error = rtIOStreamLoadLib( &(rtiostreamData->libH), libName);

       if (error != RTIOSTREAM_NO_ERROR) {
          printf("Failed to load rtIOStream shared library %s. Check that the library "
                "is located in a directory on your system path.", libName);/*lint !e534 
                                                                            * ignore return value of mexPrintf */
       }
       else {
          /* flag that we loaded the lib */
          rtiostreamData->sharedLibLoaded = 1;
       }
    }

    return error;
}

/* Function: ExtUtilUnloadSharedLib ===========================================================
 * Abstract: Unload the rtiostream library.
 */
PUBLIC int ExtUtilUnloadSharedLib(RtIOStreamData * rtiostreamData) {
   if (rtiostreamData->sharedLibLoaded) {
      /* clear flag */
      rtiostreamData->sharedLibLoaded = 0;
      /* unload */
      return rtIOStreamUnloadLib(&(rtiostreamData->libH));
   }
   else {
      /* not loaded */
      return RTIOSTREAM_NO_ERROR;
   }
}
