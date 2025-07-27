#include "sam_header.h"
#include "sam_data.h"
#include "stdio.h"
#include "sam_sfx.h"
/*typedef struct
{
    long                    bIsFree;                // This sfx is free
    char                    szFileName[MAX_QPATH];
    void                    *pBuffer;
    long                    bIsLoaded;              // Sound is loaded in pBuffer (is pBuffer is allocated)
    long                    lSizeInBytes;           // Sound size in bytes
    long                    lSizeInSamples;         // Sound size in samples
    long                    lBitsPerSample;         // Bits per sample
    long                    lBytesPerSample;        // Bytes per sample
    float                   fBaseSampleRate;        // Base sample rate in Hertz
    float                   fBaseAbsoluteLevel;     // Base absolute level to convert to real world (in dB)
    long                    lStoreMode;             // Store mode 0 = 32 bits float, 1 = 8 bits LPCM, 2 = 16 bits LPCM, 3 = XPCM, 4 = DMSP
    long                    lInUseCount;            // Usage counter 0 = if not in use, >=1 if used
    long                    lAllocTime;             // Time when this sfx was created in ms
} sam_sfx_t;
*/


SAM_SFX_DATA samSfxData;


void SAM_SFX_EnterLock ( void )
{
    _OSI_EnterCriticalSection ( &samSfxData.osiCS );
}

void SAM_SFX_LeaveLock ( void )
{
    _OSI_LeaveCriticalSection ( &samSfxData.osiCS );
}

void SAM_SFX_PreOpenMemoryNeeds ( DWORD dwSFXCount, DWORD *pdwBytesNeeds )
{
    if (pdwBytesNeeds)
        *pdwBytesNeeds = sizeof(SAM_SFX) * dwSFXCount;
}


long SAM_SFX_Open ( DWORD dwSFXCount, void * pAllocatedMemory )
{
    DWORD dwBytesNeeds;

    SAM_SFX_PreOpenMemoryNeeds ( dwSFXCount, &dwBytesNeeds );

    samSfxData.dwSFXCount   = dwSFXCount;
    samSfxData.psamSFX      = (SAM_SFX *)pAllocatedMemory; //SAM_ALLOC ( dwBytesNeeds );
    memset ( samSfxData.psamSFX, 0, dwBytesNeeds );
    
    _OSI_InitializeCriticalSectionAndSpinCount ( &samSfxData.osiCS, 0x80001000 );
    
    return 0;
}

long SAM_SFX_Close ( void )
{
    //SAM_FREE ( samSfxData.psamSFX );
    samSfxData.psamSFX = NULL;
    samSfxData.dwSFXCount = 0;
    
    _OSI_DeleteCriticalSection ( &samSfxData.osiCS );
    
    return 0;
}

long SAM_SFX_GetIt ( DWORD dwHandle, SAM_SFX ** psamSFX )
{
    if (dwHandle>=samSfxData.dwSFXCount)
        return -1;

    if (!samSfxData.psamSFX[dwHandle].bIsUsed)
        return -1;

    *psamSFX = &samSfxData.psamSFX[dwHandle];

    return 0;
}


