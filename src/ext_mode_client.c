/*
 ============================================================================
 Name        : ext_mode_client.c
 Author      : Andrea Lemez
 Version     : 1.0
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>

//#include "extsim.h"
#include "extutil.h"
#include "main_incl.h"
#include "matrix.h"
#include "ext_convert.h"
#include "rtiostream_loadlib.h"

#include "ext_comm.c"

/*Function: ExtSimStructDef====================================================
 * Abstract:
 * 		Assigns data found in the file fIn to the appropriate fields in the
 * 		External Sim struct.
 * 		Hardcoded
 */
ExternalSim* ExtSimStructDef(FILE* fIn )
{
	printf("\n---Building External Sim struct---");
	fflush(stdout);

	ExternalSim *es=malloc(sizeof *es);

	esSetVersion(es, (sizeof(ExternalSim)*10000 + 200));	/*EXTSIM_VERSION*/
	esSetModelName(es, "test");
	//esSetBdPtr(es, val);	/*pointer*/
	esSetTargetSimStatus(es, TARGET_STATUS_WAITING_TO_START);
	esSetHostMWChunkSize(es, 256); 	/*ERTMultiwordLength?*/
	esSetConnectionStatus(es, EXTMODE_CONNECTED);
	esSetTargetModelCheckSum(es, 0, 3531395276U);
	esSetTargetModelCheckSum(es, 1, 3407967652U);
	esSetTargetModelCheckSum(es, 2, 1218549062U);
	esSetTargetModelCheckSum(es, 3, 3708142253U);
	esSetAction(es, EXT_CONNECT);
	esSetVerbosity(es, 1);	/*Verbose build chosen*/
	esSetConnectTimeout(es, 120); /*DEFAULT_CONNECT_TIMEOUT_SECS*/
	//esSetError(es, 0);

	//esSetUserData(es, val); /*pointer*/
	//esSetCommBuf(es, val);	/*pointer*/
	//esSetCommBufSize(es, val);	/* size of communication buffer - in host bytes */
	//esSetIncomingPktPending(es, val);
	esSetIncomingPkt(es, EXT_CONNECT_RESPONSE);
	//esSetIncomingPktDataBufSize(es, val);
	//esSetIncomingPktDataBuf(es, val);
	//esSetIncomingPktDataNBytesInBuf(es, val);
	//esSetIncomingPktDataNBytesNeeded(es, val);
	//esSetRecvIncomingPktFcn(es, fcn);
	esSetIntOnly(es, 0);		/*?? I guessed- affects below functions*/
	//esSetSwapBytes(es, val);
	//esSetNumDataTypes(es, val)
	//esSetHostBytesPerTargetByte(es, val);
	//esSetDataTypeSize(es, idx, val);
	//esSetTargetMWChunkSize(es, val);
	esSetDoubleTargetToHostFcn(es, Double_TargetToHost);/*ext_convert.c depends if int only or not*/
	esSetDoubleHostToTargetFcn(es, Double_HostToTarget);
	esSetSingleTargetToHostFcn(es, Generic_TargetToHost);
	esSetSingleHostToTargetFcn(es, Generic_HostToTarget);
	esSetInt8TargetToHostFcn(es, Generic_TargetToHost);
	esSetInt8HostToTargetFcn(es, Generic_HostToTarget);
	esSetUInt8TargetToHostFcn(es, Generic_TargetToHost);
	esSetUInt8HostToTargetFcn(es, Generic_HostToTarget);
	esSetInt16TargetToHostFcn(es, Generic_TargetToHost);
	esSetInt16HostToTargetFcn(es, Generic_HostToTarget);
	esSetUInt16TargetToHostFcn(es, Generic_TargetToHost);
	esSetUInt16HostToTargetFcn(es, Generic_HostToTarget);
	esSetInt32TargetToHostFcn(es, Generic_TargetToHost);
	esSetInt32HostToTargetFcn(es, Generic_HostToTarget);
	esSetUInt32TargetToHostFcn(es, Generic_TargetToHost);
	esSetUInt32HostToTargetFcn(es, Generic_HostToTarget);
	esSetInt33PlusTargetToHostFcn(es, Int33Plus_TargetToHost);
	esSetInt33PlusHostToTargetFcn(es, Int33Plus_HostToTarget);
	esSetBoolTargetToHostFcn(es, Bool_TargetToHost);
	esSetBoolHostToTargetFcn(es, Bool_HostToTarget);

	//esSetSizeOfTargetDataTypeFcn(es, fcn);
	//esSetSizeOfDataTypeFcn(es, fcn);

	return es;
}

/*
 * From rtiostram_interface.c
 * Used in function MyExtConnect
 */
typedef struct UserData_tag {
    RtIOStreamData rtiostreamData;
} MyUserData;

