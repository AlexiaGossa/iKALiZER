#include "audiotools_fft_jjn.h"

static FLOAT32  c5_1 = -1.2500000000000E+00F;  /*  c5_1 = (cos(u5)+cos(2*u5))/2-1;*/
static FLOAT32  c5_2 =  5.5901699437495E-01F;  /*  c5_2 = (cos(u5)-cos(2*u5))/2;  */
static FLOAT32  c5_3 = -9.5105651629515E-01F;  /*  c5_3 = -sin(u5);               */
static FLOAT32  c5_4 = -1.5388417685876E+00F;  /*  c5_4 = -(sin(u5)+sin(2*u5));   */
static FLOAT32  c5_5 =  3.6327126400268E-01F;  /*  c5_5 = (sin(u5)-sin(2*u5));    */

static FLOAT32  c5_cx1  = -1.25000000000000000000F;               /* [cos(u)+cos(2u)]/2-1 = -5/4 */
static FLOAT32  c5_cx2  =  0.55901699437494742409F;               /* [cos(u)-cos(2u)]/2 */
static FLOAT32  c5_s1   =  0.95105651629515357211F;               /*  sin(u) */
static FLOAT32  c5_sx1  =  1.53884176858762670130F;               /* [sin(u)+sin(2u)] */
static FLOAT32  c5_sx2  =  0.36327126400268044292F;               /* [sin(u)-sin(2u)] */

void FFT_JJN_radix5 ( FLOAT32 *aRe, FLOAT32 *aIm )
{
	FLOAT32 rt,it,t1,t2,t3,t4,t5,t6,t7,t8,t9,t10;

    t1 =aRe[0];		            t2 =aIm[0];			    /** x0					x0	*/
    t3 =aRe[1]+aRe[4];	        t4 =aIm[1]+aIm[4];	    /** x1 + x4				t1	*/
    t5 =aRe[2]+aRe[3];	        t6 =aIm[2]+aIm[3];	    /** x2 + x3				t2	*/
    t7 =aRe[2]-aRe[3];	        t8 =aIm[2]-aIm[3];	    /** x2 - x3				-t4	*/
    t9 =aRe[1]-aRe[4];	        t10=aIm[1]-aIm[4];	    /** x1 - x4				t3	*/

    rt = t3+t5;			        it = t4+t6;			    /**    x1+x2+x3+x4			t1+t2=t5	*/
    t1 = t1+rt;			        t2 = t2+it;			    /** x0+x1+x2+x3+x4			x0+t5=m0	*/

    rt = t1+c5_cx1*rt;          it = t2+c5_cx1*it;		/** A   = x0+(x1+x2+x3+x4)*[c1+c2]/2	m0+m1=s1	*/
    t5 = c5_cx2*(t3-t5);	    t6 = c5_cx2*(t4-t6);	/** B   =    (x1-x2-x3+x4)*[c1-c2]/2	cc2*(t1-t2)=m2	*/

    t3 = rt+t5;			        t4 = it+t6;			    /** A+B = x0+c1*(x1+x4)+c2*(x2+x3)	s1+m2=s2	*/
    t5 = rt-t5;			        t6 = it-t6;			    /** A-B = x0+c2*(x1+x4)+c1*(x2+x3)	s1-m2=s4	*/

    rt = c5_s1*(t7-t9);	        it = c5_s1*(t8-t10);	/** C' = (x1-x2+x3-x4)* s1		s*(t3+t4)=-Im(m3)	*/
    t7 = c5_sx1* t7;		    t8 = c5_sx1* t8;		/** D' = (   x2-x3   )*[s1+s2]		ss1*(-t4)= Im(m4)	*/
    t9 = c5_sx2* t9;		    t10= c5_sx2* t10;		/** E' = (x1      -x4)*[s1-s2],		ss2*( t3)= Im(m5)	*/

    t7 = rt-t7;			        t8 = it-t8;			    /** C'+D'   = s1*(x1-x4)+s2*(x2-x3)	-Im(m3)+Im(m4)=-Im(s3)	*/
    t9 = rt+t9;			        t10= it+t10;		    /** C'-E'   = s2*(x1-x4)-s1*(x2-x3)	-Im(m3)-Im(m4)=-Im(s5)	*/


    aRe[0]=t1;			        aIm[0]=t2;			    /** X0, first output datum		m0	*/
    aRe[1]=t3-t8;		        aIm[1]=t4+t7;		    /** X1	=				s2+s3	*/
    aRe[2]=t5-t10;		        aIm[2]=t6+t9;		    /** X2	=				s4+s5	*/
    aRe[3]=t5+t10;		        aIm[3]=t6-t9;		    /** X3	=				s4-s5	*/
    aRe[4]=t3+t8;		        aIm[4]=t4-t7;		    /** X4	=				s2-s3	*/

}