/*

    Renvoi -1 si erreur normale
           -2 si erreur car il n'y plus assez d'espace libre dans les handles
           -3 si erreur car plus assez d'espace libre dans les granules

*/
DLLEXPORT   long SAM_SFX_Load ( DWORD dwHandle, char * pszSFXName, DWORD dwSampleRate, DWORD dwSamplesCount, BYTE bStereo, BYTE b16bits, void * pAudioData, DWORD * pdwAllocatedHandle )
{
    DWORD       dwIndex;
    DWORD       dwSampleBytesCount;
    DWORD       dwSFXBytesCount;
    SAM_SFX     *psamSFX;
    BYTE        bFormat;
    DWORD       dwInternalTimer;
    long        lReturn;
    
    SAM_TIMER_GetIt ( &dwInternalTimer );

    //Le handle n'est peut-être pas libre ?
    if (dwHandle!=0xFFFFFFFF)
    {
        if (SAM_SFX_Free ( dwHandle ))
            return -1;
    }
    
    SAM_SFX_EnterLock ( );
    

    //On cherche un handle
    if (dwHandle==0xFFFFFFFF)
    {
        psamSFX = samSfxData.psamSFX;
        for (dwIndex=0;dwIndex<samSfxData.dwSFXCount;dwIndex++,psamSFX++)
        {
            if (!psamSFX->bIsUsed)
            {
                dwHandle    = dwIndex;
                dwIndex     = samSfxData.dwSFXCount;
            }
        }

        //Aucun handle de libre ?
        if (dwHandle==0xFFFFFFFF)
        {
            SAM_SFX_LeaveLock ( );
            return -2;
        }
    }
    else
    {
        if (dwHandle>=samSfxData.dwSFXCount)
        {
            SAM_SFX_LeaveLock ( );
            return -1;
        }
    }
    
    //Le SFX a utiliser
    psamSFX = &(samSfxData.psamSFX[dwHandle]);

    //Le format source
    switch (b16bits)
    {
        case  0: 
        case  8: bFormat = sam_FORMAT_MONO_PCM8; break;
        case  1: 
        case 16: bFormat = sam_FORMAT_MONO_PCM16; break;
        case 32: bFormat = sam_FORMAT_MONO_FLOAT32; break;
    }
    if (bStereo) bFormat|= 0x10;

    

    //Taille du SFX avant le stockage
    SAM_FormatGetBytesCount ( bFormat, &dwSampleBytesCount );
    dwSFXBytesCount = dwSamplesCount * dwSampleBytesCount;
    
    //dwSampleRate = 48000;
    
    if ((bFormat&sam_FORMAT_TYPEMASK)==sam_FORMAT_MONO_PCM16)
    if (0)
    {
        short * psData;
        char  cValue;
        unsigned long ui32Random;
        float f1;
        long n,i,nch;
        static long lLen = 0;
        //FILE * pFile;
        
        //dwSampleRate = 22050;
        //dwSampleRate = 48000;
        //dwSampleRate = 8000;
        
        nch         = (bStereo)?(2):(1);
        ui32Random  = dwSFXBytesCount;
        psData      = (short *)pAudioData;
        
        for (i=0;i<dwSamplesCount;i++)
        {
            ui32Random = ( 1664525 * ui32Random ) + 1013904223;
            f1 = (ui32Random>>15)&0xFFFF;
            f1 -= 32768.0F;
            //*pData =    (ui32Random>>16)&0xFF;
            
            //f1 = (float)i;
            //f1 = cos ( f1 * sam_PI * 0.15F ) * 32766.0F;// * 0.063F;
            //f1 = (i&1)?(-32766):(32766);
            for (n=0;n<nch;n++)
            {
                *psData = (short)f1;
                psData++;
            }
        }
        /*
        if (dwSamplesCount>lLen)
        {
            pFile = fopen ( "C:\\Test.raw", "wb" );        
            fwrite ( pAudioData, 2*nch, dwSamplesCount, pFile );
            fclose ( pFile );
            lLen = dwSamplesCount;
        }
        */
        
    }

    //Stockage en XPCM8 pour gagner de la place
    memset ( psamSFX, 0, sizeof(SAM_SFX) );
    psamSFX->bIsUsed                = 1;
    psamSFX->bIsLoaded              = 1;
    psamSFX->bFormat                = (bFormat&sam_FORMAT_CHANNELMASK) | ika_INTERNAL_AUDIO_FORMAT; //sam_FORMAT_MONO_XD4; //XPCM8;//FLOAT32;
    psamSFX->fSampleRate            = (float)dwSampleRate;
    psamSFX->dwSamplesCount         = dwSamplesCount;
    psamSFX->dwLastAccessTimeStamp  = dwInternalTimer;
    psamSFX->dwGranuleCount         = dwSamplesCount;
    psamSFX->fUserDefaultGain       = 1.0F;


    //Nom du SFX
    if (pszSFXName)
        strcpy ( psamSFX->szName, pszSFXName );

    //Allocation et copie
    lReturn = SAM_GranulesAlloc ( 
        psamSFX->dwSamplesCount,
        psamSFX->bFormat,
        &psamSFX->dwGranuleFirst,
        &psamSFX->fReplayGain,
        pAudioData,
        bFormat,
        (float)dwSampleRate,
        &(psamSFX->psyData) );
        
    if (lReturn)
    {
        memset ( psamSFX, 0, sizeof(SAM_SFX) );
        SAM_SFX_LeaveLock ( );        
        return -3; //Plus assez de place dans les granules
    }

    //Le Handle    
    if (pdwAllocatedHandle)
        *pdwAllocatedHandle = dwHandle;
        
    psamSFX->dwLoopBeginPositionSample          = 0;
    psamSFX->dwLoopBeginGranulePositionSample   = 0;
    psamSFX->dwLoopBeginGranuleID               = psamSFX->dwGranuleFirst;
    psamSFX->dwLoopEndPositionSample            = psamSFX->dwSamplesCount-1;
    
    SAM_SFX_LeaveLock ( );
    
    return 0;    
}

DLLEXPORT   long SAM_SFX_Unload ( DWORD dwHandle )
{
    SAM_SFX     *psamSFX;

    if (dwHandle>=samSfxData.dwSFXCount)
        return -1;

    //Le SFX a utiliser
    psamSFX = &(samSfxData.psamSFX[dwHandle]);

    if (psamSFX->lInUseCount)
        return -1;
    
    SAM_SFX_EnterLock ( );

    if ((psamSFX->bIsUsed)&&(psamSFX->bIsLoaded))
    {
        SAM_GranulesFree ( psamSFX->dwGranuleFirst );
        psamSFX->dwGranuleFirst = 0xFFFFFFFF;
        psamSFX->dwGranuleCount = 0;
        psamSFX->bIsLoaded      = 0;
    }

    SAM_SFX_LeaveLock ( );
    
    return 0;
}

