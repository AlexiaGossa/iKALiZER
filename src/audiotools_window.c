/*
    (c) Alexia Gossa
*/
    #define WINDOWSHAPE_SINE                        0
    #define WINDOWSHAPE_MODIFIATESINE               1
    #define WINDOWSHAPE_KAISERBESSELDERIVEDSOFT     2
    #define WINDOWSHAPE_KAISERBESSELDERIVEDHARD     3
    #define WINDOWSHAPE_KAISERBESSELDERIVEDLOW      4
    #define WINDOWSHAPE_HANNING                     5

    #include "audiotools.h"

    void Window_FullBuild_f32 ( long lWindowShape, long lNbPoints, float * pf32Window );
    void Window_FullBuild_f64 ( long lWindowShape, long lNbPoints, double * pf64Window );
    void Window_MixBuild_f32 ( long lWindowShape, long lBeginWindowNbPoints, long lEndWindowNbPoints, float * pf32Window );
    void Window_MixBuild_f64 ( long lWindowShape, long lBeginWindowNbPoints, long lEndWindowNbPoints, double * pf64Window );

    void _Window_HalfBuild ( long lWindowShape, long lHalfNbPoints, long lIncrDecrMode, double * pf64HalfWindow, float * pf32HalfWindow );
    void _Window_FullBuild ( long lWindowShape, long lNbPoints, double * pf64HalfWindow, float * pf32HalfWindow );
    double _Window_ProcessShape ( long lShape, long lt, long ln );
    double _Window_ProcessShape_BesselI0 ( double x );
    void _Window_ProcessShape_KaiserBesselDerived ( double * pfWindowHalf64, float * pfWindowHalf32, long lSizeHalf, double fAlpha, long lProcessLeft0_Right1 );

    //Une fenêtre complète
    void Window_FullBuild_f32 ( long lWindowShape, long lNbPoints, float * pf32Window )
    {
        //Première partie de la fenêtre
        _Window_HalfBuild ( lWindowShape, lNbPoints>>1, 1, NULL, pf32Window );

        //Seconde partie de la fenêtre
        _Window_HalfBuild ( lWindowShape, lNbPoints>>1, -1, NULL, pf32Window+(lNbPoints>>1) );
    }

    //Une fenêtre complète
    void Window_FullBuild_f64 ( long lWindowShape, long lNbPoints, double * pf64Window )
    {
        //Première partie de la fenêtre
        _Window_HalfBuild ( lWindowShape, lNbPoints>>1, 1, pf64Window, NULL );

        //Seconde partie de la fenêtre
        _Window_HalfBuild ( lWindowShape, lNbPoints>>1, -1, pf64Window+(lNbPoints>>1), NULL );
        
    }

    //Une fenêtre partielle doublée avec un début et une fin différente
    void Window_MixBuild_f32 ( long lWindowShape, long lBeginWindowNbPoints, long lEndWindowNbPoints, float * pf32Window )
    {
        long i;
        long lNbPoints;
        
        //Taille globale de la fenêtre
        lNbPoints = (lBeginWindowNbPoints>lEndWindowNbPoints)?(lBeginWindowNbPoints):(lEndWindowNbPoints);

        //Initialisation de la fenêtre à 1
        for (i=(lBeginWindowNbPoints>>1);i<lNbPoints-(lEndWindowNbPoints>>1);i++) pf32Window[i] = 1.0F;

        //Première partie de la fenêtre
        _Window_HalfBuild ( lWindowShape, lBeginWindowNbPoints>>1, 1, NULL, pf32Window );

        //Seconde partie de la fenêtre
        _Window_HalfBuild ( lWindowShape, lEndWindowNbPoints>>1, -1, NULL, pf32Window+(lNbPoints-(lEndWindowNbPoints>>1)) );
    }

    //Une fenêtre partielle doublée avec un début et une fin différente
    void Window_MixBuild_f64 ( long lWindowShape, long lBeginWindowNbPoints, long lEndWindowNbPoints, double * pf64Window )
    {
        long i;
        long lNbPoints;
        
        //Taille globale de la fenêtre
        lNbPoints = (lBeginWindowNbPoints>lEndWindowNbPoints)?(lBeginWindowNbPoints):(lEndWindowNbPoints);

        //Initialisation de la fenêtre à 1
        for (i=(lBeginWindowNbPoints>>1);i<lNbPoints-(lEndWindowNbPoints>>1);i++) pf64Window[i] = 1.0F;

        //Première partie de la fenêtre
        _Window_HalfBuild ( lWindowShape, lBeginWindowNbPoints>>1, 1, pf64Window, NULL );

        //Seconde partie de la fenêtre
        _Window_HalfBuild ( lWindowShape, lEndWindowNbPoints>>1, -1, pf64Window+(lNbPoints-(lEndWindowNbPoints>>1)), NULL );
    }

    //Fonction de base pour une demi-fenêtre
    void _Window_HalfBuild ( long lWindowShape, long lHalfNbPoints, long lIncrDecrMode, double * pf64HalfWindow, float * pf32HalfWindow )
    {
        double fValue;
        long lKaiserBesselDerivedEnable;
        long i;

        switch (lWindowShape)
        {
            case WINDOWSHAPE_KAISERBESSELDERIVEDSOFT:
                fValue = 5.0F; //Alpha de la KBD (AC3 utilise également la valeur 5)
                lKaiserBesselDerivedEnable = 1;
                break;

            case WINDOWSHAPE_KAISERBESSELDERIVEDHARD:
                fValue = 8.0F; //Alpha de la KBD (La valeur 8 est assez violente)
                lKaiserBesselDerivedEnable = 1;
                break;

            case WINDOWSHAPE_KAISERBESSELDERIVEDLOW:
                fValue = 2.5F; //Alpha de la KBD (La valeur 2.5 est assez douce)
                lKaiserBesselDerivedEnable = 1;
                break;

            default:
                lKaiserBesselDerivedEnable = 0;
                break;
        }

        if (lKaiserBesselDerivedEnable)
        {
            //Calcul de la fonction de fenètrage évoluée via la KBD
            _Window_ProcessShape_KaiserBesselDerived ( 
                pf64HalfWindow, 
                pf32HalfWindow,
                lHalfNbPoints, 
                fValue, 
                (lIncrDecrMode>0)?(0):(1) );
        }
        else
        {
            //Calcul de la fonction de fenétrage non évoluée
            for (i=0;i<lHalfNbPoints;i++)
            {
                fValue = _Window_ProcessShape ( lWindowShape, i, lHalfNbPoints<<1 );

                if (pf64HalfWindow)
                {
                    if (lIncrDecrMode>0) pf64HalfWindow[i] = fValue;
                    else                 pf64HalfWindow[lHalfNbPoints-i-1] = fValue; 
                }
                if (pf32HalfWindow)
                {
                    if (lIncrDecrMode>0) pf32HalfWindow[i] = (float)fValue;
                    else                 pf32HalfWindow[lHalfNbPoints-i-1] = (float)fValue; 
                }
            }
        }
    }

    //Fonction de base pour une demi-fenêtre
    /*
    void _Window_FullBuild ( long lWindowShape, long lNbPoints, double * pf64HalfWindow, float * pf32HalfWindow )
    {
        double fValue;
        long i;

        //Calcul de la fonction de fenétrage
        for (i=0;i<lNbPoints;i++)
        {
            fValue = _Window_ProcessShape ( lWindowShape, i, lNbPoints );
            if (pf64HalfWindow) pf64HalfWindow[i] = fValue;
            if (pf32HalfWindow) pf32HalfWindow[i] = (float)fValue;
        }
    }
    */

    /*
        lShape
            0 = OGG/VORBIS sin modified
            1 = sqrt ( sin ( t ) )
            2 = other sin modified
            3 = Kaiser Bessel Derived

    */
    double _Window_ProcessShape_Source ( long lShape, long ln, long lN );

    double _Window_ProcessShape ( long lShape, long lCurrentPoint, long lNbPoints )
    {
        double fValueI, fValueO;
        double fPBCondition;
        long lValue_n, lValue_N;

        //Convertion du point courant à la valeur "n"
        lValue_n = lCurrentPoint;
        lValue_N = lNbPoints>>1;

        fValueI = _Window_ProcessShape_Source ( lShape, lValue_n, lValue_N );
        fValueO = _Window_ProcessShape_Source ( lShape, lValue_n+lValue_N, lValue_N );

        /*
            On doit satisfaire à la condition de Princen-Bradley

                  2       2
                W   + W     = 1
                 n     n+N

            n = 0...2N-1
        //

        //On doit avoir fValueI**2 + fValueO**2 = 1

        */
        
        fPBCondition = (fValueI*fValueI) + (fValueO*fValueO);

        //if (fPBCondition<1.0F) fValueI += sqrt ( (1.0F-fPBCondition)*0.5F );
        //if (fPBCondition>1.0F) fValueI -= sqrt ( (fPBCondition-1.0F)*0.5F );

        /*
        if (lCurrentPoint<(lNbPoints>>2)) return 0.0F;
        if (lCurrentPoint>=((lNbPoints*3)/4)) return 0.0F;
        return 1.0F;
        */
        


        return fValueI;
    }


    double _Window_ProcessShape_Source ( long lShape, long ln, long lN )
    {
        double fPI;
        double f1, f2, f3;
        double fResult;
        double fn;
        double fN;

        //Calcul de la valeur de PI
        fPI = atan ( 1.0 ) * 4.0;

        //Les autres valeurs
        fn = ln;
        fN = lN;

        switch (lShape)
        {
            case WINDOWSHAPE_MODIFIATESINE: //Modified Sine Window (VORBIS like)
                f1 = (fn + 0.5)*(fPI/(2.0*fN));
                f2 = sin ( f1 ) * sin ( f1 );
                f3 = (fPI/2.0) * f2;
                fResult = sin ( f3 );
                break;


            case WINDOWSHAPE_SINE:
            default: //Standard Sine - MP3 et MPEG-2 AAC
                f1 = ((fn+0.5)/(fN*2.0)); //0....1
                f2 = sin ( fPI * f1 );
                fResult = f2;
                break;

            case WINDOWSHAPE_HANNING:
                f1 = ( fn + 0.5F ) / ( fN*2.0 );
                f1 = pow ( sin ( f1 * fPI ), 2.0F );
                fResult = sin ( f1 * 0.5F * fPI );
                break;
        }

        return fResult;
    }

    void _Window_ProcessShape_KaiserBesselDerived ( double * pfWindowHalf64, float * pfWindowHalf32, long lSizeHalf, double fAlpha, long lProcessLeft0_Right1 )
    {
        long lSizeTotal, i;
        long lIndex;
        long lIndexStart;
        long lIndexIncr;
        double fSumValue;
        double fAlphaPI;

        lSizeTotal  = lSizeHalf*2;

        fAlphaPI    = fAlpha * 3.1415926535897932384626433832795;
        fSumValue   = 0;

        //Sens de progression
        if (!lProcessLeft0_Right1) { lIndexStart = 0;           lIndexIncr =  1; }
        else                       { lIndexStart = lSizeHalf-1; lIndexIncr = -1; }

        for (i=0,lIndex=lIndexStart; i<lSizeHalf; i++,lIndex+=lIndexIncr)
        {
            fSumValue += _Window_ProcessShape_BesselI0 ( fAlphaPI * sqrt(1.0 - pow(4.0*(double)i/(double)lSizeTotal-1.0, 2 ) ) );
            if (pfWindowHalf64) pfWindowHalf64[lIndex] = fSumValue;
            if (pfWindowHalf32) pfWindowHalf32[lIndex] = (float)fSumValue;
        }

        // need to add one more value to the nomalization factor at size/2:
        fSumValue += _Window_ProcessShape_BesselI0 ( fAlphaPI * sqrt(1.0 - pow(4.0*(double)lSizeHalf/(double)lSizeTotal-1.0, 2) ) );
        fSumValue = 1.0F / fSumValue;

        // normalize the window and fill in the righthand side of the window:
        for (i=0,lIndex=lIndexStart; i<lSizeHalf; i++,lIndex+=lIndexIncr)
        {
            if (pfWindowHalf64) pfWindowHalf64[lIndex] =        sqrt ( pfWindowHalf64[lIndex] * fSumValue );
            if (pfWindowHalf32) pfWindowHalf32[lIndex] = (float)sqrt ( pfWindowHalf32[lIndex] * fSumValue );
        }
   }
        

