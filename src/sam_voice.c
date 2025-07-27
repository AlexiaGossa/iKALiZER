#include "sam_header.h"
#include "sam_data.h"
#include "sam_voice.h"

            SAM_VOICE_DATA  samVoiceData;
sam_ALIGN   DWORD           sam_voice_mixsingle_dwMaskPS_0111[4]    = { 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0x00000000 };

/*
##########################################################################################################################################################
##########################################################################################################################################################


    Open/Close VOICE


##########################################################################################################################################################
##########################################################################################################################################################
*/
void SAM_VOICE_PreOpenMemoryNeeds ( DWORD dwVirtualVoicesCount, DWORD *pdwBytesNeeds )
{
    DWORD dwVoiceMirrorSize;
    DWORD dwVoiceSize;

    dwVoiceSize       = sizeof(SAM_VOICE);
    dwVoiceMirrorSize = sizeof(SAM_VOICE_MIRROR);
    
    if (pdwBytesNeeds)
        *pdwBytesNeeds = ( dwVoiceSize       * dwVirtualVoicesCount ) +
                         ( dwVoiceMirrorSize * dwVirtualVoicesCount );
}


long    SAM_VOICE_Open ( DWORD dwVirtualVoicesCount, DWORD dwRealVoicesCount, void * pAllocatedMemory )
{
    DWORD   dwVoiceBytesNeeds;
    DWORD   dwVoiceMirrorBytesNeeds;
    BYTE    *pbAllocatedMemory;

    dwVoiceBytesNeeds               = sizeof(SAM_VOICE) * dwVirtualVoicesCount;
    dwVoiceMirrorBytesNeeds         = sizeof(SAM_VOICE_MIRROR) * dwVirtualVoicesCount;
    pbAllocatedMemory               = (BYTE *)pAllocatedMemory;
    
    samVoiceData.dwFreezeCopyMirror         = 0;
    samVoiceData.dwVoiceVirtualCount        = dwVirtualVoicesCount;
    samVoiceData.dwVoiceRealCountInitial    = dwRealVoicesCount;
    samVoiceData.dwVoiceRealCountCurrent    = dwRealVoicesCount;
    samVoiceData.psamVoice                  = (SAM_VOICE        *)&(pbAllocatedMemory[0]);
    samVoiceData.psamVoiceMirror            = (SAM_VOICE_MIRROR *)&(pbAllocatedMemory[dwVoiceBytesNeeds]);

    memset ( samVoiceData.psamVoice, 0, dwVoiceBytesNeeds );
    memset ( samVoiceData.psamVoiceMirror, 0, dwVoiceMirrorBytesNeeds );
    
    /*
        Allocation de la section critique avec pré-allocation de la zone (bit31=1)
        Fixe le spin count à 4096 avant de passer en mode sommeil
    */
    _OSI_InitializeCriticalSectionAndSpinCount ( &samVoiceData.osiCSMirror, 0x80001000 );
    _OSI_InitializeCriticalSectionAndSpinCount ( &samVoiceData.osiCSFreezeCopyMirror, 0x80001000 );

    return 0;
}

long    SAM_VOICE_Close ( void )
{
    samVoiceData.dwVoiceVirtualCount        = 0;
    samVoiceData.dwVoiceRealCountInitial    = 0;
    samVoiceData.dwVoiceRealCountCurrent    = 0;
    samVoiceData.psamVoice                  = NULL;
    samVoiceData.psamVoiceMirror            = NULL;
    samVoiceData.dwFreezeCopyMirror         = 0;

    _OSI_DeleteCriticalSection ( &samVoiceData.osiCSMirror );
    _OSI_DeleteCriticalSection ( &samVoiceData.osiCSFreezeCopyMirror );

    return 0;
}














/*
##########################################################################################################################################################
##########################################################################################################################################################


    Control exclusive access to voice mirror data


##########################################################################################################################################################
##########################################################################################################################################################
*/


long    SAM_VOICE_LockEnter ( DWORD dwAccess )
{
    if (!(dwAccess&SAM_VOICE_FASTACCESS))
        _OSI_EnterCriticalSection ( &samVoiceData.osiCSMirror );           
        
    return 0;
}

long    SAM_VOICE_LockLeave ( DWORD dwAccess )
{
    if (!(dwAccess&SAM_VOICE_FASTACCESS))
        _OSI_LeaveCriticalSection ( &samVoiceData.osiCSMirror );
        
    return 0;
}