void FFT_JJN_radix5_OutInterleaved ( FLOAT32 IN *aRe, FLOAT32 IN *aIm, FLOAT32 OUT *zRe, FLOAT32 OUT *zIm, long OIl )
{
	FLOAT32 rt,it,t1,t2,t3,t4,t5,t6,t7,t8,t9,t10;
    long lOIl2, lOIl3, lOIl4;

    lOIl2 = OIl<<1;
    lOIl3 = lOIl2 + OIl;
    lOIl4 = OIl<<2;

    t1 =aRe[0];		            t2 =aIm[0];			    /** x0					x0	*/
    t3 =aRe[1]+aRe[4];	        t4 =aIm[1]+aIm[4];	    /** x1 + x4				t1	*/
    t5 =aRe[2]+aRe[3];	        t6 =aIm[2]+aIm[3];	    /** x2 + x3				t2	*/
    t7 =aRe[2]-aRe[3];	        t8 =aIm[2]-aIm[3];	    /** x2 - x3				-t4	*/
    t9 =aRe[1]-aRe[4];	        t10=aIm[1]-aIm[4];	    /** x1 - x4				t3	*/

    rt = t3+t5;			        it = t4+t6;			    /**    x1+x2+x3+x4			t1+t2=t5	*/
    t1 = t1+rt;			        t2 = t2+it;			    /** x0+x1+x2+x3+x4			x0+t5=m0	*/

    rt = t1+c5_cx1*rt;          it = t2+c5_cx1*it;		/** A   = x0+(x1+x2+x3+x4)*[c1+c2]/2	m0+m1=s1	*/
    t5 = c5_cx2*(t3-t5);	    t6 = c5_cx2*(t4-t6);	/** B   =    (x1-x2-x3+x4)*[c1-c2]/2	cc2*(t1-t2)=m2	*/

    t3 = rt+t5;			        t4 = it+t6;			    /** A+B = x0+c1*(x1+x4)+c2*(x2+x3)	s1+m2=s2	*/
    t5 = rt-t5;			        t6 = it-t6;			    /** A-B = x0+c2*(x1+x4)+c1*(x2+x3)	s1-m2=s4	*/

    rt = c5_s1*(t7-t9);	        it = c5_s1*(t8-t10);	/** C' = (x1-x2+x3-x4)* s1		s*(t3+t4)=-Im(m3)	*/
    t7 = c5_sx1* t7;		    t8 = c5_sx1* t8;		/** D' = (   x2-x3   )*[s1+s2]		ss1*(-t4)= Im(m4)	*/
    t9 = c5_sx2* t9;		    t10= c5_sx2* t10;		/** E' = (x1      -x4)*[s1-s2],		ss2*( t3)= Im(m5)	*/

    t7 = rt-t7;			        t8 = it-t8;			    /** C'+D'   = s1*(x1-x4)+s2*(x2-x3)	-Im(m3)+Im(m4)=-Im(s3)	*/
    t9 = rt+t9;			        t10= it+t10;		    /** C'-E'   = s2*(x1-x4)-s1*(x2-x3)	-Im(m3)-Im(m4)=-Im(s5)	*/

    zRe[0]      = t1;			zIm[0]      = t2;			    /** X0, first output datum		m0	*/
    zRe[OIl]    = t3-t8;		zIm[OIl]    = t4+t7;		    /** X1	=				s2+s3	*/
    zRe[lOIl2]  = t5-t10;		zIm[lOIl2]  = t6+t9;		    /** X2	=				s4+s5	*/
    zRe[lOIl3]  = t5+t10;		zIm[lOIl3]  = t6-t9;		    /** X3	=				s4-s5	*/
    zRe[lOIl4]  = t3+t8;		zIm[lOIl4]  = t4-t7;		    /** X4	=				s2-s3	*/
}

