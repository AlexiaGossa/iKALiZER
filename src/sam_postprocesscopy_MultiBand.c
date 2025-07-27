#include "sam_header.h"
#include "sam_data.h"



    



/*
float Q_fabs( float f ) {
	int tmp = * ( int * ) &f;
	tmp &= 0x7FFFFFFF;
	return * ( float * ) &tmp;
}
*/

void IKA_MakeNoise ( float * pfData, long lLength, long lSeed, float fLevel );

long IKALIZER_Limiter_Stereo_Init=0;
float IKALIZER_Limiter_Stereo_fNoise[8192];

void IKALIZER_Limiter_Stereo ( FLOAT32 * pf32SourceData, INT16 * pi16TargetData, long lSamplesCopyCount, long lTargetOffset, long lChannelCount, long lLimiterMode )
{
    long lSampleIndex;
    long lChannelIndex;
    float fTemp;
    long lDataValue;
    FLOAT32 *pf32Source;
    INT16   *pi16Target;
    long    lChannelCopyCount;
    long    lTargetIndex;
    long    lTargetMax;
    long    lTargetSamplesCount;
    float   fAbsL;
    float   fAbsR;
    float   f1,f2;
    
    float   fReleaseTime;
    float   fGainTime;    
    float   fThreshold;
    
    static float    fPeakGlobal;
    static float    fGainGlobal;
    static long     lIndexNoise;
    static long     *plNoise;


    //Initialisation    
    if (!IKALIZER_Limiter_Stereo_Init)
    {
        IKALIZER_Limiter_Stereo_Init = 1;
        fPeakGlobal = 0;
        fGainGlobal = 32760.0F;
        
        IKA_MakeNoise ( IKALIZER_Limiter_Stereo_fNoise, 8192, 0, 3.3F );
        plNoise = (long *)IKALIZER_Limiter_Stereo_fNoise;
            
        lIndexNoise = 0;
    }

    //Calc release time at 100ms (0.1s)    
    //  R = pow10 ( log10 ( 0.37 ) / ( SamplingRate * ReleaseTime ) )
    fReleaseTime = (float)pow ( 10, log10 ( 0.37F ) / ((float)samData.dwHardwaremixSampleRate * 0.030F) );
    
    //Calc gain time
    fGainTime    = (float)pow ( 10, -log10 ( 0.37F ) / ((float)samData.dwHardwaremixSampleRate * 0.020F) );
    
    //Threshold level 
    fThreshold = 0.90F * 32760.0F; //-0.44dB
    
    lTargetSamplesCount = (long)samData.dwHardwareAndSoftwareBufferSamplesCount;
    pf32Source          = pf32SourceData;
    pi16Target          = pi16TargetData;
    lChannelCopyCount   = lChannelCount; //(long)samData.dwHardwaremixChannelsCount;    
    lTargetIndex        = lChannelCopyCount * lTargetOffset;
    lTargetMax          = lChannelCopyCount * lTargetSamplesCount;

    //Correction pour la sortie 16 bits    
    for (lSampleIndex=0;lSampleIndex<lSamplesCopyCount;lSampleIndex++)
    {
        //Get peak value
        fAbsL = sam_ABS ( pf32Source[0] );
        fAbsR = sam_ABS ( pf32Source[1] );
        fPeakGlobal *= fReleaseTime;
        if (fAbsL>fPeakGlobal) fPeakGlobal = fAbsL;
        if (fAbsR>fPeakGlobal) fPeakGlobal = fAbsR;
        
        //Process gain
        f1 = fPeakGlobal * fGainGlobal;
        if (f1>fThreshold) fGainGlobal = fThreshold / fPeakGlobal;
        else fGainGlobal *= fGainTime; //1.001F;

        //Correct gain
        if (fGainGlobal>32760.0F) fGainGlobal = 32760.0F;
        if (fGainGlobal<32.7600F) fGainGlobal = 32.7600F;
        
        //Noise Generator
        lIndexNoise = (lIndexNoise+1)&8191;

        //Etape 7 : Application du gain final pour tous les canaux
        for (lChannelIndex=0;lChannelIndex<lChannelCopyCount;lChannelIndex++)
        {
            fTemp = ( pf32Source[lChannelIndex] * fGainGlobal ) + IKALIZER_Limiter_Stereo_fNoise[lIndexNoise];
            lDataValue = (long)(fTemp);
            //if (lDataValue> 32760) lDataValue =  32760;
            //if (lDataValue<-32760) lDataValue = -32760;
            
            //Ecriture sur la sortie
            pi16Target[lTargetIndex] = (INT16)lDataValue;
            lTargetIndex++;
            
            
        }
            

        //Next sample bloc
        pf32Source[0] = 0;
        pf32Source[1] = 0;
        pf32Source[2] = 0;
        pf32Source[3] = 0;
        pf32Source[4] = 0;
        pf32Source[5] = 0;
        pf32Source[6] = 0;
        pf32Source[7] = 0;
                   
        pf32Source+=8;

            
        //Loop end position to start...
        if (lTargetIndex>=lTargetMax)
            lTargetIndex = 0;
    }
    
    
    
    //Gestion de l'index du bruit
    lIndexNoise ^= *((long *)(&fPeakGlobal));
    lIndexNoise = (long)(( 1664525 * lIndexNoise ) + 1013904223);
    lIndexNoise = ((UINT64)lIndexNoise * 279470273) % 4294967291;
    lIndexNoise = (lIndexNoise>>16)&8191;

    #ifdef sam_OUTPUTFILE
    fwrite ( pi16CopyTarget, lSamplesCopyCount, 2*lChannelCopyCount, samData.pFileDebug );
    #endif
}

    



            long    multiband_lInit = 0;
            float   multiband_fCurrentLevel;
