# ext_mode_client
External Mode Client - Simulink
Contributors: Andrea Lemez, Constantin Wiesener
2015

This project runs a stand alone client in the same way as Simulink external mode. The user is able to connect the client to a target, run the model and get and set parameters and signals in the model. Currently works only in eclipse

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
