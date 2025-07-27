#include "sam_header.h"
#include "sam_data.h"
#include "ika_message.h"
#include "ika_handle.h"
#include "ika_messagestack.h"


IKA_MESSAGESTACK ikaMessageStack;




long IKA_MessageProcessStack ( void )
{
    IKA_MESSAGE *pMessage;

    //Is there any available message ?
    if (!ikaMessageStack.dwUsedCount)
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
            
        ikaMessageStack.dwUsedCount--;
    }
    
    return 0;
}


/*

How stack works ?

The stack uses a dedicated thread.
In case of post command, the message is posted into the stack.
In case of send command, the thread wait for the current stack message processing.
The thread is freezed and the send command is proceeed !

*/

//Thread proc... wait time = 10ms or less...
long IKA_MessageStackThreadProc ( void * pProcData )
{
    long lReturnState;
    
    do {
        //Wait for a semaphore (signaling a new message in the stack)
        lReturnState = 0;
        _OSI_WaitForEvent ( &ikaMessageStack.eventNewMessage, 10, &lReturnState );
        
        //A new message ???
        if (lReturnState)
        {        
            //Enter the critical section...
            _OSI_EnterCriticalSection ( &ikaMessageStack.csHandle );
        
            //Process the new message
            IKA_MessageProcessStack ( );

            //Some new messages ?
            if (ikaMessageStack.dwUsedCount)
                _OSI_SetEvent ( &ikaMessageStack.eventNewMessage );
            else
                _OSI_ResetEvent ( &ikaMessageStack.eventNewMessage );
        
            //Leave the critical section...
            _OSI_LeaveCriticalSection ( &ikaMessageStack.csHandle );
        }

    } while ( _OSI_ThreadDoWhileAndWaitExit ( pProcData ) );
          
    return 0;
}