sam_ALIGN   float   multiband_fStackValue[8][256];   //8 canaux / 256 points
            long    multiband_lStackPosition;
sam_ALIGN   float   multiband_fFilterData[4][512];   //4 filtres sur 256 points
sam_ALIGN   float   multiband_fBandValue[8][4];
sam_ALIGN   DWORD   dwAbsFloat[4] = { 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF };
            float   multiband_fBandPeak[4];
            float   multiband_fBandGain[4];
            float   multiband_fThreshold[4];
            long    multiband_lStackTotal;

            float   multiband_fLimiterValue[8]; //8 canaux
            float   multiband_fLimiterPeak;     //Le niveau crête du limiteur final
            float   multiband_fLimiterGain;
            float   multiband_fLimiterThreshold;

void SAM_PostProcessCopy_MultiBandInit ( void )
{
    float fEqualizer[4][256];
    float fFrequency[3];
    float f1, f2;
    long  lFreqIndex[3];
    long  i, j, k, lBand;
    
    if (multiband_lInit==samData.dwHardwaremixSampleRate) return;
    multiband_lInit = samData.dwHardwaremixSampleRate;
    
    //Vide les données
    multiband_fCurrentLevel = 0.0F;
    multiband_lStackPosition = 0;
    memset ( multiband_fStackValue, 0, 8 * 256 * sizeof(float) );
    memset ( multiband_fFilterData, 0, 4 * 256 * sizeof(float) );
    memset ( multiband_fBandValue, 0, 8 * 4 * sizeof(float) );
    memset ( multiband_fBandPeak, 0, 4 * sizeof(float) );
    memset ( multiband_fBandGain, 0, 4 * sizeof(float) );
    
    //Calcul des 3 fréquences charnières pour l'égalisation
    fFrequency[0] = 120.0F;
    fFrequency[1] = 1000.0F;
    fFrequency[2] = 5000.0F;
    for (i=0;i<3;i++)
    {
        f1 = fFrequency[i];
        f2 = (float)(samData.dwHardwaremixSampleRate>>1);
        f1 /= f2;
        f1 *= 256.0F;
        lFreqIndex[i] = SAM_MATH_lroundf ( f1 )+1;
    }
    
    //Vide les 4 bandes
    memset ( fEqualizer, 0, 4 * 256 * sizeof(float) );
    
    //Génération des 4 bandes
    for (i=0;i<lFreqIndex[0];i++)
        fEqualizer[0][i] = 1.0F;
    for (i=lFreqIndex[0];i<lFreqIndex[1];i++)
        fEqualizer[1][i] = 1.0F;
    for (i=lFreqIndex[1];i<lFreqIndex[2];i++)
        fEqualizer[2][i] = 1.0F;
    for (i=lFreqIndex[2];i<256;i++)
        fEqualizer[3][i] = 1.0F;
        
    //Applique un filtre dégressif pour les 3 premières bandes
    for (lBand=0;lBand<3;lBand++)
    {
        for (i=lFreqIndex[lBand]+1;i<256;i++)
        {
            f1 = (float)lFreqIndex[lBand];
            f2 = (float)i;
            f1 = (float)(log10 ( f2 / f1 ) / log10 ( 2.0F ));
            f1 = (float)pow ( 0.1666F, f1 );
            fEqualizer[lBand][i] = f1;
        }
    }
    
    //Calcule le filtre progressif pour les 3 dernières bandes
    for (lBand=1;lBand<4;lBand++)
    {
        for (i=1;i<lFreqIndex[lBand-1];i++)
        {
            f1 = (float)lFreqIndex[lBand-1];
            f2 = (float)i;
            f1 = (float)(log10 ( f1 / f2 ) / log10 ( 2.0F ));
            f1 = (float)pow ( 0.1666F, f1 );
            fEqualizer[lBand][i] = f1;
        }
    }
    
    //Compensation générale
    for (i=0;i<256;i++)
    {
        f1 = 0.0F;
        for (lBand=0;lBand<4;lBand++)
        {
            f1 += fEqualizer[lBand][i];
        }
        if ((f1>1.0F)||(f1<1.0F))
        {
            f2 = 1.0F / f1;
            
            for (lBand=0;lBand<4;lBand++)
                fEqualizer[lBand][i] *= f2;
        }
    }
    
    if (samData.dwHardwaremixSampleRate<44100) multiband_lStackTotal = 128;
    else                                       multiband_lStackTotal = 256;
        
    //Génération des 4 convolutions
    for (lBand=0;lBand<4;lBand++)
    {
        SAM_FIR_FFT_DesignEq ( 
            multiband_lStackTotal,
            multiband_fFilterData[lBand],
            256,
            fEqualizer[lBand] );
            
        memcpy ( 
            multiband_fFilterData[lBand] + multiband_lStackTotal,
            multiband_fFilterData[lBand],
            sizeof(float) * multiband_lStackTotal );
            
    }
    
    
    
    
    //Le réglage des 4 threshold (avec une forme de loudness pour privilégier les graves et les aigus
    multiband_fThreshold[0] = 0.50F;             //Bass   -6dB
    multiband_fThreshold[1] = 0.28F; //35355F;   //L-Med -14dB
    multiband_fThreshold[2] = 0.28F;             //H-Med -14dB
    multiband_fThreshold[3] = 0.50F; //17677F;   //High   -6dB
    
    multiband_fLimiterThreshold = 0.98F;         //Limite à -0.1dB
    
}

