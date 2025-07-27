/*
    (c) Alexia Gossa
*/

    #include "audiotools.h"
    #include "audiotools_fft_jjn.h"


    typedef struct {
        FLOAT32  * pf_xRe;
        FLOAT32  * pf_xIm;
        FLOAT32  * pf_yRe;
        FLOAT32  * pf_yIm;

        FLOAT32  fReEqualizeF;
        FLOAT32  fReEqualizeB;

        long    lNbTemporalPoints;

        void    * pFFTJJN;
    } FFT_INFO;

    long FFT_JJN_AllocFFTNeeds ( void ** pData, long lNbPoints );
    long FFT_JJN_FreeFFTNeeds ( void * pData );
    long FFT_JJN_do_fft ( void * pData, FLOAT32 xRe[], FLOAT32 xIm[], FLOAT32 yRe[], FLOAT32 yIm[] );

    //long FFT_JJN_do_fft ( int n, FLOAT32 xRe[], FLOAT32 xIm[], FLOAT32 yRe[], FLOAT32 yIm[] );




    void FFT_Alloc ( void ** pFFTInfo, long lNbTemporalPoints )
    {
        FFT_INFO * pFFT_Info;

        //Allocation
        pFFT_Info = (FFT_INFO *)AUDIOTOOLS_MALLOC ( sizeof(FFT_INFO) );

        //Nombre de fréquences
        pFFT_Info->lNbTemporalPoints    = lNbTemporalPoints;
        pFFT_Info->fReEqualizeF         = 4.0F / (FLOAT32)lNbTemporalPoints; //1.0F / sqrt ( (FLOAT32)lNbTemporalPoints );
        pFFT_Info->fReEqualizeB         = 1.0F / pFFT_Info->fReEqualizeB; //       sqrt ( (FLOAT32)lNbTemporalPoints );
        pFFT_Info->pf_xRe               = (FLOAT32 *)AUDIOTOOLS_MALLOC ( sizeof(FLOAT32) * lNbTemporalPoints  * 4 );
        pFFT_Info->pf_xIm               = &(pFFT_Info->pf_xRe[lNbTemporalPoints]);
        pFFT_Info->pf_yRe               = &(pFFT_Info->pf_xIm[lNbTemporalPoints]);
        pFFT_Info->pf_yIm               = &(pFFT_Info->pf_yRe[lNbTemporalPoints]);

        //Données pour la FFT
        FFT_JJN_AllocFFTNeeds ( &(pFFT_Info->pFFTJJN), lNbTemporalPoints );

        *pFFTInfo = (void *)pFFT_Info;
    }

    void FFT_Free ( void * pFFTInfo )
    {
        FFT_INFO * pFFT_Info;
        pFFT_Info = (FFT_INFO *)pFFTInfo;

        //Données pour la FFT
        FFT_JJN_FreeFFTNeeds ( pFFT_Info->pFFTJJN );

        //On vide le reste
        AUDIOTOOLS_FREE ( pFFT_Info->pf_xRe );
        AUDIOTOOLS_FREE ( pFFT_Info );
    }

    void FFT_Forward_f32 ( void * pFFTInfo, float *pfTemporalInput, float *pfFrequencyOutput, float *pfPhaseOutput )
    {
        long i;
        FFT_INFO * pFFT_Info;
        pFFT_Info = (FFT_INFO *)pFFTInfo;

        //Convertion en Real/Imaginary
        for (i=0;i<pFFT_Info->lNbTemporalPoints;i++)
        {
            pFFT_Info->pf_xRe[i] = (FLOAT32)pfTemporalInput[i];
            pFFT_Info->pf_xIm[i] = 0.0F;
        }

        FFT_JJN_do_fft ( pFFT_Info->pFFTJJN,
                 pFFT_Info->pf_xRe,
                 pFFT_Info->pf_xIm,
                 pFFT_Info->pf_yRe,
                 pFFT_Info->pf_yIm );

        if ((pfFrequencyOutput)&&(pfPhaseOutput))
        {
            for (i=0;i<pFFT_Info->lNbTemporalPoints>>1;i++)
            {
                pfFrequencyOutput[i]    = (float)( hypot ( pFFT_Info->pf_yRe[i], pFFT_Info->pf_yIm[i] ) * pFFT_Info->fReEqualizeF );
                pfPhaseOutput[i]        = (float)  atan2 ( pFFT_Info->pf_yIm[i], pFFT_Info->pf_yRe[i] );
            }
        }
        else if (pfFrequencyOutput)
        {
            for (i=0;i<pFFT_Info->lNbTemporalPoints>>1;i++)
            {
                pfFrequencyOutput[i]    = (float)( hypot ( pFFT_Info->pf_yRe[i], pFFT_Info->pf_yIm[i] ) * pFFT_Info->fReEqualizeF );
            }
        }
    }

    void FFT_Backward_f32 ( void * pFFTInfo, float *pfFrequencyInput, float *pfPhaseInput, float *pfTemporalOutput )
    {
        FLOAT32 f1;
        long i,ns2;
        FFT_INFO * pFFT_Info;
        pFFT_Info = (FFT_INFO *)pFFTInfo;

        ns2 = pFFT_Info->lNbTemporalPoints>>1;

        //Convertion en Real/Imaginary
        for (i=0;i<pFFT_Info->lNbTemporalPoints>>1;i++)
        {
            f1 = (FLOAT32)pfFrequencyInput[i] * pFFT_Info->fReEqualizeB;
            pFFT_Info->pf_xRe[i] =  f1 * (FLOAT32)cos ( pfPhaseInput[i] );
            pFFT_Info->pf_xIm[i] =  f1 * (FLOAT32)sin ( pfPhaseInput[i] );
        }
        for (;i<pFFT_Info->lNbTemporalPoints;i++)
        {
            pFFT_Info->pf_xRe[i] = 0;
            pFFT_Info->pf_xIm[i] = 0;
        }
        
        FFT_JJN_do_fft ( pFFT_Info->pFFTJJN,
                 pFFT_Info->pf_xRe,
                 pFFT_Info->pf_xIm,
                 pFFT_Info->pf_yRe,
                 pFFT_Info->pf_yIm );

        for (i=0;i<pFFT_Info->lNbTemporalPoints;i++)
        {
            pfTemporalOutput[i]    = (float)pFFT_Info->pf_yRe[i];
        }
    }

    /*
        Cette routine n'est pas la même que forward : les valeurs restent complexes
        En contrepartie, elle est plus rapide.
    */
    void FFT_ForwardComplex_f32 ( void * pFFTInfo, float * pfTemporalInput, float * pfReOutput, float *pfImOutput )
    {
        long i,n;
        float *pfRe, *pfIm;
        FFT_INFO * pFFT_Info;
        pFFT_Info = (FFT_INFO *)pFFTInfo;

        //Convertion en Real/Imaginary
        pfRe    = pFFT_Info->pf_xRe;
        pfIm    = pFFT_Info->pf_xIm;
        n       = pFFT_Info->lNbTemporalPoints;
        for (i=0;i<n;i++)
        {
            *pfRe = pfTemporalInput[i];
            *pfIm = 0.0F;
            pfRe++;
            pfIm++;
        }

        FFT_JJN_do_fft ( pFFT_Info->pFFTJJN,
                 pFFT_Info->pf_xRe,
                 pFFT_Info->pf_xIm,
                 pFFT_Info->pf_yRe,
                 pFFT_Info->pf_yIm );

        //Convertion finale
        pfRe    = pFFT_Info->pf_yRe;
        pfIm    = pFFT_Info->pf_yIm;
        n       = pFFT_Info->lNbTemporalPoints>>1;
        for (i=0;i<n;i++)
        {
            pfReOutput[i] = *pfRe;
            pfImOutput[i] = *pfIm;
            pfRe++;
            pfIm++;
        }
    }

    /*
        Cette routine n'est pas la même que backward : les valeurs restent complexes
        En contrepartie, elle est plus rapide.
    */
    void FFT_BackwardComplex_f32 ( void * pFFTInfo, float * pfReInput, float *pfImInput, float *pfTemporalOutput )
    {
        long i,n;
        float *pfRe, *pfIm;

        FFT_INFO * pFFT_Info;
        pFFT_Info = (FFT_INFO *)pFFTInfo;

        //Convertion en Real/Imaginary
        pfRe    = pFFT_Info->pf_xRe;
        pfIm    = pFFT_Info->pf_xIm;
        n       = pFFT_Info->lNbTemporalPoints>>1;
        for (i=0;i<n;i++)
        {
            *pfRe =  pfReInput[i];
            *pfIm = -pfImInput[i];
            pfRe++;
            pfIm++;
        }
        n = pFFT_Info->lNbTemporalPoints;
        for (;i<n;i++)
        {
            *pfRe = 0;
            *pfIm = 0;
            pfRe++;
            pfIm++;
        }
        
        FFT_JJN_do_fft ( pFFT_Info->pFFTJJN,
                 pFFT_Info->pf_xRe,
                 pFFT_Info->pf_xIm,
                 pFFT_Info->pf_yRe,
                 pFFT_Info->pf_yIm );

        pfRe    = pFFT_Info->pf_yRe;
        n       = pFFT_Info->lNbTemporalPoints;
        for (i=0;i<n;i++)
        {
            pfTemporalOutput[i] = *pfRe;
            pfRe++;
        }
    }




