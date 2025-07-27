//#include "sam_header.h"
//#include "sam_data.h"
#include "sam_render.h"

#ifdef _WIN32
#pragma warning( disable : 4731 )
#endif

INT16 pi16CopyTarget[128000];
#define CVT_dB2VAL(n)   ((long)((n)*16777216))


void IKALIZER_Limiter_Stereo ( FLOAT32 * pf32SourceData, INT16 * pi16TargetData, long lSamplesCopyCount, long lTargetOffset, long lChannelCount, long lLimiterMode );
extern float __0x26__f32Impulse[6][2][2048];
extern SAM_RENDER254   __0x26__Render254[6];

sam_ALIGN_SPEC(32)      FLOAT32         samData0x26_f32FIRStackValue[8*2048];
sam_ALIGN_SPEC(32)      DWORD           samData0x26_dwFIRStackPosition;

/*
    Une table de 6x2x254 entrées


*/

//sam_ALIGN_SPEC(32)      FLOAT32         samData0x26_f32MulValue[8*254]; //6Ko
//sam_ALIGN_SPEC(32)      DWORD           samData0x26_dwIndexValue[8*254]; //6Ko
//sam_ALIGN_SPEC(32)      FLOAT32         samData0x26_f32MulValue[6*256*2]; //12Ko
//sam_ALIGN_SPEC(32)      DWORD           samData0x26_dwIndexValue[6*256]; //6Ko

sam_ALIGN_SPEC(32)      DWORD           samData0x26_dwIndexCount[6];
sam_ALIGN_SPEC(32)      FLOAT32         samData0x26_f32MulValue_MuxedLR[6*128]; //3Ko
sam_ALIGN_SPEC(32)      DWORD           samData0x26_dwIndexValue_MuxedLR[6*128];  //3Ko
sam_ALIGN_SPEC(32)      WORD            samData0x26_wIndexValue_MuxedLR[6*128];  //1.5Ko

int SAM_PostProcessCopy_0x26_Init_DelayLineSortByIndexProc ( const void * pElementA, const void * pElementB )
{
    DELAYLINE   *pDelayLineChannelA;
    DELAYLINE   *pDelayLineChannelB;
    pDelayLineChannelA = (DELAYLINE *)pElementA;
    pDelayLineChannelB = (DELAYLINE *)pElementB;
    return ((int)pDelayLineChannelA->dwDelayIndex) - ((int)pDelayLineChannelB->dwDelayIndex);
}

void SAM_PostProcessCopy_0x26_Init ( void )
{
    long i,j,k,lOffset;
    long lCountL, lCountR;
    DWORD dwIndex;
    float fDelayGain;
    DELAYLINE DelayLineChannelL[128];
    DELAYLINE DelayLineChannelR[128];

    
    for (i=0;i<6;i++)
    {
        //Init the delay line
        for (j=0;j<128;j++)
        {
            DelayLineChannelL[j].dwDelayIndex = 2047;
            DelayLineChannelR[j].dwDelayIndex = 2047;
            DelayLineChannelL[j].fDelayGain   = 0.0F;
            DelayLineChannelR[j].fDelayGain   = 0.0F;
        }
    
        //Fill DelayLine
        lCountL = 0;
        lCountR = 0;
        for (j=0;j<254;j+=2)
        {
            //if (sam_ABS(__0x26__Render254[i].fDelayGain[j  ])>0.005F)  //-46dB mini
            if (lCountL<64)
            {
                DelayLineChannelL[lCountL].dwDelayIndex = __0x26__Render254[i].dwDelayIndex[j  ];
                DelayLineChannelL[lCountL].fDelayGain   = __0x26__Render254[i].fDelayGain[j  ];
                lCountL++;
            }
            
            //if (sam_ABS(__0x26__Render254[i].fDelayGain[j+1])>0.005F)  //-46dB mini
            if (lCountR<64)
            {
                DelayLineChannelR[lCountR].dwDelayIndex = __0x26__Render254[i].dwDelayIndex[j+1];
                DelayLineChannelR[lCountR].fDelayGain   = __0x26__Render254[i].fDelayGain[j+1];
                lCountR++;
            }
        }
        
        //Apply a quick sort to each channel by DelayIndex
                
        qsort (
            DelayLineChannelL,
            128,
            sizeof(DELAYLINE),
           SAM_PostProcessCopy_0x26_Init_DelayLineSortByIndexProc );
            
        qsort (
            DelayLineChannelR,
            128,
            sizeof(DELAYLINE),
            SAM_PostProcessCopy_0x26_Init_DelayLineSortByIndexProc );

        samData0x26_dwIndexCount[i] = 64;
        for (j=0;j<64;j++)
        {
            lOffset = (i*128)+(j*2);
            samData0x26_f32MulValue_MuxedLR[lOffset  ]  = DelayLineChannelL[j].fDelayGain;
            samData0x26_f32MulValue_MuxedLR[lOffset+1]  = DelayLineChannelR[j].fDelayGain;
            samData0x26_dwIndexValue_MuxedLR[lOffset  ] = (DelayLineChannelL[j].dwDelayIndex*32) + 8 + (i*4); //32 bytes per sample
            samData0x26_dwIndexValue_MuxedLR[lOffset+1] = (DelayLineChannelR[j].dwDelayIndex*32) + 8 + (i*4); //32 bytes per sample
            
            samData0x26_wIndexValue_MuxedLR[lOffset  ] = (DelayLineChannelL[j].dwDelayIndex*32) + 8 + (i*4); //32 bytes per sample
            samData0x26_wIndexValue_MuxedLR[lOffset+1] = (DelayLineChannelR[j].dwDelayIndex*32) + 8 + (i*4); //32 bytes per sample
        }
    }
    
}

                
                

