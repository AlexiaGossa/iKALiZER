#include "sam_header.h"
#include "sam_data.h"
#include "sam_voice.h"
/*
    L'assembleur :
    
        Il se charge de convertir en code exécutable de l'assembleur x86+MMX+SSE+SSE2
        
        La complexité du code doit être faible :
        - Instruction sur une seule ligne
        - Gestion des sauts conditionnels via "Jxx" et inconditionnels via "Jump"



*/


/*
    Pour chaque voix :
    
        On s'occupe de la partie 


*/


/*
    Type de routines :
        SAM_VOICE_MixSingleGranule_WithStackSSE
        SAM_VOICE_MixSingleGranule_WithoutStackSSE_2ch
        SAM_VOICE_MixSingleGranule_WithoutStackSSE_4ch
        SAM_VOICE_MixSingleGranule_WithoutStackSSE_6ch
        SAM_VOICE_MixSingleGranule_WithoutStackSSE_8ch
        
    Routine spéciale "DirectOutput" utilisé dans UrbanTerror pour les sons radio et ceux très proches (les armes notamment)        
        SAM_VOICE_MixSingleGranule_2ch_SSE (direct output L+R without)

    Routines à écrire :
        - 16bits + Over-sampling + IIR
        - 16bits + No-resampling + IIR
        - 16bits + Over-sampling
        - 16bits + No-resampling
        
    


*/

/*
    Etape 1
        IKA_DMIX_SSE_INIT    
        IKA_DMIX_SSE_BEGIN_ES
        
    Etape 2a
        IKA_DMIX_SSE_PREGET_wostk
            ou
        IKA_DMIX_SSE_PREGET_wistk
        
    Etape 2b
        switch (resamp)
            case OUI :
                IKA_DMIX_SSE_LOAD_OVERSAMP
                IKA_DMIX_SSE_GET_OVERSAMP
                if (FX-IIR)
                    IKA_DMIX_SSE_GET_OVERSAMP_FX
                else
                    IKA_DMIX_SSE_GET_OVERSAMP_NOFX
            
            case NON :
                IKA_DMIX_SSE_LOAD_NORESAMP
                if (FX-IIR)
                    IKA_DMIX_SSE_GET_NORESAMP_FX
                else
                    IKA_DMIX_SSE_GET_NORESAMP_NOFX
        
    Etape 3 
        switch (rendermode)
            case 8CH
                IKA_DMIX_SSE_RENDERLOAD_8CH
            
            case 6CH
                IKA_DMIX_SSE_RENDERLOAD_6CH
                
            case 4CH
                IKA_DMIX_SSE_RENDERLOAD_4CH
                
            case 2CH
                IKA_DMIX_SSE_RENDERLOAD_2CH
                
            case DUAL
                IKA_DMIX_SSE_RENDERLOAD_DUAL

    Etape 5                
    

*/

    //Traitement avec son mono : Resampling + IIR + Volume + Rendering
#define IKA_DMIX_SSE_INIT           __asm   /*Précharge la pile */                                                  \
                                    __asm   mov         ebx, interpolation_fStackValue                              \
                                    __asm   prefetcht0  [ebx]                                                       \
                                    __asm   prefetcht0  [ebx+32]                                                    \
                                    __asm   /*Précharge le masque*/                                                 \
                                    __asm   movaps      xmm7, sam_voice_mixsingle_dwMaskPS_0111                     \
                                    __asm   xorps       xmm6, xmm6                                                  \
                                    __asm   /*Démarre le chrono*/                                                   \
                                    __asm   rdtsc                                                                   \
                                    __asm   mov         dword ptr qwTimeStampA, eax                                 \
                                    __asm   mov         dword ptr qwTimeStampA+4, edx                               \
                                    __asm   mov         dwSamplesFastCount, 0


