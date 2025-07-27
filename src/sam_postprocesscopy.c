#include "sam_header.h"
#include "sam_data.h"

INT16 pi16CopyTarget[128000];
#define CVT_dB2VAL(n)   ((long)((n)*16777216))

void SAM_PostProcessCopy_MultiBandInit ( void );
void SAM_PostProcessCopy_MultiBand ( FLOAT32 * pf32SourceData, INT16 * pi16TargetData, long lSamplesCopyCount, long lTargetOffset, long lChannelCount, long lLimiterMode );
void SAM_PostProcessCopy_Limiter ( FLOAT32 * pf32SourceData, INT16 * pi16TargetData, long lSamplesCopyCount, long lTargetOffset, long lChannelCount, long lLimiterMode );

void SAM_PostProcessCopy_Init ( void )
{
    long lLimiterMode;
    long lIndex;

    float fLimiterGain;
    float fLimiterSpeed;
    float fLimiterThreshold;
    float f1;
    float fQuantification0dBFS;
    static long lInit = 0;
    
    
    SAM_PostProcessCopy_MultiBandInit ( );
    

    if (lInit)
        return;
    
    lInit = 1;
        
            

    fQuantification0dBFS = 32767;
    
    for (lLimiterMode=0;lLimiterMode<4;lLimiterMode++)
    {
        switch (lLimiterMode)
        {
            case 0: //0dB
                fLimiterSpeed       = 10;
                fLimiterGain        = 0.0;
                fLimiterThreshold   = -3.0;
                break;

            case 1: //+6dB
                fLimiterSpeed       = 20;
                fLimiterGain        = 6.0;
                fLimiterThreshold   = -4.0;
                break;

            case 2: //+12dB
                fLimiterSpeed       = 40;
                fLimiterGain        = 12.0;
                fLimiterThreshold   = -5.0;
                break;

            case 3: //+20dB                 
                fLimiterSpeed       = 50;
                fLimiterGain        = 20.0;
                fLimiterThreshold   = -6.0;
                break;
        }
        samData.lLimiterGainMax[lLimiterMode] = CVT_dB2VAL(fLimiterGain);

        for (lIndex=0;lIndex<1024;lIndex++)
        {
            f1 = (float)(lIndex+1);
            samData.lLimiterConvertTodB_8_24[lIndex] = (long)((log10 ( f1/128 ) * 20)*16777217);
        }

    }
    samData.fLimiterCurrentGain = 1.0;// * fQuantification0dBFS;

    for (lIndex=0;lIndex<4096;lIndex++)
    {
        //L'amplitude est de 60dB pour une plage de -40 à +20dB
        f1 = (float)lIndex;
        f1 = f1/3840.0F;
        if (f1>1.0F) f1 = 1.0F;

        f1 *= 60;
        f1 -= 40;

        f1 += 90.3F;

        samData.fLimiterGainTable[lIndex] = (float)pow ( 10, f1*0.05F );
    }


    samData.lLimiterPeakLevel   = CVT_dB2VAL(-40);
    samData.lLimiterGain        = samData.lLimiterGainMax[0];
    samData.fLimiterCurrentGain = 0;
    
    

}




void SAM_PostProcessCopy_Direct_Simple ( FLOAT32 * pf32SourceData, INT16 * pi16TargetData, long lSamplesCopyCount, long lTargetOffset, long lChannelCount, long lLimiterMode )
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

    long    lTemp;

    float   fNoiseMul;
    static  float   fNoise, fNoiseLast, fNoiseFiltered;
    static  DWORD dwNoise = 0;

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


    //Le générateur de bruit
    fNoiseMul = (float)(1 / pow ( 2, 27 )); //30

    for (lSampleIndex=0;lSampleIndex<lSamplesCopyCount;lSampleIndex++)
    {
        //Générateur de bruit
        dwNoise = ((dwNoise * 1664525)+1013904223);
        fNoiseLast = ((float)dwNoise) * fNoiseMul;
        fNoise += (fNoiseLast-fNoise)*0.9F;
        fNoiseFiltered = fNoiseLast-fNoise;

        for (lChannelIndex=0;lChannelIndex<lChannelCopyCount;lChannelIndex++)
        {
            //Donnée source
            lDataValue = (long)(((*pf32Source)*32767)+fNoiseFiltered);

            //Vide la source
            *pf32Source = 0;
            pf32Source++;

            //Saturation à 0dB FS
            if (lDataValue> 32767) lDataValue =  32767;
            if (lDataValue<-32767) lDataValue = -32767;

            //Ecriture sur la sortie
            pi16Target[lTargetIndex] = (INT16)lDataValue;//*0.1;
            lTargetIndex++;
        }

        //La position dans la sortie boucle sur le début...
        if (lTargetIndex>=lTargetMax)
            lTargetIndex = 0;
    }
}


