
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/************************************************************************
  fft(int n, FLOAT32 xRe[], FLOAT32 xIm[], FLOAT32 yRe[], FLOAT32 yIm[])
  ------------------------------------------------------------------------
  NOTE : This is copyrighted material, Not public domain. See below.
  ------------------------------------------------------------------------
  Input/output:
  int n          transformation length.
  FLOAT32 xRe[]   real part of input sequence.
  FLOAT32 xIm[]   imaginary part of input sequence.
  FLOAT32 yRe[]   real part of output sequence.
  FLOAT32 yIm[]   imaginary part of output sequence.
  ------------------------------------------------------------------------
  Function:
  The procedure performs a fast discrete Fourier transform (FFT) of
  a complex sequence, x, of an arbitrary length, n. The output, y,
  is also a complex sequence of length n.
  
  y[k] = sum(x[m]*exp(-i*2*pi*k*m/n), m=0..(n-1)), k=0,...,(n-1)
  
  The largest prime factor of n must be less than or equal to the
  constant maxPrimeFactor defined below.
  ------------------------------------------------------------------------
  Author:
  Jens Jorgen Nielsen             For non-commercial use only.
  Bakkehusene 54                  A $100 fee must be paid if used
  DK-2970 Horsholm                commercially. Please contact.
  DENMARK
  
  e-mail : jnielsen@internet.dk   All rights reserved. March 1996.
  ------------------------------------------------------------------------
  Implementation notes:
  The general idea is to factor the length of the DFT, n, into
  factors that are efficiently handled by the routines.
  
  A number of short DFT's are implemented with a minimum of
      arithmetical operations and using (almost) straight line code
      resulting in very fast execution when the factors of n belong
      to this set. Especially radix-10 is optimized.

      Prime factors, that are not in the set of short DFT's are handled
      with direct evaluation of the DFP expression.

      Please report any problems to the author. 
      Suggestions and improvements are welcomed.
 ------------------------------------------------------------------------
  Benchmarks:                   
      The Microsoft Visual C++ compiler was used with the following 
      compile options:
      /nologo /Gs /G2 /W4 /AH /Ox /D "NDEBUG" /D "_DOS" /FR
      and the FFTBENCH test executed on a 50MHz 486DX :
      
      Length  Time [s]  Accuracy [dB]

         128   0.0054     -314.8   
         256   0.0116     -309.8   
         512   0.0251     -290.8   
        1024   0.0567     -313.6   
        2048   0.1203     -306.4   
        4096   0.2600     -291.8   
        8192   0.5800     -305.1   
         100   0.0040     -278.5   
         200   0.0099     -280.3   
         500   0.0256     -278.5   
        1000   0.0540     -278.5   
        2000   0.1294     -280.6   
        5000   0.3300     -278.4   
       10000   0.7133     -278.5   
 ------------------------------------------------------------------------
  The following procedures are used :
      factorize       :  factor the transformation length.
      transTableSetup :  setup table with sofar-, actual-, and remainRadix.
      permute         :  permutation allows in-place calculations.
      twiddleTransf   :  twiddle multiplications and DFT's for one stage.
      initTrig        :  initialise sine/cosine table.
      fft_4           :  length 4 DFT, a la Nussbaumer.
      fft_5           :  length 5 DFT, a la Nussbaumer.
      fft_7           :  length 7 DFT, a la Nussbaumer 'Added by Alexia Gossa - Inspired by Ernst W. Mayer / http://hogranch.com/mayer/src/C/'
      fft_10          :  length 10 DFT using prime factor FFT.
      fft_odd         :  length n DFT, n odd.


    fft_5 from Jens Jorgen Nielsen is same as Ernst W. Mayer expect :
        J.J. Nielsen routine uses more local variables
        E.W. Mayer routine uses less local variables to improve compiler performance and memory redondancy usage

    fft_7 'by Alexia Gosssa'
        This routine was very inspired by the radix-7 FFT routine of Ernst W. Mayer
        Improvement are done to constant variables.

*************************************************************************/

#include "audiotools_fft_jjn.h"

//#define  maxPrimeFactor        37
//#define  maxPrimeFactor        4096
//#define absoluteMaxPrimeFactor     32768

