#include "sam_header.h"
#include "sam_data.h"
#include "Windows.h"

void SAM_EnterProtectThreadMixer ( void )
{
    long bWait;
    
    _OSI_EnterCriticalSection ( &samData.osiCriticalSection );
    samData.bInfiniteLoopWait = 1;
    _OSI_LeaveCriticalSection ( &samData.osiCriticalSection );
    
    bWait = 1;
    
    do {
        _OSI_Sleep ( 1 );
        
        _OSI_EnterCriticalSection ( &samData.osiCriticalSection );
        if (samData.bInfiniteLoopWait==2) bWait = 0;
        _OSI_LeaveCriticalSection ( &samData.osiCriticalSection );
    } while (bWait);
}

void SAM_LeaveProtectThreadMixer ( void )
{
    _OSI_EnterCriticalSection ( &samData.osiCriticalSection );
    samData.bInfiniteLoopWait = 0;
    _OSI_LeaveCriticalSection ( &samData.osiCriticalSection );
}

DWORD GetBufferPositionDelta ( DWORD dwCurrent, DWORD dwLast, DWORD dwTotal )
{
    if (dwLast<=dwCurrent)
        return dwCurrent - dwLast;
    else
        return (dwTotal - dwLast) + dwCurrent;
}


long SAM_ProcessProc_Mixer ( long lMode, DWORD * pdwProcessedSamples )
{
    static DWORD    dwSamples;
    static void     *pAudioBufferOutput;
    static long     lBufferUnderrun;
    static DWORD    dwWritePosition;
    static long     lLimiterMode;
    static long     lLastMode = -1;
    static DWORD    dwSampleRate;
           long     bInit, bRun;
           DWORD    dwInternalTimer;
           //DWORD    dwDistance;
           long     bInfiniteLoopWait;
           long     lReturn;
    static DWORD    dwLatencyLength;
    static DWORD    dwWritePointerLast, dwPlayPointerLast;
    static DWORD    dwWritePointerCurrent, dwPlayPointerCurrent;        
    static DWORD    dwReadyToPlayTotal;
    static DWORD    dwCurrentAvailableReadyToPlayTotal;
    static DWORD    dwWritePointerLastValidFree;
    static DWORD    dwWriteDataAvailable_ByPlayPointer;
    static DWORD    dwWriteDataAvailable_ByWritePointer;
    static DWORD    dwPlayPointerDelta;
    static DWORD    dwWritePointerDelta;
    static DWORD    dwLatencyManagerBUCountA;
    static DWORD    dwLatencyManagerBUCountB;        
           DWORD    dwProcessedSamples;
           long     lQuakeRecordAudioLen;
    static DWORD    dwTemporaryDeafness_TimeCount;

    dwProcessedSamples = 0;
           
    bInit = 0;    
    bRun  = 0;
    bInit = (lMode==0)?(1):(bInit);
    bRun  = (lMode==1)?(1):(bRun);
    if ( (lLastMode==-1) && (lMode!=0) ) bInit = 1;
    if ( (lLastMode!=-1) && (lMode==0) ) bInit = 0;
    lLastMode = lMode;
    
    lReturn = 0;

    _OSI_EnterCriticalSection ( &samData.osiCriticalSection );
    if (samData.bReInitMixer)
    {
        bInit = 1;
        samData.bReInitMixer = 0;
        
        
    }
    
    //Mode Quake Record Audio    
    if (samData.bQuakeRecordAudio)
        lQuakeRecordAudioLen = samData.lQuakeRecordAudioLen;
    else
        lQuakeRecordAudioLen = 0;
    
    //Initialisation
    SAM_PostProcessCopy_Init ( );
    
    _OSI_LeaveCriticalSection ( &samData.osiCriticalSection );
         
    if ((samData.dwHardwaremixSampleRate!=dwSampleRate)||(bInit))
    {    
        dwSamples = 0;
        pAudioBufferOutput = NULL;
        lBufferUnderrun = 0;
        dwWritePosition = 0;
        lLimiterMode = 0;
        dwLatencyLength = 0;
        dwWritePointerLast = 0;
        dwPlayPointerLast = 0;
        dwWritePointerCurrent = 0;
        dwPlayPointerCurrent = 0;
        dwReadyToPlayTotal = 0;
        dwCurrentAvailableReadyToPlayTotal = 0;
        dwWritePointerLastValidFree = 0;
        dwWriteDataAvailable_ByPlayPointer = 0;
        dwWriteDataAvailable_ByWritePointer = 0;
        dwPlayPointerDelta = 0;
        dwWritePointerDelta = 0;
        dwLatencyManagerBUCountA = 0;
        dwLatencyManagerBUCountB = 0;       
        dwTemporaryDeafness_TimeCount = 0;
    
        SAM_PostProcessCopy_0x26_Init ( );
        
        //dwSamplesWriteNeeds         = samData.dwHardwareBufferLatencySamplesCount;// samData.dwHardwaremixSampleRate / 50; //Buffer de 20ms
        //dwSamplesWriteNeeds         = dwSamplesWriteNeeds&0xFFFFE0; //résolution de 32 échantillons
        //dwSamplesDynamicWriteNeeds  = dwSamplesWriteNeeds;

        //Lecture des positions initiales
        //SAM_DirectSound_GetPosition ( &dwPositionWriteLast, &dwPositionReadCurrent );

        //dwWritePosition     = dwPositionWriteLast;
        //dwWritePendingData  = 0;
        lBufferUnderrun     = 0;
        lLimiterMode        = 0;
        dwSampleRate        = samData.dwHardwaremixSampleRate;
        
        
        
        
        
        //Latence
        dwLatencyLength = samData.dwHardwareBufferLatencySamplesCount&0xFFFFE0;
        samData.lActualLatencyInSamples = (long)dwLatencyLength;
        dwLatencyManagerBUCountA = 0;
        dwLatencyManagerBUCountB = 0;
        
        //Récupération des pointeurs Play / Write
        SAM_DirectSound_GetPosition ( &dwWritePointerCurrent, &dwPlayPointerCurrent );
        /*
        samData.aiMain.DeviceGetCursorPosition ( 
            &samData.aiMain, 
            (long *)&dwWritePointerCurrent, 
            (long *)&dwPlayPointerCurrent );
        */
        
        //Sauve les positions
        dwWritePointerLast = dwWritePointerCurrent;
        dwPlayPointerLast  = dwPlayPointerCurrent;
        
        //Initialisation des données prêtes à être lues
        dwWritePointerLastValidFree         = 0;
        dwWriteDataAvailable_ByWritePointer = 0;
        dwWriteDataAvailable_ByPlayPointer  = 0;
    }
    
    if (bRun)
    {
        lBufferUnderrun -= 1;
        if (lBufferUnderrun<0) lBufferUnderrun = 0;
    
    
        //Gestion du timer interne (basé sur les appels de la thread)
        SAM_TIMER_Increment ( &dwInternalTimer );

        //Sauve les pointeurs courants
        dwWritePointerLast = dwWritePointerCurrent;
        dwPlayPointerLast  = dwPlayPointerCurrent;
        
        //Récupération des pointeurs Play / Write
        SAM_DirectSound_GetPosition ( &dwWritePointerCurrent, &dwPlayPointerCurrent );
        /*
        samData.aiMain.DeviceGetCursorPosition ( 
            &samData.aiMain, 
            (long *)&dwWritePointerCurrent, 
            (long *)&dwPlayPointerCurrent );
        */
        
        //Nombre total d'échantillons passés en mode PLAY...
        dwPlayPointerDelta = GetBufferPositionDelta ( dwPlayPointerCurrent, dwPlayPointerLast, samData.dwHardwareAndSoftwareBufferSamplesCount );
        //samData.aiMain.qwMuxelGlobalCursorPositionEx );
        //
        
        //Nombre total d'échantillons passés en mode WRITE
        dwWritePointerDelta = GetBufferPositionDelta ( dwWritePointerCurrent, dwWritePointerLast, samData.dwHardwareAndSoftwareBufferSamplesCount );
        //samData.aiMain.qwMuxelGlobalCursorPositionEx );

        //Mise à jour des données restantes ayant été ajoutées via le pointeur d'écriture
        if (dwWriteDataAvailable_ByWritePointer>dwWritePointerDelta)
            dwWriteDataAvailable_ByWritePointer -= dwWritePointerDelta;
        else
        {
            dwWriteDataAvailable_ByWritePointer = 0;
            dwWriteDataAvailable_ByPlayPointer  = 0;
            lBufferUnderrun                     += 1;
        }
            
        //Mise à jour des données restantes ayant été ajoutées via le pointeur de lecture
        if (dwWriteDataAvailable_ByPlayPointer>dwPlayPointerDelta)
            dwWriteDataAvailable_ByPlayPointer -= dwPlayPointerDelta;
        else
        {
            dwWriteDataAvailable_ByWritePointer = 0;
            dwWriteDataAvailable_ByPlayPointer  = 0;
            lBufferUnderrun                     += 1;
        }
        
        if (lQuakeRecordAudioLen)
        {
            dwLatencyLength = lQuakeRecordAudioLen;
            dwWriteDataAvailable_ByPlayPointer = 0;
            dwWriteDataAvailable_ByWritePointer = 0;
        }
        
        //Génération d'un nouveau bloc...
        if ( (dwWriteDataAvailable_ByPlayPointer  <= dwLatencyLength) ||
             (dwWriteDataAvailable_ByWritePointer <= dwLatencyLength) )
        {
            DWORD dwWritePointerCurrentBis;
            DWORD dwPlayPointerCurrentBis;
            DWORD dwDistanceUpdate_Write;
            DWORD dwDistanceUpdate_Play;
            DWORD dwWritesCount;
            

            //Nombre de samples à ajouter
            dwWritesCount = dwLatencyLength;

            //Ecriture du Timer pour le mirroir des voix
            SAM_VOICE_Mirror ( dwInternalTimer );
            
            //Mixage du STREAM, de la MUSIC et des VOICE
            SAM_SFX_EnterLock ( );
            SAM_STREAM_Mix ( samData.pfSoftwareBuffer, samData.dwOutputSoftwareChannel, dwWritesCount, dwInternalTimer );
            SAM_MUSIC_Mix  ( samData.pfSoftwareBuffer, samData.dwOutputSoftwareChannel, dwWritesCount, dwInternalTimer );
            SAM_VOICE_Mix  ( samData.pfSoftwareBuffer, samData.dwOutputSoftwareChannel, dwWritesCount, dwInternalTimer );          
            SAM_SFX_LeaveLock ( );
            
            //Protection et ouverture de l'accès au tampon de sortie
            _OSI_EnterCriticalSection ( &samData.osiCSOutputData );
            SAM_DirectSound_BeginPaint ( &pAudioBufferOutput );
            //samData.aiMain.DeviceLock ( &samData.aiMain, &pAudioBufferOutput, NULL );
            //long (*DeviceLock) ( void * pAudioInterface, void ** pBuffer, long * plMuxelCount );
            
            
            //Après le mixage, le pointeur d'écriture s'est déplacé...        
            SAM_DirectSound_GetPosition ( &dwWritePointerCurrentBis, &dwPlayPointerCurrentBis );
            /*
            samData.aiMain.DeviceGetCursorPosition ( 
                &samData.aiMain, 
                (long *)&dwWritePointerCurrentBis, 
                (long *)&dwPlayPointerCurrentBis );
            */
            dwDistanceUpdate_Write = GetBufferPositionDelta ( dwWritePointerCurrentBis, dwWritePointerCurrent, samData.dwHardwareAndSoftwareBufferSamplesCount );
            //samData.aiMain.qwMuxelGlobalCursorPositionEx );

            dwDistanceUpdate_Play  = GetBufferPositionDelta ( dwPlayPointerCurrentBis,  dwPlayPointerCurrent,  samData.dwHardwareAndSoftwareBufferSamplesCount );
            //samData.aiMain.qwMuxelGlobalCursorPositionEx );
            
            //Détermine l'emplacement de la zone de copie en sortie (avec une marge)
            if ( (dwDistanceUpdate_Play+64  >= dwWriteDataAvailable_ByPlayPointer ) ||
                 (dwDistanceUpdate_Write+64 >= dwWriteDataAvailable_ByWritePointer) )
            {
                dwWritePosition                         = (dwWritePointerCurrentBis+64)%samData.dwHardwareAndSoftwareBufferSamplesCount;
                dwWriteDataAvailable_ByPlayPointer      = 0;
                dwWriteDataAvailable_ByWritePointer     = 0;
                dwWritePointerLastValidFree             = (dwWritePosition+dwWritesCount)%samData.dwHardwareAndSoftwareBufferSamplesCount;
                lBufferUnderrun += 1;
            }
            else
            {
                dwWritePosition                         = dwWritePointerLastValidFree;//(dwWritePointerLastUpdated+dwWriteDataAvailable)%samData.dwHardwareAndSoftwareBufferSamplesCount;
                dwWriteDataAvailable_ByPlayPointer      -= dwDistanceUpdate_Play;
                dwWriteDataAvailable_ByWritePointer     -= dwDistanceUpdate_Write;
                dwWritePointerLastValidFree             = (dwWritePointerLastValidFree+dwWritesCount)%samData.dwHardwareAndSoftwareBufferSamplesCount;
            }
            dwWritePointerCurrent                   = dwWritePointerCurrentBis;
            dwPlayPointerCurrent                    = dwPlayPointerCurrentBis;
            
            /*
            if (lBufferUnderrun)
            {
                DWORD dwIndex;
                DWORD dwChannel;
                float * pfBOut;
                float fValue;
                
                DWORD dwMod, dwCount;
                dwMod = samData.dwHardwaremixSampleRate/2000;
                
                pfBOut = samData.pfSoftwareBuffer;
                dwCount = 0;
                fValue = -0.5F;
                for (dwIndex=0;dwIndex<dwWritesCount;dwIndex++)
                {
                    
                    for (dwChannel=0;dwChannel<samData.dwOutputHardwareChannel;dwChannel++)
                    {
                        *pfBOut = fValue;
                        pfBOut++;
                    }
                    
                    dwCount = (dwCount+1)%dwMod;
                    if (dwCount==0) fValue = -fValue;
                }
            
            }
            */
            
            //Ecriture dans le tampon
            SAM_PostProcessCopy (
                samData.pfSoftwareBuffer, 
                pAudioBufferOutput,
                dwWritesCount,
                dwWritePosition,
                lLimiterMode );
                
            if (lQuakeRecordAudioLen)
            {
                DWORD dwOffsetTarget;
                DWORD dwOffsetSource;
                DWORD dwOffsetSourceMax;
                DWORD dwOffsetTargetMax;
                DWORD dwCopyLen;
                DWORD dwIndex;

                INT16 * pi16SourceBuffer;
            
                dwOffsetSource                              = dwWritePosition;
                dwOffsetSourceMax                           = samData.dwHardwareAndSoftwareBufferSamplesCount * samData.dwOutputHardwareChannel;
                dwOffsetTarget                              = 0;
                dwOffsetTargetMax                           = samData.MixerOutput_dwSamplesCount;
                dwOffsetSource                              *= samData.dwOutputHardwareChannel;
                dwOffsetTarget                              *= samData.dwOutputHardwareChannel;                
                samData.MixerOutput_dwMCSamplesOffsetWrite  = dwWritesCount;
                samData.MixerOutput_dwMCSamplesOffsetRead   = 0;
                samData.MixerOutput_dwMCSamplesReadyToRead  = dwWritesCount;
                dwCopyLen                                   = dwWritesCount;
                dwCopyLen                                   *= samData.dwOutputHardwareChannel;
                pi16SourceBuffer                            = pAudioBufferOutput;
                for (dwIndex=0;dwIndex<dwCopyLen;dwIndex++)
                {
                    if (dwOffsetSource>=dwOffsetSourceMax) dwOffsetSource -= dwOffsetSourceMax;
                    
                
                    samData.MixerOutput_pi16Buffer[dwOffsetTarget] = pi16SourceBuffer[dwOffsetSource];
                        
                    dwOffsetSource++;
                    
                    
                    dwOffsetTarget++;
                    if (dwOffsetTarget>=dwOffsetTargetMax) dwOffsetTarget -= dwOffsetTargetMax;
                }
            }
            else
            {    
                DWORD dwOffsetTarget;
                DWORD dwOffsetSource;
                DWORD dwOffsetSourceMax;
                DWORD dwOffsetTargetMax;
                DWORD dwCopyLen;
                DWORD dwIndex;

                INT16 * pi16SourceBuffer;
                
                //Copie du tampon dans le pré-out
                samData.MixerOutput_dwMCSamplesCount = samData.MixerOutput_dwSamplesCount / samData.dwOutputHardwareChannel;
                
                if (dwWritesCount>=samData.MixerOutput_dwMCSamplesCount)
                {
                    dwOffsetTarget                              = 0;
                    dwOffsetSource                              = dwWritePosition + dwWritesCount - samData.MixerOutput_dwMCSamplesCount;
                    dwCopyLen                                   = samData.MixerOutput_dwMCSamplesCount;
                    samData.MixerOutput_dwMCSamplesOffsetWrite  = 0;
                    samData.MixerOutput_dwMCSamplesOffsetRead   = 0;
                    samData.MixerOutput_dwMCSamplesReadyToRead  = 0;
                }
                else
                {
                    dwOffsetTarget                              = samData.MixerOutput_dwMCSamplesOffsetWrite;
                    dwOffsetSource                              = dwWritePosition;
                    dwCopyLen                                   = dwWritesCount;
                }
                    
                dwOffsetSource     *= samData.dwOutputHardwareChannel;
                dwOffsetTarget     *= samData.dwOutputHardwareChannel;                
                pi16SourceBuffer    = pAudioBufferOutput;
                dwOffsetSourceMax   = samData.dwHardwareAndSoftwareBufferSamplesCount * samData.dwOutputHardwareChannel;
                dwOffsetTargetMax   = samData.MixerOutput_dwSamplesCount;
                dwCopyLen           *= samData.dwOutputHardwareChannel;
                for (dwIndex=0;dwIndex<dwCopyLen;dwIndex++)
                {
                    if (dwOffsetSource>=dwOffsetSourceMax) dwOffsetSource -= dwOffsetSourceMax;
                    
                
                    samData.MixerOutput_pi16Buffer[dwOffsetTarget] = pi16SourceBuffer[dwOffsetSource];
                        
                    dwOffsetSource++;
                    
                    
                    dwOffsetTarget++;
                    if (dwOffsetTarget>=dwOffsetTargetMax) dwOffsetTarget -= dwOffsetTargetMax;
                }
                
                samData.MixerOutput_dwMCSamplesOffsetWrite = dwOffsetTarget / samData.dwOutputHardwareChannel;
                samData.MixerOutput_dwMCSamplesReadyToRead += dwCopyLen / samData.dwOutputHardwareChannel;
            }
            
            //samData.dwOutputHardwareChannel
            
            
            //Fermeture et libération de l'accès au tampon de sortie
            SAM_DirectSound_EndPaint ( );
            //samData.aiMain.DeviceUnlock ( &samData.aiMain );
            _OSI_LeaveCriticalSection ( &samData.osiCSOutputData );
            
            //Ajoute les échantillons copiés
            dwWriteDataAvailable_ByPlayPointer  += dwWritesCount;
            dwWriteDataAvailable_ByWritePointer += dwWritesCount;
            
            //On informe en retour que l'on vient de faire un mixage
            lReturn = 1;
            
            //Nombre d'échantillons réellement écrit
            dwProcessedSamples = dwWritesCount;
        }

/*            
#ifdef _DEBUG
{
    static long lCountDebug;
    char szTmp[128];
    lCountDebug = (lCountDebug+1)&63;
    
    
    if (lCountDebug==0)
    {
        szTmp[100] = 0;
        
        i = (dwWriteDataAvailable_ByWritePointer*100) / dwLatencyLength;
        i /= 2;
        if (i>100) i = 100;
        memset ( szTmp, '.', 100 );
        memset ( szTmp, '#', i );
        _SAM_DEBUG_TEXT_ ( "W:%s\n", szTmp ); //"%s\n", dwCurrentAvailableReadyToPlayTotal );

        i = (dwWriteDataAvailable_ByPlayPointer*100) / dwLatencyLength;
        i /= 2;
        if (i>100) i = 100;
        memset ( szTmp, '.', 100 );
        memset ( szTmp, '#', i );
        _SAM_DEBUG_TEXT_ ( "P:%s\n", szTmp ); //"%s\n", dwCurrentAvailableReadyToPlayTotal );
        
        
    }
    if (lBufferUnderrun) _SAM_DEBUG_TEXT_ ( "Buffer underrun !!!\n" );
}
#endif
*/

        //*********************************************************************************
        //Mise à jour des données...
        _OSI_EnterCriticalSection ( &samData.osiCriticalSection );

        //Mise à jour du buffer underrun
        if (lBufferUnderrun)
        {
            DWORD dwMaxLatency;
            
            samData.lTotalBufferUnderrunCount   += 1;
            
            dwLatencyManagerBUCountA++;
            
            if (dwLatencyManagerBUCountA>8) //10) //Passage de 10 à 8 pour être plus sensible !
            {
                dwLatencyManagerBUCountA = 0;
                dwLatencyManagerBUCountB = 0;
                
                dwMaxLatency = (samData.dwHardwareAndSoftwareBufferSamplesCount>>2)&0xFFFFE0;
                dwLatencyLength += 0x10;
                if (dwLatencyLength>dwMaxLatency)
                    dwLatencyLength = dwMaxLatency;
                //dwLatencyLength = samData.dwHardwareBufferLatencySamplesCount&0xFFFFE0;
            
                samData.lActualLatencyInSamples = (long)dwLatencyLength;
            }
        }
        else
        {
            dwLatencyManagerBUCountB+=dwProcessedSamples;
            if (dwLatencyManagerBUCountB>(samData.dwHardwaremixSampleRate*10))
            {
                dwLatencyManagerBUCountA = 0;
                dwLatencyManagerBUCountB = 0;
            }
        }

        //Récupération du mode du limiteur
        lLimiterMode                        = samData.lLimiterMode;

        //Ecriture des stats
        samData.fCyclesPerSampleMirror      = samData.fCyclesPerSample;
    
        //_OSI_LeaveCriticalSection ( &samData.osiCriticalSection );
        
        //*********************************************************************************        
        //Boucle d'attente
        //_OSI_EnterCriticalSection ( &samData.osiCriticalSection );
        if (samData.bInfiniteLoopWait==1)
        {
            samData.bInfiniteLoopWait = 2;
            bInfiniteLoopWait = 1;
        }
        else bInfiniteLoopWait = 0;
        _OSI_LeaveCriticalSection ( &samData.osiCriticalSection );
        
        if (bInfiniteLoopWait)
        {
            lReturn++;
            do {
                _OSI_Sleep ( 1 );
                
                _OSI_EnterCriticalSection ( &samData.osiCriticalSection );
                if (!samData.bInfiniteLoopWait) bInfiniteLoopWait = 0;
                _OSI_LeaveCriticalSection ( &samData.osiCriticalSection );
            } while (bInfiniteLoopWait);
        }
        
    }

    //La gestion de la douleur auditive (Pain Hearing)    
/*    
    if (samData.TemporaryDeafness_lEnable)
    {
        dwTemporaryDeafness_TimeCount += dwProcessedSamples;
        if (dwTemporaryDeafness_TimeCount>=(samData.dwHardwaremixSampleRate>>6))
        {
            dwTemporaryDeafness_TimeCount -= (samData.dwHardwaremixSampleRate>>6);
            
            if ( ((samData.TemporaryDeafness_dwVoiceCountOverlevel*TEMPORARYDEAFNESS_MAXRATIO)/100) > samData.TemporaryDeafness_dwVoiceCount )
                samData.TemporaryDeafness_lEarPainCount += TEMPORARYDEAFNESS_PAIN_INCR;
            else
                samData.TemporaryDeafness_lEarPainCount -= TEMPORARYDEAFNESS_PAIN_DECR;
                
            if (samData.TemporaryDeafness_lEarPainCount<0)
                samData.TemporaryDeafness_lEarPainCount = 0;
                
            if (samData.TemporaryDeafness_lEarPainCount>TEMPORARYDEAFNESS_MAXPAIN)
                samData.TemporaryDeafness_lEarPainCount = TEMPORARYDEAFNESS_MAXPAIN;
            
            samData.TemporaryDeafness_dwVoiceCountOverlevel = 0;
            samData.TemporaryDeafness_dwVoiceCount = 0;            
        }
    }
*/    
    if (pdwProcessedSamples)
        *pdwProcessedSamples = dwProcessedSamples;
        
    return lReturn;
}