long    SAM_VOICE_MIRROR_FreezeEnable ( void )
{
    _OSI_EnterCriticalSection ( &samVoiceData.osiCSFreezeCopyMirror );
    samVoiceData.dwFreezeCopyMirror += 1;
    _OSI_LeaveCriticalSection ( &samVoiceData.osiCSFreezeCopyMirror );
    return 0;
}

long    SAM_VOICE_MIRROR_FreezeDisable ( void )
{
    _OSI_EnterCriticalSection ( &samVoiceData.osiCSFreezeCopyMirror );
    samVoiceData.dwFreezeCopyMirror -= 1;
    _OSI_LeaveCriticalSection ( &samVoiceData.osiCSFreezeCopyMirror );
    return 0;
}





long    _SAM_VOICE_Reset ( SAM_VOICE_MIRROR *psamVoiceMirror, DWORD dwInternalTimer )
{
    memset ( psamVoiceMirror, 0, sizeof(SAM_VOICE_MIRROR) );
    psamVoiceMirror->dwUpdateFlag = UPDATE_RESET;

    //Données internes
    psamVoiceMirror->bIsUsed                  = 1;
    psamVoiceMirror->dwHandleSFX              = 0xFFFFFFFF;
    psamVoiceMirror->fFX_LevelDistanceGain    = 1.0;
    psamVoiceMirror->fFX_IIR_LowPassRatio     = 1.0;
    psamVoiceMirror->fLevelMasterGain         = 1.0;
    psamVoiceMirror->lLevelGainEnable         = 0;
    psamVoiceMirror->fLevelGainL              = 1.0F;
    psamVoiceMirror->fLevelGainR              = 1.0F;
    

    //Angle de rendu
    SAM_RENDER_GetAngleToRender ( 
        samData.psamRender254Table, 
        0, 
        &psamVoiceMirror->psamRender254,
        &psamVoiceMirror->lRenderIndexTarget );

    //Données externes
    psamVoiceMirror->dwLastTimeUpdated              = dwInternalTimer;
    psamVoiceMirror->dwTimeRenderUpdatedCurrent     = dwInternalTimer;
    psamVoiceMirror->dwTimeRenderUpdatedPrevious    = dwInternalTimer;
    psamVoiceMirror->dwUserID                       = 0xFFFFFFFF;

    //psamVoiceMirror->dwCurrentGranuleID       = 0xFFFFFFFF;

    return 0;
}

long    _SAM_VOICE_GetPlayTick ( float fPlayRate, DWORD * pdwTickIncrement )
{
    DWORD               dwTickIncrement;
    double              f64Ratio;

    f64Ratio = fPlayRate;
    f64Ratio /= (double)samData.dwHardwaremixSampleRate;
    f64Ratio *= (double)(1<<24);
    dwTickIncrement = (DWORD)SAM_MATH_lroundd ( f64Ratio );
    
    if (pdwTickIncrement)
        *pdwTickIncrement = dwTickIncrement;

    return 0;
}


DLLEXPORT   long    SAM_VOICE_GetHandleByUserID ( DWORD * pdwVoiceHandle, DWORD dwStartVoiceHandle, DWORD dwUserID )
{
    DWORD               dwIndex;
    SAM_VOICE_MIRROR    *psamVoiceMirror;
    long                lReturn;

    if (!pdwVoiceHandle)
        return -1;

    if (dwStartVoiceHandle<0)
        dwStartVoiceHandle = 0;

    if (dwStartVoiceHandle>=samVoiceData.dwVoiceVirtualCount)
        return -1;

    lReturn         = -1;
    dwIndex         = dwStartVoiceHandle;
    psamVoiceMirror = samVoiceData.psamVoiceMirror;

    SAM_VOICE_LockEnter ( 0 );
    
    for (;dwIndex<samVoiceData.dwVoiceVirtualCount;dwIndex++,psamVoiceMirror++)
    {
        if ((psamVoiceMirror->bIsUsed)&&(psamVoiceMirror->dwUserID==dwUserID))
        {
            *pdwVoiceHandle = dwIndex;
            lReturn         = 0;
            break;
        }
    }

    SAM_VOICE_LockLeave ( 0 );

    return lReturn;
}


