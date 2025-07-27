#include "sam_header.h"




//void            SAM_FIR_HWindowKBD             ( double * pf64HWindow, long lSizeTotal, double fAlpha );
//void            SAM_FIR_HWindowBlackmanHarris  ( double * pf64HWindow, long lSizeTotal );
double          SAM_FIR_BesselI0               ( double x );        
double          SAM_FIR_BesselIZero            ( double x );
long            SAM_FIR_Design                 ( long lOrder, float * pf32Coef, float f32RatioCut, long lWindowType, float f32WindowParam );
void            SAM_FIR_ReverseData ( double * pf64Data, long lLenght );

void            SAM_FIR_HWindowKBD ( double * pf64HWindow, long lSizeTotal, double fAlpha, long * plReversed );
void            SAM_FIR_HWindowBlackmanHarris  ( double * pf64HWindow, long lSizeTotal, long * plReversed );
void            SAM_FIR_HWindowHamming  ( double * pf64HWindow, long lSizeTotal, long * plReversed );
void            SAM_FIR_HWindowLanczos  ( double * pf64HWindow, long lSizeTotal, long * plReversed );
void            SAM_FIR_HWindowRiesz  ( double * pf64HWindow, long lSizeTotal, double fAlpha, long * plReversed );

    /*
    
        BesselI0

        The regular modified cylindrical Bessel function (Bessel I)

    */    
    double          SAM_FIR_BesselI0               ( double f64_x )
    {
        double f64Denominator;
        double f64Numerator;
        double f64_z;

        if (f64_x == 0.0)
            return 1.0;

        f64_z           = f64_x * f64_x;
        f64Numerator    = (f64_z* (f64_z* (f64_z* (f64_z* (f64_z* (f64_z* (f64_z* (f64_z* (f64_z* (f64_z* (f64_z* (f64_z* (f64_z* 
                            (f64_z* 0.210580722890567e-22   + 0.380715242345326e-19 ) +
                                    0.479440257548300e-16 ) + 0.435125971262668e-13 ) +
                                    0.300931127112960e-10 ) + 0.160224679395361e-7  ) +
                                    0.654858370096785e-5  ) + 0.202591084143397e-2  ) +
                                    0.463076284721000e0   ) + 0.754337328948189e2   ) +
                                    0.830792541809429e4   ) + 0.571661130563785e6   ) +
                                    0.216415572361227e8   ) + 0.356644482244025e9   ) +
                                    0.144048298227235e10  );

        f64Denominator  = ( f64_z * ( f64_z * ( f64_z     - 0.307646912682801e4 ) +
                                    0.347626332405882e7   ) - 0.144048298227235e10  );

        return -f64Numerator/f64Denominator;
    }

    /*
        
        BesselIZero (from MPlayer)
    
        Computes the 0th order modified Bessel function of the first kind.  
    
    
        y = sum( (x/(2*n))^2 )
             n

    */
    #define BIZ_EPSILON 1E-21 // Max error acceptable 


    double SAM_FIR_BesselIZero ( double x )
    { 
        double temp;
        double sum;
        double u;
        double halfx;
        long   n;

        sum   = 1.0;
        u     = 1.0;
        halfx = x/2.0;
        n     = 1;

        do {
            temp = halfx/(double)n;
            u *=temp * temp;
            sum += u;
            n++;
        } while (u >= BIZ_EPSILON * sum);
        return sum;
    }

    /*
        
        Half of the Kaiser Bessel Derived Window.
        
        (Alexia Gossa)

    */
    void SAM_FIR_HWindowKBD ( double * pf64HWindow, long lSizeTotal, double fAlpha, long * plReversed )
    {
        long lIndex;
        long lSizeHalf;
        double fSumValue;
        double fAlphaPI;

        fAlphaPI    = fAlpha * sam_PI;
        fSumValue   = 0;

        //Sens de progression
        lSizeHalf = lSizeTotal>>1;

        for (lIndex=0;lIndex<lSizeHalf;lIndex++)
        {
            fSumValue += SAM_FIR_BesselIZero ( fAlphaPI * sqrt(1.0 - pow(4.0*(double)lIndex/(double)lSizeTotal-1.0, 2 ) ) );
            pf64HWindow[             lIndex] = fSumValue;
        }

        // need to add one more value to the nomalization factor at size/2:
        fSumValue += SAM_FIR_BesselIZero ( fAlphaPI * sqrt(1.0 - pow(4.0*(double)lSizeHalf/(double)lSizeTotal-1.0, 2) ) );
        fSumValue = 1.0F / fSumValue;

        // normalize the window and fill in the righthand side of the window:
        for (lIndex=0; lIndex<lSizeHalf;lIndex++)
        {
            pf64HWindow[lIndex] = sqrt ( pf64HWindow[lIndex] * fSumValue );
        }
        
        if (plReversed) *plReversed = 0;
    }


    /*
        
        Half of the Backman-Harris Window.

        Const from Wikipedia
        
        (Alexia Gossa)
    */
    
    void            SAM_FIR_HWindowBlackmanHarris  ( double * pf64HWindow, long lSizeTotal, long * plReversed )
    {
        long i,m;
        double f64Inv1SizeAnd1;
        double f64Inv2SizeAnd1;
        double f64Inv3SizeAnd1;
        

        m = lSizeTotal>>1;
        //f64Inv1SizeMinus1 = 2 * sam_PI / (double)(lSizeTotal-1);
        f64Inv1SizeAnd1 = 2 * sam_PI / (double)(lSizeTotal-1);
        f64Inv2SizeAnd1 = 2*f64Inv1SizeAnd1;
        f64Inv3SizeAnd1 = 3*f64Inv1SizeAnd1;

        for (i=0;i<m;i++)
        {
            pf64HWindow[i]  = 0.35875 - 
                              0.48829 * cos ( (double)i * f64Inv1SizeAnd1 ) + 
                              0.14128 * cos ( (double)i * f64Inv2SizeAnd1 ) -
                              0.01168 * cos ( (double)i * f64Inv3SizeAnd1 );
        }
        
        if (plReversed) *plReversed = 0;
    }

    /*
        
        Half of the Hamming Window.

        Const from Wikipedia
        
        (Alexia Gossa)
    */
    
    void            SAM_FIR_HWindowHamming  ( double * pf64HWindow, long lSizeTotal, long * plReversed )
    {
        long i,m;
        double f64Inv1SizeMinus1;

        m = lSizeTotal>>1;
        f64Inv1SizeMinus1 = 2 * sam_PI / (double)(lSizeTotal-1);

        for (i=0;i<m;i++)
        {
            pf64HWindow[i]  = 0.54 - 
                              ( 0.46 * cos ( (double)i * f64Inv1SizeMinus1 ) );
        }
        
        if (plReversed) *plReversed = 0;
    }

    /*

        Half of the Lanczos Window

    */
    void            SAM_FIR_HWindowLanczos  ( double * pf64HWindow, long lSizeTotal, long * plReversed )
    {
        long i,m;
        double f64Inv1SizeAnd1;
        double npi;
        double Np1;
        double fAlpha;
        
        fAlpha = 2.0F; //1.7F;

        m = lSizeTotal>>1;
        //f64Inv1SizeAnd1 = 2 / (double)(lSizeTotal-1);

        Np1 = (double)(lSizeTotal+1);

        for (i=1;i<m;i++)
        {
            npi = (double)i * sam_PI;
            //pf64HWindow[i]  = sin ( npi / Np1 ) / (npi/Np1);
            
            //pf64HWindow[i] = sin ( npi / Np1 ) / (npi/Np1);
            //pf64HWindow[i] = fAlpha * sin ( npi / Np1 ) * sin ( (npi/Np1) * fAlpha ) / ( pow ( (1/Np1) * (double)i, 2 ) * sam_PI * sam_PI );
            
            npi = (double)i * sam_PI * fAlpha;
            //npi /= fAlpha;
            pf64HWindow[i] = sin ( npi / Np1 ) / (npi/Np1);
            
            
        }
        pf64HWindow[0  ] = 1;
        //pf64HWindow[m-1] = 0;
        
        if (plReversed) *plReversed = 1;
    }
