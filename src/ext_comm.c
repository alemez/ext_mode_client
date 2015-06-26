/*
 * Copyright 1994-2013 The MathWorks, Inc.
 *
 * File: ext_comm.c     
 *
 * Abstract:
 *  Host-side, transport-independent external-mode functions.  Calls to these
 *  functions originate from Simulink and are dispatched through ext_main.c
 *  Functions are included to:
 *      o set (send) packets to the target
 *      o get (receive) packets from the target
 *      o open connection with target
 *      o close connection with target
 *      o etc
 *
 *  Transport specific code (e.g., TCPIP code) resides in ext_transport.c.
 *  Modify that file to customize external mode to various transport
 *  mechanisms (e.g., shared memory, serial line, etc).
 */

/*****************
 * Include files *
 *****************/

/*ANSI C headers*/
#include <stdio.h>
#include <string.h>

/*Real Time Workshop headers*/
#include "tmwtypes.h"
#include "mex.h"
#include "extsim.h"
#include "ext_convert.h"
#include "extutil.h"
#include "ext_transport.h"
#include "ext_share.h"


/* Function: FreeAndNullUserData ===============================================
 * Abstract:
 *  Free user data and null it out in the external sim struct.
 */
PRIVATE void FreeAndNullUserData(ExternalSim *ES)
{
    ExtUserDataDestroy(esGetUserData(ES));
    esSetUserData(ES, NULL);
} /* end FreeAndNullUserData */


/* Function: ExtGetPktHdr =====================================================
 * Abstract:
 *  Get all bytes comprising a single packet header on the comm line.
 */
PRIVATE boolean_T ExtRecvIncomingPktHeader(
    ExternalSim *ES,
    PktHeader   *pktHdr)
{
	printf("\n---ExtRecvIncomingPktHeader---");
	fflush(stdout);
    char      *bufPtr;
    int       nBytes     = 0;    /* total pkt header bytes recv'd. */
    int       nGot       = 0;    /* num bytes recv'd in one call to ExtGetTargetPkt. */
    int       noDataCntr = 1000; /* determines if target exited unexpectedly. */
    boolean_T error      = EXT_NO_ERROR;

    /*
     * Loop until all bytes are received for the incoming packet header.
     * The packet header may not be read in one shot.
     */
    while (nBytes != sizeof(PktHeader)) {
        /*
         * Assume true since we should only be here after a call to
         * ExtTargetPktPending() returned true
         */
        boolean_T pending = true;

        /*
         * We may have received some of the packet header, but not all of
         * it.  Check to see if any additional data is pending to complete
         * the packet header.
         */
        if (nBytes > 0) {
            error = ExtTargetPktPending(ES, &pending, 0, 100000);
            if (error) {
                esSetError(ES,
                           "ExtTargetPktPending() call failed while checking "
                           " for target pkt\n");
                goto EXIT_POINT;
            }
        }

        /*
         * Get any pending data
         */
        if (pending) {
            bufPtr = (char_T *)pktHdr + nBytes;
            error = ExtGetTargetPkt(ES,sizeof(PktHeader)-nBytes,&nGot,bufPtr);
            if (error) {
            	printf("\nExtGetTargetPkt() call failed while checking "
                           " target pkt header.");
                esSetError(ES, 
                           "ExtGetTargetPkt() call failed while checking "
                           " target pkt header.\n");
                goto EXIT_POINT;
            }
            nBytes += nGot;
        }

        /*
         * ExtRecvIncomingPktHeader() is called only after a call to
         * ExtTargetPktPending() returns true.  For tcp/ip, this function
         * may return true if the tcp/ip communication was shut down by
         * the target unexpectedly (e.g. someone did a ctrl-c to stop
         * the target executable while connected via extmode).  If
         * ExtTargetPktPending() returns true but we do not see any data
         * after checking the communication line some number of times,
         * we must break out of this infinite loop or Matlab will hang.
         */
        if (nGot == 0) {
            noDataCntr--;
            if (noDataCntr == 0) {
            	printf("\n!!Error!!");
                error = 1;
                esSetError(ES, 
                           "ExtGetTargetPkt() call failed while checking "
                           " target pkt header.\n");
                goto EXIT_POINT;
            }
        }
    }

EXIT_POINT:
    return error;
}