void SAM_PostProcessCopy_Direct ( FLOAT32 * pf32SourceData, INT16 * pi16TargetData, long lSamplesCopyCount, long lTargetOffset, long lChannelCount, long lLimiterMode )
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

    long    lTemp;

    float   fNoiseMul;
    static  float   fNoise, fNoiseLast, fNoiseFiltered;
    static  DWORD dwNoise = 0;

    #ifdef sam_OUTPUTFILE
    long    lCopyTarget;
    lCopyTarget = 0;
    #endif

    lTargetSamplesCount = (long)samData.dwHardwareAndSoftwareBufferSamplesCount;
    lChannelCopyCount   = lChannelCount; //(long)samData.dwHardwaremixChannelsCount;
    lTargetIndex        = lChannelCopyCount * lTargetOffset;
    lTargetMax          = lChannelCopyCount * lTargetSamplesCount;

    plLimiterConvertTodB_8_24   = samData.lLimiterConvertTodB_8_24;
    lLimiterPeakLevel           = samData.lLimiterPeakLevel;
    lLimiterGain                = samData.lLimiterGain;
    lLimiterGainMax             = samData.lLimiterGainMax[lLimiterMode];
    fLimiterGain                = samData.fLimiterCurrentGain;
    lLimiterDelayBeforeGrowing  = samData.lLimiterDelayBeforeGrowing;

    pf32Source          = pf32SourceData;
    pi16Target          = pi16TargetData;

    /*
        De bons paramètres :
        
            27 et 0.9
            29 et 1,3
    
    */
    //Le générateur de bruit
    fNoiseMul = (float)(1 / pow ( 2, 27 )); //27 //30

    for (lSampleIndex=0;lSampleIndex<lSamplesCopyCount;lSampleIndex++)
    {
        //Le niveau crête pour cet échantillon multi-canal
        lPeakValue = 0;

        //Générateur de bruit
        dwNoise = ((dwNoise * 1664525)+1013904223);
        fNoiseLast = ((float)dwNoise) * fNoiseMul;
        fNoise += (fNoiseLast-fNoise)*0.9F; //0.9F;
        fNoiseFiltered = fNoiseLast-fNoise;

        
        for (lChannelIndex=0;lChannelIndex<lChannelCopyCount;lChannelIndex++)
        {
            //Donnée source
            lDataValue = (long)(((*pf32Source)*fLimiterGain)+fNoiseFiltered);

            //Vide la source
            *pf32Source = 0;
            pf32Source++;

            //Lecture de la valeur crête avant saturation à 0dB FS
            lAbsValue = sam_ABS ( lDataValue );
            if (lAbsValue>lPeakValue) lPeakValue = lAbsValue;

            //Saturation à 0dB FS
            if (lDataValue> 32767) lDataValue =  32767;
            if (lDataValue<-32767) lDataValue = -32767;

            //Ecriture sur la sortie
            pi16Target[lTargetIndex] = (INT16)lDataValue;//*0.1;
            lTargetIndex++;

            #ifdef sam_OUTPUTFILE
            pi16CopyTarget[lCopyTarget] = (INT16)lDataValue;
            lCopyTarget++;
            #endif
        }

        //La position dans la sortie boucle sur le début...
        if (lTargetIndex>=lTargetMax)
            lTargetIndex = 0;

        //Convertion du niveau en dB
        if (lPeakValue>262143) lPeakValue = 262143; //Gestion d'une sur-limite de +12dB FS
        lPeakLevel = plLimiterConvertTodB_8_24[lPeakValue>>8];

        //Détection du niveau crête (-40dB...)
        lLimiterPeakLevel -= CVT_dB2VAL(0.005); //-0.0005dB
        if (lPeakLevel>lLimiterPeakLevel) lLimiterPeakLevel = lPeakLevel;        
        if (lLimiterPeakLevel<CVT_dB2VAL(-40)) lLimiterPeakLevel = CVT_dB2VAL(-40);

        if (lLimiterPeakLevel>=CVT_dB2VAL(-3))
            lLimiterDelayBeforeGrowing = samData.dwHardwaremixSampleRate>>6;

        //On atteint la limite de la saturation ?
        if (lLimiterPeakLevel>=CVT_dB2VAL(-2))
        {
            //Ecart entre le niveau courant et la limite de la saturation
            lDeltaLevelToSat = CVT_dB2VAL(-2) - lLimiterPeakLevel;

            //On passe le niveau crête à -2dB
            lLimiterPeakLevel = CVT_dB2VAL(-2);//+lLimiterPeakLevel)/2;
        }
        else
        {
            lDeltaLevelToSat = 0;
            lLimiterDelayBeforeGrowing--;

            if (lLimiterDelayBeforeGrowing<0)
            {
                lLimiterDelayBeforeGrowing = 0;
                lLimiterGain += CVT_dB2VAL(0.005);
            }

            if (lLimiterGain>lLimiterGainMax) lLimiterGain = lLimiterGainMax;
        }

        //On modifie le niveau du limiter
        lLimiterGain += lDeltaLevelToSat;

        //Génération du niveau de sortie
        lTemp = lLimiterGain+CVT_dB2VAL(40);
        if (lTemp<0) lTemp = 0;
        fLimiterGain = samData.fLimiterGainTable[lTemp>>18];
    }

    #ifdef sam_OUTPUTFILE
    fwrite ( pi16CopyTarget, lSamplesCopyCount, 2*lChannelCopyCount, samData.pFileDebug );
    #endif

    samData.fLimiterCurrentGain = fLimiterGain;
    samData.lLimiterGain        = lLimiterGain;
    samData.lLimiterPeakLevel   = lLimiterPeakLevel;
    samData.lLimiterDelayBeforeGrowing = lLimiterDelayBeforeGrowing;
}

