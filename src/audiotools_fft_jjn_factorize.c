#include "audiotools_fft_jjn.h"


void FFT_JJN_factorize ( long n, long *nFact, long fact[] )
{
    long i,j,k;
    long nRadix;
    long radices[8];
    long factors[maxFactorCount];

    nRadix    =  7;  
    radices[1]=  2;
    radices[2]=  3;
    radices[3]=  4;
    radices[4]=  5;
    radices[5]=  7;
    radices[6]=  8;
    radices[7]= 10;

    if (n==1)
    {
        j=1;
        factors[1]=1;
    }
    else j=0;

    i=nRadix;
    while ((n>1) && (i>0))
    {
        if ((n % radices[i]) == 0)
	    {
	        n=n / radices[i];
	        j=j+1;
	        factors[j]=radices[i];
	    }
        else  i=i-1;
    }

    if (factors[j] == 2)   /*substitute factors 2*8 with 4*4 */
    {   
        i = j-1;
        while ((i>0) && (factors[i] != 8)) i--;
        if (i>0)
	    {
	        factors[j] = 4;
	        factors[i] = 4;
	    }
    }
    if (n>1)
    {
        for (k=2; k<sqrt(n)+1; k++)
	        while ((n % k) == 0)
	        {
	            n=n / k;
	            j=j+1;
	            factors[j]=k;
	        }
        if (n>1)
        {
	        j=j+1;
	        factors[j]=n;
        }
    }               
    for (i=1; i<=j; i++)         
    {
        fact[i] = factors[j-i+1];
    }
    *nFact=j;
}   /* factorize */