long IKA_MixerFreeze ( long bFreeze )
{
    if (bFreeze)
    {
        samData.bFreezeMixer = 1;
        if (samData.bFrozenMixer) return 0;
        else
        {
            do {
                _OSI_Sleep ( 1 );
            } while (!samData.bFrozenMixer);
        }
    }
    else
    {
        samData.bFreezeMixer = 0;
        if (!samData.bFrozenMixer) return 0;
        else
        {
            do {
                _OSI_Sleep ( 1 );
            } while (samData.bFrozenMixer);
        }
    }
    return 0;
}

#define CPU_USAGE_LEN   (64)
long SAM_ThreadProc_Mixer ( void * pProcData )
{
    DWORD           dwAffinityForce;
    DWORD           dwSleepTimer;
    QWORD           qwPrecisionTimerFrequecy;
    float           f32CurrentUsage, f32AverageUsage;
    float           f32LatestUsage[CPU_USAGE_LEN];
    long            lIndex;
    long            lReturn;
    DWORD           dwProcessedSamples;
    DWORD           dwSleepCount;
    double          f64DurationMixer;
    QWORD           qwHPTDuration;
    QWORD           qwHPTMixerA;
    QWORD           qwHPTMixerB;
    float           f32ProcessorUsagePercent;
    double          f64DurationSamples;

    #define         AVERAGE_USAGE_TABLE_COUNT   (1024)
    float           fAverageUsagePercentTable[AVERAGE_USAGE_TABLE_COUNT];
    long            lAverageUsageIndex;


    dwSleepTimer        = 0;
    dwAffinityForce     = 1024;
    dwSleepCount        = 0;
    
    memset ( fAverageUsagePercentTable, 0, sizeof(float)*AVERAGE_USAGE_TABLE_COUNT );
    lAverageUsageIndex  = 0;
    
    SAM_ProcessProc_Mixer ( 0, NULL );
    
    //Fixe l'usage
    f32CurrentUsage = 0.0F;
    f32AverageUsage = 0.0F;
    memset ( f32LatestUsage, 0, CPU_USAGE_LEN * sizeof(float) );

    f32ProcessorUsagePercent            = 0;
    lReturn                             = 0;
    
    do {
        lReturn = 0;
        dwProcessedSamples = 0;
    
        //Application du process de mixage
        QueryPerformanceFrequency ( (LARGE_INTEGER *)&qwPrecisionTimerFrequecy );
        QueryPerformanceCounter ( (LARGE_INTEGER *)&qwHPTMixerA );
        if (samData.bFreezeMixer)
        {
            lReturn = 0;
            samData.bFrozenMixer = 1;
        }
        else
        {
            if (samData.bQuakeRecordAudio)
            {
                lReturn = 0;
                samData.fMixerAverageUsagePercent = 0;
                samData.fMixerCurrentUsagePercent = 0;
            }
            else
            {
                lReturn = SAM_ProcessProc_Mixer ( 1, &dwProcessedSamples );
            }
            samData.bFrozenMixer = 0;
        }
        QueryPerformanceCounter ( (LARGE_INTEGER *)&qwHPTMixerB );
        
        //Mesure la durée de traitement du mixer
        if ((lReturn)&&(dwProcessedSamples))
        {
            //Durée du traitement (s)
            qwHPTDuration = (qwHPTMixerB - qwHPTMixerA);
            f64DurationMixer = (double)qwHPTDuration / (double)qwPrecisionTimerFrequecy;
            
            //Nombre d'échantillons traîtés en durée (s)
            f64DurationSamples = (double)dwProcessedSamples / (double)samData.dwHardwaremixSampleRate;
            
            if (f64DurationMixer>0)
                f32ProcessorUsagePercent = (float)( ( f64DurationMixer * 100 ) / f64DurationSamples );
            
            //_SAM_DEBUG_TEXT_ ( "Mixer process duration : %f - Processed Samples duration %f\n", f64DurationMixer, f64Temp1 );
        }
            
        
        //Y-a-t-il eu un traitement ?
        if ((lReturn)&&(dwProcessedSamples))
        {
            float fAveragePercentValue;
            float fDiv;
            long lTableCount;
            
            if (samData.lActualLatencyInSamples>1)
            {
                //On essaye de se caler sur 5 secondes...
                lTableCount = (samData.dwHardwaremixSampleRate*5)/samData.lActualLatencyInSamples;
                if (lTableCount>AVERAGE_USAGE_TABLE_COUNT) lTableCount = AVERAGE_USAGE_TABLE_COUNT;
            }
            else
                lTableCount = AVERAGE_USAGE_TABLE_COUNT;
            
            //Détermine l'occupation moyenne
            fAverageUsagePercentTable[lAverageUsageIndex] = f32ProcessorUsagePercent;
            lAverageUsageIndex = (lAverageUsageIndex+1)%lTableCount;
            fAveragePercentValue      = 0;
            fDiv                      = 0;
            for (lIndex=0;lIndex<lTableCount;lIndex++)
            {
                if (fAverageUsagePercentTable[lIndex]>0)
                {
                    fAveragePercentValue += fAverageUsagePercentTable[lIndex];
                    fDiv += 1;
                }
            }
            if (fDiv>0)
            {
                fAveragePercentValue = fAveragePercentValue/fDiv;
                samData.fMixerAverageUsagePercent = fAveragePercentValue; //(long)(f32AverageUsage*1000.0F);
            }
            
            //Informe de l'occupation durant le dernier mixage
            samData.fMixerCurrentUsagePercent = f32ProcessorUsagePercent; //(long)(f32AverageUsage*100.0F);
        }
        
        dwSleepCount++;
        
        if (dwSleepCount>10)
        {
            _OSI_Sleep ( 1 );
            //WaitForSingleObject( GetStdHandle( STD_INPUT_HANDLE ), 1 );
            dwSleepCount = 0;
        }
        else if (dwSleepCount>4) _OSI_Sleep ( 0 );

        //Gestion de l'affinité
        dwAffinityForce = (dwAffinityForce+1)&255;
        if (dwAffinityForce==0)
            SAM_ForceAffinity ( );
        

    } while ( _OSI_ThreadDoWhileAndWaitExit ( pProcData ) );
          
    return 0;
}

