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
#include <pthread.h>
#include <sched.h>
#include <unistd.h>
#include <stdint.h>
#include <math.h>

#include "extsim.h"
#include "extutil.h"
#include "main_incl.h"
#include "matrix.h"
#include "ext_convert.h"
#include "rtiostream_loadlib.h"
#include "rtiostream.h"

#include "ext_comm.c"


#define MAX_PRIO  (sched_get_priority_min(SCHED_FIFO) + 1)
pthread_t statemachine_thread;

void* statemachine(void *es)
{
	short cur_state = 0;
	int_T  nrhs;	/*Simply as argument filler. Unused*/
	const mxArray  *prhs[1]; 	/*Simply as argument filler. Unused*/
	ExternalSim * ES= (ExternalSim *) es;

	nrhs=0; /*Initialize to 0 so that the client connects properly*/
	prhs[0]=NULL; /*not used*/

	while (cur_state!=-1) {
		printf("\n\nPlease enter a state [0..15] or -1 to quit");
		fflush(stdout);
		scanf("%hd", &cur_state);

		switch (cur_state){
		case 0 :
			printf("State 0 (EXT_CONNECT) will be processed\n");
			esSetAction(ES, EXT_CONNECT);
			esSetConnectionStatus(ES, EXTMODE_CONNECTED);
			esClearError(ES);
			ExtConnect(ES, nrhs, prhs);
			InitialSetParams(ES, nrhs, prhs);
			break;
		case 1 :
			printf("State 1 (EXT_DISCONNECT_REQUEST) will be processed\n");
			esSetAction(ES, EXT_DISCONNECT_REQUEST);
			esSetConnectionStatus(ES, EXTMODE_DISCONNECT_REQUESTED);
            ExtDisconnectRequest(ES, nrhs, prhs);
			break;
		case 2 :
			printf("State 2 (EXT_DISCONNECT_REQUEST_NO_FINAL_UPLOAD) will be processed\n");
			esSetAction(ES, EXT_DISCONNECT_REQUEST_NO_FINAL_UPLOAD);
			esSetConnectionStatus(ES, EXTMODE_DISCONNECT_REQUESTED_NO_FINAL_UPLOAD);
            ExtDisconnectRequestNoFinalUpload(ES, nrhs, prhs);
			break;
		case 3 :
			printf("State 3 (EXT_DISCONNECT_CONFIRMED) will be processed\n");
			esSetAction(ES, EXT_DISCONNECT_CONFIRMED);
			esSetConnectionStatus(ES, EXTMODE_DISCONNECT_CONFIRMED );
            ExtDisconnectConfirmed(ES, nrhs, prhs);
			break;
		case 4 :
			printf("State 4 (EXT_SETPARAM) will be processed\n");
			esSetAction(ES, EXT_SETPARAM);
            UserSetParams(ES);
            ExtSendGenericPkt(ES, nrhs, prhs);
			break;
		case 5 :
			printf("State 5 (EXT_GETPARAMS) will be processed\n");
			esSetAction(ES, EXT_GETPARAMS);
			ExtGetParams(ES, nrhs, prhs);
			DisplayGetParams(ES);
			break;
		case 6 :
			printf("State 6 (EXT_SELECT_SIGNALS) will be processed\n");
			esSetAction(ES, EXT_SELECT_SIGNALS);
            ExtSendGenericPkt(ES, nrhs, prhs);
			break;
		case 7 :
			printf("State 7 (EXT_SELECT_TRIGGER) will be processed\n");
			esSetAction(ES, EXT_SELECT_TRIGGER);
            ExtSendGenericPkt(ES, nrhs, prhs);
			break;
		case 8 :
			printf("State 8 (EXT_ARM_TRIGGER) will be processed\n");
			esSetAction(ES, EXT_ARM_TRIGGER);
            ExtSendGenericPkt(ES, nrhs, prhs);
			break;
		case 9 :
			printf("State 9 (EXT_CANCEL_LOGGING) will be processed\n");
			esSetAction(ES, EXT_CANCEL_LOGGING);
            ExtSendGenericPkt(ES, nrhs, prhs);
			break;
		case 10 :
			printf("State 10(EXT_MODEL_START) will be processed\n");
			esSetAction(ES, EXT_MODEL_START);
            ExtSendGenericPkt(ES, nrhs, prhs);
			break;
		case 11 :
			printf("State 11 (EXT_MODEL_STOP) will be processed\n");
			esSetAction(ES, EXT_MODEL_STOP);
            ExtSendGenericPkt(ES, nrhs, prhs);
			break;
		case 12 :
			printf("State 12 (EXT_MODEL_PAUSE) will be processed\n");
			esSetAction(ES, EXT_MODEL_PAUSE);
            ExtSendGenericPkt(ES, nrhs, prhs);
			break;
		case 13 :
			printf("State 13 (EXT_MODEL_STEP) will be processed\n");
			esSetAction(ES, EXT_MODEL_STEP);
            ExtSendGenericPkt(ES, nrhs, prhs);
			break;
		case 14 :
			printf("State 14 (EXT_MODEL_CONTINUE) will be processed\n");
			esSetAction(ES, EXT_MODEL_CONTINUE);
            ExtSendGenericPkt(ES, nrhs, prhs);
			break;
		case 15 :
			printf("State 15 (EXT_GET_TIME) will be processed\n");
			esSetAction(ES, EXT_GET_TIME);
            ExtSendGenericPkt(ES, nrhs, prhs);
			break;
		}

	}
	pthread_exit(0);
	return 0;
}


