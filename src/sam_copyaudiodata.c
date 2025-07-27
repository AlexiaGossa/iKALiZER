#include "sam_header.h"

/*

    Codage des 6 bandes
    
        30  2 bits = Offset global par pas de 10dB
        25  5 bits = Offset pour la bande 5 (par pas de 1dB)
        ..  5 bits = Offset pour la bande x
         0  5 bits = Offset pour la bande 0
         
     Dynamique maximale : 0 à -61dB
     
     //8 bits = 0 à -127dB par pas de 1/2dB
     

*/
        /*        
        fSignalC


    static float y[NCoef+1]; //output samples
    static float x[NCoef+1]; //input samples
    int n;

    //shift the old samples
    for(n=NCoef; n>0; n--) {
       x[n] = x[n-1];
       y[n] = y[n-1];
    }

    //Calculate the new output
    x[0] = NewSample;
    y[0] = ACoef[0] * x[0];
    for(n=1; n<=NCoef; n++)
        y[0] += ACoef[n] * x[n] - BCoef[n] * y[n];
    
    return y[0];
    */
/*
    ISO 226:2003
    
    Courbe pour 80dB
    
        16
    
    


*/


long SAM_EncodePsyAcousticBand ( void * pAudioDataSource, BYTE bFormatSource, DWORD dwSamplesCount, float fSampleRate, float fGain, SAM_SFX_PSY * ppsyData )
{
    float fComp8bits;
    float fComp16bits;
    float fSignalL;
    float fSignalR;
    float fSignalC;
    float fTemp;

    long  i;
    long    lPsyPointDuration;
    long    lPsyPointPosition;
    BYTE    *pbAttenuationLevelSensor;
    DWORD   dwAttenuationLevelSensorPosition;


    BYTE    * pAudioDataSourceCurrent;
    DWORD   dwCount;
    

    float   fPeakLevel;
    float   fAttackRate;
    float   fDecayRate;
    
    QWORD ui64A = 0;

    
    //On a le droit de ne pas avoir de PsyData
    if (!ppsyData)
        return 0;
    
    fComp8bits  = 1.0 /   128.0;
    fComp16bits = 1.0 / 32768.0;
    fComp8bits  *= fGain;
    fComp16bits *= fGain;
    
    pAudioDataSourceCurrent = (BYTE *)pAudioDataSource;
    dwCount                 = dwSamplesCount;
    lPsyPointPosition       = 0;
        
    //Par défaut, on a une précision de 20ms
    lPsyPointDuration = (long)floor(fSampleRate * 0.020F);
    
    //Nombre de points pour 20ms de précision
    i = dwSamplesCount / (DWORD)lPsyPointDuration;
    if (i>1023) //Le granule sera plein !
        lPsyPointDuration = (dwSamplesCount/1023)+1;
        
    //Stocke la longueur d'un point 6 bandes :
    ppsyData->dwPsyPointDuration            = (DWORD)lPsyPointDuration;
    ppsyData->dwPsyPointDuration_Mul_Shr10  = (DWORD)SAM_MATH_lroundf ( 1024.0F / (float)(ppsyData->dwPsyPointDuration) );
    
    //Le pointeur de base
    pbAttenuationLevelSensor = ppsyData->bAttenuationLevelSensor;
    dwAttenuationLevelSensorPosition = 0;
    
    //Niveau crête
    fPeakLevel = 0.0F;
    
    //Attack et decay
    fAttackRate = (float)tan ( sam_PI * 0.5F *  ( 500.0F / fSampleRate ) );                         //500Hz
    fDecayRate = (float)tan ( sam_PI * 0.5F *  ( ( fSampleRate*0.5F - 2.0F ) / fSampleRate ) );     //2Hz
    
    do {
        
        switch (bFormatSource)
        {
            case sam_FORMAT_MONO_PCM8: // PCM 8 bits / mono
                fSignalC = (float)*(BYTE *)pAudioDataSourceCurrent;
                fSignalC = (fSignalC-128.0F) * fComp8bits;
                pAudioDataSourceCurrent += 1;
                break;
               
            case sam_FORMAT_MONO_PCM16: // PCM 16 bits / mono
                fSignalC = (float)*(INT16 *)pAudioDataSourceCurrent;
                fSignalC = fSignalC * fComp16bits;
                pAudioDataSourceCurrent += 2;
                break;

            case sam_FORMAT_MONO_FLOAT32: // FLOAT 32 bits / mono
                fSignalC = *(float *)pAudioDataSourceCurrent;
                pAudioDataSourceCurrent += 4;
                break;
               
            case sam_FORMAT_STEREO_PCM8: // PCM 8 bits / stereo
                fSignalL = (float)*(BYTE *)pAudioDataSourceCurrent;
                fSignalL = (fSignalL-128.0F) * fComp8bits;
                fSignalR = (float)*(BYTE *)(pAudioDataSourceCurrent+1);
                fSignalR = (fSignalR-128.0F) * fComp8bits;
                fSignalC = (fSignalL+fSignalR)*0.5F;
                pAudioDataSourceCurrent += 2;
                break;

            case sam_FORMAT_STEREO_PCM16: // PCM 16 bits / stereo
                fSignalL = (float)*(INT16 *)pAudioDataSourceCurrent;
                fSignalL = fSignalL * fComp16bits;
                fSignalR = (float)*(INT16 *)(pAudioDataSourceCurrent+2);
                fSignalR = fSignalR * fComp16bits;
                fSignalC = (fSignalL+fSignalR)*0.5F;
                pAudioDataSourceCurrent += 4;
                break;

            case sam_FORMAT_STEREO_FLOAT32: // FLOAT 32 bits / stereo
                fSignalL = *(float *)pAudioDataSourceCurrent;
                fSignalR = *(float *)(pAudioDataSourceCurrent+4);
                fSignalC = (fSignalL+fSignalR)*0.5F;
                pAudioDataSourceCurrent += 8;
                break;
        }

        //Filtre ITU-R 468                
        //fSignalC = (fFilterY_Banded[0][0]*0.1F) + (fFilterY_Banded[1][0]*0.9F);
        
        //Redressement
        fTemp = (float)fabs(fSignalC);
        
        //Application du decay
        fPeakLevel *= fDecayRate;
        
        //Mesure de la crête
        if (fTemp>fPeakLevel)
            fPeakLevel += (fTemp-fPeakLevel) * fAttackRate;
            
        //Niveau -120dB
        if (fPeakLevel<0.000001F)
            fPeakLevel = 0.000001F;
            
        //Stockage du niveau actuel
        lPsyPointPosition += 1;
        if (lPsyPointPosition>=lPsyPointDuration)
        {
            //Stockage de la pression globale
            fTemp = (float)(log10 ( fPeakLevel ) * 20.0F);
            fTemp = (float)-floor(fTemp * 2.0F);
            if (fTemp<0) fTemp = 0;
            if (dwAttenuationLevelSensorPosition<1024)
                pbAttenuationLevelSensor[dwAttenuationLevelSensorPosition] = (BYTE)fTemp;
            dwAttenuationLevelSensorPosition += 1;
            
            lPsyPointPosition = 0;
        }
            
        dwCount--;        
    } while (dwCount);

    if (dwAttenuationLevelSensorPosition<1023)
    {
        fTemp = (float)( log10 ( fPeakLevel ) * 20.0F);
        fTemp = (float)-floor(fTemp * 2.0F);
        if (fTemp<0) fTemp = 0;
    
        pbAttenuationLevelSensor[dwAttenuationLevelSensorPosition+1] = (BYTE)fTemp;
    }
    
    return 0;
}