/* Function: ExtRecvIncomingPkt ================================================
 * Abstract:
 *  Check for packets (poll) from target on the comm line.  If a packet
 *  is pending, set the incoming packet pending flag to true and set the
 *  incoming packet type.  Otherwise, do nothing.
 */
PRIVATE void ExtRecvIncomingPkt(
    ExternalSim    *ES,
    int_T          nrhs,
    const mxArray  *prhs[])
{
	printf("\n---ExtRecvIncomingPkt---");
	fflush(stdout);
    boolean_T pending;
    char      *bufPtr;
    int       nGot         = 0;
    boolean_T error        = EXT_NO_ERROR;
    char      *buf         = esGetIncomingPktDataBuf(ES);
    int32_T   nBytesNeeded = esGetIncomingPktDataNBytesNeeded(ES);
    int       nBytes       = esGetIncomingPktDataNBytesInBuf(ES);

    (void)nrhs; /* unused */
    (void)prhs; /* unused */

    /*
     * Start recv'ing a packet.
     */
    if (nBytesNeeded == UNKNOWN_BYTES_NEEDED) {
        assert(nBytes == 0);

        /*
         * Check for pending packet.
         */
        error = ExtTargetPktPending(ES, &pending, 0, 100000);
        if (error) {
            esSetError(ES,
                       "ExtTargetPktPending() call failed while checking "
                       " for target pkt\n");
            goto EXIT_POINT;
        }
        if (!pending) goto EXIT_POINT; /* nothing to read */

        /*
         * Process pending packet.
         */
        {
            PktHeader pktHdr;

            error = ExtRecvIncomingPktHeader(ES, &pktHdr);
            if (error) goto EXIT_POINT;
            
            /* Convert the pkt hdr to host format. */
            Copy32BitsFromTarget(ES,
                                 (uint32_T *)&pktHdr, 
                                 (char *)&pktHdr, NUM_HDR_ELS);
            if (!esIsErrorClear(ES)) goto EXIT_POINT;
            
            nBytesNeeded = pktHdr.size * esGetHostBytesPerTargetByte(ES);
            assert(nBytesNeeded <= esGetIncomingPktDataBufSize(ES));
            
            /*
             * We have a packet pending.  Set the flag and the type.
             */
            esSetIncomingPktPending(ES, TRUE);
            esSetIncomingPkt(ES, (ExtModeAction)pktHdr.type);
        }
    }
    if (nBytesNeeded == 0) {
        goto EXIT_POINT;
    } else {
        /* stay around and look for packet data */
    }
    
    bufPtr = buf + nBytes;
    
    /*
     * Check for pending pkt data.  
     */
    error = ExtTargetPktPending(ES, &pending, 0, 0);
    if (error) {
        esSetError(ES,
                   "ExtTargetPktPending() call failed while checking for "
                   " target pkt\n");
        goto EXIT_POINT;
    }
    
    /*
     * Process pending data - may not get it all in one shot.
     */
    if (pending) {
        error = ExtGetTargetPkt(ES,nBytesNeeded,&nGot,bufPtr);
        if (error) {
            esSetError(ES, "ExtGetTargetPkt() call failed while retrieving pkt data\n");
            goto EXIT_POINT;
        }
        esSetIncomingPktPending(ES, TRUE);
        
        nBytesNeeded -= nGot;
        nBytes       += nGot;
        bufPtr       += nGot;

        assert(nBytesNeeded >= 0);
    }

EXIT_POINT:
    esSetIncomingPktDataNBytesNeeded(ES, nBytesNeeded);
    esSetIncomingPktDataNBytesInBuf (ES, nBytes      );
} /* end ExtRecvIncomingPkt */


/* Function: ExtConnect ========================================================
 * Abstract:
 *  Establish communication with target.
 */