/*
long SAM_ThreadProc_Mixer_OK ( void * pProcData )
{
    DWORD           dwAffinityForce;
    DWORD           dwSleepTimer;
    DWORD           dwTemp;
    QWORD           qwPrecisionTimerFrequecy;
    QWORD           qwPrecisionTimerValue[3];
    QWORD           qwTemp;
    float           f32CurrentUsage, f32AverageUsage;
    float           f32LatestUsage[CPU_USAGE_LEN];
    long            lLatestUsagePosition, lIndex;
    long            lReturn;
    //DWORD           dwLatencyLength;
    //float           fLatencyMilliSeconds;
    double          f64Temp1, f64Temp2;
    DWORD           dwProcessedSamples;
    DWORD           dwSleepCount;


    dwSleepTimer        = 0;
    dwAffinityForce     = 1024;
    dwSleepCount        = 0;
    
    
    SAM_ProcessProc_Mixer ( 0, NULL );
    
    //Fixe l'usage
    f32CurrentUsage = 0.0F;
    f32AverageUsage = 0.0F;
    memset ( f32LatestUsage, 0, CPU_USAGE_LEN * sizeof(float) );
    lLatestUsagePosition = 0;
    QueryPerformanceFrequency ( (LARGE_INTEGER *)&qwPrecisionTimerFrequecy );
    


    //Lecture du temps initial
    QueryPerformanceCounter ( (LARGE_INTEGER *)&qwPrecisionTimerValue[0] );
    
    
    do {
        //La latence en millisecondes    
        //dwLatencyLength = samData.dwHardwareBufferLatencySamplesCount&0xFFFFE0;
        //fLatencyMilliSeconds = (dwLatencyLength*1000)/samData.dwHardwaremixSampleRate;
        
        //Gestion de l'affinité
        dwAffinityForce = (dwAffinityForce+1)&15;
        if (dwAffinityForce==0)
            SAM_ForceAffinity ( );
        
        //Process
        QueryPerformanceCounter ( (LARGE_INTEGER *)&qwPrecisionTimerValue[1] );
        if (samData.bFreezeMixer)
        {
            lReturn = 0;
            samData.bFrozenMixer = 1;
        }
        else
        {
            if (samData.bQuakeRecordAudio)
            {
                lReturn = 0;
                samData.lMixerAverageUsage = 0;
                samData.lMixerCurrentUsage = 0;
            }
            else
            {
                lReturn = SAM_ProcessProc_Mixer ( 1, &dwProcessedSamples );
            }
            samData.bFrozenMixer = 0;
        }
        QueryPerformanceCounter ( (LARGE_INTEGER *)&qwPrecisionTimerValue[2] );
        
        //Y-a-t-il eu un traitement ?
        if ((lReturn)&&(dwProcessedSamples))
        {
#define AVERAGE_USAGE_TABLE_COUNT   (256)        
            static long lAverageUsageTable[AVERAGE_USAGE_TABLE_COUNT];
            static long lAverageUsageIndex = 0;
            long lAverageValue;
            //
            //  TimerValue0     t=201ms
            //      ...WAIT...
            //  TimerValue1     t=213ms
            //      ...Process... 480 samples @ 48KHz => 10ms
            //  TimerValue2     t=216ms
            //  
            //  Toccup = t2-t1 = 3ms
            //  Tratio = (t2-t1) / 10ms = 0,3 = 30%
            
            //Détermine la durée rajoutée au tampon pour ce calcul (en s)
            f64Temp1  = (double)dwProcessedSamples;
            f64Temp1 /= (double)samData.dwHardwaremixSampleRate;
            
            //Détermine le temps nécessaire pour calculer les échantillons (en s)
            f64Temp2 = (double)(qwPrecisionTimerValue[2] - qwPrecisionTimerValue[1]);
            f64Temp2 /= (double)qwPrecisionTimerFrequecy;
            
            //Détermine l'occupation en pourcentage
            f32AverageUsage = ( f64Temp2 / f64Temp1 );
            
            lAverageUsageTable[lAverageUsageIndex] = (long)(f32AverageUsage*10000.0F);
            lAverageUsageIndex = (lAverageUsageIndex+1)%AVERAGE_USAGE_TABLE_COUNT;
            lAverageValue      = 0;
            for (lIndex=0;lIndex<AVERAGE_USAGE_TABLE_COUNT;lIndex++)
            {
                lAverageValue += lAverageUsageTable[lIndex];
            }
            lAverageValue = lAverageValue/AVERAGE_USAGE_TABLE_COUNT;
            
            
            samData.lMixerAverageUsage = lAverageValue/100; //(long)(f32AverageUsage*1000.0F);
            samData.lMixerCurrentUsage = (long)(f32AverageUsage*100.0F);
            
            
            //Refixe le timer value #0            
            qwPrecisionTimerValue[0] = qwPrecisionTimerValue[2];
        }
        
        if (!lReturn)
        {
            lReturn = 0;
        
        }
        
        dwSleepCount++;
        
        if (dwSleepCount>10)
        {
            _OSI_Sleep ( 1 );
            //WaitForSingleObject( GetStdHandle( STD_INPUT_HANDLE ), 1 );
            dwSleepCount = 0;
        }
        else if (dwSleepCount>4) _OSI_Sleep ( 0 );

    } while ( _OSI_ThreadDoWhileAndWaitExit ( pProcData ) );
        
    return 0;
}
*/
/*
long SAM_ThreadProc_Mixer_good ( void * pProcData )
{
    DWORD           dwAffinityForce;
    DWORD           dwSleepTimer;
    DWORD           dwTemp;
    QWORD           qwPrecisionTimerFrequecy;
    QWORD           qwPrecisionTimerValue[3];
    QWORD           qwTemp;
    float           f32CurrentUsage, f32AverageUsage;
    float           f32LatestUsage[CPU_USAGE_LEN];
    long            lLatestUsagePosition, lIndex;
    long            lReturn;
    DWORD           dwLatencyLength;
    float           fLatencyMilliSeconds;
    double          f64Temp1, f64Temp2;
    DWORD           dwProcessedSamples;


    dwSleepTimer        = 0;
    dwAffinityForce     = 1024;
    
    SAM_ProcessProc_Mixer ( 0, NULL );
    
    //Fixe l'usage
    f32CurrentUsage = 0.0F;
    f32AverageUsage = 0.0F;
    memset ( f32LatestUsage, 0, CPU_USAGE_LEN * sizeof(float) );
    lLatestUsagePosition = 0;
    QueryPerformanceFrequency ( (LARGE_INTEGER *)&qwPrecisionTimerFrequecy );
    
    do {

        //La latence en millisecondes    
        dwLatencyLength = samData.dwHardwareBufferLatencySamplesCount&0xFFFFE0;
        fLatencyMilliSeconds = (dwLatencyLength*1000)/samData.dwHardwaremixSampleRate;
        
        //Lecture du temps : Etape 1/3
        QueryPerformanceCounter ( (LARGE_INTEGER *)&qwPrecisionTimerValue[0] );
    
        //Gestion de l'affinité
        dwAffinityForce = (dwAffinityForce+1)&15;
        if (dwAffinityForce==0)
            SAM_ForceAffinity ( );
        
        //Process
        lReturn = SAM_ProcessProc_Mixer ( 1, &dwProcessedSamples );
        
        //Lecture du temps : Etape 2/3
        QueryPerformanceCounter ( (LARGE_INTEGER *)&qwPrecisionTimerValue[1] );
        
        //_OSI_Sleep ( 1 );
        
        if (lReturn==1)
        {        
            //Mesure le temps réellement écoulé
            qwTemp = (qwPrecisionTimerValue[1] - qwPrecisionTimerValue[0]);
            qwTemp = (qwTemp*1000)/qwPrecisionTimerFrequecy;
            
            //Une petite pause de quelques millisecondes... si possible !
            dwSleepTimer = (dwSleepTimer+1)&3;        
            if ((dwSleepTimer==0)||(qwTemp<dwLatencyLength))
            {
                dwTemp = (DWORD)(qwTemp>>1);
                if (dwTemp<1) dwTemp = 1;
                _OSI_Sleep ( dwTemp );
            }
            _OSI_Sleep ( 0 );
        }
        else _OSI_Sleep ( 1 );
            
        //if (qwTemp>=dwLatencyLength) _SAM_DEBUG_TEXT_ ( "Overload !!!\n" );
        //if (qwTemp) _SAM_DEBUG_TEXT_ ( "Delay = %d\n", qwTemp );
        
        //Lecture du temps : Etape 3/3
        QueryPerformanceCounter ( (LARGE_INTEGER *)&qwPrecisionTimerValue[2] );
        
        
        //Calcule le taux d'occupation du processeur
        if (qwPrecisionTimerValue[0]<qwPrecisionTimerValue[2])
        {
  
            
            f64Temp1 = (double)(qwPrecisionTimerValue[2] - qwPrecisionTimerValue[0]);
            f64Temp2 = (double)(qwPrecisionTimerValue[1] - qwPrecisionTimerValue[0]);
            
            if (f64Temp1>0)
            {
                f32CurrentUsage = (float)(f64Temp2/f64Temp1);
                
                f32LatestUsage[lLatestUsagePosition] = f32CurrentUsage;
                lLatestUsagePosition = (lLatestUsagePosition+1)&(CPU_USAGE_LEN-1);
    
                f32AverageUsage = 0.0F;
                for (lIndex=0;lIndex<CPU_USAGE_LEN;lIndex++)
                {
                    f32AverageUsage += f32LatestUsage[lIndex];
                }
                f32AverageUsage /= CPU_USAGE_LEN;
                
                samData.lMixerAverageUsage = (long)(f32AverageUsage*100.0F);
                samData.lMixerCurrentUsage = (long)(f32CurrentUsage*100.0F);
            }
        }

    } while ( _OSI_ThreadDoWhileAndWaitExit ( pProcData ) );
        
    return 0;
}
*/


