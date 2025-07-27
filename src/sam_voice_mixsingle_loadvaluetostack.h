
/*
    Charge la valeur et la stocke dans la pile
    
    IN :    EBX
            ESI
            EAX
            fGlobalGain16
            dwInPlaySamplePositionPrevious
            dwTempPS[4] (aligné sur 16 octets)

*/
#define ASMMACRO_LoadValueToStack_SSE_WithFPU       __asm       /* charge la pile courante avec un léger décalage */                    \
                                                    __asm       /* On déborde un peu, mais c'est pas grave car la pile grande */        \
                                                    __asm       movups      xmm0, [ebx+ 4]         /* 6:1 */                            \
                                                    __asm       movups      xmm1, [ebx+20]         /* 6:1 */                            \
                                                    __asm       movups      xmm2, [ebx+36]         /* 6:1 */                            \
                                                    __asm       movups      xmm3, [ebx+52]         /* 6:1 */                            \
                                                    __asm       fild        word ptr [esi+eax*2]                                        \
                                                    __asm       andps       xmm3, xmm7                                                  \
                                                    __asm       /* Aplique le volume ici... on économise ainsi un peu de ressource */   \
                                                    __asm       fmul        fGlobalGain16                                               \
                                                    __asm       mov         dwInPlaySamplePositionPrevious, edi                         \
                                                    __asm       fstp        dword ptr dwTempPS+12                                       \
                                                    __asm       /* Ecriture de la pile */                                               \
                                                    __asm       movaps      [ebx+ 0], xmm0         /* 6:1 */                            \
                                                    __asm       movaps      [ebx+16], xmm1         /* 6:1 */                            \
                                                    __asm       orps        xmm3, dwTempPS                                              \
                                                    __asm       movaps      [ebx+32], xmm2                                              \
                                                    __asm       movaps      [ebx+48], xmm3                                              

#define ASMMACRO_LoadValueToStack_SSE_Only          __asm       /* charge la pile courante avec un léger décalage */                    \
                                                    __asm       /* On déborde un peu, mais c'est pas grave car la pile grande */        \
                                                    __asm       movups      xmm0, [ebx+ 4]         /* 6:1 */                            \
                                                    __asm       movups      xmm1, [ebx+20]         /* 6:1 */                            \
                                                    __asm       movups      xmm2, [ebx+36]         /* 6:1 */                            \
                                                    __asm       movups      xmm3, [ebx+52]         /* 6:1 */                            \
                                                    __asm       movsx       eax, word ptr [esi+eax*2]               /* 1:1 */           \
                                                    __asm       mov         dwInPlaySamplePositionPrevious, edi     /* 1:1 */           \
                                                    __asm       cvtsi2ss    xmm6, eax              /* 12:2 */                           \
                                                    __asm       /* Ecriture de la pile */                                               \
                                                    __asm       movaps      [ebx+ 0], xmm0         /* 6:1 */                            \
                                                    __asm       movaps      [ebx+16], xmm1         /* 6:1 */                            \
                                                    __asm       /* Aplique le volume ici... on économise ainsi un peu de ressource */   \
                                                    __asm       mulss       xmm6, fGlobalGain16    /* 7:2 */                            \
                                                    __asm       movaps      [ebx+32], xmm2         /* 6:1 */                            \
                                                    __asm       andps       xmm3, xmm7             /* 4:2 */                            \
                                                    __asm       shufps      xmm6, xmm6, 00010101b  /* 6:2 */                            \
                                                    __asm       orps        xmm3, xmm6             /* 4:2 */                            \
                                                    __asm       movaps      [ebx+48], xmm3         /* 6:1 */
                                                    