//#define  maxPrimeFactorDiv2    (maxPrimeFactor+1)/2
//#define  maxFactorCount        20

static FLOAT32  c3_1 = -1.5000000000000E+00F;  /*  c3_1 = cos(2*pi/3)-1;          */
static FLOAT32  c3_2 =  8.6602540378444E-01F;  /*  c3_2 = sin(2*pi/3);            */
                                          
static FLOAT32  u5   =  1.2566370614359E+00F;  /*  u5   = 2*pi/5;                 */
static FLOAT32  c5_1 = -1.2500000000000E+00F;  /*  c5_1 = (cos(u5)+cos(2*u5))/2-1;*/
static FLOAT32  c5_2 =  5.5901699437495E-01F;  /*  c5_2 = (cos(u5)-cos(2*u5))/2;  */
static FLOAT32  c5_3 = -9.5105651629515E-01F;  /*  c5_3 = -sin(u5);               */
static FLOAT32  c5_4 = -1.5388417685876E+00F;  /*  c5_4 = -(sin(u5)+sin(2*u5));   */
static FLOAT32  c5_5 =  3.6327126400268E-01F;  /*  c5_5 = (sin(u5)-sin(2*u5));    */
static FLOAT32  c8   =  7.0710678118655E-01F;  /*  c8 = 1/sqrt(2);    */
static FLOAT32  c7_2PI7 =  0.89759790102565521098932668093700F;    /* 2*PI/7 */
static FLOAT32  c7_cos1 =  0.62348980185873353052500488400424F;  /* cos1 = cos(2*1*PI/7) */
static FLOAT32  c7_cos2 = -0.22252093395631440428890256449679F;  /* cos2 = cos(2*2*PI/7) */
static FLOAT32  c7_cos3 = -0.90096886790241912623610231950745F;  /* cos3 = cos(2*3*PI/7) */
static FLOAT32  c7_sin1 =  0.78183148246802980870844452667406F;  /* sin1 = sin(2*1*PI/7) */
static FLOAT32  c7_sin2 =  0.97492791218182360701813168299393F;  /* sin2 = sin(2*2*PI/7) */
static FLOAT32  c7_sin3 =  0.43388373911755812047576833284836F;  /* sin3 = sin(2*3*PI/7) */


static FLOAT32  c7_cx0  = -0.16666666666666666666666666666667F;  /* (cos1+cos2+cos3)/3 = -1/6 */
static FLOAT32  c7_cx1  =  1.52445866976115265676110720351170F;  /* cos1-cos3 */
static FLOAT32  c7_cx2  =  0.67844793394610472194719975501065F;  /* cos2-cos3 */
static FLOAT32  c7_cx3  =  0.73430220123575245956943565284078F;	/* (cc1+cc2-2*cc3)/3	*/
static FLOAT32  c7_sx0  =  0.44095855184409843175026929227321F;	/* (ss1+ss2-ss3)/3	*/
static FLOAT32  c7_sx1  =  1.21571522158558792918421285952240F; 	/*  ss1+ss3		*/
static FLOAT32  c7_sx2  =  1.40881165129938172749390001584230F; 	/*  ss2+ss3		*/
static FLOAT32  c7_sx3  =  0.87484229096165655222603762512157F;	/* (ss1+ss2+2*ss3)/3	*/

    static FLOAT32 c8_cx0 =-0.16666666666666666667F,	/* (cc1+cc2+cc3)/3	*/
		          c8_cx1 = 1.52445866976115265675F, 	/*  cc1-cc3		*/
		          c8_cx2 = 0.67844793394610472196F, 	/*  cc2-cc3		*/
		          c8_cx3 = 0.73430220123575245957F,	/* (cc1+cc2-2*cc3)/3	*/
		          c8_sx0 = 0.44095855184409843174F,	/* (ss1+ss2-ss3)/3	*/
		          c8_sx1 = 1.21571522158558792920F, 	/*  ss1+ss3		*/
		          c8_sx2 = 1.40881165129938172752F, 	/*  ss2+ss3		*/
		          c8_sx3 = 0.87484229096165655224F;	/* (ss1+ss2+2*ss3)/3	*/