void SAM_PostProcessCopy_MultiBand ( FLOAT32 * pf32SourceData, INT16 * pi16TargetData, long lSamplesCopyCount, long lTargetOffset, long lChannelCount, long lLimiterMode )
{
                long    lChannelIndex;
                long    lSampleIndex;
                FLOAT32 *pf32Source;
                INT16   *pi16Target;

                float   fLimiterGain;

                long    lTargetIndex;
                long    lTargetMax;

                long    lChannelCopyCount;
                long    lTargetSamplesCount;

                long    lTemp;
                float   fTemp;

                float   fNoiseMul;
    static      float   fNoise, fNoiseLast, fNoiseFiltered;
    static      DWORD   dwNoise = 0;
    
                float   *pfInputValue;
                float   *pfFilterCoefA;
                float   *pfFilterCoefB;
                float   *pfFilterCoefC;
                float   *pfFilterCoefD;
                float   fSumA;
                float   fSumB;
                float   fSumC;
                float   fSumD;
    
                float   fDecay;
                float   fGain;
                float   fMax;
                long    lBand;
                long    i;
                long    lDataValue;
                

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

    /*
        De bons paramètres :
        
            27 et 0.9
            29 et 1,3
    
    */
    //Le générateur de bruit
    fNoiseMul = (float)(1 / pow ( 2, 27 )); //27 //30
    
    //Calcul la vitesse de decay
    fTemp = (float)samData.dwHardwaremixSampleRate;
    fTemp *= 0.100F; //100ms de decay
    fTemp = (float)log10 ( 0.37F ) / fTemp; // Le decay, c'est 100% vers 10%
    fDecay = (float)pow ( 10.0F, fTemp );
    
    //Calcul de la vitesse de Gain Up
    //fGainUp = 
    
    
    //Le gain de base
    switch (lLimiterMode)
    {
        case 0:
        default:
            fGain = 1.0F;
            break;

        case 1:
            fGain = 2.0F;
            break;

        case 2:
            fGain = 4.0F;
            break;

        case 3:
            fGain = 10.0F;
            break;
    }

    //Correction pour la sortie 16 bits    
    for (lSampleIndex=0;lSampleIndex<lSamplesCopyCount;lSampleIndex++)
    {
        //Générateur de bruit
        dwNoise = ((dwNoise * 1664525)+1013904223);
        fNoiseLast = ((float)dwNoise) * fNoiseMul;
        fNoise += (fNoiseLast-fNoise)*0.9F; //0.9F;
        fNoiseFiltered = fNoiseLast-fNoise;
    
        //Etape 1 : Application du decay pour la crête...
        for (lBand=0;lBand<4;lBand++)
        {
            multiband_fBandPeak[lBand] *= fDecay;
        }
        
                
        //Etape 2 : Séparation en 4 bandes et mesure du signal crête
        multiband_lStackPosition = (multiband_lStackPosition+1)&(255);
        for (lChannelIndex=0;lChannelIndex<lChannelCopyCount;lChannelIndex++)
        {
            //Ajoute la nouvelle valeur
            multiband_fStackValue[lChannelIndex][multiband_lStackPosition] = *pf32Source * fGain;
            *pf32Source = 0;
            pf32Source++;
            
            //Calcule le FIR
            fSumA           = 0;
            fSumB           = 0;
            fSumC           = 0;
            fSumD           = 0;
            i               = multiband_lStackTotal - multiband_lStackPosition;
            pfFilterCoefA   = multiband_fFilterData[0] + i;
            pfFilterCoefB   = multiband_fFilterData[1] + i;
            pfFilterCoefC   = multiband_fFilterData[2] + i;
            pfFilterCoefD   = multiband_fFilterData[3] + i;
            pfInputValue    = multiband_fStackValue[lChannelIndex];
            
            __asm {
                        /*
                        
                        //Prépare le pointeur ESI pour les filtres
                        mov         esi, multiband_fFilterData
                        mov         eax, multiband_lStackTotal
                        sub         eax, multiband_lStackPosition
                        lea         esi, [esi+eax*4]
                        
                        //Prépare le pointeur EDI pour les canaux
                        mov         edi, multiband_fStackValue
                        
                        //Nombre de canaux
                        mov         edx, lChannelCopyCount
            
                    _loop_channel:
                        xor         eax, eax
                        mov         ecx, multiband_lStackTotal
                        
                        //      multiband_fStackValue[lChannelIndex][multiband_lStackPosition] = *pf32Source * fGain;
                        //      *pf32Source = 0;
                        //      pf32Source++;
                        mov         ebx, pf32Source
                        movss       xmm4, [ebx]
                        mov         [ebx], eax
                        mulss       xmm4, fGain
                        add         ebx, 4
                        movss       multiband_fStackValue
                        */
                        
                        //Prépare le pointeur ESI pour les filtres et le compteur ECX
                        mov         eax, multiband_lStackTotal
                        mov         esi, multiband_fFilterData
                        mov         ecx, eax
                        sub         eax, multiband_lStackPosition
                        lea         esi, [esi+eax*4]
                        
                        mov         edi, pfInputValue
                        xor         eax, eax
                                                
                        xorps       xmm0, xmm0
                        xorps       xmm1, xmm1
                        xorps       xmm2, xmm2
                        xorps       xmm3, xmm3
                        
                        
                    _loop_filter:    
                        movups      xmm4, [esi+eax+   0  ]
                        movups      xmm5, [esi+eax+2048  ]
                        movups      xmm6, [esi+eax+2048*2]
                        movups      xmm7, [esi+eax+2048*3]
                        
                        mulps       xmm4, [edi+eax]
                        mulps       xmm5, [edi+eax]
                        mulps       xmm6, [edi+eax]
                        mulps       xmm7, [edi+eax]
                        
                        addps       xmm0, xmm4
                        addps       xmm1, xmm5
                        addps       xmm2, xmm6
                        addps       xmm3, xmm7
                        
                        add         eax, 16
                        sub         ecx, 4
                        jnz         _loop_filter
                        
                        movhlps     xmm4, xmm0      //xmm2 = x x D C    xmm0 = D C B A
                        movhlps     xmm5, xmm1      //xmm2 = x x D C    xmm0 = D C B A
                        movhlps     xmm6, xmm2      //xmm2 = x x D C    xmm0 = D C B A
                        movhlps     xmm7, xmm3      //xmm2 = x x D C    xmm0 = D C B A
                        
                        addps       xmm0, xmm4      //xmm0 = x x D+B C+A
                        addps       xmm1, xmm5      //xmm0 = x x D+B C+A
                        addps       xmm2, xmm6      //xmm0 = x x D+B C+A
                        addps       xmm3, xmm7      //xmm0 = x x D+B C+A
                        
                        movaps      xmm4, xmm0      
                        movaps      xmm5, xmm1
                        movaps      xmm6, xmm2
                        movaps      xmm7, xmm3
                        
                        shufps      xmm4, xmm4, 0001b
                        shufps      xmm5, xmm5, 0001b
                        shufps      xmm6, xmm6, 0001b
                        shufps      xmm7, xmm7, 0001b
                        
                        addss       xmm0, xmm4
                        addss       xmm1, xmm5
                        addss       xmm2, xmm6
                        addss       xmm3, xmm7
                        
                        //Regroupement de xmm0, xmm1, xmm2, xmm3
                        
                        
                        
                        movss       fSumA, xmm0
                        movss       fSumB, xmm1
                        movss       fSumC, xmm2
                        movss       fSumD, xmm3
                        
                        
                        
                        
                        //Valeur absolue
                        movss       xmm4, dwAbsFloat
                        andps       xmm0, xmm4
                        andps       xmm1, xmm4
                        andps       xmm2, xmm4
                        andps       xmm3, xmm4
                        
                        movss       xmm4, multiband_fBandPeak
                        movss       xmm5, multiband_fBandPeak
                        movss       xmm6, multiband_fBandPeak
                        movss       xmm7, multiband_fBandPeak
                        cmpLTss     xmm4, xmm0                          //cmpss LessThan
                        
                        
                        
                        
                        
                        
                        
                        
            
            }
            /*
            for (i=0;i<multiband_lStackTotal;i++)
            {
                fSumA += pfInputValue[i] * pfFilterCoefA[i];
                fSumB += pfInputValue[i] * pfFilterCoefB[i];
                fSumC += pfInputValue[i] * pfFilterCoefC[i];
                fSumD += pfInputValue[i] * pfFilterCoefD[i];
            }
            */
            multiband_fBandValue[lChannelIndex][0] = fSumA;
            multiband_fBandValue[lChannelIndex][1] = fSumB;
            multiband_fBandValue[lChannelIndex][2] = fSumC;
            multiband_fBandValue[lChannelIndex][3] = fSumD;
            
            multiband_fBandPeak[0] = sam_ABS(fSumA);
            multiband_fBandPeak[1] = sam_ABS(fSumB);
            multiband_fBandPeak[2] = sam_ABS(fSumC);
            multiband_fBandPeak[3] = sam_ABS(fSumD);

            
            /*
            for (lBand=0;lBand<4;lBand++)
            {
                fTemp = 0.0F;
                for (i=0;i<multiband_lStackTotal;i++)
                {
                    fTemp += multiband_fStackValue[lChannelIndex][(multiband_lStackPosition-i)&(multiband_lStackTotal-1)] * 
                             multiband_fFilterData[lBand][i];
                }
                
                //fTemp = multiband_fStackValue[lChannelIndex][multiband_lStackPosition];
                multiband_fBandValue[lChannelIndex][lBand] = fTemp;
                
                //Redresse Temp
                if (fTemp<0)
                    fTemp = -fTemp;
                
                //Niveau crête
                if (fTemp>multiband_fBandPeak[lBand])
                    multiband_fBandPeak[lBand] = fTemp;
            }*/
        }
        
        //Etape 3 : Application du seuil limite et calcul du gain de chaque bande
        for (lBand=0;lBand<4;lBand++)
        {
            /*fTemp = multiband_fBandPeak[lBand];
            if (fTemp<fThreshold) 
            {
                multiband_fBandGain[lBand] = 1.0F;
            }
            else
            {
                fTemp = fThreshold / fTemp;
                multiband_fBandGain[lBand] = fThreshold / fTemp;  //Ici, il faudra utiliser la réciprocité en SSE ou eq.
            }
            */
            
            fTemp = multiband_fBandPeak[lBand]; //Le signal crête
            if (fTemp<multiband_fThreshold[lBand])
                fMax = 1.0F;
            else
                fMax = multiband_fThreshold[lBand] / fTemp;
            
            fTemp = multiband_fBandGain[lBand];
            fTemp *= 1.0001F;
            if (fTemp< 0.01F) fTemp = 0.01F; //Gain minimal de -20dB
            if (fTemp>  fMax) fTemp = fMax;
            
            multiband_fBandGain[lBand] = fTemp;
        }

        //Etape 4 : Application du decay pour la crête...
        multiband_fLimiterPeak *= fDecay;
        
        //Etape 5 : Regroupe les 4 bandes en une seule et mesure le niveau crête...
        for (lChannelIndex=0;lChannelIndex<lChannelCopyCount;lChannelIndex++)
        {
            //Ajoute les 4 bandes
            fTemp = 0.0F;
            for (lBand=0;lBand<4;lBand++)
            {
                fTemp += multiband_fBandValue[lChannelIndex][lBand] * multiband_fBandGain[lBand];
            }
            multiband_fLimiterValue[lChannelIndex] = fTemp;
            
            if (fTemp<0)
                fTemp = -fTemp;
            
            //Niveau crête
            if (fTemp>multiband_fLimiterPeak)
                multiband_fLimiterPeak = fTemp;
        }
        
        //Etape 6 : Application du seuil limite et calcul du gain du limiteur final
        fTemp = multiband_fLimiterPeak; //Le signal crête
        if (fTemp<multiband_fLimiterThreshold) 
            fMax = 1.0F;
        else
            fMax = multiband_fLimiterThreshold / fTemp;
            
        fTemp = multiband_fLimiterGain;
        fTemp *= 1.0001F;
        if (fTemp< 0.01F) fTemp = 0.01F; //Gain minimal de -20dB
        if (fTemp>  fMax) fTemp = fMax;
        multiband_fLimiterGain = fTemp;
        
        //Etape 7 : Application du gain final pour tous les canaux
        for (lChannelIndex=0;lChannelIndex<lChannelCopyCount;lChannelIndex++)
        {
            //Ajoute les 4 bandes
            fTemp = multiband_fLimiterValue[lChannelIndex] * multiband_fLimiterGain * 32767;
            lDataValue = (long)(fTemp+fNoiseFiltered);
            if (lDataValue> 32760) lDataValue =  32760;
            if (lDataValue<-32760) lDataValue = -32760;
            
            //lDataValue += lChannelIndex&1;
            
            //Ecriture sur la sortie
            pi16Target[lTargetIndex] = (INT16)lDataValue;
            lTargetIndex++;
            
            #ifdef sam_OUTPUTFILE
            pi16CopyTarget[lCopyTarget] = (INT16)lDataValue;
            lCopyTarget++;
            #endif
        }    
            
            
        //La position dans la sortie boucle sur le début...
        if (lTargetIndex>=lTargetMax)
            lTargetIndex = 0;
    }

    #ifdef sam_OUTPUTFILE
    fwrite ( pi16CopyTarget, lSamplesCopyCount, 2*lChannelCopyCount, samData.pFileDebug );
    #endif
}

