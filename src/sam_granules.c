#include "sam_header.h"

//Un granule... 4Ko
//Une allocation de 32Mo alloue réellement 32640Ko au lieu de 32768Ko, la différence (128Ko pour 8192 granules) permet de gérer les granules.
//Avec 32Mo, on peut stocker 25 minutes de sons à 22KHz/Mono/XPCM ou 6 minutes de sons à 22KHz/Mono/32bits


SAM_GRANULESDATA samGranulesData;

long    SAM_GranulesOpen ( DWORD dwTotalNeedsBytesCount, void * pAllocatedMemory )
{   
    DWORD dwIndex;
    
    samGranulesData.dwGranulesCount = dwTotalNeedsBytesCount / sizeof(SAM_GRANULE);// / (dwTotalNeedsMegaBytes<<20);
    samGranulesData.pGranule = (SAM_GRANULE *)pAllocatedMemory; //SAM_ALLOC ( samGranulesData.dwGranulesCount * sizeof(SAM_GRANULE) );
    memset ( samGranulesData.pGranule, 0, samGranulesData.dwGranulesCount * sizeof(SAM_GRANULE) );
    
    samGranulesData.dwRealAudioBytesInGranule = sam_GRANULE_BUFFERBYTES;
    samGranulesData.dwFirstFreeIdGranule = 0;
    samGranulesData.dwFreeBytes = samGranulesData.dwRealAudioBytesInGranule * samGranulesData.dwGranulesCount;
    for (dwIndex=0;dwIndex<samGranulesData.dwGranulesCount;dwIndex++)
    {
        samGranulesData.pGranule[dwIndex].dwIdGranuleCurrent    = dwIndex;
        samGranulesData.pGranule[dwIndex].dwIdGranuleNext       = (dwIndex<samGranulesData.dwGranulesCount-1)?(dwIndex+1):(0xFFFFFFFF);
        samGranulesData.pGranule[dwIndex].dwIdGranulePrevious   = (dwIndex>0)?(dwIndex-1):(0xFFFFFFFF);
        samGranulesData.pGranule[dwIndex].wBytesCount           = sam_GRANULE_BUFFERBYTES;
        samGranulesData.pGranule[dwIndex].wSamplesCount         = 0;
    }
    
    return 0;
}

long    SAM_GranulesClose ( void )
{
    if (samGranulesData.pGranule)
    {    
        //SAM_FREE ( samGranulesData.pGranule );
        samGranulesData.pGranule = NULL;
        samGranulesData.dwGranulesCount = 0;
    }
    
    return 0;
}

long    SAM_GranulesFree ( DWORD dwIdGranule )
{
    DWORD dwIndex;
    DWORD dwIndexLast;
 
    //Libération des granules   
    dwIndex = dwIdGranule;
    
    do {
        samGranulesData.pGranule[dwIndex].wSamplesCount = 0;
        dwIndexLast                     = dwIndex;
        dwIndex                         = samGranulesData.pGranule[dwIndex].dwIdGranuleNext;
        samGranulesData.dwFreeBytes     += samGranulesData.dwRealAudioBytesInGranule;
    } while (dwIndex!=0xFFFFFFFF);
    
    //On ajoute les granules aux granules déjà libres...
    samGranulesData.pGranule[dwIndexLast].dwIdGranuleNext = samGranulesData.dwFirstFreeIdGranule;
    samGranulesData.dwFirstFreeIdGranule = dwIdGranule;
    
    return 0;   
}