/*
long SAM_EncodePsyAcousticBand_old ( void * pAudioDataSource, BYTE bFormatSource, DWORD dwSamplesCount, float fSampleRate, float fGain, SAM_SFX_PSY * ppsyData )
{
    float fComp8bits;
    float fComp16bits;
    float fSignalL;
    float fSignalR;
    float fSignalC;
    float fTemp, fTemp2;
    float fL, fR;
    
    float   fCoefABValue_Banded[6][16];
    float   *pfCoefABValue;
    long    lCoefABCount_Banded[6];
    float   fFilterX_Banded[6][8];
    float   fFilterY_Banded[6][8];
    long    lFilterCount_Banded[6];
    long    lSampleIndex;
    long    i;
    float   fFilterBanded[7];
    float   fFilterBandedAbs[7];
    long    lFilterBandedCnt[7];
    
    //long    lPsyBanded_Offset;
    //long    lPsyBanded_Value[6];
    
    long    lPsyPointDuration;
    long    lPsyPointPosition;
    
    
    DWORD   *pdwPsychoAcousticData;
    DWORD   dwPsyValue;
    DWORD   dwPsyOffset;
    
    BYTE * pAudioDataSourceCurrent;
    DWORD dwCount;
    
    float   fGlobalLevel;
    
    QWORD ui64A;
    
    fComp8bits  = 1.0 /   128.0;
    fComp16bits = 1.0 / 32768.0;
    fComp8bits  *= fGain;
    fComp16bits *= fGain;
    
    pAudioDataSourceCurrent = (BYTE *)pAudioDataSource;
    dwCount = dwSamplesCount;
    
    if (dwGranulePsychoAcoustic==0xFFFFFFFF)
        return -1;
    
    SAM_GranulesGetData ( dwGranulePsychoAcoustic, &pdwPsychoAcousticData );    
    
    //Calcul de 5 filtres passe-bas
    fFilterBanded[0] = 100;
    fFilterBanded[1] = 250;
    fFilterBanded[2] = 625;
    fFilterBanded[3] = 1560;
    fFilterBanded[4] = 3900;
    fFilterBanded[5] = 6200;
    
    //Convertion en ratio et génération des coef
    for (i=0;i<6;i++)
    {
        SAM_IIR_Design ( 
            1,                                          //Ordre
            fCoefABValue_Banded[i],                     //Les coef
            &lCoefABCount_Banded[i],                    //Nombre de coef
            fFilterBanded[i] / (fSampleRate*0.5F),      //Fréquence de coupure
            0 );                                        //Passe-bas
            
        lFilterCount_Banded[i] = lCoefABCount_Banded[i]; //>>1)+1;
    }

    //On vide les filtres        
    for (i=0;i<6;i++)
    {
        //Décalage des anciens échantillons
        for (lSampleIndex=0;lSampleIndex<lFilterCount_Banded[i];lSampleIndex++)
        {
            fFilterX_Banded[i][lSampleIndex] = 0;
            fFilterY_Banded[i][lSampleIndex] = 0;
        }
    }
    
    for (i=0;i<7;i++)    
    {
        fFilterBandedAbs[i] = 0;
        lFilterBandedCnt[i] = 0;
    }
        
        
    lPsyPointPosition       = 0;
        
    //Par défaut, on a une précision de 20ms
    lPsyPointDuration = (long)floor(fSampleRate * 0.020F);
    
    //Nombre de points pour 20ms de précision
    i = dwSamplesCount / (DWORD)lPsyPointDuration;
    if (i>1019) //Le granule sera plein !
        lPsyPointDuration = (dwSamplesCount/1019)+1;
        
    //Stocke la longueur d'un point 6 bandes :
    *pdwPsychoAcousticData = lPsyPointDuration;
    pdwPsychoAcousticData+=1;
    
    do {
        
        switch (bFormatSource)
        {
            case sam_FORMAT_MONO_PCM8: // PCM 8 bits / mono
                fSignalC = (float)*(BYTE *)pAudioDataSourceCurrent;
                fSignalC = (fSignalC-128.0F) * fComp8bits;
                pAudioDataSourceCurrent += 1;
                break;
               
            case sam_FORMAT_MONO_PCM16: // PCM 16 bits / mono
                fSignalC = (float)*(INT16 *)pAudioDataSourceCurrent;
                fSignalC = fSignalC * fComp16bits;
                pAudioDataSourceCurrent += 2;
                break;

            case sam_FORMAT_MONO_FLOAT32: // FLOAT 32 bits / mono
                fSignalC = *(float *)pAudioDataSourceCurrent;
                pAudioDataSourceCurrent += 4;
                break;
               
            case sam_FORMAT_STEREO_PCM8: // PCM 8 bits / stereo
                fSignalL = (float)*(BYTE *)pAudioDataSourceCurrent;
                fSignalL = (fSignalL-128.0F) * fComp8bits;
                fSignalR = (float)*(BYTE *)(pAudioDataSourceCurrent+1);
                fSignalR = (fSignalR-128.0F) * fComp8bits;
                fSignalC = (fSignalL+fSignalR)*0.5F;
                pAudioDataSourceCurrent += 2;
                break;

            case sam_FORMAT_STEREO_PCM16: // PCM 16 bits / stereo
                fSignalL = (float)*(INT16 *)pAudioDataSourceCurrent;
                fSignalL = fSignalL * fComp16bits;
                fSignalR = (float)*(INT16 *)(pAudioDataSourceCurrent+2);
                fSignalR = fSignalR * fComp16bits;
                fSignalC = (fSignalL+fSignalR)*0.5F;
                pAudioDataSourceCurrent += 4;
                break;

            case sam_FORMAT_STEREO_FLOAT32: // FLOAT 32 bits / stereo
                fSignalL = *(float *)pAudioDataSourceCurrent;
                fSignalR = *(float *)(pAudioDataSourceCurrent+4);
                fSignalC = (fSignalL+fSignalR)*0.5F;
                pAudioDataSourceCurrent += 8;
                break;
        }
        
        fSignalC = rand()&32767;
        
        
        //Application des filtres
        for (i=0;i<6;i++)
        {
            //Décalage des anciens échantillons
            for (lSampleIndex=lFilterCount_Banded[i]-1;lSampleIndex>0;lSampleIndex--)
            {
                fFilterX_Banded[i][lSampleIndex] = fFilterX_Banded[i][lSampleIndex-1];
                fFilterY_Banded[i][lSampleIndex] = fFilterY_Banded[i][lSampleIndex-1];
            }
            
            //Calcule la sortie
            pfCoefABValue         = fCoefABValue_Banded[i];
            fFilterX_Banded[i][0] = fSignalC;
            fFilterY_Banded[i][0] = pfCoefABValue[0] * fFilterX_Banded[i][0];
            pfCoefABValue+=2;
            for (lSampleIndex=1;lSampleIndex<lFilterCount_Banded[i];lSampleIndex++)
            {
                fFilterY_Banded[i][0] += 
                    pfCoefABValue[0] * fFilterX_Banded[i][lSampleIndex] -
                    pfCoefABValue[1] * fFilterY_Banded[i][lSampleIndex];
                pfCoefABValue+=2;
            }
        }
            
        //Filtre 6 bandes
        fFilterBanded[0] = fFilterY_Banded[0][0];                               //   DC -  100Hz
        fFilterBanded[1] = fFilterY_Banded[1][0] - fFilterY_Banded[0][0];       //  100 -  250Hz
        fFilterBanded[2] = fFilterY_Banded[2][0] - fFilterY_Banded[1][0];       //  250 -  625Hz
        fFilterBanded[3] = fFilterY_Banded[3][0] - fFilterY_Banded[2][0];       //  625 - 1560Hz
        fFilterBanded[4] = fFilterY_Banded[4][0] - fFilterY_Banded[3][0];       // 1560 - 3900Hz
        fFilterBanded[5] = fSignalC              - fFilterY_Banded[4][0];       // 3900 - fs/2Hz
        fFilterBanded[6] = fFilterY_Banded[5][0] - fFilterY_Banded[3][0];       // 1560 - 6200Hz
        //On récupère la valeur crête des 6+1 bandes
        for (i=0;i<7;i++)
        {
            fFilterBandedAbs[i] += ( fFilterBanded[i]*fFilterBanded[i] );
            lFilterBandedCnt[i] += 1;
        }
            
        //Stockage des valeurs...
        lPsyPointPosition += 1;
        if (lPsyPointPosition>=lPsyPointDuration)
        {
            //Convertion en dB            
            for (i=0;i<7;i++)
            {
                fTemp = lFilterBandedCnt[i];
                fTemp = sqrt ( fFilterBandedAbs[i] / fTemp );
                fTemp = log10 ( fTemp ) * 20.0F;
                fFilterBandedAbs[i] = fTemp;
            }
            
            //Compensation Equi-loudness de pondération "ITU-R 468"
            fFilterBandedAbs[0] -= 25;//-25;      //   DC -  100Hz
            fFilterBandedAbs[1] -= 15;//-10;      //  100 -  250Hz
            fFilterBandedAbs[2] -=  7; //-2;      //  250 -  625Hz
            fFilterBandedAbs[3] +=  2; //-0       //  625 - 1560Hz
            fFilterBandedAbs[4] +=  8; //+3       // 1560 - 3900Hz
            fFilterBandedAbs[5] -= 15; //-5       // 3900 - fs/2Hz
            fFilterBandedAbs[6] +=  2; //-5       // 1560 - 6200Hz
            
            //Pression globale
            fGlobalLevel = 0;
            for (i=0;i<7;i++)
            {
                fTemp = fFilterBandedAbs[i];
                fGlobalLevel += (float)pow ( 10.0F, (fTemp*0.1F) );
            }
            fGlobalLevel = (long)floor ( log10 ( fGlobalLevel ) * 10.0F );
            
            //Stockage de la pression globale
            *((float *)pdwPsychoAcousticData) = fGlobalLevel;
                
            //On vide la mesure des niveaux des 5 bandes
            for (i=0;i<7;i++)
            {
                fFilterBandedAbs[i] = 0;                
                lFilterBandedCnt[i] = 0;
            }
            
            pdwPsychoAcousticData += 1;
            
            lPsyPointPosition = 0;
        }
            
            
            
        
        
        fSignalC = fFilterY_Banded[5][0] - fFilterY_Banded[1][0];
        
        if (bFormatSource==sam_FORMAT_MONO_PCM16) // PCM 16 bits / mono
        {
            *(INT16 *)(pAudioDataSourceCurrent-2) = fSignalC * 32768;
        }
        else if (bFormatSource==sam_FORMAT_STEREO_PCM16) // PCM 16 bits / stereo
        {
            *(INT16 *)(pAudioDataSourceCurrent-4) = fSignalC * 32768;            
            *(INT16 *)(pAudioDataSourceCurrent-2) = fSignalC * 32768;
        }
        
        
       
        dwCount--;        
    } while (dwCount);
    
    return 0;
}
*/

