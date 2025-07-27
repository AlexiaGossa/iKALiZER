#include <math.h>
#include "audiotools.h"



#ifndef IN
#define IN
#endif

#ifndef OUT
#define OUT
#endif

//#define  maxPrimeFactor        37
//#define  maxPrimeFactor        4096
#define absoluteMaxPrimeFactor     32768

#define  maxPrimeFactorDiv2    (maxPrimeFactor+1)/2
#define  maxFactorCount        20

typedef struct {
    long        lNbPoints;
    long        lSoFarRadix[maxFactorCount];
    long        lActualRadix[maxFactorCount];
    long        lRemainRadix[maxFactorCount];
    long        lnFactor;

    long     maxPrimeFactor;
    FLOAT32   pi;
    long     groupOffset,dataOffset,blockOffset,adr;
    long     groupNo,dataNo,blockNo,twNo;
    FLOAT32   omega, tw_re,tw_im;

    FLOAT32   *twiddleRe, 
             *twiddleIm,
             *trigRe, 
             *trigIm,
             *zReIm;
             //*zRe, 
             //*zIm;
    FLOAT32   *vRe, *vIm;
    FLOAT32   *wRe, *wIm;
    /*FLOAT32   twiddleRe[maxPrimeFactor], 
             twiddleIm[maxPrimeFactor],
             trigRe[maxPrimeFactor], 
             trigIm[maxPrimeFactor],
             zRe[maxPrimeFactor], 
             zIm[maxPrimeFactor];
    FLOAT32   vRe[maxPrimeFactorDiv2], vIm[maxPrimeFactorDiv2];
    FLOAT32   wRe[maxPrimeFactorDiv2], wIm[maxPrimeFactorDiv2];*/
} _FFT_JJN_DATA;

void FFT_JJN_factorize                  ( long n, long *nFact, long fact[] );
long FFT_JJN_transTableSetup            ( long sofar[], long actual[], long remain[], long *nFact, long *nPoints );
void FFT_JJN_permute                    ( long nPoint, long nFact, long fact[], long remain[], FLOAT32 xRe[], FLOAT32 xIm[], FLOAT32 yRe[], FLOAT32 yIm[] );


void FFT_JJN_radix2_OutInterleaved ( FLOAT32 IN *aRe, FLOAT32 IN *aIm, FLOAT32 OUT *zRe, FLOAT32 OUT *zIm, long OIl );
void FFT_JJN_radix3_OutInterleaved ( FLOAT32 IN *aRe, FLOAT32 IN *aIm, FLOAT32 OUT *zRe, FLOAT32 OUT *zIm, long OIl );
void FFT_JJN_radix4_OutInterleaved ( FLOAT32 IN *aRe, FLOAT32 IN *aIm, FLOAT32 OUT *zRe, FLOAT32 OUT *zIm, long OIl );
void FFT_JJN_radix5_OutInterleaved ( FLOAT32 IN *aRe, FLOAT32 IN *aIm, FLOAT32 OUT *zRe, FLOAT32 OUT *zIm, long OIl );
void FFT_JJN_radix7_OutInterleaved ( FLOAT32 IN *aRe, FLOAT32 IN *aIm, FLOAT32 OUT *zRe, FLOAT32 OUT *zIm, long OIl );
void FFT_JJN_radix8_OutInterleaved ( FLOAT32 IN *aRe, FLOAT32 IN *aIm, FLOAT32 OUT *zRe, FLOAT32 OUT *zIm, long OIl );
void FFT_JJN_radix10_OutInterleaved ( FLOAT32 IN *aRe, FLOAT32 IN *aIm, FLOAT32 OUT *zRe, FLOAT32 OUT *zIm, long OIl );
void FFT_JJN_radixOdd_OutInterleaved ( FLOAT32 IN *aRe, FLOAT32 IN *aIm, FLOAT32 OUT *zRe, FLOAT32 OUT *zIm, long OIl );

void FFT_JJN_radix2_OutInterleavedFast ( FLOAT32 IN *aReIm, FLOAT32 OUT *zRe, FLOAT32 OUT *zIm, long OIl );
void FFT_JJN_radix3_OutInterleavedFast ( FLOAT32 IN *aReIm, FLOAT32 OUT *zRe, FLOAT32 OUT *zIm, long OIl );
void FFT_JJN_radix4_OutInterleavedFast ( FLOAT32 IN *aReIm, FLOAT32 OUT *zRe, FLOAT32 OUT *zIm, long OIl );
void FFT_JJN_radix5_OutInterleavedFast ( FLOAT32 IN *aReIm, FLOAT32 OUT *zRe, FLOAT32 OUT *zIm, long OIl );
void FFT_JJN_radix7_OutInterleavedFast ( FLOAT32 IN *aReIm, FLOAT32 OUT *zRe, FLOAT32 OUT *zIm, long OIl );
void FFT_JJN_radix8_OutInterleavedFast ( FLOAT32 IN *aReIm, FLOAT32 OUT *zRe, FLOAT32 OUT *zIm, long OIl );
void FFT_JJN_radix10_OutInterleavedFast ( FLOAT32 IN *aReIm, FLOAT32 OUT *zRe, FLOAT32 OUT *zIm, long OIl );
void FFT_JJN_radixOdd_OutInterleavedFast ( FLOAT32 IN *aReIm, FLOAT32 OUT *zRe, FLOAT32 OUT *zIm, long OIl,
                                           long radix, 
                                           FLOAT32 * vRe, FLOAT32 * vIm, 
                                           FLOAT32 * wRe, FLOAT32 * wIm,
                                           FLOAT32 * trigRe, FLOAT32 * trigIm );



void FFT_JJN_radix2 ( FLOAT32 *aRe, FLOAT32 *aIm );
void FFT_JJN_radix3 ( FLOAT32 *aRe, FLOAT32 *aIm );
void FFT_JJN_radix4 ( FLOAT32 *aRe, FLOAT32 *aIm );
void FFT_JJN_radix5 ( FLOAT32 *aRe, FLOAT32 *aIm );
void FFT_JJN_radix7 ( FLOAT32 *aRe, FLOAT32 *aIm );
void FFT_JJN_radix8 ( FLOAT32 *aRe, FLOAT32 *aIm );
void FFT_JJN_radix10 ( FLOAT32 *aRe, FLOAT32 *aIm );
void FFT_JJN_radixOdd ( long radix, 
                       FLOAT32 * zRe, FLOAT32 * zIm, 
                       FLOAT32 * vRe, FLOAT32 * vIm, 
                       FLOAT32 * wRe, FLOAT32 * wIm,
                       FLOAT32 * trigRe, FLOAT32 * trigIm );
