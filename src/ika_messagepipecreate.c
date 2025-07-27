

//  ########################################################################################################
//
//  Create a message pipe handle
//
DLLEXPORT   long IKA_MessagePipeCreate ( DWORD * pdwHandle )
{
    DWORD               dwHandle;
    IKA_MESSAGEPIPE     *pMessagePipe;

    //Error, invalid param    
    if (!pdwHandle)
        return -1;
    
    //Create a new handle
    lReturn = IKA_HandleCreate ( &dwHandle );
    if (lReturn)
        return lReturn;
        
    //This handle is a pipe !
    IKA_HandleDataSet ( 
        dwHandle,
        IKA_HANDLE_DESC_MESSAGEPIPE,
        NULL,
        NULL );
    
    //Alloc memory of this pipe
    IKA_HandleRawAlloc ( 
        dwHandle, 
        sizeof(IKA_MESSAGEPIPE) );
        
    //Get memory location
    IKA_HandleRawGet ( 
        dwHandle, 
        &pMessagePipe );
    
    //Set pipe message count to ZERO
    pMessagePipe->dwMessagePipeCount = 0;
    
    *pdwHandle = dwHandle;
    return 0;
}
