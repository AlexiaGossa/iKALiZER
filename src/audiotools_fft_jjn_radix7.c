#include "audiotools_fft_jjn.h"

static FLOAT32  c7_cx0  = -0.16666666666666666666666666666667F;  /* (cos1+cos2+cos3)/3 = -1/6 */
static FLOAT32  c7_cx1  =  1.52445866976115265676110720351170F;  /* cos1-cos3 */
static FLOAT32  c7_cx2  =  0.67844793394610472194719975501065F;  /* cos2-cos3 */
static FLOAT32  c7_cx3  =  0.73430220123575245956943565284078F;	/* (cc1+cc2-2*cc3)/3	*/
static FLOAT32  c7_sx0  =  0.44095855184409843175026929227321F;	/* (ss1+ss2-ss3)/3	*/
static FLOAT32  c7_sx1  =  1.21571522158558792918421285952240F; 	/*  ss1+ss3		*/
static FLOAT32  c7_sx2  =  1.40881165129938172749390001584230F; 	/*  ss2+ss3		*/
static FLOAT32  c7_sx3  =  0.87484229096165655222603762512157F;	/* (ss1+ss2+2*ss3)/3	*/

// Faster DFT - 72 ADD and 16 MUL - A la Nussbaumer optimized
void FFT_JJN_radix7 ( FLOAT32 *aRe, FLOAT32 *aIm )
{
	FLOAT32 rt,it,t1,t2,t3,t4,t5,t6,t7,t8,t9,t10,t11,t12,t13,t14;

    //2x6 ADD + 2x0 MUL = 12 ADD + 0 MUL
    t1 =aRe[0];			            t2 =aIm[0];			/* x0	*/
    t3 =aRe[1]+aRe[6];	            t4 =aIm[1]+aIm[6];	/* x1 + x6	*/
    t5 =aRe[2]+aRe[5];	            t6 =aIm[2]+aIm[5];	/* x2 + x5	*/
    t7 =aRe[3]+aRe[4];	            t8 =aIm[3]+aIm[4];	/* x3 + x4	*/
    t9 =aRe[3]-aRe[4];	            t10=aIm[3]-aIm[4];	/* x3 - x4	*/
    t11=aRe[2]-aRe[5];	            t12=aIm[2]-aIm[5];	/* x2 - x5	*/
    t13=aRe[1]-aRe[6];	            t14=aIm[1]-aIm[6];	/* x1 - x6	*/

    //DC-Value
    //2x4 ADD + 2x0 MUL = 8 ADD + 0 MUL
    rt      = t3+t7+t5;             it      = t4+t8+t6;
    aRe[0]  = rt+t1;                aIm[0]  = it+t2;
    
    //2x6 ADD + 2x4 MUL = 12 ADD + 8 MUL
    t1 = (rt*c7_cx0)+t1;            t2 = (it*c7_cx0)+t2;
    t3 = t3-t5;                     t4 = t4-t6;
    t5 = t7-t5;                     t6 = t8-t6;
    t7 = (t3+t5)*c7_cx3;            t8 = (t4+t6)*c7_cx3;
    t3 = t3*c7_cx1;                 t4 = t4*c7_cx1;
    t5 = t5*c7_cx2;                 t6 = t6*c7_cx2;
    rt = t3-t7;                     it = t4-t8;
    t5 = t5-t7;                     t6 = t6-t8;

    //2x4 ADD + 2x0 MUL = 8 ADD + 0 MUL
    t3 = t1-rt-t5;                  t4 = t2-it-t6;
    t5 = t1+t5;                     t6 = t2+t6;
    t1 = t1+rt;                     t2 = t2+it;

    //2x6 ADD + 2x4 MUL = 12 ADD + 8 MUL
    t7 =(t13-t9 +t11)*c7_sx0;	    t8 =(t14-t10+t12)*c7_sx0;
    t13= t13-t11;			        t14= t14-t12;
    t11= t9 +t11;			        t12= t10+t12;
    t9 =(t11-t13)*c7_sx3;		    t10=(t12-t14)*c7_sx3;
    t13= t13*c7_sx1;			    t14= t14*c7_sx1;
    t11= t11*c7_sx2;			    t12= t12*c7_sx2;
    t13= t9+t13;			        t14= t10+t14;
    t11= t9-t11;			        t12= t10-t12;

    //2x4 ADD + 2x0 MUL = 8 ADD + 0 MUL
    t9 = t7-t13-t11;		        t10= t8-t14-t12;
    t11= t7+t11;			        t12= t8+t12;
    t7 = t7+t13;			        t8 = t8+t14;

    //2x6 ADD + 2x0 MUL = 12 ADD + 0 MUL
    aRe[1] =t1+t8;		            aIm[1] =t2-t7;
    aRe[2] =t3+t10;		            aIm[2] =t4-t9;
    aRe[3] =t5-t12;		            aIm[3] =t6+t11;
    aRe[4] =t5+t12;		            aIm[4] =t6-t11;
    aRe[5] =t3-t10;		            aIm[5] =t4+t9;
    aRe[6] =t1-t8;		            aIm[6] =t2+t7;
}