static FLOAT32  c5_cx1  = -1.25000000000000000000F;               /* [cos(u)+cos(2u)]/2-1 = -5/4 */
static FLOAT32  c5_cx2  =  0.55901699437494742409F;               /* [cos(u)-cos(2u)]/2 */
static FLOAT32  c5_s1   =  0.95105651629515357211F;               /*  sin(u) */
static FLOAT32  c5_sx1  =  1.53884176858762670130F;               /* [sin(u)+sin(2u)] */
static FLOAT32  c5_sx2  =  0.36327126400268044292F;               /* [sin(u)-sin(2u)] */

/*
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
             *zRe, 
             *zIm;
    FLOAT32   *vRe, *vIm;
    FLOAT32   *wRe, *wIm;
} _FFT_JJN_DATA;
*/



/*
#define maxPrimeFactor  (piData->maxPrimeFactor)
#define pi              (piData->pi)
#define groupOffset     (piData->groupOffset)
#define dataOffset      (piData->dataOffset)
#define blockOffset     (piData->blockOffset)
#define adr             (piData->adr)
#define groupNo         (piData->groupNo)
#define dataNo          (piData->dataNo)
#define blockNo         (piData->blockNo)
#define twNo            (piData->twNo)
#define omega           (piData->omega)
#define tw_re           (piData->tw_re)
#define tw_im           (piData->tw_im)
#define twiddleRe       (piData->twiddleRe)
#define twiddleIm       (piData->twiddleIm)
#define trigRe          (piData->trigRe)
#define trigIm          (piData->trigIm)
#define zRe             (piData->zRe)
#define zIm             (piData->zIm)
#define vRe             (piData->vRe)
#define vIm             (piData->vIm)
#define wRe             (piData->wRe)
#define wIm             (piData->wIm)
#define _def_piData_    _FFT_JJN_DATA*piData
#define _pass_piData_   piData
*/





/****************************************************************************
  Twiddle factor multiplications and transformations are performed on a
  group of data. The number of multiplications with 1 are reduced by skipping
  the twiddle multiplication of the first stage and of the first group of the
  following stages.
 ***************************************************************************/

void FFT_JJN_initTrig(long radix,_FFT_JJN_DATA *piData)
{
    long i;
    FLOAT32 w,xre,xim;

    w=2*piData->pi/radix;
    piData->trigRe[0]=1; piData->trigIm[0]=0;
    xre=(FLOAT32)cos(w); 
    xim=-(FLOAT32)sin(w);
    piData->trigRe[1]=xre; piData->trigIm[1]=xim;
    for (i=2; i<radix; i++)
    {
        piData->trigRe[i]=xre*piData->trigRe[i-1] - xim*piData->trigIm[i-1];
        piData->trigIm[i]=xim*piData->trigRe[i-1] + xre*piData->trigIm[i-1];
    }
}   /* initTrig */






void FFT_JJN_twiddleTransf(long sofarRadix, long radix, long remainRadix,
		   FLOAT32 yRe[], FLOAT32 yIm[], _FFT_JJN_DATA *piData)