long SAM_PreCopyAudioData ( void * pAudioDataSource, BYTE bFormatSource, DWORD dwSamplesCount, float fSampleRate, float * pfGain, SAM_SFX_PSY * ppsyData )
{
    float fComp8bits;
    float fComp16bits;
    float fSignalL;
    float fSignalR;
    float fSignalC;
    float fSignalPeak;
    float fTemp, fTemp2;
    float fL, fR;
    
    float   fCoefABValue_Banded[5][16];
    float   *pfCoefABValue;
    long    lCoefABCount_Banded[5];
    float   fFilterX_Banded[5][8];
    float   fFilterY_Banded[5][8];
    long    lFilterCount_Banded[5];
    long    lSampleIndex;
    long    i;
    float   fFilterBanded[6];
    float   fFilterBandedAbs[6];
    
    BYTE * pAudioDataSourceCurrent;
    DWORD dwCount;
    
    fSignalPeak = 0.000001F; //-120 dB
    fComp8bits  = 1.0 /   128.0;
    fComp16bits = 1.0 / 32768.0;
    
    pAudioDataSourceCurrent = (BYTE *)pAudioDataSource;
    dwCount = dwSamplesCount;
    
    
            
            
    
    do {
        
        switch (bFormatSource)
        {
            case sam_FORMAT_MONO_PCM8: // PCM 8 bits / mono
                fSignalC = (float)*(BYTE *)pAudioDataSourceCurrent;
                fSignalC = (fSignalC-128.0F) * fComp8bits;
                fSignalL = fSignalC;
                fSignalR = fSignalC;
                pAudioDataSourceCurrent += 1;
                break;
               
            case sam_FORMAT_MONO_PCM16: // PCM 16 bits / mono
                fSignalC = (float)*(INT16 *)pAudioDataSourceCurrent;
                fSignalC = fSignalC * fComp16bits;
                fSignalL = fSignalC;
                fSignalR = fSignalC;
                pAudioDataSourceCurrent += 2;
                break;

            case sam_FORMAT_MONO_FLOAT32: // FLOAT 32 bits / mono
                fSignalC = *(float *)pAudioDataSourceCurrent;
                fSignalL = fSignalC;
                fSignalR = fSignalC;
                pAudioDataSourceCurrent += 4;
                break;
               
            case sam_FORMAT_STEREO_PCM8: // PCM 8 bits / stereo
                fSignalL = (float)*(BYTE *)pAudioDataSourceCurrent;
                fSignalL = (fSignalL-128.0F) * fComp8bits;
                fSignalR = (float)*(BYTE *)(pAudioDataSourceCurrent+1);
                fSignalR = (fSignalR-128.0F) * fComp8bits;
                fSignalC = (fSignalL+fSignalR)*0.5F;
                pAudioDataSourceCurrent += 2;
                break;

            case sam_FORMAT_STEREO_PCM16: // PCM 16 bits / stereo
                fSignalL = (float)*(INT16 *)pAudioDataSourceCurrent;
                fSignalL = fSignalL * fComp16bits;
                fSignalR = (float)*(INT16 *)(pAudioDataSourceCurrent+2);
                fSignalR = fSignalR * fComp16bits;
                fSignalC = (fSignalL+fSignalR)*0.5F;
                pAudioDataSourceCurrent += 4;
                break;

            case sam_FORMAT_STEREO_FLOAT32: // FLOAT 32 bits / stereo
                fSignalL = *(float *)pAudioDataSourceCurrent;
                fSignalR = *(float *)(pAudioDataSourceCurrent+4);
                fSignalC = (fSignalL+fSignalR)*0.5F;
                pAudioDataSourceCurrent += 8;
                break;
        }
        
        fL = (fSignalL<0)?(-fSignalL):(fSignalL);
        fR = (fSignalR<0)?(-fSignalR):(fSignalR);
        if (fL>fSignalPeak) fSignalPeak = fL;
        if (fR>fSignalPeak) fSignalPeak = fR;
        
        dwCount--;        
    } while (dwCount);
    
    *pfGain = 1.0F / fSignalPeak;    
    
    SAM_EncodePsyAcousticBand ( 
        pAudioDataSource, 
        bFormatSource, 
        dwSamplesCount, 
        fSampleRate, 
        *pfGain, 
        ppsyData );
    
    return 0;
}