void FFT_JJN_radix7_OutInterleaved ( FLOAT32 IN *aRe, FLOAT32 IN *aIm, FLOAT32 OUT *zRe, FLOAT32 OUT *zIm, long OIl )
{
	FLOAT32 rt,it,t1,t2,t3,t4,t5,t6,t7,t8,t9,t10,t11,t12,t13,t14;
    long lOIl2, lOIl3, lOIl4, lOIl5, lOIl6;

    lOIl2 = OIl<<1;
    lOIl3 = lOIl2 + OIl;
    lOIl4 = OIl<<2;
    lOIl5 = lOIl4 + OIl;
    lOIl6 = lOIl2 + lOIl4;

    //2x6 ADD + 2x0 MUL = 12 ADD + 0 MUL
    t1 =aRe[0];			            t2 =aIm[0];			/* x0	*/
    t3 =aRe[1]+aRe[6];	            t4 =aIm[1]+aIm[6];	/* x1 + x6	*/
    t5 =aRe[2]+aRe[5];	            t6 =aIm[2]+aIm[5];	/* x2 + x5	*/
    t7 =aRe[3]+aRe[4];	            t8 =aIm[3]+aIm[4];	/* x3 + x4	*/
    t9 =aRe[3]-aRe[4];	            t10=aIm[3]-aIm[4];	/* x3 - x4	*/
    t11=aRe[2]-aRe[5];	            t12=aIm[2]-aIm[5];	/* x2 - x5	*/
    t13=aRe[1]-aRe[6];	            t14=aIm[1]-aIm[6];	/* x1 - x6	*/

    //DC-Value
    //2x4 ADD + 2x0 MUL = 8 ADD + 0 MUL
    rt      = t3+t7+t5;             it      = t4+t8+t6;
    zRe[0]  = rt+t1;                zIm[0]  = it+t2;
    
    //2x6 ADD + 2x4 MUL = 12 ADD + 8 MUL
    t1 = (rt*c7_cx0)+t1;            t2 = (it*c7_cx0)+t2;
    t3 = t3-t5;                     t4 = t4-t6;
    t5 = t7-t5;                     t6 = t8-t6;
    t7 = (t3+t5)*c7_cx3;            t8 = (t4+t6)*c7_cx3;
    t3 = t3*c7_cx1;                 t4 = t4*c7_cx1;
    t5 = t5*c7_cx2;                 t6 = t6*c7_cx2;
    rt = t3-t7;                     it = t4-t8;
    t5 = t5-t7;                     t6 = t6-t8;

    //2x4 ADD + 2x0 MUL = 8 ADD + 0 MUL
    t3 = t1-rt-t5;                  t4 = t2-it-t6;
    t5 = t1+t5;                     t6 = t2+t6;
    t1 = t1+rt;                     t2 = t2+it;

    //2x6 ADD + 2x4 MUL = 12 ADD + 8 MUL
    t7 =(t13-t9 +t11)*c7_sx0;	    t8 =(t14-t10+t12)*c7_sx0;
    t13= t13-t11;			        t14= t14-t12;
    t11= t9 +t11;			        t12= t10+t12;
    t9 =(t11-t13)*c7_sx3;		    t10=(t12-t14)*c7_sx3;
    t13= t13*c7_sx1;			    t14= t14*c7_sx1;
    t11= t11*c7_sx2;			    t12= t12*c7_sx2;
    t13= t9+t13;			        t14= t10+t14;
    t11= t9-t11;			        t12= t10-t12;

    //2x4 ADD + 2x0 MUL = 8 ADD + 0 MUL
    t9 = t7-t13-t11;		        t10= t8-t14-t12;
    t11= t7+t11;			        t12= t8+t12;
    t7 = t7+t13;			        t8 = t8+t14;

    //2x6 ADD + 2x0 MUL = 12 ADD + 0 MUL
    zRe[OIl]    = t1+t8;		    zIm[OIl]    = t2-t7;
    zRe[lOIl2]  = t3+t10;		    zIm[lOIl2]  = t4-t9;
    zRe[lOIl3]  = t5-t12;		    zIm[lOIl3]  = t6+t11;
    zRe[lOIl4]  = t5+t12;		    zIm[lOIl4]  = t6-t11;
    zRe[lOIl5]  = t3-t10;		    zIm[lOIl5]  = t4+t9;
    zRe[lOIl6]  = t1-t8;		    zIm[lOIl6]  = t2+t7;
}

