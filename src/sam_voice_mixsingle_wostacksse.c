#include "sam_header.h"
#include "sam_data.h"
#include "sam_voice.h"

extern FLOAT32     XD4_fExposant[16];
extern FLOAT32     XD4_fExposantDiv63Sign[32];
extern FLOAT32     XD4_fMantisse[128];
extern DWORD       XD4_dwShiftRight[4];
extern BYTE       XD4_bShiftRight[4];

extern DWORD       XD4_dwShiftRightFast[4];
extern FLOAT32     XD4_fExposantMantisse[2048];
extern FLOAT32     f32LPCM_Decode16;

void SAM_VOICE_MixSingleGranule_WithoutStackSSE ( SAM_VOICE * psamVoice, SAM_SFX * psamSFX, float fGlobalGain, DWORD dwLoopCount, float * pfSoftwareBuffer, DWORD dwSoftwareBufferChannelCount )
{
    float       resample_fLastValue[2];
    float       resample_fCurrValue[2];
    float       *resample_fFIRStackValue;
    long        resample_lFIRStackPosition;
    long        resample_lFIRLength;
    float       *resample_pfFIRCoef;

    float       fProcessValue[2];
    DWORD       dwInPlaySamplePosition;
    DWORD       dwInPlaySamplePositionPrevious;
    DWORD       dwInPlayTickPosition;
    DWORD       dwInPlayTickIncrement;
    DWORD       dwInPlayGranulePosition;

    DWORD       dwSamplesCount;


    float       *stack_fBufferStackValue;
    DWORD       stack_dwBufferStackPosition;

    BYTE        bFormat;
    BYTE        bLoop;
    DWORD       *pAudioDataD32;
    //BYTE        *pAudioDataB8;
    //float       *pAudioDataF32;
    //DWORD       *pAudioDataXD4;
    //QWORD       *pAudioDataQ64;

    DWORD       dwTemp;
    float       *pfSineInterpolationA;
    float       *pfSineInterpolationB;
    float       *pfSineInterpolationPackedBy2;

    DWORD       dwCurrentGranuleID;
    DWORD       dwBytesPerSampleCount;
    DWORD       dwSamplesPerGranuleCount;

    float       *pfBufferOut;

    float       fFX_IIR_LowPassRatio;
    float       fFX_IIR_LowPassValue;

    float       fResamplePreTrebleBoost;

    long        i, j;

    DWORD       render_dwEntryCount;           // 0...32
    DWORD       *render_dwDelayIndex;       // [32] Index in the delay line
    DWORD       *render_dwChannelIndex;     // [32] Channel index
    float       *render_fDelayGain;         // [32] Gain
    DWORD       dwRenderEntryIndex;
    
    DWORD       dwLoopEndPositionSample;

    void        *pCodecData;

    DWORD       dwOutputCount;
    
    QWORD       qwTimeStampA;
    QWORD       qwTimeStampB;
    static QWORD        qwTimeTotal = 0;
    static QWORD        qwSamplesCount = 0;
    
    DWORD       dwSoftwareBufferChannelCount4;
    
    float fTemp;
    float fGlobalGain16;
    
    //Lecture des données en R/O
    fGlobalGain16                               = fGlobalGain * f32LPCM_Decode16;
    dwSoftwareBufferChannelCount4               = dwSoftwareBufferChannelCount<<2;
    dwLoopEndPositionSample                     = psamSFX->dwLoopEndPositionSample;
    bFormat                                     = psamSFX->bFormat;
    bLoop                                       = (unsigned char)psamVoice->bLoopMode;
    stack_fBufferStackValue                     = &(psamVoice->stack_fBufferStackValue[0]);
    dwInPlayTickIncrement                       = psamVoice->dwInPlayTickIncrement;
    dwSamplesCount                              = psamSFX->dwSamplesCount;
    pfBufferOut                                 = pfSoftwareBuffer;
    resample_fFIRStackValue                     = psamVoice->resample_fFIRStackValue;
    dwAlignOn16 ( (DWORD *)&resample_fFIRStackValue );
    resample_lFIRLength                         = psamVoice->resample_lFIRLength;
    resample_pfFIRCoef                          = psamVoice->resample_pfFIRCoef;
    fFX_IIR_LowPassRatio                        = psamVoice->fFX_IIR_LowPassRatio;
    render_dwEntryCount                         = psamVoice->psamRender->dwEntryCount;
    //render_dwDelayIndex                         = psamVoice->psamRender->dwDelayIndex;
    //render_dwChannelIndex                       = psamVoice->psamRender->dwChannelIndex;
    //render_fDelayGain                           = psamVoice->psamRender->fDelayGain;
    if (render_dwEntryCount<=8)
    {
        render_dwDelayIndex                         = NULL; //psamVoice->psamRender8->dwDelayIndex;
        render_dwChannelIndex                       = NULL; //psamVoice->psamRender8->dwChannelIndex;
        render_fDelayGain                           = psamVoice->psamRender8->fDelayGain;
    }
    else
    {
        render_dwDelayIndex                         = psamVoice->psamRender->dwDelayIndex;
        render_dwChannelIndex                       = psamVoice->psamRender->dwChannelIndex;
        render_fDelayGain                           = psamVoice->psamRender->fDelayGain;
    }
    pCodecData                                  = psamVoice->dwCodecData;

    //Les données pour l'interpolation
    pfSineInterpolationA                        = samData.pfInterpolationTable512;
    pfSineInterpolationB                        = pfSineInterpolationA+256;
    fResamplePreTrebleBoost                     = samData.fResamplePreTrebleBoost;
    pfSineInterpolationPackedBy2                = samData.pfInterpolationTable512PackedBy2;

    //Nombre de samples par granule
    SAM_FormatGetBytesCount ( bFormat, &dwBytesPerSampleCount );
    dwSamplesPerGranuleCount                    = sam_GRANULE_BUFFERBYTES / dwBytesPerSampleCount;

    //Lecture des données en R/W
    resample_fCurrValue[0]                      = psamVoice->resample_fCurrValue[0];
    resample_fCurrValue[1]                      = psamVoice->resample_fCurrValue[1];
    resample_fLastValue[0]                      = psamVoice->resample_fLastValue[0];
    resample_fLastValue[1]                      = psamVoice->resample_fLastValue[1];
    dwInPlaySamplePosition                      = psamVoice->dwInPlaySamplePosition;
    dwInPlaySamplePositionPrevious              = psamVoice->dwInPlaySamplePositionPrevious;
    dwInPlayTickPosition                        = psamVoice->dwInPlayTickPosition;
    dwInPlayGranulePosition                     = psamVoice->dwInPlayGranulePosition;
    stack_dwBufferStackPosition                 = psamVoice->stack_dwBufferStackPosition;
    dwCurrentGranuleID                          = psamVoice->dwCurrentGranuleID;
    resample_lFIRStackPosition                  = psamVoice->resample_lFIRStackPosition;
    fFX_IIR_LowPassValue                        = psamVoice->fFX_IIR_LowPassValue;

    //Les données du granule en cours...
    SAM_GranulesGetData ( dwCurrentGranuleID, &pAudioDataD32 );
    //pAudioDataF32 = (float *)pAudioDataB8;
    //pAudioDataXD4 = (DWORD *)pAudioDataB8;
    //pAudioDataQ64 = (QWORD *)pAudioDataB8;
    dwOutputCount = dwLoopCount;
    

    if (bFormat&sam_FORMAT_CHANNELMASK)
    {
        //Traitement avec un son stéréo : Resampling + Volume + Mixage Stéréo
        do {
            //*******************************************************
            //Devons-nous charger une nouvelle donnée ?
            if (dwInPlaySamplePosition!=dwInPlaySamplePositionPrevious)
            {
                resample_fLastValue[0] = resample_fCurrValue[0];
                resample_fLastValue[1] = resample_fCurrValue[1];

                //dwTemp = dwInPlayGranulePosition&3;
                //resample_fCurrValue[0] = SAM_XD4_DecodeValue ( (DWORD)(pAudioDataQ64[dwInPlayGranulePosition>>2]&0xFFFFFFFF), dwTemp );
                //resample_fCurrValue[1] = SAM_XD4_DecodeValue ( (DWORD)(pAudioDataQ64[dwInPlayGranulePosition>>2]>>32), dwTemp );
                resample_fCurrValue[0] = SAM_LPCM_DecodeValue16 ( pAudioDataD32[dwInPlayGranulePosition], 0 );
                resample_fCurrValue[1] = SAM_LPCM_DecodeValue16 ( pAudioDataD32[dwInPlayGranulePosition], 1 );

                //Augmente les hautes fréquences (trés utile pour le FIR-16) mais ajoute de la phase
                resample_fCurrValue[0] += (resample_fCurrValue[0]-resample_fLastValue[0])*fResamplePreTrebleBoost;
                resample_fCurrValue[1] += (resample_fCurrValue[1]-resample_fLastValue[1])*fResamplePreTrebleBoost;

                dwInPlaySamplePositionPrevious = dwInPlaySamplePosition;
            }

            //*******************************************************
            // Application de l'interpolation de resampling
            dwTemp = (dwInPlayTickPosition>>16)&0xFF;
            fProcessValue[0] = ( resample_fLastValue[0] * pfSineInterpolationA[dwTemp] ) + ( resample_fCurrValue[0] * pfSineInterpolationB[dwTemp] );
            fProcessValue[1] = ( resample_fLastValue[1] * pfSineInterpolationA[dwTemp] ) + ( resample_fCurrValue[1] * pfSineInterpolationB[dwTemp] );

            //*******************************************************
            // Application du FIR de resampling
            resample_fFIRStackValue[resample_lFIRStackPosition   ] = fProcessValue[0];
            resample_fFIRStackValue[resample_lFIRStackPosition+16] = fProcessValue[1];
            fProcessValue[0] = 0;
            fProcessValue[1] = 0;
            j = resample_lFIRLength - resample_lFIRStackPosition;
            for (i=0;i<resample_lFIRLength;i++,j++)
            {
                fProcessValue[0] += resample_fFIRStackValue[i   ] * resample_pfFIRCoef[j];
                fProcessValue[1] += resample_fFIRStackValue[i+16] * resample_pfFIRCoef[j];
            }
            resample_lFIRStackPosition = (resample_lFIRStackPosition+1)&(resample_lFIRLength-1);

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
        //Traitement avec son mono : Resampling + IIR + Volume + Rendering
        __asm {
                    pushad
                    
                    rdtsc
                    mov dword ptr qwTimeStampA, eax
                    mov dword ptr qwTimeStampA+4, edx
                    
                    
                    movss   xmm6, resample_fCurrValue
                    movss   xmm7, resample_fLastValue
                    
                _loop_eachsample:    
            
            
                    //qwSamplesCount++;
                    mov eax, dword ptr qwSamplesCount
                    mov edx, dword ptr qwSamplesCount+4
            
                    add eax, 1
                    adc edx, 0
            
                    mov dword ptr qwSamplesCount, eax
                    mov dword ptr qwSamplesCount+4, edx
            
                    
                    /*
                    //*******************************************************
                    //Devons-nous charger une nouvelle donnée ?
                    if (dwInPlaySamplePosition!=dwInPlaySamplePositionPrevious)
                    {
                        dwInPlaySamplePositionPrevious = dwInPlaySamplePosition;
                        
                        resample_fLastValue[0] = resample_fCurrValue[0];
                        resample_fCurrValue[0] = SAM_XD4_DecodeValue ( pAudioDataXD4[dwInPlayGranulePosition>>2], (dwInPlayGranulePosition&3) );
                        
                        //Augmente les hautes fréquences (trés utile pour le FIR-16) mais ajoute de la phase
                        resample_fCurrValue[0] += (resample_fCurrValue[0]-resample_fLastValue[0])*fResamplePreTrebleBoost;


                        
                    }
                    */    
                    mov         edi, dwInPlaySamplePosition
                    cmp         edi, dwInPlaySamplePositionPrevious
                    je          _no_new_data
                    
                    mov         ecx, dwInPlayGranulePosition
                    mov         dwInPlaySamplePositionPrevious, edi
                    
                    mov         eax, ecx
                    mov         esi, pAudioDataD32
                    
                    shr         eax, 1
                    and         ecx, 1
                    
                    shl         ecx, 4                    
                    mov         eax, [esi+eax*4]
                    
                    shr         eax, cl
                    movsx       edx, ax
                    
                    movss       xmm7, xmm6
                    movss       xmm2, fGlobalGain16               //Aplique le volume ici... on économise ainsi un peu de ressource...
                    
                    cvtsi2ss    xmm6, edx
                    mulss       xmm6, xmm2

                _no_new_data:
            
                    /*
                    //*******************************************************
                    // Application de l'interpolation de resampling
                    dwTemp = (dwInPlayTickPosition>>16)&0xFF;
                    fProcessValue[0] = ( resample_fLastValue[0] * pfSineInterpolationA[dwTemp] ) + ( resample_fCurrValue[0] * pfSineInterpolationB[dwTemp] );
                    
                    dwTemp = (dwInPlayTickPosition>>15)&0x1FE);
                    fProcessValue[0] = ( resample_fLastValue[0] * pfSineInterpolationPackedBy2[dwTemp] ) + ( resample_fCurrValue[0] * pfSineInterpolationPackedBy2[dwTemp+1] );
                    
                    
                    IN  : XMM6 = resample_fCurrValue
                          XMM7 = resample_fLastValue
                    OUT : XMM0 = fProcessValue[0]
                    TS  : 10 cycles

                    //*******************************************************
                    // Application du FIR de resampling
                    /*resample_fFIRStackValue[resample_lFIRStackPosition] = fProcessValue[0];
                    fProcessValue[0] = 0;
                    j = resample_lFIRLength - resample_lFIRStackPosition;
                    for (i=0;i<resample_lFIRLength;i++,j++)
                        fProcessValue[0] += resample_fFIRStackValue[i] * resample_pfFIRCoef[j];
                    resample_lFIRStackPosition = (resample_lFIRStackPosition+1)&(resample_lFIRLength-1);
                    
                    IN  : XMM0 = fProcessValue[0]
                    OUT :
                    TS  : 40 cycles
                    */
                    
                    mov         edi, dwInPlayTickPosition
                    mov         edx, pfSineInterpolationPackedBy2
                    
                    shr         edi, 13
                    mov         eax, resample_lFIRStackPosition
                    
                    and         edi, 0xFF8
                    mov         ebx, 15
                    
                    mov         ecx, eax
                    mov         esi, resample_fFIRStackValue                //La pile du FIR
                    
                    inc         ecx
                    add         edx, edi
                    
                    sub         ebx, eax
                    and         ecx, 15
                    
                    movss       xmm0, [edx]             // xmm0 = InterpolationA
                    movss       xmm1, [edx+4]           // xmm1 = InterpolationB
                    
                    mulss       xmm0, xmm7
                    mov         edi, resample_pfFIRCoef
                    
                    mulss       xmm1, xmm6
                    mov         resample_lFIRStackPosition, ecx
                    
                    addss       xmm0, xmm1
                    lea         edi, [edi+ebx*4]
                    
                    movss       [esi+eax*4], xmm0
                    
                    movups      xmm0, [edi+ 0]
                    movups      xmm1, [edi+16]
                    movups      xmm2, [edi+32]
                    movups      xmm3, [edi+48]
                    mulps       xmm0, [esi+ 0]
                    mulps       xmm1, [esi+16]
                    mulps       xmm2, [esi+32]
                    mulps       xmm3, [esi+48]
                    
                    movss       xmm4, fFX_IIR_LowPassValue
                    
                    //Additionne le tout (addition verticale)
                    addps       xmm0, xmm1
                    addps       xmm2, xmm3
                    addps       xmm0, xmm2

                    //Additionne le reste (addition horizontale en SSE)                    
                    movhlps     xmm2, xmm0      //xmm2 = x x D C    xmm0 = D C B A
                    addps       xmm0, xmm2      //xmm0 = x x D+B C+A
                    movaps      xmm2, xmm0      
                    shufps      xmm2, xmm2, 0001b
                    addps       xmm0, xmm2
                    
                    
                    

                    
                    /*
                    pshufd      xmm1, xmm0, 1110b //SSE2
                    addps       xmm0, xmm1
                    pshufd      xmm1, xmm0, 0001b
                    addss       xmm0, xmm1
                    */
                    //en SSE3 on fait simplement "haddps xmm0, xmm0" deux fois de suite !
                    //pxor xmm0, xmm0
                    
                    
                    /*
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
                    
                    IN  : XMM0 = fProcessValue[0]
                    OUT : XMM4 = OutputData
                    TS  : 20 cycles
                    */
                    
                    
                    subss       xmm0, xmm4
                    mulss       xmm0, fFX_IIR_LowPassRatio
            
                    //On envoie sur la sortie avec le générateur de rendu
                    /*
                    for (dwRenderEntryIndex=0;dwRenderEntryIndex<render_dwEntryCount;dwRenderEntryIndex++)
                    {
                        pfBufferOut[render_dwChannelIndex[dwRenderEntryIndex]] +=
                            xmm4 * render_fDelayGain[dwRenderEntryIndex];
                    }
                    
                    TS  : 60 cycles en mode 5.1 (uniquement sur le panneau initial)
                    */
                    
                    mov         edi, pfBufferOut
                    mov         esi, render_fDelayGain

                    addss       xmm4, xmm0
                    
                    unpcklps    xmm4, xmm4                  // - | - | s | s
                    shufps      xmm4, xmm4, 01000100b       // s | s | s | s
                    
                    movss       fFX_IIR_LowPassValue, xmm4
                    
                    movaps      xmm0, [esi]                 //Lecture du niveau de gain Ch0 à Ch3
                    movaps      xmm1, [esi+16]              //Lecture du niveau de gain Ch4 à Ch7
                    
                    mulps       xmm0, xmm4
                    movups      xmm2, [edi]                 //Lecture de la sortie Ch0 à Ch3
                    mulps       xmm1, xmm4
                    movlps      xmm3, [edi+16]              //Lecture de la sortie Ch4 à Ch5
                    
                    addps       xmm2, xmm0
                    addps       xmm3, xmm1
                    
                    movups      [edi], xmm2
                    movlps      [edi+16], xmm3
                    
                    
                    
                    
                    
                    /*
                    //On déplace le pointeur d'échantillon
                    dwInPlayTickPosition    += dwInPlayTickIncrement;
                    dwTemp                  = (dwInPlayTickPosition>>24)&0xFF;
                    dwInPlaySamplePosition  += dwTemp;
                    dwInPlayGranulePosition += dwTemp;
                    dwInPlayTickPosition    = (dwInPlayTickPosition&0xFFFFFF);
                    */
                    
                                        
                    mov         eax, dwInPlayTickPosition
                    mov         ebx, dwInPlaySamplePosition
                    
                    add         eax, dwInPlayTickIncrement
                    mov         ecx, dwInPlayGranulePosition
                    
                    mov         esi, eax
                    
                    shr         esi, 24
                    and         eax, 0xFFFFFF
                    
                    and         esi, 0xFF
                    mov         dwInPlayTickPosition, eax
                    
                    add         ebx, esi
                    add         ecx, esi
                    
                    mov         dwInPlaySamplePosition, ebx
                    mov         dwInPlayGranulePosition, ecx
                    
                    //***********************************************
                    //Gestion du bouclage / fin de l'échantillon
                    /*
                    if (bLoop)
                    {
                        //Devons-nous boucler ?
                        if (dwInPlaySamplePosition>dwLoopEndPositionSample)
                        {
                            dwTemp                  = dwInPlaySamplePosition - psamSFX->dwLoopEndPositionSample - 1;
                            dwInPlaySamplePosition  = psamSFX->dwLoopBeginPositionSample + dwTemp;
                            dwInPlayGranulePosition = psamSFX->dwLoopBeginGranulePositionSample + dwTemp;
                            dwCurrentGranuleID      = psamSFX->dwLoopBeginGranuleID;
                            
                            //Mise à jour des pointeurs de données sources
                            SAM_GranulesGetData ( dwCurrentGranuleID, &pAudioDataB8 );
                            pAudioDataXD4 = (DWORD *)pAudioDataB8;
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
                    */
                    
                    cmp         bLoop, 0
                    je          _no_testloop
                    
                    mov         eax, dwLoopEndPositionSample
                    cmp         dwInPlaySamplePosition, eax
                    jbe         _endtestloop
                    
                    movss       resample_fCurrValue, xmm6 
                    movss       resample_fLastValue, xmm7

                    popad
            }
                    dwTemp                  = dwInPlaySamplePosition - psamSFX->dwLoopEndPositionSample - 1;
                    dwInPlaySamplePosition  = psamSFX->dwLoopBeginPositionSample + dwTemp;
                    dwInPlayGranulePosition = psamSFX->dwLoopBeginGranulePositionSample + dwTemp;
                    dwCurrentGranuleID      = psamSFX->dwLoopBeginGranuleID;
                    
                    //Mise à jour des pointeurs de données sources
                    SAM_GranulesGetData ( dwCurrentGranuleID, &pAudioDataD32 );
                    
            __asm {        
                    pushad
                    movss       xmm6, resample_fCurrValue
                    movss       xmm7, resample_fLastValue
                    

                    jmp         _endtestloop
                _no_testloop:
                    
                    mov         eax, dwSamplesCount
                    cmp         dwInPlaySamplePosition, eax
                    jb          _endtestloop
                    
                    xor         eax, eax
                    mov         edi, psamVoice
                    mov         esi, psamSFX
                    
                    mov         dwInPlaySamplePosition, eax
                    mov         dwInPlayGranulePosition, eax
                    mov         dword ptr [edi+SAM_VOICE.bIsPlay], eax
                    mov         ebx, [esi+SAM_SFX.dwGranuleFirst]
                    mov         dwCurrentGranuleID, ebx
                    mov         dwOutputCount, 1
                    jmp         _endloop_eachsample
                    
                _endtestloop:
                    /*
                    //Avons-nous atteint la fin de ce granule ?
                    if (dwInPlayGranulePosition>=dwSamplesPerGranuleCount)
                    {
                        //On va au début du granule suivant
                        dwInPlayGranulePosition -= dwSamplesPerGranuleCount;
                        
                        //Cherche le granule suivant
                        SAM_GranulesGetNext ( dwCurrentGranuleID, &dwCurrentGranuleID );

                        //Mise à jour des pointeurs de données sources
                        SAM_GranulesGetData ( dwCurrentGranuleID, &pAudioDataB8 );
                        //pAudioDataF32           = (float *)pAudioDataB8;
                        pAudioDataXD4           = (DWORD *)pAudioDataB8;
                    }
                    */
                    mov         eax, dwSamplesPerGranuleCount
                    cmp         dwInPlayGranulePosition, eax
                    jb          _endofgranule
                    movss       resample_fCurrValue, xmm6 
                    movss       resample_fLastValue, xmm7
                    popad
        }
        
                        //On va au début du granule suivant
                        dwInPlayGranulePosition -= dwSamplesPerGranuleCount;
                        
                        //Cherche le granule suivant
                        SAM_GranulesGetNext ( dwCurrentGranuleID, &dwCurrentGranuleID );

                        //Mise à jour des pointeurs de données sources
                        SAM_GranulesGetData ( dwCurrentGranuleID, &pAudioDataD32 );
                        
        __asm {            
                    pushad
                    movss       xmm6, resample_fCurrValue
                    movss       xmm7, resample_fLastValue
                _endofgranule:    

                    //pfBufferOut += dwSoftwareBufferChannelCount;
                    // while (--dwOutputCount);
                    mov         esi, pfBufferOut
                    mov         ecx, dwOutputCount
                    
                    add         esi, dwSoftwareBufferChannelCount4
                    dec         ecx
                    
                    
                    mov         pfBufferOut, esi
                    mov         dwOutputCount, ecx
                    jnz         _loop_eachsample
                    
                _endloop_eachsample:
                
                    movss   resample_fCurrValue, xmm6 
                    movss   resample_fLastValue, xmm7
                
                    
            rdtsc
            mov dword ptr qwTimeStampB, eax
            mov dword ptr qwTimeStampB+4, edx
            
            popad
        }
        
        
        qwTimeTotal += (qwTimeStampB - qwTimeStampA);
        
        samData.fCyclesPerSample = 0.1F * (float)(qwTimeTotal*10 / qwSamplesCount);
        
    }

    //Ecriture des données en R/W
    psamVoice->resample_fLastValue[0]           = resample_fLastValue[0];
    psamVoice->resample_fLastValue[1]           = resample_fLastValue[1];
    psamVoice->resample_fCurrValue[0]           = resample_fCurrValue[0];
    psamVoice->resample_fCurrValue[1]           = resample_fCurrValue[1];
    psamVoice->dwInPlaySamplePosition           = dwInPlaySamplePosition;
    psamVoice->dwInPlaySamplePositionPrevious   = dwInPlaySamplePositionPrevious;
    psamVoice->dwInPlayTickPosition             = dwInPlayTickPosition;
    psamVoice->dwInPlayGranulePosition          = dwInPlayGranulePosition;
    psamVoice->stack_dwBufferStackPosition      = stack_dwBufferStackPosition;
    psamVoice->dwCurrentGranuleID               = dwCurrentGranuleID;
    psamVoice->resample_lFIRStackPosition       = resample_lFIRStackPosition;
    psamVoice->fFX_IIR_LowPassValue             = fFX_IIR_LowPassValue;
}
