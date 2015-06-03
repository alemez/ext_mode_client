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

#include "extsim.h"
#include "extutil.h"
#include "main_incl.h"

//#include "ext_convert.c"
//#include "ext_comm.c"

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
	//esSetError(es, '\0');
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

#ifdef COMMENTOUT
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
	        MyUserData *userData = ExtUserDataCreate();
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
	    MyExtProcessArgs(ES,hostName,arr);
	    if (!esIsErrorClear(ES)) {
	        const char errMsgHeader[] = "Failed to connect to the target. Possible reasons for the failure:";
	        sprintf(failedToConnectErrMsg, "%s\n %s\n Caused by:\n %s", errMsgHeader, failedToConnectCauses, esGetError(ES));
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
	        esSetError(ES, failedToConnectErrMsg);
	        goto EXIT_POINT;
	      }
	        assert(pending);

	        error = ExtRecvIncomingPktHeader(ES, &pktHdr);
	        if (error) goto EXIT_POINT;

	        ProcessConnectResponse1(ES, &pktHdr);
	        if (!esIsErrorClear(ES)) goto EXIT_POINT;

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
	              esSetError(
	                ES, "Timed-out waiting for second connect response packet.\n");
	            goto EXIT_POINT;
	          }
	            assert(pending);

	        error = ExtRecvIncomingPktHeader(ES, &pktHdr);
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
	     //esSetRecvIncomingPktFcn(ES, ExtRecvIncomingPkt);

	    EXIT_POINT:
	       if (!esIsErrorClear(ES)) {
	            ExtCloseConnection(ES);
	            FreeAndNullUserData(ES);
	        }
}/*End MyExtComm */
#endif

/*Function: MyExtProcessArgs=============================================
 * Abstract:
 * 	Adapted version on ExtProcessArgs without nrhs and prhs args
 *
 */
void MyExtProcessArgs(ExternalSim *ES, const char hostName[], const int arr[])
{
	//look in rtiostream_interface.c
}/*end MyExtProcessArgs*/


/* Function: main =======================================================
 * Abstract:
 *    Main entry point for external communications processing.
 *    Hardcoded version
 */
int main(void) {
	puts("!!!Hello World!!!"); /* prints !!!Hello World!!! */

	ExternalSim  *ES;	/*Pointer to ExternalSim struct, to pass as args*/

	int nlhs=-1;	/*Default value to initialize ES*/
	int arr[3];	/*Argument for MyExtConnect*/
	char name[]="HostName"; /*Host name to be passed to MyExtConnect*/

	FILE *rtwPtr;
	const char *fileName="test.txt"; //will of course have to be dynamic

	rtwPtr= fopen(fileName, "r");

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
		printf("This external mex file is used by Simulink in external "
			  "mode\nfor communicating with Code Generation targets "
			  "using interprocess communications.\n");
		fflush(stdout);
	        goto EXIT_POINT;
	    }

	fclose(rtwPtr);

	arr[0]=esGetVerbosity(ES);
	//arr[1]=TCP port value
	arr[2]=esGetConnectTimeout(ES);


	switch(esGetAction(ES)) {

	    case EXT_CONNECT:
	        /* Connect to target. */
	        MyExtConnect(ES, name, arr);
	        if (esGetVerbosity(ES)) {
	        	printf("action: EXT_CONNECT\n");
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
	return EXIT_SUCCESS;
}
