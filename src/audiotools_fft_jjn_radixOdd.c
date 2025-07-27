#include "audiotools_fft_jjn.h"

void FFT_JJN_radixOdd ( long radix, 
                       FLOAT32 * zRe, FLOAT32 * zIm, 
                       FLOAT32 * vRe, FLOAT32 * vIm, 
                       FLOAT32 * wRe, FLOAT32 * wIm,
                       FLOAT32 * trigRe, FLOAT32 * trigIm )
{
    FLOAT32  rere, reim, imre, imim;
    long     i,j,k,n,max;

    n = radix;
    max = (n + 1)/2;
    for (j=1; j < max; j++)
    {
        vRe[j] = zRe[j] + zRe[n-j];
        vIm[j] = zIm[j] - zIm[n-j];
        wRe[j] = zRe[j] - zRe[n-j];
        wIm[j] = zIm[j] + zIm[n-j];
    }

    for (j=1; j < max; j++)
    {
        zRe[j]=zRe[0]; 
        zIm[j]=zIm[0];
        zRe[n-j]=zRe[0]; 
        zIm[n-j]=zIm[0];
        k=j;
        for (i=1; i < max; i++)
        {
	        rere = trigRe[k] * vRe[i];
	        imim = trigIm[k] * vIm[i];
	        reim = trigRe[k] * wIm[i];
	        imre = trigIm[k] * wRe[i];
            
	        zRe[n-j] += rere + imim;
	        zIm[n-j] += reim - imre;
	        zRe[j]   += rere - imim;
	        zIm[j]   += reim + imre;

	        k = k + j;
	        if (k >= n)  k = k - n;
        }
    }

    for (j=1; j < max; j++)
    {
      zRe[0]=zRe[0] + vRe[j]; 
      zIm[0]=zIm[0] + wIm[j];
    }
}

void FFT_JJN_radixOdd_OutInterleavedFast ( FLOAT32 IN *aReIm, FLOAT32 OUT *zRe, FLOAT32 OUT *zIm, long OIl,
                                           long radix, 
                                           FLOAT32 * vRe, FLOAT32 * vIm, 
                                           FLOAT32 * wRe, FLOAT32 * wIm,
                                           FLOAT32 * trigRe, FLOAT32 * trigIm )
/*void FFT_JJN_radixOdd ( long radix, 
                       FLOAT32 * zRe, FLOAT32 * zIm, 
                       FLOAT32 * vRe, FLOAT32 * vIm, 
                       FLOAT32 * wRe, FLOAT32 * wIm,
                       FLOAT32 * trigRe, FLOAT32 * trigIm )*/
{
    FLOAT32  rere, reim, imre, imim;
    long    i,j,k,n,max;
    long    pru, piu;   //pointer to real up / pointer to imaginary up
    long    prd, pid;   //pointer to real down / pointer to imaginary down
    long    ou, od;     //offset up / offset down

    n = radix;
    max = (n + 1)/2;

    pru = 2;
    piu = pru+1;
    prd = (n-1)<<1;
    pid = prd+1;
    for (j=1; j < max; j++)
    {
        vRe[j] = aReIm[pru] + aReIm[prd];
        vIm[j] = aReIm[piu] - aReIm[pid];
        wRe[j] = aReIm[pru] - aReIm[prd];
        wIm[j] = aReIm[piu] + aReIm[pid];

        pru += 2;
        piu += 2;
        prd -= 2;
        pid -= 2;

    }

    zRe[0] = aReIm[0];
    zIm[0] = aReIm[1];

    ou = OIl;
    od = (n-1)*OIl;

    for (j=1; j < max; j++)
    {
        zRe[ou]=aReIm[0]; 
        zIm[ou]=aReIm[1];
        zRe[od]=aReIm[0]; 
        zIm[od]=aReIm[1];
        k=j;
        for (i=1; i < max; i++)
        {
	        rere = trigRe[k] * vRe[i];
	        imim = trigIm[k] * vIm[i];
	        reim = trigRe[k] * wIm[i];
	        imre = trigIm[k] * wRe[i];
            
	        zRe[od] += rere + imim;
	        zIm[od] += reim - imre;
	        zRe[ou] += rere - imim;
	        zIm[ou] += reim + imre;

	        k = k + j;
	        if (k >= n)  k = k - n;
        }

        ou += OIl;
        od -= OIl;
    }


    for (j=1; j < max; j++)
    {
        zRe[0]=zRe[0] + vRe[j]; 
        zIm[0]=zIm[0] + wIm[j];
    }
}


/*
void FFT_JJN_fft_odd(long radix,_FFT_JJN_DATA *piData)
{
    FLOAT32  rere, reim, imre, imim;
    long     i,j,k,n,max;

    n = radix;
    max = (n + 1)/2;
    for (j=1; j < max; j++)
    {
        vRe[j] = zRe[j] + zRe[n-j];
        vIm[j] = zIm[j] - zIm[n-j];
        wRe[j] = zRe[j] - zRe[n-j];
        wIm[j] = zIm[j] + zIm[n-j];
    }

    for (j=1; j < max; j++)
    {
        zRe[j]=zRe[0]; 
        zIm[j]=zIm[0];
        zRe[n-j]=zRe[0]; 
        zIm[n-j]=zIm[0];
        k=j;
        for (i=1; i < max; i++)
        {
	        rere = trigRe[k] * vRe[i];
	        imim = trigIm[k] * vIm[i];
	        reim = trigRe[k] * wIm[i];
	        imre = trigIm[k] * wRe[i];
            
	        zRe[n-j] += rere + imim;
	        zIm[n-j] += reim - imre;
	        zRe[j]   += rere - imim;
	        zIm[j]   += reim + imre;

	        k = k + j;
	        if (k >= n)  k = k - n;
        }
    }

    for (j=1; j < max; j++)
    {
      zRe[0]=zRe[0] + vRe[j]; 
      zIm[0]=zIm[0] + wIm[j];
    }
}
*/