/*Function: MyExtConnect================================================
 * Abstract:
 * 		Adapted version of ExtConnect (ext_comm.c)
 * 		Establish communication with target
 */
void MyExtConnect(ExternalSim *ES, const char hostName[], const int arr[])
{
	printf("\n---MyExtConnect---");
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
	            "c) The application for the model is not running on the target. "
	            "You might have clicked the Stop button.");
	fflush(stdout);

	    {
	        UserData *userData = ExtUserDataCreate();
	        if (userData == NULL) {
	            esSetError(ES, "Memory allocation error.");
	            goto EXIT_POINT;
	        }

	        esSetUserData(ES, (void*)userData);
	    }

	    /*
	     * Parse the arguments.
	     */
	    assert(esIsErrorClear(ES));
	    //Problem here
	    MyExtProcessArgs(ES,hostName,arr);

	    if (!esIsErrorClear(ES)) {
	        const char errMsgHeader[] = "Failed to connect to the target. Possible reasons for the failure:";
	        sprintf(failedToConnectErrMsg, "%s\n %s\n Caused by:\n %s", errMsgHeader, failedToConnectCauses, esGetError(ES));
	        fflush(stdout);
	        esSetError(ES, failedToConnectErrMsg);
	        goto EXIT_POINT;
	   }


	    assert(esIsErrorClear(ES));
	    timeOutSecs  =  (long int) esGetConnectTimeout(ES);

	    ExtOpenConnection(ES);
	    if (!esIsErrorClear(ES)) goto EXIT_POINT;

	    /*
	     * Send the EXT_CONNECT pkt to the target.  This packet consists
	     * of the string 'ext-mode'.  The purpose of this header is to serve
	     * as a flag to start the handshaking process.
	     */
	    (void)memcpy((void *)&pktHdr,"ext-mode",8);
	    error = ExtSetTargetPkt(ES,sizeof(pktHdr),(char *)&pktHdr,&nSet);
	    if (error || (nSet != sizeof(pktHdr))) {
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
	        const char errMsgHeader[] = "Failed to connect to the target. A time-out occurred while "
	                "waiting for the first connect response packet. Possible reasons for the time-out:";
	        sprintf(failedToConnectErrMsg, "%s\n%s", errMsgHeader, failedToConnectCauses);
	       fflush(stdout);
	        esSetError(ES, failedToConnectErrMsg);
	        goto EXIT_POINT;
	      }
	        assert(pending);

	        error = MyExtRecvIncomingPktHeader(ES, &pktHdr);
	        if (error) goto EXIT_POINT;

	        ProcessConnectResponse1(ES, &pktHdr);
	        if (!esIsErrorClear(ES)) goto EXIT_POINT;

	        if (esGetVerbosity(ES)) {
	           MachByteOrder hostEndian =
	                (*((int8_T *) &one) == 1) ? LittleEndian : BigEndian;

	          printf("\nhost endian mode:\t%s\n",
	      	  (hostEndian == LittleEndian) ? "Little":"Big");
	          fflush(stdout);

	          printf("\nbyte swapping required:\t%s\n",
	             (esGetSwapBytes(ES)) ? "true":"false");
	          fflush(stdout);
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
	              esSetError(
	                ES, "Timed-out waiting for second connect response packet.\n");
	            goto EXIT_POINT;
	          }
	            assert(pending);

	        error = MyExtRecvIncomingPktHeader(ES, &pktHdr);
	        if (error) goto EXIT_POINT;

	        Copy32BitsFromTarget(ES, (uint32_T *)&pktHdr, (char *)&pktHdr, NUM_HDR_ELS);
	        if (!esIsErrorClear(ES)) goto EXIT_POINT;

	        if (pktHdr.type != EXT_CONNECT_RESPONSE) {
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
	                    printf("\ntarget integer only code:\t%s\n",
	                              (esGetIntOnly(ES)) ? "true":"false");
	      	          fflush(stdout);
	                }

	                /* process multiword data type chunk size */
	                esSetTargetMWChunkSize(ES, *bufPtr++);

	                if (esGetVerbosity(ES)) {
	                    printf("\ntarget multiword chunk size:\t%d bytes\n",
	                              (esGetTargetMWChunkSize(ES)));
	      	          fflush(stdout);
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
	     //esSetRecvIncomingPktFcn(ES, ExtRecvIncomingPkt);

	    EXIT_POINT:
	       if (!esIsErrorClear(ES)) {
	            ExtCloseConnection(ES);
	            MyFreeAndNullUserData(ES);
	        }
}/*End MyExtComm */


/*Function: MyExtProcessArgs=============================================
 * Abstract:
 * 	Adapted version on ExtProcessArgs without nrhs and prhs args
 * 	from rtiostream_interface.c
 *
 */