void SAM_PostProcessCopy_MultiBand_old ( FLOAT32 * pf32SourceData, INT16 * pi16TargetData, long lSamplesCopyCount, long lTargetOffset, long lChannelCount, long lLimiterMode )
{
                long    lChannelIndex;
                long    lSampleIndex;
                FLOAT32 *pf32Source;
                INT16   *pi16Target;

                float   fLimiterGain;

                long    lTargetIndex;
                long    lTargetMax;

                long    lChannelCopyCount;
                long    lTargetSamplesCount;

                long    lTemp;
                float   fTemp;

                float   fNoiseMul;
    static      float   fNoise, fNoiseLast, fNoiseFiltered;
    static      DWORD   dwNoise = 0;
    
                float   *pfInputValue;
                float   *pfFilterCoefA;
                float   *pfFilterCoefB;
                float   *pfFilterCoefC;
                float   *pfFilterCoefD;
                float   fSumA;
                float   fSumB;
                float   fSumC;
                float   fSumD;
    
                float   fDecay;
                float   fGain;
                float   fMax;
                long    lBand;
                long    i;
                long    lDataValue;
                

    #ifdef sam_OUTPUTFILE
    long    lCopyTarget;
    lCopyTarget = 0;
    #endif
    
    SAM_PostProcessCopy_MultiBandInit ( );    

    lTargetSamplesCount = (long)samData.dwHardwareAndSoftwareBufferSamplesCount;
    lChannelCopyCount   = lChannelCount; //(long)samData.dwHardwaremixChannelsCount;
    lTargetIndex        = lChannelCopyCount * lTargetOffset;
    lTargetMax          = lChannelCopyCount * lTargetSamplesCount;

    pf32Source          = pf32SourceData;
    pi16Target          = pi16TargetData;

    /*
        De bons paramètres :
        
            27 et 0.9
            29 et 1,3
    
    */
    //Le générateur de bruit
    fNoiseMul = (float)(1 / pow ( 2, 27 )); //27 //30
    
    //Calcul la vitesse de decay
    fTemp = (float)samData.dwHardwaremixSampleRate;
    fTemp *= 0.100F; //100ms de decay
    fTemp = (float)log10 ( 0.37F ) / fTemp; // Le decay, c'est 100% vers 10%
    fDecay = (float)pow ( 10.0F, fTemp );
    
    //Calcul de la vitesse de Gain Up
    //fGainUp = 
    
    
    //Le gain de base
    switch (lLimiterMode)
    {
        case 0:
        default:
            fGain = 1.0F;
            break;

        case 1:
            fGain = 2.0F;
            break;

        case 2:
            fGain = 4.0F;
            break;

        case 3:
            fGain = 10.0F;
            break;
    }

    //Correction pour la sortie 16 bits    
    for (lSampleIndex=0;lSampleIndex<lSamplesCopyCount;lSampleIndex++)
    {
        //Générateur de bruit
        dwNoise = ((dwNoise * 1664525)+1013904223);
        fNoiseLast = ((float)dwNoise) * fNoiseMul;
        fNoise += (fNoiseLast-fNoise)*0.9F; //0.9F;
        fNoiseFiltered = fNoiseLast-fNoise;
    
        //Etape 1 : Application du decay pour la crête...
        for (lBand=0;lBand<4;lBand++)
        {
            multiband_fBandPeak[lBand] *= fDecay;
        }
        
                
        //Etape 2 : Séparation en 4 bandes et mesure du signal crête
        multiband_lStackPosition = (multiband_lStackPosition+1)&(255);
        for (lChannelIndex=0;lChannelIndex<lChannelCopyCount;lChannelIndex++)
        {
            //Ajoute la nouvelle valeur
            multiband_fStackValue[lChannelIndex][multiband_lStackPosition] = *pf32Source * fGain;
            *pf32Source = 0;
            pf32Source++;
            
            //Calcule le FIR
            fSumA           = 0;
            fSumB           = 0;
            fSumC           = 0;
            fSumD           = 0;
            i               = multiband_lStackTotal - multiband_lStackPosition;
            pfFilterCoefA   = multiband_fFilterData[0] + i;
            pfFilterCoefB   = multiband_fFilterData[1] + i;
            pfFilterCoefC   = multiband_fFilterData[2] + i;
            pfFilterCoefD   = multiband_fFilterData[3] + i;
            pfInputValue    = multiband_fStackValue[lChannelIndex];
            
            for (i=0;i<multiband_lStackTotal;i++)
            {
                fSumA += pfInputValue[i] * pfFilterCoefA[i];
                fSumB += pfInputValue[i] * pfFilterCoefB[i];
                fSumC += pfInputValue[i] * pfFilterCoefC[i];
                fSumD += pfInputValue[i] * pfFilterCoefD[i];
            }
            
            multiband_fBandValue[lChannelIndex][0] = fSumA;
            multiband_fBandValue[lChannelIndex][1] = fSumB;
            multiband_fBandValue[lChannelIndex][2] = fSumC;
            multiband_fBandValue[lChannelIndex][3] = fSumD;
            
            multiband_fBandPeak[0] = sam_ABS(fSumA);
            multiband_fBandPeak[1] = sam_ABS(fSumB);
            multiband_fBandPeak[2] = sam_ABS(fSumC);
            multiband_fBandPeak[3] = sam_ABS(fSumD);

            
            /*
            for (lBand=0;lBand<4;lBand++)
            {
                fTemp = 0.0F;
                for (i=0;i<multiband_lStackTotal;i++)
                {
                    fTemp += multiband_fStackValue[lChannelIndex][(multiband_lStackPosition-i)&(multiband_lStackTotal-1)] * 
                             multiband_fFilterData[lBand][i];
                }
                
                //fTemp = multiband_fStackValue[lChannelIndex][multiband_lStackPosition];
                multiband_fBandValue[lChannelIndex][lBand] = fTemp;
                
                //Redresse Temp
                if (fTemp<0)
                    fTemp = -fTemp;
                
                //Niveau crête
                if (fTemp>multiband_fBandPeak[lBand])
                    multiband_fBandPeak[lBand] = fTemp;
            }*/
        }
        
        //Etape 3 : Application du seuil limite et calcul du gain de chaque bande
        for (lBand=0;lBand<4;lBand++)
        {
            /*fTemp = multiband_fBandPeak[lBand];
            if (fTemp<fThreshold) 
            {
                multiband_fBandGain[lBand] = 1.0F;
            }
            else
            {
                fTemp = fThreshold / fTemp;
                multiband_fBandGain[lBand] = fThreshold / fTemp;  //Ici, il faudra utiliser la réciprocité en SSE ou eq.
            }
            */
            
            fTemp = multiband_fBandPeak[lBand]; //Le signal crête
            if (fTemp<multiband_fThreshold[lBand])
                fMax = 1.0F;
            else
                fMax = multiband_fThreshold[lBand] / fTemp;
            
            fTemp = multiband_fBandGain[lBand];
            fTemp *= 1.0001F;
            if (fTemp< 0.01F) fTemp = 0.01F; //Gain minimal de -20dB
            if (fTemp>  fMax) fTemp = fMax;
            
            multiband_fBandGain[lBand] = fTemp;
        }

        //Etape 4 : Application du decay pour la crête...
        multiband_fLimiterPeak *= fDecay;
        
        //Etape 5 : Regroupe les 4 bandes en une seule et mesure le niveau crête...
        for (lChannelIndex=0;lChannelIndex<lChannelCopyCount;lChannelIndex++)
        {
            //Ajoute les 4 bandes
            fTemp = 0.0F;
            for (lBand=0;lBand<4;lBand++)
            {
                fTemp += multiband_fBandValue[lChannelIndex][lBand] * multiband_fBandGain[lBand];
            }
            multiband_fLimiterValue[lChannelIndex] = fTemp;
            
            if (fTemp<0)
                fTemp = -fTemp;
            
            //Niveau crête
            if (fTemp>multiband_fLimiterPeak)
                multiband_fLimiterPeak = fTemp;
        }
        
        //Etape 6 : Application du seuil limite et calcul du gain du limiteur final
        fTemp = multiband_fLimiterPeak; //Le signal crête
        if (fTemp<multiband_fLimiterThreshold) 
            fMax = 1.0F;
        else
            fMax = multiband_fLimiterThreshold / fTemp;
            
        fTemp = multiband_fLimiterGain;
        fTemp *= 1.0001F;
        if (fTemp< 0.01F) fTemp = 0.01F; //Gain minimal de -20dB
        if (fTemp>  fMax) fTemp = fMax;
        multiband_fLimiterGain = fTemp;
        
        //Etape 7 : Application du gain final pour tous les canaux
        for (lChannelIndex=0;lChannelIndex<lChannelCopyCount;lChannelIndex++)
        {
            //Ajoute les 4 bandes
            fTemp = multiband_fLimiterValue[lChannelIndex] * multiband_fLimiterGain * 32767;
            lDataValue = (long)(fTemp+fNoiseFiltered);
            if (lDataValue> 32760) lDataValue =  32760;
            if (lDataValue<-32760) lDataValue = -32760;
            
            //lDataValue += lChannelIndex&1;
            
            //Ecriture sur la sortie
            pi16Target[lTargetIndex] = (INT16)lDataValue;
            lTargetIndex++;
            
            #ifdef sam_OUTPUTFILE
            pi16CopyTarget[lCopyTarget] = (INT16)lDataValue;
            lCopyTarget++;
            #endif
        }    
            
            
        //La position dans la sortie boucle sur le début...
        if (lTargetIndex>=lTargetMax)
            lTargetIndex = 0;
    }

    #ifdef sam_OUTPUTFILE
    fwrite ( pi16CopyTarget, lSamplesCopyCount, 2*lChannelCopyCount, samData.pFileDebug );
    #endif
}