void FFT_JJN_radix7_OutInterleavedFast ( FLOAT32 IN *aReIm, FLOAT32 OUT *zRe, FLOAT32 OUT *zIm, long OIl )
{
	FLOAT32 rt,it,t1,t2,t3,t4,t5,t6,t7,t8,t9,t10,t11,t12,t13,t14;
    long lOIl2, lOIl3, lOIl4, lOIl5, lOIl6;

    lOIl2 = OIl<<1;
    lOIl3 = lOIl2 + OIl;
    lOIl4 = OIl<<2;
    lOIl5 = lOIl4 + OIl;
    lOIl6 = lOIl2 + lOIl4;

    //2x6 ADD + 2x0 MUL = 12 ADD + 0 MUL
    t1 =aReIm[0];			        t2 =aReIm[1];			/* x0	*/
    t3 =aReIm[2]+aReIm[12];	        t4 =aReIm[3]+aReIm[13];	/* x1 + x6	*/
    t5 =aReIm[4]+aReIm[10];	        t6 =aReIm[5]+aReIm[11];	/* x2 + x5	*/
    t7 =aReIm[6]+aReIm[8];	        t8 =aReIm[7]+aReIm[9];	/* x3 + x4	*/
    t9 =aReIm[6]-aReIm[8];	        t10=aReIm[7]-aReIm[9];	/* x3 - x4	*/
    t11=aReIm[4]-aReIm[10];	        t12=aReIm[5]-aReIm[11];	/* x2 - x5	*/
    t13=aReIm[2]-aReIm[12];	        t14=aReIm[3]-aReIm[13];	/* x1 - x6	*/

    //DC-Value
    //2x4 ADD + 2x0 MUL = 8 ADD + 0 MUL
    rt      = t3+t7+t5;             it      = t4+t8+t6;
    zRe[0]  = rt+t1;                zIm[0]  = it+t2;
    
    //2x6 ADD + 2x4 MUL = 12 ADD + 8 MUL
    t1 = (rt*c7_cx0)+t1;            t2 = (it*c7_cx0)+t2;
    t3 = t3-t5;                     t4 = t4-t6;
    t5 = t7-t5;                     t6 = t8-t6;
    t7 = (t3+t5)*c7_cx3;            t8 = (t4+t6)*c7_cx3;
    t3 = t3*c7_cx1;                 t4 = t4*c7_cx1;
    t5 = t5*c7_cx2;                 t6 = t6*c7_cx2;
    rt = t3-t7;                     it = t4-t8;
    t5 = t5-t7;                     t6 = t6-t8;

    //2x4 ADD + 2x0 MUL = 8 ADD + 0 MUL
    t3 = t1-rt-t5;                  t4 = t2-it-t6;
    t5 = t1+t5;                     t6 = t2+t6;
    t1 = t1+rt;                     t2 = t2+it;

    //2x6 ADD + 2x4 MUL = 12 ADD + 8 MUL
    t7 =(t13-t9 +t11)*c7_sx0;	    t8 =(t14-t10+t12)*c7_sx0;
    t13= t13-t11;			        t14= t14-t12;
    t11= t9 +t11;			        t12= t10+t12;
    t9 =(t11-t13)*c7_sx3;		    t10=(t12-t14)*c7_sx3;
    t13= t13*c7_sx1;			    t14= t14*c7_sx1;
    t11= t11*c7_sx2;			    t12= t12*c7_sx2;
    t13= t9+t13;			        t14= t10+t14;
    t11= t9-t11;			        t12= t10-t12;

    //2x4 ADD + 2x0 MUL = 8 ADD + 0 MUL
    t9 = t7-t13-t11;		        t10= t8-t14-t12;
    t11= t7+t11;			        t12= t8+t12;
    t7 = t7+t13;			        t8 = t8+t14;

    //2x6 ADD + 2x0 MUL = 12 ADD + 0 MUL
    zRe[OIl]    = t1+t8;		    zIm[OIl]    = t2-t7;
    zRe[lOIl2]  = t3+t10;		    zIm[lOIl2]  = t4-t9;
    zRe[lOIl3]  = t5-t12;		    zIm[lOIl3]  = t6+t11;
    zRe[lOIl4]  = t5+t12;		    zIm[lOIl4]  = t6-t11;
    zRe[lOIl5]  = t3-t10;		    zIm[lOIl5]  = t4+t9;
    zRe[lOIl6]  = t1-t8;		    zIm[lOIl6]  = t2+t7;
}