void FFT_JJN_radix5_OutInterleavedFast ( FLOAT32 IN *aReIm, FLOAT32 OUT *zRe, FLOAT32 OUT *zIm, long OIl )
{
	FLOAT32 rt,it,t1,t2,t3,t4,t5,t6,t7,t8,t9,t10;
    long lOIl2, lOIl3, lOIl4;

    lOIl2 = OIl<<1;
    lOIl3 = lOIl2 + OIl;
    lOIl4 = OIl<<2;

    t1 =aReIm[0];		        t2 =aReIm[1];			    /** x0					x0	*/
    t3 =aReIm[2]+aReIm[8];	    t4 =aReIm[3]+aReIm[9];	    /** x1 + x4				t1	*/
    t5 =aReIm[4]+aReIm[6];	    t6 =aReIm[5]+aReIm[7];	    /** x2 + x3				t2	*/
    t7 =aReIm[4]-aReIm[6];	    t8 =aReIm[5]-aReIm[7];	    /** x2 - x3				-t4	*/
    t9 =aReIm[2]-aReIm[8];	    t10=aReIm[3]-aReIm[9];	    /** x1 - x4				t3	*/

    rt = t3+t5;			        it = t4+t6;			    /**    x1+x2+x3+x4			t1+t2=t5	*/
    t1 = t1+rt;			        t2 = t2+it;			    /** x0+x1+x2+x3+x4			x0+t5=m0	*/

    rt = t1+c5_cx1*rt;          it = t2+c5_cx1*it;		/** A   = x0+(x1+x2+x3+x4)*[c1+c2]/2	m0+m1=s1	*/
    t5 = c5_cx2*(t3-t5);	    t6 = c5_cx2*(t4-t6);	/** B   =    (x1-x2-x3+x4)*[c1-c2]/2	cc2*(t1-t2)=m2	*/

    t3 = rt+t5;			        t4 = it+t6;			    /** A+B = x0+c1*(x1+x4)+c2*(x2+x3)	s1+m2=s2	*/
    t5 = rt-t5;			        t6 = it-t6;			    /** A-B = x0+c2*(x1+x4)+c1*(x2+x3)	s1-m2=s4	*/

    rt = c5_s1*(t7-t9);	        it = c5_s1*(t8-t10);	/** C' = (x1-x2+x3-x4)* s1		s*(t3+t4)=-Im(m3)	*/
    t7 = c5_sx1* t7;		    t8 = c5_sx1* t8;		/** D' = (   x2-x3   )*[s1+s2]		ss1*(-t4)= Im(m4)	*/
    t9 = c5_sx2* t9;		    t10= c5_sx2* t10;		/** E' = (x1      -x4)*[s1-s2],		ss2*( t3)= Im(m5)	*/

    t7 = rt-t7;			        t8 = it-t8;			    /** C'+D'   = s1*(x1-x4)+s2*(x2-x3)	-Im(m3)+Im(m4)=-Im(s3)	*/
    t9 = rt+t9;			        t10= it+t10;		    /** C'-E'   = s2*(x1-x4)-s1*(x2-x3)	-Im(m3)-Im(m4)=-Im(s5)	*/

    zRe[0]      = t1;			zIm[0]      = t2;			    /** X0, first output datum		m0	*/
    zRe[OIl]    = t3-t8;		zIm[OIl]    = t4+t7;		    /** X1	=				s2+s3	*/
    zRe[lOIl2]  = t5-t10;		zIm[lOIl2]  = t6+t9;		    /** X2	=				s4+s5	*/
    zRe[lOIl3]  = t5+t10;		zIm[lOIl3]  = t6-t9;		    /** X3	=				s4-s5	*/
    zRe[lOIl4]  = t3+t8;		zIm[lOIl4]  = t4-t7;		    /** X4	=				s2-s3	*/
}