void SAM_PostProcessCopy_Limiter ( FLOAT32 * pf32SourceData, INT16 * pi16TargetData, long lSamplesCopyCount, long lTargetOffset, long lChannelCount, long lLimiterMode )
{
                long    lChannelIndex;
                long    lSampleIndex;
                FLOAT32 *pf32Source;
                INT16   *pi16Target;

                float   fLimiterGain;

                long    lTargetIndex;
                long    lTargetMax;

                long    lChannelCopyCount;
                long    lTargetSamplesCount;

                long    lTemp;
                float   fTemp;
                float   fTempA;
                float   fTempB;

                float   fNoiseMul;
    static      float   fNoise, fNoiseLast, fNoiseFiltered;
    static      DWORD   dwNoise = 0;
    
                float   fDecay;
                float   fGain;
                float   fMax;
                long    lBand;
                long    i;
                long    lDataValue;
                
                float   fGainDecay;

    static      float   pre_fStackDelay[8][256];
    static      long    pre_lStackPosition = 0;            
                

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

    /*
        De bons paramètres :
        
            27 et 0.9
            29 et 1,3
    
    */
    //Le générateur de bruit
    fNoiseMul = (float)(1 / pow ( 2, 27 )); //27 //30
    
    //Calcul la vitesse de decay du peak-meter
    fTemp = (float)samData.dwHardwaremixSampleRate;
    fTemp *= 0.100F; //100ms de decay
    fTemp = (float)log10 ( 0.10F ) / fTemp; // Le decay, c'est 100% vers 10%
    fDecay = (float)pow ( 10.0F, fTemp );
    //fDecay = 0.995F;
    
    //Calcul de la vitesse de decay du gain
    fTemp = (float)samData.dwHardwaremixSampleRate;
    fTemp *= 0.470F; //480ms de decay
    fTemp = (float)log10 ( 0.10F ) / fTemp; // Le decay, c'est 100% vers 10%
    fGainDecay = (float)(1 / pow ( 10.0F, fTemp ));
    //1.0001F = 0.9999
    
    //Le gain de base
    switch (lLimiterMode)
    {
        case 0:
        default:
            fGain = 1.0F;
            break;

        case 1:
            fGain = 2.0F;
            break;

        case 2:
            fGain = 4.0F;
            break;

        case 3:
            fGain = 10.0F;
            break;
    }

    //Correction pour la sortie 16 bits    
    for (lSampleIndex=0;lSampleIndex<lSamplesCopyCount;lSampleIndex++)
    {
        //Générateur de bruit
        dwNoise = ((dwNoise * 1664525)+1013904223);
        fNoiseLast = ((float)dwNoise) * fNoiseMul;
        fNoise += (fNoiseLast-fNoise)*0.9F; //0.9F;
        fNoiseFiltered = fNoiseLast-fNoise;
    
        //Etape 1 : Application du decay pour la crête...
        multiband_fLimiterPeak *= fDecay;
        
        // Gestion de la pile de retard
        pre_lStackPosition = (pre_lStackPosition+1)&127; //2,67ms @ 48KHz
                
        //Etape 2 : Lecture et mesure du signal crête
        for (lChannelIndex=0;lChannelIndex<lChannelCopyCount;lChannelIndex++)
        {
            //Stocke dans la pile            
            pre_fStackDelay[lChannelIndex][pre_lStackPosition] = *pf32Source;
            
            //Efface le tampon source et passe à l'élément suivant
            *pf32Source = 0;
            pf32Source++;
            
            //La valeur courante est retardée
            multiband_fLimiterValue[lChannelIndex] = pre_fStackDelay[lChannelIndex][(pre_lStackPosition-127)&127] * fGain;

            //Détecteur de crête (de la valeur retardée et celle en avance...)            
            fTempA = multiband_fLimiterValue[lChannelIndex];
            fTempB = pre_fStackDelay[lChannelIndex][pre_lStackPosition] * fGain;
            
            if (fTempA<0) fTempA = -fTempA;
            if (fTempB<0) fTempB = -fTempB;
            
            if (fTempB>fTempA) fTempA = fTempB;
        
            //Niveau crête
            if (fTempA>multiband_fLimiterPeak)
                multiband_fLimiterPeak = fTempA;
        }
        
        //Etape 6 : Application du seuil limite et calcul du gain du limiteur final
        fTemp = multiband_fLimiterPeak; //Le signal crête
        if (fTemp<multiband_fLimiterThreshold) 
            fMax = 1.0F;
        else
            fMax = multiband_fLimiterThreshold / fTemp;
            
        fTemp = multiband_fLimiterGain * fGainDecay;
        if (fTemp< 0.01F) fTemp = 0.01F; //Gain minimal de -20dB
        if (fTemp>  fMax) fTemp = fMax;
        multiband_fLimiterGain = fTemp;
        
        //Etape 7 : Application du gain final pour tous les canaux
        for (lChannelIndex=0;lChannelIndex<lChannelCopyCount;lChannelIndex++)
        {
            //Ajoute les 4 bandes
            fTemp = multiband_fLimiterValue[lChannelIndex] * multiband_fLimiterGain * 32767;
            lDataValue = (long)(fTemp+fNoiseFiltered);
            if (lDataValue> 32760) lDataValue =  32760;
            if (lDataValue<-32760) lDataValue = -32760;
            
            //lDataValue += lChannelIndex&1;
            
            //Ecriture sur la sortie
            pi16Target[lTargetIndex] = (INT16)lDataValue;
            lTargetIndex++;
            
            #ifdef sam_OUTPUTFILE
            pi16CopyTarget[lCopyTarget] = (INT16)lDataValue;
            lCopyTarget++;
            #endif
        }    
            
            
        //La position dans la sortie boucle sur le début...
        if (lTargetIndex>=lTargetMax)
            lTargetIndex = 0;
    }

    #ifdef sam_OUTPUTFILE
    fwrite ( pi16CopyTarget, lSamplesCopyCount, 2*lChannelCopyCount, samData.pFileDebug );
    #endif
}



