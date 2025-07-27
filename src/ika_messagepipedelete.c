//  ########################################################################################################
//
//  Delete a message pipe handle
//
DLLEXPORT   long IKA_MessagePipeDelete ( DWORD dwHandle )
{
    DWORD dwDescription;

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
        
    //Delete the handle and allocated memory        
    IKA_HandleDelete ( dwHandle );
    
    return 0;    
}
