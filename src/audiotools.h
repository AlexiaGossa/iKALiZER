/*
    (c) Alexia Gossa
*/

    #include <math.h>
    #include <stdio.h>
    #include <stdlib.h>
    #include <memory.h>

    #ifdef _WIN32
        #include <windows.h>
        #include <windowsx.h>        
        #define _CRT_SECURE_NO_WARNINGS
        #pragma warning(disable : 4996)
    #endif

    typedef float               FLOAT32;
    typedef double              FLOAT64;

    #define AUDIOTOOLS_MALLOC(x)           malloc(x)
    #define AUDIOTOOLS_CALLOC(x,y)         calloc(x,y)
    #define AUDIOTOOLS_REALLOC(x,y)        realloc(x,y)
    #define AUDIOTOOLS_FREE(x)             free(x)


    #ifndef _WIN32
    typedef unsigned __int8     BYTE;
    typedef unsigned __int16    WORD;
    typedef unsigned __int32    DWORD;
    #endif
    typedef unsigned __int64    QWORD;


    #define AUDIOTOOLS_PI       3.1415926535897932384626433832795F

    //Fonctions scalaires
    void Scalar_MulAndUpdate_f32 ( long lNbPoints, float *pf32ValueList, float f32Mul );
    void Scalar_MulAndUpdate_f64 ( long lNbPoints, double *pf64ValueList, double f64Mul );
    void Scalar_MulToNew_f32 ( long lNbPoints, float *pf32InValueList, float *pf32OutValueList, float f32Mul );
    void Scalar_MulToNew_f64 ( long lNbPoints, double *pf64InValueList, double *pf64OutValueList, double f64Mul );

    //Fonctions vectorielles
    void Vector_MulAndUpdate_f32 ( long lNbPoints, float *pf32ValueList, float *pf32MulList );
    void Vector_MulAndUpdate_f64 ( long lNbPoints, double *pf64ValueList, double *pf64MulList );
    void Vector_MulToNew_f32 ( long lNbPoints, float *pf32InValueList, float *pf32OutValueList, float *pf32MulList );
    void Vector_MulToNew_f64 ( long lNbPoints, double *pf64InValueList, double *pf64OutValueList, double *pf64MulList );
    void Vector_AddAndUpdate_f32 ( long lNbPoints, float *pf32InOutValueList, float *pf32AddList );
    void Vector_AddAndUpdate_f64 ( long lNbPoints, double *pf64InOutValueList, double *pf64AddList );
    void Vector_AddToNew_f32 ( long lNbPoints, float *pf32InValueList, float *pf32OutValueList, float *pf32AddList );
    void Vector_AddToNew_f64 ( long lNbPoints, double *pf64InValueList, double *pf64OutValueList, float *pf64AddList );
    void Vector_MulAddToNew_f32 ( long lNbPoints, float *pf32InValueList, float *pf32OutValueList, float *pf32MulList );
    void Vector_MulAddToNew_f64 ( long lNbPoints, double *pf64InValueList, double *pf64OutValueList, double *pf64MulList );


    //Fonctions de fenêtres
    void Window_FullBuild_f32 ( long lWindowShape, long lNbPoints, float * pf32Window );
    void Window_FullBuild_f64 ( long lWindowShape, long lNbPoints, double * pf64Window );
    void Window_MixBuild_f32 ( long lWindowShape, long lBeginWindowNbPoints, long lEndWindowNbPoints, float * pf32Window );
    void Window_MixBuild_f64 ( long lWindowShape, long lBeginWindowNbPoints, long lEndWindowNbPoints, double * pf64Window );

    //Fonctions MDCT (traitement interne 64 bits)
    void MDCT_Alloc ( void ** pMDCTInfo, long lNbFrequencyLines );
    void MDCT_Free ( void * pMDCTInfo );
    void MDCT_Forward_f32 ( void * pMDCTInfo, float *pfTemporalInput, float *pfFrequencyOutput );
    void MDCT_Forward_f64 ( void * pMDCTInfo, double *pfTemporalInput, double *pfFrequencyOutput );
    void MDCT_Backward_f32 ( void * pMDCTInfo, float *pfFrequencyInput, float *pfTemporalOutput );
    void MDCT_Backward_f64 ( void * pMDCTInfo, double *pfFrequencyInput, double *pfTemporalOutput );

    //Fonctions FFT (traitement interne 64 bits)
    //512 spectral lines = 1024 temporal points
    void FFT_Alloc ( void ** pFFTInfo, long lNbTemporalPoints );
    void FFT_Free ( void * pFFTInfo );
    void FFT_Forward_f32 ( void * pFFTInfo, float *pfTemporalInput, float *pfFrequencyOutput, float *pfPhaseOutput );
    void FFT_Backward_f32 ( void * pFFTInfo, float *pfFrequencyInput, float *pfPhaseInput, float *pfTemporalOutput );
    void FFT_ForwardComplex_f32 ( void * pFFTInfo, float * pfTemporalInput, float * pfReOutput, float *pfImOutput );
    void FFT_BackwardComplex_f32 ( void * pFFTInfo, float * pfReInput, float *pfImInput, float *pfTemporalOutput );

    //Fonctions BitStream
    void                BitStream_Alloc             ( void ** pBitStream );
    void                BitStream_Free              ( void * pBitStream );
    void                BitStream_ResetPosition     ( void * pBitStream );
    void                BitStream_ResetBuffer       ( void * pBitStream );
    void                BitStream_GetBuffer         ( void * pBitStream, unsigned char * pBuffer, unsigned long * plBytesSize );
    void                BitStream_SetBuffer         ( void * pBitStream, unsigned char * pBuffer, unsigned long lBytesSize );
    unsigned long       BitStream_GetBit            ( void * pBitStream );
    unsigned long       BitStream_GetBits           ( void * pBitStream, unsigned long lNbBits );
    void                BitStream_PutBit            ( void * pBitStream, unsigned long lValue );
    void                BitStream_PutBits           ( void * pBitStream, unsigned long lValue, unsigned long lNbBits );

    //Fonctions AudioSampleFormat
    #define AUDIOSAMPLEFORMAT_8BITS_SIGNED              0x0000
    #define AUDIOSAMPLEFORMAT_8BITS_UNSIGNED            0x0001
    #define AUDIOSAMPLEFORMAT_16BITS_SIGNED             0x0020
    #define AUDIOSAMPLEFORMAT_16BITS_UNSIGNED           0x0021
    #define AUDIOSAMPLEFORMAT_24BITS_SIGNED             0x0060
    #define AUDIOSAMPLEFORMAT_24BITS_UNSIGNED           0x0061
    #define AUDIOSAMPLEFORMAT_32BITS_SIGNED             0x0070
    #define AUDIOSAMPLEFORMAT_32BITS_UNSIGNED           0x0071
    #define AUDIOSAMPLEFORMAT_32BITS_FLOAT              0x00A0
    #define AUDIOSAMPLEFORMAT_64BITS_FLOAT              0x00B0
    
    void AudioSampleFormatConvertToFloat32 ( FLOAT32 * pfAudioSampleOutput, void * pAudioSampleInput, DWORD dwAudioSampleInputFormat );
    void AudioSampleFormatConvertToFloat64 ( FLOAT64 * pfAudioSampleOutput, void * pAudioSampleInput, DWORD dwAudioSampleInputFormat );
    void AudioSampleFormatConvertFromFloat32 ( void * pAudioSampleOutput, FLOAT32 * pfAudioSampleInput, DWORD dwAudioSampleOutputFormat );
    void AudioSampleFormatConvertFromFloat64 ( void * pAudioSampleOutput, FLOAT64 * pfAudioSampleInput, DWORD dwAudioSampleOutputFormat );


    //Fonctions IO des fichiers WAVE
    #define wioWAVE_FORMAT_PCM          0x0001
    #define wioWAVE_FORMAT_IEEE_FLOAT   0x0003

    #ifndef DEFwioWAVEFORMAT
    #define DEFwioWAVEFORMAT
    #pragma pack(push, 1)
    typedef struct {
        WORD    wFormatTag;             //1 = PCM, 3 = FLOAT
        WORD    wChannels;              //Nombre de canaux
        DWORD   dwSampleRate;           //Fréquence d'échantillonnage
        DWORD   dwAverageBytesPerSec;   //Nombre moyen d'octets par secondes
        WORD    wBlocAlign;             //Nombre d'octets pour les canaux échantillonnés soit (wChannels*wBitsPerSample)>>3
        WORD    wBitsPerSingleSample;   //Nombre de bits par échantillon d'un seul canal
    } wioWAVEFORMAT;
    #pragma pack(pop)
    #endif

    long WaveIO_IFileOpen               ( void ** pWaveIO, char * pszFileNameToRead );
    long WaveIO_OFileOpen               ( void ** pWaveIO, char * pszFileNameToWrite );

    long WaveIO_IFileClose              ( void * pWaveIO );
    long WaveIO_OFileClose              ( void * pWaveIO );

    long WaveIO_IFileRead               ( void * pWaveIO, void * pAudioBuffer, DWORD dwTotalSamplesToRead, DWORD * pdwTotalSamplesRead );
    long WaveIO_IFileGetSpecs           ( void * pWaveIO, wioWAVEFORMAT * pwioWaveFormat, DWORD * pdwTotalSamples );

    long WaveIO_OFileWrite              ( void * pWaveIO, void * pAudioBuffer, DWORD dwSamplesToWrite, DWORD * pdwSamplesWritten );
    long WaveIO_OFileSetSpecs           ( void * pWaveIO, wioWAVEFORMAT * pwioWaveFormat );