/*Function: ExtSimStructDef====================================================
 * Abstract:
 * 		Assigns certain data found in the file rtw to the appropriate fields in the
 * 		External Sim struct.
 *
 */
ExternalSim* ExtSimStructDef()
{
	printf("\n---Building External Sim struct---");
	fflush(stdout);

	ExternalSim *es=malloc(sizeof(ExternalSim));
	FILE * rtw;
	char modelName[100];
	char fileName[100];
	char line[256]; /*Line pointer to read in from file*/
	//int numDataTypes, numDataTypesLine=612;
	int i, count=0;

	for(i=0; i<100; i++)
		modelName[i]='\0';

	FILENAME:
		printf("\n\nPlease enter the model name (ex: test): ");
		scanf("%s", modelName);

		strcpy(fileName, modelName);
		strcat(fileName, ".txt");

		rtw=fopen(fileName, "r");

		if(rtw==NULL){
			printf("\nCouldn't open file! Please try again.");
			goto FILENAME;
		}

#ifdef HARDCODE
		i=0;
		while(fgets(line, sizeof(line), rtw)!=NULL)
		{
			if(count==numDataTypesLine-1)
			{
				printf("\n!! %s", line);
				while(!isdigit(line[i]))
					i++;
				numDataTypes=line[i]-48; /*From ASCII char code to num*/

				if(isdigit(line[i++]))
				{
					numDataTypes*=10;
					numDataTypes= numDataTypes + (line[i]-48);
				}
				esSetNumDataTypes(es, numDataTypes);	/*line 612 of test.rtw*/
				break;
			}
			else
				count++;
		}
#endif

	esSetModelName(es, modelName); /*given by user*/
	esSetNumDataTypes(es, 14);	/*line 612 of test.rtw*/


	/*Assuming thus far that these are the same for every model.*/
	esSetVersion(es, (sizeof(ExternalSim)*10000 + 200));	/*EXTSIM_VERSION*/
	esSetHostMWChunkSize(es, 256); 	/*ERTMultiwordLength?*/
	esClearError(es);		/*Set error to null to begin*/

	esSetIncomingPktDataNBytesNeeded(es, -1);
	esSetIncomingPktDataNBytesInBuf (es, 0 );

	/*-----------Unnecessary?!? ----------------------*/
	//esSetBdPtr(es, val);	/*pointer*/
	//esSetSizeOfTargetDataTypeFcn(es, fcn);
	//esSetSizeOfDataTypeFcn(es, fcn);

	fclose(rtw);
	return es;
}

/*Function: DisplayGetParams================================================
 * Abstract: Displays the parameter identifiers available in
 * 			 the model to the user
 */
void DisplayGetParams(ExternalSim *es)
{
	const char *pkt=esGetIncomingPktDataBuf(es);
	int32_T    nParams;
	int i;
	for(i=0; i<esGetIncomingPktDataBufSize(es); i++){
	(void)memcpy(&nParams, pkt, sizeof(int32_T));
	printf("\n!! i:%d ,  %d", i, nParams);
	pkt += sizeof(int32_T);
	}
}

/*Function: UserSetParams===================================================
 * Abstract: Allows the user to change certain parameters,
 * 			sets the commBuf to the appropriate pkt
 */
