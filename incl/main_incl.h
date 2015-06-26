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

/*
 * Function for setting some of the variables in
 * the ExternalSim struct. Gets information from the
 * models rtw file. Returns an ES pointer.
 */
ExternalSim* ExtSimStructDef();

/*
 * Displays the available parameters in the model
 */
void DisplayGetParams(ExternalSim *es);

/*
 * Allows the user to input a parameter value to be changed
 * on the model and sets the pkt on the communication buffer
 */
void UserSetParams(ExternalSim *es);

/*
 * When the client and target connect, updates the initial
 * parameter values
 */
void InitialSetParams(ExternalSim *es, int_T nrhs,const mxArray *prhs[]);


#endif /* MAIN_INCL_H_ */
