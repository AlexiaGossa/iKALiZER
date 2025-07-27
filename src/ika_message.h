#   ifndef _IKA_MESSAGE_H_
#       define _IKA_MESSAGE_H_

        #include "ika_types.h"
        #include "ika_osi.h"


        #if defined(SAMLIB_DLL_EXPORTS) || defined(IKALIZER_DLL_EXPORTS)
            #ifndef DLLEXPORT
                #define DLLEXPORT   __declspec(dllexport)
            #endif
        #else
            #define DLLEXPORT
        #endif

        long    IKA_MessageOpen    ( void );
        long    IKA_MessageClose   ( void );

        //**********************************************************************************************
        //
        //  Public
        //        
        DLLEXPORT   long IKA_MessagePipeAdd ( DWORD dwHandle, DWORD dwMessage, QWORD qwParamA, QWORD qwParamB );
        DLLEXPORT   long IKA_MessagePipeCreate ( DWORD * pdwHandle );
        DLLEXPORT   long IKA_MessagePipeDelete ( DWORD dwHandle );
        DLLEXPORT   long IKA_MessageSend ( DWORD dwMessage, QWORD qwParamA, QWORD qwParamB );
        DLLEXPORT   long IKA_MessagePost ( DWORD dwMessage, QWORD qwParamA, QWORD qwParamB );
        DLLEXPORT   long IKA_MessagePipeExec ( DWORD dwHandle, long lImmediateMode, long lEnableAutoPipeDelete );
        
        //**********************************************************************************************
        //
        //  Private
        //        
        typedef struct {
            DWORD   dwFlags;
            DWORD   dwMessage;
            QWORD   qwParamA;
            QWORD   qwParamB;
            QWORD   qwParamC;
            QWORD   qwParamD;
            QWORD   qwParamE;
            QWORD   qwParamF;
            QWORD   qwMessageTimeStamp;
        } IKA_MESSAGE;
        
        
        


#   endif

/*

    Fonctionnement de la file d'attente des messages



*/











/*
long IKA_MessagePipeDeclare ( DWORD dwHandle );
long IKA_MessagePipeAdd ( DWORD dwHandle, DWORD dwMessage, DWORD dwParamA, DWORD dwParamB );
long IKA_MessagePipeSend ( DWORD dwHandle, long lEnableAutoDelete );
long IKA_MessagePipePost ( DWORD dwHandle, long lEnableAutoDelete );
long IKA_MessageSend ( DWORD dwMessage, DWORD dwParamA, DWORD dwParamB );
long IKA_MessagePost ( DWORD dwMessage, DWORD dwParamA, DWORD dwParamB );
long IKA_MessageSendEx ( DWORD dwMessage, DWORD dwParamA, DWORD dwParamB, DWORD dwParamC, DWORD dwParamD );
long IKA_MessagePostEx ( DWORD dwMessage, DWORD dwParamA, DWORD dwParamB, DWORD dwParamC, DWORD dwParamD );
long IKA_MessageSendExt ( DWORD dwMessage, DWORD dwParamA, DWORD dwParamB, DWORD dwParamC, DWORD dwParamD, DWORD dwParamE, DWORD dwParamF );
long IKA_MessagePostExt ( DWORD dwMessage, DWORD dwParamA, DWORD dwParamB, DWORD dwParamC, DWORD dwParamD, DWORD dwParamE, DWORD dwParamF );
*/
//Le mode AutoDelete se charge de supprimer le handle une fois l'exécution terminée.
//Etant donné que toutes les fonctions sont gérables par des messages, le AutoDelete
//est simplement un message de type "HandleDelete %handle%" en plus dans le MessagePipe


#define IKA_MESSAGEFLAGS_PIPEEXEC       0x00000001
#define IKA_MESSAGEFLAGS_MESSAGE        0x00000002
#define IKA_MESSAGEFLAGS_PARAMA         0x00000004
#define IKA_MESSAGEFLAGS_PARAMB         0x00000008
#define IKA_MESSAGEFLAGS_PARAMC         0x00000010
#define IKA_MESSAGEFLAGS_PARAMD         0x00000020
#define IKA_MESSAGEFLAGS_PARAME         0x00000040
#define IKA_MESSAGEFLAGS_PARAMF         0x00000080


typedef struct {
    DWORD           dwMessagePipeCount;
    IKA_MESSAGE     pMessage[1];
} IKA_MESSAGEPIPE;

enum
{
    IKA_MESSAGE_CREATEHANDLE,
    IKA_MESSAGE_DELETEHANDLE,
    
    IKA_MESSAGE_PIPEDECLARE,
    IKA_MESSAGE_PIPEADD,
    IKA_MESSAGE_PIPESEND,
    IKA_MESSAGE_PIPEPOST,
    IKA_MESSAGE_PIPEEXEC,
    
    IKA_MESSAGE_POST,
    IKA_MESSAGE_POSTEX,
    IKA_MESSAGE_POSTEXT,
    
    IKA_MESSAGE_VARSET_UNITY,
    IKA_MESSAGE_VARGET_UNITY,
    
    IKA_MESSAGE_CONFIGSET_MEMORYSOUND,
    IKA_MESSAGE_CONFIGSET_OUTPUT_SAMPLINGRATE,
    IKA_MESSAGE_CONFIGSET_OUTPUT_CHANNELMODE,
    IKA_MESSAGE_CONFIGSET_OUTPUT_BUFFERLATENCYDURATION,
    IKA_MESSAGE_CONFIGSET_OUTPUT_BUFFERTOTALDURATION,
    IKA_MESSAGE_CONFIGSET_OUTPUT_
    //IKA_MESSAGE_CONFIGSET_
};

/*
typedef struct {
    IKA_MESSAGE     * pikaMessage;
    DWORD           dwMessageCount;
    DWORD           dwMessageMemoryAllocCount;
    
    

} IKA_MESSAGESTACK;
*/