void SAM_PostProcessCopy_0x26 ( FLOAT32 * pf32SourceData, INT16 * pi16TargetData, long lSamplesCopyCount, long lTargetOffset, long lChannelCount, long lLimiterMode )
{
    FLOAT32     *pf32Source;
    INT16       *pi16Target;

    float       fLimiterGain;

    static      DWORD dwMMX_32 = 32;
    static      float f32SSE_LimiterGain[4];

    pf32Source          = pf32SourceData;
    pi16Target          = pi16TargetData;
    
    switch (lLimiterMode)
    {
        case 0: //0dB
            fLimiterGain        = 1.0;
            break;

        case 1: //+6dB
            fLimiterGain        = 2.0;
            break;

        case 2: //+12dB
            fLimiterGain        = 4.0;
            break;

        case 3: //+20dB                 
            fLimiterGain        = 10.0;
            break;
    }

    f32SSE_LimiterGain[0] = fLimiterGain;
    f32SSE_LimiterGain[1] = fLimiterGain;
    f32SSE_LimiterGain[2] = 0;
    f32SSE_LimiterGain[3] = 0;
    
    //3%
        __asm {
                movd        mm4, dwMMX_32
                movd        mm7, pf32Source
                
                movups      xmm5, f32SSE_LimiterGain
                
                mov         ecx, samData0x26_dwFIRStackPosition
                
                mov         eax, lSamplesCopyCount
                push        ebp
                mov         ebp, eax
                
                // mm0 = samData0x26_dwFIRStackPosition
                // mm7 = pf32Source
                .align 16                
            _loop_sample:
                sub         ecx, 32
                mov         edx, 192*16
                and         ecx, 0xFFFF
                movd        esi, mm7
        
                //Send 8 channels to the stack
                movaps      xmm0, [esi]                                     //load [c3][c2][fR][fL]
                movaps      xmm1, [esi+16]                                  //load [c7][c6][c5][c4]
                movaps      samData0x26_f32FIRStackValue[ecx   ], xmm0      //write to stack
                movaps      samData0x26_f32FIRStackValue[ecx+16], xmm1      //write to stack
                
                //Set output channels
                movaps      xmm6, xmm0
                xorps       xmm4, xmm4
                
                //jmp         _loop_channel_and_render

                //.align 16                        
            _loop_channel_and_render:

                mov         eax, ecx
                mov         ebx, ecx
                mov         edi, ecx
                mov         esi, ecx
                
                add         eax, samData0x26_dwIndexValue_MuxedLR[edx   -16]
                add         ebx, samData0x26_dwIndexValue_MuxedLR[edx+ 4-16]
                add         edi, samData0x26_dwIndexValue_MuxedLR[edx+ 8-16]
                add         esi, samData0x26_dwIndexValue_MuxedLR[edx+12-16]
                
                and         eax, 65535
                and         ebx, 65535
                and         edi, 65535
                and         esi, 65535
                
                movss       xmm0, samData0x26_f32FIRStackValue[eax]
                movss       xmm1, samData0x26_f32FIRStackValue[ebx]
                movss       xmm2, samData0x26_f32FIRStackValue[edi]
                movss       xmm3, samData0x26_f32FIRStackValue[esi]

                movlhps     xmm0, xmm1                                  //xmm0 = [---][ebx][---][eax]
                movlhps     xmm2, xmm3                                  //xmm1 = [---][esi][---][edi]
                
                shufps      xmm0, xmm2, 10001000b

                mulps       xmm0, samData0x26_f32MulValue_MuxedLR[edx-16]  // load [mul_R1][mul_L1][mul_R0][mul_L0]
                
                addps       xmm4, xmm0
                
                sub         edx, 16
                jnz         _loop_channel_and_render
                
                                            // xmm4 = [R1][L1][R0][L0]
                movhlps     xmm0, xmm4      // xmm0 = [--][--][R1][L1]
                
                addps       xmm4, xmm0      // xmm4 = [--][--][R ][L ]
                addps       xmm6, xmm4
                
                
                movd        esi, mm7
                mulps       xmm6, xmm5
                dec         ebp
                paddq       mm7, mm4
                movaps      [esi], xmm6
                
                jnz         _loop_sample
                
                pop         ebp                
                mov         samData0x26_dwFIRStackPosition, ecx
                movd        pf32Source, mm7
                
                emms
                
        }

    //0,1 à 0,4%
    IKALIZER_Limiter_Stereo ( 
        pf32SourceData, 
        pi16TargetData, 
        lSamplesCopyCount, 
        lTargetOffset, 
        lChannelCount, 
        lLimiterMode );

}
        
        
    