void SAM_PostProcessCopy_DolbyProLogic ( FLOAT32 * pf32SourceData, INT16 * pi16TargetData, long lSamplesCopyCount, long lTargetOffset, long lChannelCount, long lLimiterMode )
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

    static      float   fFIRStackValue[256] = { 0 };
    static      long    lFIRStackPosition   = 0;

                long    lTemp, i, j;

                float   fNoiseMul;
    static      float   fNoise, fNoiseLast, fNoiseFiltered;
    static      DWORD   dwNoise = 0;
                float   fL, fR, fC, fS;
                float   fChannel[2];

    #ifdef sam_OUTPUTFILE
    long    lCopyTarget;
    lCopyTarget = 0;
    #endif

    lTargetSamplesCount = (long)samData.dwHardwareAndSoftwareBufferSamplesCount;
    lChannelCopyCount   = lChannelCount; //(long)samData.dwHardwaremixChannelsCount;
    lTargetIndex        = lChannelCopyCount * lTargetOffset;
    lTargetMax          = lChannelCopyCount * lTargetSamplesCount;

    plLimiterConvertTodB_8_24   = samData.lLimiterConvertTodB_8_24;
    lLimiterPeakLevel           = samData.lLimiterPeakLevel;
    lLimiterGain                = samData.lLimiterGain;
    lLimiterGainMax             = samData.lLimiterGainMax[lLimiterMode];
    fLimiterGain                = samData.fLimiterCurrentGain;
    lLimiterDelayBeforeGrowing  = samData.lLimiterDelayBeforeGrowing;

    pf32Source          = pf32SourceData;
    pi16Target          = pi16TargetData;


    //Le générateur de bruit
    fNoiseMul = (float)(1 / pow ( 2, 27 )); //30

    for (lSampleIndex=0;lSampleIndex<lSamplesCopyCount;lSampleIndex++)
    {
        //Lecture des canaux d'origine
        fL = pf32Source[0];
        fR = pf32Source[1];
        fC = pf32Source[2];
        fS = pf32Source[3];

        //Vide la source
        pf32Source[0] = 0;
        pf32Source[1] = 0;
        pf32Source[2] = 0;
        pf32Source[3] = 0;
        pf32Source+=4;

        //Application de la transformation de Hilbert via le FIR
        fFIRStackValue[lFIRStackPosition]   = fS;
        fS                                  = 0;
        j = 128 - lFIRStackPosition;
        for (i=0;i<128;i++,j++)
            fS += fFIRStackValue[i] * samData.fHilbertFIRCoef[j];
        lFIRStackPosition = (lFIRStackPosition+1)&127;

        //Matriçage de la sortie
        fC = 0.707F * fC;
        fS = 0.707F * fS;
        fChannel[0] = fL + fC + fS;
        fChannel[1] = fR + fC - fS;

        //Le niveau crête pour cet échantillon multi-canal
        lPeakValue = 0;

        //Générateur de bruit
        dwNoise = ((dwNoise * 1664525)+1013904223);
        fNoiseLast = ((float)dwNoise) * fNoiseMul;
        fNoise += (fNoiseLast-fNoise)*0.9F;
        fNoiseFiltered = fNoiseLast-fNoise;

        for (lChannelIndex=0;lChannelIndex<2;lChannelIndex++)
        {
            //Donnée source
            lDataValue = (long)((fChannel[lChannelIndex]*fLimiterGain)+fNoiseFiltered);

            //Lecture de la valeur crête avant saturation à 0dB FS
            lAbsValue = sam_ABS ( lDataValue );
            if (lAbsValue>lPeakValue) lPeakValue = lAbsValue;

            //Saturation à 0dB FS
            if (lDataValue> 32767) lDataValue =  32767;
            if (lDataValue<-32767) lDataValue = -32767;

            //Ecriture sur la sortie
            pi16Target[lTargetIndex] = (INT16)lDataValue;//*0.1;
            lTargetIndex++;

            #ifdef sam_OUTPUTFILE
            pi16CopyTarget[lCopyTarget] = (INT16)lDataValue;
            lCopyTarget++;
            #endif
        }

        //La position dans la sortie boucle sur le début...
        if (lTargetIndex>=lTargetMax)
            lTargetIndex = 0;

        //Convertion du niveau en dB
        if (lPeakValue>262143) lPeakValue = 262143; //Gestion d'une sur-limite de +12dB FS
        lPeakLevel = plLimiterConvertTodB_8_24[lPeakValue>>8];

        //Détection du niveau crête (-40dB...)
        lLimiterPeakLevel -= CVT_dB2VAL(0.005); //-0.0005dB
        if (lPeakLevel>lLimiterPeakLevel) lLimiterPeakLevel = lPeakLevel;        
        if (lLimiterPeakLevel<CVT_dB2VAL(-40)) lLimiterPeakLevel = CVT_dB2VAL(-40);

        if (lLimiterPeakLevel>=CVT_dB2VAL(-3))
            lLimiterDelayBeforeGrowing = samData.dwHardwaremixSampleRate>>6;

        //On atteint la limite de la saturation ?
        if (lLimiterPeakLevel>=CVT_dB2VAL(-2))
        {
            //Ecart entre le niveau courant et la limite de la saturation
            lDeltaLevelToSat = CVT_dB2VAL(-2) - lLimiterPeakLevel;

            //On passe le niveau crête à -2dB
            lLimiterPeakLevel = CVT_dB2VAL(-2);//+lLimiterPeakLevel)/2;
        }
        else
        {
            lDeltaLevelToSat = 0;
            lLimiterDelayBeforeGrowing--;

            if (lLimiterDelayBeforeGrowing<0)
            {
                lLimiterDelayBeforeGrowing = 0;
                lLimiterGain += CVT_dB2VAL(0.005);
            }

            if (lLimiterGain>lLimiterGainMax) lLimiterGain = lLimiterGainMax;
        }

        //On modifie le niveau du limiter
        lLimiterGain += lDeltaLevelToSat;

        //Génération du niveau de sortie
        lTemp = lLimiterGain+CVT_dB2VAL(40);
        if (lTemp<0) lTemp = 0;
        fLimiterGain = samData.fLimiterGainTable[lTemp>>18];
    }

    #ifdef sam_OUTPUTFILE
    fwrite ( pi16CopyTarget, lSamplesCopyCount, 2*lChannelCopyCount, samData.pFileDebug );
    #endif

    samData.fLimiterCurrentGain = fLimiterGain;
    samData.lLimiterGain        = lLimiterGain;
    samData.lLimiterPeakLevel   = lLimiterPeakLevel;
    samData.lLimiterDelayBeforeGrowing = lLimiterDelayBeforeGrowing;
}