long SAM_CopyAudioData ( void * pAudioDataTarget, BYTE bFormatTarget, void * pAudioDataSource, BYTE bFormatSource, DWORD dwSamplesCount, float fGain )
{
    float fComp8bits;
    float fComp16bits;
    float fSignalL;
    float fSignalR;
    float fSignalC;
    


    float fSavedSignalL[4];
    float fSavedSignalR[4];
    float fSavedSignalC[4];

    BYTE * pAudioDataSourceCurrent;
    BYTE * pAudioDataTargetCurrent;

    DWORD dwXD4Count;

    DWORD dwCount;
    
    for (dwCount=0;dwCount<4;dwCount++)
    {
        fSavedSignalL[dwCount] = 0.0F;
        fSavedSignalR[dwCount] = 0.0F;
        fSavedSignalC[dwCount] = 0.0F;
    }
    
    fComp8bits  = 1.0 /   128.0;
    fComp16bits = 1.0 / 32768.0;
    fComp8bits  *= fGain;
    fComp16bits *= fGain;
    
    pAudioDataSourceCurrent = (BYTE *)pAudioDataSource;
    pAudioDataTargetCurrent = (BYTE *)pAudioDataTarget;

    dwXD4Count = 0;
    dwCount = dwSamplesCount;
    
    

   
    do {

        
        switch (bFormatSource)
        {
            case sam_FORMAT_MONO_PCM8: // PCM 8 bits / mono
                fSignalC = (float)*(BYTE *)pAudioDataSourceCurrent;
                fSignalC = (fSignalC-128.0F) * fComp8bits;
                fSignalL = fSignalC;
                fSignalR = fSignalC;
                pAudioDataSourceCurrent += 1;
                break;
               
            case sam_FORMAT_MONO_PCM16: // PCM 16 bits / mono
                fSignalC = (float)*(INT16 *)pAudioDataSourceCurrent;
                fSignalC = fSignalC * fComp16bits;
                fSignalL = fSignalC;
                fSignalR = fSignalC;
                pAudioDataSourceCurrent += 2;
                break;

            case sam_FORMAT_MONO_FLOAT32: // FLOAT 32 bits / mono
                fSignalC = *(float *)pAudioDataSourceCurrent;
                fSignalC = fSignalC * fGain;
                fSignalL = fSignalC;
                fSignalR = fSignalC;
                pAudioDataSourceCurrent += 4;
                break;
               
            case sam_FORMAT_STEREO_PCM8: // PCM 8 bits / stereo
                fSignalL = (float)*(BYTE *)pAudioDataSourceCurrent;
                fSignalL = (fSignalL-128.0F) * fComp8bits;
                fSignalR = (float)*(BYTE *)(pAudioDataSourceCurrent+1);
                fSignalR = (fSignalR-128.0F) * fComp8bits;
                fSignalC = (fSignalL+fSignalR)*0.5F;
                pAudioDataSourceCurrent += 2;
                break;

            case sam_FORMAT_STEREO_PCM16: // PCM 16 bits / stereo
                fSignalL = (float)*(INT16 *)pAudioDataSourceCurrent;
                fSignalL = fSignalL * fComp16bits;
                fSignalR = (float)*(INT16 *)(pAudioDataSourceCurrent+2);
                fSignalR = fSignalR * fComp16bits;
                fSignalC = (fSignalL+fSignalR)*0.5F;
                pAudioDataSourceCurrent += 4;
                break;

            case sam_FORMAT_STEREO_FLOAT32: // FLOAT 32 bits / stereo
                fSignalL = *(float *)pAudioDataSourceCurrent;
                fSignalL = fSignalL * fGain;
                fSignalR = *(float *)(pAudioDataSourceCurrent+4);
                fSignalR = fSignalR * fGain;
                fSignalC = (fSignalL+fSignalR)*0.5F;
                pAudioDataSourceCurrent += 8;
                break;

        }
        
        fSavedSignalL[3] = fSavedSignalL[2];
        fSavedSignalL[2] = fSavedSignalL[1];
        fSavedSignalL[1] = fSavedSignalL[0];
        fSavedSignalL[0] = fSignalL;

        fSavedSignalR[3] = fSavedSignalR[2];
        fSavedSignalR[2] = fSavedSignalR[1];
        fSavedSignalR[1] = fSavedSignalR[0];
        fSavedSignalR[0] = fSignalR;

        fSavedSignalC[3] = fSavedSignalC[2];
        fSavedSignalC[2] = fSavedSignalC[1];
        fSavedSignalC[1] = fSavedSignalC[0];
        fSavedSignalC[0] = fSignalC;

        switch (bFormatTarget)
         {
            case sam_FORMAT_MONO_FLOAT32: // FLOAT 32 bits / mono
                *(float *)pAudioDataTargetCurrent = fSignalC;
                pAudioDataTargetCurrent += 4;
                break;
                
            case sam_FORMAT_MONO_XPCM8: // XPCM 8 bits / mono
                *pAudioDataTargetCurrent = SAM_XPCM_EncodeValue ( fSignalC );
                pAudioDataTargetCurrent += 1;
                break;

            case sam_FORMAT_MONO_PCM16: // PCM 16 bits / mono
                *(WORD *)pAudioDataTargetCurrent = SAM_LPCM_EncodeValue16 ( fSignalC );
                pAudioDataTargetCurrent += 2;
                break;

            case sam_FORMAT_MONO_XD4: // XD4 / Mono
                if (dwXD4Count==3)
                {
                    *(DWORD *)pAudioDataTargetCurrent = SAM_XD4_EncodeValue (
                        fSavedSignalC[3],
                        fSavedSignalC[2],
                        fSavedSignalC[1],
                        fSavedSignalC[0] );
                    pAudioDataTargetCurrent += 4;
                }
                break;

            case sam_FORMAT_MONO_XD4ADPCM: // XD4ADPCM / Mono
                if (dwXD4Count==3)
                {
                    *(DWORD *)pAudioDataTargetCurrent = SAM_XD4ADPCM_EncodeValue (
                        fSavedSignalC[3],
                        fSavedSignalC[2],
                        fSavedSignalC[1],
                        fSavedSignalC[0] );
                    pAudioDataTargetCurrent += 4;
                }
                break;
                
            case sam_FORMAT_STEREO_FLOAT32: // FLOAT 32 bits / stereo
                *(float *)pAudioDataTargetCurrent     = fSignalL;
                *(float *)(pAudioDataTargetCurrent+4) = fSignalR;
                pAudioDataTargetCurrent += 8;
                break;
                
            case sam_FORMAT_STEREO_XPCM8: // XPCM 8 bits / stereo
                pAudioDataTargetCurrent[0] = SAM_XPCM_EncodeValue ( fSignalL );
                pAudioDataTargetCurrent[1] = SAM_XPCM_EncodeValue ( fSignalR );
                pAudioDataTargetCurrent += 2;
                break;
                
            case sam_FORMAT_STEREO_PCM16: // PCM 16 bits / stereo
                *(WORD *)pAudioDataTargetCurrent        = SAM_LPCM_EncodeValue16 ( fSignalL );
                *(WORD *)(pAudioDataTargetCurrent+2)    = SAM_LPCM_EncodeValue16 ( fSignalR );
                pAudioDataTargetCurrent += 4;
                break;

            case sam_FORMAT_STEREO_XD4: // XD4 / Stereo
                if (dwXD4Count==3)
                {
                    *(DWORD *)pAudioDataTargetCurrent = SAM_XD4_EncodeValue (
                        fSavedSignalL[3],
                        fSavedSignalL[2],
                        fSavedSignalL[1],
                        fSavedSignalL[0] );
                    pAudioDataTargetCurrent += 4;
                    *(DWORD *)pAudioDataTargetCurrent = SAM_XD4_EncodeValue (
                        fSavedSignalR[3],
                        fSavedSignalR[2],
                        fSavedSignalR[1],
                        fSavedSignalR[0] );
                    pAudioDataTargetCurrent += 4;

                }
                break;

        }
        
        dwCount--;        
        dwXD4Count = (dwXD4Count+1)&3;
    } while (dwCount);

    return 0;
}