#define IKA_DMIX_SSE_BEGIN_ES       __asm   align 16                                                                \
                                    __asm   _loop_eachsample:                                                       \
                                    __asm   mov         ebx, interpolation_fStackValue                              \
                                    __asm   mov         edx, dwInPlayTickPosition                                   \
                                    __asm   /*Devons-nous charger une nouvelle donnée ?*/                           \
                                    __asm   mov         edi, dwInPlaySamplePosition                                 \
                                    __asm   inc         dwSamplesFastCount                                          \
                                    __asm   cmp         edi, dwInPlaySamplePositionPrevious                         \
                                    __asm   je          _no_new_data                                                \
                                    __asm   mov         eax, dwInPlayGranulePosition                                \
                                    __asm   mov         esi, pAudioDataD32                                          \
                                    __asm   /* eax = dwInPlayGranulePosition */                                     \
                                    __asm   /* ebx = interpolation_fStackValue */                                   \
                                    __asm   /* edx = dwInPlayTickPosition */                                        \
                                    __asm   /* esi = pAudioDataD32 */                                               \
                                    __asm   /* edi = dwInPlaySamplePosition */                                      \
                                    __asm   align 16                                                                \
                                    __asm   _test_eachsample:                                                       \
                                    __asm   /* Sommes-nous à la fin du son ? */                                     \
                                    __asm   cmp         edi, dwDetectEndOfSamples                                   \
                                    __asm   jb          _no_end_of_sound                                            \
                                    __asm   /*###############################################################*/     \
                                    __asm   /*Traitement pour la fin du son*/                                       \
                                    __asm   /*Sommes-nous dans une boucle ? */                                      \
                                    __asm   cmp         bLoop, 1                                                    \
                                    __asm   je          _end_of_sound_yesloop                                       \
                                    __asm   /*###############################################################*/     \
                                    __asm   /*Nous sommes à la fin définitive du son, tout est fini...*/            \
                                    __asm   _end_of_sound_noloop:                                                   \
                                    __asm   xor         eax, eax                                                    \
                                    __asm   mov         edi, psamVoice                                              \
                                    __asm   mov         esi, psamSFX                                                \
                                    __asm   mov         dwInPlaySamplePosition, eax                                 \
                                    __asm   mov         dwInPlayGranulePosition, eax                                \
                                    __asm   mov         dword ptr [edi+SAM_VOICE.bIsPlay], eax                      \
                                    __asm   mov         ebx, [esi+SAM_SFX.dwGranuleFirst]                           \
                                    __asm   mov         dwCurrentGranuleID, ebx                                     \
                                    __asm   jmp         _endloop_eachsample                                         \
                                    __asm   /*###############################################################*/     \
                                    __asm   /*Nous sommes à la fin du son, on boucle au début...*/                  \
                                    __asm   _end_of_sound_yesloop:                                                  \
                                    __asm   /*dwTemp = dwInPlaySamplePosition - dwDetectEndOfSamples;*/             \
                                    __asm   mov         ecx, edi                                                    \
                                    __asm   sub         ecx, dwDetectEndOfSamples                                   \
                                    __asm   mov         esi, psamSFX                                                \
                                    __asm   /*dwInPlaySamplePosition  = psamSFX->dwLoopBeginPositionSample + dwTemp;*/\
                                    __asm   mov         edi, [esi+SAM_SFX.dwLoopBeginPositionSample]                \
                                    __asm   add         edi, ecx                                                    \
                                    __asm   mov         dwInPlaySamplePosition, edi                                 \
                                    __asm   /*dwInPlayGranulePosition = psamSFX->dwLoopBeginGranulePositionSample + dwTemp;*/\
                                    __asm   mov         eax, [esi+SAM_SFX.dwLoopBeginGranulePositionSample]         \
                                    __asm   add         eax, ecx                                                    \
                                    __asm   mov         dwInPlayGranulePosition, eax                                \
                                    __asm   /*dwCurrentGranuleID      = psamSFX->dwLoopBeginGranuleID;*/            \
                                    __asm   mov         ecx, [esi+SAM_SFX.dwLoopBeginGranuleID]                     \
                                    __asm   mov         dwCurrentGranuleID, ecx                                     \
                                    __asm   /*Mise à jour des pointeurs de données sources*/                        \
                                    __asm   /*SAM_GranulesGetData ( dwCurrentGranuleID, &pAudioDataD32 );*/         \
                                    __asm   shl	        ecx, 12                                                     \
                                    __asm   add         ecx, DWORD PTR samGranulesData.pGranule                     \
                                    __asm   lea         esi, [ecx+SAM_GRANULE.bAudioData]                           \
                                    __asm   mov         pAudioDataD32, esi                                          \
                                    __asm   /* *************************************************** */               \
                                    __asm   /* Nous ne sommes pas à la fin du son */                                \
                                    __asm   align 16                                                                \
                                    __asm   _no_end_of_sound:                                                       \
                                    __asm   /* Sommes-nous à la fin du granules */                                  \
                                    __asm   cmp         eax, dwSamplesPerGranuleCount                               \
                                    __asm   jb          _no_end_of_granule                                          \
                                    __asm   /*######################################################*/              \
                                    __asm   /*Traitement pour la fin du granule*/                                   \
                                    __asm   /*On va au début du granule suivant*/                                   \
                                    __asm   /*dwInPlayGranulePosition -= dwSamplesPerGranuleCount;*/                \
                                    __asm   sub         eax, dwSamplesPerGranuleCount                               \
                                    __asm   mov         dwInPlayGranulePosition, eax                                \
                                    __asm   /*Cherche le granule suivant et Mise à jour des pointeurs de données sources*/\
                                    __asm   /*SAM_GranulesGetNext ( dwCurrentGranuleID, &dwCurrentGranuleID );*/    \
                                    __asm   /*SAM_GranulesGetData ( dwCurrentGranuleID, &pAudioDataD32 );*/         \
	                                __asm   mov	        ecx, dwCurrentGranuleID                                     \
	                                __asm   shl	        ecx, 12                                                     \
	                                __asm   mov         esi, DWORD PTR samGranulesData.pGranule                     \
	                                __asm   mov         ecx, [ecx+esi+SAM_GRANULE.dwIdGranuleNext]                  \
	                                __asm   mov         dwCurrentGranuleID, ecx                                     \
	                                __asm   shl         ecx, 12                                                     \
	                                __asm   add         esi, ecx                                                    \
	                                __asm   mov         pAudioDataD32, esi
	                                //__asm   jmp         _no_end_of_granule