/*
    for (lSampleIndex=0;lSampleIndex<lSamplesCopyCount;lSampleIndex++)
    {
        //Décalage du pointeur
        samData0x26_dwFIRStackPosition = (samData0x26_dwFIRStackPosition+1)&2047;
        
        __asm {
                mov         eax, samData0x26_dwFIRStackPosition
                add         eax, 32
                and         eax, 2047*32
                mov         samData0x26_dwFIRStackPosition, eax
                
                mov         esi, pf32Source
                
                //Send 8 channels to the stack
                movaps      xmm0, [esi]                                     //Charge [c3][c2][fR][fL]
                movaps      xmm1, [esi+16]                                  //Charge [c7][c6][c5][c4]
                movaps      samData0x26_f32FIRStackValue[eax], xmm0
                movaps      samData0x26_f32FIRStackValue[eax+16], xmm1
                
                mov         cl, 127
                
            _loop_render:
                
                movaps      xmm2, samData0x26_f32MulValue[edx   ]
                movaps      xmm3, samData0x26_f32MulValue[edx+16]
                
                mov         eax, samData0x26_dwFIRStackPosition
                mov         ebx, samData0x26_dwFIRStackPosition
                sub         eax, samData0x26_dwIndexValue[edx   ]
                sub         ebx, samData0x26_dwIndexValue[edx+ 4]
                
                lFIRStackPosition-samData0x26_dwIndexValue[lOffset  ]

                mov         eax, lFIRStackPosition
                mov         ebx, lFIRStackPosition
                mov         ecx, lFIRStackPosition
                mov         edx, lFIRStackPosition
                sub         eax, [esi       +SAM_RENDER254.dwDelayIndex]
                sub         ebx, [esi+4     +SAM_RENDER254.dwDelayIndex]
                sub         ecx, [esi  +2048+SAM_RENDER254.dwDelayIndex]
                sub         edx, [esi+4+2048+SAM_RENDER254.dwDelayIndex]
                and         eax, 2047
                and         ebx, 2047
                and         ecx, 2047
                and         edx, 2047
                
                

        //Lecture des canaux direct to out
        fL = pf32Source[0];
        fR = pf32Source[1];
    
        //Lecture des canaux d'origine et vidage de la source
        for (i=0;i<8;i++)
        {
            
            fFIRStackValue[i][lFIRStackPosition] = pf32Source[i+2];
            pf32Source[i+2] = 0;
        }
        
        //Génération du rendu
        for (c=0;c<6;c++)
        {
            for (i=0;i<__0x26__Render254[c].dwEntryCount;i+=2)
            {
                fL += fFIRStackValue[c][(lFIRStackPosition-__0x26__Render254[c].dwDelayIndex[i])&2047] * 
                    __0x26__Render254[c].fDelayGain[i];
                    
                fR += fFIRStackValue[c][(lFIRStackPosition-__0x26__Render254[c].dwDelayIndex[i+1])&2047] * 
                    __0x26__Render254[c].fDelayGain[i+1];
            }
        }
        
        //Matriçage de la sortie
        pf32Source[0] = fL * fLimiterGain;
        pf32Source[1] = fR * fLimiterGain;
        pf32Source+=8;

    }


/*
    lOffset = 0;
    for (i=0;i<127;i++)
    {
        for (c=0;c<6;c++)
        {
            fL += samData0x26_f32MulValue[lOffset  ] *
                samData0x26_f32FIRStackValue[(lFIRStackPosition-samData0x26_dwIndexValue[lOffset  ])&2047];
                
            fR += samData0x26_f32MulValue[lOffset+1] *
                samData0x26_f32FIRStackValue[(lFIRStackPosition-samData0x26_dwIndexValue[lOffset+1])&2047];
                
            lOffset+=2;    
        }
    }

        for (c=0;c<6;c++)
        {
            for (i=0;i<__0x26__Render254[c].dwEntryCount;i+=2)
            {
                fL += fFIRStackValue[c][(lFIRStackPosition-__0x26__Render254[c].dwDelayIndex[i])&2047] * 
                    __0x26__Render254[c].fDelayGain[i];
                    
                fR += fFIRStackValue[c][(lFIRStackPosition-__0x26__Render254[c].dwDelayIndex[i+1])&2047] * 
                    __0x26__Render254[c].fDelayGain[i+1];
            }
        }
*/

