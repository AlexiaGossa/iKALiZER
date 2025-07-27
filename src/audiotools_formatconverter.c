#include "AudioTools.h"

#define CONST_DIV_1_2P7     (1.0/128.0)
#define CONST_DIV_1_2P15    (1.0/32768.0)
#define CONST_DIV_1_2P23    (1.0/8388608.0)
#define CONST_DIV_1_2P31    (1.0/2147483648.0)

#define CONST_MUL_2P7     (128.0)
#define CONST_MUL_2P15    (32767.0)
#define CONST_MUL_2P23    (8388607.0)
#define CONST_MUL_2P31    (2147483647.0)


void AudioSampleFormatConvertToFloat32 ( FLOAT32 * pfAudioSampleOutput, void * pAudioSampleInput, DWORD dwAudioSampleInputFormat )
{
    DWORD dwTempData;

    switch (dwAudioSampleInputFormat)
    {
        case AUDIOSAMPLEFORMAT_8BITS_SIGNED:
            *pfAudioSampleOutput = (FLOAT32)(((FLOAT32)*((signed __int8 *)pAudioSampleInput))*CONST_DIV_1_2P7);
            break;

        case AUDIOSAMPLEFORMAT_8BITS_UNSIGNED:
            *pfAudioSampleOutput = (FLOAT32)((((FLOAT32)*((unsigned __int8 *)pAudioSampleInput))-128.0F)*CONST_DIV_1_2P7);
            break;
            
        case AUDIOSAMPLEFORMAT_16BITS_SIGNED:
            *pfAudioSampleOutput = (FLOAT32)((((FLOAT32)*((signed __int16 *)pAudioSampleInput)))*CONST_DIV_1_2P15);
            break;

        case AUDIOSAMPLEFORMAT_16BITS_UNSIGNED:
            *pfAudioSampleOutput = (FLOAT32)((((FLOAT32)*((unsigned __int16 *)pAudioSampleInput))-32768.0F)*CONST_DIV_1_2P15);
            break;

        case AUDIOSAMPLEFORMAT_24BITS_SIGNED:
            dwTempData = (((BYTE *)pAudioSampleInput)[0]<<8) | (((BYTE *)pAudioSampleInput)[1]<<16) | (((BYTE *)pAudioSampleInput)[2]<<24);
            *pfAudioSampleOutput = (FLOAT32)((((FLOAT32)*((signed __int32 *)&dwTempData)))*CONST_DIV_1_2P31);
            break;

        case AUDIOSAMPLEFORMAT_24BITS_UNSIGNED:
            dwTempData = (((BYTE *)pAudioSampleInput)[0]<<8) | (((BYTE *)pAudioSampleInput)[1]<<16) | (((BYTE *)pAudioSampleInput)[2]<<24);
            *pfAudioSampleOutput = (FLOAT32)((((FLOAT32)*((unsigned __int32 *)&dwTempData))-2147483648)*CONST_DIV_1_2P31);
            break;

        case AUDIOSAMPLEFORMAT_32BITS_SIGNED:
            *pfAudioSampleOutput = (FLOAT32)((((FLOAT32)*((signed __int32 *)pAudioSampleInput)))*CONST_DIV_1_2P31);
            break;

        case AUDIOSAMPLEFORMAT_32BITS_UNSIGNED:
            *pfAudioSampleOutput = (FLOAT32)((((FLOAT32)*((unsigned __int32 *)pAudioSampleInput))-2147483648)*CONST_DIV_1_2P31);
            break;

        case AUDIOSAMPLEFORMAT_32BITS_FLOAT:
            *pfAudioSampleOutput = *((FLOAT32 *)pAudioSampleInput);
            break;
        
        case AUDIOSAMPLEFORMAT_64BITS_FLOAT:
            *pfAudioSampleOutput = (FLOAT32)*((FLOAT64 *)pAudioSampleInput);
            break;
    }
}