#define IKA_DMIX_SSE_LOAD_OVERSAMP  __asm   /*align 16*/                                                            \
                                    __asm   /* *************************************************** */               \
                                    __asm   /* Effectue le décalage de la pile et le chargement de la nouvelle donnée */ \
                                    __asm   align 16                                                                \
                                    __asm   _no_end_of_granule:                                                     \
                                    __asm   /*charge la pile courante avec un léger décalage*/                      \
                                    __asm   movups      xmm0, [ebx+ 4]                                              \
                                    __asm   movups      xmm1, [ebx+20]                                              \
                                    __asm   movups      xmm2, [ebx+36]                                              \
                                    __asm   movups      xmm3, [ebx+52] /*On déborde un peu, mais c'est pas grave car la pile fait plus de 16 points (pour la gestion en stéréo)*/\
                                    __asm   movsx       eax, word ptr [esi+eax*2]                                   \
                                    __asm   mov         dwInPlaySamplePositionPrevious, edi                         \
                                    __asm   cvtsi2ss    xmm6, eax                                                   \
                                    __asm   /*Ecriture de la pile*/                                                 \
                                    __asm   movaps      [ebx+ 0], xmm0                                              \
                                    __asm   mulss       xmm6, fGlobalGain16 /*Aplique le volume ici...*/            \
                                    __asm   movaps      [ebx+16], xmm1                                              \
                                    __asm   andps       xmm3, xmm7                                                  \
                                    __asm   shufps      xmm6, xmm6, 00010101b                                       \
                                    __asm   movaps      [ebx+32], xmm2                                              \
                                    __asm   orps        xmm3, xmm6                                                  \
                                    __asm   movaps      [ebx+48], xmm3                                              
                
