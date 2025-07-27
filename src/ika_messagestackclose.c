#include "sam_header.h"
#include "sam_data.h"
#include "ika_message.h"
#include "ika_handle.h"
#include "ika_messagestack.h"

long IKA_MessageStackClose ( void )
{
    DWORD dwIndex;
    IKA_MESSAGE *pMessage;
    
    _OSI_ThreadClose ( &ikaMessageStack.threadMessageStack, 500 );
    
    _OSI_EnterCriticalSection ( &ikaMessageStack.csHandle );

    //Get memory location
    IKA_HandleRawGet ( 
        ikaMessageStack.dwHandle, 
        &pMessage );
    
    for (dwIndex=0;dwIndex<ikaMessageStack.dwTotalCount;dwIndex++)
    {
        if (pMessage->dwFlags&IKA_MESSAGEFLAGS_PIPEEXEC)
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
    
    //Use a Event
    _OSI_DeleteEvent ( &ikaMessageStack.eventNewMessage );
    
    return 0;    
}