void AudioSampleFormatConvertToFloat64 ( FLOAT64 * pfAudioSampleOutput, void * pAudioSampleInput, DWORD dwAudioSampleInputFormat )
{
    DWORD dwTempData;

    switch (dwAudioSampleInputFormat)
    {
        case AUDIOSAMPLEFORMAT_8BITS_SIGNED:
            *pfAudioSampleOutput = ((FLOAT64)*((signed __int8 *)pAudioSampleInput))*CONST_DIV_1_2P7;
            break;

        case AUDIOSAMPLEFORMAT_8BITS_UNSIGNED:
            *pfAudioSampleOutput = (((FLOAT64)*((unsigned __int8 *)pAudioSampleInput))-128.0F)*CONST_DIV_1_2P7;
            break;
            
        case AUDIOSAMPLEFORMAT_16BITS_SIGNED:
            *pfAudioSampleOutput = (((FLOAT64)*((signed __int16 *)pAudioSampleInput)))*CONST_DIV_1_2P15;
            break;

        case AUDIOSAMPLEFORMAT_16BITS_UNSIGNED:
            *pfAudioSampleOutput = (((FLOAT64)*((unsigned __int16 *)pAudioSampleInput))-32768.0F)*CONST_DIV_1_2P15;
            break;

        case AUDIOSAMPLEFORMAT_24BITS_SIGNED:
            dwTempData = (((BYTE *)pAudioSampleInput)[0]<<8) | (((BYTE *)pAudioSampleInput)[1]<<16) | (((BYTE *)pAudioSampleInput)[2]<<24);
            *pfAudioSampleOutput = (((FLOAT64)*((signed __int32 *)&dwTempData)))*CONST_DIV_1_2P31;
            break;

        case AUDIOSAMPLEFORMAT_24BITS_UNSIGNED:
            dwTempData = (((BYTE *)pAudioSampleInput)[0]<<8) | (((BYTE *)pAudioSampleInput)[1]<<16) | (((BYTE *)pAudioSampleInput)[2]<<24);
            *pfAudioSampleOutput = (((FLOAT64)*((unsigned __int32 *)&dwTempData))-2147483648)*CONST_DIV_1_2P31;
            break;

        case AUDIOSAMPLEFORMAT_32BITS_SIGNED:
            *pfAudioSampleOutput = (((FLOAT64)*((signed __int32 *)pAudioSampleInput)))*CONST_DIV_1_2P31;
            break;

        case AUDIOSAMPLEFORMAT_32BITS_UNSIGNED:
            *pfAudioSampleOutput = (((FLOAT64)*((unsigned __int32 *)pAudioSampleInput))-2147483648)*CONST_DIV_1_2P31;
            break;

        case AUDIOSAMPLEFORMAT_32BITS_FLOAT:
            *pfAudioSampleOutput = (FLOAT64)*((FLOAT32 *)pAudioSampleInput);
            break;
        
        case AUDIOSAMPLEFORMAT_64BITS_FLOAT:
            *pfAudioSampleOutput = *((FLOAT64 *)pAudioSampleInput);
            break;
    }
}

