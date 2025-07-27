#include "ca_adler.h"

    void Diagnostic_GetCRC_C ( unsigned char * pData, long lSize, unsigned long * pdwCRC );
    void Diagnostic_GetCRC_ALU ( unsigned char * pData, long lSize, unsigned long * pdwCRC );
    
    

    void CA_GetAdler ( unsigned char * pData, long lSize, unsigned long * pdwCRC )
    {
        //Diagnostic_GetCRC_C ( pData, lSize, pdwCRC );
        Diagnostic_GetCRC_ALU ( pData, lSize, pdwCRC );
    }

    /*
        Algorithme ADLER de calcul du CRC
    */
    #define MOD_ADLER 65521
    void Diagnostic_GetCRC_C ( unsigned char * pData, long lSize, unsigned long * pdwCRC )
    {
        unsigned long a, b;
        unsigned long tlen;
        unsigned char *data;

        data = pData;
        
        a = 0;
        b = 0;

        while (lSize) 
        {
            tlen = lSize > 5550 ? 5550 : lSize;
            lSize -= tlen;
            do {
                a += *data++;
                b += a;
            } while (--tlen);
            a = (a & 0xffff) + (a >> 16) * (65536-MOD_ADLER);
            b = (b & 0xffff) + (b >> 16) * (65536-MOD_ADLER);
        }

        /* It can be shown that a <= 0x1013a here, so a single subtract will do. */
        if (a >= MOD_ADLER)
            a -= MOD_ADLER;
        /* It can be shown that b can reach 0xffef1 here. */
        b = (b & 0xffff) + (b >> 16) * (65536-MOD_ADLER);
        if (b >= MOD_ADLER)
            b -= MOD_ADLER;

        *pdwCRC = (b << 16) | a;
    }

    void Diagnostic_GetCRC_ALU ( unsigned char * pData, long lSize, unsigned long * pdwCRC )
    {
        unsigned long a, b;
        unsigned long const5500;
        unsigned long const64KMA;

        const5500   = 5500;
        const64KMA  = 65536-MOD_ADLER;

        __asm {
                    push    ebx

                    xor     eax, eax    //a
                    xor     ebx, ebx    //b
                    mov     esi, pData
                    xor     ecx, ecx

                _loop_Bloc:

                    mov     edi, lSize
                    cmp     edi, const5500
                    cmova   edi, const5500
                    sub     lSize, edi

                _loop_Data:

                    mov     cl, [esi]
                    inc     esi
                    add     eax, ecx
                    add     ebx, eax

                    dec     edi
                    jnz     _loop_Data

                    mov     edi, eax
                    mov     edx, ebx

                    shr     edi, 16
                    and     eax, 0xFFFF

                    shr     edx, 16
                    and     ebx, 0xFFFF

                    imul    edi, const64KMA
                    imul    edx, const64KMA

                    add     eax, edi
                    add     ebx, edx

                    cmp     lSize, 0
                    jne     _loop_Bloc


                    mov     a, eax
                    mov     b, ebx

                    pop     ebx
        }

        /* It can be shown that a <= 0x1013a here, so a single subtract will do. */
        if (a >= MOD_ADLER)
            a -= MOD_ADLER;
        /* It can be shown that b can reach 0xffef1 here. */
        b = (b & 0xffff) + (b >> 16) * (65536-MOD_ADLER);
        if (b >= MOD_ADLER)
            b -= MOD_ADLER;

        *pdwCRC = (b << 16) | a;
    }
