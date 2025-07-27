#include "sam_header.h"
#include "sam_data.h"
#include "ika_message.h"
#include "ika_handle.h"


typedef struct {
    DWORD               dwHandle;
    
    IKA_MESSAGE         *pMessage;
   
    DWORD               dwBaseIndex;
    DWORD               dwTotalCount;
    DWORD               dwUsedCount;
    
    CRITICALSECTION     csHandle;
    
} IKA_MESSAGESTACK;

IKA_MESSAGESTACK ikaMessageStack;

long IKA_MessageStackOpen ( void )
{
    //Use a handle for Message Stack
    IKA_HandleCreate ( &ikaMessageStack.dwHandle );
    
    //Use a Critical Section
    _OSI_InitializeCriticalSection ( &ikaMessageStack.csHandle );
    
    //Init base value
    ikaMessageStack.dwBaseIndex     = 0;
    ikaMessageStack.dwTotalCount    = 1024;
    ikaMessageStack.dwUsedCount     = 0;

    //This handle is a pipe !
    IKA_HandleDataSet ( 
        ikaMessageStack.dwHandle,
        IKA_HANDLE_DESC_MESSAGESTACK,
        NULL,
        NULL );
    
    //Alloc memory for this stack
    IKA_HandleRawAlloc ( 
        ikaMessageStack.dwHandle, 
        sizeof(IKA_MESSAGE)*ikaMessageStack.dwTotalCount );
        
    //Get memory location
    IKA_HandleRawGet ( 
        ikaMessageStack.dwHandle, 
        &(ikaMessageStack.pMessage) );
    
    return 0;
}

long IKA_MessageStackClose ( void )
{
    DWORD dwIndex;
    IKA_MESSAGE *pMessage;
    
    _OSI_EnterCriticalSection ( &ikaMessageStack.csHandle );

    //Get memory location
    IKA_HandleRawGet ( 
        ikaMessageStack.dwHandle, 
        &pMessage );
    
    for (dwIndex=0;dwIndex<ikaMessageStack.dwTotalCount;dwIndex++)
    {
        if (pMessage->dwFlags)
        {
            if (pMessage->dwMessage==IKA_MESSAGE_PIPEEXEC)
            {
                if ((pMessage->dwParamB)&1) //Enable autodelete !
                    IKA_HandleDelete ( pMessage->dwParamA );
            }
            pMessage->dwFlags = 0;
        }
        pMessage++;
    }

    IKA_HandleDelete ( ikaMessageStack.dwHandle );    
    _OSI_LeaveCriticalSection ( &ikaMessageStack.csHandle );    
    _OSI_DeleteCriticalSection ( &ikaMessageStack.csHandle );
}


long IKA_MessageProcessStack ( void )
{
    IKA_MESSAGE *pMessage;

    //Is there any available message ?
    if (!ikaMessageStack.dwMessageCount)
        return 0;
        
    //Get message
    pMessage = &(ikaMessageStack.pMessage[ikaMessageStack.dwBaseIndex]);
    
    if (pMessage->dwFlags)
    {
        IKA_MessageProcess ( pMessage );
        
        pMessage->dwFlags = 0;
        ikaMessageStack.dwBaseIndex++;
        if (ikaMessageStack.dwBaseIndex>=ikaMessageStack.dwTotalCount)
            ikaMessageStack.dwBaseIndex = 0;            
            
        ikaMessageStack->dwUsedCount--;
    }
    
    return 0;
}

DLLEXPORT   long IKA_MessagePost ( DWORD dwMessage, DWORD dwParamA, DWORD dwParamB )
{
    _OSI_EnterCriticalSection ( &ikaMessageStack.csHandle );
    
    
    _OSI_LeaveCriticalSection ( &ikaMessageStack.csHandle );
}


