#include "sam_render.h"


int SAM_RENDER_DelayLineSortByLevelProc ( const void * pElementA, const void * pElementB )
{
    DELAYLINE   *pDelayLineChannelA;
    DELAYLINE   *pDelayLineChannelB;
    float       fLevelA;
    float       fLevelB;
    long        lLevelA;
    long        lLevelB;

    pDelayLineChannelA = (DELAYLINE *)pElementA;
    pDelayLineChannelB = (DELAYLINE *)pElementB;
    
    fLevelA = (float)fabs ( pDelayLineChannelA->fDelayGain );
    fLevelB = (float)fabs ( pDelayLineChannelB->fDelayGain );
    
    if (fLevelA>0.0F) fLevelA = (float)log10 ( fLevelA ) * 20.0F;
    else              fLevelA = -140.0F;
    
    if (fLevelB>0.0F) fLevelB = (float)log10 ( fLevelB ) * 20.0F;
    else              fLevelB = -140.0F;
    
    lLevelA = (long)floor ( fLevelA * 10.0F );
    lLevelB = (long)floor ( fLevelB * 10.0F );
    
    return lLevelB - lLevelA;
}


void SAM_RENDER_LimitTotalAmountDelayCount ( SAM_RENDER254 * pRender, DWORD dwEntryCountMax )
{
    DELAYLINE DelayLineChannelL[128];
    DELAYLINE DelayLineChannelR[128];
    DWORD dwIndex;
    
    //Le limiteur de lignes à retard ne fonctionne qu'avec les modes hybrid hrtf
    if (samData.lWithoutStackMode)
        return;
        
    //On a un rendu final avec 2 canaux... Donc on a un nombre de lignes à retard toujours paires
    dwEntryCountMax &= 0xFE;

    //Avons-nous des lignes à retard à supprimer ?
    if (dwEntryCountMax>pRender->dwEntryCount)
        return;
        
    //On copie les lignes à retard...
    for (dwIndex=0;dwIndex<(pRender->dwEntryCount)>>1;dwIndex++)
    {
        DelayLineChannelL[dwIndex].dwDelayIndex = pRender->dwDelayIndex[(dwIndex<<1)  ];
        DelayLineChannelR[dwIndex].dwDelayIndex = pRender->dwDelayIndex[(dwIndex<<1)+1];
        DelayLineChannelL[dwIndex].fDelayGain   = pRender->fDelayGain[(dwIndex<<1)  ];
        DelayLineChannelR[dwIndex].fDelayGain   = pRender->fDelayGain[(dwIndex<<1)+1];
    }
    
    //On effectue un tri des lignes à retard...
    qsort (
        DelayLineChannelL,
        (pRender->dwEntryCount)>>1,
        sizeof(DELAYLINE),
        SAM_RENDER_DelayLineSortByLevelProc );
    qsort (
        DelayLineChannelR,
        (pRender->dwEntryCount)>>1,
        sizeof(DELAYLINE),
        SAM_RENDER_DelayLineSortByLevelProc );        
        
    //Copie les nouvelles lignes à retard de la plus audible à la moins audible
    for (dwIndex=0;dwIndex<dwEntryCountMax>>1;dwIndex++)
    {
        pRender->dwDelayIndex[(dwIndex<<1)  ] = DelayLineChannelL[dwIndex].dwDelayIndex;
        pRender->dwDelayIndex[(dwIndex<<1)+1] = DelayLineChannelR[dwIndex].dwDelayIndex;
        pRender->fDelayGain[(dwIndex<<1)  ]   = DelayLineChannelL[dwIndex].fDelayGain;
        pRender->fDelayGain[(dwIndex<<1)+1]   = DelayLineChannelR[dwIndex].fDelayGain;
    }
    
    //Les autres lignes à retard sont vidées
    for (;dwIndex<(pRender->dwEntryCount)>>1;dwIndex++)
    {
        pRender->dwDelayIndex[(dwIndex<<1)  ] = 0;
        pRender->dwDelayIndex[(dwIndex<<1)+1] = 0;
        pRender->fDelayGain[(dwIndex<<1)  ]   = 0.0F;
        pRender->fDelayGain[(dwIndex<<1)+1]   = 0.0F;
    }
    
    //On assigne le nouveau nombre de lignes à retard
    pRender->dwEntryCount = dwEntryCountMax;
}