/*
long SAM_ThreadProc_Mixer2 ( void * pProcData )
{
    DWORD dwPositionWriteCurrent, dwPositionWriteLast;
    DWORD dwPositionReadCurrent, dwPositionReadLast;
    DWORD dwSamplesWriteNeeds;
    void  * pAudioBufferOutput;
    long    lBufferUnderrun;
    DWORD   dwWritePosition;
    DWORD   dwWritePendingData;
    DWORD   dwSamplesRead;
    DWORD   dwInternalTimer;
    DWORD   dwSleepTimer;
    long    lLimiterMode;
    DWORD   dwAffinityForce;

    dwSamplesWriteNeeds = samData.dwHardwareBufferLatencySamplesCount;// samData.dwHardwaremixSampleRate / 50; //Buffer de 20ms
    dwSamplesWriteNeeds = dwSamplesWriteNeeds&0xFFFFF0; //résolution de 16 échantillons

    //Lecture des positions initiales
    SAM_DirectSound_GetPosition ( &dwPositionWriteLast, &dwPositionReadCurrent );

    dwWritePosition     = dwPositionWriteLast;
    dwWritePendingData  = 0;
    lBufferUnderrun     = 0;
    dwInternalTimer     = 0;
    dwSleepTimer        = 0;
    lLimiterMode        = 0;
    dwAffinityForce     = 1024;

    do {
    
        //Gestion de l'affinité
        //dwAffinityForce++;
        //if (dwAffinityForce>16)
        //{
        //    SAM_ForceAffinity ( );
        //    dwAffinityForce = 0;
        //}
        
        //Gestion du timer interne (basé sur les appels de la thread)
        dwInternalTimer += 1;        

        lBufferUnderrun -= 1;
        if (lBufferUnderrun<0) lBufferUnderrun = 0;

        //Lecture de la position de lecture et d'écriture...
        dwPositionReadLast = dwPositionReadCurrent;
        SAM_DirectSound_GetPosition ( &dwPositionWriteCurrent, &dwPositionReadCurrent );

        //Nombre de samples lus...
        if (dwPositionReadCurrent>=dwPositionReadLast) 
            dwSamplesRead = dwPositionReadCurrent - dwPositionReadLast;
        else
            dwSamplesRead = dwPositionReadCurrent + (samData.dwHardwareAndSoftwareBufferSamplesCount-dwPositionReadLast);

        //Nombre de données restantes...
        if (dwSamplesRead>dwWritePendingData)
        {
            //Detection d'un buffer underrun
            dwWritePendingData                  = 0;
            lBufferUnderrun                     += 1;
        }
        else
            dwWritePendingData -= dwSamplesRead;

        //Buffer vide !!! On recommence à écrire au bon endroit !
        if (dwWritePendingData==0)
            dwWritePosition = dwPositionWriteCurrent;

        //Il reste moins de XX millisecondes ?
        if (dwWritePendingData<dwSamplesWriteNeeds)
        {
            
            SAM_SFX_EnterLock ( );
            
            SAM_STREAM_Mix ( 
                samData.pfSoftwareBuffer,
                samData.dwOutputSoftwareChannel,
                dwSamplesWriteNeeds,
                dwInternalTimer );
               
            SAM_MUSIC_Mix (
                samData.pfSoftwareBuffer,
                samData.dwOutputSoftwareChannel,
                dwSamplesWriteNeeds,
                dwInternalTimer );
            
            SAM_VOICE_Mix ( 
                samData.pfSoftwareBuffer,                
                samData.dwOutputSoftwareChannel,
                dwSamplesWriteNeeds,
                dwInternalTimer );          
            
            SAM_SFX_LeaveLock ( );
            
            //Ouverture de l'accès au tampon de sortie            
            SAM_DirectSound_BeginPaint ( &pAudioBufferOutput );
                                             
            //Post-process anti-saturation et conversion dans les 16 bits...
            SAM_PostProcessCopy (
                samData.pfSoftwareBuffer, 
                pAudioBufferOutput,
                dwSamplesWriteNeeds,                      
                dwWritePosition,
                lLimiterMode );

            //Fermeture de l'accès au tampon de sortie
            SAM_DirectSound_EndPaint ( );

            //On met à jour les positions
            dwWritePendingData += dwSamplesWriteNeeds;
            dwWritePosition = (dwWritePosition+dwSamplesWriteNeeds)%samData.dwHardwareAndSoftwareBufferSamplesCount;
        }

        //Une petite pause de 1ms... Mais que de temps en temps ! Une fois sur quatre, soit 0,250ms
        dwSleepTimer++;
        //if (dwSleepTimer>=4)
        {
            _OSI_Sleep ( 1 );
            dwSleepTimer = 0;
        }

        //On en profite pour corréler SAM_VOICE et SAM_VOICE_MIRROR
        SAM_VOICE_Mirror ( dwInternalTimer );

        //*********************************************************************************
        //Mise à jour des données...
        _OSI_EnterCriticalSection ( &samData.osiCriticalSection );

        //Mise à jour du buffer underrun
        if (lBufferUnderrun)
            samData.lTotalBufferUnderrunCount   += 1;

        //Récupération du mode du limiteur
        lLimiterMode                        = samData.lLimiterMode;

        //Ecriture des stats
        samData.fCyclesPerSampleMirror      = samData.fCyclesPerSample;
        

        _OSI_LeaveCriticalSection ( &samData.osiCriticalSection );

    } while ( _OSI_ThreadDoWhileAndWaitExit ( pProcData ) );
        
    return 0;
}
*/
