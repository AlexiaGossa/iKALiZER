#include "sam_header.h"
#include "sam_data.h"
#include "ika_message.h"
#include "ika_handle.h"
#include "ika_messagestack.h"


DLLEXPORT   long IKA_MessageSend ( DWORD dwMessage, DWORD dwParamA, DWORD dwParamB )
{
    long        lReturn;
    IKA_MESSAGE Message;
    
    _OSI_EnterCriticalSection ( &ikaMessageStack.csHandle );
    
    Message.dwFlags   = IKA_MESSAGEFLAGS_MESSAGE | IKA_MESSAGEFLAGS_PARAMA | IKA_MESSAGEFLAGS_PARAMB;
    Message.dwMessage = dwMessage;
    Message.dwParamA  = dwParamA;
    Message.dwParamB  = dwParamB;
    
    lReturn = IKA_MessageProcess ( &Message );
    
    _OSI_LeaveCriticalSection ( &ikaMessageStack.csHandle );
    
    return lReturn;
}    