void SAM_RENDER_LimitTotalAmountDelayCount_2Kto64 ( SAM_RENDER254 * pRenderTarget, SAM_RENDER2K * pRender2KSource, DWORD dwEntryCountMax )
{
    DELAYLINE DelayLineChannelL[1024];
    DELAYLINE DelayLineChannelR[1024];
    DWORD dwIndex;
    
    //Le limiteur de lignes à retard ne fonctionne qu'avec les modes hybrid hrtf
    //if (samData.lWithoutStackMode)
    //    return;
        
    //On a un rendu final avec 2 canaux... Donc on a un nombre de lignes à retard toujours paires
    dwEntryCountMax &= 0xFFFE;

    //Avons-nous des lignes à retard à supprimer ?
    if (dwEntryCountMax>pRender2KSource->dwEntryCount)
        dwEntryCountMax = pRender2KSource->dwEntryCount;
    
    memset ( DelayLineChannelL, 0, sizeof(DELAYLINE)*1024 );
    memset ( DelayLineChannelR, 0, sizeof(DELAYLINE)*1024 );
        
    //On copie les lignes à retard...
    for (dwIndex=0;dwIndex<(pRender2KSource->dwEntryCount)>>1;dwIndex++)
    {
        DelayLineChannelL[dwIndex].dwDelayIndex = pRender2KSource->dwDelayIndex[(dwIndex<<1)  ];
        DelayLineChannelR[dwIndex].dwDelayIndex = pRender2KSource->dwDelayIndex[(dwIndex<<1)+1];
        DelayLineChannelL[dwIndex].fDelayGain   = pRender2KSource->fDelayGain[(dwIndex<<1)  ];
        DelayLineChannelR[dwIndex].fDelayGain   = pRender2KSource->fDelayGain[(dwIndex<<1)+1];
    }
    
    //On effectue un tri des lignes à retard...
    qsort (
        DelayLineChannelL,
        (pRender2KSource->dwEntryCount)>>1,
        sizeof(DELAYLINE),
        SAM_RENDER_DelayLineSortByLevelProc );
    qsort (
        DelayLineChannelR,
        (pRender2KSource->dwEntryCount)>>1,
        sizeof(DELAYLINE),
        SAM_RENDER_DelayLineSortByLevelProc );        
        
    //Copie les nouvelles lignes à retard de la plus audible à la moins audible
    for (dwIndex=0;dwIndex<dwEntryCountMax>>1;dwIndex++)
    {
        pRenderTarget->dwDelayIndex[(dwIndex<<1)  ] = DelayLineChannelL[dwIndex].dwDelayIndex;
        pRenderTarget->dwDelayIndex[(dwIndex<<1)+1] = DelayLineChannelR[dwIndex].dwDelayIndex;
        pRenderTarget->fDelayGain[(dwIndex<<1)  ]   = DelayLineChannelL[dwIndex].fDelayGain;
        pRenderTarget->fDelayGain[(dwIndex<<1)+1]   = DelayLineChannelR[dwIndex].fDelayGain;
    }
    
    //On assigne le nouveau nombre de lignes à retard
    pRenderTarget->dwEntryCount = dwEntryCountMax;
}


static float pfChannelDataStatic_48000_8[48000*8*2];

