#include "sam_header.h"
#include "sam_data.h"
#include "sam_voice.h"

/*
#define UPDATE_SFX                  0x00000001                  //Il faut remettre à zéro la position et plein d'autres données !!!
#define UPDATE_PLAYRATE             0x00000002
#define UPDATE_LOOP                 0x00000004
#define UPDATE_PLAY                 0x00000008
#define UPDATE_USED                 0x00000010
#define UPDATE_MASTERLEVEL          0x00000020
#define UPDATE_FXLEVEL              0x00000040
#define UPDATE_FXFILTER             0x00000080
#define UPDATE_RENDER               0x00000100
#define UPDATE_RESET                0xFFFFFFFF
*/

void SAM_VOICE_MirrorFastReset ( void * pData, DWORD dw32BytesCount )
{
    __asm {
            mov     ecx, dw32BytesCount
            mov     esi, pData
            
            shl     ecx, 5
            xorps   xmm0, xmm0
            
        _loop:    
            movaps  [esi+ecx-32], xmm0
            movaps  [esi+ecx-16], xmm0
            
            sub     ecx, 32
            jnz     _loop
    }
}

void SAM_VOICE_Mirror ( DWORD dwInternalTimer )
{
    DWORD               dwVoiceIndex;
    SAM_VOICE           *psamVoice;
    SAM_VOICE_MIRROR    *psamVoiceMirror;
    DWORD               dwVoiceUsedCountLooped;
    DWORD               dwVoiceUsedCountUnlooped;
    DWORD               dwFreezeCopyMirror;
    SAM_SFX             *psamSFX;
    DWORD               dwInterpolationTime;
    DWORD               dwNewSamplePosition;
    DWORD               dwGranuleSamplePosition;
    DWORD               dwGranuleEntry;
    DWORD               dwSamplesPerGranule;
    DWORD               dwBytesPerSampleCount;
    DWORD               dwCount;
    
    //Devons-nous copier le VoiceMirror ?
    _OSI_EnterCriticalSection ( &samVoiceData.osiCSFreezeCopyMirror );
    dwFreezeCopyMirror = samVoiceData.dwFreezeCopyMirror;
    _OSI_LeaveCriticalSection ( &samVoiceData.osiCSFreezeCopyMirror );
    if (dwFreezeCopyMirror) return;

    
    
    //Prépare les pointeurs...
    dwVoiceIndex                = 0;
    psamVoice                   = samVoiceData.psamVoice;
    psamVoiceMirror             = samVoiceData.psamVoiceMirror;
    dwVoiceUsedCountLooped      = 0;
    dwVoiceUsedCountUnlooped    = 0;

    SAM_VOICE_LockEnter ( 0 );

    //samData.dwInternalTimer = dwInternalTimer; //Write only !

    for (dwVoiceIndex=0;
         dwVoiceIndex<samVoiceData.dwVoiceVirtualCount;
         dwVoiceIndex++,
         psamVoice++,
         psamVoiceMirror++)
    {
    
        if (psamVoiceMirror->dwUpdateFlag)
        {
            //##############################################################################################################
            //Reset de la voix !
            if ((psamVoiceMirror->dwUpdateFlag)&UPDATE_RESET)//==UPDATE_RESET)
            {
                //On fixe tout plein de nouvelles choses...
                memset ( psamVoice, 0, sizeof(SAM_VOICE) );
                psamVoice->bIsUsed                      = psamVoiceMirror->bIsUsed;
                psamVoice->dwHandleSFX                  = psamVoiceMirror->dwHandleSFX;
                //psamVoice->bLoopMode                    = 0;
                //psamVoice->bIsPlay                      = 0;
                psamVoice->fFX_LevelDistanceGain        = psamVoiceMirror->fFX_LevelDistanceGain;
                psamVoice->fFX_IIR_LowPassRatio         = psamVoiceMirror->fFX_IIR_LowPassRatio;
                psamVoice->fLevelMasterGain             = psamVoiceMirror->fLevelMasterGain;
                psamVoice->psamRender254                = psamVoiceMirror->psamRender254;
                psamVoice->lRenderPositionTarget        = psamVoiceMirror->lRenderIndexTarget;
                psamVoice->lRenderPositionCurrent_F16   = psamVoiceMirror->lRenderIndexTarget<<16;
                //psamVoice->lRenderIncrement_F16         = 0;
                psamVoice->dwCurrentGranuleID           = 0xFFFFFFFF;
                psamVoice->lLevelGainEnable             = psamVoiceMirror->lLevelGainEnable;
                psamVoice->fLevelGainL                  = psamVoiceMirror->fLevelGainL;
                psamVoice->fLevelGainR                  = psamVoiceMirror->fLevelGainR;
                psamVoice->fSpeedShift                  = psamVoiceMirror->fSpeedShift;
                
            }
            else
            {
                //Demande la libération de la voix via "SAM_VOICE_Free(...)"
                if ((psamVoiceMirror->dwUpdateFlag)&UPDATE_UNUSED)
                {
                    psamVoice->bIsUsed              = 0; //La voice est libérée
                    psamVoiceMirror->dwUpdateFlag   = 0; //Les autres flags ne servent plus à rien... lol
                    psamVoiceMirror->bIsUsed        = 0;
                }
                else
                {
                    psamVoiceMirror->bIsUsed        = psamVoice->bIsUsed;
                }
                
            }

            //##############################################################################################################
            //On place un nouveau SFX ! Super, j'ai enfin un son à jouer... C'est trop sympa ! Merci !
            if ((psamVoiceMirror->dwUpdateFlag)&UPDATE_SFX)
            {
                if (!SAM_SFX_GetIt ( psamVoiceMirror->dwHandleSFX, &psamSFX ))
                {
                    psamVoice->dwHandleSFX                      = psamVoiceMirror->dwHandleSFX;
                    //psamVoice->dwInPlayTickIncrement            = psamVoiceMirror->dwInPlayTickIncrement;
                    //psamVoice->resample_pfFIRCoef               = psamVoiceMirror->resample_pfFIRCoef;
                    //psamVoice->bIsPlay                          = psamVoiceMirror->bIsPlay;

                    psamVoice->dwCurrentGranuleID               = psamSFX->dwGranuleFirst;
                    psamVoice->dwInPlayGranulePosition          = 0;
                    psamVoice->dwInPlaySamplePosition           = 0;
                    psamVoice->dwInPlaySamplePositionPrevious   = 0;
                    psamVoice->dwInPlayTickPosition             = 0;
                    psamVoice->fFX_IIR_LowPassValue             = 0;
                    psamVoice->fLevelSFXReplayGain              = psamSFX->fReplayGain * psamSFX->fUserDefaultGain;

                    //On vide les variables
                    if (!((psamVoiceMirror->dwUpdateFlag)&UPDATE_RESET))
                    {
                        SAM_VOICE_MirrorFastReset ( psamVoice->interpolation_fStackValue, 4 );
                        SAM_VOICE_MirrorFastReset ( psamVoice->stack_fBufferStackValue,   ( sam_VOICEBUFFERSTACK_COUNT * 4 ) >> 5 );
                        //memset ( psamVoice->interpolation_fStackValue, 0, 32 * sizeof(float) );
                        //memset ( psamVoice->stack_fBufferStackValue, 0, sam_VOICEBUFFERSTACK_COUNT * sizeof(float) );
                    }
                }
            }
        
            //##############################################################################################################
            //Position de lecture modifiée !
            if ((psamVoiceMirror->dwUpdateFlag)&UPDATE_POSITION)
            {
                
                if (!SAM_SFX_GetIt ( psamVoiceMirror->dwHandleSFX, &psamSFX ))
                {
                    dwNewSamplePosition = psamVoiceMirror->dwInPlaySamplePosition;
                    
                    if (dwNewSamplePosition<psamSFX->dwSamplesCount)
                    {
                        dwGranuleEntry          = psamSFX->dwGranuleFirst;
                        dwGranuleSamplePosition = dwNewSamplePosition;

                        //Nombre de samples par granule
                        SAM_FormatGetBytesCount ( psamSFX->bFormat, &dwBytesPerSampleCount );
                        dwSamplesPerGranule = sam_GRANULE_BUFFERBYTES / dwBytesPerSampleCount;
                        
                        if (dwGranuleSamplePosition>=dwSamplesPerGranule)
                        {
                            dwCount = dwGranuleSamplePosition/dwSamplesPerGranule;
                            dwGranuleSamplePosition = dwGranuleSamplePosition%dwSamplesPerGranule;
                            
                            __asm {
                                    mov     esi, samGranulesData.pGranule
                                    mov     eax, dwGranuleEntry
                                    mov     ecx, dwCount
                                _loop_next_granule:
                                    shl     eax, 12
                                    mov     eax, [esi+eax+4080+8]
                                    
                                    dec     ecx
                                    jnz     _loop_next_granule
                                    mov     dwGranuleEntry, eax
                            }
                                                    
                            /*do {
                                dwGranuleEntry = samGranulesData.pGranule[dwGranuleEntry].dwIdGranuleNext;
                                dwCount--;
                            } while (dwCount);
                            */
                            /*
                            do {
                                dwGranuleSamplePosition -= dwSamplesPerGranule;
                                
                                dwGranuleEntry = samGranulesData.pGranule[dwGranuleEntry].dwIdGranuleNext;
                                //SAM_GranulesGetNext ( dwGranuleEntry, &dwGranuleEntry );
                            } while (dwGranuleSamplePosition>=dwSamplesPerGranule);
                            */
                        }
                        
                        psamVoice->dwInPlaySamplePosition   = dwNewSamplePosition;
                        psamVoice->dwInPlayGranulePosition  = dwGranuleSamplePosition;
                        psamVoice->dwCurrentGranuleID       = dwGranuleEntry;
                    }
                }
            }
        
            if ((psamVoiceMirror->dwUpdateFlag)&UPDATE_POSITIONTICK)
            {
                psamVoice->dwInPlayTickPosition = psamVoiceMirror->dwInPlayTickPosition;
            }

            //Vitesse de lecture modifiée !
            if ((psamVoiceMirror->dwUpdateFlag)&UPDATE_PLAYRATE)
            {
                psamVoice->dwInPlayTickIncrement = psamVoiceMirror->dwInPlayTickIncrement;
                psamVoice->dwInPlaySampleRate    = psamVoiceMirror->dwInPlaySampleRate;
                psamVoice->fSpeedShift           = psamVoiceMirror->fSpeedShift;
            }


            //Play or not play ! That is the question...
            if (((psamVoiceMirror->dwUpdateFlag)&UPDATE_PLAY)&&(psamVoiceMirror->bIsPlay))
            {
                psamVoice->bIsPlay = psamVoiceMirror->bIsPlay;
            }
            
            //Les drapeaux ont permi la mise à jour de la voix.
            psamVoiceMirror->dwUpdateFlag = 0;
        }
                              

        psamVoice->bLoopMode                = psamVoiceMirror->bLoopMode;
        psamVoice->fLevelMasterGain         = psamVoiceMirror->fLevelMasterGain;
        psamVoice->fFX_LevelDistanceGain    = psamVoiceMirror->fFX_LevelDistanceGain;
        psamVoice->fFX_IIR_LowPassRatio     = psamVoiceMirror->fFX_IIR_LowPassRatio;
        psamVoice->psamRender254            = psamVoiceMirror->psamRender254;
        psamVoice->fDistanceMeters          = psamVoiceMirror->fDistanceMeters;
        psamVoice->lAngleDegrees            = psamVoiceMirror->lAngleDegrees;
        psamVoice->lRenderPositionTarget    = psamVoiceMirror->lRenderIndexTarget;
        psamVoice->lLevelGainEnable         = psamVoiceMirror->lLevelGainEnable;
        psamVoice->fLevelGainL              = psamVoiceMirror->fLevelGainL;
        psamVoice->fLevelGainR              = psamVoiceMirror->fLevelGainR;
        psamVoice->fSpeedShift              = psamVoiceMirror->fSpeedShift;
        
        
        if (psamVoiceMirror->lRenderOrderChangeFlag)
        {
            //On vient de détecter un changement de l'origine
            psamVoiceMirror->lRenderOrderChangeFlag = 0;
        }
        
        //Copie de l'état de lecture        
        psamVoiceMirror->bIsPlayedState         = psamVoice->bIsPlay;
        psamVoiceMirror->dwInPlaySamplePosition = psamVoice->dwInPlaySamplePosition;
        psamVoiceMirror->dwInPlayTickPosition   = psamVoice->dwInPlayTickPosition;
    }

    SAM_VOICE_LockLeave ( 0 );
    
    //_OSI_LeaveCriticalSection ( &samVoiceData.osiCSFreezeCopyMirror );
}



        /*
            
            //Mesure la difference temporelle entre le précédent changement et celui-ci
            dwInterpolationTime = psamVoiceMirror->dwTimeRenderUpdatedCurrent - psamVoiceMirror->dwTimeRenderUpdatedPrevious;
            if (dwInterpolationTime<=0)
            {
                //La différence temporelle est trop faible, on applique un changement immédiat
                psamVoice->lRenderIncrement_F24         = 0;
                psamVoice->lRenderPositionCurrent_F24   = (psamVoiceMirror->lRenderIndexTarget)<<24;
            }
            else
            {
                long lTotalInterpolationSamples;
                long lDistanceVariation_F24;
                long lSensVariation;
                long lRenderPositionTarget_F24;
                
                //On limite la différence à une valeur maximale
                if (dwInterpolationTime>(1000/sam_RENDERFREQMIN))
                    dwInterpolationTime = (1000/sam_RENDERFREQMIN);
                
                //On fixe la durée totale de l'interpolation en samples
                lTotalInterpolationSamples = (long)(samData.dwHardwaremixSampleRate * dwInterpolationTime / 1000);
                
                //La position de destination
                lRenderPositionTarget_F24 = (psamVoiceMirror->lRenderIndexTarget)<<24;

                //Mesure la distance de variation
                lDistanceVariation_F24 = lRenderPositionTarget_F24 - psamVoice->lRenderPositionCurrent_F24;
                
                //Choisi le sens de variation
                if (lDistanceVariation_F24==0)
                {
                    //Pas de variation
                    psamVoice->lRenderIncrement_F24         = 0;
                    psamVoice->lRenderPositionCurrent_F24   = (psamVoiceMirror->lRenderIndexTarget)<<24;
                }
                else if (sam_ABS(lDistanceVariation_F24)<=(32<<24))
                {
                    //Variation minimale par défaut
                    if (lDistanceVariation_F24>0)   lSensVariation =  1;
                    else                            lSensVariation = -1;
                    
                    psamVoice->lRenderIncrement_F24 = sam_ABS(lDistanceVariation_F24) / lTotalInterpolationSamples;
                    psamVoice->lRenderIncrement_F24 *= lSensVariation;
                }
                else
                {
                    //Exemple A : Current =  7, Target = 52     => Sens = -1
                    //Exemple B : Current = 40, Target = 1      => Sens =  1
                    
                    if (psamVoice->lRenderPositionCurrent_F24 < lRenderPositionTarget_F24 )
                    {
                        //On est dans l'exemple A
                        lSensVariation          = -1;    
                        lDistanceVariation_F24  = psamVoice->lRenderPositionCurrent_F24 + ( 0x3FFFFFFF - lRenderPositionTarget_F24 );
                    }
                    else
                    {
                        //On est dans l'exemple B
                        lSensVariation          =  1;
                        lDistanceVariation_F24  = ( 0x3FFFFFFF - psamVoice->lRenderPositionCurrent_F24 ) + lRenderPositionTarget_F24;
                    }
                    
                    psamVoice->lRenderIncrement_F24 = lDistanceVariation_F24 / lTotalInterpolationSamples;
                    psamVoice->lRenderIncrement_F24 *= lSensVariation;
                }
            }
        }
            
            
            
            
            
            
            /*
            lDistanceVariation = psamVoice->lRenderIndexTarget - psamVoice->lRenderIndexCurrent;
            if (lDistanceVariation==0)
            {
                psamVoice->lRenderIndexStepDirection = 0;
            }
            else
            {
                if (sam_ABS(lDistanceVariation)<=32)
                {
                    if (lDistanceVariation>0) psamVoice->lRenderIndexStepDirection =  1;
                    else                      psamVoice->lRenderIndexStepDirection = -1;
                    psamVoice->lRenderIndexDecimalCeiling /= sam_ABS(lDistanceVariation);
                }
                else
                {
                    
                
                }
                    
            }
            */
        
        
        /*
        //Détermine la vitesse de variation de l'incrément pour le render
        //La vitesse de variation correspond à la fréquence d'échantillonnage
        lDistanceVariation = psamVoice->lRenderIndexTarget - psamVoice->lRenderIndexCurrent;
        if (lDistanceVariation==0)
        {
            psamVoice->lRenderIndexStepDirection = 0;
            
        }
        else if (sam_ABS(lDistanceVariation)<=32)
        {
            if (psamVoice->lRenderIndexTarget
        
        }
        
        /*
            La routine
            
        psamVoice->lRenderIndexDecimalValue += lNbSamples;
        if (psamVoice->lRenderIndexDecimalValue>=psamVoice->lRenderIndexDecimalCeiling)
        {
            psamVoice->lRenderIndexDecimalValue%=psamVoice->lRenderIndexDecimalCeiling;
            psamVoice->lRenderIndexCurrent += psamVoice->lRenderIndexStepDirection;
            if (psamVoice->lRenderIndexCurrent==psamVoice->lRenderIndexTarget)
                psamVoice->lRenderIndexStepDirection = 0;
        }
        */