PRIVATE void ExtConnect(
    ExternalSim    *ES,
    int_T          nrhs,
    const mxArray  *prhs[])
{
	printf("\n---ExtConnect---\n");
	fflush(stdout);

    int_T          nGot;
    int_T          nSet;
    PktHeader      pktHdr;
    boolean_T      pending;
    int16_T        one          = 1;
    boolean_T      error        = EXT_NO_ERROR;
    long int       timeOutSecs  = 0;
    const long int timeOutUSecs = 0;
    
    /* Allocate error messages in case we fail to connect */
    #define FAILED_TO_CONNECT_MSG_LEN 1024
    #define FAILED_TO_CONNECT_CAUSES_MSG_LEN 512
    char failedToConnectErrMsg[FAILED_TO_CONNECT_MSG_LEN];
    char failedToConnectCauses[FAILED_TO_CONNECT_CAUSES_MSG_LEN];
    
    /* This is the list of error causes when we fail to connect */
    sprintf(failedToConnectCauses, "%s\n%s\n%s\n",
            "a) The target is not switched on.",
            "b) The target is not connected to your host machine.",
            "c) The application for the model is not running on the target. You might have clicked the Stop button.");    
            
    {
        UserData *userData = ExtUserDataCreate();
        if (userData == NULL) {
        	printf("\nMemory allocation error");
        	fflush(stdout);
            esSetError(ES, "Memory allocation error.");
            goto EXIT_POINT;
        }

        esSetUserData(ES, (void*)userData);
    }

    /*
     * Parse the arguments.
     */
    assert(esIsErrorClear(ES));
    ExtProcessArgs(ES,nrhs,prhs);
    if (!esIsErrorClear(ES)) {
    	printf("\nFailed to connect to target");
    	fflush(stdout);
        const char errMsgHeader[] = "Failed to connect to the target. Possible reasons for the failure:";
        sprintf(failedToConnectErrMsg, "%s\n %s\n Caused by:\n %s", errMsgHeader, failedToConnectCauses, esGetError(ES));        
        esSetError(ES, failedToConnectErrMsg);
        goto EXIT_POINT;
    }

    assert(esIsErrorClear(ES));
    timeOutSecs  =  (long int) esGetConnectTimeout(ES);
    
    ExtOpenConnection(ES);
    if (!esIsErrorClear(ES))
    	{
    		printf("\nError with ext open connection");
    		fflush(stdout);
    		goto EXIT_POINT;
    	}

    /*
     * Send the EXT_CONNECT pkt to the target.  This packet consists
     * of the string 'ext-mode'.  The purpose of this header is to serve
     * as a flag to start the handshaking process.
     */
    (void)memcpy((void *)&pktHdr,"ext-mode",8);
    error = ExtSetTargetPkt(ES,sizeof(pktHdr),(char *)&pktHdr,&nSet);
    if (error || (nSet != sizeof(pktHdr))) {
    	printf("\nExtSetTargetPkt() call failed on EXT_CONNECT");
    	fflush(stdout);
        esSetError(ES, "ExtSetTargetPkt() call failed on EXT_CONNECT.\n"
	               "Ensure target is still running\n");
        goto EXIT_POINT;
    }

    /*
     * Get the first of two EXT_CONNECT_RESPONSE packets.  See the 
     * ext_conv.c/ProcessConnectResponse1() function for a description of
     * the packet.  If the timeout elapses before receiving the response
     * pkt, an error is returned.
     *
     * NOTE: Until both EXT_CONNECT_RESPONSE packets are read, packets
     *       received from the target consists solely of unsigned 32 bit
     *       integers.
     */
    error = ExtTargetPktPending(ES, &pending, timeOutSecs, timeOutUSecs);
    if (error || !pending) {
    	printf("\nFailed to connect to target");
    	fflush(stdout);
        const char errMsgHeader[] = "Failed to connect to the target. A time-out occurred while "
                "waiting for the first connect response packet. Possible reasons for the time-out:";
        sprintf(failedToConnectErrMsg, "%s\n%s", errMsgHeader, failedToConnectCauses);
        esSetError(ES, failedToConnectErrMsg);
        goto EXIT_POINT;
    }
    assert(pending);

    error = ExtRecvIncomingPktHeader(ES, &pktHdr);
    if (error) goto EXIT_POINT;
            
    ProcessConnectResponse1(ES, &pktHdr);
    if (!esIsErrorClear(ES))
    	{
    		printf("\nError with ProcessConnectResponse1");
    		fflush(stdout);
    		goto EXIT_POINT;
    	}
    
    if (esGetVerbosity(ES)) {
        MachByteOrder hostEndian = 
            (*((int8_T *) &one) == 1) ? LittleEndian : BigEndian;

        printf("host endian mode:\t%s\n",
	  (hostEndian == LittleEndian) ? "Little":"Big");

        printf("byte swapping required:\t%s\n",
            (esGetSwapBytes(ES)) ? "true":"false");
    }
    
    /*
     * Get the second of two EXT_CONNECT_RESPONSE packets.  If the timeout
     * elapses before receiving the response pkt, an error is returned.
     * The format of the packet is:
     *
     * CS1 - checksum 1 (uint32_T)
     * CS2 - checksum 2 (uint32_T)
     * CS3 - checksum 3 (uint32_T)
     * CS4 - checksum 4 (uint32_T)
     * 
     * intCodeOnly   - flag indicating if target is integer only (uint32_T)
     *
     * MWChunkSize   - multiword data type chunk size on target (uint32_T)
     * 
     * targetStatus  - the status of the target (uint32_T)
     *
     * nDataTypes    - # of data types          (uint32_T)
     * dataTypeSizes - 1 per nDataTypes         (uint32_T[])
     */
    error = ExtTargetPktPending(ES, &pending, timeOutSecs, timeOutUSecs);
    if (error || !pending) {
    	printf("\nError waiting for second connect response packet");
    	fflush(stdout);
        esSetError(
            ES, "Timed-out waiting for second connect response packet.\n");
        goto EXIT_POINT;
    }
    assert(pending);

    error = ExtRecvIncomingPktHeader(ES, &pktHdr);
    if (error) goto EXIT_POINT;


    Copy32BitsFromTarget(ES, (uint32_T *)&pktHdr, (char *)&pktHdr, NUM_HDR_ELS);
    if (!esIsErrorClear(ES))
    	{
    		printf("\nError with second Copy32BitsFromTarget");
    		fflush(stdout);
    		goto EXIT_POINT;
    	}

    if (pktHdr.type != EXT_CONNECT_RESPONSE) {
    	printf("\n!!Error. Unexpected response from target.");
    	fflush(stdout);
        esSetError(ES, "Unexpected response from target. "
                       "Expected EXT_CONNECT_RESPONSE.\n");
        goto EXIT_POINT;
    }

    /*
     * Allocate space to hold packet.
     */
    {
        uint32_T *tmpBuf;
        uint32_T *bufPtr;
        int_T    pktSize    = pktHdr.size * esGetHostBytesPerTargetByte(ES);
        int      bytesToGet = pktSize;

        tmpBuf = (uint32_T *)malloc(pktSize);
        if (tmpBuf == NULL) {
        	printf("\n!!Memory allocation error!!");
        	fflush(stdout);
            esSetError(ES, "Memory allocation failure\n");
            goto EXIT_POINT;
        }
        bufPtr = tmpBuf;

        /*
         * Get the 2nd connect response packet.  It may not be transmitted as
         * one whole packet.
         */
        {
            char_T *buf = (char_T *)tmpBuf;

            while(bytesToGet != 0) {
                /*
                 * Look for any pending data.  If we ever go more than
                 * 'timeOutVal' seconds, bail with an error.
                 */
                error = ExtTargetPktPending(ES,&pending,timeOutSecs,timeOutUSecs);
                if (error || !pending) {
                	printf("\nExtTargetPktPending() call failed while checking"
                			"for 2nd EXT_CONNECT_RESPONSE target pkt");
                	fflush(stdout);
                    free(tmpBuf);
                    esSetError(
                        ES, "ExtTargetPktPending() call failed while checking "
                        " for 2nd EXT_CONNECT_RESPONSE target pkt\n");
                    goto EXIT_POINT;
                }

                /*
                 * Grab the data.
                 */
                error = ExtGetTargetPkt(ES,bytesToGet,&nGot,buf);
                if (error) {
                    free(tmpBuf);
                    esSetError(ES, 
                               "ExtGetTargetPkt() call failed while getting "
                               " for 2nd EXT_CONNECT_RESPONSE target pkt\n");
                    goto EXIT_POINT;
                }

                buf        += nGot;
                bytesToGet -= nGot;
                assert(bytesToGet >= 0);
            }
        }

        /*
         * This packet happens to consists of all uint32_T.  Convert them
         * in a batch.
         */
        Copy32BitsFromTarget(ES,tmpBuf,(char *)tmpBuf,pktSize/sizeof(uint32_T));
        if (!esIsErrorClear(ES)) goto EXIT_POINT;

        /* process 128 bit checksum */
        esSetTargetModelCheckSum(ES, 0, *bufPtr++);
        esSetTargetModelCheckSum(ES, 1, *bufPtr++);
        esSetTargetModelCheckSum(ES, 2, *bufPtr++);
        esSetTargetModelCheckSum(ES, 3, *bufPtr++);

        /* process integer only flag */
        esSetIntOnly(ES, (boolean_T)*bufPtr++);

        if (esGetIntOnly(ES)) InstallIntegerOnlyDoubleConversionRoutines(ES);

        if (esGetVerbosity(ES)) {
            printf("target integer only code:\t%s\n",
                      (esGetIntOnly(ES)) ? "true":"false");
        }

        /* process multiword data type chunk size */
        esSetTargetMWChunkSize(ES, *bufPtr++);

        if (esGetVerbosity(ES)) {
            printf("target multiword chunk size:\t%d bytes\n",
                      (esGetTargetMWChunkSize(ES)));
        }
    
        /* process target status */
        esSetTargetSimStatus(ES, (TargetSimStatus)*bufPtr++);

        ProcessTargetDataSizes(ES, bufPtr);
        free(tmpBuf);
        if (!esIsErrorClear(ES)) goto EXIT_POINT;
    }
    
    /*
     * Set up function pointers for Simulink.
     */
    esSetRecvIncomingPktFcn(ES, ExtRecvIncomingPkt);

    printf("\n!!swap bytes: %d", esGetSwapBytes(ES));

EXIT_POINT:
    if (!esIsErrorClear(ES)) {
        ExtCloseConnection(ES);
        FreeAndNullUserData(ES);
        printf("\n---Ext finished with errors---");
        fflush(stdout);
    }
} /* end ExtConnect */


