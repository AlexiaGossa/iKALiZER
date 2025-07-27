#include "audiotools_fft_jjn.h"

/****************************************************************************
  The sequence y is the permuted input sequence x so that the following
  transformations can be performed in-place, and the final result is the
  normal order.
 ****************************************************************************/

void FFT_JJN_permute(long nPoint, long nFact,
             long fact[], long remain[],
             FLOAT32 xRe[], FLOAT32 xIm[],
             FLOAT32 yRe[], FLOAT32 yIm[] )

{
    long i,j,k;
    long count[maxFactorCount]; 

    for (i=1; i<=nFact; i++) count[i]=0;
    k=0;
    for (i=0; i<=nPoint-2; i++)
    {
        yRe[i] = xRe[k];
        yIm[i] = xIm[k];
        j=1;
        k=k+remain[j];
        count[1] = count[1]+1;
        while (count[j] >= fact[j])
        {
	        count[j]=0;
	        k=k-remain[j-1]+remain[j+1];
	        j=j+1;
	        count[j]=count[j]+1;
        }
    }
    yRe[nPoint-1]=xRe[nPoint-1];
    yIm[nPoint-1]=xIm[nPoint-1];

}   /* permute */

/*
void FFT_JJN_PreparePermute( long nPoint, long nFact,
             long fact[], long remain[],
             long nSourceList[],
             _FFT_JJN_DATA *piData )

{
    long i,j,k;
    long count[maxFactorCount]; 

    for (i=1; i<=nFact; i++) count[i]=0;
    k=0;
    for (i=0; i<=nPoint-2; i++)
    {
        yRe[i] = xRe[k];
        yIm[i] = xIm[k];
        j=1;
        k=k+remain[j];
        count[1] = count[1]+1;
        while (count[j] >= fact[j])
        {
	        count[j]=0;
	        k=k-remain[j-1]+remain[j+1];
	        j=j+1;
	        count[j]=count[j]+1;
        }
    }
    yRe[nPoint-1]=xRe[nPoint-1];
    yIm[nPoint-1]=xIm[nPoint-1];

}
*/
