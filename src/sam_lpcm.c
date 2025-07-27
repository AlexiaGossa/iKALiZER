/*

        

*/


#include "sam_header.h"

FLOAT32     f32LPCM_Decode16;


void        SAM_LPCM_Init           ( void );
WORD        SAM_LPCM_EncodeValue16  ( FLOAT32 f32AbsoluteValue );
FLOAT32     SAM_LPCM_DecodeValue16  ( DWORD dwLPCMValue, long lEntryValueUsed );




WORD        SAM_LPCM_EncodeValue16 ( FLOAT32 f32AbsoluteValue )
{
    FLOAT32 fValue;
    INT16   iValue;
    fValue = f32AbsoluteValue*32767;
    if (fValue> 32767) fValue =  32767;
    if (fValue<-32767) fValue = -32767;
    
    iValue = (INT16)fValue;
    
    return (WORD)iValue;
}
    
FLOAT32     SAM_LPCM_DecodeValue16  ( DWORD dwLPCMValue, long lEntryValueUsed )
{
    FLOAT32 f32Return;
    
    __asm {
        push ecx
        mov eax, dwLPCMValue
        mov ecx, lEntryValueUsed
        shl ecx, 4
        shr eax, cl
        movsx ecx, ax
        mov f32Return, ecx
        fild dword ptr f32Return
        fmul dword ptr f32LPCM_Decode16
        fstp f32Return
        pop ecx
    }
    
    //f32Return = dwLPCMValue;
    return f32Return;// * f32LPCM_Decode16;
}



void        SAM_LPCM_Init ( void )
{
    f32LPCM_Decode16 = 1.0F / 32767.0F;
}


