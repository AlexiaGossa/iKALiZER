//  ########################################################################################################
//
//  Add a message to a message pipe handle
//
DLLEXPORT   long IKA_MessagePipeAdd ( DWORD dwHandle, DWORD dwMessage, DWORD dwParamA, DWORD dwParamB )
{
    DWORD               dwDescription;
    IKA_MESSAGEPIPE     *pMessagePipe;
    IKA_MESSAGE         *pMessage;

    //Handle exists ?    
    if (IKA_HandleExist(dwHandle,NULL))
        return -1;

    //Handle is a message pipe ?        
    dwDescription = 0;
    IKA_HandleDataGet ( 
        dwHandle,
        &dwDescription,
        NULL,
        NULL );
    if (dwDescription!=IKA_HANDLE_DESC_MESSAGEPIPE)
        return -1;

    //Get memory location
    IKA_HandleRawGet ( 
        dwHandle, 
        &pMessagePipe );
    if (pMessagePipe==NULL)
        return -1;
        
    //Add a new message
    pMessagePipe->dwMessagePipeCount+=1;
    
    //Re-allocation
    IKA_HandleRawAlloc ( 
        dwHandle, 
        sizeof(IKA_MESSAGEPIPE)+(sizeof(IKA_MESSAGE)*(pMessagePipe->dwMessagePipeCount)) );

    //Get the new area
    IKA_HandleRawGet ( 
        dwHandle, 
        &pMessagePipe );

    //Add the message        
    pMessage = &(pMessagePipe->pMessage[pMessagePipe->dwMessagePipeCount-1]);
    pMessage->dwMessage = dwMessage;
    pMessage->dwParamA  = dwParamA;
    pMessage->dwParamB  = dwParamB;
    pMessage->dwFlags   = IKA_MESSAGEFLAGS_MESSAGE | IKA_MESSAGEFLAGS_PARAMA | IKA_MESSAGEFLAGS_PARAMB;
        
    return 0;
}