/*
        Np1 = (double)(lSizeTotal+1);
        Np1 = sam_PI / Np1;
        api = sam_PI * fAlpha;

        for (i=1;i<m-1;i++)
        {
            npi = (double)i;
            //pf64HWindow[i]  = sin ( npi / Np1 ) / (npi/Np1);
            
            //pf64HWindow[i] = sin ( npi / Np1 ) / (npi/Np1);
            //pf64HWindow[i] = fAlpha * sin ( npi / Np1 ) * sin ( (npi/Np1) * fAlpha ) / ( pow ( (1/Np1) * (double)i, 2 ) * sam_PI * sam_PI );
            
            npi = (double)i;
            npi *= api;
            pf64HWindow[i] = sin ( npi * Np1 ) / (npi * Np1);
            
            
        }
        pf64HWindow[0  ] = 1;
        pf64HWindow[m-1] = 0;

*/

    /*

        Half of the Riesz Window*
        
        Default alpha = 2.0

    */
    void            SAM_FIR_HWindowRiesz  ( double * pf64HWindow, long lSizeTotal, double fAlpha, long * plReversed )
    {
        long i,m;
        double f64Inv1SizeAnd1;
        double npi;
        double Np1;

        m = lSizeTotal>>1;
        f64Inv1SizeAnd1 = 1 / (double)(1+lSizeTotal);

        Np1 = (double)(1+lSizeTotal);

        for (i=0;i<m;i++)
        {
            npi = (double)i * sam_PI;
            pf64HWindow[i]  = 1-pow ( (double)i / (Np1), fAlpha );
        }
    }


    /*
        
        Half of the Gossa modified KBD Window.

        Measures :

            Fs = 48000 Hz
            Fc = 5622 Hz

            1st side lobe : -48 dB @ 6351 Hz
            2nd side lobe : -43 dB @ 6656 Hz
            Other lobes   : -49 dB and better

            The mod kbd have a better rejection on the first lobe (better for resampling)... better than 15 dB
            

        (Alexia Gossa)
    */
    
    void            SAM_FIR_HWindowGossaModKBD  ( double * pf64HWindow, long lSizeTotal, double fAlpha, long * plReversed )
    {
        long i,m;
        double f1, f2;
        long lReversed;

        m = lSizeTotal>>1;
        SAM_FIR_HWindowKBD ( pf64HWindow, lSizeTotal, 1, &lReversed );
        if (lReversed) SAM_FIR_ReverseData ( pf64HWindow, lSizeTotal>>1 );
        

        f1 = (sam_PI*0.5)/(double)m;
        f2 = f1*0.5; //0.0;
        for (i=0;i<m;i++)
        {
            pf64HWindow[i] *= pow ( sin ( f2 ), 0.7 );
            f2 += f1;
        }
        
        if (plReversed) *plReversed = 0;
    }

    void SAM_FIR_ReverseData ( double * pf64Data, long lLenght )
    {
        static double f64ReverseTemp[1024*1024];
        long i;
        
        memcpy ( f64ReverseTemp, pf64Data, lLenght * sizeof(double) );
        for (i=0;i<lLenght;i++)
        {
            pf64Data[i] = f64ReverseTemp[lLenght-1-i];
        }
    }

    /*
        Design a FIR filter...
        
        f32RatioCut     must be : 0...0.5
        
        lOrder          2...512 (even only)

        (Alexia Gossa)




        return -1 on error.    

    */
    long SAM_FIR_Design ( long lOrder, float * pf32Coef, float f32RatioCut, long lWindowType, float f32WindowParam )
    {
        static  double  f64Coef[1024*1024];
                double  *pf64SincDivisor;
                double  f64AlphaCut;
                double  f64Gain;
                double  f64Rad_AC, f64RadIncr_AC;
                long    lHalfOrder;
                long    i, lIndex;
                double  f1, f2;
                long    lReversed;

        // Verify input data (even only)
        if ( (!lOrder) || (!pf32Coef) || (lOrder>(1024*1024)) || (lOrder&1) )
            return -1;

        // We work on a half window (it's faster)
        lHalfOrder      = lOrder>>1;
        lReversed       = 0;
        
        // Process the window
        switch (lWindowType)
        {
            case sam_FIR_RECTANGLE:         //OK ! Validated on 2008/30/07 !!!
                for (i=0;i<lHalfOrder;i++) f64Coef[i] = 1;
                break;
            
            case sam_FIR_KAISERBESSEL:      //OK ! Validated on 2008/30/07 !!!
                SAM_FIR_HWindowKBD ( f64Coef, lOrder, f32WindowParam, &lReversed );
                break;

            case sam_FIR_BLACKMANHARRIS:    //OK ! Validated on 2008/30/07 !!!
                SAM_FIR_HWindowBlackmanHarris ( f64Coef, lOrder, &lReversed );
                break;

            case sam_FIR_HAMMING:           //OK ! Validated on 2008/30/07 !!!
                SAM_FIR_HWindowHamming ( f64Coef, lOrder, &lReversed );
                break;

            case sam_FIR_LANCZOS:           //OK ! Validated on 2008/30/07 !!!
                SAM_FIR_HWindowLanczos ( f64Coef, lOrder, &lReversed );
                break;

            case sam_FIR_RIESZ:             //OK ! Validated on 2008/30/07 !!!
                SAM_FIR_HWindowRiesz ( f64Coef, lOrder, f32WindowParam, &lReversed );
                break;

            case sam_FIR_GOSSAMODKBD:       //OK ! Validated on 2008/30/07 !!!
                SAM_FIR_HWindowGossaModKBD  ( f64Coef, lOrder, 0.0, &lReversed );
                break;

            case sam_FIR_RIESZ_TO_GOSSAMODKBD:
                SAM_FIR_HWindowGossaModKBD ( f64Coef, lOrder, 2.0, &lReversed );
                if (lReversed) SAM_FIR_ReverseData ( f64Coef, lHalfOrder );
                
                SAM_FIR_HWindowRiesz ( f64Coef+lHalfOrder, lOrder, 2.0, &lReversed );
                if (lReversed) SAM_FIR_ReverseData ( f64Coef+lHalfOrder, lHalfOrder );
                
                if (f32WindowParam>1.0) f32WindowParam = 1.0;
                for (i=0;i<lHalfOrder;i++)
                {
                    f64Coef[i] = f64Coef[i] * f32WindowParam + f64Coef[i+lHalfOrder] * (1-f32WindowParam);
                }                
                lReversed = 1;
                break;
                
        }
        
        if (lReversed) SAM_FIR_ReverseData ( f64Coef, lHalfOrder );

        // Set values
        pf64SincDivisor = (double *)sam_dwSincDivisorLUT512;
        f64AlphaCut     = (double)f32RatioCut;
        f64AlphaCut    *= 2.0 * sam_PI;    
        f64Rad_AC       = 0.5 * f64AlphaCut;
        f64RadIncr_AC   =       f64AlphaCut;
        f64Gain         = 0.0;

        // Apply filter -> sin(x)/x
        for (i=0;i<lHalfOrder;i++)
        {
            lIndex = lHalfOrder-1-i;
            f1 = (double)i;
            f1 += 0.50;
            f1 *= sam_PI;

            
            f64Coef[lIndex] *= sin(f64Rad_AC) / f1; //pf64SincDivisor[i];
            //f64Coef[i]              = sin(f64Rad_AC) * (1.0F/f64Rad_AC); //pf64SincDivisor[i];
            //f64Coef[i]              = sin(f64Rad_AC) / f64Rad_AC;
            
            //f1 = (double)i;
            //f1 += 0.5F;            
            //f1 += 0.5F;
            //f1 /= 
            //f1 *= 2.0F * sam_PI * f64AlphaCut * 0.25F;
            //f2 = sin ( f1 ) / f1;
            
            //f2 = sin ( 2 * sam_PI * f32RatioCut * f1 ) / (f1 * sam_PI);
            //f64Coef[lHalfOrder-i-1] = f2;
            
            
            f64Gain                 += f64Coef[lIndex];
            f64Rad_AC               += f64RadIncr_AC;
        }
        
        /*
        {
            float f1, f2;
            long n;
            
            for (i=0;i<lOrder;i++)
            {
                n = i - lHalfOrder;
                f1 = ( n ) + fPhase; //+ 0.5F + fPhase );
                f2 = f1 * f64AlphaCut;
                
                if (f1==0) f64Coef[i] = 1.0F;
                else       f64Coef[i] = sin(f2) / f1; //pf64SincDivisor[i];
                f64Gain += f64Coef[i];
            }
        }
                
        
        f64Gain=1.0/f64Gain;
        for (i=0;i<lOrder;i++)
        {
            pf32Coef[i]             = (float)(f64Coef[i] * f64Gain);
        }
        */

        // Normalization and dupplication (half to full)
        f64Gain=0.5/f64Gain; //Half window need half value
        for (i=0;i<lHalfOrder;i++)
        {
            pf32Coef[i]             = (float)(f64Coef[i] * f64Gain);
            pf32Coef[lOrder-i-1]    = pf32Coef[i];
        }
        /*
        for (i=0;i<lHalfOrder-1;i++)
        {
            //pf32Coef[i]             = (float)(f64Coef[i] * f64Gain);
            pf32Coef[i+lHalfOrder]    = pf32Coef[lHalfOrder-2-i];
        }
        pf32Coef[i+lHalfOrder] = 0.0F;
        */

        //Add more precision and less distorsion
        /*f64Gain = 0.0;
        for (i=0;i<lHalfOrder;i++)
        {
            f64Gain += pf32Coef[i];
        }
        f64Gain -= 0.5;
        f64Coef[lOrder-1] -= f64Gain;
        f64Coef[0       ] -= f64Gain;*/
        
            

        return 0;
    }




