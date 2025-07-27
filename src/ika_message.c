#include "ika_message.h"
#include "ika_handle.h"


typedef struct {
    DWORD               dwHandle;
    
    IKA_MESSAGE         *pMessage;
   
    DWORD               dwBaseIndex;
    DWORD               dwTotalCount;
    DWORD               dwUsedCount;
    
    _OSI_CRITICAL_SECTION       csHandle;
    _OSI_EVENT                  eventNewMessage;
    _OSI_THREAD                 threadMessageStack;
    
} IKA_MESSAGESTACK;

IKA_MESSAGESTACK ikaMessageStack;


long    IKA_MessageStackThreadProc ( void * pProcData );
long    IKA_MessageProcessStack ( void );
long    IKA_MessageProcess ( IKA_MESSAGE * pMessage );

/*
    ########################################################################################################################
    
    IKA_MessageOpen
    
    ########################################################################################################################
*/


long IKA_MessageOpen ( void )
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


/*
    ########################################################################################################################
    
    IKA_MessageClose
    
    ########################################################################################################################
*/

long IKA_MessageClose ( void )
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
                if ((pMessage->qwParamB)&1) //Enable autodelete !
                    IKA_HandleDelete ( (DWORD)(pMessage->qwParamA) );
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

/*
    ########################################################################################################################
    
    IKA_MessageProcessStack
    
    ########################################################################################################################
*/


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
    ########################################################################################################################
    
    IKA_MessageStackThreadProc
    
    ########################################################################################################################
*/

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