void SAM_RENDER_MaximizeLevel ( SAM_RENDER254 * pRender, long lChannelCount, long lSampleRate )
{
    DWORD   dwBufferStackPosition;
    float   fBufferStackValue[sam_VOICEBUFFERSTACK_COUNT];
    DWORD   dwRenderEntryIndex;

    float   * pfChannelData;
    float   * pfBufferOut;

    long    lSampleCount;
    long    lByteSize;
    long    lIndex, lChannelIndex;

    QWORD   ui64A;
    float   fTemp, fSum, fLev, fRatio;
    FLOAT64 f64Temp;
    FLOAT64 f64ChannelRMS[16]; //16 canaux max

    lSampleCount = lSampleRate*2;
    
    //Aucune maximisation en mode multi-canaux
    //if (samData.lWithoutStackMode)
    //    return;
    
    //Limitation du nombre de lignes à retard
    SAM_RENDER_LimitTotalAmountDelayCount ( pRender, 128 );
    
    //Allocation du tampon de traitement
    lByteSize     = lChannelCount * lSampleCount * sizeof(float);
    pfChannelData = pfChannelDataStatic_48000_8; //(float *)SAM_KERNEL_ALLOC ( lByteSize );
    memset ( pfChannelData, 0, lByteSize );

    //On vide la pile
    memset ( fBufferStackValue, 0, sizeof(float) * sam_VOICEBUFFERSTACK_COUNT );
    dwBufferStackPosition = 0;

    //Traîtement des échantillons
    ui64A       = 0;
    pfBufferOut = pfChannelData;
    for (lIndex=0;lIndex<lSampleCount;lIndex++)
    {
        //Génération de bruit blanc (-1 à +1)
        ui64A = 279470273 * ui64A + 4294967291U;
        fTemp = (float)(ui64A&0xFFFF);
        fTemp -= 32768;
        fTemp /= 32768;

        //On stocke dans la pile
        dwBufferStackPosition = (dwBufferStackPosition+1)&sam_VOICEBUFFERSTACK_MASK;
        fBufferStackValue[dwBufferStackPosition] = fTemp;

        //On envoie sur la sortie avec le générateur de rendu
        if (samData.lWithoutStackMode)
        {
            //Modes 2ch to 6ch
            for (dwRenderEntryIndex=0;dwRenderEntryIndex<pRender->dwEntryCount;dwRenderEntryIndex++)
            {
                pfBufferOut[dwRenderEntryIndex] +=
                    fBufferStackValue[(dwBufferStackPosition-pRender->dwDelayIndex[dwRenderEntryIndex])&sam_VOICEBUFFERSTACK_MASK] *
                    pRender->fDelayGain[dwRenderEntryIndex];
            }
        }
        else
        {
            //Modes 2ch hybrid hrtf
            for (dwRenderEntryIndex=0;dwRenderEntryIndex<pRender->dwEntryCount;dwRenderEntryIndex++)
            {
                pfBufferOut[dwRenderEntryIndex&1] +=
                    fBufferStackValue[(dwBufferStackPosition-pRender->dwDelayIndex[dwRenderEntryIndex])&sam_VOICEBUFFERSTACK_MASK] *
                    pRender->fDelayGain[dwRenderEntryIndex];
            }        
        }
        //La position suivante
        pfBufferOut += lChannelCount;
    }

    //Pour chaque canal, on applique un filtre passe-bas...
    fRatio = 8000 / ((float)lSampleRate/2);
    for (lChannelIndex=0;lChannelIndex<lChannelCount;lChannelIndex++)
    {
        pfBufferOut = pfChannelData + lChannelIndex;
        fTemp = 0;
        for (lIndex=0;lIndex<lSampleCount;lIndex++)
        {
            fTemp += ( (*pfBufferOut)-fTemp ) * fRatio;
            *pfBufferOut = fTemp;
            pfBufferOut += lChannelCount;
        }

    }

    //Calcul des valeurs RMS...
    pfBufferOut = pfChannelData;
    for (lChannelIndex=0;lChannelIndex<lChannelCount;lChannelIndex++)
        f64ChannelRMS[lChannelIndex] = 0.0;

    for (lIndex=0;lIndex<lSampleCount;lIndex++)
    {
        //Relevé du niveau de chacun des canaux
        for (lChannelIndex=0;lChannelIndex<lChannelCount;lChannelIndex++)
        {
            f64Temp = (FLOAT64)*pfBufferOut;
            f64ChannelRMS[lChannelIndex] += f64Temp*f64Temp;
            pfBufferOut++;
        }
    }

    //Calcul du niveau RMS
    fSum = 0;
    for (lChannelIndex=0;lChannelIndex<lChannelCount;lChannelIndex++)
    {
        //Calcul du niveau RMS
        f64Temp = f64ChannelRMS[lChannelIndex];
        f64Temp = sqrt ( f64Temp / (FLOAT64)lSampleCount );
        fTemp   = (float)f64Temp;

        fSum += fTemp;
    }

    //Calcul de la compensation de niveau
    if (fSum>0) fLev = 1/fSum;
    else        fLev = 1.0;

    //Application de la compensation de niveau
    /*for (lChannelIndex=0;lChannelIndex<lChannelCount;lChannelIndex++)
    {
        for (dwRenderEntryIndex=0;dwRenderEntryIndex<pRender->dwEntryCount;dwRenderEntryIndex++)
        {
            //if (pRender->[dwRenderEntryIndex]==(unsigned)lChannelIndex)
            {
                pRender->fDelayGain[dwRenderEntryIndex] *= fLev;
            }
        }
    }
    */
    
    for (dwRenderEntryIndex=0;dwRenderEntryIndex<pRender->dwEntryCount;dwRenderEntryIndex++)
    {
        pRender->fDelayGain[dwRenderEntryIndex] *= fLev;
    }
    

    //SAM_KERNEL_FREE ( pfChannelData );
}
 