void FFT_JJN_fft_7_good ( FLOAT32 aRe[], FLOAT32 aIm[] )
{
	FLOAT32 rt,it,t1,t2,t3,t4,t5,t6,t7,t8,t9,t10,t11,t12,t13,t14;
    static FLOAT32 cx0 =-0.16666666666666666667F,	/* (cc1+cc2+cc3)/3	*/
		          cx1 = 1.52445866976115265675F, 	/*  cc1-cc3		*/
		          cx2 = 0.67844793394610472196F, 	/*  cc2-cc3		*/
		          cx3 = 0.73430220123575245957F,	/* (cc1+cc2-2*cc3)/3	*/
		          sx0 = 0.44095855184409843174F,	/* (ss1+ss2-ss3)/3	*/
		          sx1 = 1.21571522158558792920F, 	/*  ss1+ss3		*/
		          sx2 = 1.40881165129938172752F, 	/*  ss2+ss3		*/
		          sx3 = 0.87484229096165655224F;	/* (ss1+ss2+2*ss3)/3	*/


    t1 =aRe[0];			t2 =aIm[0];			/* x0	*/
    t3 =aRe[1]+aRe[6];	t4 =aIm[1]+aIm[6];	/* x1 + x6	*/
    t5 =aRe[2]+aRe[5];	t6 =aIm[2]+aIm[5];	/* x2 + x5	*/
    t7 =aRe[3]+aRe[4];	t8 =aIm[3]+aIm[4];	/* x3 + x4	*/
    t9 =aRe[3]-aRe[4];	t10=aIm[3]-aIm[4];	/* x3 - x4	*/
    t11=aRe[2]-aRe[5];	t12=aIm[2]-aIm[5];	/* x2 - x5	*/
    t13=aRe[1]-aRe[6];	t14=aIm[1]-aIm[6];	/* x1 - x6	*/

    rt = t3+t7+t5; 
    aRe[0]=rt+t1;	

    it = t4+t8+t6;         
    aIm[0]=it+t2;

    t1 = rt*cx0+t1;		t2 = it*cx0+t2;
    t3 = t3-t5;			t4 = t4-t6;
    t5 = t7-t5;			t6 = t8-t6;
    t7 =(t3+t5)*cx3;		t8 =(t4+t6)*cx3;
    t3 = t3*cx1;			t4 = t4*cx1;
    t5 = t5*cx2;			t6 = t6*cx2;
    rt = t3-t7;			it = t4-t8;
    t5 = t5-t7;			t6 = t6-t8;

    t3 = t1-rt-t5;		t4 = t2-it-t6;
    t5 = t1+t5;			t6 = t2+t6;
    t1 = t1+rt;			t2 = t2+it;

    t7 =(t13-t9 +t11)*sx0;	t8 =(t14-t10+t12)*sx0;
    t13= t13-t11;			t14= t14-t12;
    t11= t9 +t11;			t12= t10+t12;
    t9 =(t11-t13)*sx3;		t10=(t12-t14)*sx3;
    t13= t13*sx1;			t14= t14*sx1;
    t11= t11*sx2;			t12= t12*sx2;
    t13= t9+t13;			t14= t10+t14;
    t11= t9-t11;			t12= t10-t12;

    t9 = t7-t13-t11;		t10= t8-t14-t12;
    t11= t7+t11;			t12= t8+t12;
    t7 = t7+t13;			t8 = t8+t14;

    aRe[1] =t1+t8;		aIm[1] =t2-t7;
    aRe[2] =t3+t10;		aIm[2] =t4-t9;
    aRe[3] =t5-t12;		aIm[3] =t6+t11;
    aRe[4] =t5+t12;		aIm[4] =t6-t11;
    aRe[5] =t3-t10;		aIm[5] =t4+t9;
    aRe[6] =t1-t8;		aIm[6] =t2+t7;
}