void SAM_PostProcessCopy_0x26_lastok ( FLOAT32 * pf32SourceData, INT16 * pi16TargetData, long lSamplesCopyCount, long lTargetOffset, long lChannelCount, long lLimiterMode )
{
    long lChannelIndex;
    long lSampleIndex;
    FLOAT32 * pf32Source;
    INT16 * pi16Target;

    float   fLimiterGain;

    long    lAbsValue;
    long    lPeakValue;
    long    lDataValue;

    long    lTargetIndex;
    long    lTargetMax;

    long    lChannelCopyCount;
    long    lTargetSamplesCount;

    long    *plLimiterConvertTodB_8_24;
    long    lPeakLevel;
    long    lLimiterPeakLevel;
    long    lLimiterGain;
    long    lLimiterGainMax;
    long    lDeltaLevelToSat;
    long    lLimiterDelayBeforeGrowing;

                long    lTemp, i, j, c, n;

                float   fNoiseMul;
    static      float   fNoise, fNoiseLast, fNoiseFiltered;
    static      DWORD   dwNoise = 0;
                float   fL, fR, fV;
                float   fChannel[2];
                
    static      DWORD dwMMX_MaskStack[2] = { 65535, 65535 }; //2047*32, 2047*32 };                
    static      DWORD dwMMX_32 = 32;
    static      DWORD dwMMX_2047m32 = 2047 * 32;
    static      float f32SSE_LimiterGain[4];

    #ifdef sam_OUTPUTFILE
    long    lCopyTarget;
    lCopyTarget = 0;
    #endif

    lTargetSamplesCount = (long)samData.dwHardwareAndSoftwareBufferSamplesCount;
    lChannelCopyCount   = lChannelCount; //(long)samData.dwHardwaremixChannelsCount;
    lTargetIndex        = lChannelCopyCount * lTargetOffset;
    lTargetMax          = lChannelCopyCount * lTargetSamplesCount;


    pf32Source          = pf32SourceData;
    pi16Target          = pi16TargetData;
    
    switch (lLimiterMode)
    {
        case 0: //0dB
            fLimiterGain        = 1.0;
            break;

        case 1: //+6dB
            fLimiterGain        = 2.0;
            break;

        case 2: //+12dB
            fLimiterGain        = 4.0;
            break;

        case 3: //+20dB                 
            fLimiterGain        = 10.0;
            break;
    }

    f32SSE_LimiterGain[0] = fLimiterGain;
    f32SSE_LimiterGain[1] = fLimiterGain;
    f32SSE_LimiterGain[2] = 0;
    f32SSE_LimiterGain[3] = 0;
    
    
    if (samData0x26_f32FIRStackValue[0]>0)
    {
        lSampleIndex = 0;
    }
        
        __asm {
        
                mov         eax, lSamplesCopyCount
                mov         lSampleIndex, eax
                movd        mm0, samData0x26_dwFIRStackPosition
                movq        mm1, dwMMX_MaskStack
                movd        mm4, dwMMX_32
                movd        mm5, dwMMX_2047m32
                movd        mm7, pf32Source
                
                // mm0 = samData0x26_dwFIRStackPosition
                // mm7 = pf32Source
                
            _loop_sample:
                psubd       mm0, mm4
                pand        mm0, mm5
                movd        edi, mm0
                
                movd        esi, mm7
        
                //Send 8 channels to the stack
                movaps      xmm0, [esi]                                     //load [c3][c2][fR][fL]
                movaps      xmm1, [esi+16]                                  //load [c7][c6][c5][c4]
                movaps      samData0x26_f32FIRStackValue[edi   ], xmm0      //write to stack
                movaps      samData0x26_f32FIRStackValue[edi+16], xmm1      //write to stack
                
                //Set output channels
                movaps      xmm6, xmm0
                xorps       xmm4, xmm4
                      
                //movss       xmm6, xmm0
                //movaps      xmm7, xmm0
                //xorps       xmm4, xmm4
                //shufps      xmm7, xmm7, 01010101b
                
        
                xor         edx, edx
                
                punpckldq   mm0, mm0
                
                
                
                mov         ch, 6 //6 channels to process
                
            _loop_channel:
                
                mov         cl, 32 //32 double render to process
                        
            _loop_render:
            
                movd        eax, mm0
                movd        ebx, mm0
                mov         edi, eax
                mov         esi, ebx
                
                add         eax, samData0x26_dwIndexValue_MuxedLR[edx   ]
                add         ebx, samData0x26_dwIndexValue_MuxedLR[edx+ 4]
                add         edi, samData0x26_dwIndexValue_MuxedLR[edx+ 8]
                add         esi, samData0x26_dwIndexValue_MuxedLR[edx+12]
                
                and         eax, 65535
                and         ebx, 65535
                and         edi, 65535
                and         esi, 65535
                
                //movq        mm2, mm0                                        //load Stack Position
                //movq        mm3, mm0
                //paddd       mm2, samData0x26_dwIndexValue_MuxedLR[edx  ]
                //paddd       mm3, samData0x26_dwIndexValue_MuxedLR[edx+8]
                //pand        mm2, mm1
                //pand        mm3, mm1
                
                /*
                mov         eax, samData0x26_dwFIRStackPosition
                mov         ebx, samData0x26_dwFIRStackPosition
                
                add         eax, samData0x26_dwIndexValue_MuxedLR[edx  ]
                add         ebx, samData0x26_dwIndexValue_MuxedLR[edx+4]
                
                and         eax, 2047*32
                and         ebx, 2047*32
                */
                
                //movd        eax, mm2
                //movd        ebx, mm3
                
                /*
                // 4x ( L=4 ; T=2 )
                movss       xmm2, samData0x26_f32MulValue_MuxedLR[edx   ]  // load mul_L0
                movss       xmm3, samData0x26_f32MulValue_MuxedLR[edx+ 4]  // load mul_R0
                movss       xmm4, samData0x26_f32MulValue_MuxedLR[edx+ 8]  // load mul_L1
                movss       xmm5, samData0x26_f32MulValue_MuxedLR[edx+12]  // load mul_R1
                
                // 4x ( L=7 ; T=2 )
                mulss       xmm2, samData0x26_f32FIRStackValue[eax]
                mulss       xmm3, samData0x26_f32FIRStackValue[ebx]
                mulss       xmm4, samData0x26_f32FIRStackValue[edi]
                mulss       xmm5, samData0x26_f32FIRStackValue[esi]
                
                // 4x ( L=5 ; T=2 )
                addss       xmm6, xmm2
                addss       xmm6, xmm4
                addss       xmm7, xmm3
                addss       xmm7, xmm5
                */
                
                // 4x ( L=4 ; T=2 )
                movss       xmm0, samData0x26_f32FIRStackValue[eax]
                movss       xmm1, samData0x26_f32FIRStackValue[ebx]
                movss       xmm2, samData0x26_f32FIRStackValue[edi]
                movss       xmm3, samData0x26_f32FIRStackValue[esi]

                // 2x ( L=4 ; T=2 )                
                movlhps     xmm0, xmm1                                  //xmm0 = [---][ebx][---][eax]
                movlhps     xmm2, xmm3                                  //xmm1 = [---][esi][---][edi]
                
                // 1x ( L=6 ; T=2 )
                shufps      xmm0, xmm2, 10001000b

                // 1x ( L=7 ; T=2 )               
                mulps       xmm0, samData0x26_f32MulValue_MuxedLR[edx]  // load [mul_R1][mul_L1][mul_R0][mul_L0]
                
                // 1x ( L=5 ; T=2 )
                addps       xmm4, xmm0
                
                //mulss       xmm2, samData0x26_f32FIRStackValue[eax]
                //mulss       xmm4, samData0x26_f32FIRStackValue[ebx]
                
                //punpckhdq   mm2, mm2
                //punpckhdq   mm3, mm3
                //movd        eax, mm2
                //movd        ebx, mm3
                
                //addss       xmm6, xmm2
                //addss       xmm6, xmm4

                //mulss       xmm3, samData0x26_f32FIRStackValue[eax]
                //mulss       xmm5, samData0x26_f32FIRStackValue[ebx]
                
                //addss       xmm7, xmm3
                //addss       xmm7, xmm5
                
                add         edx, 16
                
                dec         cl
                jnz         _loop_render
                
                dec         ch
                jnz         _loop_channel
                
                                            // xmm4 = [R1][L1][R0][L0]
                movhlps     xmm5, xmm4      // xmm5 = [--][--][R1][L1]
                addps       xmm4, xmm5      // xmm4 = [--][--][R ][L ]
                addps       xmm6, xmm4
                
                //addss       xmm6, xmm4
                
                
                
                movd        esi, mm7
                
                /*
                mulss       xmm6, fLimiterGain
                mulss       xmm7, fLimiterGain
                movss       [esi], xmm6
                movss       [esi+4], xmm7
                */
                mulps       xmm6, f32SSE_LimiterGain
                movlps      [esi], xmm6
                
                paddq       mm7, mm4
            
                dec         lSampleIndex
                jnz         _loop_sample
                
                movd        samData0x26_dwFIRStackPosition, mm0
                movd        pf32Source, mm7
                
                emms
                
        }                

    IKALIZER_Limiter_Stereo ( 
        pf32SourceData, 
        pi16TargetData, 
        lSamplesCopyCount, 
        lTargetOffset, 
        lChannelCount, 
        lLimiterMode );
        
}