void FFT_JJN_fft_5 ( FLOAT32 aRe[], FLOAT32 aIm[])
{
static FLOAT32 cc1 = -1.25000000000000000000F,	/* [cos(u)+cos(2u)]/2-1 = -5/4 */
		      cc2 =  0.55901699437494742409F,	/* [cos(u)-cos(2u)]/2 */
		      s   =  0.95105651629515357211F,	/*  sin(u) */
		      ss1 =  1.53884176858762670130F,	/* [sin(u)+sin(2u)] */
		      ss2 =  0.36327126400268044292F;	/* [sin(u)-sin(2u)] */
	FLOAT32 rt,it
		,t1,t2,t3,t4,t5,t6,t7,t8,t9,t10;

      /*
	  t1_re =aRe[1]+aRe[4];	        t1_im =aIm[1]+aIm[4];	// ** x1 + x4				t1
	  t2_re =aRe[2]+aRe[3];	        t2_im =aIm[2]+aIm[3];	// ** x2 + x3				t2
	  t3_re =aRe[2]-aRe[3];	        t3_im =aIm[2]-aIm[3];	// * x2 - x3				-t4
	  t4_re =aRe[1]-aRe[4];	        t4_im =aIm[1]-aIm[4];	// * x1 - x4				t3	

      //rt = t3+t5;			it = t4+t6;			// **    x1+x2+x3+x4			t1+t2=t5	
      t5_re =t1_re+t2_re;           t5_im =t1_im+t2_im;

      //t1 = t1+rt;			t2 = t2+it;			// ** x0+x1+x2+x3+x4			x0+t5=m0	
      aRe[0] = aRe[0] + t5_re;      aIm[0] = aIm[0] + t5_im;

      //rt = t1+cc1*rt;		it = t2+cc1*it;		// ** A   = x0+(x1+x2+x3+x4)*[c1+c2]/2	m0+m1=s1	
      m1_re = cc1*t5_re;            m1_im = cc1*t5_im;
      s1_re = aRe[0] + m1_re;       s1_im = aIm[0] + m1_im;

      //t5 = cc2*(t3-t5);		t6 = cc2*(t4-t6);	/** B   =    (x1-x2-x3+x4)*[c1-c2]/2	cc2*(t1-t2)=m2	
      m2_re = cc2*(t1_re-t2_re);    m2_im = cc2*(t1_im-t2_im);

      //rt = s  *(t9-t7);		it = s  *(t10-t8);	/** C' = (x1-x2+x3-x4)* s1		s*(t3+t4)=-Im(m3)	
      m3_re = s*(t4_re-t3_re);      m3_im = s*(t4_im-t3_im);

      //t7 = ss1* t7;			t8 = ss1* t8;		/** D' = (   x2-x3   )*[s1+s2]		ss1*(-t4)= Im(m4)	
      m4_re = ss1*t3_re;            m4_im = ss1*t3_im;

      //t9 = ss2* t9;			t10= ss2* t10;		/** E' = (x1      -x4)*[s1-s2],		ss2*( t3)= Im(m5)	
      m5_re = ss2*t4_re;            m5_im = ss2*t4_im;

	  //t3 = rt+t5;			t4 = it+t6;			/** A+B = x0+c1*(x1+x4)+c2*(x2+x3)	s1+m2=s2	
	  //t5 = rt-t5;			t6 = it-t6;			/** A-B = x0+c2*(x1+x4)+c1*(x2+x3)	s1-m2=s4	
      s2_re = s1_re + m2_re;        s2_im = s1_im + m2_im;
      s4_re = s1_re - m2_re;        s4_im = s1_im - m2_im;

	  //t7 = rt+t7;			t8 = it+t8;			/** C'+D'   = s1*(x1-x4)+s2*(x2-x3)	-Im(m3)+Im(m4)=-Im(s3)	
	  //t9 = rt-t9;			t10= it-t10;		/** C'-E'   = s2*(x1-x4)-s1*(x2-x3)	-Im(m3)-Im(m4)=-Im(s5)	
      s3_re = m3_re+m4_re;          s3_im = m3_im+m4_im;
      s5_re = m3_re-m5_re;          s5_im = m3_im-m5_im;

      aRe[1] = s2_re - s3_im;       aIm[1] = s2_im + s3_re;
      aRe[2] = s4_re - s5_im;       aIm[2] = s4_im + s5_re;
      aRe[3] = s4_re + s5_im;       aIm[3] = s4_im - s5_re;
      aRe[4] = s2_re + s3_im;       aIm[1] = s2_im - s3_re;
      */

/*       gather the needed data (5 64-bit complex, i.e. 10 64-bit reals) and begin the transform...	*/
									            /** Quantity				= Nussbaumer's	*/
	  t1 =aRe[0];		    t2 =aIm[0];			/** x0					x0	*/
	  t3 =aRe[1]+aRe[4];	t4 =aIm[1]+aIm[4];	/** x1 + x4				t1	*/
	  t5 =aRe[2]+aRe[3];	t6 =aIm[2]+aIm[3];	/** x2 + x3				t2	*/
	  t7 =aRe[2]-aRe[3];	t8 =aIm[2]-aIm[3];	/** x2 - x3				-t4	*/
	  t9 =aRe[1]-aRe[4];	t10=aIm[1]-aIm[4];	/** x1 - x4				t3	*/

    
      

/*       ...now complete the first complex datum of the result.	*/

	  rt = t3+t5;			it = t4+t6;			/**    x1+x2+x3+x4			t1+t2=t5	*/
	  t1 = t1+rt;			t2 = t2+it;			/** x0+x1+x2+x3+x4			x0+t5=m0	*/

	  rt = t1+cc1*rt;		it = t2+cc1*it;		/** A   = x0+(x1+x2+x3+x4)*[c1+c2]/2	m0+m1=s1	*/
	  t5 = cc2*(t3-t5);		t6 = cc2*(t4-t6);	/** B   =    (x1-x2-x3+x4)*[c1-c2]/2	cc2*(t1-t2)=m2	*/

	  t3 = rt+t5;			t4 = it+t6;			/** A+B = x0+c1*(x1+x4)+c2*(x2+x3)	s1+m2=s2	*/
	  t5 = rt-t5;			t6 = it-t6;			/** A-B = x0+c2*(x1+x4)+c1*(x2+x3)	s1-m2=s4	*/

	  rt = s  *(t7-t9);		it = s  *(t8-t10);	/** C' = (x1-x2+x3-x4)* s1		s*(t3+t4)=-Im(m3)	*/
	  t7 = ss1* t7;			t8 = ss1* t8;		/** D' = (   x2-x3   )*[s1+s2]		ss1*(-t4)= Im(m4)	*/
	  t9 = ss2* t9;			t10= ss2* t10;		/** E' = (x1      -x4)*[s1-s2],		ss2*( t3)= Im(m5)	*/

	  t7 = rt-t7;			t8 = it-t8;			/** C'+D'   = s1*(x1-x4)+s2*(x2-x3)	-Im(m3)+Im(m4)=-Im(s3)	*/
	  t9 = rt+t9;			t10= it+t10;		/** C'-E'   = s2*(x1-x4)-s1*(x2-x3)	-Im(m3)-Im(m4)=-Im(s5)	*/

/*...Inline multiply of sine part by I into finishing phase...	*/

	  aRe[0]=t1;			aIm[0]=t2;			/** X0, first output datum		m0	*/
	  aRe[1]=t3-t8;		    aIm[1]=t4+t7;		/** X1	=				s2+s3	*/
	  aRe[2]=t5-t10;		aIm[2]=t6+t9;		/** X2	=				s4+s5	*/
	  aRe[3]=t5+t10;		aIm[3]=t6-t9;		/** X3	=				s4-s5	*/
	  aRe[4]=t3+t8;		    aIm[4]=t4-t7;		/** X4	=				s2-s3	*/
}


