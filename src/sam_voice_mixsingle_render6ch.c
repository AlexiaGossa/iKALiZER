#include "sam_header.h"
#include "sam_data.h"
#include "sam_voice.h"

void SAM_VOICE_MixSingle_Render6ch ( SAM_VOICE * psamVoice, SAM_SFX * psamSFX, float fGlobalGain, DWORD dwLoopCount, float * pfSoftwareBuffer, DWORD dwSoftwareBufferChannelCount, DWORD dwParam )
{
    float       fGlobalGain16;
    float       *interpolation_fStackValue;
    
    DWORD       dwOutputCount;

    DWORD       dwInPlaySamplePosition;
    DWORD       dwInPlaySamplePositionPrevious;
    DWORD       dwInPlayTickPosition;
    DWORD       dwInPlayTickIncrement;
    DWORD       dwInPlayGranulePosition;
    DWORD       dwDetectEndOfSamples;

    BYTE        bFormat;
    BYTE        bLoop;
    DWORD       *pAudioDataD32;

    DWORD       dwCurrentGranuleID;
    DWORD       dwBytesPerSampleCount;
    DWORD       dwSamplesPerGranuleCount;

    float       *pfBufferOut;

    float       fFX_IIR_LowPassRatio;
    float       fFX_IIR_LowPassValue;

    DWORD       render_dwEntryCountShl3;
    DWORD       render_dwEntryCount;           // 0...32
    long        lRenderPositionCurrent_F16;
    long        lRenderIncrement_F16;
    
    SAM_RENDER254  *pRender;
    
    
            DWORD       dwLoopEndPositionSample;

    
            QWORD       qwTimeStampA;
            QWORD       qwTimeStampB;
    static  QWORD       qwTimeTotal = 0;
    static  QWORD       qwSamplesCount = 0;
            DWORD       dwSamplesFastCount;
    
            DWORD       dwSoftwareBufferChannelCount4;

    if ( (psamVoice->dwInPlayTickIncrement>0x00FAE000)&&
         (psamVoice->dwInPlayTickIncrement<0x01052000)&&
         (psamVoice->bVariableRate==0) )
    {
        SAM_VOICE_MixSingle_Render6ch_noresamp (
            psamVoice, 
            psamSFX, 
            fGlobalGain,
            dwLoopCount, 
            pfSoftwareBuffer, 
            dwSoftwareBufferChannelCount, 
            dwParam );
        return ;
    }
    
    //Lecture des données en R/O
    fGlobalGain16                               = fGlobalGain * f32LPCM_Decode16;
    dwSoftwareBufferChannelCount4               = dwSoftwareBufferChannelCount<<2;
    bFormat                                     = psamSFX->bFormat;
    bLoop                                       = (unsigned char)psamVoice->bLoopMode;
    dwInPlayTickIncrement                       = psamVoice->dwInPlayTickIncrement;
    dwLoopEndPositionSample                     = psamSFX->dwLoopEndPositionSample;
    dwDetectEndOfSamples                        = (bLoop)?(psamSFX->dwLoopEndPositionSample-1):(psamSFX->dwSamplesCount);
    pfBufferOut                                 = pfSoftwareBuffer;
    fFX_IIR_LowPassRatio                        = psamVoice->fFX_IIR_LowPassRatio;
    render_dwEntryCount                         = psamVoice->psamRender254->dwEntryCount>>1;
    render_dwEntryCountShl3                     = render_dwEntryCount<<3;
    pRender                                     = &samData.psamRender254Table[ ((psamVoice->lRenderPositionCurrent_F16)>>16) ];
    interpolation_fStackValue                   = psamVoice->interpolation_fStackValue;
    lRenderIncrement_F16                        = psamVoice->lRenderIncrement_F16;

    //Nombre de samples par granule
    SAM_FormatGetBytesCount ( bFormat, &dwBytesPerSampleCount );
    dwSamplesPerGranuleCount                    = sam_GRANULE_BUFFERBYTES / dwBytesPerSampleCount;

    //Lecture des données en R/W
    dwInPlaySamplePosition                      = psamVoice->dwInPlaySamplePosition;
    dwInPlaySamplePositionPrevious              = psamVoice->dwInPlaySamplePositionPrevious;
    dwInPlayTickPosition                        = psamVoice->dwInPlayTickPosition;
    dwInPlayGranulePosition                     = psamVoice->dwInPlayGranulePosition;
    dwCurrentGranuleID                          = psamVoice->dwCurrentGranuleID;
    fFX_IIR_LowPassValue                        = psamVoice->fFX_IIR_LowPassValue;
    lRenderPositionCurrent_F16                  = psamVoice->lRenderPositionCurrent_F16;

    //Les données du granule en cours...
    SAM_GranulesGetData ( dwCurrentGranuleID, &pAudioDataD32 );
    
    //Le compteur
    dwOutputCount = dwLoopCount<<4;

    
    //Traitement avec son mono : Resampling + IIR + Volume + Rendering
        
    __asm {
                //Précharge la pile
                mov         ebx, interpolation_fStackValue
                prefetcht0  [ebx]
                prefetcht0  [ebx+32]
                
                //Précharge le masque
                movaps      xmm7, sam_voice_mixsingle_dwMaskPS_0111
                xorps       xmm6, xmm6
                
                //Démarre le chrono                    
                rdtsc
                mov         dword ptr qwTimeStampA, eax
                mov         dword ptr qwTimeStampA+4, edx
                
                mov         dwSamplesFastCount, 0
                
                jmp         _loop_eachsample
                
                //.align 16, 0x90
                align 16
            _loop_eachsample:    
                mov         ebx, interpolation_fStackValue
                mov         edx, dwInPlayTickPosition
                
                /*
                //*******************************************************
                //Devons-nous charger une nouvelle donnée ?
                if (dwInPlaySamplePosition!=dwInPlaySamplePositionPrevious)
                {
                    dwInPlaySamplePositionPrevious = dwInPlaySamplePosition;
                    
                    SAM_XD4_DecodeValue ( pAudioDataXD4[dwInPlayGranulePosition>>2], (dwInPlayGranulePosition&3) );
                }
                */    
                mov         edi, dwInPlaySamplePosition
                inc         dwSamplesFastCount
                
                cmp         edi, dwInPlaySamplePositionPrevious
                je          _no_new_data
                
                mov         eax, dwInPlayGranulePosition
                mov         esi, pAudioDataD32
                
                // eax = dwInPlayGranulePosition
                // ebx = interpolation_fStackValue
                // edx = dwInPlayTickPosition
                // esi = pAudioDataD32
                // edi = dwInPlaySamplePosition
                
                align 16
            _test_eachsample:    
                // Sommes-nous à la fin du son ?
                cmp         edi, dwDetectEndOfSamples
                jb          _no_end_of_sound
                
                
                //############################################################################################
                //Traitement pour la fin du son
                
                //Sommes-nous dans une boucle ?
                cmp         bLoop, 1
                je          _end_of_sound_yesloop
                

                //############################################################################################                    
                //Nous sommes à la fin définitive du son, tout est fini...
            _end_of_sound_noloop:

                xor         eax, eax
                mov         edi, psamVoice
                mov         esi, psamSFX
                
                mov         dwInPlaySamplePosition, eax
                mov         dwInPlayGranulePosition, eax
                mov         dword ptr [edi+SAM_VOICE.bIsPlay], eax
                mov         ebx, [esi+SAM_SFX.dwGranuleFirst]
                mov         dwCurrentGranuleID, ebx
                jmp         _endloop_eachsample

                //############################################################################################                    
                //Nous sommes à la fin du son, on boucle au début...
            _end_of_sound_yesloop:
            
                //dwTemp = dwInPlaySamplePosition - dwDetectEndOfSamples;
                mov         ecx, edi
                sub         ecx, dwDetectEndOfSamples
                
                mov         esi, psamSFX
                
                //dwInPlaySamplePosition  = psamSFX->dwLoopBeginPositionSample + dwTemp;
                mov         edi, [esi+SAM_SFX.dwLoopBeginPositionSample]
                add         edi, ecx
                mov         dwInPlaySamplePosition, edi
                
                //dwInPlayGranulePosition = psamSFX->dwLoopBeginGranulePositionSample + dwTemp;
                mov         eax, [esi+SAM_SFX.dwLoopBeginGranulePositionSample]
                add         eax, ecx
                mov         dwInPlayGranulePosition, eax
                
                //dwCurrentGranuleID      = psamSFX->dwLoopBeginGranuleID;
                mov         ecx, [esi+SAM_SFX.dwLoopBeginGranuleID]
                mov         dwCurrentGranuleID, ecx
                
                //Mise à jour des pointeurs de données sources
                //SAM_GranulesGetData ( dwCurrentGranuleID, &pAudioDataD32 );
                shl	        ecx, 12
                add         ecx, DWORD PTR samGranulesData.pGranule
                lea         esi, [ecx+SAM_GRANULE.bAudioData]
                mov         pAudioDataD32, esi
                
                // ***************************************************
                // Nous ne sommes pas à la fin du son
                align 16
            _no_end_of_sound:
            
                // Sommes-nous à la fin du granules
                cmp         eax, dwSamplesPerGranuleCount
                jb          _no_end_of_granule

                //############################################################################################                    
                //Traitement pour la fin du granule
                
                //On va au début du granule suivant
                //dwInPlayGranulePosition -= dwSamplesPerGranuleCount;
                sub         eax, dwSamplesPerGranuleCount
                mov         dwInPlayGranulePosition, eax
                
                //Cherche le granule suivant et Mise à jour des pointeurs de données sources
                //SAM_GranulesGetNext ( dwCurrentGranuleID, &dwCurrentGranuleID );
                //SAM_GranulesGetData ( dwCurrentGranuleID, &pAudioDataD32 );
	            mov	        ecx, dwCurrentGranuleID
	            shl	        ecx, 12
	            mov         esi, DWORD PTR samGranulesData.pGranule
	            mov         ecx, [ecx+esi+SAM_GRANULE.dwIdGranuleNext]
	            mov         dwCurrentGranuleID, ecx
	            shl         ecx, 12
	            add         esi, ecx
	            mov         pAudioDataD32, esi
	            
	            jmp         _no_end_of_granule

                // ***************************************************
                // Effectue le décalage de la pile et le chargement de la nouvelle donnée
                align 16
            _no_end_of_granule:
                //charge la pile courante avec un léger décalage                    
                movups      xmm0, [ebx+ 4]                          //6:1
                movups      xmm1, [ebx+20]                          //6:1
                movups      xmm2, [ebx+36]                          //6:1
                movups      xmm3, [ebx+52]                          //6:1       //On déborde un peu, mais c'est pas grave car la pile fait plus de 16 points (pour la gestion en stéréo)
                
                movsx       eax, word ptr [esi+eax*2]               //1:1
                mov         dwInPlaySamplePositionPrevious, edi     //1:1
                
                cvtsi2ss    xmm6, eax                               //12:2
                
                //Ecriture de la pile
                movaps      [ebx+ 0], xmm0                          //6:1
                
                mulss       xmm6, fGlobalGain16                     //7:2    Aplique le volume ici... on économise ainsi un peu de ressource...
                movaps      [ebx+16], xmm1                          //6:1
                andps       xmm3, xmm7                              //4:2
                shufps      xmm6, xmm6, 00010101b                   //6:2 
                movaps      [ebx+32], xmm2                          //6:1
                orps        xmm3, xmm6                              //4:2
                movaps      [ebx+48], xmm3                          //6:1
                
                
            _no_new_data:
            
                /*
                    //*******************************************************
                    // Génération de la phase sur 10 bits
                    dwTemp = (dwInPlayTickPosition>>14)&0x3FF;

                    //*******************************************************
                    // Application de l'interpolation
                    fProcessValue[0] = 0;
                    pfInterpolator = &samData.pfInterpolationFIR_1024_16[dwTemp<<4];
                    for (i=0;i<16;i++)
                    {
                        fProcessValue[0] += interpolation_fStackValue[i] * pfInterpolator[i];
                    }

                    //*******************************************************
                    //Application du IIR pour les effets
                    fFX_IIR_LowPassValue += (fProcessValue[0]-fFX_IIR_LowPassValue) * fFX_IIR_LowPassRatio;
                    
                    //*******************************************************
                    //On stocke dans la pile
                    stack_dwBufferStackPosition = (stack_dwBufferStackPosition+1)&sam_VOICEBUFFERSTACK_MASK;
                    stack_fBufferStackValue[stack_dwBufferStackPosition] = fProcessValue[0];
                    
                    IN:     edx     dwInPlayTickPosition
                            ebx     interpolation_fStackValue
                
                */
                shr         edx, 8                    
                and         edx, 0xFFC0
                
                //Lecture de la pile des données audio pour l'interpolation
                movaps      xmm0, [ebx+ 0]
                movaps      xmm1, [ebx+16]
                movaps      xmm2, [ebx+32]
                movaps      xmm3, [ebx+48]

                //Multiplication par l'interpolateur à phase variable
                mulps       xmm0, samData_f32InterpolationData_1024_16[edx+ 0]
                mulps       xmm1, samData_f32InterpolationData_1024_16[edx+16]
                mulps       xmm2, samData_f32InterpolationData_1024_16[edx+32]
                mulps       xmm3, samData_f32InterpolationData_1024_16[edx+48]

                //Additionne le tout (addition verticale)
                addps       xmm0, xmm1                    
                addps       xmm2, xmm3
                movss       xmm4, fFX_IIR_LowPassValue          //IIR+Stack
                addps       xmm0, xmm2

                mov         edx, pfBufferOut
                mov         ebx, render_dwEntryCountShl3
                
                //Additionne le reste (addition horizontale en SSE)
                movhlps     xmm2, xmm0      //xmm2 = x x D C    xmm0 = D C B A
                addps       xmm0, xmm2      //xmm0 = x x D+B C+A
                movaps      xmm2, xmm0      
                shufps      xmm2, xmm2, 0001b
                addss       xmm0, xmm2
                //movss       xmm2, [edx  ]                       //Charge la sortie #0 - Left
                movups      xmm2, [edx]                             //Charge la sortie #0-#3 - FL/FR/RL/RR
                subss       xmm0, xmm4
                mulss       xmm0, fFX_IIR_LowPassRatio
                addss       xmm4, xmm0
                //movss       xmm3, [edx+4]                       //Charge la sortie #1 - Right
                movups      xmm3, [edx+16]                          //Charge la sortie #4-#7
                
                movss       fFX_IIR_LowPassValue, xmm4
                
                //*******************************************************
                //On envoie sur la sortie avec le générateur de rendu
                /*
                for (dwRenderEntryIndex=0;dwRenderEntryIndex<render_dwEntryCount;dwRenderEntryIndex+=2)
                {
                    *pfBufferOut =
                        stack_fBufferStackValue[ (stack_dwBufferStackPosition-SAM_wi512_dwRender_DelayIndex_LUT[dwRenderEntryIndex])&1023 ] *
                        SAM_wi512_fRender_DelayGain_LUT[dwRenderEntryIndex];
                    *pfBufferOut+1 =
                        stack_fBufferStackValue[ (stack_dwBufferStackPosition-SAM_wi512_dwRender_DelayIndex_LUT[dwRenderEntryIndex+1])&1023 ] *
                        SAM_wi512_fRender_DelayGain_LUT[dwRenderEntryIndex+1];

                }
                
                IN  : edi  = stack_fBufferStackValue
                        ecx  = stack_dwBufferStackPosition (à écrire)
                OUT : -
                */
                
                /*
                    In      :   edx = pfBufferOut
                                ebx = render_dwEntryCountShl3
                                
                    Out     :   -
                    
                    In/Out  :   xmm2, xmm3 => Left/Right output channels
                            
                    Temp    :   eax, ebx, xmm0, xmm1
                */

                //Duplication de XMM4 (x1) en XMM4 (x4)                
                mov         esi, pRender
                
                unpcklps    xmm4, xmm4                  // - | - | s | s
                shufps      xmm4, xmm4, 01000100b       // s | s | s | s
                movaps      xmm5, xmm4
                    
                mulps       xmm4, [esi+SAM_RENDER254.fDelayGain]//SAM_wi512_fRender_DelayGain_LUT+[esi+edx]
                mulps       xmm5, [esi+SAM_RENDER254.fDelayGain+16]//SAM_wi512_fRender_DelayGain_LUT+[esi+edx]
                addps       xmm2, xmm4
                addps       xmm3, xmm5
                
                //***********************************************
                //Misc
                
                mov         edx, pfBufferOut
                mov         eax, dwInPlayTickPosition
                
                mov         ebx, dwInPlaySamplePosition                    
                add         edx, 24
                
                add         eax, dwInPlayTickIncrement
                mov         ecx, dwInPlayGranulePosition                    
                
                mov         esi, eax
                mov         pfBufferOut, edx
                
                shr         esi, 24
                and         eax, 0xFFFFFF
                
                and         esi, 0xFF
                mov         dwInPlayTickPosition, eax
                
                add         ebx, esi
                shr         eax, 8
                add         ecx, esi
                and         eax, 0xFFC0
                
                mov         dwInPlaySamplePosition, ebx
                mov         dwInPlayGranulePosition, ecx
                
                movups       [edx-24], xmm2
                movups       [edx-8], xmm3
                //movss       [edx-8], xmm2
                //movss       [edx-4], xmm3
                
                prefetcht0  samData_f32InterpolationData_1024_16[eax]

            
                sub         byte ptr dwOutputCount, 16
                jnz         _loop_eachsample
                
                mov         edx, lRenderPositionCurrent_F16                 // Interpolation du Render 1/7
                mov         ecx, dwOutputCount
                
                add         edx, lRenderIncrement_F16                       // Interpolation du Render 2/7
                sub         ecx, 256
                
                mov         lRenderPositionCurrent_F16,edx                  // Interpolation du Render 3/7
                mov         dwOutputCount, ecx
                
                and         edx, 0x3F0000                                   // Interpolation du Render 4/7
                shr         edx, 5                                          // Interpolation du Render 5/7
                add         edx, samData.psamRender254Table                 // Interpolation du Render 6/7
                
                cmp         ecx, 0
                mov         pRender, edx                                    // Interpolation du Render 7/7
                prefetcht0  [edx]
                
                jnz         _loop_eachsample
                
            _endloop_eachsample:
            
        rdtsc
        mov dword ptr qwTimeStampB, eax
        mov dword ptr qwTimeStampB+4, edx
        
        //emms
    }
    
    qwSamplesCount += (QWORD)dwSamplesFastCount;
    qwTimeTotal += (qwTimeStampB - qwTimeStampA);
    samData.fCyclesPerSample = 0.1F * (float)(qwTimeTotal*10 / qwSamplesCount);

    //Ecriture des données en R/W
    psamVoice->dwInPlaySamplePosition           = dwInPlaySamplePosition;
    psamVoice->dwInPlaySamplePositionPrevious   = dwInPlaySamplePositionPrevious;
    psamVoice->dwInPlayTickPosition             = dwInPlayTickPosition;
    psamVoice->dwInPlayGranulePosition          = dwInPlayGranulePosition;
    psamVoice->dwCurrentGranuleID               = dwCurrentGranuleID;
    psamVoice->fFX_IIR_LowPassValue             = fFX_IIR_LowPassValue;
    psamVoice->lRenderPositionCurrent_F16       = lRenderPositionCurrent_F16;
}

