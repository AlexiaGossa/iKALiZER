#include "audiotools_fft_jjn.h"

// radix-10 uses 2 x radix-5 sub-routine
void FFT_JJN_radix5 ( FLOAT32 *aRe, FLOAT32 *aIm );


void FFT_JJN_radix10 ( FLOAT32 *aRe, FLOAT32 *aIm )
{
    FLOAT32  xRe[5], xIm[5], yRe[5], yIm[5];

    xRe[0] = aRe[0];            yRe[0] = aRe[5];
    xRe[1] = aRe[2];            yRe[1] = aRe[7];
    xRe[2] = aRe[4];            yRe[2] = aRe[9];
    xRe[3] = aRe[6];            yRe[3] = aRe[1];
    xRe[4] = aRe[8];            yRe[4] = aRe[3];

    xIm[0] = aIm[0];            yIm[0] = aIm[5];
    xIm[1] = aIm[2];            yIm[1] = aIm[7];
    xIm[2] = aIm[4];            yIm[2] = aIm[9];
    xIm[3] = aIm[6];            yIm[3] = aIm[1];
    xIm[4] = aIm[8];            yIm[4] = aIm[3];

    FFT_JJN_radix5 ( xRe, xIm );
    FFT_JJN_radix5 ( yRe, yIm );
    //FFT_JJN_fft_5 ( xRe, xIm, piData );
    //FFT_JJN_fft_5 ( yRe, yIm, piData );

    aRe[0] = xRe[0] + yRe[0];   aRe[5] = xRe[0] - yRe[0];
    aRe[6] = xRe[1] + yRe[1];   aRe[1] = xRe[1] - yRe[1];
    aRe[2] = xRe[2] + yRe[2];   aRe[7] = xRe[2] - yRe[2];
    aRe[8] = xRe[3] + yRe[3];   aRe[3] = xRe[3] - yRe[3];
    aRe[4] = xRe[4] + yRe[4];   aRe[9] = xRe[4] - yRe[4];
                                        
    aIm[0] = xIm[0] + yIm[0];   aIm[5] = xIm[0] - yIm[0];
    aIm[6] = xIm[1] + yIm[1];   aIm[1] = xIm[1] - yIm[1];
    aIm[2] = xIm[2] + yIm[2];   aIm[7] = xIm[2] - yIm[2];
    aIm[8] = xIm[3] + yIm[3];   aIm[3] = xIm[3] - yIm[3];
    aIm[4] = xIm[4] + yIm[4];   aIm[9] = xIm[4] - yIm[4];
}

void FFT_JJN_radix10_OutInterleaved ( FLOAT32 IN *aRe, FLOAT32 IN *aIm, FLOAT32 OUT *zRe, FLOAT32 OUT *zIm, long OIl )
{
    FLOAT32  xRe[5], xIm[5], yRe[5], yIm[5];
    long    lOIl2, lOIl3, lOIl4, lOIl5, lOIl6, lOIl7, lOIl8, lOIl9;

    lOIl2 = OIl<<1;
    lOIl3 = lOIl2 + OIl;
    lOIl4 = OIl<<2;
    lOIl5 = lOIl4 + OIl;
    lOIl6 = lOIl2 + lOIl4;
    lOIl7 = lOIl6 + OIl;
    lOIl8 = OIl<<3;
    lOIl9 = lOIl8 + OIl;

    xRe[0] = aRe[0];                        yRe[0] = aRe[5];
    xRe[1] = aRe[2];                        yRe[1] = aRe[7];
    xRe[2] = aRe[4];                        yRe[2] = aRe[9];
    xRe[3] = aRe[6];                        yRe[3] = aRe[1];
    xRe[4] = aRe[8];                        yRe[4] = aRe[3];

    xIm[0] = aIm[0];                        yIm[0] = aIm[5];
    xIm[1] = aIm[2];                        yIm[1] = aIm[7];
    xIm[2] = aIm[4];                        yIm[2] = aIm[9];
    xIm[3] = aIm[6];                        yIm[3] = aIm[1];
    xIm[4] = aIm[8];                        yIm[4] = aIm[3];

    FFT_JJN_radix5 ( xRe, xIm );
    FFT_JJN_radix5 ( yRe, yIm );
    //FFT_JJN_fft_5 ( xRe, xIm, piData );
    //FFT_JJN_fft_5 ( yRe, yIm, piData );

    zRe[0    ] = xRe[0] + yRe[0];           zIm[0    ] = xIm[0] + yIm[0];
    zRe[OIl  ] = xRe[1] - yRe[1];           zIm[OIl  ] = xIm[1] - yIm[1];
    zRe[lOIl2] = xRe[2] + yRe[2];           zIm[lOIl2] = xIm[2] + yIm[2];
    zRe[lOIl3] = xRe[3] - yRe[3];           zIm[lOIl3] = xIm[3] - yIm[3];
    zRe[lOIl4] = xRe[4] + yRe[4];           zIm[lOIl4] = xIm[4] + yIm[4];
    zRe[lOIl5] = xRe[0] - yRe[0];           zIm[lOIl5] = xIm[0] - yIm[0];
    zRe[lOIl6] = xRe[1] + yRe[1];           zIm[lOIl6] = xIm[1] + yIm[1];
    zRe[lOIl7] = xRe[2] - yRe[2];           zIm[lOIl7] = xIm[2] - yIm[2];
    zRe[lOIl8] = xRe[3] + yRe[3];           zIm[lOIl8] = xIm[3] + yIm[3];
    zRe[lOIl9] = xRe[4] - yRe[4];           zIm[lOIl9] = xIm[4] - yIm[4];
}