DLLEXPORT   long    SAM_VOICE_GetVoiceUsedCount ( DWORD * pdwVoiceTotalUsedCount, DWORD * pdwVoiceUsedCountUnlooped, DWORD * pdwVoiceUsedCountLooped )
{
    DWORD dwLoopedCount;
    DWORD dwUnloopedCount;

    SAM_VOICE_LockEnter ( 0 );

    dwLoopedCount   = 0;
    dwUnloopedCount = 0;

    SAM_VOICE_LockLeave ( 0 );

    if (pdwVoiceTotalUsedCount)
        *pdwVoiceTotalUsedCount = dwLoopedCount + dwUnloopedCount;

    if (pdwVoiceUsedCountUnlooped)
        *pdwVoiceUsedCountUnlooped = dwUnloopedCount;

    if (pdwVoiceUsedCountLooped)
        *pdwVoiceUsedCountLooped = dwLoopedCount;


    return 0;
}

/*
    Réserve une voice
*/
DLLEXPORT   long    SAM_VOICE_Alloc ( DWORD * pdwVoiceHandle, long lForceAlloc, DWORD * pdwKilledVoiceHandle )
{
    DWORD               dwIndex;
    DWORD               dwFoundVoiceIndex;
    DWORD               dwOldestTime;
    DWORD               dwOldestIndex;
    SAM_VOICE_MIRROR    *psamVoiceMirror;
    DWORD               dwKilledVoiceHandle;
    DWORD               dwInternalTimer;

    if (!pdwVoiceHandle)
        return -1;

    SAM_TIMER_GetIt ( &dwInternalTimer );

    dwKilledVoiceHandle = 0xFFFFFFFF;

    SAM_VOICE_LockEnter ( 0 );

    //Recherche une Voice disponible
    psamVoiceMirror     = samVoiceData.psamVoiceMirror;
    dwFoundVoiceIndex   = 0xFFFFFFFF;
    for (dwIndex=0;dwIndex<samVoiceData.dwVoiceVirtualCount;dwIndex++,psamVoiceMirror++)
    {
        if (!psamVoiceMirror->bIsUsed)
        {
            //On a trouver une Voice !
            dwFoundVoiceIndex = dwIndex;
            dwIndex = samVoiceData.dwVoiceVirtualCount;
        }
    }

    //Nous n'avons rien trouvé, mais nous ne devons pas forcer l'allocation ?
    if ( (dwFoundVoiceIndex==0xFFFFFFFF) && (lForceAlloc==0) )
    {
        SAM_VOICE_LockLeave ( 0 );
        return -1;
    }

    //On a rien trouvé, mais on doit forcer l'allocation !
    if (dwFoundVoiceIndex==0xFFFFFFFF)
    {
        //Recherche la voix la plus ancienne
        dwOldestTime        = 0xFFFFFFFF;
        dwOldestIndex       = 0xFFFFFFFF;
        psamVoiceMirror     = samVoiceData.psamVoiceMirror;
        for (dwIndex=0;dwIndex<samVoiceData.dwVoiceVirtualCount;dwIndex++,psamVoiceMirror++)
        {
            if ( (psamVoiceMirror->dwLastTimeUpdated<dwOldestTime) || (dwIndex==0) )
            {
                if ( ((lForceAlloc&1)&&(psamVoiceMirror->bLoopMode==0)) ||
                     ((lForceAlloc&2)&&(psamVoiceMirror->bLoopMode==1)) )
                {
                    dwOldestTime = psamVoiceMirror->dwLastTimeUpdated;
                    dwOldestIndex = dwIndex;
                }
            }
        }
        if (dwOldestIndex==0xFFFFFFFF)
        {
            SAM_VOICE_LockLeave ( 0 );

            if (pdwKilledVoiceHandle)
                *pdwKilledVoiceHandle = dwKilledVoiceHandle;

            return -1;
        }

        dwFoundVoiceIndex   = dwOldestIndex;
        dwKilledVoiceHandle = dwOldestIndex;
    }

    //La Voice
    psamVoiceMirror = &samVoiceData.psamVoiceMirror[dwFoundVoiceIndex];

    //Remise à zéro
    _SAM_VOICE_Reset ( psamVoiceMirror, dwInternalTimer );

    SAM_VOICE_LockLeave ( 0 );
    
    //Retourne le numéro de la voice
    *pdwVoiceHandle = dwFoundVoiceIndex;

    if (pdwKilledVoiceHandle)
        *pdwKilledVoiceHandle = dwKilledVoiceHandle;

    return 0;
}


