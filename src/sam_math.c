#include "sam_header.h"

long SAM_MATH_lroundf ( float f )
{
    float f1, f2;

    f1 = (float) floor ( f );
    f2 = f - f1;
    if (f2>=0.5F) f1++;
    return (long)f1;
}

long SAM_MATH_lroundd ( double d )
{
    double d1, d2;
                                //f  = 2.54      -3.41
    d1 = (double) floor ( d );   //f1 = 2         -3               
    d2 = d - d1;                //f2 = 0.54
    if (d2>=0.5) d1++;         //f1 = 3
    return (long)d1;
}

long SAM_MATH_log2 ( DWORD dwValue )
{
    DWORD dwKept;
    long lBitCount;
    
    dwKept = dwValue;
    lBitCount = 0;
    do {
        dwKept = dwKept>>1;
        lBitCount++;
    } while (dwKept);
    
    return lBitCount;
}