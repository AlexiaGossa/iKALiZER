#include "sam_render.h"
#include "ca_key.h"
#include "ca_crypt.h"
#include <stdio.h>

//DWORD __0x26rr__dwCryptKey[2] = { 0x773d9f0b, 0xdc70b61e };


#include "sam_render_2ch-impulse-fl.wav.c"
#include "sam_render_2ch-impulse-fr.wav.c"
#include "sam_render_2ch-impulse-fc.wav.c"
#include "sam_render_2ch-impulse-rl.wav.c"
#include "sam_render_2ch-impulse-rr.wav.c"
#include "sam_render_2ch-impulse-rc.wav.c"

float           __0x26__f32Impulse[6][2][2048];
SAM_RENDER254   __0x26__Render254[6];

void IKA_MakeNoise ( float * pfData, long lLength, long lSeed, float fLevel );

void IKA_EqualizeConvolution2Kmax ( float * pfConvL, float * pfConvR, long lConvLength )
{
    long lEqualizeLength;
    float *pfNoise;
    float *pfNoiseConvL;
    float *pfNoiseConvR;
    float fStack[2048];
    
    long lStackPos;
    long i, j, k, n;
    float f1, f2, fL, fR, fLevel, fDiv;
    void    *pFFTInfo;
    
    float fWindow[4096];
    float fNoiseFreeTime[4096];
    float fNoiseConvTime[2][4096];
    float fNoiseFreeLevelFreq[2048];
    float fNoiseConvLevelFreq[2048];
    float fEqual[2048];
    
    float fImpulseTime[4096];        
    float fImpulseReal[2048];
    float fImpulseImag[2048];
    
    float *pfConv;
    
    //La fenêtre Kaiser-Bessel Soft
    Window_FullBuild_f32 ( 1, 4096, fWindow );    

    //Allocations...        
    FFT_Alloc ( &pFFTInfo, 4096 );   
    lEqualizeLength = 100000;
    pfNoise      = (float *)malloc ( sizeof(float) * lEqualizeLength );
    pfNoiseConvL = (float *)malloc ( sizeof(float) * lEqualizeLength );
    pfNoiseConvR = (float *)malloc ( sizeof(float) * lEqualizeLength );

    //Génération du bruit blanc...    
    IKA_MakeNoise ( pfNoise, lEqualizeLength, 0, 1.0F );

    
    //Pré-remplissage de la pile
    memcpy ( fStack, &pfNoise[lEqualizeLength-2048], 2048 * sizeof(float) );
    lStackPos = 0;
    
    //Traitement
    for (i=0;i<lEqualizeLength;i++)
    {
        //Lecture des échantillons
        fStack[lStackPos] = pfNoise[i];
        
        //Convolution
        fL = 0;
        fR = 0;
        for (k=0;k<lConvLength;k++)
        {
            f1 = fStack[(lStackPos-k)&2047];
            fL += f1 * pfConvL[k];
            fR += f1 * pfConvR[k];
        }

        //Sauve le résultat        
        pfNoiseConvL[i] = fL;
        pfNoiseConvR[i] = fR;
        
        //Décale la pile
        lStackPos = (lStackPos+1)&2047;
    }
    
    /*
        La convolution est maintenant appliquée sur le bruit blanc...
        Il faut comparer le bruit blanc source à celui des canaux gauche et droit
        et égaliser les différences.
    */
    
    //On initialise les mesures...
    memset ( fNoiseFreeLevelFreq, 0, 2048 * 4 );
    memset ( fNoiseConvLevelFreq, 0, 2048 * 4 );
    
    //On effectue l'ensemble des mesures... avec un overlap de 50%
    fDiv = 0;
    for (i=0;i<lEqualizeLength-4096;i+=1024)
    {
        //On applique la fenêtre...
        for (k=0;k<4096;k++)
        {
            fNoiseFreeTime[k] = fWindow[k] * pfNoise[i+k];
            fNoiseConvTime[0][k] = fWindow[k] * pfNoiseConvL[i+k];
            fNoiseConvTime[1][k] = fWindow[k] * pfNoiseConvR[i+k];
        }
        
        //On applique la FFT
        FFT_ForwardComplex_f32 ( 
            pFFTInfo, 
            fNoiseFreeTime, 
            fImpulseReal,
            fImpulseImag );

        //La crête...
        for (k=0;k<2048;k++)
        {
            f1 = sam_ABS ( (float)hypot ( fImpulseReal[k], fImpulseImag[k] ) );
            //if (f1>fNoiseFreeLevelFreq[k]) fNoiseFreeLevelFreq[k] = f1;
            
            fNoiseFreeLevelFreq[k] += f1;
        }
            
        //On applique la FFT
        FFT_ForwardComplex_f32 ( 
            pFFTInfo, 
            fNoiseConvTime[0], 
            fImpulseReal,
            fImpulseImag );

        //La crête...
        for (k=0;k<2048;k++)
        {
            f1 = sam_ABS ( (float)hypot ( fImpulseReal[k], fImpulseImag[k] ) );
            //if (f1>fNoiseConvLevelFreq[k]) fNoiseConvLevelFreq[k] = f1;
            
            fNoiseConvLevelFreq[k] += f1*0.5F;
        }

        //On applique la FFT
        FFT_ForwardComplex_f32 ( 
            pFFTInfo, 
            fNoiseConvTime[1], 
            fImpulseReal,
            fImpulseImag );

        //La crête...
        for (k=0;k<2048;k++)
        {
            f1 = sam_ABS ( (float)hypot ( fImpulseReal[k], fImpulseImag[k] ) );
            //if (f1>fNoiseConvLevelFreq[k]) fNoiseConvLevelFreq[k] = f1;
            
            fNoiseConvLevelFreq[k] += f1*0.5F;
        }

            
    }

    //La moyenne du bruit blanc sur toutes les fréquences...    
    /*f1 = 0;
    for (i=0;i<2048;i++)
    {
        f1 += fNoiseFreeLevelFreq[i];
    }
    f1 /= 2048.0F;*/
    
    //Egalisation des fréquences
    for (i=0;i<2048;i++)
    {
        f1 = fNoiseFreeLevelFreq[i];
        f2 = fNoiseConvLevelFreq[i];
        if (f2>0) fLevel = f1 / f2;
        else      fLevel = 1.0F;
        
        fImpulseTime[i] = fLevel;
    }
    
    for (i=0;i<2048;i++)
    {    
        fLevel = 0.0F;
        f2 = 0.0F;
        n = 1+(i>>6);
        for (k=-n;k<=n;k++)
        {
            if (k==0) f1 = 0.5F;
            else      f1 = 1.0F / (float)sam_ABS(k);
            f1 *= 2.0F;
            f1 = (float)sqrt ( f1 );
            
            j = i+k;
            if ((j>=0)&&(j<2048))
            {
                fLevel += f1;
                f2 += f1 * fImpulseTime[j];
            }
        }
        f2 /= fLevel;
        fEqual[i] = f2;
    }
    

    memset ( fImpulseTime, 0, 4096 * 4 );
    fImpulseTime[2048] = 1.0F;

    //FFT
    FFT_ForwardComplex_f32 ( 
        pFFTInfo, 
        fImpulseTime, 
        fImpulseReal,
        fImpulseImag );
    
    // Application de l'égalisation
    for (k=0;k<2048;k++)
    {
        f1 = (float)hypot ( fImpulseReal[k], fImpulseImag[k] ) * fEqual[k];
        f2 = (float)atan2 ( fImpulseImag[k], fImpulseReal[k] );
        fImpulseReal[k] =  f1 * (float)cos ( f2 );                
        fImpulseImag[k] =  f1 * (float)sin ( f2 );                
    }
    
    // Application de la iFFT
    FFT_BackwardComplex_f32 (
        pFFTInfo, 
        fImpulseReal,
        fImpulseImag,
        fImpulseTime );
        
    for (k=0;k<4096;k++)
        fImpulseTime[k] /= 2048.0F;
        
    
    
    //On applique l'égalisation
    for (i=0;i<2;i++)
    {
        if (i==0) pfConv = pfConvL;
        else      pfConv = pfConvR;
    
        
        for (k=0;k<lConvLength;k++)
        {
            f1 = 0.0F;
            for (n=0;n<4096;n++)
            {   
                j = k-2048+n;
                if ((j>=0)&&(j<lConvLength))
                    f1 += fImpulseTime[n] * pfConv[j];
            }
            
            fImpulseReal[k] = f1;
        }
        
        for (k=0;k<lConvLength;k++)
            pfConv[k] = fImpulseReal[k];
    }    
    
    
    
    /*
    //On applique l'égalisation
    for (i=0;i<2;i++)
    {
        if (i==0) pfConv = pfConvL;
        else      pfConv = pfConvR;

        //On place l'impulse au milieu            
        memset ( fImpulseTime, 0, 4096 * 4 );
        for (k=0;k<lConvLength;k++)
            fImpulseTime[k+2048-1024] = pfConv[k];
                
        //FFT
        FFT_ForwardComplex_f32 ( 
            pFFTInfo, 
            fImpulseTime, 
            fImpulseReal,
            fImpulseImag );
        
        // Application de l'égalisation
        for (k=0;k<2048;k++)
        {
            f1 = hypot ( fImpulseReal[k], fImpulseImag[k] ) * fEqual[k];
            f2 = atan2 ( fImpulseImag[k], fImpulseReal[k] );
            fImpulseReal[k] =  f1 * cos ( f2 );                
            fImpulseImag[k] =  f1 * sin ( f2 );                
        }
        
        // Application de la iFFT
        FFT_BackwardComplex_f32 (
            pFFTInfo, 
            fImpulseReal,
            fImpulseImag,
            fImpulseTime );

        for (k=0;k<lConvLength;k++)
            pfConv[k] = fImpulseTime[k+2048-1024] / 2048.0F;
    }
    */
    
    free ( pfNoise );
    free ( pfNoiseConvL );
    free ( pfNoiseConvR );
    
    FFT_Free ( pFFTInfo );
}