// Normal DFT - 60 ADD and 36 MUL
void FFT_JJN_fft_7_ADD ( FLOAT32 aRe[], FLOAT32 aIm[] )
{
    static FLOAT32 cc1 =  0.62348980185873353053F,	/* cos(2*pi/7) */
		          ss1 =  0.78183148246802980870F,	/* sin(2*pi/7) */
		          cc2 = -0.22252093395631440427F,	/* cos(2*u) */
		          ss2 =  0.97492791218182360702F,	/* sin(2*u) */
		          cc3 = -0.90096886790241912622F,	/* cos(3*u) */
		          ss3 =  0.43388373911755812050F;	/* sin(3*u) */
	FLOAT32 rt,it,re,im,t1,t2,t3,t4,t5,t6,t7,t8,t9,t10,t11,t12,t13,t14;

      t1 =aRe[0];			t2 =aIm[0];			/* x0	*/
	  t3 =aRe[1]+aRe[6];	t4 =aIm[1]+aIm[6];	/* x1 + x6	*/
	  t5 =aRe[2]+aRe[5];	t6 =aIm[2]+aIm[5];	/* x2 + x5	*/
	  t7 =aRe[3]+aRe[4];	t8 =aIm[3]+aIm[4];	/* x3 + x4	*/
	  t9 =aRe[3]-aRe[4];	t10=aIm[3]-aIm[4];	/* x3 - x4	*/
	  t11=aRe[2]-aRe[5];	t12=aIm[2]-aIm[5];	/* x2 - x5	*/
	  t13=aRe[1]-aRe[6];	t14=aIm[1]-aIm[6];	/* x1 - x6	*/

        /*
      t1_re = aRe[1]+aRe[6];    t1_im = aIm[1]+aIm[6];
      t2_re = aRe[1]-aRe[6];    t2_im = aIm[1]-aIm[6];
      t3_re = aRe[2]+aRe[5];    t3_im = aIm[2]+aIm[5];
      t4_re = aRe[2]-aRe[5];    t4_im = aIm[2]-aIm[5];
      t5_re = aRe[3]+aRe[4];    t5_im = aIm[3]+aIm[4];
      t6_re = aRe[3]-aRe[4];    t6_im = aIm[3]-aIm[4];



      m1_re = c7_cos1*t1_re;    m1_im = c7_cos1*t1_im;
      m2_re = 
      */


/*      ...now complete the first complex datum of the result.	*/

        /*
        aj1p0r = t1+t3+t5+t7;		aj1p0i = t2+t4+t6+t8	;
	    rt = t1+cc1*t3+cc2*t5+cc3*t7;	it = t2+cc1*t4+cc2*t6+cc3*t8;
	    re = t1+cc2*t3+cc3*t5+cc1*t7;	im = t2+cc2*t4+cc3*t6+cc1*t8;
	    t1 = t1+cc3*t3+cc1*t5+cc2*t7;	t2 = t2+cc3*t4+cc1*t6+cc2*t8;

	    t3 = ss1*t13+ss2*t11+ss3*t9;	t4 = ss1*t14+ss2*t12+ss3*t10;
	    t5 = ss2*t13-ss3*t11-ss1*t9;	t6 = ss2*t14-ss3*t12-ss1*t10;
	    t7 = ss3*t13-ss1*t11+ss2*t9;	t8 = ss3*t14-ss1*t12+ss2*t10;

	    aj1p1r=rt+t4;			aj1p1i=it-t3;
	    aj1p2r=re+t6;			aj1p2i=im-t5;
	    aj1p3r=t1+t8;			aj1p3i=t2-t7;
	    aj1p4r=t1-t8;			aj1p4i=t2+t7;
	    aj1p5r=re-t6;			aj1p5i=im+t5;
	    aj1p6r=rt-t4;			aj1p6i=it+t3;
        */

	  aRe[0] = t1+t3+t5+t7;	        aIm[0] = t2+t4+t6+t8;	        /* X0	*/
	  rt = t1+cc1*t3+cc2*t5+cc3*t7;	it = t2+cc1*t4+cc2*t6+cc3*t8;	/* C1	*/
	  re = t1+cc2*t3+cc3*t5+cc1*t7;	im = t2+cc2*t4+cc3*t6+cc1*t8;	/* C2	*/
	  t1 = t1+cc3*t3+cc1*t5+cc2*t7;	t2 = t2+cc3*t4+cc1*t6+cc2*t8;	/* C3	*/

	  t3 = ss1*t13+ss2*t11+ss3*t9;	t4 = ss1*t14+ss2*t12+ss3*t10;	/* S1	*/
	  t5 = ss2*t13-ss3*t11-ss1*t9;	t6 = ss2*t14-ss3*t12-ss1*t10;	/* S2	*/
	  t7 = ss3*t13-ss1*t11+ss2*t9;	t8 = ss3*t14-ss1*t12+ss2*t10;	/* S3	*/

/*...Inline multiply of sine parts by +-I into finishing phase...	*/

	  aRe[1]=rt+t4;		aIm[1]=it-t3;	/* X1 = C1 - I*S1	*/
	  aRe[2]=re+t6;		aIm[2]=im-t5;	/* X2 = C2 - I*S2	*/
	  aRe[3]=t1+t8;		aIm[3]=t2-t7;	/* X3 = C3 - I*S3	*/
	  aRe[4]=t1-t8;		aIm[4]=t2+t7;	/* X4 =	C3 + I*S3	*/
	  aRe[5]=re-t6;		aIm[5]=im+t5;	/* X5 =	C2 + I*S2	*/
	  aRe[6]=rt-t4;		aIm[6]=it+t3;	/* X6 =	C1 + I*S1	*/
                                                                /* Totals: 60 FADD, 36 FMUL.	*/
}