void AudioSampleFormatConvertFromFloat32 ( void * pAudioSampleOutput, FLOAT32 * pfAudioSampleInput, DWORD dwAudioSampleOutputFormat )
{
    DWORD dwTemp;

    switch (dwAudioSampleOutputFormat)
    {
        case AUDIOSAMPLEFORMAT_8BITS_SIGNED:
            dwTemp = (DWORD)((*pfAudioSampleInput)*CONST_MUL_2P7);
            if ((signed)dwTemp<-128) dwTemp = (signed)-128;
            if ((signed)dwTemp> 127) dwTemp = (signed) 127;
            *((signed __int8 *)pAudioSampleOutput) = (signed __int8)dwTemp;
            break;

        case AUDIOSAMPLEFORMAT_8BITS_UNSIGNED:
            dwTemp = (DWORD)((*pfAudioSampleInput)*CONST_MUL_2P7);
            if ((signed)dwTemp<-128) dwTemp = (signed)-128;
            if ((signed)dwTemp> 127) dwTemp = (signed) 127;
            *((unsigned __int8 *)pAudioSampleOutput) = (unsigned __int8)(dwTemp+128);
            break;
            
        case AUDIOSAMPLEFORMAT_16BITS_SIGNED:
            dwTemp = (DWORD)((*pfAudioSampleInput)*CONST_MUL_2P15);
            if ((signed)dwTemp<-32768) dwTemp = (signed)-32768;
            if ((signed)dwTemp> 32767) dwTemp = (signed) 32767;
            *((signed __int16 *)pAudioSampleOutput) = (signed __int16)dwTemp;
            break;

        case AUDIOSAMPLEFORMAT_16BITS_UNSIGNED:
            dwTemp = (DWORD)((*pfAudioSampleInput)*CONST_MUL_2P15);
            if ((signed)dwTemp<-32768) dwTemp = (signed)-32768;
            if ((signed)dwTemp> 32767) dwTemp = (signed) 32767;
            *((unsigned __int16 *)pAudioSampleOutput) = (unsigned __int16)(dwTemp+32768);
            break;

        case AUDIOSAMPLEFORMAT_24BITS_SIGNED:
            dwTemp = (DWORD)((*pfAudioSampleInput)*CONST_MUL_2P23);
            if ((signed)dwTemp<-8388608) dwTemp = (signed)-8388608;
            if ((signed)dwTemp> 8388607) dwTemp = (signed) 8388607;
            *((BYTE *)pAudioSampleOutput)   = ((BYTE *)dwTemp)[0];
            *((BYTE *)pAudioSampleOutput+1) = ((BYTE *)dwTemp)[1];
            *((BYTE *)pAudioSampleOutput+2) = ((BYTE *)dwTemp)[2];
            break;

        case AUDIOSAMPLEFORMAT_24BITS_UNSIGNED:
            dwTemp = (DWORD)((*pfAudioSampleInput)*CONST_MUL_2P23);
            if ((signed)dwTemp<-8388608) dwTemp = (signed)-8388608;
            if ((signed)dwTemp> 8388607) dwTemp = (signed) 8388607;
            dwTemp += 8388608;
            *((BYTE *)pAudioSampleOutput)   = ((BYTE *)dwTemp)[0];
            *((BYTE *)pAudioSampleOutput+1) = ((BYTE *)dwTemp)[1];
            *((BYTE *)pAudioSampleOutput+2) = ((BYTE *)dwTemp)[2];
            break;

        case AUDIOSAMPLEFORMAT_32BITS_SIGNED:
            dwTemp = (DWORD)((*pfAudioSampleInput)*CONST_MUL_2P31);
            *((signed __int32 *)pAudioSampleOutput) = (signed __int32)dwTemp;
            break;

        case AUDIOSAMPLEFORMAT_32BITS_UNSIGNED:
            dwTemp = (DWORD)((*pfAudioSampleInput)*CONST_MUL_2P31);
            *((signed __int32 *)pAudioSampleOutput) = (signed __int32)(dwTemp+2147483648);
            break;

        case AUDIOSAMPLEFORMAT_32BITS_FLOAT:
            *((FLOAT32 *)pAudioSampleOutput) = *pfAudioSampleInput;
            break;
        
        case AUDIOSAMPLEFORMAT_64BITS_FLOAT:
            *((FLOAT64 *)pAudioSampleOutput) = (FLOAT64)*pfAudioSampleInput;
            break;
    }
}