//#define _RENDER_WITH_MMX
void SAM_PostProcessCopy_0x26_ok3 ( FLOAT32 * pf32SourceData, INT16 * pi16TargetData, long lSamplesCopyCount, long lTargetOffset, long lChannelCount, long lLimiterMode )
{
    long lChannelIndex;
    long lSampleIndex;
    FLOAT32 * pf32Source;
    INT16 * pi16Target;

    float   fLimiterGain;

    long    lAbsValue;
    long    lPeakValue;
    long    lDataValue;

    long    lTargetIndex;
    long    lTargetMax;

    long    lChannelCopyCount;
    long    lTargetSamplesCount;

    long    *plLimiterConvertTodB_8_24;
    long    lPeakLevel;
    long    lLimiterPeakLevel;
    long    lLimiterGain;
    long    lLimiterGainMax;
    long    lDeltaLevelToSat;
    long    lLimiterDelayBeforeGrowing;

                long    lTemp, i, j, c, n;

                float   fNoiseMul;
    static      float   fNoise, fNoiseLast, fNoiseFiltered;
    static      DWORD   dwNoise = 0;
                float   fL, fR, fV;
                float   fChannel[2];
                
    static      DWORD dwMMX_MaskStack[2] = { 65535, 65535 }; //2047*32, 2047*32 };                
    static      DWORD dwMMX_32 = 32;
    static      DWORD dwMMX_2047m32 = 2047 * 32;
    static      float f32SSE_LimiterGain[4];

    #ifdef sam_OUTPUTFILE
    long    lCopyTarget;
    lCopyTarget = 0;
    #endif

    lTargetSamplesCount = (long)samData.dwHardwareAndSoftwareBufferSamplesCount;
    lChannelCopyCount   = lChannelCount; //(long)samData.dwHardwaremixChannelsCount;
    lTargetIndex        = lChannelCopyCount * lTargetOffset;
    lTargetMax          = lChannelCopyCount * lTargetSamplesCount;


    pf32Source          = pf32SourceData;
    pi16Target          = pi16TargetData;
    
    switch (lLimiterMode)
    {
        case 0: //0dB
            fLimiterGain        = 1.0;
            break;

        case 1: //+6dB
            fLimiterGain        = 2.0;
            break;

        case 2: //+12dB
            fLimiterGain        = 4.0;
            break;

        case 3: //+20dB                 
            fLimiterGain        = 10.0;
            break;
    }

    f32SSE_LimiterGain[0] = fLimiterGain;
    f32SSE_LimiterGain[1] = fLimiterGain;
    f32SSE_LimiterGain[2] = 0;
    f32SSE_LimiterGain[3] = 0;
    
        __asm {
        
                mov         eax, lSamplesCopyCount
                mov         lSampleIndex, eax
                movd        mm0, samData0x26_dwFIRStackPosition
                movq        mm1, dwMMX_MaskStack
                movd        mm4, dwMMX_32
                movd        mm5, dwMMX_2047m32
                movd        mm7, pf32Source
                
                // mm0 = samData0x26_dwFIRStackPosition
                // mm7 = pf32Source
                
            _loop_sample:
#ifdef _RENDER_WITH_MMX            
                psubw       mm0, mm4
                movd        esi, mm7
                movd        edi, mm0
                xor         edx, edx
                and         edi, 0xFFFF
                pshufw      mm0, mm0, 00000000b
#else
                psubd       mm0, mm4
                pand        mm0, mm5
                xor         edx, edx
                movd        edi, mm0
                movd        esi, mm7
                punpckldq   mm0, mm0
#endif
        
                //Send 8 channels to the stack
                movaps      xmm0, [esi]                                     //load [c3][c2][fR][fL]
                movaps      xmm1, [esi+16]                                  //load [c7][c6][c5][c4]
                movaps      samData0x26_f32FIRStackValue[edi   ], xmm0      //write to stack
                movaps      samData0x26_f32FIRStackValue[edi+16], xmm1      //write to stack
                
                //Set output channels
                movaps      xmm6, xmm0
                xorps       xmm4, xmm4
                
                
                
                //mov         ch, 6 //6 channels to process
                //mov         cl, 32 //32 double render to process
                //xor         cl, cl
                mov         cl, 192 //0x0600
                
                //jmp         _loop_channel_and_render
                
            
                
                

.align 16                        
            _loop_channel_and_render:

#ifdef _RENDER_WITH_MMX            
                movq        mm2, mm0
                paddw       mm2, samData0x26_wIndexValue_MuxedLR[edx]
                
                movd        eax, mm2
                psrlq       mm2, 32
                movd        edi, mm2
                
                mov         ebx, eax
                mov         esi, edi
                shr         ebx, 16
                and         eax, 0xFFFF
                shr         esi, 16
                and         edi, 0xFFFF
                
#else                
                movd        eax, mm0
                movd        ebx, mm0
                mov         edi, eax
                mov         esi, ebx
                
                add         eax, samData0x26_dwIndexValue_MuxedLR[edx   ]
                add         ebx, samData0x26_dwIndexValue_MuxedLR[edx+ 4]
                add         edi, samData0x26_dwIndexValue_MuxedLR[edx+ 8]
                add         esi, samData0x26_dwIndexValue_MuxedLR[edx+12]
                
                and         eax, 65535
                and         ebx, 65535
                and         edi, 65535
                and         esi, 65535
#endif
                
                // 4x ( L=4 ; T=2 )
                movss       xmm0, samData0x26_f32FIRStackValue[eax]
                movss       xmm1, samData0x26_f32FIRStackValue[ebx]
                movss       xmm2, samData0x26_f32FIRStackValue[edi]
                movss       xmm3, samData0x26_f32FIRStackValue[esi]

                // 2x ( L=4 ; T=2 )                
                movlhps     xmm0, xmm1                                  //xmm0 = [---][ebx][---][eax]
                movlhps     xmm2, xmm3                                  //xmm1 = [---][esi][---][edi]
                
                // 1x ( L=6 ; T=2 )
                shufps      xmm0, xmm2, 10001000b

                // 1x ( L=7 ; T=2 )               
#ifdef _RENDER_WITH_MMX                
                mulps       xmm0, samData0x26_f32MulValue_MuxedLR[edx*2-16]  // load [mul_R1][mul_L1][mul_R0][mul_L0]
                
                add         edx, 8
                dec         cl
                
                // 1x ( L=5 ; T=2 )
                addps       xmm4, xmm0                
#else
                mulps       xmm0, samData0x26_f32MulValue_MuxedLR[edx]  // load [mul_R1][mul_L1][mul_R0][mul_L0]
                
                add         edx, 16
                dec         cl
                
                // 1x ( L=5 ; T=2 )
                addps       xmm4, xmm0
#endif
                
                
                
                //jnz         _loop_render
                
                //mov         cl, 32 //32 double render to process
                //dec         ch
                
                jnz         _loop_channel_and_render
                
                                            // xmm4 = [R1][L1][R0][L0]
                movhlps     xmm5, xmm4      // xmm5 = [--][--][R1][L1]
                addps       xmm4, xmm5      // xmm4 = [--][--][R ][L ]
                addps       xmm6, xmm4
                
                //addss       xmm6, xmm4
                
                
                
                movd        esi, mm7
                mulps       xmm6, f32SSE_LimiterGain
                movaps      [esi], xmm6
                paddq       mm7, mm4
            
                dec         lSampleIndex
                jnz         _loop_sample
                
                movd        samData0x26_dwFIRStackPosition, mm0
                movd        pf32Source, mm7
                
                emms
                
        }                

    IKALIZER_Limiter_Stereo ( 
        pf32SourceData, 
        pi16TargetData, 
        lSamplesCopyCount, 
        lTargetOffset, 
        lChannelCount, 
        lLimiterMode );
        
}