/*
void FFT_JJN_fft_7_old ( _FFT_JJN_DATA *piData )
{
    FLOAT32  t1_re, t1_im, t2_re, t2_im, t3_re, t3_im;
    FLOAT32  t4_re, t4_im, t5_re, t5_im, t6_re, t6_im;

    FLOAT32  m1_re, m1_im, m2_re, m2_im, m3_re, m3_im;
    FLOAT32  m4_re, m4_im, m5_re, m5_im, m6_re, m6_im;

    t1_re = zRe[1] + zRe[6];    t1_im = zIm[1] + zIm[6];
    t2_re = zRe[1] - zRe[6];    t2_im = zIm[1] - zIm[6];
    t3_re = zRe[2] + zRe[5];    t3_im = zIm[2] + zIm[5];
    t4_re = zRe[2] - zRe[5];    t4_im = zIm[2] - zIm[5];
    t5_re = zRe[3] + zRe[4];    t5_im = zIm[3] + zIm[4];
    t6_re = zRe[3] - zRe[4];    t6_im = zIm[3] - zIm[4];

    //m1 = x0+c1*(x1+x6)+c2*(x2+x5)+c3*(x3+x4)
    m1_re = zRe[0] + (c7_cos1 * t1_re) + (c7_cos2 * t3_re) + (c7_cos3 * t5_re);
    m1_im = zIm[0] + (c7_cos1 * t1_im) + (c7_cos2 * t3_im) + (c7_cos3 * t5_im);

    //m2 = x0+c2*(x1+x6)+c3*(x2+x5)+c1*(x3+x4)
    m2_re = zRe[0] + (c7_cos2 * t1_re) + (c7_cos3 * t3_re) + (c7_cos1 * t5_re);
    m2_im = zIm[0] + (c7_cos2 * t1_im) + (c7_cos3 * t3_im) + (c7_cos1 * t5_im);

    //m3 = x0+c3*(x1+x6)+c1*(x2+x5)+c2*(x3+x4)
    m3_re = zRe[0] + (c7_cos3 * t1_re) + (c7_cos1 * t3_re) + (c7_cos2 * t5_re);
    m3_im = zIm[0] + (c7_cos3 * t1_im) + (c7_cos1 * t3_im) + (c7_cos2 * t5_im);

    //m4 =    s1*(x1-x6)+s2*(x2-x5)+s3*(x3-x4)
    m4_re =          (c7_sin1 * t2_re) + (c7_sin2 * t4_re) + (c7_sin3 * t6_re);
    m4_im =          (c7_sin1 * t2_im) + (c7_sin2 * t4_im) + (c7_sin3 * t6_im);

    //m5 =    s2*(x1-x6)-s3*(x2-x5)-s1*(x3-x4)
    m5_re =          (c7_sin2 * t2_re) - (c7_sin3 * t4_re) - (c7_sin1 * t6_re);
    m5_im =          (c7_sin2 * t2_im) - (c7_sin3 * t4_im) - (c7_sin1 * t6_im);

    //m6 =    s3*(x1-x6)-s1*(x2-x5)+s2*(x3-x4)
    m6_re =          (c7_sin3 * t2_re) - (c7_sin1 * t4_re) + (c7_sin2 * t6_re);
    m6_im =          (c7_sin3 * t2_im) - (c7_sin1 * t4_im) + (c7_sin2 * t6_im);

    zRe[0] += t1_re + t3_re + t5_re;
    zIm[0] += t1_im + t3_im + t5_im;

    zRe[1] = m1_re + m4_re;     zIm[1] = m1_im - m4_im;
    zRe[2] = m2_re + m5_re;     zIm[2] = m2_im - m5_im;
    zRe[3] = m3_re + m6_re;     zIm[3] = m3_im - m6_im;
    zRe[4] = m3_re - m6_re;     zIm[4] = m3_im + m6_im;
    zRe[5] = m2_re - m5_re;     zIm[5] = m2_im + m5_im;
    zRe[6] = m1_re - m4_re;     zIm[6] = m1_im + m4_im;

    



    /*


    //Calcul de zRe[0]
    zRe[0] = aRe[0] + aRe[1] + aRe[2] + aRe[3] + aRe[4] + aRe[5] + aRe[6];
    zIm[0] = aIm[0] + aIm[1] + aIm[2] + aIm[3] + aIm[4] + aIm[5] + aIm[6];



    vRe[1] = zRe[1] + zRe[6];    vIm[1] = zIm[1] - zIm[6];
    wRe[1] = zRe[1] - zRe[6];    wIm[1] = zIm[1] + zIm[6];
    vRe[2] = zRe[2] + zRe[5];    vIm[2] = zIm[2] - zIm[5];
    wRe[2] = zRe[2] - zRe[5];    wIm[2] = zIm[2] + zIm[5];
    vRe[3] = zRe[3] + zRe[4];    vIm[3] = zIm[3] - zIm[4];
    wRe[3] = zRe[3] - zRe[4];    wIm[3] = zIm[3] + zIm[4];

    zRe[1] = zRe[0];    zIm[1] = zIm[0];
    zRe[6] = zRe[0];    zIm[6] = zIm[0];

    //********************************
    //j = 1; i = 1; k = 1;
    rere = trigRe[1] * vRe[1];   imim = trigIm[1] * vIm[1];
    reim = trigRe[1] * wIm[1];   imre = trigIm[1] * wRe[1];
    zRe[6] += rere + imim;
    zIm[6] += reim - imre;
    zRe[1] += rere - imim;
    zIm[1] += reim + imre;

    //j = 1; i = 2; k = 2;
    rere = trigRe[2] * vRe[2];   imim = trigIm[2] * vIm[2];
    reim = trigRe[2] * wIm[2];   imre = trigIm[2] * wRe[2];
    zRe[6] += rere + imim;
    zIm[6] += reim - imre;
    zRe[1] += rere - imim;
    zIm[1] += reim + imre;

    //j = 1; i = 3; k = 3;
    rere = trigRe[3] * vRe[3];   imim = trigIm[3] * vIm[3];
    reim = trigRe[3] * wIm[3];   imre = trigIm[3] * wRe[3];
    zRe[6] += rere + imim;
    zIm[6] += reim - imre;
    zRe[1] += rere - imim;
    zIm[1] += reim + imre;

    //********************************
    //j = 2; i = 1; k = 2;
    //            k        i                   k        i
    rere = trigRe[2] * vRe[1];   imim = trigIm[2] * vIm[1];
    reim = trigRe[2] * wIm[1];   imre = trigIm[2] * wRe[1];
    zRe[5] += rere + imim;
    zIm[5] += reim - imre;
    zRe[2] += rere - imim;
    zIm[2] += reim + imre;

    //j = 2; i = 2; k = 4;    
    rere = trigRe[4] * vRe[2];   imim = trigIm[4] * vIm[2];
    reim = trigRe[4] * wIm[2];   imre = trigIm[4] * wRe[2];
    zRe[5] += rere + imim;
    zIm[5] += reim - imre;
    zRe[2] += rere - imim;
    zIm[2] += reim + imre;

    //j = 2; i = 3; k = 6;
    rere = trigRe[6] * vRe[3];   imim = trigIm[6] * vIm[3];
    reim = trigRe[6] * wIm[3];   imre = trigIm[6] * wRe[3];
    zRe[5] += rere + imim;
    zIm[5] += reim - imre;
    zRe[2] += rere - imim;
    zIm[2] += reim + imre;
    */
    
//}