DLLEXPORT   long SAM_SFX_Free ( DWORD dwHandle )
{
    SAM_SFX     *psamSFX;

    if (dwHandle>=samSfxData.dwSFXCount)
        return -1;

    //Le SFX a utiliser
    psamSFX = &(samSfxData.psamSFX[dwHandle]);

    if (psamSFX->lInUseCount)
        return -1;
        
    SAM_SFX_EnterLock ( );

    if ((psamSFX->bIsUsed)&&(psamSFX->bIsLoaded))
    {
        SAM_GranulesFree ( psamSFX->dwGranuleFirst );
    }

    memset ( psamSFX, 0, sizeof(SAM_SFX) );
    
    SAM_SFX_LeaveLock ( );

    return 0;
}

DLLEXPORT   long SAM_SFX_GetOldest ( DWORD * pdwOldestHandle, DWORD * pdwReservedHandle, DWORD dwReservedHandleCount )
{
    DWORD       dwIndex;
    DWORD       dwHandle;
    DWORD       dwTimeStamp;
    SAM_SFX     *psamSFX;
    
    SAM_SFX_EnterLock ( );

    //On protège temporairement une liste de Handle
    if ((pdwReservedHandle)&&(dwReservedHandleCount))
    {
        for (dwIndex=0;dwIndex<dwReservedHandleCount;dwIndex++)
        {
            dwHandle = pdwReservedHandle[dwIndex];
            if (dwHandle<samSfxData.dwSFXCount)
            {
                psamSFX = &(samSfxData.psamSFX[dwHandle]);
                if ((psamSFX->bIsUsed)&&(psamSFX->bIsLoaded))
                    psamSFX->bProtected |= 2;
            }
        }
    }


    dwTimeStamp = 0xFFFFFFFF;
    dwHandle    = 0xFFFFFFFF;
    psamSFX = samSfxData.psamSFX;
    for (dwIndex=0;dwIndex<samSfxData.dwSFXCount;dwIndex++,psamSFX++)
    {
        if ((psamSFX->bIsUsed)&&(!psamSFX->bProtected)&&(psamSFX->bIsLoaded))
        {
            if ( (dwTimeStamp==0xFFFFFFFF) || (psamSFX->dwLastAccessTimeStamp<dwTimeStamp) )
            {
                dwHandle    = dwIndex;
                dwTimeStamp = psamSFX->dwLastAccessTimeStamp;
            }

            //Déprotection...
            psamSFX->bProtected = (psamSFX->bProtected&1);
        }
    }
    
    SAM_SFX_LeaveLock ( );

    if (pdwOldestHandle)
        *pdwOldestHandle = dwHandle;

    if (dwHandle==0xFFFFFFFF)
        return -1;
    else                 
        return 0;
}


DLLEXPORT   long SAM_SFX_IsLoaded ( char * pszSFXName, DWORD * pdwAllocatedHandle )
{
    SAM_SFX     *psamSFX;
    DWORD       dwIndex;

    if (!pszSFXName)
        return -1;

    if (strlen(pszSFXName)>255)
        return -1;

    for (dwIndex = 0, psamSFX=samSfxData.psamSFX ;
         dwIndex < samSfxData.dwSFXCount ;
         dwIndex++,psamSFX++)
    {
        if ( (psamSFX->bIsUsed) && 
             (stricmp(pszSFXName,psamSFX->szName)==0) )
        {
            if (pdwAllocatedHandle)
                *pdwAllocatedHandle = dwIndex;

            return 0;
        }
    }

    return -1;
}

DLLEXPORT   long SAM_SFX_GetLoadedMemoryState ( DWORD dwHandle, DWORD * pdwLoadedMemoryState )
{
    SAM_SFX     *psamSFX;
    DWORD       dwLoadedMemoryState;

    if (dwHandle>=samSfxData.dwSFXCount)
        return -1;

    psamSFX = &samSfxData.psamSFX[dwHandle];

    if (!psamSFX->bIsUsed)
        return -1;

    dwLoadedMemoryState = (DWORD)psamSFX->bIsLoaded;
    if (pdwLoadedMemoryState) *pdwLoadedMemoryState = dwLoadedMemoryState;
    
    return 0;
}

DLLEXPORT   long SAM_SFX_GetName ( DWORD dwHandle, char * pszSFXName )
{
    SAM_SFX     *psamSFX;

    if (!pszSFXName)
        return -1;

    if (dwHandle>=samSfxData.dwSFXCount)
        return -1;

    psamSFX = &samSfxData.psamSFX[dwHandle];

    if (!psamSFX->bIsUsed)
        return -1;

    strcpy ( pszSFXName, psamSFX->szName );
    return 0;
}

long SAM_SFX_SetUserDefaultLevel ( DWORD dwHandle, float fDefaultLevel )
{
    SAM_SFX     *psamSFX;
    float       fGain;
    
    fGain = pow ( 10.0F, fDefaultLevel*0.05F );

    if (dwHandle>=samSfxData.dwSFXCount)
        return -1;

    psamSFX = &samSfxData.psamSFX[dwHandle];

    if (!psamSFX->bIsUsed)
        return -1;
        
    psamSFX->fUserDefaultGain = fGain;
    
    return 0;
}