void UserSetParams(ExternalSim *es)
{
	int64_T param;	//for conversion to built in data type
	double DType; //User input
	int32_T temp, temp2;
	/*The array is as follows:
	 * tmp[0]= number of parameters being changed
	 * tmp[1]= data type transition index (B in ext_svr.c)
	 * tmp[2]= starting offset data (S in ext_svr.c)
	 * tmp[3]= number of elements for this parameter (W in ext_svr.c)
	 * tmp[4]= index into rtw data type table (DI in ext_svr.c)
	 * tmp[5]=0
	 * tmp[6]= the parameter value (in TARGET format)
	 * tmp[7]=0
	 */
	int32_T tmpArr[]={1,0,4,1,0,0,1074266112,0};

	/*Will have a menu or something that displays parameters
	 * available to be changed. Maybe a key with integers
	 * matching each block
	printf("Please select the parameter you would like to change: ");
	scanf("%d", &select);
	tmp[2]=select;
	 */

	printf("\n\nPlease enter a sample time parameter value: ");
	scanf("%lf", &DType);

	/*
	 * Convert user input param value from double to
	 * 2 32-bit double precision integers to send in the pkt
	 * 		-temp will almost always be zero (upper 32 bits)
	 * 		-temp2 will probably contain all the bits needed for the
	 * 		 correct precision (lower 32 bits)
	 */
	(void)memcpy(&param, &DType, 32);
	temp=param;
	temp2= param >> 32;

	tmpArr[0]=1;	//updating 1 parameter
	tmpArr[1]=0;
	tmpArr[6]=temp2;

	esSetCommBuf(es, NULL);
	//free(esGetCommBuf(es));

	char* pkt= malloc(sizeof(int32_T)*8);
	(void)memcpy(pkt, &tmpArr, sizeof(int32_T)*8);

	  esSetCommBufSize(es, sizeof(tmpArr));
	  esSetCommBuf(es, pkt);
}

void InitialSetParams(ExternalSim* es, int_T nrhs, const mxArray *prhs[])
{
	int32_T pktArr[]={5,0,0,1,0,0,1072693248,0,
						1,1,0,0,1076101120, 0,2,
						1,0,0,1075052544,0,3,1
						,0,0,0,0,4,1,0,0,1074266112,0};

	char* pkt= malloc(sizeof(int32_T)*32);
	(void)memcpy(pkt, &pktArr, sizeof(int32_T)*32);

	esSetCommBufSize(es, sizeof(pktArr));
	esSetCommBuf(es, pkt);

	esSetAction(es, EXT_SETPARAM);
	ExtSendGenericPkt(es, nrhs, prhs);

	/*
	 * For some reason this is needed or the model will
	 * run with bugs and you wont be able to change params after
	 * running the model
	 */
	char* pkt2= malloc(sizeof(int32_T)*8);
	(void)memcpy(pkt, &pktArr[9], sizeof(int32_T)*32);
	esSetCommBufSize(es, sizeof(int32_T)*8);
	esSetCommBuf(es, pkt2);

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

	char *buf= (char *)malloc(500); /*The communication buffer*/
	char *incomingBuf= (char *) malloc(150); /*The incoming packet buffer */

	if(buf==NULL)
	{
		printf("Error allocating memory for buffer");
		fflush(stdout);
	}

		 ES=ExtSimStructDef();
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


	 esSetCommBuf(ES, buf);
	 esSetCommBufSize(ES, 124);	/* size of communication buffer - in host bytes */
	 ES->TargetDataInfo.dataTypeSizes=malloc(esGetNumDataTypes(ES)*sizeof(uint32_T));

	 esSetIncomingPktDataBufSize(ES, 500);
	 esSetIncomingPktDataBuf(ES, incomingBuf); /*The buffer for incoming packets*/

	 printf("\n!!!Starting the state machine!!!\n");

	pthread_attr_t attr;
	struct sched_param sched_param;


	// Prepare task attributes
	pthread_attr_init(&attr);
	//pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
	//pthread_attr_setschedpolicy(&attr, SCHED_FIFO);

	// Starting the base rate thread
	//sched_param.sched_priority = MAX_PRIO;
	//pthread_attr_setschedparam(&attr, &sched_param);
	pthread_create(&statemachine_thread, &attr, statemachine, ES);
	pthread_attr_destroy(&attr);

	// External mode
	while (1) {}

	pthread_join(statemachine_thread, NULL);

#ifdef SWITCH
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
#endif

	EXIT_POINT:

	free(buf);
	free(incomingBuf);
	free(ES->DTypeFcn.sizeOfDataType);
	free(ES);

	printf("\n---!!!Exit Success!!!---");
	fflush(stdout);
	return EXIT_SUCCESS;
}