#define IKA_DMIX_SSE_LOAD_NORESAMP  __asm   /*align 16*/                                                            \
                                    __asm   _no_end_of_granule:                                                     \
                                    __asm   movsx       eax, word ptr [esi+eax*2]                                   \
                                    __asm   mov         dwInPlaySamplePositionPrevious, edi                         \
                                    __asm   cvtsi2ss    xmm6, eax                                                   \
                                    __asm   mulss       xmm6, fGlobalGain16 /*Aplique le volume ici...*/            \
                                    __asm   movss       [ebx+ 0], xmm6                          

#define IKA_DMIX_SSE_PREGET_wistk   __asm   _no_new_data:                                                           \
                                    __asm   shr         edx, 8                                                      \
                                    __asm   mov         ecx, stack_dwBufferStackPosition                            \
                                    __asm   and         edx, 0xFFC0                                                 \
                                    __asm   dec         ecx                                                         \
                                    __asm   mov         edi, stack_fBufferStackValue                                \
                                    __asm   and         ecx, sam_VOICEBUFFERSTACK_MASK

#define IKA_DMIX_SSE_PREGET_wostk   __asm   _no_new_data:                                                           \
                                    __asm   shr         edx, 8                                                      \
                                    __asm   and         edx, 0xFFC0
                
#define IKA_DMIX_SSE_GET_OVERSAMP_NOFX __asm   /*Génération de la phase sur 10 bits*/                               \
                                    __asm   /*Application de l'interpolation*/                                      \
                                    __asm   /*Application du IIR pour les effets*/                                  \
                                    __asm   /*On stocke dans la pile*/                                              \
                                    __asm   /*Lecture de la pile des données audio pour l'interpolation*/           \
                                    __asm   movaps      xmm4, [ebx+ 0]                                              \
                                    __asm   movaps      xmm1, [ebx+16]                                              \
                                    __asm   movaps      xmm2, [ebx+32]                                              \
                                    __asm   movaps      xmm3, [ebx+48]                                              \
                                    __asm   /*Multiplication par l'interpolateur à phase variable*/                 \
                                    __asm   mulps       xmm4, samData_f32InterpolationData_1024_16[edx+ 0]          \
                                    __asm   mulps       xmm1, samData_f32InterpolationData_1024_16[edx+16]          \
                                    __asm   mulps       xmm2, samData_f32InterpolationData_1024_16[edx+32]          \
                                    __asm   mulps       xmm3, samData_f32InterpolationData_1024_16[edx+48]          \
                                    __asm   /*Additionne le tout (addition verticale)              */               \
                                    __asm   addps       xmm4, xmm1                                                  \
                                    __asm   addps       xmm2, xmm3                                                  \
                                    __asm   addps       xmm4, xmm2                                                  \
                                    __asm   /*Additionne le reste (addition horizontale en SSE)*/                   \
                                    __asm   movhlps     xmm2, xmm4                                                  \
                                    __asm   addps       xmm4, xmm2                                                  \
                                    __asm   movaps      xmm2, xmm4                                                  \
                                    __asm   shufps      xmm2, xmm2, 0001b                                           \
                                    __asm   addss       xmm4, xmm2