long    SAM_GranulesAlloc ( DWORD dwTotalSamplesCount, BYTE bFormat, DWORD * pdwIdGranule, float *pfInvertedGain, void * pAudioDataSource, BYTE bFormatSource, float fSampleRate, SAM_SFX_PSY * ppsyData )
{
    DWORD dwBytesPerSample;
    DWORD dwBytesNeeds;
    DWORD dwIndex;
    DWORD dwIndexInitial;
    DWORD dwIndexNext;
    BYTE * pAudioDataSourceCurrent;
    DWORD dwSamplesCount;
    DWORD dwGranuleSamplesCount;
    DWORD dwGranulesNeeds;
    DWORD dwSourceIncr;
    float fGain;
    DWORD dwAvailableSamplesCount;


    dwAvailableSamplesCount = dwTotalSamplesCount;
    
    //Détermine l'espace nécessaire
    SAM_FormatGetBytesCount ( bFormat, &dwBytesPerSample );
    dwBytesNeeds    = dwBytesPerSample*dwTotalSamplesCount;
    dwGranulesNeeds = dwBytesNeeds / samGranulesData.dwRealAudioBytesInGranule;
    if (dwBytesNeeds%samGranulesData.dwRealAudioBytesInGranule) dwGranulesNeeds++;
    if (samGranulesData.dwFreeBytes<dwBytesNeeds)
    {
        SAM_ErrorAddMessage ( "Not enough free granules in memory." );
        return -1;
    }
    
    //Allocation et remplissage des granules
    dwIndexInitial  = samGranulesData.dwFirstFreeIdGranule;
    dwIndex         = dwIndexInitial;
    
    //Nombre de samples par granules
    dwGranuleSamplesCount = samGranulesData.dwRealAudioBytesInGranule / dwBytesPerSample;

    //L'incrementation de la source...
    SAM_FormatGetBytesCount ( bFormatSource, &dwSourceIncr );
    dwSourceIncr *= dwGranuleSamplesCount;

    //Le pointeur de la source    
    pAudioDataSourceCurrent = (BYTE *)pAudioDataSource;
    
    //Mesure le gain à appliquer (trés utile en mode XPCM)
    if ((pfInvertedGain)&&(ppsyData))
        SAM_PreCopyAudioData ( pAudioDataSource, bFormatSource, dwTotalSamplesCount, fSampleRate, &fGain, ppsyData );
    else
        fGain = 1.0F;
    
    do {
        dwGranulesNeeds--;
        
        //Nombre de samples à copier
        if (dwAvailableSamplesCount>dwGranuleSamplesCount)
            dwSamplesCount = (long)dwGranuleSamplesCount;
        else
            dwSamplesCount = dwAvailableSamplesCount;
            
        //Copie des samples
        SAM_CopyAudioData (
            samGranulesData.pGranule[dwIndex].bAudioData, bFormat,
            pAudioDataSourceCurrent, bFormatSource, dwSamplesCount, fGain );
            
        //On déplace le pointeur source
        pAudioDataSourceCurrent += dwSourceIncr;
        
        //On détermine le nombre de samples restant
        dwAvailableSamplesCount -= dwSamplesCount;
        
        //On se positionne sur le prochain granule
        dwIndexNext = samGranulesData.pGranule[dwIndex].dwIdGranuleNext;
        if (!dwGranulesNeeds) samGranulesData.pGranule[dwIndex].dwIdGranuleNext = 0xFFFFFFFF;
        
        //La quantité de mémoire libre diminue...
        samGranulesData.dwFreeBytes -= samGranulesData.dwRealAudioBytesInGranule;
        
        dwIndex = dwIndexNext;
    } while (dwGranulesNeeds);

    //On redétermine le premier granule de libre    
    samGranulesData.dwFirstFreeIdGranule = dwIndex;
    
    //On renvoi le granule de départ...
    *pdwIdGranule               = dwIndexInitial;

    //On renvoi le gain inverse    
    if (pfInvertedGain)
        *pfInvertedGain             = 1/fGain;
   
    return 0;
}



long    SAM_GranulesGetData ( DWORD dwIdGranule, void ** pData )
{
    if (dwIdGranule>=samGranulesData.dwGranulesCount)
        return -1;

    if (pData)
        *pData = samGranulesData.pGranule[dwIdGranule].bAudioData;

    return 0;
}

long    SAM_GranulesGetNext_Fast ( DWORD dwIdGranule, DWORD * pdwIdGranuleNext )
{
    __asm {
                mov         edx, dwIdGranule
                mov         eax, 1
                
                shl         edx, 12
                mov         ebx, pdwIdGranuleNext
                
                xor         eax, eax
                mov         ecx, [edx+SAM_GRANULE.dwIdGranuleNext]
                
                mov         [ebx], ecx
    }
}
                
                
                
                
                
    
    
long    SAM_GranulesGetNext ( DWORD dwIdGranule, DWORD * pdwIdGranuleNext )
{
    DWORD dwIdGranuleNext;

    if (dwIdGranule>=samGranulesData.dwGranulesCount)
        return -1;

    dwIdGranuleNext = samGranulesData.pGranule[dwIdGranule].dwIdGranuleNext;

    if (pdwIdGranuleNext)
        *pdwIdGranuleNext = dwIdGranuleNext;
    
    return 0;

}