DLLEXPORT   long    SAM_VOICE_AllocByVoiceHandle ( DWORD dwVoiceHandle, long lForceAlloc )
{
    SAM_VOICE_MIRROR   *psamVoiceMirror;
    DWORD               dwInternalTimer;
    DWORD               dwVoice;
    DWORD               dwFastAccess;
    
    dwVoice         = dwVoiceHandle&SAM_VOICE_MASK;
    dwFastAccess    = dwVoiceHandle&SAM_VOICE_FASTACCESS;

    if (dwVoice>=samVoiceData.dwVoiceVirtualCount)
        return -1;

    SAM_TIMER_GetIt ( &dwInternalTimer );

    //La Voice
    psamVoiceMirror = &samVoiceData.psamVoiceMirror[dwVoice];

    SAM_VOICE_LockEnter ( dwFastAccess );

    //Il faut forcer l'allocation ?
    if ((psamVoiceMirror->bIsUsed)&&(!lForceAlloc))
    {
        SAM_VOICE_LockLeave ( dwFastAccess );
        return -1;
    }

    //Remise à zéro
    _SAM_VOICE_Reset ( psamVoiceMirror, dwInternalTimer );

    SAM_VOICE_LockLeave ( dwFastAccess );
    return 0;
}

DLLEXPORT   long    SAM_VOICE_AllocByUserID ( DWORD dwUserID, long lForceAlloc )
{
    long                lReturn;
    DWORD               dwVoiceHandle;
    SAM_VOICE_MIRROR    *psamVoiceMirror;
    
    lReturn = SAM_VOICE_Alloc ( 
        &dwVoiceHandle,
        lForceAlloc, NULL );

    if (lReturn)
        return -1;

    psamVoiceMirror = &samVoiceData.psamVoiceMirror[dwVoiceHandle];
    psamVoiceMirror->dwUserID = dwUserID;

    return 0;
}

DLLEXPORT   long    SAM_VOICE_Free ( DWORD dwVoiceHandle )
{
    SAM_VOICE_MIRROR    *psamVoiceMirror;
    DWORD               dwVoice;
    DWORD               dwFastAccess;
    
    dwVoice         = dwVoiceHandle&SAM_VOICE_MASK;
    dwFastAccess    = dwVoiceHandle&SAM_VOICE_FASTACCESS;

    if (dwVoice>=samVoiceData.dwVoiceVirtualCount)
        return -1;

    //La Voice
    psamVoiceMirror = &samVoiceData.psamVoiceMirror[dwVoice];

    SAM_VOICE_LockEnter ( dwFastAccess );

    //Libération
    psamVoiceMirror->dwUpdateFlag |= UPDATE_UNUSED;
    psamVoiceMirror->bIsUsed       = 0;
    psamVoiceMirror->bIsPlay       = 0;
    

    SAM_VOICE_LockLeave ( dwFastAccess );
    
    return 0;
}

DLLEXPORT   long    SAM_VOICE_FreeByUserID ( DWORD dwUserID )
{
    DWORD dwVoiceHandle, dwStartVoiceHandle;
    long lReturn;

    SAM_VOICE_LockEnter ( 0 );

    dwStartVoiceHandle = 0;
    lReturn = -1;

    do {
        if (SAM_VOICE_GetHandleByUserID ( &dwVoiceHandle, dwStartVoiceHandle, dwUserID ))
        {
            SAM_VOICE_LockLeave ( 0 );
            return lReturn;
        }
        SAM_VOICE_Free ( dwVoiceHandle );
        dwStartVoiceHandle = dwVoiceHandle + 1;
        lReturn = 0;
    } while (1);

    //SAM_VOICE_LockLeave ( );
    return 0;
}

