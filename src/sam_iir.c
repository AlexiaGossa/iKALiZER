#include "sam_header.h"
#include "stdlib.h"




typedef struct {
    double  pReal[64];
    double  pImag[64];
    double  z[64];
    double  aCoeff[64];
    double  bCoeff[64];

    double  fN;
    double  fCut;
    
    double  rate;
    double  ripple;
    long    order;          // 1 to ...
    long    prototype;      // 0 = butterworth 1 = chebychev
    long    freqPoints;
    
    long    lEnableHighPass;

} sam_filter_iir_t;



//void SAM_IIRFilterSetIIR ( sam_filter_iir_t * piir, long lPrototype, long lOrder, float fRipple, float fSampleRate, float fCutFreq );

void    SAM_IIR_Design ( long lOrder, float *pf32ABCoef, long * plABCoefCount, float f32RatioCut, long lEnableHighPass );
void    SAM_IIR_Init ( sam_filter_iir_t * piir, long lPrototype, long lOrder, float fRipple, float fSampleRate, float fCutFreq );
void    SAM_IIR_ProcessPolesAndZeros ( sam_filter_iir_t * piir );
void    SAM_IIR_Normalize ( sam_filter_iir_t * piir );
void    SAM_IIR_ApplyDesign ( sam_filter_iir_t * piir );
double  SAM_IIR_PowBy2 ( double a );
long    SAM_IIR_IsOdd ( long a );

double  SAM_IIR_PowBy2 ( double a )
{
    return a*a;
}

long    SAM_IIR_IsOdd ( long a )
{
    return ((a%2)!=0)?(1):(0);
}


void    SAM_IIR_Design ( long lOrder, float *pf32ABCoef, long * plABCoefCount, float f32RatioCut, long lEnableHighPass )
{
    sam_filter_iir_t    iir;
    long i,j;
    
    if (lEnableHighPass)
        f32RatioCut = 1 - f32RatioCut;

    SAM_IIR_Init ( &iir, 1, lOrder, 1, 48000, 24000 * f32RatioCut );
    iir.lEnableHighPass = lEnableHighPass;

    SAM_IIR_ApplyDesign ( &iir );
    
    /*if (lEnableHighPass)
    {
        for (i=0;i<=lOrder;i++)
        {
            iir.aCoeff[i]   = (1.0F - iir.aCoeff[i]);
            if (i&1) iir.aCoeff[i] = -iir.aCoeff[i+1];
        }
    }
    */
    
    for (i=0,j=0;i<=lOrder;i++)
    {
        pf32ABCoef[j  ] = (float)iir.aCoeff[i];
        pf32ABCoef[j+1] = (float)iir.bCoeff[i];
        j+=2;
    }
    

    if (plABCoefCount)
        *plABCoefCount = lOrder+1;
}


void SAM_IIR_Init ( sam_filter_iir_t * piir, long lPrototype, long lOrder, float fRipple, float fSampleRate, float fCutFreq )
{
    piir->rate          = fSampleRate;
    piir->fN            = 0.5 * fSampleRate;
    //piir->fp1           = fCutFreq;
    piir->fCut          = fCutFreq;
    piir->prototype     = lPrototype;
    piir->ripple        = fRipple;
    piir->order         = abs(lOrder);
    piir->freqPoints    = 256;
}

void SAM_IIR_ProcessPolesAndZeros ( sam_filter_iir_t * piir )
{
    // determines poles and zeros of IIR filter
    // based on bilinear transform method
    double  *pReal      = piir->pReal;
    double  *pImag      = piir->pImag;
    double  *z          = piir->z;
    double  ln10        = log(10.0);
    long    k;
    long    n;
    long    ir;
    long    n1;
    long    n2;
    long    m;
    double  f1;
    double  tanw1;
    double  tansqw1;
    double  t, a, r, i;
    double  b3;
    double  d, e, x, b, c3, c4, c5;

    for(k = 1; k <= piir->order; k++)
    {
        pReal[k] = 0.0;
        pImag[k] = 0.0;
    }

    // Butterworth, Chebyshev parameters
    n = piir->order;
    ir = n % 2;
    n1 = n + ir;
    n2 = (3*n + ir)/2 - 1;
    
    
    f1 = piir->fCut;
    
    
    

    tanw1 = tan(0.5*sam_PI*f1/piir->fN);
    tansqw1 = SAM_IIR_PowBy2(tanw1);

    // Real and Imaginary parts of low-pass poles
    a = 1.0;
    r = 1.0;
    i = 1.0;
    for (k = n1; k <= n2; k++)
    {
        t = 0.5*(2*k + 1 - ir)*sam_PI/(double)n;

        switch (piir->prototype)
        {
            case 0:
                b3 = 1.0 - 2.0*tanw1*cos(t) + tansqw1;
                r = (1.0 - tansqw1)/b3;
                i = 2.0*tanw1*sin(t)/b3;
                break;
            case 1:
                d = 1.0 - exp(-0.05*piir->ripple*ln10);
                e = 1.0 / sqrt(1.0 / SAM_IIR_PowBy2(1.0 - d) - 1.0);
                x = pow(sqrt(e*e + 1.0) + e, 1.0/(double)n);
                a = 0.5*(x - 1.0/x);
                b = 0.5*(x + 1.0/x);
                c3 = a*tanw1*cos(t);
                c4 = b*tanw1*sin(t);
                c5 = SAM_IIR_PowBy2(1.0 - c3) + SAM_IIR_PowBy2(c4);
                r = 2.0*(1.0 - c3)/c5 - 1.0;
                i = 2.0*c4/c5;
                break;
        }
        m = 2*(n2 - k) + 1;
        pReal[m + ir]     = r;
        pImag[m + ir]     = fabs(i);
        pReal[m + ir + 1] = r;
        pImag[m + ir + 1] = - fabs(i);
    }
    if (SAM_IIR_IsOdd(n)) 
    {
        if (piir->prototype == 0) r = (1.0 - tansqw1)/(1.0 + 2.0*tanw1+tansqw1); 
        if (piir->prototype == 1) r = 2.0/(1.0 + a*tanw1) - 1.0;
        pReal[1] = r;
        pImag[1] = 0.0;
    }

    if (piir->lEnableHighPass)
    {
        for (m = 1; m <= n; m++)
        {
            z[m]= 1.0;
            pReal[m] = -pReal[m];
        }
    }
    else
    {
        for (m = 1; m <= n; m++)
            z[m]= -1.0;
    }
}


