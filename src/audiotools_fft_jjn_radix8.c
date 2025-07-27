#include "audiotools_fft_jjn.h"

// radix-10 uses 2 x radix-5 sub-routine
void FFT_JJN_radix4 ( FLOAT32 *aRe, FLOAT32 *aIm );

static FLOAT32  c8_rsqrt2   =  0.70710678118655F;  /*  c8 = 1/sqrt(2);    */


void FFT_JJN_radix8 ( FLOAT32 *aRe, FLOAT32 *aIm )
{
    FLOAT32  xRe[4], xIm[4], yRe[4], yIm[4], gem;

    xRe[0] = aRe[0];    yRe[0] = aRe[1];
    xRe[1] = aRe[2];    yRe[1] = aRe[3];
    xRe[2] = aRe[4];    yRe[2] = aRe[5];
    xRe[3] = aRe[6];    yRe[3] = aRe[7];

    xIm[0] = aIm[0];    yIm[0] = aIm[1];
    xIm[1] = aIm[2];    yIm[1] = aIm[3];
    xIm[2] = aIm[4];    yIm[2] = aIm[5];
    xIm[3] = aIm[6];    yIm[3] = aIm[7];

    FFT_JJN_radix4 ( xRe, xIm ); 
    FFT_JJN_radix4 ( yRe, yIm );

    gem    = c8_rsqrt2*(yRe[1] + yIm[1]);
    yIm[1] = c8_rsqrt2*(yIm[1] - yRe[1]);
    yRe[1] = gem;
    gem    = yIm[2];
    yIm[2] =-yRe[2];
    yRe[2] = gem;
    gem    = c8_rsqrt2*(yIm[3] - yRe[3]);
    yIm[3] =-c8_rsqrt2*(yRe[3] + yIm[3]);
    yRe[3] = gem;

    aRe[0] = xRe[0] + yRe[0]; aRe[4] = xRe[0] - yRe[0];
    aRe[1] = xRe[1] + yRe[1]; aRe[5] = xRe[1] - yRe[1];
    aRe[2] = xRe[2] + yRe[2]; aRe[6] = xRe[2] - yRe[2];
    aRe[3] = xRe[3] + yRe[3]; aRe[7] = xRe[3] - yRe[3];

    aIm[0] = xIm[0] + yIm[0]; aIm[4] = xIm[0] - yIm[0];
    aIm[1] = xIm[1] + yIm[1]; aIm[5] = xIm[1] - yIm[1];
    aIm[2] = xIm[2] + yIm[2]; aIm[6] = xIm[2] - yIm[2];
    aIm[3] = xIm[3] + yIm[3]; aIm[7] = xIm[3] - yIm[3];
}

void FFT_JJN_radix8_OutInterleaved ( FLOAT32 IN *aRe, FLOAT32 IN *aIm, FLOAT32 OUT *zRe, FLOAT32 OUT *zIm, long OIl )
{
    FLOAT32  xRe[4], xIm[4], yRe[4], yIm[4], gem;
    long lOIl2, lOIl3, lOIl4, lOIl5, lOIl6, lOIl7;

    lOIl2 = OIl<<1;
    lOIl3 = lOIl2 + OIl;
    lOIl4 = OIl<<2;
    lOIl5 = lOIl4 + OIl;
    lOIl6 = lOIl2 + lOIl4;
    lOIl7 = lOIl6 + OIl;


    xRe[0] = aRe[0];    yRe[0] = aRe[1];
    xRe[1] = aRe[2];    yRe[1] = aRe[3];
    xRe[2] = aRe[4];    yRe[2] = aRe[5];
    xRe[3] = aRe[6];    yRe[3] = aRe[7];

    xIm[0] = aIm[0];    yIm[0] = aIm[1];
    xIm[1] = aIm[2];    yIm[1] = aIm[3];
    xIm[2] = aIm[4];    yIm[2] = aIm[5];
    xIm[3] = aIm[6];    yIm[3] = aIm[7];

    FFT_JJN_radix4 ( xRe, xIm ); 
    FFT_JJN_radix4 ( yRe, yIm );

    gem    = c8_rsqrt2*(yRe[1] + yIm[1]);
    yIm[1] = c8_rsqrt2*(yIm[1] - yRe[1]);
    yRe[1] = gem;
    gem    = yIm[2];
    yIm[2] =-yRe[2];
    yRe[2] = gem;
    gem    = c8_rsqrt2*(yIm[3] - yRe[3]);
    yIm[3] =-c8_rsqrt2*(yRe[3] + yIm[3]);
    yRe[3] = gem;

    zRe[0    ] = xRe[0] + yRe[0];   zIm[0    ] = xIm[0] + yIm[0];
    zRe[OIl  ] = xRe[1] + yRe[1];   zIm[OIl  ] = xIm[1] + yIm[1];
    zRe[lOIl2] = xRe[2] + yRe[2];   zIm[lOIl2] = xIm[2] + yIm[2];
    zRe[lOIl3] = xRe[3] + yRe[3];   zIm[lOIl3] = xIm[3] + yIm[3];
    zRe[lOIl4] = xRe[0] - yRe[0];   zIm[lOIl4] = xIm[0] - yIm[0];
    zRe[lOIl5] = xRe[1] - yRe[1];   zIm[lOIl5] = xIm[1] - yIm[1];
    zRe[lOIl6] = xRe[2] - yRe[2];   zIm[lOIl6] = xIm[2] - yIm[2];
    zRe[lOIl7] = xRe[3] - yRe[3];   zIm[lOIl7] = xIm[3] - yIm[3];
}