/*

    Attention à cette fonction :
    La voix sur laquelle le SFX est appliqué est stoppé ET remise au début
    Le LoopMode est conservé (par défaut 0), tout comme les autres paramètres, ils ne sont pas modifiés !
    
*/
DLLEXPORT   long    SAM_VOICE_SetSFX ( DWORD dwVoiceHandle, DWORD dwHandleSFX )
{
    SAM_VOICE_MIRROR    *psamVoiceMirror;
    SAM_SFX             *psamSFX;
    DWORD               dwTickIncrement;
    DWORD               dwInternalTimer;
    DWORD               dwVoice;
    DWORD               dwFastAccess;
    
    dwVoice         = dwVoiceHandle&SAM_VOICE_MASK;
    dwFastAccess    = dwVoiceHandle&SAM_VOICE_FASTACCESS;
    
    SAM_TIMER_GetIt ( &dwInternalTimer );    

    if (dwVoice>=samVoiceData.dwVoiceVirtualCount)
        return -1;

    //Les infos du SFX
    if (SAM_SFX_GetIt ( dwHandleSFX, &psamSFX ))
        return -1;

    if (!psamSFX->bIsUsed)
        return -1;

    //La Voice
    SAM_VOICE_LockEnter ( dwFastAccess );

    psamVoiceMirror = &samVoiceData.psamVoiceMirror[dwVoice];

    if (!psamVoiceMirror->bIsUsed)
    {
        SAM_VOICE_LockLeave ( dwFastAccess );
        return -1;
    }

    //Recherche du meilleur FIR à appliquer et du Tick
    _SAM_VOICE_GetPlayTick ( psamSFX->fSampleRate, &dwTickIncrement );

    //On fixe les variables propres au SFX
    psamVoiceMirror->dwUpdateFlag |= UPDATE_SFX | UPDATE_PLAYRATE | UPDATE_PLAY;

    psamVoiceMirror->dwHandleSFX                = dwHandleSFX;
    psamVoiceMirror->dwInPlayTickIncrement      = dwTickIncrement;
    psamVoiceMirror->dwInPlaySampleRate         = (DWORD)psamSFX->fSampleRate;
    psamVoiceMirror->bIsPlay                    = 0;               

    /*
    psamVoice->dwCurrentGranuleID               = psamSFX->dwGranuleFirst;
    psamVoice->dwInPlayGranulePosition          = 0;
    psamVoice->dwInPlaySamplePosition           = 0;
    psamVoice->dwInPlaySamplePositionPrevious   = 0;
    psamVoice->dwInPlayTickPosition             = 0;
    psamVoice->fFX_IIR_LowPassValue             = 0;
    psamVoice->fLevelSFXReplayGain              = psamSFX->fReplayGain;

    //Le FIR
    psamVoice->resample_lFIRStackPosition       = 0;
    psamVoice->resample_lFIRLength              = samData.lFIRLenght;

    //On vide les variables
    memset ( psamVoice->stack_fBufferStackValue, 0, sam_VOICEBUFFERSTACK_COUNT * sizeof(float) );
    memset ( psamVoice->resample_fFIRStackValue, 0, 256 * sizeof(float) );
    psamVoice->resample_fLastValue[0] = 0;
    psamVoice->resample_fLastValue[1] = 0;

    */

    SAM_VOICE_LockLeave ( dwFastAccess );

    //On met à jour la Voice
    psamVoiceMirror->dwLastTimeUpdated                = dwInternalTimer;

    return 0;
}

DLLEXPORT   long    SAM_VOICE_SetSampleRate ( DWORD dwVoiceHandle, DWORD dwSampleRate_Hz )
{
    SAM_VOICE_MIRROR    *psamVoiceMirror;
    DWORD               dwTickIncrement;
    DWORD               dwVoice;
    DWORD               dwFastAccess;
    
    dwVoice         = dwVoiceHandle&SAM_VOICE_MASK;
    dwFastAccess    = dwVoiceHandle&SAM_VOICE_FASTACCESS;
    
    if (dwVoice>=samVoiceData.dwVoiceVirtualCount)
        return -1;

    //Précalcul
    _SAM_VOICE_GetPlayTick ( (float)dwSampleRate_Hz, &dwTickIncrement );

    SAM_VOICE_LockEnter ( dwFastAccess );

    //La Voice
    psamVoiceMirror = &samVoiceData.psamVoiceMirror[dwVoice];
    if (!psamVoiceMirror->bIsUsed)
    {
        SAM_VOICE_LockLeave ( dwFastAccess );
        return -1;
    }

    //On fixe les données
    psamVoiceMirror->dwUpdateFlag          |= UPDATE_PLAYRATE;
    psamVoiceMirror->dwInPlayTickIncrement  = dwTickIncrement;
    psamVoiceMirror->dwInPlaySampleRate     = dwSampleRate_Hz;

    SAM_VOICE_LockLeave ( dwFastAccess );
    return 0;
}

