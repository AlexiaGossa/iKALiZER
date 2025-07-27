#   ifndef _IKA_HANDLE_H_
#       define _IKA_HANDLE_H_

        #include "ika_types.h"
        #include "ika_osi.h"

/*
Fonctionnement des handles

Un handle est fourni sous forme d'une valeur 32 bits.

Un handle peut contenir pas mal de choses et surtout, il fournit le support de l'allocation mémoire dynamique encore absent d'iKALiZER.
Cependant, étant donné que cette allocation est controlée par les handles, il n'y a aucun risque de fuite mémoire, même si
l'utilisateur oublie de supprimer la zone mémoire.
On gagne énormément en stabilité.


*/

        

        #define IKA_HANDLE_COUNTMAX         262144
        #define IKA_HANDLE_ERR              0xFFFFFFFF
        #define IKA_HANDLE_ERROR            (IKA_HANDLE_ERR)

        enum
        {
            IKA_HANDLE_DESC_NULL,
            IKA_HANDLE_DESC_MESSAGEPIPE,
            IKA_HANDLE_DESC_MESSAGESTACK
        };

        typedef struct {
            DWORD   dwHandleUsage;               //0 = free
            DWORD   dwDescription;
            DWORD   dwParamA;
            DWORD   dwParamB;
            void    (*pFreeProc) ( DWORD dwHandle );
            void    *pRaw;
            DWORD   dwRawBytes;
            DWORD   dwRawBytesAlloc;
        } IKA_HANDLE;

        typedef struct {
            DWORD                       dwHandleCount;
            IKA_HANDLE                  *pHandle;
            DWORD                       dwHandleIndexFirstFree;
            DWORD                       dwHandleAvailableCount;
            _OSI_CRITICAL_SECTION       csHandle;
            

        } IKA_HANDLE_GLOBALDATA;

        //Fonctions externes et internes
        long IKA_HandleCreate ( DWORD * pdwHandle );
        long IKA_HandleDelete ( DWORD dwHandle );

        //Fonctions internes uniquement
        long IKA_HandleOpen ( DWORD * pdwTotalNeedsBytesCount, void * pAllocatedMemory );
        long IKA_HandleClose ( void );
        long IKA_HandleDataSet ( DWORD dwHandle, DWORD dwDescription, DWORD dwParamA, DWORD dwParamB );
        long IKA_HandleDataGet ( DWORD dwHandle, DWORD *pdwDescription, DWORD *pdwParamA, DWORD *pdwParamB );
        long IKA_HandleFreeProcSet ( DWORD dwHandle, void (*pFreeProc) (DWORD dwHandle) );
        long IKA_HandleRawAlloc ( DWORD dwHandle, DWORD dwBytesSize );
        long IKA_HandleRawFree ( DWORD dwHandle );
        long IKA_HandleRawGet ( DWORD dwHandle, void **pRaw );

        //Fonctions locales uniquement
        long IKA_HandleExist ( DWORD dwHandle, IKA_HANDLE ** pHandle );

#   endif    