void MyExtProcessArgs(ExternalSim *ES, const char hostName[], const int arr[])
{
	printf("\n---MyExtProcessArgs---");
	fflush(stdout);
    int argc = 0;
    const char * argv[ARGC_MAX] = ARGV_INIT;
    UserData  * const userData = (UserData *)esGetUserData(ES);
    int errorOccurred = 0;

    errorOccurred = MyExtUtilCreateRtIOStreamArgs(ES, hostName, arr, &argc, argv);

    if (errorOccurred) {
       return;
    }

    assert(argc <= ARGC_MAX);

    {
        /* Load the rtIOStream shared library */
        int error;
        error = ExtUtilLoadSharedLib(&userData->rtiostreamData);
        if (error != 0) {
        	fprintf(stderr,"\n---Error loading rtIOStream shared library---");
        	fflush(stdout);
            /* Error out immediately */
//            mexErrMsgIdAndTxt("rtiostream_interface:ExtProcessArgs:"
//                              "LoadSharedLibFailure",
//                              "Error loading rtIOStream shared library; "
//                              "see command window for details.");

        } else {
        	/* Call rtIOStreamOpen */
            userData->rtiostreamData.streamID = ( *(userData->rtiostreamData.libH.openFn) ) (argc, (void *)argv);
            printf("\nrtiostreamData: %d", userData->rtiostreamData.streamID);
            	    fflush(stdout);
            if (userData->rtiostreamData.streamID == -1) {
                const char msg[] =
                    "An error occurred attempting to open an rtIOStream. More detail "
                    "may be reported in the MATLAB command window\n";
                esSetError(ES, msg);
            }
        }
    }

}/*end MyExtProcessArgs*/

/*Function: MyFreeAndNullUserData===========================================
 * Abstract:
 * 	version of FreeAndNullUserData (ext_comm.c)
 *
 */
void MyFreeAndNullUserData(ExternalSim *ES)
{
    ExtUserDataDestroy(esGetUserData(ES));
    esSetUserData(ES, NULL);
} /* end FreeAndNullUserData */

/*Function: MyExtRecvIncomingPktHeader======================================
 * Abstract:
 * 	version of ExtRecvIncomingPktHeader from ext_comm.c *
 */
