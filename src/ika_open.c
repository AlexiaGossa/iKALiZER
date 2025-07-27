
/*
    Liste des fonctions à exporter :
    
*/

//Send a message and wait for result (immediate mode)
DLLEXPORT   long IKA_MessageSend ( DWORD dwMessage, DWORD dwParamA, DWORD dwParamB );
    //OK

//Port a message and return immediately without result (delayed mode)
DLLEXPORT   long IKA_MessagePost ( DWORD dwMessage, DWORD dwParamA, DWORD dwParamB );
    //OK

//Create a pipe handle for embedding messages
DLLEXPORT   long IKA_MessagePipeCreate ( DWORD * pdwHandle );

//Delete a pipe handle
DLLEXPORT   long IKA_MessagePipeDelete ( DWORD dwHandle );

//Add a message to a pipe
DLLEXPORT   long IKA_MessagePipeAdd ( DWORD dwHandle, DWORD dwMessage, DWORD dwParamA, DWORD dwParamB );

//Execute the pipe in immediate mode (like SEND) or delayed mode (like POST) and optionnaly delete the pipe handle
DLLEXPORT   long IKA_MessagePipeExec ( DWORD dwHandle, long lImmediateMode, long lEnableAutoPipeDelete );
    //If posted : dwParamA = dwHandle, bit0(dwParamB) = lEnableAutoPipeDelete
    