#define IKA_DMIX_SSE_GET_OVERSAMP_FX __asm   /*Génération de la phase sur 10 bits*/                                 \
                                    __asm   /*Application de l'interpolation*/                                      \
                                    __asm   /*Application du IIR pour les effets*/                                  \
                                    __asm   /*On stocke dans la pile*/                                              \
                                    __asm   /*Lecture de la pile des données audio pour l'interpolation*/           \
                                    __asm   movaps      xmm0, [ebx+ 0]                                              \
                                    __asm   movaps      xmm1, [ebx+16]                                              \
                                    __asm   movaps      xmm2, [ebx+32]                                              \
                                    __asm   movaps      xmm3, [ebx+48]                                              \
                                    __asm   /*Multiplication par l'interpolateur à phase variable*/                 \
                                    __asm   mulps       xmm0, samData_f32InterpolationData_1024_16[edx+ 0]          \
                                    __asm   mulps       xmm1, samData_f32InterpolationData_1024_16[edx+16]          \
                                    __asm   mulps       xmm2, samData_f32InterpolationData_1024_16[edx+32]          \
                                    __asm   mulps       xmm3, samData_f32InterpolationData_1024_16[edx+48]          \
                                    __asm   /*Additionne le tout (addition verticale)              */               \
                                    __asm   addps       xmm0, xmm1                                                  \
                                    __asm   addps       xmm2, xmm3                                                  \
                                    __asm   addps       xmm0, xmm2                                                  \
                                    __asm   /*Additionne le reste (addition horizontale en SSE)*/                   \
                                    __asm   movhlps     xmm2, xmm0                                                  \
                                    __asm   addps       xmm0, xmm2                                                  \
                                    __asm   movaps      xmm2, xmm0                                                  \
                                    __asm   shufps      xmm2, xmm2, 0001b                                           \
                                    __asm   movss       xmm4, fFX_IIR_LowPassValue                                  \
                                    __asm   addss       xmm0, xmm2                                                  \
                                    __asm   subss       xmm0, xmm4                                                  \
                                    __asm   mulss       xmm0, fFX_IIR_LowPassRatio                                  \
                                    __asm   addss       xmm4, xmm0                                                  \
                                    __asm   movss       fFX_IIR_LowPassValue, xmm4

#define IKA_DMIX_SSE_GET_NORESAMP_NOFX __asm movss      xmm4, [ebx+ 0]


#define IKA_DMIX_SSE_GET_NORESAMP_FX __asm  movss       xmm4, fFX_IIR_LowPassValue                                  \
                                    __asm   movss       xmm0, [ebx+ 0]                                              \
                                    __asm   subss       xmm0, xmm4                                                  \
                                    __asm   mulss       xmm0, fFX_IIR_LowPassRatio                                  \
                                    __asm   addss       xmm4, xmm0                                                  \
                                    __asm   movss       fFX_IIR_LowPassValue, xmm4

                                    
                                    

#define IKA_DMIX_SSE_RENDER_wistk2CH        __asm   mov         edx, pfBufferOut                                            \
                                            __asm   mov         ebx, render_dwEntryCountShl3                                \
                                            __asm   movss       [edi+ecx*4], xmm4 /*edi+ecx*4 = BufferStackValue + stack_dwBufferStackPosition*/ \
                                            __asm   movss       xmm2, [edx  ]   /*Charge la sortie #0 - Left*/              \
                                            __asm   movss       xmm3, [edx+4]   /*Charge la sortie #1 - Right*/             \
                                            __asm   /*On envoie sur la sortie avec le générateur de rendu*/                 \
                                            __asm   mov         esi, pRender                                                \
                                            __asm   xor         edx, edx                                                    \
                                            __asm   add         esi, ebx                                                    \
                                            __asm   mov         stack_dwBufferStackPosition, ecx                            \
                                            __asm   sub         edx, ebx                                                    \
                                            __asm   mov         edi, stack_fBufferStackValue                                \
                                            __asm   /* edx = décompteur (par deux) */                                       \
                                            __asm   /* ecx = stack_dwBufferStackPosition */                                 \
                                            __asm   /* esi = offset des 2 LUT (Delay et Gain) */                            \
                                            __asm   /* edi = offset de la pile "stack_fBufferStackValue" */                 \
                                            __asm   _loop_render:                                                           \
                                            __asm   mov         eax, ecx                                                    \
                                            __asm   mov         ebx, ecx                                                    \
                                            __asm   add         eax, [esi+edx+  SAM_RENDER254.dwDelayIndex]                 \
                                            __asm   add         ebx, [esi+edx+4+SAM_RENDER254.dwDelayIndex]                 \
                                            __asm   and         eax, sam_VOICEBUFFERSTACK_MASK                              \
                                            __asm   and         ebx, sam_VOICEBUFFERSTACK_MASK                              \
                                            __asm   movss       xmm0, [esi+edx+  SAM_RENDER254.fDelayGain]                  \
                                            __asm   movss       xmm1, [esi+edx+4+SAM_RENDER254.fDelayGain]                  \
                                            __asm   mulss       xmm0, [edi+eax*4]                                           \
                                            __asm   mulss       xmm1, [edi+ebx*4]                                           \
                                            __asm   addss       xmm2, xmm0                                                  \
                                            __asm   addss       xmm3, xmm1                                                  \
                                            __asm   add         edx, 8                                                      \
                                            __asm   jnz         _loop_render                                                \
                                            __asm   movss       [edx  ], xmm2                                               \
                                            __asm   movss       [edx+4], xmm3