/* Function: ExtDisconnectRequest ==============================================
 * Abstract:
 *  A request to terminate communication with target has been made.  Notify
 *  the target (if it is up and running).  The connection status will be:
 *  EXTMODE_DISCONNECT_REQUESTED
 */
PRIVATE void ExtDisconnectRequest(
    ExternalSim    *ES,
    int_T          nrhs,
    const mxArray  *prhs[])
{
	printf("\n---ExtDisconnectRequest---\n");
    PktHeader pktHdr;
    int       nSet;
    boolean_T error = EXT_NO_ERROR;

    assert(esGetConnectionStatus(ES) == EXTMODE_DISCONNECT_REQUESTED);

    (void)nrhs; /* unused */
    (void)prhs; /* unused */

    pktHdr.size = 0;
    pktHdr.type = EXT_DISCONNECT_REQUEST;

    Copy32BitsToTarget(ES,(char *)&pktHdr,(uint32_T *)&pktHdr,NUM_HDR_ELS);
    if (!esIsErrorClear(ES)) goto EXIT_POINT;

    error = ExtSetTargetPkt(ES,sizeof(pktHdr),(char *)&pktHdr,&nSet);
    if (error || (nSet != sizeof(pktHdr))) {
    	printf("\n!!Finishing with errors in ExtDisconnectRequest");
    	fflush(stdout);
        esSetError(ES, "ExtSetTargetPkt() call failed on CLOSE.\n"
	               "Ensure target is still running\n");
        goto EXIT_POINT;
    }

EXIT_POINT:
    return;
} /* end ExtDisconnectRequest */


