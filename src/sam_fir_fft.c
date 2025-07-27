#include "sam_header.h"
#include "audiotools.h"




long            SAM_FIR_FFT_Design                  ( long lLength, float * pf32Coef, float f32RatioCut );


long            SAM_FIR_FFT_DesignEq                ( long lCoefLength, float * pf32Coef, long lEqualizerLenght_pow2, float * f32EqualizerCoef )
{
    static  float   f32Coef[8192];
    static  float   f32FFT_RealPart[4096];
    static  float   f32FFT_ImagPart[4096];
    static  float   f32EqualizerMul[4096];
            float   f1, f2;
            double  d1;
            void    *pFFTInfo;
            long    i,j,k,n,n2;
            
            
            
            
    //*****************************************
    //Création de l'égaliseur
    for (i=0;i<4096;i++)
        f32EqualizerMul[i] = 1.0F;
        
    n = 4096 / lEqualizerLenght_pow2;
    k = 0;
    for (i=0;i<lEqualizerLenght_pow2;i++)
    {   
        f1 = f32EqualizerCoef[i];
        for (j=0;j<n;j++)
        {
            f32EqualizerMul[k] = f1;
            k++;
        }
    }
    
    /*
    // Création des ZERO
    memset (
        f32Coef,
        0,
        sizeof(float) * 8192 );
        
    // Création de l'impulsion
    f32Coef[4095] = 1.0F;
    */
    
    for (i=0;i<4096;i++)
    {
        //Calcul du dividende et diviseur
        d1 = (double)i;
        d1 += 0.5;
        d1 *= sam_PI;
        
        //Sinc
        f1 = (float)(sin ( d1 ) / d1);
        
        f32Coef[4096+i] = f1;
        f32Coef[4095-i] = f1;
    }

    /*
    {
        FILE *pFile;
        pFile = fopen ( "C:\\Sinc.dbl", "wb" );
        for (i=0;i<8192;i++)
        {
            d1 = f32Coef[i];
            fwrite ( &d1, 8, 1, pFile );
        }
        fclose ( pFile );
    }
    */
    
    // Allocation de la FFT en mémoire
    FFT_Alloc ( &pFFTInfo, 8192 );
    
    // Application de la FFT
    FFT_ForwardComplex_f32 ( 
        pFFTInfo, 
        f32Coef, 
        f32FFT_RealPart,
        f32FFT_ImagPart );

    // Application de l'égaliseur
    for (i=0;i<4096;i++)
    {
        f1 = (float)(hypot ( f32FFT_RealPart[i], f32FFT_ImagPart[i] ) * f32EqualizerMul[i]);
        f2 = (float)atan2 ( f32FFT_ImagPart[i], f32FFT_RealPart[i] );
        f32FFT_RealPart[i] =  f1 * (float)cos ( f2 );                
        f32FFT_ImagPart[i] =  f1 * (float)sin ( f2 );                
    }
    
    // Application de la iFFT
    FFT_BackwardComplex_f32 (
        pFFTInfo, 
        f32FFT_RealPart,
        f32FFT_ImagPart,
        f32Coef );

    // Compensation du niveau        
    for (i=0;i<8192;i++)
    {
        f32Coef[i] *= 1.0F/4096.0F;
    }
        
    // Libération de la FFT en mémoire
    FFT_Free ( pFFTInfo );
        
    // Copie uniquement les points nécessaires
    n = 4096 - (lCoefLength>>1);
    for (i=0;i<lCoefLength;i++)
    {
        pf32Coef[i] = f32Coef[n+i];    
    }
    
    
    return 0;
}

    

    /*
        Design a FIR filter...
        
        f32RatioCut     must be : 0...0.5
        
        lOrder          2...512 (even only)

        (Alexia Gossa)




        return -1 on error.    

    */
    long SAM_FIR_FFT_Design ( long lLength, float * pf32Coef, float f32RatioCut )
    {
        static  float   f32Coef[8192];
        static  float   f32FFT_RealPart[4096];
        static  float   f32FFT_ImagPart[4096];
                void    *pFFTInfo;
                float   f1, f2, f3, fGain;
                long    i,n,n2;

        // Verify input data (even only)
        if ( (!lLength) || (!pf32Coef) || (lLength>4096) || (lLength&1) )
            return -1;

        // Make all ZERO
        memset (
            f32Coef,
            0,
            sizeof(float) * 8192 );
            
        // Make Impulse
        f32Coef[4095] = 1.0F;
        
        // Allocate FFT memory
        FFT_Alloc ( &pFFTInfo, 8192 );
        
        // Do FFT
        FFT_ForwardComplex_f32 ( 
            pFFTInfo, 
            f32Coef, 
            f32FFT_RealPart,
            f32FFT_ImagPart );

        // Do low pass filter
        n = (long)(4096.0F * fabs(f32RatioCut));
        for (i=n;i<4096;i++)
        {
            f32FFT_RealPart[i] = 0.0F;                
            f32FFT_ImagPart[i] = 0.0F;
        }        
        /*
        for (i=n;i<4096;i++)
        {
            if (i>n*2.0F)
            {
                fGain = 1.0F;
            }
            else fGain = 0.1F;
            
            //fGain = 0.0F;
        
            f1 = hypot ( f32FFT_RealPart[i], f32FFT_ImagPart[i] ) * fGain;
            f2 = atan2 ( f32FFT_ImagPart[i], f32FFT_RealPart[i] );

            f32FFT_RealPart[i] =  f1 * cos ( f2 );                
            f32FFT_ImagPart[i] =  f1 * sin ( f2 );                
        }
        
        */
        // Enhance Limit high frequency
        /*
        n2 = (long)(4096.0F * fabs(f32RatioCut*0.8F));
#if sam_FIRLEN==32
        fGain = 7.0F;
#else
        fGain = 10.0F;
#endif
        
        if (n<4080)
        {
            for (i=n;i>n2;i--)
            {
                f1 = hypot ( f32FFT_RealPart[i], f32FFT_ImagPart[i] ) * fGain;
                f2 = atan2 ( f32FFT_ImagPart[i], f32FFT_RealPart[i] );
                
                
                f32FFT_RealPart[i] =  f1 * cos ( f2 );                
                f32FFT_ImagPart[i] =  f1 * sin ( f2 );                
                                      
                fGain *= 0.98F;
                if (fGain<1.0F) fGain = 1.0F;   
            }
        }
        */
            
        // Do iFFT
        FFT_BackwardComplex_f32 ( 
            pFFTInfo, 
            f32FFT_RealPart,
            f32FFT_ImagPart,
            f32Coef );
            
        for (i=0;i<8192;i++)
        {
            f32Coef[i] *= 1.0F/4096.0F;
        }
        
        // Free FFT memory
        FFT_Free ( pFFTInfo );
        
        // Do copy of needed points
        n = 4096 - (lLength>>1);
        for (i=0;i<lLength;i++)
        {
            pf32Coef[i] = f32Coef[n+i];    
        }
        
        return 0;
    }