#define IKA_DMIX_SSE_RENDER_directLR        __asm   mov         edx, pfBufferOut                                            \
                                            __asm   movaps      xmm3, _SAM_VOICE_MSG_2ch_fLevelGainLR                       \
                                            __asm   unpcklps    xmm4, xmm4                                                  \
                                            __asm   movlps      xmm2, [edx]     /*Charge la sortie #1 & #0 (ou FR & FL)*/   \
                                            __asm   mulps       xmm4, xmm3                                                  \
                                            __asm   addps       xmm2, xmm4                                                  \
                                            __asm   movlps      [edx], xmm2

#define IKA_DMIX_SSE_RENDER_wostk2CH        __asm   mov         edx, pfBufferOut                                            \
                                            __asm   mov         ebx, render_dwEntryCountShl3                                \
                                            __asm   mov         esi, pRender                                                \
                                            __asm   movss       xmm0, xmm4                                                  \
                                            __asm   movss       xmm1, xmm4                                                  \
                                            __asm   movss       xmm2, [edx  ]   /*Charge la sortie #0 - Left*/              \
                                            __asm   mulss       xmm0, [esi+  SAM_RENDER254.fDelayGain]                      \
                                            __asm   movss       xmm3, [edx+4]   /*Charge la sortie #1 - Right*/             \
                                            __asm   mulss       xmm1, [esi+4+SAM_RENDER254.fDelayGain]                      \
                                            __asm   addss       xmm2, xmm0                                                  \
                                            __asm   addss       xmm3, xmm1                                                  \
                                            __asm   movss       [edx  ], xmm2                                               \
                                            __asm   movss       [edx+4], xmm3
                                            

#define IKA_DMIX_SSE_RENDER_wostk4CH        __asm   mov         edx, pfBufferOut                                            \
                                            __asm   mov         ebx, render_dwEntryCountShl3                                \
                                            __asm   mov         esi, pRender                                                \
                                            __asm   unpcklps    xmm4, xmm4                                                  \
                                            __asm   movaps      xmm2, [edx]     /*Charge la sortie #0-#3 - FL/FR/RL/RR*/    \
                                            __asm   shufps      xmm4, xmm4, 01000100b                                       \
                                            __asm   mulps       xmm4, [esi+SAM_RENDER254.fDelayGain]                        \
                                            __asm   addps       xmm2, xmm4                                                  \
                                            __asm   movaps      [edx], xmm2


#define IKA_DMIX_SSE_RENDER_wostk6CH        __asm   mov         edx, pfBufferOut                                            \
                                            __asm   mov         ebx, render_dwEntryCountShl3                                \
                                            __asm   mov         esi, pRender                                                \
                                            __asm   unpcklps    xmm4, xmm4                                                  \
                                            __asm   movups      xmm2, [edx]     /*Charge la sortie #0-#3 - FL/FR/RL/RR*/    \
                                            __asm   shufps      xmm4, xmm4, 01000100b                                       \
                                            __asm   movups      xmm3, [edx+16]  /*Charge la sortie #4-#7*/                  \
                                            __asm   movaps      xmm5, xmm4                                                  \
                                            __asm   mulps       xmm4, [esi+SAM_RENDER254.fDelayGain]                        \
                                            __asm   mulps       xmm5, [esi+SAM_RENDER254.fDelayGain+16]                     \
                                            __asm   addps       xmm2, xmm4                                                  \
                                            __asm   addps       xmm3, xmm5                                                  \
                                            __asm   movups      [edx   ], xmm2                                              \
                                            __asm   movups      [edx+16], xmm3