void SAM_VOICE_MixSingle_Render6ch_noresamp ( SAM_VOICE * psamVoice, SAM_SFX * psamSFX, float fGlobalGain, DWORD dwLoopCount, float * pfSoftwareBuffer, DWORD dwSoftwareBufferChannelCount, DWORD dwParam )
{
    float       fGlobalGain16;
    float       *interpolation_fStackValue;
    
    DWORD       dwOutputCount;

    DWORD       dwInPlaySamplePosition;
    DWORD       dwInPlaySamplePositionPrevious;
    DWORD       dwInPlayTickPosition;
    DWORD       dwInPlayTickIncrement;
    DWORD       dwInPlayGranulePosition;
    DWORD       dwDetectEndOfSamples;

    BYTE        bFormat;
    BYTE        bLoop;
    DWORD       *pAudioDataD32;

    DWORD       dwCurrentGranuleID;
    DWORD       dwBytesPerSampleCount;
    DWORD       dwSamplesPerGranuleCount;

    float       *pfBufferOut;

    float       fFX_IIR_LowPassRatio;
    float       fFX_IIR_LowPassValue;

    DWORD       render_dwEntryCountShl3;
    DWORD       render_dwEntryCount;           // 0...32
    long        lRenderPositionCurrent_F16;
    long        lRenderIncrement_F16;
    
    SAM_RENDER254  *pRender;
    
    
            DWORD       dwLoopEndPositionSample;

    
            QWORD       qwTimeStampA;
            QWORD       qwTimeStampB;
    static  QWORD       qwTimeTotal = 0;
    static  QWORD       qwSamplesCount = 0;
            DWORD       dwSamplesFastCount;
    
            DWORD       dwSoftwareBufferChannelCount4;
    
    //Lecture des données en R/O
    fGlobalGain16                               = fGlobalGain * f32LPCM_Decode16;
    dwSoftwareBufferChannelCount4               = dwSoftwareBufferChannelCount<<2;
    bFormat                                     = psamSFX->bFormat;
    bLoop                                       = (unsigned char)psamVoice->bLoopMode;
    dwInPlayTickIncrement                       = psamVoice->dwInPlayTickIncrement;
    dwLoopEndPositionSample                     = psamSFX->dwLoopEndPositionSample;
    dwDetectEndOfSamples                        = (bLoop)?(psamSFX->dwLoopEndPositionSample-1):(psamSFX->dwSamplesCount);
    pfBufferOut                                 = pfSoftwareBuffer;
    fFX_IIR_LowPassRatio                        = psamVoice->fFX_IIR_LowPassRatio;
    render_dwEntryCount                         = psamVoice->psamRender254->dwEntryCount>>1;
    render_dwEntryCountShl3                     = render_dwEntryCount<<3;
    pRender                                     = &samData.psamRender254Table[ ((psamVoice->lRenderPositionCurrent_F16)>>16) ];
    interpolation_fStackValue                   = psamVoice->interpolation_fStackValue;
    lRenderIncrement_F16                        = psamVoice->lRenderIncrement_F16;

    //Nombre de samples par granule
    SAM_FormatGetBytesCount ( bFormat, &dwBytesPerSampleCount );
    dwSamplesPerGranuleCount                    = sam_GRANULE_BUFFERBYTES / dwBytesPerSampleCount;

    //Lecture des données en R/W
    dwInPlaySamplePosition                      = psamVoice->dwInPlaySamplePosition;
    dwInPlaySamplePositionPrevious              = psamVoice->dwInPlaySamplePositionPrevious;
    dwInPlayTickPosition                        = psamVoice->dwInPlayTickPosition;
    dwInPlayGranulePosition                     = psamVoice->dwInPlayGranulePosition;
    dwCurrentGranuleID                          = psamVoice->dwCurrentGranuleID;
    fFX_IIR_LowPassValue                        = psamVoice->fFX_IIR_LowPassValue;
    lRenderPositionCurrent_F16                  = psamVoice->lRenderPositionCurrent_F16;

    //Les données du granule en cours...
    SAM_GranulesGetData ( dwCurrentGranuleID, &pAudioDataD32 );
    
    //Le compteur
    dwOutputCount = dwLoopCount<<4;

    
    //Traitement avec son mono : Resampling + IIR + Volume + Rendering
        
    __asm {
                //Précharge la pile
                mov         ebx, interpolation_fStackValue
                prefetcht0  [ebx]
                prefetcht0  [ebx+32]
                
                //Précharge le masque
                movaps      xmm7, sam_voice_mixsingle_dwMaskPS_0111
                xorps       xmm6, xmm6
                
                //Démarre le chrono                    
                rdtsc
                mov         dword ptr qwTimeStampA, eax
                mov         dword ptr qwTimeStampA+4, edx
                
                mov         dwSamplesFastCount, 0
                
                jmp         _loop_eachsample
                
                //.align 16, 0x90
                align 16
            _loop_eachsample:    
                mov         ebx, interpolation_fStackValue
                mov         edx, dwInPlayTickPosition
                
                /*
                //*******************************************************
                //Devons-nous charger une nouvelle donnée ?
                if (dwInPlaySamplePosition!=dwInPlaySamplePositionPrevious)
                {
                    dwInPlaySamplePositionPrevious = dwInPlaySamplePosition;
                    
                    SAM_XD4_DecodeValue ( pAudioDataXD4[dwInPlayGranulePosition>>2], (dwInPlayGranulePosition&3) );
                }
                */    
                mov         edi, dwInPlaySamplePosition
                inc         dwSamplesFastCount
                
                cmp         edi, dwInPlaySamplePositionPrevious
                je          _no_new_data
                
                mov         eax, dwInPlayGranulePosition
                mov         esi, pAudioDataD32
                
                // eax = dwInPlayGranulePosition
                // ebx = interpolation_fStackValue
                // edx = dwInPlayTickPosition
                // esi = pAudioDataD32
                // edi = dwInPlaySamplePosition
                
                align 16
            _test_eachsample:    
                // Sommes-nous à la fin du son ?
                cmp         edi, dwDetectEndOfSamples
                jb          _no_end_of_sound
                
                
                //############################################################################################
                //Traitement pour la fin du son
                
                //Sommes-nous dans une boucle ?
                cmp         bLoop, 1
                je          _end_of_sound_yesloop
                

                //############################################################################################                    
                //Nous sommes à la fin définitive du son, tout est fini...
            _end_of_sound_noloop:

                xor         eax, eax
                mov         edi, psamVoice
                mov         esi, psamSFX
                
                mov         dwInPlaySamplePosition, eax
                mov         dwInPlayGranulePosition, eax
                mov         dword ptr [edi+SAM_VOICE.bIsPlay], eax
                mov         ebx, [esi+SAM_SFX.dwGranuleFirst]
                mov         dwCurrentGranuleID, ebx
                jmp         _endloop_eachsample

                //############################################################################################                    
                //Nous sommes à la fin du son, on boucle au début...
            _end_of_sound_yesloop:
            
                //dwTemp = dwInPlaySamplePosition - dwDetectEndOfSamples;
                mov         ecx, edi
                sub         ecx, dwDetectEndOfSamples
                
                mov         esi, psamSFX
                
                //dwInPlaySamplePosition  = psamSFX->dwLoopBeginPositionSample + dwTemp;
                mov         edi, [esi+SAM_SFX.dwLoopBeginPositionSample]
                add         edi, ecx
                mov         dwInPlaySamplePosition, edi
                
                //dwInPlayGranulePosition = psamSFX->dwLoopBeginGranulePositionSample + dwTemp;
                mov         eax, [esi+SAM_SFX.dwLoopBeginGranulePositionSample]
                add         eax, ecx
                mov         dwInPlayGranulePosition, eax
                
                //dwCurrentGranuleID      = psamSFX->dwLoopBeginGranuleID;
                mov         ecx, [esi+SAM_SFX.dwLoopBeginGranuleID]
                mov         dwCurrentGranuleID, ecx
                
                //Mise à jour des pointeurs de données sources
                //SAM_GranulesGetData ( dwCurrentGranuleID, &pAudioDataD32 );
                shl	        ecx, 12
                add         ecx, DWORD PTR samGranulesData.pGranule
                lea         esi, [ecx+SAM_GRANULE.bAudioData]
                mov         pAudioDataD32, esi
                
                // ***************************************************
                // Nous ne sommes pas à la fin du son
                align 16
            _no_end_of_sound:
            
                // Sommes-nous à la fin du granules
                cmp         eax, dwSamplesPerGranuleCount
                jb          _no_end_of_granule

                //############################################################################################                    
                //Traitement pour la fin du granule
                
                //On va au début du granule suivant
                //dwInPlayGranulePosition -= dwSamplesPerGranuleCount;
                sub         eax, dwSamplesPerGranuleCount
                mov         dwInPlayGranulePosition, eax
                
                //Cherche le granule suivant et Mise à jour des pointeurs de données sources
                //SAM_GranulesGetNext ( dwCurrentGranuleID, &dwCurrentGranuleID );
                //SAM_GranulesGetData ( dwCurrentGranuleID, &pAudioDataD32 );
	            mov	        ecx, dwCurrentGranuleID
	            shl	        ecx, 12
	            mov         esi, DWORD PTR samGranulesData.pGranule
	            mov         ecx, [ecx+esi+SAM_GRANULE.dwIdGranuleNext]
	            mov         dwCurrentGranuleID, ecx
	            shl         ecx, 12
	            add         esi, ecx
	            mov         pAudioDataD32, esi
	            
	            jmp         _no_end_of_granule

                // ***************************************************
                // Effectue le décalage de la pile et le chargement de la nouvelle donnée
                align 16
            _no_end_of_granule:
                movsx       eax, word ptr [esi+eax*2]               //1:1
                mov         dwInPlaySamplePositionPrevious, edi     //1:1
                cvtsi2ss    xmm6, eax                               //12:2
                mulss       xmm6, fGlobalGain16                     //7:2    Aplique le volume ici... on économise ainsi un peu de ressource...
                movss       [ebx+ 0], xmm6                          //6:1
                
            _no_new_data:
            
                /*
                    //*******************************************************
                    //Application du IIR pour les effets
                    fFX_IIR_LowPassValue += (fProcessValue[0]-fFX_IIR_LowPassValue) * fFX_IIR_LowPassRatio;
                    
                    //*******************************************************
                    //On stocke dans la pile
                    stack_dwBufferStackPosition = (stack_dwBufferStackPosition+1)&sam_VOICEBUFFERSTACK_MASK;
                    stack_fBufferStackValue[stack_dwBufferStackPosition] = fProcessValue[0];
                    
                    IN:     edx     dwInPlayTickPosition
                            ebx     interpolation_fStackValue
                
                */
                mov         edx, pfBufferOut
                
                movss       xmm0, [ebx+ 0]                          //6:1
                movss       xmm4, fFX_IIR_LowPassValue          //IIR+Stack

                movups      xmm2, [edx]                             //Charge la sortie #0-#3 - FL/FR/RL/RR
                subss       xmm0, xmm4
                mulss       xmm0, fFX_IIR_LowPassRatio
                addss       xmm4, xmm0
                movups      xmm3, [edx+16]                          //Charge la sortie #4-#7
                
                movss       fFX_IIR_LowPassValue, xmm4
                
                //*******************************************************
                //On envoie sur la sortie avec le générateur de rendu
                /*
                for (dwRenderEntryIndex=0;dwRenderEntryIndex<render_dwEntryCount;dwRenderEntryIndex+=2)
                {
                    *pfBufferOut =
                        stack_fBufferStackValue[ (stack_dwBufferStackPosition-SAM_wi512_dwRender_DelayIndex_LUT[dwRenderEntryIndex])&1023 ] *
                        SAM_wi512_fRender_DelayGain_LUT[dwRenderEntryIndex];
                    *pfBufferOut+1 =
                        stack_fBufferStackValue[ (stack_dwBufferStackPosition-SAM_wi512_dwRender_DelayIndex_LUT[dwRenderEntryIndex+1])&1023 ] *
                        SAM_wi512_fRender_DelayGain_LUT[dwRenderEntryIndex+1];

                }
                
                IN  : edi  = stack_fBufferStackValue
                        ecx  = stack_dwBufferStackPosition (à écrire)
                OUT : -
                */
                
                /*
                    In      :   edx = pfBufferOut
                                ebx = render_dwEntryCountShl3
                                
                    Out     :   -
                    
                    In/Out  :   xmm2, xmm3 => Left/Right output channels
                            
                    Temp    :   eax, ebx, xmm0, xmm1
                */

                //Duplication de XMM4 (x1) en XMM4 (x4)                
                mov         esi, pRender
                
                unpcklps    xmm4, xmm4                  // - | - | s | s
                shufps      xmm4, xmm4, 01000100b       // s | s | s | s
                movaps      xmm5, xmm4
                    
                mulps       xmm4, [esi+SAM_RENDER254.fDelayGain]//SAM_wi512_fRender_DelayGain_LUT+[esi+edx]
                mulps       xmm5, [esi+SAM_RENDER254.fDelayGain+16]//SAM_wi512_fRender_DelayGain_LUT+[esi+edx]
                addps       xmm2, xmm4
                addps       xmm3, xmm5
                
                //***********************************************
                //Misc
                
                mov         edx, pfBufferOut
                mov         eax, dwInPlayTickPosition
                
                mov         ebx, dwInPlaySamplePosition                    
                add         edx, 24
                
                add         eax, dwInPlayTickIncrement
                mov         ecx, dwInPlayGranulePosition                    
                
                mov         esi, eax
                mov         pfBufferOut, edx
                
                shr         esi, 24
                and         eax, 0xFFFFFF
                
                and         esi, 0xFF
                mov         dwInPlayTickPosition, eax
                
                add         ebx, esi
                add         ecx, esi
                
                mov         dwInPlaySamplePosition, ebx
                mov         dwInPlayGranulePosition, ecx
                
                movups       [edx-24], xmm2
                movups       [edx-8], xmm3
            
                sub         byte ptr dwOutputCount, 16
                jnz         _loop_eachsample
                
                mov         edx, lRenderPositionCurrent_F16                 // Interpolation du Render 1/7
                mov         ecx, dwOutputCount
                
                add         edx, lRenderIncrement_F16                       // Interpolation du Render 2/7
                sub         ecx, 256
                
                mov         lRenderPositionCurrent_F16,edx                  // Interpolation du Render 3/7
                mov         dwOutputCount, ecx
                
                and         edx, 0x3F0000                                   // Interpolation du Render 4/7
                shr         edx, 5                                          // Interpolation du Render 5/7
                add         edx, samData.psamRender254Table                 // Interpolation du Render 6/7
                
                cmp         ecx, 0
                mov         pRender, edx                                    // Interpolation du Render 7/7
                prefetcht0  [edx]
                
                jnz         _loop_eachsample
                
            _endloop_eachsample:
            
        rdtsc
        mov dword ptr qwTimeStampB, eax
        mov dword ptr qwTimeStampB+4, edx
        
        //emms
    }
    
    qwSamplesCount += (QWORD)dwSamplesFastCount;
    qwTimeTotal += (qwTimeStampB - qwTimeStampA);
    samData.fCyclesPerSample = 0.1F * (float)(qwTimeTotal*10 / qwSamplesCount);

    //Ecriture des données en R/W
    psamVoice->dwInPlaySamplePosition           = dwInPlaySamplePosition;
    psamVoice->dwInPlaySamplePositionPrevious   = dwInPlaySamplePositionPrevious;
    psamVoice->dwInPlayTickPosition             = dwInPlayTickPosition;
    psamVoice->dwInPlayGranulePosition          = dwInPlayGranulePosition;
    psamVoice->dwCurrentGranuleID               = dwCurrentGranuleID;
    psamVoice->fFX_IIR_LowPassValue             = fFX_IIR_LowPassValue;
    psamVoice->lRenderPositionCurrent_F16       = lRenderPositionCurrent_F16;
}