long Convolution_GetCount ( T_CONVOLUTION * pConvolution, long lSampleRate )
{
    long i;
    
    do {
        if (pConvolution->lSampleRate==lSampleRate)
            return pConvolution->lCount;
        
        pConvolution++;
    } while (pConvolution->lCount);    
    return 0;
}

float Convolution_GetValue ( T_CONVOLUTION * pConvolution, long lSampleRate, long lIndex )
{
    long i;
    
    do {
        if (pConvolution->lSampleRate==lSampleRate)
        {
            if (lIndex>=pConvolution->lCount) return 0;
            else return pConvolution->fValue[lIndex];
        }
        
        pConvolution++;
    } while (pConvolution->lCount);    
    return 0;
}

//float render_SineBinauralValue[2][32];


/*
    Encodage Dolby Surround / Pro logic

    Rear
        +/-90°
        Hilbert Transform

Type : Uncle Hilbert
References : Posted by Christian[at]savioursofsoul[dot]de

Notes : 
This is the delphi code to create the filter coefficients, which are needed to phaseshift a signal by 90°
This may be useful for an evelope detector...

By windowing the filter coefficients you can trade phase response flatness with magnitude response flatness.

I had problems checking its response by using a dirac impulse. White noise works fine.

Also this introduces a latency of N/2!

Code : 
type TSingleArray = Array of Single;

procedure UncleHilbert(var FilterCoefficients: TSingleArray; N : Integer);
var i,j : Integer;
begin
SetLength(FilterCoefficients,N);
for i:=0 to (N div 4) do
  begin
   FilterCoefficients[(N div 2)+(2*i-1)]:=+2/(PI*(2*i-1));
   FilterCoefficients[(N div 2)-(2*i-1)]:=-2/(PI*(2*i-1));
  end;
end;



*/


/*

    Headphone rendering

        ITD = Interaural Time Difference
        IIT/ILD = Interaural Intensity/Level Difference
        Bauer delay = Use multiple delay line to simulate HRTF
        Surround spatialisation = Apply a surround+reverd effect to rear sounds

*/
void SAM_RENDER_InitBinaural ( float fSineBinauralValue[2][32], float fDistanceValue, float fSampleRate, long lBinauralModel )
{
    float   fBinauralEffectGain;
    float   fBinauralEffectIncr;
    float   f1, f2, f3, f4;
    float   fA, fB;
    float   fGain, fGainL, fGainR;
    UINT32  ui32Random;
    long    i;

    ui32Random          = 0;
    fBinauralEffectGain = -50; //-35;//0;
    fBinauralEffectIncr = 2.0;//1.3; //3;
    for (i=0;i<32;i++)
    {
        ui32Random = ( 1664525 * ui32Random ) + 1013904223;

        // Base Gain
        fGain = (float)pow ( 10, (fBinauralEffectGain) * 0.05 ); 
        
        // Gain modulation
        f1 = (float)((ui32Random>> 0)&65535);
        f2 = (float)((ui32Random>>16)&65535);
        f1 = (float)sin ( f1 );
        f2 = (float)sin ( f2 );

        if (lBinauralModel) //Headphone
        {
            fGain = (fGain*2 + (( fGain*sam_ABS(f2) + f1 ) * 0.5F))*0.3F;
        }
        else                //360 Virtual Sound
        {
            fGain *= f1;
            if (i&1) fGain *= 0.5F; //-6dB
            if (i&2) fGain *= 0.7F; //-3dB
        }
        
        // Surround effect (with variable phase)
        f1 = (float)((ui32Random>> 8)&255);
        f2 = (float)((ui32Random>>16)&255);
        fGainL  = (float)sin ( f1 );
        fGainR  = (float)sin ( f2 );

        // Surround effect (with fixed phase)
        fA = (float)((ui32Random>> 0)&255);
        fB = (float)((ui32Random>>24)&255);
        f3      = sam_ABS ( (float)sin ( fA ) );
        f4      = sam_ABS ( (float)sin ( fB ) );

        // Progressive surround based on decay
        fA      = ((float)i)/31; //0...1
        fB      = 1-fA;
        f3      = (fGainL*3.0F+f3)*0.25F;
        f4      = (fGainR*3.0F+f4)*0.25F;
        fGainL  = fGainL*fA + f3*fB;
        fGainR  = fGainR*fA + f4*fB;

        // Output gain
        f1 = fGainL * fGain * 2;
        f2 = fGainR * fGain * 2;
        fSineBinauralValue[0][i] = f1;
        fSineBinauralValue[1][i] = f2;


        // Redo processing if gain is below -80dB
        if ( (sam_ABS(f1)<0.0001) && (sam_ABS(f2)<0.0001) )
        {
            i--;
        }
        else
        {
            // Apply decay to gain
            fBinauralEffectGain += fBinauralEffectIncr;
            fBinauralEffectIncr *= 0.98F; //0.918;
        }
    }
}



