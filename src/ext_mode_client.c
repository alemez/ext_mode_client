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
#include "main_incl.h"
#include "ext_convert.c"


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

/*Function: MyExtConnect================================================
 * Abstract:
 * 		Adapted version of ExtConnect (ext_comm.c)
 * 		Establish communication with target
 */
void MyExtConnect(ExternalSim *ES, const char hostName[], const int arr[])
{


}


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
