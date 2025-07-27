#include "audiotools_fft_jjn.h"

/****************************************************************************
  After N is factored the parameters that control the stages are generated.
  For each stage we have:
    sofar   : the product of the radices so far.
    actual  : the radix handled in this stage.
    remain  : the product of the remaining radices.
 ****************************************************************************/

long FFT_JJN_transTableSetup(long sofar[], long actual[], long remain[],
                     long *nFact,
                     long *nPoints )
{
    long i;

    FFT_JJN_factorize ( *nPoints, nFact, actual );
    if (actual[*nFact] > absoluteMaxPrimeFactor)
    {
        //printf("\nPrime factor of FFT length too large : %6d",actual[*nFact]);
        return -1;
    }
    remain[0]=*nPoints;
    sofar[1]=1;
    remain[1]=*nPoints / actual[1];
    for (i=2; i<=*nFact; i++)
    {
        sofar[i]=sofar[i-1]*actual[i-1];
        remain[i]=remain[i-1] / actual[i];
    }

    return 0;
}   /* transTableSetup */