void SAM_PostProcessCopy_0x26_old ( FLOAT32 * pf32SourceData, INT16 * pi16TargetData, long lSamplesCopyCount, long lTargetOffset, long lChannelCount, long lLimiterMode )
{
    long lChannelIndex;
    long lSampleIndex;
    FLOAT32 * pf32Source;
    INT16 * pi16Target;

    float   fLimiterGain;

    long    lAbsValue;
    long    lPeakValue;
    long    lDataValue;

    long    lTargetIndex;
    long    lTargetMax;

    long    lChannelCopyCount;
    long    lTargetSamplesCount;

    long    *plLimiterConvertTodB_8_24;
    long    lPeakLevel;
    long    lLimiterPeakLevel;
    long    lLimiterGain;
    long    lLimiterGainMax;
    long    lDeltaLevelToSat;
    long    lLimiterDelayBeforeGrowing;

                long    lTemp, i, j, c, n;

                float   fNoiseMul;
    static      float   fNoise, fNoiseLast, fNoiseFiltered;
    static      DWORD   dwNoise = 0;
                float   fL, fR, fV;
                float   fChannel[2];

    #ifdef sam_OUTPUTFILE
    long    lCopyTarget;
    lCopyTarget = 0;
    #endif

    lTargetSamplesCount = (long)samData.dwHardwareAndSoftwareBufferSamplesCount;
    lChannelCopyCount   = lChannelCount; //(long)samData.dwHardwaremixChannelsCount;
    lTargetIndex        = lChannelCopyCount * lTargetOffset;
    lTargetMax          = lChannelCopyCount * lTargetSamplesCount;


    pf32Source          = pf32SourceData;
    pi16Target          = pi16TargetData;


    switch (lLimiterMode)
    {
        case 0: //0dB
            fLimiterGain        = 1.0;
            break;

        case 1: //+6dB
            fLimiterGain        = 2.0;
            break;

        case 2: //+12dB
            fLimiterGain        = 4.0;
            break;

        case 3: //+20dB                 
            fLimiterGain        = 10.0;
            break;
    }

    /*
    for (lSampleIndex=0;lSampleIndex<lSamplesCopyCount;lSampleIndex++)
    {
        //Décalage du pointeur
        lFIRStackPosition = (lFIRStackPosition+1)&2047;

        //Lecture des canaux direct to out
        fL = pf32Source[0];
        fR = pf32Source[1];
        pf32Source[0] = 0;
        pf32Source[1] = 0;        
    
        //Lecture des canaux d'origine et vidage de la source
        for (i=0;i<6;i++)
        {
            fFIRStackValue[i][lFIRStackPosition] = pf32Source[i+2];
            pf32Source[i+2] = 0;
        }
        
        //Génération du rendu
        for (c=0;c<6;c++)
        {
            for (i=0;i<__0x26__Render254[c].dwEntryCount;i+=2)
            {
                fL += fFIRStackValue[c][(lFIRStackPosition-__0x26__Render254[c].dwDelayIndex[i])&2047] * 
                    __0x26__Render254[c].fDelayGain[i];
                    
                fR += fFIRStackValue[c][(lFIRStackPosition-__0x26__Render254[c].dwDelayIndex[i+1])&2047] * 
                    __0x26__Render254[c].fDelayGain[i+1];
            }
        }
        
        //Matriçage de la sortie
        pf32Source[0] = fL * fLimiterGain;
        pf32Source[1] = fR * fLimiterGain;
        pf32Source+=8;

    }
    */
    
    /*
    __asm {
                mov         eax, lSamplesCopyCount
                mov         lSampleIndex, eax
            _loop:
                mov         eax, lFIRStackPosition
                mov         esi, pf32Source
                inc         eax
                and         eax, 2047
                mov         lFIRStackPosition, eax
                
                movss       xmm0, [esi]
                movss       xmm1, [esi+4]
                movss       xmm2, [esi+8]
                movss       xmm3, [esi+12]
                movss       xmm4, [esi+16]
                movss       xmm5, [esi+20]
                movss       xmm6, [esi+24]
                movss       xmm7, [esi+28]
                
                movss       samData_f32FIRStackValue[eax*4      ], xmm2
                movss       samData_f32FIRStackValue[eax*4+ 8192], xmm3
                movss       samData_f32FIRStackValue[eax*4+16384], xmm4
                movss       samData_f32FIRStackValue[eax*4+24576], xmm5
                movss       samData_f32FIRStackValue[eax*4+32768], xmm6
                movss       samData_f32FIRStackValue[eax*4+40960], xmm7
                
                /*
                movaps      xmm0, [esi]         //Charge [c3][c2][fR][fL]
                movss       xmm1, [esi+4]       //Charge             [fR]
                movaps      xmm2, [esi+16]      //Charge [c7][c6][c5][c4]

                shl         eax, 5                
                movaps      samData_f32FIRStackValue[eax], xmm0
                movaps      samData_f32FIRStackValue[eax+16], xmm2
                
                
                //
                lea         edi, samData_f32FIRStackValue
                lea         edx, __0x26__Render254
                //add         edi, 4
                mov         ch, 6
            _loop_channel:    
                mov         esi, edx
                add         edx, 2048
                mov         cl, BYTE PTR [esi+SAM_RENDER254.dwEntryCount] 
            _loop_render:
                
                mov         eax, lFIRStackPosition
                mov         ebx, lFIRStackPosition
                mov         ecx, lFIRStackPosition
                mov         edx, lFIRStackPosition
                sub         eax, [esi       +SAM_RENDER254.dwDelayIndex]
                sub         ebx, [esi+4     +SAM_RENDER254.dwDelayIndex]
                sub         ecx, [esi  +2048+SAM_RENDER254.dwDelayIndex]
                sub         edx, [esi+4+2048+SAM_RENDER254.dwDelayIndex]
                and         eax, 2047
                and         ebx, 2047
                and         ecx, 2047
                and         edx, 2047
                
                movss       xmm2, [esi       +SAM_RENDER254.fDelayGain] // Ch2 to L
                movss       xmm3, [esi+4     +SAM_RENDER254.fDelayGain] // Ch2 to R
                movss       xmm4, [esi  +2048+SAM_RENDER254.fDelayGain] // Ch3 to L
                movss       xmm5, [esi+4+2048+SAM_RENDER254.fDelayGain] // Ch3 to R
                movss       xmm6, [esi  +4096+SAM_RENDER254.fDelayGain] // Ch4 to L
                movss       xmm7, [esi+4+4096+SAM_RENDER254.fDelayGain] // Ch4 to R
                
                mulss       xmm2, [edi+eax*4]
                mulss       xmm3, [edi+ebx*4]
                mulss       xmm4, [edi+ecx*4+8192]
                mulss       xmm5, [edi+edx*4+8192]
                
                addss       xmm0, xmm2
                addss       xmm1, xmm3
                addss       xmm0, xmm4
                addss       xmm1, xmm5
                
                mov         eax, lFIRStackPosition
                mov         ebx, lFIRStackPosition
                mov         ecx, lFIRStackPosition
                mov         edx, lFIRStackPosition
                sub         eax, [esi  +4096+SAM_RENDER254.dwDelayIndex]
                sub         ebx, [esi+4+4096+SAM_RENDER254.dwDelayIndex]
                sub         ecx, [esi  +6144+SAM_RENDER254.dwDelayIndex]
                sub         edx, [esi+4+6144+SAM_RENDER254.dwDelayIndex]
                and         eax, 2047
                and         ebx, 2047
                and         ecx, 2047
                and         edx, 2047
                
                mulss       xmm5, [edi+eax*4+16384]
                mulss       xmm6, [edi+ebx*4+16384]
                
                
                
                add         esi, 8
                
                sub         cl, 2
                jnz         _loop_render
                
                //add         edi, 4
                add         edi, 8192
                dec         ch
                jnz         _loop_channel
                
                
                /*
                //OK!!!
                lea         edi, samData_f32FIRStackValue
                lea         edx, __0x26__Render254
                //add         edi, 4
                mov         ch, 6
            _loop_channel:    
                mov         esi, edx
                add         edx, 2048
                mov         cl, BYTE PTR [esi+SAM_RENDER254.dwEntryCount] 
            _loop_render:
                
                mov         eax, lFIRStackPosition
                mov         ebx, lFIRStackPosition
                sub         eax, [esi  +SAM_RENDER254.dwDelayIndex]
                sub         ebx, [esi+4+SAM_RENDER254.dwDelayIndex]
                and         eax, 2047
                and         ebx, 2047
                
                movss       xmm2, [esi  +SAM_RENDER254.fDelayGain]
                movss       xmm3, [esi+4+SAM_RENDER254.fDelayGain]
                
                //shl         eax, 5
                //shl         ebx, 5
                mulss       xmm2, [edi+eax*4]
                mulss       xmm3, [edi+ebx*4]
                
                addss       xmm0, xmm2
                addss       xmm1, xmm3
                
                add         esi, 8
                
                sub         cl, 2
                jnz         _loop_render
                
                //add         edi, 4
                add         edi, 8192
                dec         ch
                jnz         _loop_channel
                */
                
                /*
                mov         esi, pf32Source
                mulss       xmm0, fLimiterGain
                mulss       xmm1, fLimiterGain
                movss       [esi], xmm0
                movss       [esi+4], xmm1
                add         esi, 32
                mov         pf32Source, esi
                 
            
            
                dec         lSampleIndex
                jnz         _loop
    
    
    }
    */

    IKALIZER_Limiter_Stereo ( 
        pf32SourceData, 
        pi16TargetData, 
        lSamplesCopyCount, 
        lTargetOffset, 
        lChannelCount, 
        lLimiterMode );
        
}