void SAM_IIR_ApplyDesign ( sam_filter_iir_t * piir )
{
    double * aCoeff = piir->aCoeff;
    double * bCoeff = piir->bCoeff;
    long i, k, n, p, pairs, m;

    double newA[64];
    double newB[64];

    double alpha1;
    double alpha2;
    double beta1;
    double beta2;


    SAM_IIR_ProcessPolesAndZeros ( piir ); // find filter poles and zeros

    // compute filter coefficients from pole/zero values
    aCoeff[0]= 1.0;
    bCoeff[0]= 1.0;
    for (i = 1; i <= piir->order; i++)
    {
        aCoeff[i] = 0.0;
        bCoeff[i] = 0.0;
    }
    k = 0;
    n = piir->order;
    pairs = n/2;
    if (SAM_IIR_IsOdd(piir->order))
    {
        // first subfilter is first order
        aCoeff[1] = - piir->z[1];
        bCoeff[1] = - piir->pReal[1];
        k = 1;
    }

    for (p = 1; p <= pairs; p++)
    {
        m = 2*p - 1 + k;
        alpha1      = - (piir->z[m] + piir->z[m+1]);
        alpha2      = piir->z[m]*piir->z[m+1];
        beta1       = - 2.0*piir->pReal[m];
        beta2       = SAM_IIR_PowBy2(piir->pReal[m]) + SAM_IIR_PowBy2(piir->pImag[m]);
        newA[1]     = aCoeff[1] + alpha1*aCoeff[0];
        newB[1]     = bCoeff[1] + beta1 *bCoeff[0];
        for (i = 2; i <= n; i++)
        {
            newA[i] = aCoeff[i] + alpha1*aCoeff[i-1] + alpha2*aCoeff[i-2];
            newB[i] = bCoeff[i] + beta1 *bCoeff[i-1] + beta2 *bCoeff[i-2];
        }
        for (i = 1; i <= n; i++)
        {
            aCoeff[i] = newA[i];
            bCoeff[i] = newB[i];
        }
    }

    SAM_IIR_Normalize ( piir );
}


void SAM_IIR_Normalize ( sam_filter_iir_t * piir )
{
    // filter gain at uniform frequency intervals
    float   g[512];
    double  theta, s, c, sac, sas, sbc, sbs;
    long    i, k;
    //float[] g = new float[freqPoints+1];
    
    float gMax;
    float sc;
    double t;
    float normFactor;

    
    gMax = -100.0f;
    sc = 10.0f/(float)log(10.0f);
    t = sam_PI / piir->freqPoints;

    for (i = 0; i <= piir->freqPoints; i++) 
    {
        theta = i*t;
        if (i == 0) theta = sam_PI*0.0001;
        if (i == piir->freqPoints) theta = sam_PI*0.9999;
        sac = 0.0f;
        sas = 0.0f;
        sbc = 0.0f;
        sbs = 0.0f;
        for (k = 0; k <= piir->order; k++) 
        {
            c = cos(k*theta);
            s = sin(k*theta);
            sac += c*piir->aCoeff[k];
            sas += s*piir->aCoeff[k];
            sbc += c*piir->bCoeff[k];
            sbs += s*piir->bCoeff[k];
        }
        g[i] = sc*(float)log((SAM_IIR_PowBy2(sac) + SAM_IIR_PowBy2(sas))/(SAM_IIR_PowBy2(sbc) + SAM_IIR_PowBy2(sbs)));
        gMax = max(gMax, g[i]);
    }
    // normalise to 0 dB maximum gain
    for (i=0; i<=piir->freqPoints; i++)
        g[i] -= gMax;
    
    // normalise numerator (a) coefficients
    normFactor = (float)pow(10.0, -0.05*gMax);
    for (i=0; i<=piir->order; i++) 
        piir->aCoeff[i] *= normFactor;
}















