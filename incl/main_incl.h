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


#endif /* MAIN_INCL_H_ */