{   /* twiddleTransf */ 
    FLOAT32  cosw, sinw, gem;
    long lOffReal;
    long lOffImag;
    long lOffAdrs;
    long lOffBloc;
    long lCounter;

    FFT_JJN_initTrig(radix, piData);

    piData->omega = 2*piData->pi/(FLOAT32)(sofarRadix*radix);
    cosw =  (FLOAT32)cos(piData->omega);
    sinw = -(FLOAT32)sin(piData->omega);
    piData->tw_re = 1.0;
    piData->tw_im = 0;
    piData->dataOffset=0;
    piData->groupOffset=piData->dataOffset;
    piData->adr=piData->groupOffset;
    for (piData->dataNo=0; piData->dataNo<sofarRadix; piData->dataNo++)
    {
        if (sofarRadix>1)
        {
	        piData->twiddleRe[0] = 1.0; 
	        piData->twiddleIm[0] = 0.0;
	        piData->twiddleRe[1] = piData->tw_re;
	        piData->twiddleIm[1] = piData->tw_im;
            lCounter             = radix - 1;
            lOffAdrs             = 2;
            lOffBloc             = 1;

	        for (;lCounter;lCounter--)
            {
	            piData->twiddleRe[lOffAdrs]=piData->tw_re*piData->twiddleRe[lOffBloc] - piData->tw_im*piData->twiddleIm[lOffBloc];
	            piData->twiddleIm[lOffAdrs]=piData->tw_im*piData->twiddleRe[lOffBloc] + piData->tw_re*piData->twiddleIm[lOffBloc];
                lOffAdrs++;
                lOffBloc++;
            }

            /*
	        for (piData->twNo=2; piData->twNo<radix; piData->twNo++)
            {
	            piData->twiddleRe[piData->twNo]=piData->tw_re*piData->twiddleRe[piData->twNo-1] - piData->tw_im*piData->twiddleIm[piData->twNo-1];
	            piData->twiddleIm[piData->twNo]=piData->tw_im*piData->twiddleRe[piData->twNo-1] + piData->tw_re*piData->twiddleIm[piData->twNo-1];
            }
            */
	        gem             = cosw*piData->tw_re - sinw*piData->tw_im;
	        piData->tw_im   = sinw*piData->tw_re + cosw*piData->tw_im;
	        piData->tw_re   = gem;                      
        }

        for (piData->groupNo=0; piData->groupNo<remainRadix; piData->groupNo++)
        {
            
	        if ((sofarRadix>1) && (piData->dataNo > 0))
            {
                
                //piData->zReIm[0]=yRe[piData->adr];
	            //piData->zReIm[1]=yIm[piData->adr];
	            //piData->blockNo=1;
	            //do {
		        //    piData->adr = piData->adr + sofarRadix;
		        //    piData->zReIm[ (piData->blockNo)<<1   ]=  piData->twiddleRe[piData->blockNo] * yRe[piData->adr] - piData->twiddleIm[piData->blockNo] * yIm[piData->adr];
		        //    piData->zReIm[((piData->blockNo)<<1)+1]=  piData->twiddleRe[piData->blockNo] * yIm[piData->adr] + piData->twiddleIm[piData->blockNo] * yRe[piData->adr]; 
		        //    piData->blockNo++;
	            //} while (piData->blockNo < radix);

                lOffReal    = 0;
                lOffImag    = 1;
                lOffAdrs    = piData->adr;
                lOffBloc    = 0;
                lCounter    = radix - piData->blockNo;
                if (lCounter<1) lCounter = 1;

		        piData->zReIm[lOffReal] = yRe[lOffAdrs];
		        piData->zReIm[lOffImag] = yIm[lOffAdrs];

                for (;lCounter;lCounter--)
                {
                    lOffAdrs += sofarRadix;
                    lOffReal += 2;
                    lOffImag += 2;
                    lOffBloc += 1;

		            piData->zReIm[lOffReal] = 
                        piData->twiddleRe[lOffBloc] * yRe[lOffAdrs] - 
                        piData->twiddleIm[lOffBloc] * yIm[lOffAdrs];

		            piData->zReIm[lOffImag] = 
                        piData->twiddleRe[lOffBloc] * yIm[lOffAdrs] + 
                        piData->twiddleIm[lOffBloc] * yRe[lOffAdrs]; 
                }
            }
	        else
            {
                   
	            //for (piData->blockNo=0; piData->blockNo<radix; piData->blockNo++)
	            //{
		        //    piData->zReIm[ (piData->blockNo)<<1   ]=yRe[piData->adr];
		        //    piData->zReIm[((piData->blockNo)<<1)+1]=yIm[piData->adr];
		        //    piData->adr=piData->adr+sofarRadix;
	            //}

                lCounter    = radix;
                lOffReal    = 0;
                lOffImag    = 1;
                lOffAdrs    = piData->adr;                
	            for (;lCounter;lCounter--)
	            {
		            piData->zReIm[lOffReal] = yRe[lOffAdrs];
		            piData->zReIm[lOffImag] = yIm[lOffAdrs];
                    lOffAdrs += sofarRadix;
                    lOffReal += 2;
                    lOffImag += 2;
	            }
            }
            

            
	        switch(radix) 
            {
	            case  2: FFT_JJN_radix2_OutInterleavedFast ( piData->zReIm, yRe+piData->groupOffset, yIm+piData->groupOffset, sofarRadix ); break;
	            case  3: FFT_JJN_radix3_OutInterleavedFast ( piData->zReIm, yRe+piData->groupOffset, yIm+piData->groupOffset, sofarRadix ); break;
                case  4: FFT_JJN_radix4_OutInterleavedFast ( piData->zReIm, yRe+piData->groupOffset, yIm+piData->groupOffset, sofarRadix ); break;
                case  5: FFT_JJN_radix5_OutInterleavedFast ( piData->zReIm, yRe+piData->groupOffset, yIm+piData->groupOffset, sofarRadix ); break;
                case  7: FFT_JJN_radix7_OutInterleavedFast ( piData->zReIm, yRe+piData->groupOffset, yIm+piData->groupOffset, sofarRadix ); break;
                case  8: FFT_JJN_radix8_OutInterleavedFast ( piData->zReIm, yRe+piData->groupOffset, yIm+piData->groupOffset, sofarRadix ); break;
                case 10: FFT_JJN_radix10_OutInterleavedFast ( piData->zReIm, yRe+piData->groupOffset, yIm+piData->groupOffset, sofarRadix ); break;
	            default: FFT_JJN_radixOdd_OutInterleavedFast ( piData->zReIm, yRe+piData->groupOffset, yIm+piData->groupOffset, sofarRadix,
                                                               radix, piData->vRe, piData->vIm, piData->wRe, piData->wIm, piData->trigRe, piData->trigIm ); break;
	        }
            
            /*
            if (lDoCopyReAlign)
            {
	            piData->adr=piData->groupOffset;
	            for (piData->blockNo=0; piData->blockNo<radix; piData->blockNo++)
                {
	                yRe[piData->adr]=piData->zReIm[ (piData->blockNo)<<1   ]; 
                    yIm[piData->adr]=piData->zReIm[piData->blockNo];
	                piData->adr=piData->adr+sofarRadix;
                }
            }
            */
            
	        piData->groupOffset=piData->groupOffset+sofarRadix*radix;
	        piData->adr=piData->groupOffset;
        }
        piData->dataOffset=piData->dataOffset+1;
        piData->groupOffset=piData->dataOffset;
        piData->adr=piData->groupOffset;
    }
}   /* twiddleTransf */