/* Function: ExtDisconnectRequestNoFinalUpload =================================
 * Abstract:
 *  A request to terminate communication with target has been made.  Notify
 *  the target (if it is up and running).  The connection status will be:
 *  EXTMODE_DISCONNECT_REQUESTED_NO_FINAL_UPLOAD
 */
PRIVATE void ExtDisconnectRequestNoFinalUpload(
    ExternalSim    *ES,
    int_T          nrhs,
    const mxArray  *prhs[])
{
    PktHeader pktHdr;
    int       nSet;
    boolean_T error = EXT_NO_ERROR;

    assert(esGetConnectionStatus(ES) == EXTMODE_DISCONNECT_REQUESTED_NO_FINAL_UPLOAD);

    (void)nrhs; /* unused */
    (void)prhs; /* unused */

    pktHdr.size = 0;
    pktHdr.type = EXT_DISCONNECT_REQUEST_NO_FINAL_UPLOAD; 

    Copy32BitsToTarget(ES,(char *)&pktHdr,(uint32_T *)&pktHdr,NUM_HDR_ELS);
    if (!esIsErrorClear(ES)) goto EXIT_POINT;

    error = ExtSetTargetPkt(ES,sizeof(pktHdr),(char *)&pktHdr,&nSet);
    if (error || (nSet != sizeof(pktHdr))) {
        esSetError(ES, "ExtSetTargetPkt() call failed on CLOSE.\n"
	               "Ensure target is still running\n");
        goto EXIT_POINT;
    }

EXIT_POINT:
    return;
} /* end ExtDisconnectRequestNoFinalUpload */


