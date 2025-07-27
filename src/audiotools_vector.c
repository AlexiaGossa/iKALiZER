/*
    (c) Alexia Gossa
*/

    #include "AudioTools.h"

    void Vector_MulAndUpdate_f32 ( long lNbPoints, float *pf32ValueList, float *pf32MulList );
    void Vector_MulAndUpdate_f64 ( long lNbPoints, double *pf64ValueList, double *pf64MulList );
    void Vector_MulToNew_f32 ( long lNbPoints, float *pf32InValueList, float *pf32OutValueList, float *pf32MulList );
    void Vector_MulToNew_f64 ( long lNbPoints, double *pf64InValueList, double *pf64OutValueList, double *pf64MulList );

    void Vector_MulAndUpdate_f32 ( long lNbPoints, float *pf32ValueList, float *pf32MulList )
    {
        long i;
        for (i=0;i<lNbPoints;i++) pf32ValueList[i] *= pf32MulList[i];
    }

    void Vector_MulToNew_f32 ( long lNbPoints, float *pf32InValueList, float *pf32OutValueList, float *pf32MulList )
    {
        long i;
        for (i=0;i<lNbPoints;i++) pf32OutValueList[i] = pf32InValueList[i] * pf32MulList[i];
    }

    void Vector_MulAddToNew_f32 ( long lNbPoints, float *pf32InValueList, float *pf32OutValueList, float *pf32MulList )
    {
        long i;
        for (i=0;i<lNbPoints;i++) pf32OutValueList[i] += pf32InValueList[i] * pf32MulList[i];
    }

    void Vector_MulAndUpdate_f64 ( long lNbPoints, double *pf64ValueList, double *pf64MulList )
    {
        long i;
        for (i=0;i<lNbPoints;i++) pf64ValueList[i] *= pf64MulList[i];
    }

    void Vector_MulToNew_f64 ( long lNbPoints, double *pf64InValueList, double *pf64OutValueList, double *pf64MulList )
    {
        long i;
        for (i=0;i<lNbPoints;i++) pf64OutValueList[i] = pf64InValueList[i] * pf64MulList[i];
    }

    void Vector_MulAddToNew_f64 ( long lNbPoints, double *pf64InValueList, double *pf64OutValueList, double *pf64MulList )
    {
        long i;
        for (i=0;i<lNbPoints;i++) pf64OutValueList[i] += pf64InValueList[i] * pf64MulList[i];
    }

    void Vector_AddAndUpdate_f32 ( long lNbPoints, float *pf32InOutValueList, float *pf32AddList )
    {
        long i;
        for (i=0;i<lNbPoints;i++) pf32InOutValueList[i] += pf32AddList[i];
    }

    void Vector_AddToNew_f32 ( long lNbPoints, float *pf32InValueList, float *pf32OutValueList, float *pf32AddList )
    {
        long i;
        for (i=0;i<lNbPoints;i++) pf32OutValueList[i] = pf32InValueList[i] + pf32AddList[i];
    }

    void Vector_AddAndUpdate_f64 ( long lNbPoints, double *pf64InOutValueList, double *pf64AddList )
    {
        long i;
        for (i=0;i<lNbPoints;i++) pf64InOutValueList[i] += pf64AddList[i];
    }

    void Vector_AddToNew_f64 ( long lNbPoints, double *pf64InValueList, double *pf64OutValueList, float *pf64AddList )
    {
        long i;
        for (i=0;i<lNbPoints;i++) pf64OutValueList[i] = pf64InValueList[i] + pf64AddList[i];
    }