long FFT_JJN_AllocFFTNeeds ( void ** pData, long lNbPoints )
{
    _FFT_JJN_DATA       *piData;
    long                count;

    long                lMaxPrimeFactor;
    long                lMaxPrimeFactorDiv2;
    FLOAT32              * pdfTempSpace;
    long                lNbElements;

    //Convention de renommage simplifier
    piData = (_FFT_JJN_DATA *)AUDIOTOOLS_MALLOC(sizeof(_FFT_JJN_DATA));
    
    //Vide la structure
    memset ( piData, 0, sizeof(_FFT_JJN_DATA) );    

    //Récupération de "PI"
    piData->pi = 4*(FLOAT32)atan(1);

    //Stockage du nombre de points
    piData->lNbPoints = lNbPoints;

    //Génération des facteurs...
    if (FFT_JJN_transTableSetup ( 
            piData->lSoFarRadix, 
            piData->lActualRadix,
            piData->lRemainRadix,
            &piData->lnFactor, 
            &piData->lNbPoints ))
    {
        AUDIOTOOLS_FREE ( piData );
        return -1;
    }

    //Recherche du plus grand nombre indivisible...
    lMaxPrimeFactor = 16;
    for (count=1; count<=piData->lnFactor; count++)
    {
        if (piData->lActualRadix[count]>lMaxPrimeFactor) lMaxPrimeFactor = piData->lActualRadix[count];
    }
    piData->maxPrimeFactor      = lMaxPrimeFactor;
    lMaxPrimeFactorDiv2 =   (lMaxPrimeFactor+1)/2;

    //if (lMaxPrimeFactor<32) printf ( "Low Prime : %d : %d\n", n, lMaxPrimeFactor );
    //if (lMaxPrimeFactor>255) printf ( "High Prime : %d : %d\n", n, lMaxPrimeFactor );

    //Allocation du tampon
    lNbElements = 
        lMaxPrimeFactor +       //twiddleRe
        lMaxPrimeFactor +       //twiddleIm
        lMaxPrimeFactor +       //trigRe
        lMaxPrimeFactor +       //trigIm
        lMaxPrimeFactor +       //zRe
        lMaxPrimeFactor +       //zRe
        lMaxPrimeFactorDiv2 +   //vRe
        lMaxPrimeFactorDiv2 +   //vIm
        lMaxPrimeFactorDiv2 +   //wRe
        lMaxPrimeFactorDiv2;    //wIm

    pdfTempSpace = (FLOAT32 *)AUDIOTOOLS_CALLOC ( lNbElements, sizeof(FLOAT32) );

    //On place les pointeurs
    piData->twiddleRe = pdfTempSpace;
    piData->twiddleIm = pdfTempSpace + (lMaxPrimeFactor);
    piData->trigRe    = pdfTempSpace + (lMaxPrimeFactor*2);
    piData->trigIm    = pdfTempSpace + (lMaxPrimeFactor*3);
    piData->zReIm     = pdfTempSpace + (lMaxPrimeFactor*4);
    //piData->zRe       = pdfTempSpace + (lMaxPrimeFactor*4);
    //piData->zIm       = pdfTempSpace + (lMaxPrimeFactor*5);
    piData->vRe       = pdfTempSpace + (lMaxPrimeFactor*6);
    piData->vIm       = pdfTempSpace + (lMaxPrimeFactor*6) + (lMaxPrimeFactorDiv2);
    piData->wRe       = pdfTempSpace + (lMaxPrimeFactor*6) + (lMaxPrimeFactorDiv2*2);
    piData->wIm       = pdfTempSpace + (lMaxPrimeFactor*6) + (lMaxPrimeFactorDiv2*3);

    //Passage du pointeur
    *pData = (void*)piData;

    return 0;
}