/*
    ########################################################################################################################
    
    IKA_MessageProcess
    
    ########################################################################################################################
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
                lBackValue = IKA_HandleCreate ( (DWORD *)(pMessage->qwParamA) );
                break;
            
            case IKA_MESSAGE_DELETEHANDLE:
                lBackValue = IKA_HandleDelete ( (DWORD)(pMessage->qwParamA) );
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
    
//  ########################################################################################################
//
//  Add a message to a message pipe handle
//
DLLEXPORT   long IKA_MessagePipeAdd ( DWORD dwHandle, DWORD dwMessage, QWORD qwParamA, QWORD qwParamB )
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
    pMessage->qwParamA  = qwParamA;
    pMessage->qwParamB  = qwParamB;
    pMessage->dwFlags   = IKA_MESSAGEFLAGS_MESSAGE | IKA_MESSAGEFLAGS_PARAMA | IKA_MESSAGEFLAGS_PARAMB;
        
    return 0;
}

//  ########################################################################################################
//
//  Create a message pipe handle
//
DLLEXPORT   long IKA_MessagePipeCreate ( DWORD * pdwHandle )
{
    DWORD               dwHandle;
    IKA_MESSAGEPIPE     *pMessagePipe;
    long                lReturn;

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
        0,
        0 );
    
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

//  ########################################################################################################
//
//  Send a message
//
DLLEXPORT   long IKA_MessageSend ( DWORD dwMessage, QWORD qwParamA, QWORD qwParamB )
{
    long        lReturn;
    IKA_MESSAGE Message;
    
    _OSI_EnterCriticalSection ( &ikaMessageStack.csHandle );
    
    Message.dwFlags   = IKA_MESSAGEFLAGS_MESSAGE | IKA_MESSAGEFLAGS_PARAMA | IKA_MESSAGEFLAGS_PARAMB;
    Message.dwMessage = dwMessage;
    Message.qwParamA  = qwParamA;
    Message.qwParamB  = qwParamB;
    
    lReturn = IKA_MessageProcess ( &Message );
    
    _OSI_LeaveCriticalSection ( &ikaMessageStack.csHandle );
    
    return lReturn;
}

//  ########################################################################################################
//
//  Post a message
//
DLLEXPORT   long IKA_MessagePost ( DWORD dwMessage, QWORD qwParamA, QWORD qwParamB )
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
    pMessage[dwIndex].qwParamA  = qwParamA;
    pMessage[dwIndex].qwParamB  = qwParamB;
    
    ikaMessageStack.dwUsedCount += 1;
    _OSI_SetEvent ( &ikaMessageStack.eventNewMessage );
    
    _OSI_LeaveCriticalSection ( &ikaMessageStack.csHandle );
    
    return 0;
}


//  ########################################################################################################
//
//  Execute the pipe
//
//  Execute the pipe in immediate mode (like SEND) or delayed mode (like POST) and optionnaly delete the pipe handle
//  //If posted : dwParamA = dwHandle, bit0(dwParamB) = lEnableAutoPipeDelete
//
DLLEXPORT   long IKA_MessagePipeExec ( DWORD dwHandle, long lImmediateMode, long lEnableAutoPipeDelete )
{

}



/*
#include "sam_header.h"
#include "sam_data.h"
#include "ika_message.h"
#include "ika_handle.h"

/*
typedef struct {
    IKA_MESSAGE     * pMessage;
    DWORD           dwMessageCount;
    DWORD           dwMessageAllocatedCount;
    DWORD           dwMessageFreeCount;
    DWORD           dwMessageCurrentIndex;
    
    DWORD           dwBaseIndex;
    DWORD           dwTotalCount;
    DWORD           dwUsedCount;


} IKA_MESSAGESTACK;

IKA_MESSAGESTACK ikaMessageStack;
*/
/*
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





long IKA_MessagePipeDeclare ( DWORD dwHandle )
{
    IKA_MESSAGEPIPE * pMessagePipe;
    
    if (IKA_HandleExist(dwHandle,NULL))
        return -1;

    //On assigne le handle comme étant un pipe
    IKA_HandleDataSet ( 
        dwHandle,
        IKA_HANDLE_DESC_MESSAGEPIPE,
        NULL,
        NULL );
        
    //On effectue une déclaration mémoire
    IKA_HandleRawAlloc ( 
        dwHandle, 
        sizeof(IKA_MESSAGEPIPE) );
        
    IKA_HandleRawGet ( 
        dwHandle, 
        &pMessagePipe );
    
    //On force bien le nombre d'entrée à zéro, même si l'allocation force les valeurs à ZERO
    pMessagePipe->dwMessagePipeCount = 0;
        
    return 0;
}

long IKA_MessagePipeAdd ( DWORD dwHandle, DWORD dwMessage, DWORD dwParamA, DWORD dwParamB )
{
    IKA_MESSAGEPIPE *pMessagePipe;
    IKA_MESSAGE     *pMessage;
    DWORD           dwDescription;
    
    if (IKA_HandleExist(dwHandle,NULL))
        return -1;
        
    dwDescription = 0;
    pMessagePipe = NULL;
    
        
    IKA_HandleDataGet ( 
        dwHandle, 
        &dwDescription,
        NULL,
        NULL );

    IKA_HandleRawGet ( 
        dwHandle, 
        &pMessagePipe );
        
    if ( (dwDescription!=IKA_HANDLE_DESC_MESSAGEPIPE) ||
         (pMessagePipe==NULL) )
        return -1;

    //Ajoute un message        
    pMessagePipe->dwMessagePipeCount+=1;
    
    //Ré-allocation
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


/*

//Les 2 seules fonctions d'initialisation
DWORD IKA32_GetFunction ( DWORD dwFunction, DWORD dwBackAdress );
DWORD IKA64_GetFunction ( DWORD dwFunction, QWORD qwBackAdress );

//Les fonctions de messages
DWORD IKA_MessageSend ( DWORD dwContext, DWORD dwMessage, DWORD dwParamA, DWORD dwParamB, DWORD dwValue );

//Create a pipe
DWORD IKA_MessagePipeCreate ( DWORD * pdwHandlePipe );
//Delete a pipe
DWORD IKA_MessagePipeDelete ( DWORD dwHandlePipe );
//Add a message to a pipe
DWORD IKA_MessagePipeAddTo ( DWORD dwHandlePipe, DWORD dwContext, DWORD dwMessage, DWORD dwParamA, DWORD dwParamB, DWORD dwValue );
//Send a pipe and return back immediately
DWORD IKA_MessagePipeSend ( DWORD dwHandlePipe );
//Flush/Empty a pipe 
DWORD IKA_MessagePipeFlush ( DWORD dwHandlePipe );
//Wait for the end of a posted pipe
DWORD IKA_MessagePipeWaitForEnd ( DWORD dwHandlePipe );

typedef struct {
    DWORD   dwMessageCurrent;
    DWORD   dwMessagePrevious;
    DWORD   dwMessageNext;
    
    DWORD   dwContext;
    DWORD   dwMessage;
    DWORD   dwParamA;
    DWORD   dwParamB;
    DWORD   dwValueA;
    DWORD   dwValueB;
} IKA_MESSAGE;

typedef struct {
    DWORD               dw
    

} IKA_MESSAGE_PIPE;

typedef struct {
    DWORD               dwMessageCount;
    IKA_MESSAGE         *pMessage;
    
    DWORD               dwMessageFirstFree;
    
    
   

    DWORD               dwTotalHandleCount;
    DWORD               dwTotalHandleAvailable;
    DWORD               *pdwHandleMessagePipe;
    
    DWORD               dwTotalMessagePipe;
    IKA_MESSAGE_PIPE    *pMessagePipe;
    
    
    

} IKA_MESSAGE_GLOBALDATA;

IKA_MESSAGE_GLOBALDATA ikaMessageGlobalData;
#define IKA_MESSAGE_COUNTMAX        32768
#define IKA_MESSAGE_NOINDEX         0xFFFFFFFF

long IKA_MessageOpen ( DWORD * pdwTotalNeedsBytesCount, void * pAllocatedMemory )
{
    DWORD dwIndex;
    
    memset ( 
        &ikaMessageGlobalData,
        0,
        sizeof(ikaMessageGlobalData) );

    //Répond à la question : De quelle quantité de mémoire avons-nous besoin ?        
    if ((pdwTotalNeedsBytesCount)&&(!pAllocatedMemory))
    {
        *pdwTotalNeedsBytesCount = sizeof(IKA_MESSAGE) * IKA_MESSAGE_COUNTMAX;
        return 0;
    }
    
    //Allocation
    if ((pdwTotalNeedsBytesCount)&&(pAllocatedMemory))
    {
        ikaMessageGlobalData.dwMessageCount = (*pdwTotalNeedsBytesCount) / sizeof(IKA_MESSAGE);// IKA_MESSAGE_COUNTMAX;
        ikaMessageGlobalData.pMessage       = (IKA_MESSAGE *)pAllocatedMemory;
        
        for (dwIndex=0;dwIndex<ikaMessageGlobalData.dwMessageCount;dwIndex++)
        {
            ikaMessageGlobalData.pMessage[dwIndex].dwMessageCurrent     = dwIndex;
            ikaMessageGlobalData.pMessage[dwIndex].dwMessagePrevious    = (dwIndex>0)?(dwIndex-1):(IKA_MESSAGE_NOINDEX);
            ikaMessageGlobalData.pMessage[dwIndex].dwMessageNext        = (dwIndex<ikaMessageGlobalData.dwMessageCount)?(dwIndex+1):(IKA_MESSAGE_NOINDEX);
        }
        
        ikaMessageGlobalData.dwMessageFirstFree = 0;
                
        return 0;
    }
    
    //Pas les bons paramètres
    return -1;
}

long IKA_MessageClose ( void )
{

}

long IKA_MessagePipeCreate ( DWORD * pdwHandleMessagePipe )
{
    if (ikaMessageGlobalData.dwMessageFirstFree==IKA_MESSAGE_NOINDEX)
        return -1;
        
    if (pdwHandleMessagePipe)
        *pdwHandleMessagePipe = ikaMessageGlobalData.dwMessageFirstFree;
*/    