/* Function: ExtDisconnectConfirmed ============================================
 * Abstract:
 *  Terminate communication with target.  This should only be called after the
 *  target has been notified of the termination (if the target is up and 
 *  running).  The connection status will be: EXTMODE_DISCONNECT_CONFIRMED.
 */
PRIVATE void ExtDisconnectConfirmed(
    ExternalSim    *ES,
    int_T          nrhs,
    const mxArray  *prhs[])
{
    assert(esGetConnectionStatus(ES) == EXTMODE_DISCONNECT_CONFIRMED);

    (void)nrhs; /* unused */
    (void)prhs; /* unused */
    
    ExtCloseConnection(ES);
    FreeAndNullUserData(ES);
    
    return;
} /* end ExtDisconnectConfirmed */


/* Function: ExtSendGenericPkt =================================================
 * Abstract:
 *  Send generic packet to the target.  This function simply passes the
 *  already formatted and packed packet to the target.
 */
PRIVATE void ExtSendGenericPkt(
    ExternalSim    *ES,
    int_T          nrhs,
    const mxArray  *prhs[])
{
	printf("\n---ExtSendGenericPkt---");

    int        nSet;
    PktHeader  pktHdr;
    int        pktSize;
    boolean_T  error = EXT_NO_ERROR;

    (void)nrhs; /* unused */
    (void)prhs; /* unused */

    pktSize =esGetCommBufSize(ES);

    pktHdr.type = (uint32_T)esGetAction(ES);
    pktHdr.size = (uint32_T)(pktSize/esGetHostBytesPerTargetByte(ES));

    Copy32BitsToTarget(ES, (char *)&pktHdr, (uint32_T *)&pktHdr, NUM_HDR_ELS);
    if (!esIsErrorClear(ES))
    	{
    		printf("\n!!Error with Copy32BitsToTarget in ExtSendGenericPkt()");
    		fflush(stdout);
    		goto EXIT_POINT;
    	}

    /*
     * Send packet header.
     */
    error = ExtSetTargetPkt(ES,sizeof(pktHdr),(char *)&pktHdr,&nSet);
    if (error || (nSet != sizeof(pktHdr))) {
    	printf("\nExtSetTargetPkt() call failed for ExtSendGenericPkt()");
    	fflush(stdout);
        esSetError(ES,"ExtSetTargetPkt() call failed for ExtSendGenericPkt().\n"
	              "Ensure target is still running\n");
        goto EXIT_POINT;
    }
    int32_T i=0, n;
    char *buf=esGetCommBuf(ES);
    	while(i<=esGetCommBufSize(ES)){
    		(void)memcpy(&n, buf, sizeof(int32_T));
    		printf("! %d ", n);
    		i+=sizeof(int32_T);
    		buf+=sizeof(int32_T);
    	}
    /*
     * Send packet data - if any.
     */
    if (pktSize > 0) {
        error = ExtSetTargetPkt(ES,pktSize,esGetCommBuf(ES),&nSet);
        if (error || (nSet != pktSize)) {
        	printf("\nExtSetTargetPkt() call failed for ExtSendGenericPkt()");
        	fflush(stdout);
            esSetError(ES,
                "ExtSetTargetPkt() call failed for ExtSendGenericPkt().\n"
	        "Ensure target is still running\n");
            goto EXIT_POINT;
        }
    }

EXIT_POINT:
    return;
} /* end ExtSendGenericPkt */


/* Function: ExtGetParams ======================================================
 * Abstract:
 *  Send request to the target to upload the accessible parameters.  Block
 *  until the parameters arrive.  This is done during the connection process
 *  when inline parameters is 'on'.
 */
