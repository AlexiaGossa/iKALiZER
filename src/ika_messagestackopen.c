#include "sam_header.h"
#include "sam_data.h"
#include "ika_message.h"
#include "ika_handle.h"
#include "ika_messagestack.h"

long IKA_MessageStackOpen ( void )
{
    //Use a handle for Message Stack
    IKA_HandleCreate ( &ikaMessageStack.dwHandle );
    
    //Use a Critical Section
    _OSI_InitializeCriticalSection ( &ikaMessageStack.csHandle );
    
    //Use a Event
    _OSI_InitializeEvent ( &ikaMessageStack.eventNewMessage, 0, 0 );
    
    //Init base value
    ikaMessageStack.dwBaseIndex     = 0;
    ikaMessageStack.dwTotalCount    = 1024;
    ikaMessageStack.dwUsedCount     = 0;

    //This handle is a pipe !
    IKA_HandleDataSet ( 
        ikaMessageStack.dwHandle,
        IKA_HANDLE_DESC_MESSAGESTACK,
        0,
        0 );
    
    //Alloc memory for this stack
    IKA_HandleRawAlloc ( 
        ikaMessageStack.dwHandle, 
        sizeof(IKA_MESSAGE)*ikaMessageStack.dwTotalCount );
        
    //Get memory location
    IKA_HandleRawGet ( 
        ikaMessageStack.dwHandle, 
        &(ikaMessageStack.pMessage) );
        

    //Start thread        
    _OSI_ThreadOpen ( 
            &ikaMessageStack.threadMessageStack,
            IKA_MessageStackThreadProc,
            0,
            NULL );
        
    
    return 0;
}