void SAM_RENDER_DecodePrepare_0x26 ( void )
{
    float f1, f2;
    float fTable[2][2048];
    float fL, fR;
    long c,i,j,k,o1,o2;
    T_CONVOLUTION_FILTER_LR ConvFilter;
    SAM_RENDER254   *pRender;
    SAM_RENDER2K    Render2K;
    UINT32          ui32Random;
    float fNormalize;    

    
    CA_Decrypt64 ( (BYTE *)__0x26fl__dwDataChannel0, __0x26fl__dw32bitsBlocCount>>1, __0x26fl__dwCryptKey[0], __0x26fl__dwCryptKey[1] );
    CA_Decrypt64 ( (BYTE *)__0x26fl__dwDataChannel1, __0x26fl__dw32bitsBlocCount>>1, __0x26fl__dwCryptKey[0], __0x26fl__dwCryptKey[1] );

    CA_Decrypt64 ( (BYTE *)__0x26fr__dwDataChannel0, __0x26fr__dw32bitsBlocCount>>1, __0x26fr__dwCryptKey[0], __0x26fr__dwCryptKey[1] );
    CA_Decrypt64 ( (BYTE *)__0x26fr__dwDataChannel1, __0x26fr__dw32bitsBlocCount>>1, __0x26fr__dwCryptKey[0], __0x26fr__dwCryptKey[1] );

    CA_Decrypt64 ( (BYTE *)__0x26fc__dwDataChannel0, __0x26fc__dw32bitsBlocCount>>1, __0x26fc__dwCryptKey[0], __0x26fc__dwCryptKey[1] );
    CA_Decrypt64 ( (BYTE *)__0x26fc__dwDataChannel1, __0x26fc__dw32bitsBlocCount>>1, __0x26fc__dwCryptKey[0], __0x26fc__dwCryptKey[1] );

    CA_Decrypt64 ( (BYTE *)__0x26rc__dwDataChannel0, __0x26rc__dw32bitsBlocCount>>1, __0x26rc__dwCryptKey[0], __0x26rc__dwCryptKey[1] );
    CA_Decrypt64 ( (BYTE *)__0x26rc__dwDataChannel1, __0x26rc__dw32bitsBlocCount>>1, __0x26rc__dwCryptKey[0], __0x26rc__dwCryptKey[1] );

    CA_Decrypt64 ( (BYTE *)__0x26rl__dwDataChannel0, __0x26rl__dw32bitsBlocCount>>1, __0x26rl__dwCryptKey[0], __0x26rl__dwCryptKey[1] );
    CA_Decrypt64 ( (BYTE *)__0x26rl__dwDataChannel1, __0x26rl__dw32bitsBlocCount>>1, __0x26rl__dwCryptKey[0], __0x26rl__dwCryptKey[1] );

    CA_Decrypt64 ( (BYTE *)__0x26rr__dwDataChannel0, __0x26rr__dw32bitsBlocCount>>1, __0x26rr__dwCryptKey[0], __0x26rr__dwCryptKey[1] );
    CA_Decrypt64 ( (BYTE *)__0x26rr__dwDataChannel1, __0x26rr__dw32bitsBlocCount>>1, __0x26rr__dwCryptKey[0], __0x26rr__dwCryptKey[1] );
    
    memset ( __0x26__f32Impulse, 0, 6*2*2048*4 );
    
    memcpy ( __0x26__f32Impulse[0][0], __0x26fl__dwDataChannel0, __0x26fl__dwDataChannelCount*4 );
    memcpy ( __0x26__f32Impulse[0][1], __0x26fl__dwDataChannel1, __0x26fl__dwDataChannelCount*4 );
    
    memcpy ( __0x26__f32Impulse[1][0], __0x26fr__dwDataChannel0, __0x26fr__dwDataChannelCount*4 );
    memcpy ( __0x26__f32Impulse[1][1], __0x26fr__dwDataChannel1, __0x26fr__dwDataChannelCount*4 );
    
    memcpy ( __0x26__f32Impulse[2][0], __0x26fc__dwDataChannel0, __0x26fc__dwDataChannelCount*4 );
    memcpy ( __0x26__f32Impulse[2][1], __0x26fc__dwDataChannel1, __0x26fc__dwDataChannelCount*4 );
    
    memcpy ( __0x26__f32Impulse[3][0], __0x26rc__dwDataChannel0, __0x26rc__dwDataChannelCount*4 );
    memcpy ( __0x26__f32Impulse[3][1], __0x26rc__dwDataChannel1, __0x26rc__dwDataChannelCount*4 );
    
    memcpy ( __0x26__f32Impulse[4][0], __0x26rl__dwDataChannel0, __0x26rl__dwDataChannelCount*4 );
    memcpy ( __0x26__f32Impulse[4][1], __0x26rl__dwDataChannel1, __0x26rl__dwDataChannelCount*4 );
    
    memcpy ( __0x26__f32Impulse[5][0], __0x26rr__dwDataChannel0, __0x26rr__dwDataChannelCount*4 );
    memcpy ( __0x26__f32Impulse[5][1], __0x26rr__dwDataChannel1, __0x26rr__dwDataChannelCount*4 );
    
    //Process SRC Conversion
    if (samData.dwHardwaremixSampleRate!=48000)
    {
        long lOutFFTLen;
        long lOutFFTBegin;
        float fImpulseTime[4096];
        float fImpulseReal[2048];
        float fImpulseImag[2048];
        void  *pFFTInfo48K;
        void  *pFFTInfoSRC;
        //FILE * pFile;
        
                
        switch (samData.dwHardwaremixSampleRate)
        {
            case 22050: lOutFFTLen = 1880; lOutFFTBegin = 470; break;  //Error of -2 samples
            case 24000: lOutFFTLen = 2048; lOutFFTBegin = 512; break;
            case 32000: lOutFFTLen = 2728; lOutFFTBegin = 682; break;  //Error of -2 samples
            case 44100: lOutFFTLen = 3764; lOutFFTBegin = 941; break;
        }        

        FFT_Alloc ( &pFFTInfo48K, 4096 );
        FFT_Alloc ( &pFFTInfoSRC, lOutFFTLen );
        
        fNormalize = 1.0F / 2048.0F;
        f1 = 1024.0F;
        f2 = (float)lOutFFTBegin;
        fNormalize *= (float)pow ( f1 / f2, 2.0F );
        
        
        //pFile = fopen ( "Z:\\ImpulseGroup.pcm", "wb" );
        
        //Do not process first sample of all impulses...
        for (i=0;i<6;i++)
        {
            for (j=0;j<2;j++)
            {
                //Copy data
                memset ( fImpulseTime, 0, 4096*sizeof(float) );
                memcpy ( fImpulseTime+1024, __0x26__f32Impulse[i][j], 2048*sizeof(float) );
                
                //FFT
                FFT_ForwardComplex_f32 ( 
                    pFFTInfo48K, 
                    fImpulseTime, 
                    fImpulseReal,
                    fImpulseImag );
                    
                memset ( fImpulseTime, 0, 4096*sizeof(float) );
                
                //iFFT                
                FFT_BackwardComplex_f32 (
                    pFFTInfoSRC, 
                    fImpulseReal,
                    fImpulseImag,
                    fImpulseTime );
                
                //Copy data
                memset ( __0x26__f32Impulse[i][j], 0, 2048*sizeof(float) );
                memcpy ( __0x26__f32Impulse[i][j], fImpulseTime+lOutFFTBegin, (lOutFFTLen>>1)*sizeof(float) );
                
                //fwrite ( fImpulseTime, 4096*sizeof(float), 1, pFile );
            }
        }
        
        //fclose ( pFile );
        
        FFT_Free ( pFFTInfo48K );
        FFT_Free ( pFFTInfoSRC );
    }
    else fNormalize = 1.0F;
    
    /*
    for (i=0;i<2044;i++)
    {

        fL = 0;
        fR = 0;
        f1 = __0x26__f32Impulse[2][0][i];
        f2 = __0x26__f32Impulse[2][1][i];   
        for (j=0;j<2044;j++)
        {
            fL += ( f1 * __0x26__f32Impulse[0][0][j] ) +
                           ( f2 * __0x26__f32Impulse[1][0][j] );
                           
            fR += ( f1 * __0x26__f32Impulse[0][1][j] ) +
                           ( f2 * __0x26__f32Impulse[1][1][j] );
        }
        
        fTable[0][i] = fL * 16;
        fTable[1][i] = fR * 16;
    }

    for (i=0;i<2044;i++)
    {
        __0x26__f32Impulse[2][0][i] = fTable[0][i];
        __0x26__f32Impulse[2][1][i] = fTable[1][i];
    }
    */

    for (i=0;i<2044;i++)
    {
        f1 = ( __0x26__f32Impulse[0][0][i] + __0x26__f32Impulse[1][0][i] ) * 0.7F;
        f2 = ( __0x26__f32Impulse[0][1][i] + __0x26__f32Impulse[1][1][i] ) * 0.7F;
        fL = __0x26__f32Impulse[2][0][i];
        fR = __0x26__f32Impulse[2][1][i];
        
        if (i<80)
        {
            f1 = __0x26__f32Impulse[0][0][i] * 0.6F + __0x26__f32Impulse[1][0][i] * 0.4F;
            f2 = __0x26__f32Impulse[0][1][i] * 0.4F + __0x26__f32Impulse[1][1][i] * 0.6F;
        }
        
        f1 = ( __0x26__f32Impulse[0][0][i] + __0x26__f32Impulse[1][0][i] ) * 0.5F;
        f2 = ( __0x26__f32Impulse[0][1][i] + __0x26__f32Impulse[1][1][i] ) * 0.5F;
        
        __0x26__f32Impulse[2][0][i] = f1; //( f1 * 0.9 ) + ( fL * 0.1 );
        __0x26__f32Impulse[2][1][i] = f2; //( f2 * 0.9 ) + ( fR * 0.1 );
    }
    
    
    
    /*
    {
        void    *pFFTInfo;
        float fImpulseTime[4096];
        float fImpulseReal[2048];
        float fImpulseImag[2048];
        float fImpulsePeak[2048];
        float fImpulsePeak2[2048];
        
        float fImpulseLevel[2][2048];
        float fImpulsePhase[2][2048];
        
        float fImpulseGainEq[2048];
        
        
        FFT_Alloc ( &pFFTInfo, 4096 );
        
        memset ( fImpulsePeak, 0, 2048 * 4 );
        
        for (c=0;c<2;c++)
        {
            memset ( fImpulseTime, 0, 4096 * 4 );

            //On place l'impulse au milieu            
            for (i=0;i<2044;i++)
                fImpulseTime[i+2048-1024] = __0x26__f32Impulse[2][c][i];
                
            //FFT
            FFT_ForwardComplex_f32 ( 
                pFFTInfo, 
                fImpulseTime, 
                fImpulseReal,
                fImpulseImag );

            //Freq/Phase                
            for (i=0;i<2048;i++)
            {
                fImpulseLevel[c][i] = hypot ( fImpulseReal[i], fImpulseImag[i] );
                fImpulsePhase[c][i] = atan2 ( fImpulseImag[i], fImpulseReal[i] );
                
                f1 = sam_ABS ( fImpulseLevel[c][i] );
                if (f1>fImpulsePeak[i]) fImpulsePeak[i] = f1;
                
                
                //fImpulsePeak[i] += sam_ABS ( fImpulseLevel[c][i] );
                
                //if (fImpulsePeak[i]>0) fImpulseGainEq[i] = 1 / fImpulsePeak[i];
                //else                   fImpulseGainEq[i] = 0.0F;
            }
        }
        
        //Moyenne de la crête...
        for (i=0;i<2048;i++)
        {
            //fImpulsePeak[i] *= 0.5F;
            
            if (fImpulsePeak[i]>0) fImpulseGainEq[i] = 1 / fImpulsePeak[i];
            else                   fImpulseGainEq[i] = 0.0F;
        }
        
        
        for (c=0;c<2;c++)
        {
            for (i=0;i<2048;i++)
            {
                f1 = fImpulseLevel[c][i] * fImpulseGainEq[i];
                f2 = fImpulsePhase[c][i];
                fImpulseReal[i] =  f1 * cos ( f2 );                
                fImpulseImag[i] =  f1 * sin ( f2 );                
            }

            // Application de la iFFT
            FFT_BackwardComplex_f32 (
                pFFTInfo, 
                fImpulseReal,
                fImpulseImag,
                fImpulseTime );
            

            for (i=0;i<2044;i++)
                __0x26__f32Impulse[2][c][i] = fImpulseTime[i+2048-1024] / 2048.0F;
        }
        
        
        FFT_Free ( pFFTInfo );        
            
        
    
    }
    */
    
    /*
    IKA_EqualizeConvolution2Kmax ( 
        __0x26__f32Impulse[2][0],
        __0x26__f32Impulse[2][1],
        2044 );
    */
    
    if (0)
    {
        FILE * pFile;
        float fStackData[2048];
        long lStackPosition;
        UINT32 ui32Value;
        

        
        memset ( fStackData, 0, 2048 * 4 );
        lStackPosition  = 0;
        ui32Random      = 984235797;
        
        pFile = fopen ( "Z:\\testnoise-48000-32-1.pcm", "wb" );
    
        for (c=0;c<4800000;c++)
        {
            ui32Random = ( 1664525 * ui32Random ) + 1013904223;
            
            ui32Random = (UINT32)(((UINT64)ui32Random * 279470273) % 4294967291);
            k = ((ui32Random>>10)&0xFFFFF);
            f1 = ((float)k)/1048576.0F;
            
            //ui32Random = ( 1664525 * ui32Random ) + 1013904223;
            ui32Random = (UINT32)(((UINT64)ui32Random * 279470273) % 4294967291);
            k = ((ui32Random>>10)&0xFFFFF);
            f2 = ((float)k)/1048576.0F;
            
            //f1 = f1 * sin ( f2 * 2.0F * sam_PI ) * 0.5F;
            
            f1 = (float)( ( sin ( 2.0F * sam_PI * f1 ) + cos ( 2.0F * sam_PI * f2 ) ) * 0.25 );
            
            fStackData[lStackPosition] = f1;
            lStackPosition = (lStackPosition+1)&2047;
            
            fL = 0;
            fR = 0;
            
            
            for (i=0;i<2044;i++)
            {
                f2 = fStackData[(lStackPosition-i)&2047];
                fL += f2 * __0x26__f32Impulse[2][0][i];
                fR += f2 * __0x26__f32Impulse[2][1][i];
            }
            
            
            f2 = ( fL + fR ) * 0.1F;
            
            
            if (c>2044)
            {
                fwrite ( &f1, 4, 1, pFile );
                
                //fL *= 0.1F;
                //fR *= 0.1F;
                //fwrite ( &fL, 4, 1, pFile );
                //fwrite ( &fR, 4, 1, pFile );
                
            }
        }
        
        fclose ( pFile );
    }
        
    
    for (c=0;c<6;c++)
    {
        ui32Random = c;
        SAM_RENDER_ConvolutionFilter_Init ( &ConvFilter );
        
        i=0;
        for (k=0;k<2044;k++)
        {
            f1 = __0x26__f32Impulse[c][0][k] * fNormalize;// * 16.0F;
            f2 = __0x26__f32Impulse[c][1][k] * fNormalize;// * 16.0F;
            
            if (((fabs(f1)>0.0003)||(fabs(f2)>0.0003))&&(i<(sam_VOICEBUFFERSTACK_COUNT-1))) //-70dB minimum
            {
                //Génération de la convolution   
                //SAM_RENDER_ConvolutionFilter_Add ( &ConvFilter, k, f1, 0 );
                //SAM_RENDER_ConvolutionFilter_Add ( &ConvFilter, k, f2, 1 );
                
                ui32Random = ( 1664525 * ui32Random ) + 1013904223;
                
                o1 = (ui32Random>>8)&7;
                o2 = (ui32Random>>20)&7;
                o1 += k;
                o2 += k;
                
                if (o1>2044)
                {
                    f1 = 0.0F;
                    o1 = 2044;
                }
                if (o2>2044)
                {
                    f2 = 0.0F;
                    o2 = 2044;
                }
                
                
                
                //if (ui32Random&20) f1 = -f1;
                //if (ui32Random&16) f2 = -f2;
                
                
                j = (i<<1);
                Render2K.dwDelayIndex[j  ] = k; //k + (ui32Random&3);
                Render2K.fDelayGain[j  ]   = f1;
                Render2K.dwDelayIndex[j+1] = k; //k + (ui32Random&3);
                Render2K.fDelayGain[j+1]   = f2;
                i++;
            }
        }
        Render2K.dwEntryCount = i;
        
        /*
        //Remplissage de pRender2K
        for (i=0;i<sam_VOICEBUFFERSTACK_COUNT-1;i++)
        {
            j = (i<<1);
            Render2K.dwDelayIndex[j  ] = i;
            Render2K.fDelayGain[j  ]   = ConvFilter.fConvolutionLeft[i];
            Render2K.dwDelayIndex[j+1] = i;
            Render2K.fDelayGain[j+1]   = ConvFilter.fConvolutionRight[i];
        }
        Render2K.dwEntryCount = sam_VOICEBUFFERSTACK_COUNT-1;
        */
        
        pRender = &(__0x26__Render254[c]);
        SAM_RENDER_LimitTotalAmountDelayCount_2Kto64 ( pRender, &Render2K, 254 );
    }
}


void IKA_MakeNoise ( float * pfData, long lLength, long lSeed, float fLevel )
{
    float f1;
    float f2;
    long i,k;
    UINT32 ui32Random;
    float fLevelFinal;

    ui32Random      = (UINT32)lSeed;
    fLevelFinal     = fLevel * 0.5F;
    ui32Random = ( 1664525 * ui32Random ) + 1013904223;
    for (i=0;i<lLength;i++)
    {
        
        ui32Random = (UINT32)(((UINT64)ui32Random * 279470273) % 4294967291);
        //k = ((ui32Random>>8)&0xFFFFF);
        //f1 = ((float)k)/1048576.0F;
        f1 = ((float)k) / 4294967291;
            
        ui32Random = (UINT32)(((UINT64)ui32Random * 279470273) % 4294967291);
        k = ((ui32Random>>10)&0xFFFFF);
        f2 = ((float)k)/1048576.0F;
        
        //pfData[i] = ( sin ( 2.0F * sam_PI * f1 ) + cos ( 2.0F * sam_PI * f2 ) ) * fLevelFinal;
        pfData[i] = (float)sin ( 2.0F * sam_PI * f2 ) * fLevel;
    }
}        
