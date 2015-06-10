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
#include "matrix.h"
#include "ext_convert.h"
#include "rtiostream_loadlib.h"
#include "rtiostream.h"

#include "ext_comm.c"

/*Function: ExtSimStructDef====================================================
 * Abstract:
 * 		Assigns data found in the file fIn to the appropriate fields in the
 * 		External Sim struct.
 * 		Hardcoded
 * 		Unnecessary?
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

	esSetTargetModelCheckSum(es, 0, 3531395276U);
	esSetTargetModelCheckSum(es, 1, 3407967652U);
	esSetTargetModelCheckSum(es, 2, 1218549062U);
	esSetTargetModelCheckSum(es, 3, 3708142253U);

	esSetAction(es, EXT_DISCONNECT_REQUEST);
	esSetConnectionStatus(es, EXTMODE_DISCONNECT_REQUESTED);

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
#ifdef abc
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
#endif

	//esSetSizeOfTargetDataTypeFcn(es, fcn);
	//esSetSizeOfDataTypeFcn(es, fcn);

	return es;
}



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
	int_T  nrhs;	/*Simply as argument filler. Unused*/
	const mxArray  *prhs[1]; 	/*Simply as argument filler. Unused*/

	int arr[3];	/*Argument for MyExtConnect*/
	char name[]="localHost"; /*Host name to be passed to MyExtConnect*/

	FILE *rtwPtr;
	const char *fileName="test.txt"; //will of course have to be dynamic
	rtwPtr= fopen(fileName, "r");

#ifdef MX
	/*For mxArray*/
	mxArray *prhs[4];
	int ndim=1, dims[]={1};
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
		printf("\n---File opened---");
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
	        esSetMexFuncGateWayFcn(ES,main);
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


	switch(esGetAction(ES)) {

	    case EXT_CONNECT:
	        /* Connect to target. */
	        ExtConnect(ES, nrhs, prhs);
	        if (esGetVerbosity(ES)) {
	        	printf("\naction: EXT_CONNECT\n");
	        	fflush(stdout);
	        }
	        break;

	    case EXT_DISCONNECT_REQUEST:
	            /* Request to terminate communication has been made - notify target. */
	            if (esGetVerbosity(ES))
	            	printf("\naction: EXT_DISCONNECT_REQUEST\n");
	            ExtDisconnectRequest(ES, nrhs, prhs);
	            break;

	     case EXT_DISCONNECT_REQUEST_NO_FINAL_UPLOAD:
	            /* Request to terminate communication has been made - notify target. */
	            if (esGetVerbosity(ES)) {
	                printf("\naction: EXT_DISCONNECT_REQUEST_NO_FINAL_UPLOAD\n");
	            }
	            ExtDisconnectRequestNoFinalUpload(ES, nrhs, prhs);
	            break;

	     case EXT_DISCONNECT_CONFIRMED:
	                /* Terminate communication with target. */
	                if (esGetVerbosity(ES)) printf("action: EXT_DISCONNECT_CONFIRMED\n");
	                ExtDisconnectConfirmed(ES, nrhs, prhs);
	                break;

	     case EXT_SETPARAM:
	                /* Download parameters to be set on target. */
	                if (esGetVerbosity(ES)) printf("action: EXT_SETPARAM\n");
	                ExtSendGenericPkt(ES, nrhs, prhs);
	                break;

	      case EXT_GETPARAMS:
	                /* Upload interfaceable variables from target. */
	                if (esGetVerbosity(ES)) printf("action: EXT_GETPARAMS\n");
	                ExtGetParams(ES, nrhs, prhs);
	                break;

	      case EXT_SELECT_SIGNALS:
	                /* Select signals for data uploading. */
	                if (esGetVerbosity(ES)) printf("action: EXT_SELECT_SIGNALS\n");
	                ExtSendGenericPkt(ES, nrhs, prhs);
	                break;

	      case EXT_SELECT_TRIGGER:
	                /* Select signals for data uploading. */
	                if (esGetVerbosity(ES)) printf("action: EXT_SELECT_TRIGGER\n");
	                ExtSendGenericPkt(ES, nrhs, prhs);
	                break;

	      case EXT_ARM_TRIGGER:
	                /* Select signals for data uploading. */
	                if (esGetVerbosity(ES)) printf("action: EXT_ARM_TRIGGER\n");
	                ExtSendGenericPkt(ES, nrhs, prhs);
	                break;

	      case EXT_CANCEL_LOGGING:
	                /* Send packet to target to cancel the current data logging session. */
	                if (esGetVerbosity(ES)) printf("action: EXT_CANCEL_LOGGING\n");
	                ExtSendGenericPkt(ES, nrhs, prhs);
	                break;

	      case EXT_MODEL_START:
	                /* Start the external simulation. */
	                if (esGetVerbosity(ES)) printf("action: EXT_MODEL_START\n");
	                ExtSendGenericPkt(ES, nrhs, prhs);
	                break;

	      case EXT_MODEL_STOP:
	                /* Stop the external simulation and kill target program. */
	                if (esGetVerbosity(ES)) printf("action: EXT_MODEL_STOP\n");
	                ExtSendGenericPkt(ES, nrhs, prhs);
	                break;

	       case EXT_MODEL_PAUSE:
	                /* Pause the target (internal testing only). */
	                if (esGetVerbosity(ES)) printf("action: EXT_MODEL_PAUSE\n");
	                ExtSendGenericPkt(ES, nrhs, prhs);
	                break;

	       case EXT_MODEL_STEP:
	                /* Run the model for 1 step - if paused (internal
	                   testing only). */
	                if (esGetVerbosity(ES)) printf("action: EXT_MODEL_STEP\n");
	                ExtSendGenericPkt(ES, nrhs, prhs);
	                break;

	       case EXT_MODEL_CONTINUE:
	                /* Run the model for 1 step - if paused (internal
	                   testing only). */
	                if (esGetVerbosity(ES)) printf("action: EXT_MODEL_CONTINUE\n");
	                ExtSendGenericPkt(ES, nrhs, prhs);
	                break;

	       case EXT_GET_TIME:
	                /*
	                 * Request the sim time from the target.
	                 *
	                 * NOTE:
	                 *  Skip verbosity.  There are too many of these packets when
	                 *  auto-updating Simulink's status bar clock.
	                 */
	                /*if (esGetVerbosity(ES)) fprintf("action: EXT_GET_TIME\n");*/
	                ExtSendGenericPkt(ES, nrhs, prhs);
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