long FFT_JJN_FreeFFTNeeds ( void * pData )
{
    _FFT_JJN_DATA   *piData;

    if (!pData) return -1;

    //Pointeur sur les données
    piData = (_FFT_JJN_DATA *)pData;

    //Libération de la mémoire
    AUDIOTOOLS_FREE ( piData->twiddleRe );
    AUDIOTOOLS_FREE ( piData );
    
    return 0;
}

long FFT_JJN_do_fft ( void * pData, FLOAT32 xRe[], FLOAT32 xIm[], FLOAT32 yRe[], FLOAT32 yIm[] )
{
    _FFT_JJN_DATA   *piData;
    long            lIndex;

    //Pointeur sur les données
    piData = (_FFT_JJN_DATA *)pData;

  
    //Permutation
    FFT_JJN_permute ( 
        piData->lNbPoints, 
        piData->lnFactor, 
        piData->lActualRadix, 
        piData->lRemainRadix, 
        xRe, 
        xIm, 
        yRe, 
        yIm );
    
    for (lIndex=1;lIndex<=piData->lnFactor;lIndex++)
    {
        FFT_JJN_twiddleTransf ( 
            piData->lSoFarRadix[lIndex], 
            piData->lActualRadix[lIndex], 
            piData->lRemainRadix[lIndex], 
		    yRe, yIm,
            piData );
    }

    return 0;
}