void FFT_JJN_radix10_OutInterleavedFast ( FLOAT32 IN *aReIm, FLOAT32 OUT *zRe, FLOAT32 OUT *zIm, long OIl )
{
    FLOAT32  xRe[5], xIm[5], yRe[5], yIm[5];
    long    lOIl2, lOIl3, lOIl4, lOIl5, lOIl6, lOIl7, lOIl8, lOIl9;

    lOIl2 = OIl<<1;
    lOIl3 = lOIl2 + OIl;
    lOIl4 = OIl<<2;
    lOIl5 = lOIl4 + OIl;
    lOIl6 = lOIl2 + lOIl4;
    lOIl7 = lOIl6 + OIl;
    lOIl8 = OIl<<3;
    lOIl9 = lOIl8 + OIl;

    xRe[0] = aReIm[0];                        yRe[0] = aReIm[10];
    xRe[1] = aReIm[4];                        yRe[1] = aReIm[14];
    xRe[2] = aReIm[8];                        yRe[2] = aReIm[18];
    xRe[3] = aReIm[12];                       yRe[3] = aReIm[2];
    xRe[4] = aReIm[16];                       yRe[4] = aReIm[6];

    xIm[0] = aReIm[1];                        yIm[0] = aReIm[11];
    xIm[1] = aReIm[5];                        yIm[1] = aReIm[15];
    xIm[2] = aReIm[9];                        yIm[2] = aReIm[19];
    xIm[3] = aReIm[13];                       yIm[3] = aReIm[3];
    xIm[4] = aReIm[17];                       yIm[4] = aReIm[7];

    FFT_JJN_radix5 ( xRe, xIm );
    FFT_JJN_radix5 ( yRe, yIm );
    //FFT_JJN_fft_5 ( xRe, xIm, piData );
    //FFT_JJN_fft_5 ( yRe, yIm, piData );

    zRe[0    ] = xRe[0] + yRe[0];           zIm[0    ] = xIm[0] + yIm[0];
    zRe[OIl  ] = xRe[1] - yRe[1];           zIm[OIl  ] = xIm[1] - yIm[1];
    zRe[lOIl2] = xRe[2] + yRe[2];           zIm[lOIl2] = xIm[2] + yIm[2];
    zRe[lOIl3] = xRe[3] - yRe[3];           zIm[lOIl3] = xIm[3] - yIm[3];
    zRe[lOIl4] = xRe[4] + yRe[4];           zIm[lOIl4] = xIm[4] + yIm[4];
    zRe[lOIl5] = xRe[0] - yRe[0];           zIm[lOIl5] = xIm[0] - yIm[0];
    zRe[lOIl6] = xRe[1] + yRe[1];           zIm[lOIl6] = xIm[1] + yIm[1];
    zRe[lOIl7] = xRe[2] - yRe[2];           zIm[lOIl7] = xIm[2] - yIm[2];
    zRe[lOIl8] = xRe[3] + yRe[3];           zIm[lOIl8] = xIm[3] + yIm[3];
    zRe[lOIl9] = xRe[4] - yRe[4];           zIm[lOIl9] = xIm[4] - yIm[4];
}

/*
void FFT_JJN_fft_10(_FFT_JJN_DATA *piData)
{
  FLOAT32  aRe[5], aIm[5], bRe[5], bIm[5];

  aRe[0] = zRe[0];    bRe[0] = zRe[5];
  aRe[1] = zRe[2];    bRe[1] = zRe[7];
  aRe[2] = zRe[4];    bRe[2] = zRe[9];
  aRe[3] = zRe[6];    bRe[3] = zRe[1];
  aRe[4] = zRe[8];    bRe[4] = zRe[3];

  aIm[0] = zIm[0];    bIm[0] = zIm[5];
  aIm[1] = zIm[2];    bIm[1] = zIm[7];
  aIm[2] = zIm[4];    bIm[2] = zIm[9];
  aIm[3] = zIm[6];    bIm[3] = zIm[1];
  aIm[4] = zIm[8];    bIm[4] = zIm[3];

  FFT_JJN_fft_5(aRe, aIm, piData);
  FFT_JJN_fft_5(bRe, bIm, piData);

  zRe[0] = aRe[0] + bRe[0]; zRe[5] = aRe[0] - bRe[0];
  zRe[6] = aRe[1] + bRe[1]; zRe[1] = aRe[1] - bRe[1];
  zRe[2] = aRe[2] + bRe[2]; zRe[7] = aRe[2] - bRe[2];
  zRe[8] = aRe[3] + bRe[3]; zRe[3] = aRe[3] - bRe[3];
  zRe[4] = aRe[4] + bRe[4]; zRe[9] = aRe[4] - bRe[4];

  zIm[0] = aIm[0] + bIm[0]; zIm[5] = aIm[0] - bIm[0];
  zIm[6] = aIm[1] + bIm[1]; zIm[1] = aIm[1] - bIm[1];
  zIm[2] = aIm[2] + bIm[2]; zIm[7] = aIm[2] - bIm[2];
  zIm[8] = aIm[3] + bIm[3]; zIm[3] = aIm[3] - bIm[3];
  zIm[4] = aIm[4] + bIm[4]; zIm[9] = aIm[4] - bIm[4];
}*/