void AudioSampleFormatConvertFromFloat64 ( void * pAudioSampleOutput, FLOAT64 * pfAudioSampleInput, DWORD dwAudioSampleOutputFormat )
{
    DWORD dwTemp;

    switch (dwAudioSampleOutputFormat)
    {
        case AUDIOSAMPLEFORMAT_8BITS_SIGNED:
            dwTemp = (DWORD)((*pfAudioSampleInput)*CONST_MUL_2P7);
            if ((signed)dwTemp<-128) dwTemp = (signed)-128;
            if ((signed)dwTemp> 127) dwTemp = (signed) 127;
            *((signed __int8 *)pAudioSampleOutput) = (signed __int8)dwTemp;
            break;

        case AUDIOSAMPLEFORMAT_8BITS_UNSIGNED:
            dwTemp = (DWORD)((*pfAudioSampleInput)*CONST_MUL_2P7);
            if ((signed)dwTemp<-128) dwTemp = (signed)-128;
            if ((signed)dwTemp> 127) dwTemp = (signed) 127;
            *((unsigned __int8 *)pAudioSampleOutput) = (unsigned __int8)(dwTemp+128);
            break;
            
        case AUDIOSAMPLEFORMAT_16BITS_SIGNED:
            dwTemp = (DWORD)((*pfAudioSampleInput)*CONST_MUL_2P15);
            if ((signed)dwTemp<-32768) dwTemp = (signed)-32768;
            if ((signed)dwTemp> 32767) dwTemp = (signed) 32767;
            *((signed __int16 *)pAudioSampleOutput) = (signed __int16)dwTemp;
            break;

        case AUDIOSAMPLEFORMAT_16BITS_UNSIGNED:
            dwTemp = (DWORD)((*pfAudioSampleInput)*CONST_MUL_2P15);
            if ((signed)dwTemp<-32768) dwTemp = (signed)-32768;
            if ((signed)dwTemp> 32767) dwTemp = (signed) 32767;
            *((unsigned __int16 *)pAudioSampleOutput) = (unsigned __int16)(dwTemp+32768);
            break;

        case AUDIOSAMPLEFORMAT_24BITS_SIGNED:
            dwTemp = (DWORD)((*pfAudioSampleInput)*CONST_MUL_2P23);
            if ((signed)dwTemp<-8388608) dwTemp = (signed)-8388608;
            if ((signed)dwTemp> 8388607) dwTemp = (signed) 8388607;
            *((BYTE *)pAudioSampleOutput)   = ((BYTE *)dwTemp)[0];
            *((BYTE *)pAudioSampleOutput+1) = ((BYTE *)dwTemp)[1];
            *((BYTE *)pAudioSampleOutput+2) = ((BYTE *)dwTemp)[2];
            break;

        case AUDIOSAMPLEFORMAT_24BITS_UNSIGNED:
            dwTemp = (DWORD)((*pfAudioSampleInput)*CONST_MUL_2P23);
            if ((signed)dwTemp<-8388608) dwTemp = (signed)-8388608;
            if ((signed)dwTemp> 8388607) dwTemp = (signed) 8388607;
            dwTemp += 8388608;
            *((BYTE *)pAudioSampleOutput)   = ((BYTE *)dwTemp)[0];
            *((BYTE *)pAudioSampleOutput+1) = ((BYTE *)dwTemp)[1];
            *((BYTE *)pAudioSampleOutput+2) = ((BYTE *)dwTemp)[2];
            break;

        case AUDIOSAMPLEFORMAT_32BITS_SIGNED:
            dwTemp = (DWORD)((*pfAudioSampleInput)*CONST_MUL_2P31);
            *((signed __int32 *)pAudioSampleOutput) = (signed __int32)dwTemp;
            break;

        case AUDIOSAMPLEFORMAT_32BITS_UNSIGNED:
            dwTemp = (DWORD)((*pfAudioSampleInput)*CONST_MUL_2P31);
            *((signed __int32 *)pAudioSampleOutput) = (signed __int32)(dwTemp+2147483648);
            break;

        case AUDIOSAMPLEFORMAT_32BITS_FLOAT:
            *((FLOAT32 *)pAudioSampleOutput) = (FLOAT32)*pfAudioSampleInput;
            break;
        
        case AUDIOSAMPLEFORMAT_64BITS_FLOAT:
            *((FLOAT64 *)pAudioSampleOutput) = *pfAudioSampleInput;
            break;
    }
}