DLLEXPORT   long    SAM_VOICE_SetLoop ( DWORD dwVoiceHandle, long bLoop )
{
    SAM_VOICE_MIRROR    *psamVoiceMirror;
    DWORD               dwVoice;
    DWORD               dwFastAccess;
    
    dwVoice         = dwVoiceHandle&SAM_VOICE_MASK;
    dwFastAccess    = dwVoiceHandle&SAM_VOICE_FASTACCESS;

    if (dwVoice>=samVoiceData.dwVoiceVirtualCount)
        return -1;

    SAM_VOICE_LockEnter ( dwFastAccess );

    //La Voice
    psamVoiceMirror = &samVoiceData.psamVoiceMirror[dwVoice];
    if (!psamVoiceMirror->bIsUsed)
    {
        SAM_VOICE_LockLeave ( dwFastAccess );
        return -1;
    }

    //On fixe les données
    psamVoiceMirror->bLoopMode      = bLoop;

    SAM_VOICE_LockLeave ( dwFastAccess );
    return 0;
}

DLLEXPORT   long    SAM_VOICE_SetMasterLevel ( DWORD dwVoiceHandle, float fMasterLevel_dB )
{
    SAM_VOICE_MIRROR    *psamVoiceMirror;
    float               fGain;
    DWORD               dwVoice;
    DWORD               dwFastAccess;
    
    dwVoice         = dwVoiceHandle&SAM_VOICE_MASK;
    dwFastAccess    = dwVoiceHandle&SAM_VOICE_FASTACCESS;

    if (dwVoice>=samVoiceData.dwVoiceVirtualCount)
        return -1;

    //Précalcul
    fGain = (float)pow ( 10, fMasterLevel_dB * 0.05 );

    SAM_VOICE_LockEnter ( dwFastAccess );

    //La Voice
    psamVoiceMirror = &samVoiceData.psamVoiceMirror[dwVoice];
    if (!psamVoiceMirror->bIsUsed)
    {
        SAM_VOICE_LockLeave ( dwFastAccess );
        return -1;
    }

    //On fixe les données
    psamVoiceMirror->fLevelMasterGain   = fGain;

    SAM_VOICE_LockLeave ( dwFastAccess );
    return 0;
}

DLLEXPORT   long    SAM_VOICE_SetDistanceLevel ( DWORD dwVoiceHandle, float fDistanceLevel_dB )
{
    SAM_VOICE_MIRROR    *psamVoiceMirror;
    float               fGain;
    DWORD               dwVoice;
    DWORD               dwFastAccess;
    
    dwVoice         = dwVoiceHandle&SAM_VOICE_MASK;
    dwFastAccess    = dwVoiceHandle&SAM_VOICE_FASTACCESS;

    if (dwVoice>=samVoiceData.dwVoiceVirtualCount)
        return -1;

    //Précalcul
    fGain = (float)pow ( 10, fDistanceLevel_dB * 0.05 );

    SAM_VOICE_LockEnter ( dwFastAccess );

    //La Voice
    psamVoiceMirror = &samVoiceData.psamVoiceMirror[dwVoice];
    if (!psamVoiceMirror->bIsUsed)
    {
        SAM_VOICE_LockLeave ( dwFastAccess );
        return -1;
    }

    //On fixe les données
    psamVoiceMirror->fFX_LevelDistanceGain  = fGain;

    SAM_VOICE_LockLeave ( dwFastAccess );
    return 0;
}

DLLEXPORT   long    SAM_VOICE_SetRatioIIR ( DWORD dwVoiceHandle, float fRatioIIR )
{
    SAM_VOICE_MIRROR    *psamVoiceMirror;
    DWORD               dwVoice;
    DWORD               dwFastAccess;
    
    dwVoice         = dwVoiceHandle&SAM_VOICE_MASK;
    dwFastAccess    = dwVoiceHandle&SAM_VOICE_FASTACCESS;

    if (dwVoice>=samVoiceData.dwVoiceVirtualCount)
        return -1;

    SAM_VOICE_LockEnter ( dwFastAccess );

    //La Voice
    psamVoiceMirror = &samVoiceData.psamVoiceMirror[dwVoice];
    if (!psamVoiceMirror->bIsUsed)
    {
        SAM_VOICE_LockLeave ( dwFastAccess );
        return -1;
    }

    //On fixe les données
    psamVoiceMirror->fFX_IIR_LowPassRatio   = fRatioIIR;

    SAM_VOICE_LockLeave ( dwFastAccess );
    return 0;
}

