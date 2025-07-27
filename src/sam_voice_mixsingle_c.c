#include "sam_header.h"
#include "sam_data.h"
#include "sam_voice.h"

extern     DWORD       dwMaskPS_0111[4];
extern     DWORD       dwZeroPS[4];



void SAM_VOICE_MixSingleGranule_C ( SAM_VOICE * psamVoice, SAM_SFX * psamSFX, float fGlobalGain, DWORD dwLoopCount, float * pfSoftwareBuffer, DWORD dwSoftwareBufferChannelCount )
{
    float       fProcessValue[2];
    DWORD       dwInPlaySamplePosition;
    DWORD       dwInPlaySamplePositionPrevious;
    DWORD       dwInPlayTickPosition;
    DWORD       dwInPlayTickIncrement;
    DWORD       dwInPlayGranulePosition;
    float       *interpolation_fStackValue;
    float       *pfInterpolator;

    DWORD       dwSamplesCount;


    float       *stack_fBufferStackValue;
    DWORD       stack_dwBufferStackPosition;

    BYTE        bFormat;
    BYTE        bLoop;
    //BYTE        *pAudioDataB8;
    DWORD       *pAudioDataD32;
    //float       *pAudioDataF32;
    //DWORD       *pAudioDataXD4;
    //QWORD       *pAudioDataQ64;

    DWORD       dwTemp;

    DWORD       dwCurrentGranuleID;
    DWORD       dwBytesPerSampleCount;
    DWORD       dwSamplesPerGranuleCount;

    float       *pfBufferOut;

    float       fFX_IIR_LowPassRatio;
    float       fFX_IIR_LowPassValue;

    long        i, j;

    DWORD       render_dwEntryCount;           // 0...32
    DWORD       *render_dwDelayIndex;       // [32] Index in the delay line
    DWORD       *render_dwChannelIndex;     // [32] Channel index
    float       *render_fDelayGain;         // [32] Gain
    DWORD       dwRenderEntryIndex;

    DWORD       dwOutputCount;
    
    QWORD       qwTimeStampA;
    QWORD       qwTimeStampB;
    static QWORD        qwTimeTotal = 0;
    static QWORD        qwSamplesCount = 0;
    DWORD       dwSoftwareBufferChannelCount4;
    
    //Lecture des données en R/O
    bFormat                                     = psamSFX->bFormat;
    bLoop                                       = (unsigned char)psamVoice->bLoopMode;
    stack_fBufferStackValue                     = &(psamVoice->stack_fBufferStackValue[0]);
    dwInPlayTickIncrement                       = psamVoice->dwInPlayTickIncrement;
    dwSamplesCount                              = psamSFX->dwSamplesCount;
    pfBufferOut                                 = pfSoftwareBuffer;
    fFX_IIR_LowPassRatio                        = psamVoice->fFX_IIR_LowPassRatio;
    render_dwEntryCount                         = psamVoice->psamRender254->dwEntryCount;
    render_dwDelayIndex                         = psamVoice->psamRender254->dwDelayIndex;
    //render_dwChannelIndex                       = psamVoice->psamRender->dwChannelIndex;
    render_fDelayGain                           = psamVoice->psamRender254->fDelayGain;

    //Nombre de samples par granule
    SAM_FormatGetBytesCount ( bFormat, &dwBytesPerSampleCount );
    dwSamplesPerGranuleCount                    = sam_GRANULE_BUFFERBYTES / dwBytesPerSampleCount;

    //Lecture des données en R/W
    dwInPlaySamplePosition                      = psamVoice->dwInPlaySamplePosition;
    dwInPlaySamplePositionPrevious              = psamVoice->dwInPlaySamplePositionPrevious;
    dwInPlayTickPosition                        = psamVoice->dwInPlayTickPosition;
    dwInPlayGranulePosition                     = psamVoice->dwInPlayGranulePosition;
    stack_dwBufferStackPosition                 = psamVoice->stack_dwBufferStackPosition;
    dwCurrentGranuleID                          = psamVoice->dwCurrentGranuleID;
    fFX_IIR_LowPassValue                        = psamVoice->fFX_IIR_LowPassValue;
    interpolation_fStackValue                   = psamVoice->interpolation_fStackValue;

    //Les données du granule en cours...
    SAM_GranulesGetData ( dwCurrentGranuleID, &pAudioDataD32 );
    //pAudioDataF32 = (float *)pAudioDataB8;
    //pAudioDataXD4 = (DWORD *)pAudioDataB8;
    //pAudioDataQ64 = (QWORD *)pAudioDataB8;
    //pAudioDataD32 = (DWORD *)pAudioDataB8;
    dwOutputCount = dwLoopCount;
    

    if (bFormat&sam_FORMAT_CHANNELMASK)
    {
        //Traitement avec un son stéréo : Resampling + Volume + Mixage Stéréo
        do {
            //*******************************************************
            //Devons-nous charger une nouvelle donnée ?
            if (dwInPlaySamplePosition!=dwInPlaySamplePositionPrevious)
            {
                for (i=0;i<15;i++)
                {
                    interpolation_fStackValue[i   ] = interpolation_fStackValue[i+1 ];
                    interpolation_fStackValue[i+16] = interpolation_fStackValue[i+17];
                }
#if ika_INTERNAL_AUDIO_FORMAT == sam_FORMAT_MONO_PCM16
                interpolation_fStackValue[15] = SAM_LPCM_DecodeValue16 ( pAudioDataD32[dwInPlayGranulePosition], 0 );
                interpolation_fStackValue[31] = SAM_LPCM_DecodeValue16 ( pAudioDataD32[dwInPlayGranulePosition], 1 );
#endif

#if ika_INTERNAL_AUDIO_FORMAT == sam_FORMAT_MONO_XD4
                interpolation_fStackValue[15] = SAM_XD4_DecodeValue ( pAudioDataD32[((dwInPlayGranulePosition>>1)&0xFFFFFFFE)  ], dwInPlayGranulePosition&3 );
                interpolation_fStackValue[31] = SAM_XD4_DecodeValue ( pAudioDataD32[((dwInPlayGranulePosition>>1)&0xFFFFFFFE)+1], dwInPlayGranulePosition&3 );
#endif
                dwInPlaySamplePositionPrevious = dwInPlaySamplePosition;
            }

            //*******************************************************
            // Génération de la phase sur 10 bits
            dwTemp = (dwInPlayTickPosition>>14)&0x3FF;

            //*******************************************************
            // Application de l'interpolation
            fProcessValue[0] = 0;
            fProcessValue[1] = 0;
            pfInterpolator = &samData_f32InterpolationData_1024_16[dwTemp<<4];
            for (i=0;i<16;i++)
            {
                fProcessValue[0] += interpolation_fStackValue[i   ] * pfInterpolator[i];
                fProcessValue[1] += interpolation_fStackValue[i+16] * pfInterpolator[i];
            }

            //*******************************************************
            //Application du gain final et stockage
            pfBufferOut[samData.dwChannelIndexLeft ] += fProcessValue[0] * fGlobalGain;
            pfBufferOut[samData.dwChannelIndexRight] += fProcessValue[1] * fGlobalGain;

            //On déplace le pointeur d'échantillon
            dwInPlayTickPosition    += dwInPlayTickIncrement;
            dwInPlaySamplePosition  += dwInPlayTickPosition>>24;
            dwInPlayGranulePosition += dwInPlayTickPosition>>24;
            dwInPlayTickPosition    &= 0xFFFFFF;
            
            if (bLoop)
            {
                //Devons-nous boucler ?
                if (dwInPlaySamplePosition>psamSFX->dwLoopEndPositionSample)
                {
                    dwTemp                  = dwInPlaySamplePosition - psamSFX->dwLoopEndPositionSample - 1;
                    dwInPlaySamplePosition  = psamSFX->dwLoopBeginPositionSample + dwTemp;
                    dwInPlayGranulePosition = psamSFX->dwLoopBeginGranulePositionSample + dwTemp;
                    dwCurrentGranuleID      = psamSFX->dwLoopBeginGranuleID;
                    
                    //Mise à jour des pointeurs de données sources
                    SAM_GranulesGetData ( dwCurrentGranuleID, &pAudioDataD32 );
                }
            }
            else
            {
                //Avons-nous atteint la fin de cet échantillon ?
                if (dwInPlaySamplePosition>=dwSamplesCount)
                {
                    //Fin de la lecture
                    dwInPlaySamplePosition      = 0;
                    dwInPlayGranulePosition     = 0; //dwInPlaySamplePosition;
                    psamVoice->bIsPlay          = 0;
                    //psamVoice->bIsUsed        = 0;
                    
                    //Retourne au premier granule
                    dwCurrentGranuleID          = psamSFX->dwGranuleFirst;
                    dwOutputCount               = 1;
                    break;
                
                
                }
            }

            //Avons-nous atteint la fin de ce granule ?
            if (dwInPlayGranulePosition>=dwSamplesPerGranuleCount)
            {
                //On va au début du granule suivant
                dwInPlayGranulePosition -= dwSamplesPerGranuleCount;
                
                //Cherche le granule suivant
                SAM_GranulesGetNext ( dwCurrentGranuleID, &dwCurrentGranuleID );

                //Mise à jour des pointeurs de données sources
                SAM_GranulesGetData ( dwCurrentGranuleID, &pAudioDataD32 );
            }

            pfBufferOut += dwSoftwareBufferChannelCount;
        } while (--dwOutputCount);

    }
    else
    {
    
        __asm {
            push eax
            push edx
            rdtsc
            mov dword ptr qwTimeStampA, eax
            mov dword ptr qwTimeStampA+4, edx
            pop edx
            pop eax
        }
            
    
        
        //Traitement avec son mono : Resampling + IIR + Volume + Rendering
        do {
            qwSamplesCount++;
            
            //*******************************************************
            //Devons-nous charger une nouvelle donnée ?
            if (dwInPlaySamplePosition!=dwInPlaySamplePositionPrevious)
            {
#if ika_INTERNAL_AUDIO_FORMAT == sam_FORMAT_MONO_PCM16            
                fProcessValue[0] = SAM_LPCM_DecodeValue16 ( pAudioDataD32[dwInPlayGranulePosition>>1], dwInPlayGranulePosition&1 );
#endif                
                
#if ika_INTERNAL_AUDIO_FORMAT == sam_FORMAT_MONO_XD4
                fProcessValue[0] = SAM_XD4_DecodeValue ( pAudioDataD32[dwInPlayGranulePosition>>2], (dwInPlayGranulePosition&3) );
#endif
                
                /*__asm {
                
                    mov esi, interpolation_fStackValue
                    
                    movups  xmm0, [esi+ 4]
                    movss   xmm4, fProcessValue
                    movups  xmm1, [esi+20]
                    movups  xmm2, [esi+36]
                    movups  xmm3, [esi+52] //On déborde un peu, mais c'est pas grave car la pile fait plus de 16 points (pour la gestion en stéréo)
                    
                    movaps  [esi+ 0], xmm0
                    movaps  [esi+16], xmm1
                    movaps  [esi+32], xmm2
                    movaps  [esi+48], xmm3
                    movss   [esi+60], xmm4
                }
                */
                
                /*                
                __asm {
                    mov ebx, interpolation_fStackValue
                    movaps      xmm7, dwMaskPS_0111
                    movaps      xmm6, dwZeroPS
                    
                    //charge la pile courante avec un léger décalage                    
                    movups      xmm0, [ebx+ 4]
                    movups      xmm1, [ebx+20]
                    movups      xmm2, [ebx+36]
                    movups      xmm3, [ebx+52] //On déborde un peu, mais c'est pas grave car la pile fait plus de 16 points (pour la gestion en stéréo)
                    
                    //movsx       eax, word ptr [esi+eax*2]
                    //mov         dwInPlaySamplePositionPrevious, edi
                    
                    //cvtsi2ss    xmm6, eax
                    movss       xmm6, fProcessValue
                    
                    //Ecriture de la pile
                    movaps      [ebx+ 0], xmm0
                    movaps      [ebx+16], xmm1
                    //mulss       xmm6, fGlobalGain16                     //Aplique le volume ici... on économise ainsi un peu de ressource...
                    movaps      [ebx+32], xmm2
                    
                    andps       xmm3, xmm7
                    shufps      xmm6, xmm6, 00010101b
                    orps        xmm3, xmm6
                    movaps      [ebx+48], xmm3
                }
                */
                for (i=0;i<15;i++)
                {
                    interpolation_fStackValue[i   ] = interpolation_fStackValue[i+1];
                }
                interpolation_fStackValue[15] = fProcessValue[0];
                
                dwInPlaySamplePositionPrevious = dwInPlaySamplePosition;
            }

            //*******************************************************
            // Génération de la phase sur 10 bits
            dwTemp = (dwInPlayTickPosition>>14)&0x3FF;

            //*******************************************************
            // Application de l'interpolation
            fProcessValue[0] = 0;
            pfInterpolator = &samData_f32InterpolationData_1024_16[dwTemp<<4];
            for (i=0;i<16;i++)
            {
                fProcessValue[0] += interpolation_fStackValue[i] * pfInterpolator[i];
            }

/*
            __asm {
                    mov         ebx, interpolation_fStackValue
                    mov         edx, pfInterpolator
                    //mov         edx, dwTemp
                    //shl         edx, 4+4
                    
                    
                    //Lecture de la pile des données audio pour l'interpolation
                    movaps      xmm0, [ebx+ 0]
                    movaps      xmm1, [ebx+16]
                    movaps      xmm2, [ebx+32]
                    movaps      xmm3, [ebx+48]
                    
                    //Multiplication par l'interpolateur à phase variable
                    mulps       xmm0, [edx+ 0]
                    mulps       xmm1, [edx+16]
                    mulps       xmm2, [edx+32]
                    mulps       xmm3, [edx+48]
                    
                    //Addition verticale
                    addps       xmm0, xmm1                    
                    addps       xmm2, xmm3
                    addps       xmm0, xmm2
                    
                    //Addition horizontale
                    movhlps     xmm2, xmm0      //xmm2 = x x D C    xmm0 = D C B A
                    addps       xmm0, xmm2      //xmm0 = x x D+B C+A
                    movaps      xmm2, xmm0      
                    shufps      xmm2, xmm2, 0001b
                    addss       xmm0, xmm2
                    
                    movss       fProcessValue, xmm0
                    
                    
            }
*/            

            //*******************************************************
            //Application du IIR pour les effets
            fFX_IIR_LowPassValue += (fProcessValue[0]-fFX_IIR_LowPassValue) * fFX_IIR_LowPassRatio;

            //*******************************************************
            //Application du gain final
            fProcessValue[0] = fFX_IIR_LowPassValue * fGlobalGain;
            
            //*******************************************************
            //On stocke dans la pile
            stack_dwBufferStackPosition = (stack_dwBufferStackPosition+1)&sam_VOICEBUFFERSTACK_MASK;
            stack_fBufferStackValue[stack_dwBufferStackPosition] = fProcessValue[0];

            //On envoie sur la sortie avec le générateur de rendu
            for (dwRenderEntryIndex=0;dwRenderEntryIndex<render_dwEntryCount;dwRenderEntryIndex+=2)
            {
                pfBufferOut[0] +=
                    stack_fBufferStackValue[(stack_dwBufferStackPosition-render_dwDelayIndex[dwRenderEntryIndex])&sam_VOICEBUFFERSTACK_MASK] *
                    render_fDelayGain[dwRenderEntryIndex];

                pfBufferOut[1] +=
                    stack_fBufferStackValue[(stack_dwBufferStackPosition-render_dwDelayIndex[dwRenderEntryIndex+1])&sam_VOICEBUFFERSTACK_MASK] *
                    render_fDelayGain[dwRenderEntryIndex+1];

            }
            
            
            //On déplace le pointeur d'échantillon
            dwInPlayTickPosition    += dwInPlayTickIncrement;
            dwTemp                  = (dwInPlayTickPosition>>24)&0xFF;
            dwInPlaySamplePosition  += dwTemp;
            dwInPlayGranulePosition += dwTemp;
            dwInPlayTickPosition    = (dwInPlayTickPosition&0xFFFFFF);
            
            
            if (bLoop)
            {
                //Devons-nous boucler ?
                if (dwInPlaySamplePosition>psamSFX->dwLoopEndPositionSample)
                {
                    dwTemp                  = dwInPlaySamplePosition - psamSFX->dwLoopEndPositionSample - 1;
                    dwInPlaySamplePosition  = psamSFX->dwLoopBeginPositionSample + dwTemp;
                    dwInPlayGranulePosition = psamSFX->dwLoopBeginGranulePositionSample + dwTemp;
                    dwCurrentGranuleID      = psamSFX->dwLoopBeginGranuleID;
                    
                    //Mise à jour des pointeurs de données sources
                    SAM_GranulesGetData ( dwCurrentGranuleID, &pAudioDataD32 );
                    //pAudioDataXD4 = (DWORD *)pAudioDataB8;
                }
            }
            else
            {
                //Avons-nous atteint la fin de cet échantillon ?
                if (dwInPlaySamplePosition>=dwSamplesCount)
                {
                    //Fin de la lecture
                    dwInPlaySamplePosition      = 0;
                    dwInPlayGranulePosition     = 0; //dwInPlaySamplePosition;
                    psamVoice->bIsPlay          = 0;
                    //psamVoice->bIsUsed        = 0;
                    
                    //Retourne au premier granule
                    dwCurrentGranuleID          = psamSFX->dwGranuleFirst;
                    dwOutputCount               = 1;
                    break;
                
                
                }
            }
            

            //Avons-nous atteint la fin de ce granule ?
            if (dwInPlayGranulePosition>=dwSamplesPerGranuleCount)
            {
                //On va au début du granule suivant
                dwInPlayGranulePosition -= dwSamplesPerGranuleCount;
                
                //Cherche le granule suivant
                SAM_GranulesGetNext ( dwCurrentGranuleID, &dwCurrentGranuleID );

                //Mise à jour des pointeurs de données sources
                SAM_GranulesGetData ( dwCurrentGranuleID, &pAudioDataD32 );
                //pAudioDataF32           = (float *)pAudioDataB8;
                //pAudioDataXD4           = (DWORD *)pAudioDataB8;
            }

            pfBufferOut += dwSoftwareBufferChannelCount;
        } while (--dwOutputCount);
        
        __asm {
            push eax
            push edx
            rdtsc
            mov dword ptr qwTimeStampB, eax
            mov dword ptr qwTimeStampB+4, edx
            pop edx
            pop eax
        }
        qwTimeTotal += (qwTimeStampB - qwTimeStampA);
        
        samData.fCyclesPerSample = 0.1F * (float)(qwTimeTotal*10 / qwSamplesCount);
        
    }

    //Ecriture des données en R/W
    psamVoice->dwInPlaySamplePosition           = dwInPlaySamplePosition;
    psamVoice->dwInPlaySamplePositionPrevious   = dwInPlaySamplePositionPrevious;
    psamVoice->dwInPlayTickPosition             = dwInPlayTickPosition;
    psamVoice->dwInPlayGranulePosition          = dwInPlayGranulePosition;
    psamVoice->stack_dwBufferStackPosition      = stack_dwBufferStackPosition;
    psamVoice->dwCurrentGranuleID               = dwCurrentGranuleID;
    psamVoice->fFX_IIR_LowPassValue             = fFX_IIR_LowPassValue;
}



