/*
    (c) Alexia Gossa
*/

    #include "AudioTools.h"

    void Scalar_MulAndUpdate_f32 ( long lNbPoints, float *pf32ValueList, float f32Mul );
    void Scalar_MulAndUpdate_f64 ( long lNbPoints, double *pf64ValueList, double f64Mul );
    void Scalar_MulToNew_f32 ( long lNbPoints, float *pf32InValueList, float *pf32OutValueList, float f32Mul );
    void Scalar_MulToNew_f64 ( long lNbPoints, double *pf64InValueList, double *pf64OutValueList, double f64Mul );

    void Scalar_MulAndUpdate_f32 ( long lNbPoints, float *pf32ValueList, float f32Mul )
    {
        long i;
        for (i=0;i<lNbPoints;i++) pf32ValueList[i] *= f32Mul;
    }

    void Scalar_MulToNew_f32 ( long lNbPoints, float *pf32InValueList, float *pf32OutValueList, float f32Mul )
    {
        long i;
        for (i=0;i<lNbPoints;i++) pf32OutValueList[i] = pf32InValueList[i] * f32Mul;
    }

    void Scalar_MulAndUpdate_f64 ( long lNbPoints, double *pf64ValueList, double f64Mul )
    {
        long i;
        for (i=0;i<lNbPoints;i++) pf64ValueList[i] *= f64Mul;
    }

    void Scalar_MulToNew_f64 ( long lNbPoints, double *pf64InValueList, double *pf64OutValueList, double f64Mul )
    {
        long i;
        for (i=0;i<lNbPoints;i++) pf64OutValueList[i] = pf64InValueList[i] * f64Mul;
    }
