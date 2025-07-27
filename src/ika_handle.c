#include "ika_handle.h"


IKA_HANDLE_GLOBALDATA   ikaHandleGlobalData;


/*
    ###################################################################################################

        IKA_HandleOpen
        
        Type    [x] Internal call
                [ ] External call

    ###################################################################################################
*/
long IKA_HandleOpen ( DWORD * pdwTotalNeedsBytesCount, void * pAllocatedMemory )
{
    DWORD dwHandleCountMax;

    //On vide !    
    memset ( &ikaHandleGlobalData, 0, sizeof(IKA_HANDLE_GLOBALDATA) );

    //Combien de Handles ?    
    dwHandleCountMax = IKA_HANDLE_COUNTMAX;
    if (dwHandleCountMax>1048575) dwHandleCountMax = 1048575;
    
    
    if (pdwTotalNeedsBytesCount)
    {
        *pdwTotalNeedsBytesCount = sizeof(IKA_HANDLE) * dwHandleCountMax;
        return 0;
    }
    
    if (pAllocatedMemory)
    {
        ikaHandleGlobalData.dwHandleCount           = dwHandleCountMax;
        ikaHandleGlobalData.pHandle                 = (IKA_HANDLE *)pAllocatedMemory;
        ikaHandleGlobalData.dwHandleIndexFirstFree  = 0;
        ikaHandleGlobalData.dwHandleAvailableCount  = dwHandleCountMax;
        memset ( ikaHandleGlobalData.pHandle, 0, sizeof(IKA_HANDLE) * dwHandleCountMax );
        
        _OSI_InitializeCriticalSection ( &ikaHandleGlobalData.csHandle );
        return 0;
    }
    
    return -1;
}

/*
    ###################################################################################################

        IKA_HandleClose
        
        Type    [x] Internal call
                [ ] External call

    ###################################################################################################
*/
long IKA_HandleClose ( void )
{
    DWORD dwHandleIndex;
    IKA_HANDLE *pHandle;

    _OSI_EnterCriticalSection ( &ikaHandleGlobalData.csHandle );
    pHandle = ikaHandleGlobalData.pHandle;
    for (dwHandleIndex=0;dwHandleIndex<ikaHandleGlobalData.dwHandleCount;dwHandleIndex++,pHandle++)
    {
        if (pHandle->dwHandleUsage)
        {   
            //Traitement interne au handle suivant son contenu        
            if (pHandle->pFreeProc)
                pHandle->pFreeProc ( dwHandleIndex );
            
            //On vide ce handle
            if (pHandle->pRaw)
                free ( pHandle->pRaw );
                
            pHandle->dwHandleUsage = 0;
        }
    }
    memset ( ikaHandleGlobalData.pHandle, 0, sizeof(IKA_HANDLE) * ikaHandleGlobalData.dwHandleCount );
    ikaHandleGlobalData.dwHandleCount               = 0;
    ikaHandleGlobalData.pHandle                     = NULL;
    ikaHandleGlobalData.dwHandleAvailableCount      = 0;
    ikaHandleGlobalData.dwHandleIndexFirstFree      = IKA_HANDLE_ERROR;
    _OSI_LeaveCriticalSection ( &ikaHandleGlobalData.csHandle );
    _OSI_DeleteCriticalSection ( &ikaHandleGlobalData.csHandle );
    return 0;
}

/*
    ###################################################################################################

        IKA_HandleCreate
        
        Type    [x] Internal call
                [x] External call

    ###################################################################################################
*/
long IKA_HandleCreate ( DWORD * pdwHandle )
{
    DWORD dwHandleIndex;
    DWORD dwIndex, dwEntry;
    
    _OSI_EnterCriticalSection ( &ikaHandleGlobalData.csHandle );
    
    dwHandleIndex = ikaHandleGlobalData.dwHandleIndexFirstFree;
    if (dwHandleIndex!=IKA_HANDLE_ERROR)
    {    
        //On vient de prendre un handle
        ikaHandleGlobalData.dwHandleAvailableCount -= 1;
        
        //Recherche rapidement le prochain handle de libre (en général, il est à la suite)
        if (ikaHandleGlobalData.dwHandleAvailableCount)
        {
            dwEntry = (dwHandleIndex+1)%ikaHandleGlobalData.dwHandleCount;
            for (dwIndex=0;dwIndex<ikaHandleGlobalData.dwHandleCount;dwIndex++)
            {
                if (!ikaHandleGlobalData.pHandle[dwEntry].dwHandleUsage)
                {
                    ikaHandleGlobalData.dwHandleIndexFirstFree = dwEntry;
                    break;
                }    
                dwEntry = (dwEntry+1)%ikaHandleGlobalData.dwHandleCount;
            }
        }
        else
        {
            //Il n'y a plus de handle de disponible
            ikaHandleGlobalData.dwHandleIndexFirstFree = IKA_HANDLE_ERROR;
        }
        
        //Vide ce handle et l'assigne
        memset ( &ikaHandleGlobalData.pHandle[dwHandleIndex], 0, sizeof(IKA_HANDLE) );
        ikaHandleGlobalData.pHandle[dwHandleIndex].dwHandleUsage = 1;
    }
    
    _OSI_LeaveCriticalSection ( &ikaHandleGlobalData.csHandle );
    
    if (pdwHandle)
        *pdwHandle = dwHandleIndex;
        
    return (dwHandleIndex==IKA_HANDLE_ERROR)?(-1):(0);
}    
        
