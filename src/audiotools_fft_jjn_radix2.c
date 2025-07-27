#include "audiotools_fft_jjn.h"

void FFT_JJN_radix2 ( FLOAT32 *aRe, FLOAT32 *aIm )
{
    FLOAT32 t1;

    t1=aRe[0] + aRe[1];

    aRe[1]=aRe[0] -  aRe[1];
    aRe[0]=t1;

    t1=aIm[0] + aIm[1];
    aIm[1]=aIm[0] - aIm[1];
    aIm[0]=t1;
}

void FFT_JJN_radix2_OutInterleaved ( FLOAT32 IN *aRe, FLOAT32 IN *aIm, FLOAT32 OUT *zRe, FLOAT32 OUT *zIm, long OIl )
{
    zRe[  0] = aRe[0] + aRe[1];
    zRe[OIl] = aRe[0] - aRe[1];

    zIm[  0] = aIm[0] + aIm[1];
    zIm[OIl] = aIm[0] - aIm[1];
}

void FFT_JJN_radix2_OutInterleavedFast ( FLOAT32 IN *aReIm, FLOAT32 OUT *zRe, FLOAT32 OUT *zIm, long OIl )
{
    zRe[  0] = aReIm[0] + aReIm[2];
    zRe[OIl] = aReIm[0] - aReIm[2];

    zIm[  0] = aReIm[1] + aReIm[3];
    zIm[OIl] = aReIm[1] - aReIm[3];
}