void SAM_PostProcessCopy_DolbyProLogicII ( FLOAT32 * pf32SourceData, INT16 * pi16TargetData, long lSamplesCopyCount, long lTargetOffset, long lChannelCount, long lLimiterMode )
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

    static      float   fFIRStackValue[256] = { 0 };
    static      long    lFIRStackPosition   = 0;

                long    lTemp, i, j;

                float   fNoiseMul;
    static      float   fNoise, fNoiseLast, fNoiseFiltered;
    static      DWORD   dwNoise = 0;
                float   fL, fR, fC, fSL, fSR;
                float   fChannel[2];

    #ifdef sam_OUTPUTFILE
    long    lCopyTarget;
    lCopyTarget = 0;
    #endif

    lTargetSamplesCount = (long)samData.dwHardwareAndSoftwareBufferSamplesCount;
    lChannelCopyCount   = lChannelCount; //(long)samData.dwHardwaremixChannelsCount;
    lTargetIndex        = lChannelCopyCount * lTargetOffset;
    lTargetMax          = lChannelCopyCount * lTargetSamplesCount;

    plLimiterConvertTodB_8_24   = samData.lLimiterConvertTodB_8_24;
    lLimiterPeakLevel           = samData.lLimiterPeakLevel;
    lLimiterGain                = samData.lLimiterGain;
    lLimiterGainMax             = samData.lLimiterGainMax[lLimiterMode];
    fLimiterGain                = samData.fLimiterCurrentGain;
    lLimiterDelayBeforeGrowing  = samData.lLimiterDelayBeforeGrowing;

    pf32Source          = pf32SourceData;
    pi16Target          = pi16TargetData;


    //Le générateur de bruit
    fNoiseMul = (float)(1 / pow ( 2, 27 )); //30

    for (lSampleIndex=0;lSampleIndex<lSamplesCopyCount;lSampleIndex++)
    {
        //Lecture des canaux d'origine
        fL  = pf32Source[0];
        fR  = pf32Source[1];
        fC  = pf32Source[2];
        fSL = pf32Source[4];
        fSR = pf32Source[5];

        //Vide la source
        pf32Source[0] = 0;
        pf32Source[1] = 0;
        pf32Source[2] = 0;
        pf32Source[3] = 0;
        pf32Source[4] = 0;
        pf32Source[5] = 0;
        pf32Source+=6;

        //Application de la transformation de Hilbert via le FIR
        fFIRStackValue[lFIRStackPosition]       = fSL;
        fFIRStackValue[lFIRStackPosition+128]   = fSR;
        fSL                                     = 0;
        j                                       = 128 - lFIRStackPosition;
        for (i=0;i<128;i++,j++)
        {
            fSL += fFIRStackValue[i    ] * samData.fHilbertFIRCoef[j];
            fSR += fFIRStackValue[i+128] * samData.fHilbertFIRCoef[j];
        }
        lFIRStackPosition = (lFIRStackPosition+1)&127;

        //Matriçage de la sortie
        fC  = 0.707F * fC;
        fChannel[0] = fL + fC + fSL * 0.8165F + fSR * 0.5774F;
        fChannel[1] = fR + fC - fSL * 0.5774F - fSR * 0.8165F;

        //Le niveau crête pour cet échantillon multi-canal
        lPeakValue = 0;

        //Générateur de bruit
        dwNoise = ((dwNoise * 1664525)+1013904223);
        fNoiseLast = ((float)dwNoise) * fNoiseMul;
        fNoise += (fNoiseLast-fNoise)*0.9F;
        fNoiseFiltered = fNoiseLast-fNoise;

        for (lChannelIndex=0;lChannelIndex<2;lChannelIndex++)
        {
            //Donnée source
            lDataValue = (long)((fChannel[lChannelIndex]*fLimiterGain)+fNoiseFiltered);

            //Lecture de la valeur crête avant saturation à 0dB FS
            lAbsValue = sam_ABS ( lDataValue );
            if (lAbsValue>lPeakValue) lPeakValue = lAbsValue;

            //Saturation à 0dB FS
            if (lDataValue> 32767) lDataValue =  32767;
            if (lDataValue<-32767) lDataValue = -32767;

            //Ecriture sur la sortie
            pi16Target[lTargetIndex] = (INT16)lDataValue;//*0.1;
            lTargetIndex++;

            #ifdef sam_OUTPUTFILE
            pi16CopyTarget[lCopyTarget] = (INT16)lDataValue;
            lCopyTarget++;
            #endif
        }

        //La position dans la sortie boucle sur le début...
        if (lTargetIndex>=lTargetMax)
            lTargetIndex = 0;

        //Convertion du niveau en dB
        if (lPeakValue>262143) lPeakValue = 262143; //Gestion d'une sur-limite de +12dB FS
        lPeakLevel = plLimiterConvertTodB_8_24[lPeakValue>>8];

        //Détection du niveau crête (-40dB...)
        lLimiterPeakLevel -= CVT_dB2VAL(0.005); //-0.0005dB
        if (lPeakLevel>lLimiterPeakLevel) lLimiterPeakLevel = lPeakLevel;        
        if (lLimiterPeakLevel<CVT_dB2VAL(-40)) lLimiterPeakLevel = CVT_dB2VAL(-40);

        if (lLimiterPeakLevel>=CVT_dB2VAL(-3))
            lLimiterDelayBeforeGrowing = samData.dwHardwaremixSampleRate>>6;

        //On atteint la limite de la saturation ?
        if (lLimiterPeakLevel>=CVT_dB2VAL(-2))
        {
            //Ecart entre le niveau courant et la limite de la saturation
            lDeltaLevelToSat = CVT_dB2VAL(-2) - lLimiterPeakLevel;

            //On passe le niveau crête à -2dB
            lLimiterPeakLevel = CVT_dB2VAL(-2);//+lLimiterPeakLevel)/2;
        }
        else
        {
            lDeltaLevelToSat = 0;
            lLimiterDelayBeforeGrowing--;

            if (lLimiterDelayBeforeGrowing<0)
            {
                lLimiterDelayBeforeGrowing = 0;
                lLimiterGain += CVT_dB2VAL(0.005);
            }

            if (lLimiterGain>lLimiterGainMax) lLimiterGain = lLimiterGainMax;
        }

        //On modifie le niveau du limiter
        lLimiterGain += lDeltaLevelToSat;

        //Génération du niveau de sortie
        lTemp = lLimiterGain+CVT_dB2VAL(40);
        if (lTemp<0) lTemp = 0;
        fLimiterGain = samData.fLimiterGainTable[lTemp>>18];
    }

    #ifdef sam_OUTPUTFILE
    fwrite ( pi16CopyTarget, lSamplesCopyCount, 2*lChannelCopyCount, samData.pFileDebug );
    #endif

    samData.fLimiterCurrentGain = fLimiterGain;
    samData.lLimiterGain        = lLimiterGain;
    samData.lLimiterPeakLevel   = lLimiterPeakLevel;
    samData.lLimiterDelayBeforeGrowing = lLimiterDelayBeforeGrowing;
}