DLLEXPORT   long    SAM_VOICE_SetPlay ( DWORD dwVoiceHandle, long bPlay )
{
    SAM_VOICE_MIRROR    *psamVoiceMirror;
    DWORD               dwVoice;
    DWORD               dwFastAccess;
    
    dwVoice         = dwVoiceHandle&SAM_VOICE_MASK;
    dwFastAccess    = dwVoiceHandle&SAM_VOICE_FASTACCESS;

    if (dwVoice>=samVoiceData.dwVoiceVirtualCount)
        return -1;

    SAM_VOICE_LockEnter ( dwFastAccess );

    //La Voice
    psamVoiceMirror = &samVoiceData.psamVoiceMirror[dwVoice];
    if (!psamVoiceMirror->bIsUsed)
    {
        SAM_VOICE_LockLeave ( dwFastAccess );
        return -1;
    }

    //On fixe les données
    psamVoiceMirror->dwUpdateFlag           |= UPDATE_PLAY;
    psamVoiceMirror->bIsPlay                = bPlay;
    
    //BugFix : Cela permet de renvoyer que la voix est active même si n'est pas encore mixée.
    // Sinon, l'appelant croit qu'elle est terminée et la supprime et donc, pas de son...
    psamVoiceMirror->bIsPlayedState         = 1;                

    SAM_VOICE_LockLeave ( dwFastAccess );
    return 0;
}

DLLEXPORT   long    SAM_VOICE_SetOrigin ( DWORD dwVoiceHandle, long lAngleDegrees, float fDistanceMeters )
{
    SAM_VOICE_MIRROR    *psamVoiceMirror;
    SAM_RENDER254       *psamRender;
    DWORD               dwVoice;
    DWORD               dwFastAccess;
    long                lRenderIndexTarget;
    DWORD               dwInternalTimer;
    
    dwVoice         = dwVoiceHandle&SAM_VOICE_MASK;
    dwFastAccess    = dwVoiceHandle&SAM_VOICE_FASTACCESS;

    if (dwVoice>=samVoiceData.dwVoiceVirtualCount)
        return -1;
        
    SAM_TIMER_GetIt ( &dwInternalTimer );

    SAM_VOICE_LockEnter ( dwFastAccess );

    //La Voice
    psamVoiceMirror = &samVoiceData.psamVoiceMirror[dwVoice];
    if (!psamVoiceMirror->bIsUsed)
    {
        SAM_VOICE_LockLeave ( dwFastAccess );
        return -1;
    }

        
    //if (samData.dwDynamicDelayLinesCount==0xFFFFFFFE)
    if (0)
    {
        /*SAM_RENDER_Processor_CreateRender ( 
            samData.dwHardwaremixSampleRate,
            &(psamVoiceMirror->samRenderDLFprocessor),
            lAngleDegrees,
            fDistanceMeters );
            
        psamVoiceMirror->psamRender = &(psamVoiceMirror->samRenderDLFprocessor);
        */
    }
    else
    {
        //Précalcul
        SAM_RENDER_GetAngleToRender ( 
            samData.psamRender254Table, 
            lAngleDegrees, 
            &psamRender,
            &lRenderIndexTarget );
    }
    
    //On fixe les données
    psamVoiceMirror->psamRender254                  = psamRender;
    psamVoiceMirror->lRenderIndexTarget             = lRenderIndexTarget;
    psamVoiceMirror->dwTimeRenderUpdatedPrevious    = psamVoiceMirror->dwTimeRenderUpdatedCurrent;
    psamVoiceMirror->dwTimeRenderUpdatedCurrent     = dwInternalTimer;
    psamVoiceMirror->lRenderOrderChangeFlag         = 1; 
    
    if (fDistanceMeters<0.0F) fDistanceMeters = 0.0F;

#ifdef _DEBUG
    _SAM_DEBUG_TEXT_ ( "Dist = %f", fDistanceMeters );
#endif    
    
    if ( (fDistanceMeters<0.02F) &&
         (samData.dwChannelMode==0x26) )
    {
    
        psamVoiceMirror->fLevelGainL        = 1.0F;
        psamVoiceMirror->fLevelGainR        = 1.0F;
        psamVoiceMirror->lLevelGainEnable   = 1;
    }
    else psamVoiceMirror->lLevelGainEnable  = 0;
    
    psamVoiceMirror->fDistanceMeters    = fDistanceMeters;
    psamVoiceMirror->lAngleDegrees      = lAngleDegrees;

    SAM_VOICE_LockLeave ( dwFastAccess );
    return 0;
}

