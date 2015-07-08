# ext_mode_client
External Mode Client - Simulink
Contributors: Andrea Lemez, Constantin Wiesener
2015

This project runs a stand alone client in the same way as Simulink external mode. The user is able to connect the client to a target, run the model and get and set parameters and signals in the model. Currently works only in eclipse

Files:
  ext_mode_client.c : Contains the main method for the program along with functions for starting and running the target
  ext_comm.c        : Host-side, transport-independent external-mode functions
  ext_convert.c     : Utility functions for ext_comm.c
  ext_util.c        : Host-side utility functions for External mode
  rtiostream_interface.c : Provide a host-side communications driver interface for Simulink external mode.
  rtiostram_tcpip.c : Implements both client-side and server-side TCP/IP and UDP/IP communication
  
  ext_convert.h     : Headers for utility functions in ext_util.c.
  ext_share.h       : External mode shared data structures used by the external communication	mex link, the generated                        code, and Simulink
  ext_transport.h   : PUBLIC interface for ext_<mechanism>_transport.c
  extsim.h          : Real-Time Workshop external simulation data structure
  extutil.h         : Utilities such as assert
  main_incl.h       : Contains Function definitions for use in ext_mode_client.c
  matrix.h          : 
  mex.h             : 
  rtiostream_loadlib.h : Header for use with utility functions for loading shared library
  rtiostream_utils.h   : 
  rtiostream.h      : Function prototypes and defines for rtIOStream API
  rtwtypes.h        : Definitions required by Real-Time Workshop generated code
  tmwtypes.h        : Required include file for custom external mode
  version.h

Instructions for use:
  -Import the model executable file and its corresponding modelName.rtw file generated in Simulink
  -Start the target (can be in waiting state)
  -Run the project in eclipse
  -When prompted, enter the model name into the console
  -When prompted, connect the client to the target by entering 0 (zero)
  -To run the model enter 10
  -To get parametrs enter 5
  -To set parameters enter 4, then the desired parameter value
  -To stop the model enter 11
  -To disconnect the client enter 3



Not for commercial use. Research use only.