/*
void FFT_JJN_fft_5_old ( FLOAT32 aRe[], FLOAT32 aIm[],_FFT_JJN_DATA *piData)
{    
  FLOAT32  t1_re,t1_im, t2_re,t2_im, t3_re,t3_im;
  FLOAT32  t4_re,t4_im, t5_re,t5_im;
  FLOAT32  m2_re,m2_im, m3_re,m3_im, m4_re,m4_im;
  FLOAT32  m1_re,m1_im, m5_re,m5_im;
  FLOAT32  s1_re,s1_im, s2_re,s2_im, s3_re,s3_im;
  FLOAT32  s4_re,s4_im, s5_re,s5_im;

  t1_re=aRe[1] + aRe[4];    t1_im=aIm[1] + aIm[4];
  t2_re=aRe[2] + aRe[3];    t2_im=aIm[2] + aIm[3];
  t3_re=aRe[1] - aRe[4];    t3_im=aIm[1] - aIm[4];
  t4_re=aRe[3] - aRe[2];    t4_im=aIm[3] - aIm[2];
  t5_re=t1_re + t2_re;      t5_im=t1_im + t2_im;
  aRe[0]=aRe[0] + t5_re;    aIm[0]=aIm[0] + t5_im;
  m1_re=c5_1*t5_re; m1_im=c5_1*t5_im;
  m2_re=c5_2*(t1_re - t2_re); m2_im=c5_2*(t1_im - t2_im);

  m3_re=-c5_3*(t3_im + t4_im); m3_im=c5_3*(t3_re + t4_re);
  m4_re=-c5_4*t4_im; m4_im=c5_4*t4_re;
  m5_re=-c5_5*t3_im; m5_im=c5_5*t3_re;

  s3_re=m3_re - m4_re; s3_im=m3_im - m4_im;
  s5_re=m3_re + m5_re; s5_im=m3_im + m5_im;
  s1_re=aRe[0] + m1_re; s1_im=aIm[0] + m1_im;
  s2_re=s1_re + m2_re; s2_im=s1_im + m2_im;
  s4_re=s1_re - m2_re; s4_im=s1_im - m2_im;

  aRe[1]=s2_re + s3_re; aIm[1]=s2_im + s3_im;
  aRe[2]=s4_re + s5_re; aIm[2]=s4_im + s5_im;
  aRe[3]=s4_re - s5_re; aIm[3]=s4_im - s5_im;
  aRe[4]=s2_re - s3_re; aIm[4]=s2_im - s3_im;
}   // fft_5
*/