/*
long FFT_JJN_do_fft_old ( long n, FLOAT32 xRe[], FLOAT32 xIm[],
	 FLOAT32 yRe[], FLOAT32 yIm[])
{
    long    sofarRadix[maxFactorCount], 
            actualRadix[maxFactorCount], 
            remainRadix[maxFactorCount];
    long    nFactor;
    long    count;

    long    lMaxPrimeFactor;
    long    lMaxPrimeFactorDiv2;

    FLOAT32  * pdfTempSpace;
    long    lNbElements;
    _FFT_JJN_DATA   iData, * piData;

    //Données internes
    piData = &iData;
    memset ( piData, 0, sizeof(_FFT_JJN_DATA) );

    piData->pi = 4*atan(1);

    //Génération des facteurs...
    if (FFT_JJN_transTableSetup ( 
            sofarRadix, 
            actualRadix,
            remainRadix,
            &nFactor, 
            &n ))
    {
        return -1;
    }

    //Recherche du plus grand nombre indivisible...
    lMaxPrimeFactor = 16;
    for (count=1; count<=nFactor; count++)
    {
        if (actualRadix[count]>lMaxPrimeFactor) lMaxPrimeFactor = actualRadix[count];
    }
    piData->maxPrimeFactor      = lMaxPrimeFactor;
    lMaxPrimeFactorDiv2 =   (lMaxPrimeFactor+1)/2;

    //if (lMaxPrimeFactor<32) printf ( "Low Prime : %d : %d\n", n, lMaxPrimeFactor );
    //if (lMaxPrimeFactor>255) printf ( "High Prime : %d : %d\n", n, lMaxPrimeFactor );


    //Allocation du tampon
    lNbElements = 
        lMaxPrimeFactor +       //twiddleRe
        lMaxPrimeFactor +       //twiddleIm
        lMaxPrimeFactor +       //trigRe
        lMaxPrimeFactor +       //trigIm
        lMaxPrimeFactor +       //zRe
        lMaxPrimeFactor +       //zRe
        lMaxPrimeFactorDiv2 +   //vRe
        lMaxPrimeFactorDiv2 +   //vIm
        lMaxPrimeFactorDiv2 +   //wRe
        lMaxPrimeFactorDiv2;    //wIm

    pdfTempSpace = (FLOAT32 *)calloc ( lNbElements, sizeof(FLOAT32) );

    //On place les pointeurs
    piData->twiddleRe = pdfTempSpace;
    piData->twiddleIm = pdfTempSpace + (lMaxPrimeFactor);
    piData->trigRe    = pdfTempSpace + (lMaxPrimeFactor*2);
    piData->trigIm    = pdfTempSpace + (lMaxPrimeFactor*3);
    piData->zRe       = pdfTempSpace + (lMaxPrimeFactor*4);
    piData->zIm       = pdfTempSpace + (lMaxPrimeFactor*5);
    piData->vRe       = pdfTempSpace + (lMaxPrimeFactor*6);
    piData->vIm       = pdfTempSpace + (lMaxPrimeFactor*6) + (lMaxPrimeFactorDiv2);
    piData->wRe       = pdfTempSpace + (lMaxPrimeFactor*6) + (lMaxPrimeFactorDiv2*2);
    piData->wIm       = pdfTempSpace + (lMaxPrimeFactor*6) + (lMaxPrimeFactorDiv2*3);

    FFT_JJN_permute ( 
        n, 
        nFactor, 
        actualRadix, 
        remainRadix, 
        xRe, 
        xIm, 
        yRe, 
        yIm );

    for (count=1; count<=nFactor; count++)
    {
        FFT_JJN_twiddleTransf ( 
            sofarRadix[count], 
            actualRadix[count], 
            remainRadix[count], 
		    yRe, yIm,
            piData );
    }

    free ( pdfTempSpace );
    return 0;
}
*/


/*********************************************************************** 
  FFT - Do an FFT on the signal. The FFT coefficients are divided by 
  sqrt(N), so that the sum(X^2) = sum(x^2).

  Args:
  mag - The signal which will contain the magnitude of the FT.
  ph  - The signal which will contain the phase of the FT.
  
  Result: Nothing.
  **********************************************************************/

/*
void DoFFT ( 



Signal* &mag, Signal* &ph)
{       
  mag = new Signal(length/2);
  ph = new Signal(length/2);
  FLOAT32 *xRe=0, *xIm=0, *yRe=0, *yIm=0, angle;
  int t;
  float sqrtN;

  sqrtN = sqrt(length);

  if (!(xRe = (FLOAT32*) malloc(sizeof(FLOAT32)*length*4)))
    {
      fprintf(stderr, "FFT: Out of memory!\n");
    }
  xIm = xRe + length*1;
  yRe = xRe + length*2;
  yIm = xRe + length*3;

  for (t = 0; t < length; t++)
    {
      xRe[t] = data[t];
      xIm[t] = 0;
    }
  fft(length, xRe, xIm, yRe, yIm);

  for (t = 0; t < length/2; t++)
    {
      mag->data[t] = (float) hypot(yRe[t],yIm[t]) / sqrtN;
      ph->data[t] = (float) atan2(yIm[t], yRe[t]);
    }
  free(xRe);
}
*/