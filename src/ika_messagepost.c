#include "sam_header.h"
#include "sam_data.h"
#include "ika_message.h"
#include "ika_handle.h"
#include "ika_messagestack.h"


DLLEXPORT   long IKA_MessagePost ( DWORD dwMessage, DWORD dwParamA, DWORD dwParamB )
{
    DWORD dwIndex;
    IKA_MESSAGE *pMessage;
    
    _OSI_EnterCriticalSection ( &ikaMessageStack.csHandle );
    
    if (ikaMessageStack.dwUsedCount>=ikaMessageStack.dwTotalCount)
    {
        do {
            _OSI_LeaveCriticalSection ( &ikaMessageStack.csHandle );
            _OSI_Sleep ( 1 );
            _OSI_EnterCriticalSection ( &ikaMessageStack.csHandle );
        } while (ikaMessageStack.dwUsedCount>=ikaMessageStack.dwTotalCount);
    }
    
    dwIndex = ( ikaMessageStack.dwBaseIndex + ikaMessageStack.dwUsedCount ) % ikaMessageStack.dwTotalCount;
    
    IKA_HandleRawGet ( 
        ikaMessageStack.dwHandle, 
        &pMessage );
        
    pMessage[dwIndex].dwFlags   = IKA_MESSAGEFLAGS_MESSAGE | IKA_MESSAGEFLAGS_PARAMA | IKA_MESSAGEFLAGS_PARAMB;
    pMessage[dwIndex].dwMessage = dwMessage;
    pMessage[dwIndex].dwParamA  = dwParamA;
    pMessage[dwIndex].dwParamB  = dwParamB;
    
    ikaMessageStack.dwUsedCount += 1;
    _OSI_SetEvent ( &ikaMessageStack.eventNewMessage );
    
    _OSI_LeaveCriticalSection ( &ikaMessageStack.csHandle );
    
    return 0;
}