DLLEXPORT   long    SAM_VOICE_GetUserData ( DWORD dwVoiceHandle, BYTE * pbReceiveBuffer, DWORD dwByteCount )
{
    SAM_VOICE_MIRROR    *psamVoiceMirror;
    long                bIsUsed;
    DWORD               dwVoice;
    DWORD               dwFastAccess;
    
    dwVoice         = dwVoiceHandle&SAM_VOICE_MASK;
    dwFastAccess    = dwVoiceHandle&SAM_VOICE_FASTACCESS;

    if (dwVoice>=samVoiceData.dwVoiceVirtualCount)
        return -1;

    //La Voice
    SAM_VOICE_LockEnter ( dwFastAccess );
    psamVoiceMirror = &samVoiceData.psamVoiceMirror[dwVoice];
    bIsUsed = psamVoiceMirror->bIsUsed;
    SAM_VOICE_LockLeave ( dwFastAccess );
    if (!bIsUsed) return -1;

    if ((pbReceiveBuffer)&&(dwByteCount>0)&&(dwByteCount<=sam_VOICEUSERDATACOUNT))
    {
        memcpy ( pbReceiveBuffer, psamVoiceMirror->bUserData, dwByteCount );
        return 0;
    }

    return -1;
}

DLLEXPORT   long    SAM_VOICE_SetUserData ( DWORD dwVoiceHandle, BYTE * pbSendBuffer, DWORD dwByteCount )
{
    SAM_VOICE_MIRROR    *psamVoiceMirror;
    long                bIsUsed;
    DWORD               dwVoice;
    DWORD               dwFastAccess;
    
    dwVoice         = dwVoiceHandle&SAM_VOICE_MASK;
    dwFastAccess    = dwVoiceHandle&SAM_VOICE_FASTACCESS;

    if (dwVoice>=samVoiceData.dwVoiceVirtualCount)
        return -1;

    //La Voice
    SAM_VOICE_LockEnter ( dwFastAccess );
    psamVoiceMirror = &samVoiceData.psamVoiceMirror[dwVoice];
    bIsUsed = psamVoiceMirror->bIsUsed;
    SAM_VOICE_LockLeave ( dwFastAccess );
    if (!bIsUsed) return -1;

    if ((pbSendBuffer)&&(dwByteCount>0)&&(dwByteCount<=sam_VOICEUSERDATACOUNT))
    {
        memcpy ( psamVoiceMirror->bUserData, pbSendBuffer, dwByteCount );
        return 0;
    }

    return -1;
}

DLLEXPORT   long    SAM_VOICE_GetUserID ( DWORD dwVoiceHandle, DWORD * pdwUserID )
{
    SAM_VOICE_MIRROR    *psamVoiceMirror;
    long                bIsUsed;
    DWORD               dwVoice;
    DWORD               dwFastAccess;
    
    dwVoice         = dwVoiceHandle&SAM_VOICE_MASK;
    dwFastAccess    = dwVoiceHandle&SAM_VOICE_FASTACCESS;

    if (dwVoice>=samVoiceData.dwVoiceVirtualCount)
        return -1;

    if (!pdwUserID)
        return -1;

    //La Voice
    SAM_VOICE_LockEnter ( dwFastAccess );
    psamVoiceMirror = &samVoiceData.psamVoiceMirror[dwVoice];
    bIsUsed = psamVoiceMirror->bIsUsed;
    SAM_VOICE_LockLeave ( dwFastAccess );
    if (!bIsUsed) return -1;

    *pdwUserID = psamVoiceMirror->dwUserID;

    return 0;
}

DLLEXPORT   long    SAM_VOICE_SetUserID ( DWORD dwVoiceHandle, DWORD dwUserID )
{
    SAM_VOICE_MIRROR    *psamVoiceMirror;
    long                bIsUsed;
    DWORD               dwVoice;
    DWORD               dwFastAccess;
    
    dwVoice         = dwVoiceHandle&SAM_VOICE_MASK;
    dwFastAccess    = dwVoiceHandle&SAM_VOICE_FASTACCESS;

    if (dwVoice>=samVoiceData.dwVoiceVirtualCount)
        return -1;

    //La Voice
    SAM_VOICE_LockEnter ( dwFastAccess );
    psamVoiceMirror = &samVoiceData.psamVoiceMirror[dwVoice];
    bIsUsed = psamVoiceMirror->bIsUsed;
    SAM_VOICE_LockLeave ( dwFastAccess );
    if (!bIsUsed) return -1;

    psamVoiceMirror->dwUserID = dwUserID;

    return 0;
}

