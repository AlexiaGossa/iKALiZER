#include "audiotools_fft_jjn.h"

void FFT_JJN_radix4 ( FLOAT32 *aRe, FLOAT32 *aIm )
{
  FLOAT32  t1_re,t1_im, t2_re,t2_im;
  FLOAT32  m2_re,m2_im, m3_re,m3_im;

  t1_re=aRe[0] + aRe[2]; t1_im=aIm[0] + aIm[2];
  t2_re=aRe[1] + aRe[3]; t2_im=aIm[1] + aIm[3];

  m2_re=aRe[0] - aRe[2]; m2_im=aIm[0] - aIm[2];
  m3_re=aIm[1] - aIm[3]; m3_im=aRe[3] - aRe[1];

  aRe[0]=t1_re + t2_re; aIm[0]=t1_im + t2_im;
  aRe[2]=t1_re - t2_re; aIm[2]=t1_im - t2_im;
  aRe[1]=m2_re + m3_re; aIm[1]=m2_im + m3_im;
  aRe[3]=m2_re - m3_re; aIm[3]=m2_im - m3_im;
}   /* fft_4 */

void FFT_JJN_radix4_OutInterleaved ( FLOAT32 IN *aRe, FLOAT32 IN *aIm, FLOAT32 OUT *zRe, FLOAT32 OUT *zIm, long OIl )
{
  FLOAT32  t1_re,t1_im, t2_re,t2_im;
  FLOAT32  m2_re,m2_im, m3_re,m3_im;
  long lOIl2, lOIl3;

  lOIl2 = OIl<<1;
  lOIl3 = lOIl2 + OIl;

  t1_re=aRe[0] + aRe[2]; t1_im=aIm[0] + aIm[2];
  t2_re=aRe[1] + aRe[3]; t2_im=aIm[1] + aIm[3];

  m2_re=aRe[0] - aRe[2]; m2_im=aIm[0] - aIm[2];
  m3_re=aIm[1] - aIm[3]; m3_im=aRe[3] - aRe[1];

  zRe[0]     =t1_re + t2_re;     zIm[0]     =t1_im + t2_im;
  zRe[lOIl2] =t1_re - t2_re;     zIm[lOIl2] =t1_im - t2_im;
  zRe[OIl]   =m2_re + m3_re;     zIm[OIl]   =m2_im + m3_im;
  zRe[lOIl3] =m2_re - m3_re;     zIm[lOIl3] =m2_im - m3_im;
}

void FFT_JJN_radix4_OutInterleavedFast ( FLOAT32 IN *aReIm, FLOAT32 OUT *zRe, FLOAT32 OUT *zIm, long OIl )
{
  FLOAT32  t1_re,t1_im, t2_re,t2_im;
  FLOAT32  m2_re,m2_im, m3_re,m3_im;
  long lOIl2, lOIl3;

  lOIl2 = OIl<<1;
  lOIl3 = lOIl2 + OIl;

  t1_re=aReIm[0] + aReIm[4];    t1_im=aReIm[1] + aReIm[5];
  t2_re=aReIm[2] + aReIm[6];    t2_im=aReIm[3] + aReIm[7];

  m2_re=aReIm[0] - aReIm[4];    m2_im=aReIm[1] - aReIm[5];
  m3_re=aReIm[3] - aReIm[7];    m3_im=aReIm[6] - aReIm[2];

  zRe[0]     =t1_re + t2_re;    zIm[0]     =t1_im + t2_im;
  zRe[lOIl2] =t1_re - t2_re;    zIm[lOIl2] =t1_im - t2_im;
  zRe[OIl]   =m2_re + m3_re;    zIm[OIl]   =m2_im + m3_im;
  zRe[lOIl3] =m2_re - m3_re;    zIm[lOIl3] =m2_im - m3_im;
}

