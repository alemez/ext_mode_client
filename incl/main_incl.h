/*
 * main_incl.h
 *
 *  Created on: Jun 1, 2015
 *      Author: Andrea Lemez
 *
 *      Contains Function definitions for use in ext_mode_client.c
 */

#ifndef MAIN_INCL_H_
#define MAIN_INCL_H_


ExternalSim* ExtSimStructDef(FILE* fIn );

void MyExtConnect(ExternalSim *ES, const char hostName[], const int arr[]);
void MyExtProcessArgs(ExternalSim *ES, const char hostName[], const int arr[]);
boolean_T MyExtRecvIncomingPktHeader( ExternalSim *ES, PktHeader   *pktHdr);
void MyFreeAndNullUserData(ExternalSim *ES);
int MyExtUtilCreateRtIOStreamArgs(ExternalSim   *ES,
                        const char hostName[],
                        const int arr[],
                        int * argc,
                        const char * argv[]);


#endif /* MAIN_INCL_H_ */