//////////////////////////////////////////////////////////////////////////


//////////////////////////////
//
// KBDWindow -- Kaiser Bessel Derived Window
//      fills the input window array with size samples of the
//      KBD window with the given tuning parameter alpha.
//
/*
#ifndef PI
   #define PI 3.14159265358979323846264338328
#endif

void KBDWindow(double* window, int size, double alpha) {
   double sumvalue = 0.0;
   int i;
   
   for (i=0; i<size/2; i++) {
      sumvalue += BesselI0(PI * alpha * sqrt(1.0 - pow(4.0*i/size - 1.0, 2)));
      window[i] = sumvalue;
   }

   // need to add one more value to the nomalization factor at size/2:
   sumvalue += BesselI0(PI * alpha * sqrt(1.0 - pow(4.0*(size/2)/size-1.0, 2)));

   // normalize the window and fill in the righthand side of the window:
   for (i=0; i<size/2; i++) {
      window[i] = sqrt(window[i]/sumvalue);
      window[size-1-i] = window[i];
   }
}
*/

    //////////////////////////////
    //
    // BesselI0 -- Regular Modified Cylindrical Bessel Function (Bessel I).
    //
    double _Window_ProcessShape_BesselI0 ( double x )
    {
        double denominator;
        double numerator;
        double z;

        if (x == 0.0)
        {
            return 1.0;
        } 
        else
        {
            z = x * x;
            numerator = (z* (z* (z* (z* (z* (z* (z* (z* (z* (z* (z* (z* (z* 
                     (z* 0.210580722890567e-22  + 0.380715242345326e-19 ) +
                         0.479440257548300e-16) + 0.435125971262668e-13 ) +
                         0.300931127112960e-10) + 0.160224679395361e-7  ) +
                         0.654858370096785e-5)  + 0.202591084143397e-2  ) +
                         0.463076284721000e0)   + 0.754337328948189e2   ) +
                         0.830792541809429e4)   + 0.571661130563785e6   ) +
                         0.216415572361227e8)   + 0.356644482244025e9   ) +
                         0.144048298227235e10);

            denominator = (z*(z*(z-0.307646912682801e4)+
                              0.347626332405882e7)-0.144048298227235e10);
        }

        return -numerator/denominator;
    }