/*
    ###################################################################################################

        IKA_HandleDelete
        
        Type    [x] Internal call
                [x] External call

    ###################################################################################################
*/
long IKA_HandleDelete ( DWORD dwHandle )
{
    IKA_HANDLE * pHandle;
    //On vérifie l'existance du Handle
    if (IKA_HandleExist(dwHandle,&pHandle))
        return -1;

    //Traitement interne au handle suivant son contenu        
    if (pHandle->pFreeProc)
        pHandle->pFreeProc ( dwHandle );
    
    //On vide ce handle
    if (pHandle->pRaw)
        free ( pHandle->pRaw );
    memset ( pHandle, 0, sizeof(IKA_HANDLE) );
    
    //On libère le handle pour le rendre à nouveau disponible
    _OSI_EnterCriticalSection ( &ikaHandleGlobalData.csHandle );
    
    ikaHandleGlobalData.dwHandleAvailableCount += 1;
    
    if (ikaHandleGlobalData.dwHandleIndexFirstFree==IKA_HANDLE_ERROR)
        ikaHandleGlobalData.dwHandleIndexFirstFree = dwHandle;
    
    _OSI_LeaveCriticalSection ( &ikaHandleGlobalData.csHandle );
    
    return 0;
}































long IKA_HandleExist ( DWORD dwHandle, IKA_HANDLE ** pHandle )
{
    //Le numéro du handle est correct ?
    if (dwHandle>=ikaHandleGlobalData.dwHandleCount)
        return -1;    
    
    //Handle réellement utilisé    
    if (!ikaHandleGlobalData.pHandle[dwHandle].dwHandleUsage)
        return -1;
        
    if (pHandle)
        *pHandle = &ikaHandleGlobalData.pHandle[dwHandle];

    return 0;
}

long IKA_HandleDataSet ( DWORD dwHandle, DWORD dwDescription, DWORD dwParamA, DWORD dwParamB )
{
    IKA_HANDLE * pHandle;
    
    //On vérifie l'existance du Handle
    if (IKA_HandleExist(dwHandle,&pHandle))
        return -1;

    pHandle->dwDescription  = dwDescription;
    pHandle->dwParamA       = dwParamA;
    pHandle->dwParamB       = dwParamB;
    
    return 0;    
}

long IKA_HandleDataGet ( DWORD dwHandle, DWORD *pdwDescription, DWORD *pdwParamA, DWORD *pdwParamB )
{
    IKA_HANDLE * pHandle;
    
    //On vérifie l'existance du Handle
    if (IKA_HandleExist(dwHandle,&pHandle))
        return -1;

    if (pdwDescription)
        *pdwDescription = pHandle->dwDescription;
    
    if (pdwParamA)
        *pdwParamA = pHandle->dwParamA;

    if (pdwParamB)
        *pdwParamB = pHandle->dwParamB;
    
    return 0;    
}


long IKA_HandleFreeProcSet ( DWORD dwHandle, void (*pFreeProc) (DWORD dwHandle) )
{
    IKA_HANDLE * pHandle;
    
    //On vérifie l'existance du Handle
    if (IKA_HandleExist(dwHandle,&pHandle))
        return -1;
        
    pHandle->pFreeProc = pFreeProc;

    return 0;
}

long IKA_HandleRawGet ( DWORD dwHandle, void **pRaw )
{
    IKA_HANDLE * pHandle;
    
    //On vérifie l'existance du Handle
    if (IKA_HandleExist(dwHandle,&pHandle))
        return -1;
        
    if (pRaw)
        *pRaw = pHandle->pRaw;

    return 0;
}

long IKA_HandleRawAlloc ( DWORD dwHandle, DWORD dwBytesSize )
{
    IKA_HANDLE  *pHandle;
    DWORD       dwBytesSizeAlloc;

    //Allocation par bloc de 256 octets    
    dwBytesSizeAlloc = dwBytesSize&0xFFFFFF00;
    if (dwBytesSize&0xFF) dwBytesSizeAlloc += 256;
    
    //On vérifie l'existance du Handle
    if (IKA_HandleExist(dwHandle,&pHandle))
        return -1;
        
    if (!dwBytesSize)
    {
        if (pHandle->pRaw)
            free ( pHandle->pRaw );
            
        pHandle->pRaw               = NULL;
        pHandle->dwRawBytes         = 0;
        pHandle->dwRawBytesAlloc    = 0;
    }
    else
    {
        if (pHandle->pRaw)
        {
            //Il y a déjà une allocation en cours, on va réallouer
            pHandle->pRaw = (void *)realloc (
                pHandle->pRaw,
                dwBytesSizeAlloc );
                
            //La nouvelle allocation est plus grande que l'ancienne, on vide le surplus
            if (dwBytesSize>pHandle->dwRawBytes)
            {
                BYTE * pRaw;
                pRaw = (BYTE *)pHandle->pRaw;
                memset ( pRaw+pHandle->dwRawBytes, 0, dwBytesSize-pHandle->dwRawBytes );
            }
            
            pHandle->dwRawBytes         = dwBytesSize;
            pHandle->dwRawBytesAlloc    = dwBytesSizeAlloc;
        }
        else
        {
            pHandle->pRaw = (void *)malloc ( dwBytesSizeAlloc );
            memset ( pHandle->pRaw, 0, dwBytesSize );
            pHandle->dwRawBytes         = dwBytesSize;
            pHandle->dwRawBytesAlloc    = dwBytesSizeAlloc;
        }
    }    
    return 0;
}

long IKA_HandleRawFree ( DWORD dwHandle )
{
    return IKA_HandleRawAlloc ( dwHandle, 0 );
}




    