void SAM_PostProcessCopy ( FLOAT32 * pf32SourceData, INT16 * pi16TargetData, long lSamplesCopyCount, long lTargetOffset, long lLimiterMode )
{
    switch (samData.dwOutputEncoder)
    {
        case 0: //Direct copy
            //SAM_PostProcessCopy_Direct ( pf32SourceData, pi16TargetData, lSamplesCopyCount, lTargetOffset, (long)samData.dwOutputHardwareChannel, lLimiterMode );
            //SAM_PostProcessCopy_MultiBand ( pf32SourceData, pi16TargetData, lSamplesCopyCount, lTargetOffset, (long)samData.dwOutputHardwareChannel, lLimiterMode );
            SAM_PostProcessCopy_Limiter ( pf32SourceData, pi16TargetData, lSamplesCopyCount, lTargetOffset, (long)samData.dwOutputHardwareChannel, lLimiterMode );
            break;

        case 1: //Dolby Pro Logic 4 channel => 2.0
            SAM_PostProcessCopy_DolbyProLogic ( pf32SourceData, pi16TargetData, lSamplesCopyCount, lTargetOffset, (long)samData.dwOutputHardwareChannel, lLimiterMode );
            break;    

        case 2: //Dolby Pro Logic 6 channel => 2.0
            SAM_PostProcessCopy_DolbyProLogicII ( pf32SourceData, pi16TargetData, lSamplesCopyCount, lTargetOffset, (long)samData.dwOutputHardwareChannel, lLimiterMode );
            break;
            
        case 3: //0x26 Headphones : Virtual Holographic 6ch => 2.0    
            SAM_PostProcessCopy_0x26 ( pf32SourceData, pi16TargetData, lSamplesCopyCount, lTargetOffset, (long)samData.dwOutputHardwareChannel, lLimiterMode );
            break;
    }
}

