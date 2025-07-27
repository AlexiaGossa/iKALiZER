#include "audiotools_fft_jjn.h"

static FLOAT32  c3_1 = -1.5000000000000E+00F;  /*  c3_1 = cos(2*pi/3)-1;          */
static FLOAT32  c3_2 =  8.6602540378444E-01F;  /*  c3_2 = sin(2*pi/3);            */

void FFT_JJN_radix3 ( FLOAT32 *aRe, FLOAT32 *aIm )
{
    FLOAT32 t1_re, t1_im;
    FLOAT32 m1_re, m1_im;
    FLOAT32 m2_re, m2_im;
    FLOAT32 s1_re, s1_im;

    t1_re=aRe[1] + aRe[2];              t1_im=aIm[1] + aIm[2];
    aRe[0]=aRe[0] + t1_re;              aIm[0]=aIm[0] + t1_im;
    m1_re=c3_1*t1_re;                   m1_im=c3_1*t1_im;
    m2_re=c3_2*(aIm[1] - aIm[2]);       m2_im=c3_2*(aRe[2] -  aRe[1]);
    s1_re=aRe[0] + m1_re;               s1_im=aIm[0] + m1_im;
    aRe[1]=s1_re + m2_re;               aIm[1]=s1_im + m2_im;
    aRe[2]=s1_re - m2_re;               aIm[2]=s1_im - m2_im;
}


void FFT_JJN_radix3_OutInterleaved ( FLOAT32 IN *aRe, FLOAT32 IN *aIm, FLOAT32 OUT *zRe, FLOAT32 OUT *zIm, long OIl )
{
    FLOAT32 t1_re, t1_im;
    FLOAT32 m1_re, m1_im;
    FLOAT32 m2_re, m2_im;
    FLOAT32 s1_re, s1_im;
    long lOIl2;

    lOIl2 = OIl<<1;

    t1_re       = aRe[1] + aRe[2];              t1_im       = aIm[1] + aIm[2];
    zRe[0]      = aRe[0] + t1_re;               zIm[0]      = aIm[0] + t1_im;
    m1_re       = c3_1*t1_re;                   m1_im       = c3_1*t1_im;
    m2_re       = c3_2*(aIm[1] - aIm[2]);       m2_im       = c3_2*(aRe[2] -  aRe[1]);
    s1_re       = zRe[0] + m1_re;               s1_im       = zIm[0] + m1_im;
    zRe[OIl]    = s1_re + m2_re;                zIm[OIl]    = s1_im + m2_im;
    zRe[lOIl2]  = s1_re - m2_re;                zIm[lOIl2]  = s1_im - m2_im;
}

void FFT_JJN_radix3_OutInterleavedFast ( FLOAT32 IN *aReIm, FLOAT32 OUT *zRe, FLOAT32 OUT *zIm, long OIl )
{
    FLOAT32 t1_re, t1_im;
    FLOAT32 m1_re, m1_im;
    FLOAT32 m2_re, m2_im;
    FLOAT32 s1_re, s1_im;
    long lOIl2;

    lOIl2 = OIl<<1;

    t1_re       = aReIm[2] + aReIm[4];          t1_im       = aReIm[3] + aReIm[5];
    zRe[0]      = aReIm[0] + t1_re;             zIm[0]      = aReIm[1] + t1_im;
    m1_re       = c3_1*t1_re;                   m1_im       = c3_1*t1_im;
    m2_re       = c3_2*(aReIm[3] - aReIm[5]);   m2_im       = c3_2*(aReIm[4] -  aReIm[2]);
    s1_re       = zRe[0] + m1_re;               s1_im       = zIm[0] + m1_im;
    zRe[OIl]    = s1_re + m2_re;                zIm[OIl]    = s1_im + m2_im;
    zRe[lOIl2]  = s1_re - m2_re;                zIm[lOIl2]  = s1_im - m2_im;
}
