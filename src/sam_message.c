#include "sam_header.h"
#include "sam_data.h"
#include "sam_voice.h"

DLLEXPORT   long    SAM_Message ( DWORD dwMessage, DWORD dwParamA, DWORD dwParamB )
{
    DWORD * pdwReturn;
    
    switch (dwMessage)
    {
        case SAM_MESSAGE_DEVICESETENUM:
            //SAM_DirectSound_GetEnum ( );
            break;
            
        case SAM_MESSAGE_DYNAMICDELAYLINES:
            switch (dwParamA)
            {
                case 0xFFFFFFFF: //DDL = auto mode
                    break;
                case 0xFFFFFFFE: //DDL = off, use DLF processor
                    break;
                default:
                    if (dwParamA>100) dwParamA = 100;
            }
            
            if (samData.dwDynamicDelayLinesCount!=dwParamA)
                samData.dwDynamicDelayLinesCount = dwParamA;
                
            if (dwParamB)
            {
                pdwReturn = (DWORD *)dwParamB;
                *pdwReturn = samData.dwDynamicDelayLinesCount;
            }
            break;
        
        case SAM_MESSAGE_VOICE_SYNCFREEZE:
            if (dwParamA) SAM_VOICE_MIRROR_FreezeEnable ( );                
            else          SAM_VOICE_MIRROR_FreezeDisable ( );
            break;
            
        case SAM_MESSAGE_VOICE_ISPLAYED: //dwParamA = numéro de la voix, dwParamB = adresse de retour de l'état de la lecture (DWORD * pdwState)
            {
                SAM_VOICE_MIRROR    *psamVoiceMirror;
                long                bIsPlayedState;
                DWORD               *pdwPlayState;
                
                DWORD               dwVoice;
                DWORD               dwFastAccess;
                
                dwVoice         = dwParamA&SAM_VOICE_MASK;
                dwFastAccess    = dwParamA&SAM_VOICE_FASTACCESS;
                
                if ( (dwVoice>=samVoiceData.dwVoiceVirtualCount) ||
                     (dwParamB==0) )
                    return -1;
                    
                psamVoiceMirror = &(samVoiceData.psamVoiceMirror[dwVoice]);
                
                SAM_VOICE_LockEnter ( dwFastAccess );
                bIsPlayedState = psamVoiceMirror->bIsPlayedState;
                SAM_VOICE_LockLeave ( dwFastAccess );
                
                pdwPlayState = (DWORD *)dwParamB;
                *pdwPlayState = bIsPlayedState;
            }
            break;
            
        case SAM_MESSAGE_VOICE_SPEEDSHIFT_SET: //dwParamA = numéro de la voix, dwParamB = adresse du float pour le speedshift (float * pf32SpeedShift)
            {
                SAM_VOICE_MIRROR    *psamVoiceMirror;
                float               *pf32SpeedShift, f32SpeedShift;
                
                DWORD               dwVoice;
                DWORD               dwFastAccess;
                
                dwVoice         = dwParamA&SAM_VOICE_MASK;
                dwFastAccess    = dwParamA&SAM_VOICE_FASTACCESS;
                
                if ( (dwVoice>=samVoiceData.dwVoiceVirtualCount) ||
                     (dwParamB==0) )
                    return -1;
                    
                psamVoiceMirror = &(samVoiceData.psamVoiceMirror[dwVoice]);
                pf32SpeedShift = (float *)dwParamB;
                f32SpeedShift = *pf32SpeedShift;
                
                SAM_VOICE_LockEnter ( dwFastAccess );
                psamVoiceMirror->dwUpdateFlag |= UPDATE_PLAYRATE;
                psamVoiceMirror->fSpeedShift = f32SpeedShift;
                SAM_VOICE_LockLeave ( dwFastAccess );
            }
            break;    

        case SAM_MESSAGE_VOICE_SPEEDSHIFT_GET: //dwParamA = numéro de la voix, dwParamB = adresse du float pour le speedshift (float * pf32SpeedShift)
            {
                SAM_VOICE_MIRROR    *psamVoiceMirror;
                float               *pf32SpeedShift, f32SpeedShift;
                
                DWORD               dwVoice;
                DWORD               dwFastAccess;
                
                dwVoice         = dwParamA&SAM_VOICE_MASK;
                dwFastAccess    = dwParamA&SAM_VOICE_FASTACCESS;
                
                if ( (dwVoice>=samVoiceData.dwVoiceVirtualCount) ||
                     (dwParamB==0) )
                    return -1;
                    
                psamVoiceMirror = &(samVoiceData.psamVoiceMirror[dwVoice]);
                pf32SpeedShift = (float *)dwParamB;
                
                SAM_VOICE_LockEnter ( dwFastAccess );
                f32SpeedShift = psamVoiceMirror->fSpeedShift;
                SAM_VOICE_LockLeave ( dwFastAccess );
                
                 *pf32SpeedShift = f32SpeedShift;
            }
            break;    
            
        case SAM_MESSAGE_VOICE_VIRTUALCOUNT: //dwParamA = adresse de retour pour le nombre de voix virtuelles (DWORD * pdwVirtualVoicesCount)
            {
                DWORD * pdwVirtualVoicesCount;
                
                if (!dwParamA)
                    return -1;
                    
                pdwVirtualVoicesCount = (DWORD *)dwParamA;
                *pdwVirtualVoicesCount = samData.dwTotalVirtualVoicesCount;
            }
            break;
            
        case SAM_MESSAGE_VOICE_POSITION_GET: //dwParamA = numéro de la voix, dwParamB = adresse de retour pour la position courante (DWORD * pdwPosition)
            {
                SAM_VOICE_MIRROR    *psamVoiceMirror;
                DWORD               dwVoice;
                DWORD               dwFastAccess;
                DWORD               dwPosition, *pdwPosition;
                
                dwVoice         = dwParamA&SAM_VOICE_MASK;
                dwFastAccess    = dwParamA&SAM_VOICE_FASTACCESS;
                
                if ( (dwVoice>=samVoiceData.dwVoiceVirtualCount) ||
                     (dwParamB==0) )
                    return -1;
                    
                psamVoiceMirror = &(samVoiceData.psamVoiceMirror[dwVoice]);
                
                SAM_VOICE_LockEnter ( dwFastAccess );
                dwPosition = psamVoiceMirror->dwInPlaySamplePosition;
                SAM_VOICE_LockLeave ( dwFastAccess );
                    
                pdwPosition = (DWORD *)dwParamB;
                *pdwPosition = dwPosition;
            }
            break;
            
        case SAM_MESSAGE_VOICE_POSITION_SET: //dwParamA = numéro de la voix, dwParamB = position courante (DWORD dwPosition)
            {
                SAM_VOICE_MIRROR    *psamVoiceMirror;
                DWORD               dwVoice;
                DWORD               dwFastAccess;
                
                dwVoice         = dwParamA&SAM_VOICE_MASK;
                dwFastAccess    = dwParamA&SAM_VOICE_FASTACCESS;
                
                if ( (dwVoice>=samVoiceData.dwVoiceVirtualCount) ||
                     (dwParamB==0) )
                    return -1;
                    
                psamVoiceMirror = &(samVoiceData.psamVoiceMirror[dwVoice]);
                
                SAM_VOICE_LockEnter ( dwFastAccess );
                psamVoiceMirror->dwUpdateFlag |= UPDATE_POSITION;
                psamVoiceMirror->dwInPlaySamplePosition = dwParamB;
                SAM_VOICE_LockLeave ( dwFastAccess );
            }
            break;
            
        case SAM_MESSAGE_VOICE_POSITIONTICK_GET: //dwParamA = numéro de la voix, dwParamB = adresse de retour pour la position tick courante (DWORD * pdwPosition)
            {
                SAM_VOICE_MIRROR    *psamVoiceMirror;
                DWORD               dwVoice;
                DWORD               dwFastAccess;
                DWORD               dwPosition, *pdwPosition;
                
                dwVoice         = dwParamA&SAM_VOICE_MASK;
                dwFastAccess    = dwParamA&SAM_VOICE_FASTACCESS;
                
                if ( (dwVoice>=samVoiceData.dwVoiceVirtualCount) ||
                     (dwParamB==0) )
                    return -1;
                    
                psamVoiceMirror = &(samVoiceData.psamVoiceMirror[dwVoice]);
                
                SAM_VOICE_LockEnter ( dwFastAccess );
                dwPosition = psamVoiceMirror->dwInPlayTickPosition;
                SAM_VOICE_LockLeave ( dwFastAccess );
                    
                pdwPosition = (DWORD *)dwParamB;
                *pdwPosition = dwPosition;
            }
            break;
            
        case SAM_MESSAGE_VOICE_POSITIONTICK_SET: //dwParamA = numéro de la voix, dwParamB = position tick courante (DWORD dwPosition)
            {
                SAM_VOICE_MIRROR    *psamVoiceMirror;
                DWORD               dwVoice;
                DWORD               dwFastAccess;
                
                dwVoice         = dwParamA&SAM_VOICE_MASK;
                dwFastAccess    = dwParamA&SAM_VOICE_FASTACCESS;
                
                if ( (dwVoice>=samVoiceData.dwVoiceVirtualCount) ||
                     (dwParamB==0) )
                    return -1;
                    
                psamVoiceMirror = &(samVoiceData.psamVoiceMirror[dwVoice]);
                
                SAM_VOICE_LockEnter ( dwFastAccess );
                psamVoiceMirror->dwUpdateFlag |= UPDATE_POSITIONTICK;
                psamVoiceMirror->dwInPlayTickPosition = dwParamB;
                SAM_VOICE_LockLeave ( dwFastAccess );
            }
            break;
            
        case SAM_MESSAGE_PROCESS_MIXER:
            SAM_ProcessProc_Mixer ( 1, NULL );
            break;
            
        case SAM_MESSAGE_MIXER_SET_RECORD2QUAKE:
            _OSI_EnterCriticalSection ( &samData.osiCriticalSection );
            samData.bQuakeRecordAudio       = dwParamA;
            samData.lQuakeRecordAudioLen    = dwParamB;
            _OSI_LeaveCriticalSection ( &samData.osiCriticalSection );
            
            _OSI_EnterCriticalSection ( &samData.osiCSOutputData );
            samData.MixerOutput_dwMCSamplesOffsetRead   = 0;
            samData.MixerOutput_dwMCSamplesOffsetWrite  = 0;
            samData.MixerOutput_dwMCSamplesReadyToRead  = 0;
            _OSI_LeaveCriticalSection ( &samData.osiCSOutputData );
            
            break;
            
        case SAM_MESSAGE_OUTPUT_FLUSH:
            {
                void * pAudioBufferOutput;
                long lLen;
                
                _OSI_EnterCriticalSection ( &samData.osiCSOutputData );
                /*samData.aiMain.DeviceLock ( &samData.aiMain, &pAudioBufferOutput, &lLen );
                
                memset ( 
                    pAudioBufferOutput, 
                    0, 
                    lLen * 
                    samData.aiMain.aiAudioFormat.dwChannelCount *
                    ikAudioInterfaceGetBitsPerSampleFromMode ( samData.aiMain.aiAudioFormat.dwBitsPerSampleModeTarget ) );
                    
                samData.aiMain.DeviceUnlock ( &samData.aiMain );
                */
                
                
                SAM_DirectSound_BeginPaint ( &pAudioBufferOutput );
            
                memset ( 
                    pAudioBufferOutput,
                    0,
                    samData.dwOutputHardwareChannel * samData.dwHardwareAndSoftwareBufferSamplesCount * sizeof(WORD) );

                SAM_DirectSound_EndPaint ( );
                
                _OSI_LeaveCriticalSection ( &samData.osiCSOutputData );
            }
            break;
        //dwParam1 = &SAM_SoftRestart
        
        case SAM_MESSAGE_SOFTRESTART:
            {
                SAM_SoftRestart     *psamSoftRestart;
                SAM_DATASTATE       samDataStateOld;
                SAM_DATASTATE       samDataStateNew;
                float               fDistanceWithinSpeakers;
                DWORD               dwTempA;
                DWORD               dwTempB;
                DWORD               dwIndex;
                SAM_RENDER254       samRender254Table[64];
                long                i, j;
                long                lError;
                void (*pSAM_RENDER_InitProc) ( float fDistanceWithinSpeakers, float fSampleRate, SAM_RENDER254 * psamRender );
                
                if (dwParamA)
                {
                    psamSoftRestart = (SAM_SoftRestart *)dwParamA;

                    //Sauve l'état général
                    SAM_DATA_SaveState ( &samDataStateOld );
                    
                    //Gestion du nouvel état
                    memcpy ( &samDataStateNew, &samDataStateOld, sizeof(SAM_DATASTATE) );
                    
                    //Le sampling rate
                    if (psamSoftRestart->dwSamplingRate!=0xFFFFFFFF)
                    {
                        switch (psamSoftRestart->dwSamplingRate)
                        {
                            case 22050:
                            case 24000:
                            case 32000:
                            case 44100:
                            case 48000:
                                break;
                                
                            default:
                                psamSoftRestart->dwSamplingRate = 24000;
                                break;
                        }
                        samDataStateNew.dwHardwaremixSampleRate = psamSoftRestart->dwSamplingRate;
                    }

                    //Le channel mode                        
                    if (psamSoftRestart->dwChannelMode!=0xFFFFFFFF)
                        samDataStateNew.dwChannelMode = psamSoftRestart->dwChannelMode;
                        
                    //Le périphérique
                    if (psamSoftRestart->dwDeviceSelect!=0xFFFFFFFF)
                        samDataStateNew.dwDeviceSelect = psamSoftRestart->dwDeviceSelect;
                        
                    //La latence
                    /*
                    if (psamSoftRestart->dwLatencyDuration!=0xFFFFFFFF)
                    {
                        dwTempA = psamSoftRestart->dwLatencyDuration;
                        if (dwTempA<sam_LATENCY_DURATION_MIN) dwTempA = sam_LATENCY_DURATION_MIN;
                        if (dwTempA>sam_LATENCY_DURATION_MAX) dwTempA = sam_LATENCY_DURATION_MAX;
                        samDataStateNew.dwHardwareBufferLatencySamplesCount = (samDataStateNew.dwHardwaremixSampleRate * dwTempA)/1000;
                    }
                    else dwTempA = (samDataStateNew.dwHardwareBufferLatencySamplesCount*1000) / samDataStateNew.dwHardwaremixSampleRate;
                    
                    //Le tampon
                    if (psamSoftRestart->dwBufferDuration!=0xFFFFFFFF)
                    {
                        dwTempB = psamSoftRestart->dwBufferDuration;    
                        if (dwTempB<(dwTempA+sam_BUFFER_DURATION_MIN)) dwTempB = dwTempA+sam_BUFFER_DURATION_MIN;
                        if (dwTempB>sam_BUFFER_DURATION_MAX) dwTempB = sam_BUFFER_DURATION_MAX;
                        samDataStateNew.dwHardwareAndSoftwareBufferSamplesCount = (samDataStateNew.dwHardwaremixSampleRate * dwTempB)/1000;
                    }
                    */
                    
                    //################################################################################
                    // Etape 1 : On reconstruit le render mode
                    
                    //Protection : Début
                    SAM_SFX_EnterLock ( );
                    _OSI_EnterCriticalSection ( &samData.osiCSOutputData );
                    SAM_VOICE_LockEnter ( 0 );
                
                    //Changement du mode de rendu à la volée, on a besoin de certaines variables...
                    SAM_DATA_RestoreState ( &samDataStateNew );
                
                    //Recherche et modifie les paramètres
                    SAM_RENDER_InitChannelsModeValues ( 
                        &fDistanceWithinSpeakers, 
                        (void **)&pSAM_RENDER_InitProc );
                    
                    //On stocke les modifications
                    SAM_DATA_SaveState ( &samDataStateNew );
                    SAM_DATA_RestoreState ( &samDataStateOld );

                    //Protection : Fin
                    SAM_VOICE_LockLeave ( 0 );                
                    _OSI_LeaveCriticalSection ( &samData.osiCSOutputData );
                    SAM_SFX_LeaveLock ( );
                    
                    //Création du nouveau render mode
                    memset ( samRender254Table,  0, 64 * sizeof(SAM_RENDER254) );
                    pSAM_RENDER_InitProc ( fDistanceWithinSpeakers, (float)samDataStateNew.dwHardwaremixSampleRate, samRender254Table );
                    for (i=0;i<64;i++)
                    {
                        SAM_RENDER_MaximizeLevel ( &samRender254Table[i], samDataStateNew.dwOutputSoftwareChannel, samDataStateNew.dwHardwaremixSampleRate );
                    }
                    
                    //################################################################################
                    // Etape 2 : Applique tous les changements !
                    
                    //Retourne dans la protection et tente d'appliquer les changements
                    //IKA_MixerFreeze ( 1 );
                    SAM_EnterProtectThreadMixer ( );
                                        
                    //Termine l'ancien DirectSound
                    SAM_DirectSound_Close ( );
                    //ikAudioInterfaceDeviceClose ( &samData.aiMain );
                    
                    //_OSI_Sleep ( 500 );
                    
                    //Applique les nouveaux paramètres
                    SAM_DATA_RestoreState ( &samDataStateNew );
                    samData.bReInitMixer = 1;
                    
                
                    //Sélection du périphérique de sortie
                    /*
                    lError = ikAudioInterfaceDeviceOpen ( 
                        &samData.aiMain, 
                        samData.dwHardwareDeviceSelected, 
                        samData.dwHardwaremixSampleRate, 
                        samData.likChannelCountMode,
                        1 );
                    */
                    
                    lError = SAM_DirectSound_Open ( 
                        samData.dwHardwareDeviceSelected,
                        samData.pDeviceParam,
                        samData.dwHardwaremixSampleRate,
                        samData.dwOutputHardwareChannel,
                        samData.dwHardwareAndSoftwareBufferSamplesCount,
                        &samData.dwHardwareAndSoftwareBufferSamplesCount );
                    
                    //Traîtement d'une éventuelle erreur de DirectSound
                    if (lError>=0)  //Pas d'erreur
                    {
                        //Applique les nouvelles tables de rendu
                        memcpy ( samData.psamRender254Table,  samRender254Table,  64 * sizeof(SAM_RENDER254) );
                    
                        //Récupération du périphérique sélectionné
                        samData.dwHardwareDeviceSelected = (DWORD)lError;
                        
                        //Applique le nouveau SR à la musique
                        _SAM_VOICE_GetPlayTick ( 
                            (float)samData.samMusic.dwCurrentSampleRate,
                            &samData.samMusic.dwInPlayTickIncrement );
                            
                        //Applique le nouveau SR au stream
                        _SAM_VOICE_GetPlayTick ( 
                            (float)samData.samStreaming.dwCurrentSampleRate,
                            &samData.samStreaming.dwInPlayTickIncrement ); 

                        //Applique le nouveau SR aux voix
                        for (dwIndex=0;dwIndex<samVoiceData.dwVoiceVirtualCount;dwIndex++)
                        {
                            _SAM_VOICE_GetPlayTick (
                                (float)samVoiceData.psamVoiceMirror[dwIndex].dwInPlaySampleRate,
                                &samVoiceData.psamVoiceMirror[dwIndex].dwInPlayTickIncrement );

                            _SAM_VOICE_GetPlayTick (
                                (float)samVoiceData.psamVoice[dwIndex].dwInPlaySampleRate,
                                &samVoiceData.psamVoice[dwIndex].dwInPlayTickIncrement );
                        }
                    }
                    else
                    {
                        //Restaure les anciennes données
                        SAM_DATA_RestoreState ( &samDataStateOld );
                        
                        /*
                        lError = ikAudioInterfaceDeviceOpen ( 
                            &samData.aiMain, 
                            samData.dwHardwareDeviceSelected, 
                            samData.dwHardwaremixSampleRate, 
                            samData.likChannelCountMode,
                            1 );
                        */

                        lError = SAM_DirectSound_Open ( 
                            samData.dwHardwareDeviceSelected,
                            samData.pDeviceParam,
                            samData.dwHardwaremixSampleRate,
                            samData.dwOutputHardwareChannel,
                            samData.dwHardwareAndSoftwareBufferSamplesCount,
                            &samData.dwHardwareAndSoftwareBufferSamplesCount );

                        if (lError<0)
                        {
                            
                        
                        }
                        else
                        {
                            //Récupération du périphérique sélectionné
                            samData.dwHardwareDeviceSelected = (DWORD)lError;
                        }
                    }
                
                    //La protection n'est plus nécessaire
                    //IKA_MixerFreeze ( 0 );
                    SAM_LeaveProtectThreadMixer ( );
                }
            }
            break;
            
        case SAM_MESSAGE_SAMPLINGRATE_SET:
            if (samData.dwHardwaremixSampleRate!=dwParamA)
            {
                SAM_SoftRestart samSoftRestart;
                memset ( &samSoftRestart, 0xFF, sizeof(SAM_SoftRestart) );
                samSoftRestart.dwSamplingRate = dwParamA;
                SAM_Message ( SAM_MESSAGE_SOFTRESTART, (DWORD)&samSoftRestart, 0 );
            }
            if (dwParamB)
            {
                pdwReturn = (DWORD *)dwParamB;
                *pdwReturn = samData.dwHardwaremixSampleRate;
            }
            break;
            
        case SAM_MESSAGE_DEVICESELECT_SET:
            if (samData.dwHardwareDeviceSelected!=dwParamA)
            {
                SAM_SoftRestart samSoftRestart;
                memset ( &samSoftRestart, 0xFF, sizeof(SAM_SoftRestart) );
                samSoftRestart.dwDeviceSelect = dwParamA;
                SAM_Message ( SAM_MESSAGE_SOFTRESTART, (DWORD)&samSoftRestart, 0 );
            }
            if (dwParamB)
            {
                pdwReturn = (DWORD *)dwParamB;
                *pdwReturn = samData.dwHardwareDeviceSelected;
            }
            break;
        
        case SAM_MESSAGE_LATENCYDURATION_SET:
            {
                SAM_SoftRestart samSoftRestart;
                memset ( &samSoftRestart, 0xFF, sizeof(SAM_SoftRestart) );            
                samSoftRestart.dwLatencyDuration = dwParamA;
                SAM_Message ( SAM_MESSAGE_SOFTRESTART, (DWORD)&samSoftRestart, 0 );
            }
            if (dwParamB)
            {
                pdwReturn = (DWORD *)dwParamB;
                *pdwReturn = (samData.dwHardwareBufferLatencySamplesCount*1000)/samData.dwHardwaremixSampleRate;
            }
            break;
            
        case SAM_MESSAGE_BUFFERDURATION_SET:
            {
                SAM_SoftRestart samSoftRestart;
                memset ( &samSoftRestart, 0xFF, sizeof(SAM_SoftRestart) );            
                samSoftRestart.dwBufferDuration = dwParamA;
                SAM_Message ( SAM_MESSAGE_SOFTRESTART, (DWORD)&samSoftRestart, 0 );
            }
            if (dwParamB)
            {
                pdwReturn = (DWORD *)dwParamB;
                *pdwReturn = (samData.dwHardwareAndSoftwareBufferSamplesCount*1000)/samData.dwHardwaremixSampleRate;
            }
            break;
            
        case SAM_MESSAGE_RENDERMODE_SET:
            if (samData.dwChannelMode!=dwParamA)
            {
                SAM_SoftRestart samSoftRestart;
                SAM_RENDER_ValidateChannelsModeValues ( &dwParamA );
                memset ( &samSoftRestart, 0xFF, sizeof(SAM_SoftRestart) );
                samSoftRestart.dwChannelMode = dwParamA;
                SAM_Message ( SAM_MESSAGE_SOFTRESTART, (DWORD)&samSoftRestart, 0 );
            }
            
            if (dwParamB)
            {
                pdwReturn = (DWORD *)dwParamB;
                *pdwReturn = samData.dwChannelMode;
            }
            break;
            
        case SAM_MESSAGE_MAXUSAGE_SET:
            if (dwParamA<  0) dwParamA =   0;
            if (dwParamA> 30) dwParamA =  30;

            if ( (samData.lInstancesCount>1) && ( (samData.dwForceAffinityMode==0) || (samData.dwForceAffinityMode==1) ) )
                dwParamA = 70; //Fixe une valeur de 70% en mode dual core ou auto avec plus d'un core
            
            _OSI_EnterCriticalSection ( &samData.osiCSOutputData );
            samData.fMaxProcessorUsagePercent = (float)dwParamA;
            _OSI_LeaveCriticalSection ( &samData.osiCSOutputData );
            
            if (dwParamB)
            {
                pdwReturn = (DWORD *)dwParamB;
                *pdwReturn = (DWORD)samData.fMaxProcessorUsagePercent;
            }
            break;
            
        case SAM_MESSAGE_LIMITERLEVEL_SET:
            SAM_LimiterSet ( dwParamA );
            if (dwParamB)
            {
                pdwReturn = (DWORD *)dwParamB;
                *pdwReturn = samData.lLimiterMode;
            }
            break;
            
        case SAM_MESSAGE_SFX_DEFAULTLEVEL_SET:
            SAM_SFX_SetUserDefaultLevel ( dwParamA, *((float *)dwParamB) );
            break;
            
        case SAM_MESSAGE_LATENCYDURATION_BU_GET:
            _OSI_EnterCriticalSection ( &samData.osiCriticalSection );
            if (dwParamA)
                //samData.aiMain.DeviceGetLatency ( &samData.aiMain, ((long *)dwParamA) );
                *((long *)dwParamA) = samData.lActualLatencyInSamples;
                //SAM_OutputGetAverageDeltaRW() + samData.lActualLatencyInSamples;
            
            
            if (dwParamB)
                *((long *)dwParamB) = samData.lTotalBufferUnderrunCount;                
            _OSI_LeaveCriticalSection ( &samData.osiCriticalSection );
            break;
            
        case SAM_MESSAGE_MIXER_OUTPUT_KEEP:
            {
                _OSI_EnterCriticalSection ( &samData.osiCSOutputData );
                
                if (dwParamA>samData.MixerOutput_dwMCSamplesReadyToRead) dwParamA = samData.MixerOutput_dwMCSamplesReadyToRead;
                
                samData.MixerOutput_dwMCSamplesReadyToRead = dwParamA;
                
                if (dwParamA>samData.MixerOutput_dwMCSamplesOffsetWrite)
                {
                    samData.MixerOutput_dwMCSamplesOffsetRead = samData.MixerOutput_dwMCSamplesOffsetWrite - dwParamA;
                    samData.MixerOutput_dwMCSamplesOffsetRead+=samData.MixerOutput_dwSamplesCount;
                }
                else
                    samData.MixerOutput_dwMCSamplesOffsetRead  = samData.MixerOutput_dwMCSamplesOffsetWrite - dwParamA;
                    
                    
                //if (samData.MixerOutput_dwMCSamplesOffsetRead>=samData.MixerOutput_dwSamplesCount) 
                //    samData.MixerOutput_dwMCSamplesOffsetRead+=samData.MixerOutput_dwSamplesCount;
                
                _OSI_LeaveCriticalSection ( &samData.osiCSOutputData );
            }
            break;
            
        case SAM_MESSAGE_MIXER_OUTPUT_GET:
            if ((dwParamA==0xFFFFFFFF)&&(dwParamB==0xFFFFFFFF))
            {
                _OSI_EnterCriticalSection ( &samData.osiCSOutputData );
                samData.MixerOutput_dwMCSamplesOffsetRead   = 0;
                samData.MixerOutput_dwMCSamplesOffsetWrite  = 0;
                samData.MixerOutput_dwMCSamplesReadyToRead  = 0;
                _OSI_LeaveCriticalSection ( &samData.osiCSOutputData );
            }
            else if ((dwParamA==0)&&(dwParamB==0))
            {
                DWORD dwIndex;
                _OSI_EnterCriticalSection ( &samData.osiCSOutputData );
                dwIndex = samData.MixerOutput_dwMCSamplesReadyToRead;
                _OSI_LeaveCriticalSection ( &samData.osiCSOutputData );
                
                return dwIndex;
            }
            else
            {
                INT16 * pi16Buffer;
                DWORD dwNeededMCSamples;
                DWORD dwCopiedMCSamples;
                DWORD dwOffsetSource;
                DWORD dwOffsetSourceMax;
                DWORD dwCopyLen;
                DWORD dwIndex;
                
                
                dwNeededMCSamples   = dwParamA;                
                pi16Buffer          = (INT16 *)dwParamB;
                
                _OSI_EnterCriticalSection ( &samData.osiCSOutputData );
                
                if (dwNeededMCSamples>samData.MixerOutput_dwMCSamplesReadyToRead) dwCopiedMCSamples = samData.MixerOutput_dwMCSamplesReadyToRead;
                else                                                              dwCopiedMCSamples = dwNeededMCSamples;
                
                dwCopyLen           = dwCopiedMCSamples * samData.dwOutputHardwareChannel;
                dwOffsetSource      = samData.MixerOutput_dwMCSamplesOffsetRead * samData.dwOutputHardwareChannel; 
                dwOffsetSourceMax   = samData.MixerOutput_dwSamplesCount;
                
                for (dwIndex=0;dwIndex<dwCopyLen;dwIndex++)
                {
                    pi16Buffer[dwIndex] = samData.MixerOutput_pi16Buffer[dwOffsetSource];
                        
                    dwOffsetSource++;
                    if (dwOffsetSource>=dwOffsetSourceMax) dwOffsetSource -= dwOffsetSourceMax;
                }
                
                samData.MixerOutput_dwMCSamplesReadyToRead -= dwCopiedMCSamples;
                samData.MixerOutput_dwMCSamplesOffsetRead   = dwOffsetSource / samData.dwOutputHardwareChannel;
                
                
                
                _OSI_LeaveCriticalSection ( &samData.osiCSOutputData );
                
                return dwCopiedMCSamples;
            }
            break;
            
        case SAM_MESSAGE_UNDEFINED:
            return -1;
            
        default:
            return -1;
    
    }

    return 0;
}