boolean_T MyExtRecvIncomingPktHeader( ExternalSim *ES, PktHeader   *pktHdr)
{
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
        boolean_T pending = 1; //true

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


/*Function: MyExtUtilCreateRTIOStreamArgs
 * Abstract:
 * 		adapted from ExtUtilCreateRTIOStreamArgs (ext_util.c)
 * 		Process arguments required for TCP/IP communications
 *
 */
int MyExtUtilCreateRtIOStreamArgs(ExternalSim   *ES,
                        const char hostName[],
                        const int arr[],
                        int * argc,
                        const char * argv[])
{

	   int errorOccurred = 0;
	   /* initialize connect timeout value, may get overridden by user specified mex file fourth argument */
	   int connectTimeOutSecs = 120; //DEFAULT_CONNECT_TIMEOUT_SECS =120

	   /* ... Argument 1 - host name */
	   if(strlen(hostName)>1 ){ /*maybe add error checking for chars*/
	      /* host name specified */
	      char * argValue = 0;
	      //errorOccurred = ExtUtilProcessTCPIPHostArg(ES, arr[0], &argValue);
	         argv[(*argc)++] = "-hostname";
	         argv[(*argc)++] = argValue;
	   }
	   else{
		   errorOccurred=1;
	        return errorOccurred;
	   }


	   /* ... Argument 2 - verbosity */
	   if( arr[0]==0 || arr[0]==1) {
	      int argValue = 0;
	      //errorOccurred = ExtUtilProcessVerboseArg(ES, arr[0], &argValue);
	         esSetVerbosity(ES, argValue); /*lint !e734 loss of precision 31 bits to 8 bits*/
	   }
	   else{
		   	   errorOccurred=1;
	 	        return errorOccurred;
	 	   }


	   /* ... Argument 3 - TCP port value */
	   if(arr[1]>=256 && arr[1]<=65535){
		   char * argValue = 0;
	      //errorOccurred = ExtUtilProcessTCPIPPortArg(ES, arr[1], &argValue);
	         argv[(*argc)++] = "-port";
	         argv[(*argc)++] = argValue;
	   }
	   else{
		   	   errorOccurred=1;
	 	        return errorOccurred;
	 	   }

	   /* ... Argument 4 - connection timeout */
	   if(arr[2]<0 ){ //add error checking for real scalar int
		   errorOccurred=1;
		   return errorOccurred;
	       //errorOccurred = ExtUtilProcessConnectTimeoutArg(ES, arr[2], &connectTimeOutSecs);
	   }

	   esSetConnectTimeout(ES, connectTimeOutSecs); /*lint !e734 loss of precision 31 bits to 8 bits*/

	   /* ... Argument 5 - open rtIOStream as TCP/IP client */
	   argv[(*argc)++] = "-client";
	   argv[(*argc)++] = "1";

	   return errorOccurred;

}/*end MyExtUtilCreateRTIOStreamArgs*/


/* Function: main =======================================================
 * Abstract:
 *    Main entry point for external communications processing.
 *    Hardcoded version
 */
int main(void) {

	printf("---Starting main---");
	fflush(stdout);

	ExternalSim  *ES;	/*Pointer to ExternalSim struct, to pass as args*/

	int nlhs=-1;	/*Default value to initialize ES*/
	int arr[3];	/*Argument for MyExtConnect*/
	char name[]="HostName"; /*Host name to be passed to MyExtConnect*/
	int ndim=1, dims[]={1};

	FILE *rtwPtr;
	const char *fileName="test.txt"; //will of course have to be dynamic
	rtwPtr= fopen(fileName, "r");

#ifdef MX
	/*For mxArray*/
	mxArray *prhs[4];
	mxChar *orig_ptr, *ptr;
	char *data_ptr;
	int i;
	int *verbos;//=1;
	int *tcp_port;//=17725;
	int *timeout;//=120;

	verbos=mxCalloc(1, sizeof(int));
	tcp_port=mxCalloc(1, sizeof(int));
	timeout=mxCalloc(1, sizeof(int));

	verbos[0]=1;
	tcp_port[0]=17725;
	timeout[0]=120;

	prhs[0]=mxCreateCharArray(ndim, dims);

	orig_ptr=(mxChar *)mxGetPr(prhs[0]);

	for(i=0; i<strlen(name); i++){
		ptr=orig_ptr+i;
		data_ptr=name[i];
	}

	prhs[1]=mxCreateNumericArray(ndim, dims, mxINT8_CLASS, mxREAL);
	prhs[2]=mxCreateNumericArray(ndim, dims, mxINT32_CLASS, mxREAL);
	prhs[3]=mxCreateNumericArray(ndim, dims, mxINT32_CLASS, mxREAL);

	mxSetData(prhs[1], verbos); //set data being problematic....
	mxSetData(prhs[2], tcp_port);
	mxSetData(prhs[3], timeout);
	printf("---here 2---");
			fflush(stdout);
#endif

	if(rtwPtr==NULL)
	{
		fprintf(stderr, "Cannot open the file!\n");
		fflush(stdout);
		exit(1);
	}
	else{
		printf("---File opened---");
		fflush(stdout);
	}


	 if (nlhs == -1) {
		 ES=ExtSimStructDef(rtwPtr );
	        if (((int *)ES)[0] != EXTSIM_VERSION) {
	        	fprintf(stderr,"\nError with EXTSIM_VERSION...Exiting...");
	        	fflush(stdout);
	            esSetError(ES,
	                "\nThe version of the 'ExternalSim' structure passed by "
	                "Simulink and the version of the structure used by ext_main.c "
	                "are not consistent.  The mex file is either out of date or "
	                "built with structure alignment not equal to 8.  Ensure that "
	                "the external mode mex file was built using the include file: "
	                "<matlabroot>/toolbox/coder/simulinkcoder_core/ext_mode/host/common/extsim.h.\n");
	            goto EXIT_POINT;
	        }
	        /*
	         * Provide simulink with a direct pointer to this function so that
	         * subsequent calls can be made more efficiently.
	         */
	        //esSetMexFuncGateWayFcn(ES,main);
	    } else {
		printf("\nThis external mex file is used by Simulink in external "
			  "mode\nfor communicating with Code Generation targets "
			  "using interprocess communications.\n");
		fflush(stdout);
	        goto EXIT_POINT;
	    }

	fclose(rtwPtr);

	arr[0]=esGetVerbosity(ES);
	arr[1]=17725; //TCP port value
	arr[2]=esGetConnectTimeout(ES);

	printf("\n---Switch case:---");
    fflush(stdout);

	switch(esGetAction(ES)) {

	    case EXT_CONNECT:
	        /* Connect to target. */
	    	printf("\n---EXT_CONNECT---");
	    	fflush(stdout);
	        MyExtConnect(ES, name, arr);
	        if (esGetVerbosity(ES)) {
	        	printf("\naction: EXT_CONNECT\n");
	        	fflush(stdout);
	        }
	        break;

	    default:
	            esSetError(ES,"\nUnrecognized external communication action.");
	            printf("\n%s", esGetError(ES));
	            fflush(stdout);
	            goto EXIT_POINT;
	        } /* end switch */



	EXIT_POINT:
	printf("\n---!!!Exit Success!!!---");
	fflush(stdout);
	return EXIT_SUCCESS;
}