#define IKA_DMIX_SSE_RENDER_wostk8CH        __asm   mov         edx, pfBufferOut                                            \
                                            __asm   mov         ebx, render_dwEntryCountShl3                                \
                                            __asm   mov         esi, pRender                                                \
                                            __asm   unpcklps    xmm4, xmm4                                                  \
                                            __asm   movaps      xmm2, [edx]     /*Charge la sortie #0-#3 - FL/FR/RL/RR*/    \
                                            __asm   shufps      xmm4, xmm4, 01000100b                                       \
                                            __asm   movaps      xmm3, [edx+16]  /*Charge la sortie #4-#7*/                  \
                                            __asm   movaps      xmm5, xmm4                                                  \
                                            __asm   mulps       xmm4, [esi+SAM_RENDER254.fDelayGain]                        \
                                            __asm   mulps       xmm5, [esi+SAM_RENDER254.fDelayGain+16]                     \
                                            __asm   addps       xmm2, xmm4                                                  \
                                            __asm   addps       xmm3, xmm5                                                  \
                                            __asm   movaps      [edx   ], xmm2                                              \
                                            __asm   movaps      [edx+16], xmm3

//emit
#define IKA_DMIX_SSE_ENDLOOP_OVERSAMP_render(iChannelCount) __asm /*Fin de la boucle avec gestion rendering et oversamp*/   \
                                            __asm   mov         edx, pfBufferOut                                            \
                                            __asm   mov         eax, dwInPlayTickPosition                                   \
                                            __asm   mov         ebx, dwInPlaySamplePosition                                 \
                                            __asm   add         edx, iChannelCount*4                                        \
                                            __asm   add         eax, dwInPlayTickIncrement                                  \
                                            __asm   mov         ecx, dwInPlayGranulePosition                                \
                                            __asm   mov         esi, eax                                                    \
                                            __asm   mov         pfBufferOut, edx                                            \
                                            __asm   shr         esi, 24                                                     \
                                            __asm   and         eax, 0xFFFFFF                                               \
                                            __asm   and         esi, 0xFF                                                   \
                                            __asm   mov         dwInPlayTickPosition, eax                                   \
                                            __asm   add         ebx, esi                                                    \
                                            __asm   shr         eax, 8                                                      \
                                            __asm   add         ecx, esi                                                    \
                                            __asm   and         eax, 0xFFC0                                                 \
                                            __asm   mov         dwInPlaySamplePosition, ebx                                 \
                                            __asm   mov         dwInPlayGranulePosition, ecx                                \
                                            __asm   prefetcht0  samData_f32InterpolationData_1024_16[eax]                   \
                                            __asm   sub         byte ptr dwOutputCount, 16                                  \
                                            __asm   jnz         _loop_eachsample                                            \
                                            __asm   mov         edx, lRenderPositionCurrent_F16 /*Interpolation du Render 1/7*/\
                                            __asm   mov         ecx, dwOutputCount                                          \
                                            __asm   add         edx, lRenderIncrement_F16       /*Interpolation du Render 2/7*/\
                                            __asm   sub         ecx, 256                                                    \
                                            __asm   mov         lRenderPositionCurrent_F16,edx  /*Interpolation du Render 3/7*/\
                                            __asm   mov         dwOutputCount, ecx                                          \
                                            __asm   and         edx, 0x3F0000                   /*Interpolation du Render 4/7*/\
                                            __asm   shr         edx, 5                          /*Interpolation du Render 5/7*/\
                                            __asm   add         edx, samData.psamRender254Table /*Interpolation du Render 6/7*/\
                                            __asm   cmp         ecx, 0                                                      \
                                            __asm   mov         pRender, edx                    /*Interpolation du Render 7/7*/\
                                            __asm   prefetcht0  [edx]                                                       \
                                            __asm   jnz         _loop_eachsample                                            \
                                            __asm   _endloop_eachsample:

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
/*
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
                add         edx, 32
                
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
                
                movaps       [edx-32], xmm2
                movaps       [edx-16], xmm3
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

*/