void FFT_JJN_radix8_OutInterleavedFast ( FLOAT32 IN *aReIm, FLOAT32 OUT *zRe, FLOAT32 OUT *zIm, long OIl )
{
    FLOAT32  xRe[4], xIm[4], yRe[4], yIm[4], gem;
    long lOIl2, lOIl3, lOIl4, lOIl5, lOIl6, lOIl7;

    lOIl2 = OIl<<1;
    lOIl3 = lOIl2 + OIl;
    lOIl4 = OIl<<2;
    lOIl5 = lOIl4 + OIl;
    lOIl6 = lOIl2 + lOIl4;
    lOIl7 = lOIl6 + OIl;


    xRe[0] = aReIm[0];    yRe[0] = aReIm[2];
    xRe[1] = aReIm[4];    yRe[1] = aReIm[6];
    xRe[2] = aReIm[8];    yRe[2] = aReIm[10];
    xRe[3] = aReIm[12];   yRe[3] = aReIm[14];

    xIm[0] = aReIm[1];    yIm[0] = aReIm[3];
    xIm[1] = aReIm[5];    yIm[1] = aReIm[7];
    xIm[2] = aReIm[9];    yIm[2] = aReIm[11];
    xIm[3] = aReIm[13];   yIm[3] = aReIm[15];

    FFT_JJN_radix4 ( xRe, xIm ); 
    FFT_JJN_radix4 ( yRe, yIm );

    gem    = c8_rsqrt2*(yRe[1] + yIm[1]);
    yIm[1] = c8_rsqrt2*(yIm[1] - yRe[1]);
    yRe[1] = gem;
    gem    = yIm[2];
    yIm[2] =-yRe[2];
    yRe[2] = gem;
    gem    = c8_rsqrt2*(yIm[3] - yRe[3]);
    yIm[3] =-c8_rsqrt2*(yRe[3] + yIm[3]);
    yRe[3] = gem;

    zRe[0    ] = xRe[0] + yRe[0];   zIm[0    ] = xIm[0] + yIm[0];
    zRe[OIl  ] = xRe[1] + yRe[1];   zIm[OIl  ] = xIm[1] + yIm[1];
    zRe[lOIl2] = xRe[2] + yRe[2];   zIm[lOIl2] = xIm[2] + yIm[2];
    zRe[lOIl3] = xRe[3] + yRe[3];   zIm[lOIl3] = xIm[3] + yIm[3];
    zRe[lOIl4] = xRe[0] - yRe[0];   zIm[lOIl4] = xIm[0] - yIm[0];
    zRe[lOIl5] = xRe[1] - yRe[1];   zIm[lOIl5] = xIm[1] - yIm[1];
    zRe[lOIl6] = xRe[2] - yRe[2];   zIm[lOIl6] = xIm[2] - yIm[2];
    zRe[lOIl7] = xRe[3] - yRe[3];   zIm[lOIl7] = xIm[3] - yIm[3];
}


/*
void FFT_JJN_fft_8(_FFT_JJN_DATA *piData)
{
  FLOAT32  aRe[4], aIm[4], bRe[4], bIm[4], gem;

  aRe[0] = zRe[0];    bRe[0] = zRe[1];
  aRe[1] = zRe[2];    bRe[1] = zRe[3];
  aRe[2] = zRe[4];    bRe[2] = zRe[5];
  aRe[3] = zRe[6];    bRe[3] = zRe[7];

  aIm[0] = zIm[0];    bIm[0] = zIm[1];
  aIm[1] = zIm[2];    bIm[1] = zIm[3];
  aIm[2] = zIm[4];    bIm[2] = zIm[5];
  aIm[3] = zIm[6];    bIm[3] = zIm[7];

  FFT_JJN_fft_4(aRe, aIm, piData); 
  FFT_JJN_fft_4(bRe, bIm, piData);

  gem    = c8*(bRe[1] + bIm[1]);
  bIm[1] = c8*(bIm[1] - bRe[1]);
  bRe[1] = gem;
  gem    = bIm[2];
  bIm[2] =-bRe[2];
  bRe[2] = gem;
  gem    = c8*(bIm[3] - bRe[3]);
  bIm[3] =-c8*(bRe[3] + bIm[3]);
  bRe[3] = gem;
    
  zRe[0] = aRe[0] + bRe[0]; zRe[4] = aRe[0] - bRe[0];
  zRe[1] = aRe[1] + bRe[1]; zRe[5] = aRe[1] - bRe[1];
  zRe[2] = aRe[2] + bRe[2]; zRe[6] = aRe[2] - bRe[2];
  zRe[3] = aRe[3] + bRe[3]; zRe[7] = aRe[3] - bRe[3];

  zIm[0] = aIm[0] + bIm[0]; zIm[4] = aIm[0] - bIm[0];
  zIm[1] = aIm[1] + bIm[1]; zIm[5] = aIm[1] - bIm[1];
  zIm[2] = aIm[2] + bIm[2]; zIm[6] = aIm[2] - bIm[2];
  zIm[3] = aIm[3] + bIm[3]; zIm[7] = aIm[3] - bIm[3];
}
*/
