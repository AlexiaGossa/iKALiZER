#include "sam_header.h"
#include "sam_data.h"
#include "stdio.h"
#include "sam_sfx.h"


extern SAM_GRANULESDATA samGranulesData;




/*

    Renvoi -1 si erreur normale
           -2 si erreur car il n'y plus assez d'espace libre dans les handles
           -3 si erreur car plus assez d'espace libre dans les granules

    Les SFX dynamiques ont une particularité : on peut mettre à jour leurs données en temps réel !
    Cela implique juste une petite modification au niveau du chargement et de la psycho-acoustique : 
    - Ne pas maximiser le signal
    - Fixer en dur la psycho-acoustique à 0dB OU la mettre à jour à chaque écriture
*/
DLLEXPORT   long SAM_SFX_DynamicLoad ( DWORD dwHandle, char * pszSFXName, DWORD dwSampleRate, DWORD dwSamplesCount, BYTE bAudioFormat, void * pAudioData, DWORD * pdwAllocatedHandle )
{
    DWORD       dwIndex;
    DWORD       dwSampleBytesCount;
    DWORD       dwSFXBytesCount;
    SAM_SFX     *psamSFX;
    BYTE        bFormat;
    DWORD       dwInternalTimer;
    long        lReturn;
    long        lPsyPointDuration, i;

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
    bFormat = bAudioFormat;
    /*
    switch (b16bits)
    {
        case  0: 
        case  8: bFormat = sam_FORMAT_MONO_PCM8; break;
        case  1: 
        case 16: bFormat = sam_FORMAT_MONO_PCM16; break;
        case 32: bFormat = sam_FORMAT_MONO_FLOAT32; break;
    }
    if (bStereo) bFormat|= 0x10;
    */

    //Taille du SFX avant le stockage
    SAM_FormatGetBytesCount ( bFormat, &dwSampleBytesCount );
    dwSFXBytesCount = dwSamplesCount * dwSampleBytesCount;

    //Stockage en PCM16
    memset ( psamSFX, 0, sizeof(SAM_SFX) );
    psamSFX->bIsUsed                = 1;
    psamSFX->bIsLoaded              = 1;
    psamSFX->bFormat                = (bFormat&sam_FORMAT_CHANNELMASK) | sam_FORMAT_MONO_PCM16; //sam_FORMAT_MONO_XD4; //XPCM8;//FLOAT32;
    psamSFX->fSampleRate            = (float)dwSampleRate;
    psamSFX->dwSamplesCount         = dwSamplesCount;
    psamSFX->dwLastAccessTimeStamp  = dwInternalTimer;
    psamSFX->dwGranuleCount         = dwSamplesCount;
    psamSFX->fReplayGain            = 1.0F;
    psamSFX->fUserDefaultGain       = 1.0F;
    
    //Par défaut, on a une précision de 20ms
    lPsyPointDuration = (long)floor(psamSFX->fSampleRate * 0.020F);
    
    //Nombre de points pour 20ms de précision
    i = dwSamplesCount / (DWORD)lPsyPointDuration;
    if (i>1023) //Le granule sera plein !
        lPsyPointDuration = (dwSamplesCount/1023)+1;
        
    //Stocke la longueur d'un point
    psamSFX->psyData.dwPsyPointDuration            = (DWORD)lPsyPointDuration;
    psamSFX->psyData.dwPsyPointDuration_Mul_Shr10  = (DWORD)SAM_MATH_lroundf ( 1024.0F / (float)(lPsyPointDuration) );

    //Nom du SFX
    if (pszSFXName)
        strcpy ( psamSFX->szName, pszSFXName );
    
    //Allocation et copie
    lReturn = SAM_GranulesAlloc ( 
        psamSFX->dwSamplesCount,
        psamSFX->bFormat,
        &psamSFX->dwGranuleFirst,
        NULL, //&psamSFX->fReplayGain,
        pAudioData,
        bFormat,
        (float)dwSampleRate,
        NULL ); //&(psamSFX->psyData) );
        
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

DLLEXPORT   long SAM_SFX_DynamicWrite ( DWORD dwHandle, DWORD dwSamplesOffset, DWORD dwSamplesCount, BYTE bAudioFormat, void * pAudioData )
{
    SAM_SFX     *psamSFX;
    DWORD       dwSamplesPerGranuleCount;
    DWORD       dwSamplesPerGranuleCountSource;
    DWORD       dwKeepSamples;
    DWORD       dwCurrentGranuleID;
    DWORD       dwTemp;
    DWORD       dwOffsetGranule;
    DWORD       dwWritesCount;
    BYTE        *pbSourceData;
    DWORD       dwBytesPerSampleCount;
    long        lRetour;
    
    if (dwHandle>=samSfxData.dwSFXCount)
        return -1;

    //Le SFX a utiliser
    psamSFX = &(samSfxData.psamSFX[dwHandle]);

    if (psamSFX->lInUseCount)
        return -1;

    SAM_SFX_EnterLock ( );
    
    
    //Nombre de samples par granules
    SAM_FormatGetBytesCount ( psamSFX->bFormat, &dwBytesPerSampleCount );
    SAM_FormatGetBytesCount ( bAudioFormat, &dwSamplesPerGranuleCountSource );
    dwSamplesPerGranuleCount = dwSamplesPerGranuleCountSource;
    
    
    //Vérifie la position "offset"
    if (dwSamplesOffset>=psamSFX->dwSamplesCount)
    {
        SAM_SFX_LeaveLock ( );
        return -1;
    }
    
    //Vérifie le nombre de samples à écrire sans débordement
    dwKeepSamples = psamSFX->dwSamplesCount - dwSamplesOffset;
    if (dwSamplesCount>=dwKeepSamples)
        dwSamplesCount = dwKeepSamples;
    if (dwSamplesCount==0)
    {
        //On a rien à écrire...
        SAM_SFX_LeaveLock ( );
        return -1;
    }
    
    
    //Recherche le premier granule à écrire
    dwCurrentGranuleID  = psamSFX->dwGranuleFirst;
    dwTemp              = dwSamplesOffset / dwSamplesPerGranuleCount;
    dwOffsetGranule     = dwSamplesOffset % dwSamplesPerGranuleCount;
    if (dwTemp)
    {
        do {
            dwTemp--;
            SAM_GranulesGetNext ( dwCurrentGranuleID, &dwCurrentGranuleID );
        } while (dwTemp);
    }
    
    //Conversion et écriture des données audio
    pbSourceData = (BYTE *)pAudioData;
    do {
        //Nombre d'échantillons à écrire dans ce granule
        dwWritesCount = dwSamplesPerGranuleCount-dwOffsetGranule;
        if (dwWritesCount>=dwSamplesCount) dwWritesCount = dwSamplesCount;
        
        //Copie des données
        SAM_CopyAudioData (
            samGranulesData.pGranule[dwCurrentGranuleID].bAudioData, psamSFX->bFormat,
            pbSourceData, bAudioFormat, dwWritesCount, 1.0F );

        //On déplace le pointeur des données            
        pbSourceData += dwSamplesPerGranuleCountSource * dwWritesCount;
        
        //On se déplace sur le granule suivant
        lRetour = SAM_GranulesGetNext ( dwCurrentGranuleID, &dwCurrentGranuleID );
        
        //On détermine le nombre de données restantes
        dwSamplesCount -= dwWritesCount;
        
    } while ((lRetour==0)&&(dwSamplesCount>0));
        
    SAM_SFX_LeaveLock ( );
    
    return 0;
}