PRIVATE void ExtGetParams(
    ExternalSim    *ES,
    int_T          nrhs,
    const mxArray  *prhs[])
{
	printf("\n---ExtGetParams---");
	fflush(stdout);
    int            nSet;
    int            nGot;
    int            bytesToGet;
    PktHeader      pktHdr;
    boolean_T      pending;
    boolean_T      error        = EXT_NO_ERROR;
    const long int timeOutSecs  = 60;
    const long int timeOutUSecs = 0;
    char_T         *buf         = esGetIncomingPktDataBuf(ES);

    /*
     * Instruct target to send parameters.
     */
    pktHdr.type = (uint32_T)esGetAction(ES);
    pktHdr.size = 0;

    assert(pktHdr.type == EXT_GETPARAMS);
    
    Copy32BitsToTarget(ES, (char *)&pktHdr, (uint32_T *)&pktHdr, NUM_HDR_ELS);
    if (!esIsErrorClear(ES)) goto EXIT_POINT;

    error = ExtSetTargetPkt(ES,sizeof(pktHdr),(char *)&pktHdr,&nSet);
    if (error || (nSet != sizeof(pktHdr))) {
        esSetError(ES,"ExtSetTargetPkt() call failed for ExtSendGenericPkt().\n"
	              "Ensure target is still running\n");
        goto EXIT_POINT;
    }

    /*
     * Wait for start of parameters packet - error if timeOut.
     */
    error = ExtTargetPktPending(ES, &pending, timeOutSecs, timeOutUSecs);
    if (error || !pending) {
        esSetError(
            ES, "ExtTargetPktPending() call failed while checking "
                " for target pkt\n");
        goto EXIT_POINT;
    }
    assert(pending);

    /*
     * Get packet header - assume that it's all there.
     */
    error = ExtRecvIncomingPktHeader(ES, &pktHdr);
    if (error) goto EXIT_POINT;

    /*
     * EXT_GETPARAMS_RESPONSE should be set in ext_svr.c
     * clearly then the target and client are not communicating*/

    esSetAction(ES, EXT_GETPARAMS_RESPONSE);
    pktHdr.type=(uint32_T)esGetAction(ES);
    pktHdr.size=100;

    /*
     * Convert size to host format/host bytes & verify packet type.
     */
    Copy32BitsFromTarget(ES,
        (uint32_T *)&pktHdr, (char *)&pktHdr, NUM_HDR_ELS);


    assert(pktHdr.type == EXT_GETPARAMS_RESPONSE);
    if (!esIsErrorClear(ES))
    	{
    		printf("\n!!Error after assert!!");
    		fflush(stdout);
    		goto EXIT_POINT;
    	}
            
    bytesToGet = pktHdr.size * esGetHostBytesPerTargetByte(ES);
    printf("\n bytes to get: %d", bytesToGet);

    /*
     * Get the parameters.
     */
    while(bytesToGet != 0) {
        /*
         * Look for any pending data.  If we ever go more than
         * 'timeOutVal' seconds, bail with an error.
         */
        error = ExtTargetPktPending(ES, &pending, timeOutSecs, timeOutUSecs);
        if (error || !pending) {
        	printf("\n!!Error with checking for target!!");
        	fflush(stdout);
            esSetError(
                ES, "ExtTargetPktPending() call failed while checking "
                    " for target pkt\n");
            goto EXIT_POINT;
        }

        /*
         * Grab the data.
         */
        error = ExtGetTargetPkt(ES,bytesToGet,&nGot,buf);
        if (error) {
        		printf("\n!!Error while checking target pkt's");
        		fflush(stdout);
            esSetError(ES, 
                       "ExtGetTargetPkt() call failed while checking "
                       " target pkt's\n");
            goto EXIT_POINT;
        }

        buf        += nGot;
        bytesToGet -= nGot;
        assert(bytesToGet >= 0);
    }

EXIT_POINT:
    return;
} /* end ExtGetParams */


/*******************************************************************
 * Include ext_main.c, the mex file wrapper containing mexFunction *
 *******************************************************************/
//#include "ext_main.c"


/* [EOF] ext_comm.c */




/* LocalWords:  Hdr recv'd Recv tcp ip extmode recv'ing hdr endian nd pkt's
 */
