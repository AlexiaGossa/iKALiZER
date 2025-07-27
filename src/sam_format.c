#include "sam_header.h"

BYTE    SAM_FormatGetBytesCount_bTable[256];
long    SAM_FormatGetBytesCount_lInit = 0;

long    SAM_FormatGetBytesCount ( BYTE bFormat, DWORD * pdwBytesCount )
{
    if (!SAM_FormatGetBytesCount_lInit)
    {
        DWORD dwBytesPerSample;
        DWORD dwIndex;
        
        SAM_FormatGetBytesCount_lInit = 1;
        
        for (dwIndex=0;dwIndex<256;dwIndex++)
        {
            dwBytesPerSample = 1;
            switch (dwIndex)
            {
                case sam_FORMAT_MONO_FLOAT32:       dwBytesPerSample = 4; break;
                case sam_FORMAT_MONO_XPCM8:         dwBytesPerSample = 1; break;
                case sam_FORMAT_MONO_PCM8:          dwBytesPerSample = 1; break;
                case sam_FORMAT_MONO_PCM16:         dwBytesPerSample = 2; break;
                case sam_FORMAT_MONO_XD4:           dwBytesPerSample = 1; break;
                case sam_FORMAT_MONO_XD4ADPCM:      dwBytesPerSample = 1; break;
                case sam_FORMAT_STEREO_FLOAT32:     dwBytesPerSample = 8; break;
                case sam_FORMAT_STEREO_XPCM8:       dwBytesPerSample = 2; break;
                case sam_FORMAT_STEREO_PCM8:        dwBytesPerSample = 2; break;
                case sam_FORMAT_STEREO_PCM16:       dwBytesPerSample = 4; break;
                case sam_FORMAT_STEREO_XD4:         dwBytesPerSample = 2; break;
                case sam_FORMAT_STEREO_XD4ADPCM:    dwBytesPerSample = 2; break;
            }
            SAM_FormatGetBytesCount_bTable[dwIndex] = (BYTE)dwBytesPerSample;
        }
    }
    
    __asm {
                    movzx   eax, BYTE PTR bFormat
                    mov     edx, pdwBytesCount
                    
                    
                    //On assume que le pointeur est toujours non-nul
                    //cmp     edx, 0
                    //je      _error
                    
                    mov     al, SAM_FormatGetBytesCount_bTable[eax]
                    mov     [edx], eax
                    
                //_error:
    }
    
    return 0;
}

