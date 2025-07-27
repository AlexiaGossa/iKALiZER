#include "sam_header.h"
#include "sam_data.h"
#include "ika_message.h"

/*

    Fonctionnement de la file d'attente des messages
    
    On dispose d'une file d'attente assez importante pour traîter les messages
    
    En mode post, les messages sont stocker dans la file d'attente et le retour est immédiat
    pour l'appelant. Il ne saura pas si le message aura été traîter correctement.
    
    En mode send, les messages sont traîtés immédiatement et le code de retour est transmis.
    
    Pour les MessagePipe, c'est un peu le même concept.
    



*/





long IKA_MessageProcess ( IKA_MESSAGE * pMessage )
{
    long lBackValue;
    
    lBackValue = -1;
    
    if (pMessage->dwMessage&IKA_MESSAGEFLAGS_MESSAGE)
    {    
        switch (pMessage->dwMessage)
        {
            default:
                lBackValue = -2;
                break;
                
            case IKA_MESSAGE_CREATEHANDLE:
                lBackValue = IKA_HandleCreate ( (DWORD *)pMessage->dwParamA );
                break;
            
            case IKA_MESSAGE_DELETEHANDLE:
                lBackValue = IKA_HandleDelete ( pMessage->dwParamA );
                break;
                
            case IKA_MESSAGE_PIPEDECLARE:
                lBackValue = IKA_MessagePipeDeclare ( pMessage->dwParamA );
                break;
            /*    
            case IKA_MESSAGE_PIPEADD:
                lBackValue = IKA_MessagePipeAdd (
                    pMessage->dwParamA,
                    pMessage->dwParamB,
                    pMessage->dwParamC,
                    pMessage->dwParamD );
                break;
            /*    
            case IKA_MESSAGE_PIPESEND:
                lBackValue = IKA_MessagePipeSend (
                    pMessage->dwParamA,
                    (long)pMessage->dwParamB );
                break;
                
            case IKA_MESSAGE_PIPEPOST:
                lBackValue = IKA_MessagePipePost (
                    pMessage->dwParamA,
                    (long)pMessage->dwParamB );
                break;
            */    
            case IKA_MESSAGE_POST:
                break;
            case IKA_MESSAGE_POSTEX:
                break;
            case IKA_MESSAGE_POSTEXT:
                break;
                
                
            /*
            Set unity base for I/O data convertion.
            iKALiZER internal unity is ISO meter (m).
            Possible unity :
            - meter (m)
            - foot (f)
            - inch (")
            - ...
            */                
            case IKA_MESSAGE_VARSET_UNITY:
                break;
        
        
        }
    }
    return lBackValue;
}
    