void SAM_RENDER_GetAngleToRender ( SAM_RENDER254 * psamRenderTable, long lAngleDegrees, SAM_RENDER254 ** psamRenderSet, long * plIndexRender )
{
    
    if (lAngleDegrees>0) //L'angle est positif
    {
        //On passe à un angle de 0...359
        lAngleDegrees = lAngleDegrees%360;

        //La partie supérieur à 180 passe à -180
        if (lAngleDegrees>180)
            lAngleDegrees = lAngleDegrees-360;
    }
    else if (lAngleDegrees<0) //L'angle est négatif
    {
        //On passe à un angle de -0...-359        
        lAngleDegrees = lAngleDegrees%360;

        //La partie inférieur à -180 passe à 180
        if (lAngleDegrees<-180)
            lAngleDegrees = lAngleDegrees+360;
    }

    //Convertion dans l'index
    lAngleDegrees = (lAngleDegrees+180)*8/45;
    lAngleDegrees = lAngleDegrees&63;

    if (psamRenderSet)
        *psamRenderSet = psamRenderTable+lAngleDegrees;
    if (plIndexRender)
        *plIndexRender = lAngleDegrees;
}



float fCorrectOffsetDC_DataA[65536];
float fCorrectOffsetDC_DataB[32768*8];


void SAM_RENDER_CorrectOffsetDC ( SAM_RENDER254 * pRender, DWORD dwChannelCount )
{
    /*long i;
    float f1;
    double fSum, fSumInitial;
    double fSumTable[8];
    DWORD dwIndex;
    float * pfBufferOut;
    float   fBufferStackValue[sam_VOICEBUFFERSTACK_COUNT];
    DWORD   dwBufferStackPosition;
    
    fSum = 0;
    for (i=0;i<65536;i++)
    {
        f1 = (float)i;
        f1 *= sam_PI * 0.00390625F;
        f1 = sin ( f1 );
        fCorrectOffsetDC_DataA[i] = f1;
        fSum += (double)f1;
    }
    
    fSumInitial = fSum;
    
    for (i=0;i<8;i++)
    {
        fSumTable[i] = 0;
    }
    memset ( fCorrectOffsetDC_DataB, 0, sizeof(float)*8*32768 );
    
    //On vide la pile
    memset ( fBufferStackValue, 0, sizeof(float) * sam_VOICEBUFFERSTACK_COUNT );
    dwBufferStackPosition = 0;
    
    
    pfBufferOut = fCorrectOffsetDC_DataB;
    for (i=0;i<32768;i++)
    {
        //On stocke dans la pile
        dwBufferStackPosition = (dwBufferStackPosition+1)&sam_VOICEBUFFERSTACK_MASK;
        fBufferStackValue[dwBufferStackPosition] = fCorrectOffsetDC_DataA[i];
    
    
        if (samData.lWithoutStackMode)
        {
            for (dwRenderEntryIndex=0;dwRenderEntryIndex<pRender->dwEntryCount;dwRenderEntryIndex++)
            {
                pfBufferOut[dwRenderEntryIndex] +=
                        fBufferStackValue[(dwBufferStackPosition-pRender->dwDelayIndex[dwRenderEntryIndex])&sam_VOICEBUFFERSTACK_MASK] *
                        pRender->fDelayGain[dwRenderEntryIndex];
            }
        }
        else
        {
            //Modes 2ch hybrid hrtf
            for (dwRenderEntryIndex=0;dwRenderEntryIndex<pRender->dwEntryCount;dwRenderEntryIndex++)
            {
                pfBufferOut[dwRenderEntryIndex&1] +=
                    fBufferStackValue[(dwBufferStackPosition-pRender->dwDelayIndex[dwRenderEntryIndex])&sam_VOICEBUFFERSTACK_MASK] *
                    pRender->fDelayGain[dwRenderEntryIndex];
            }        
        }
    }   
    */
}