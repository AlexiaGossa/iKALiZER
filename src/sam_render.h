#include "sam_header.h"
#include "sam_data.h"
#include "sam_voice.h"


#define RENDER_MAXCOUNT (64)

typedef struct {
    DWORD dwDelayIndex;
    float fDelayGain;
} DELAYLINE;



int SAM_RENDER_DelayLineSortByLevelProc ( const void * pElementA, const void * pElementB );
void SAM_RENDER_LimitTotalAmountDelayCount ( SAM_RENDER254 * pRender, DWORD dwEntryCountMax );
void SAM_RENDER_LimitTotalAmountDelayCount_2Kto64 ( SAM_RENDER254 * pRenderTarget, SAM_RENDER2K * pRender2KSource, DWORD dwEntryCountMax );
void SAM_RENDER_MaximizeLevel ( SAM_RENDER254 * pRender, long lChannelCount, long lSampleRate );

void SAM_RENDER_InitBinaural ( float fSineBinauralValue[2][32], float fDistanceValue, float fSampleRate, long lBinauralModel );

typedef struct {
    double fValue[1024];
    long lCount;
    long lSampleRate;
} T_CONVOLUTION;
 
long Convolution_GetCount ( T_CONVOLUTION * pConvolution, long lSampleRate );
float Convolution_GetValue ( T_CONVOLUTION * pConvolution, long lSampleRate, long lIndex );


//Les tables de convolution
extern  T_CONVOLUTION convolutionHeadphonesRearL[6];
extern  T_CONVOLUTION convolutionHeadphonesRearR[6];


void SAM_RENDER_Init_2CH_Headphones_Holographic ( float fDistanceWithinSpeakers, float fSampleRate, SAM_RENDER254 * psamRender );
void SAM_RENDER_Init_2CH_Headphones_Hybrid_HRTF ( float fDistanceWithinSpeakers, float fSampleRate, SAM_RENDER254 * psamRender );
void SAM_RENDER_InitSpecial_2CH_Headphones_Holographic ( float fDistanceWithinSpeakers, float fSampleRate, SAM_RENDER254 * psamRender, long lMode );

typedef struct {
    float fConvolutionLeft[2048];
    float fConvolutionRight[2048];
} T_CONVOLUTION_FILTER_LR;

void SAM_RENDER_ConvolutionFilter_Init ( T_CONVOLUTION_FILTER_LR * pConvFilter );
void SAM_RENDER_ConvolutionFilter_Add ( T_CONVOLUTION_FILTER_LR * pConvFilter, DWORD dwIndex, float fGain, long lChannel );

/*
extern DWORD __0x26fl__dwCryptKey[];
extern DWORD __0x26fl__dwDataChannelCount;
extern DWORD __0x26fl__dw32bitsBlocCount;
extern DWORD __0x26fl__dwDataChannel0[];
extern DWORD __0x26fl__dwDataChannel1[];

extern DWORD __0x26fr__dwCryptKey[];
extern DWORD __0x26fr__dwDataChannelCount;
extern DWORD __0x26fr__dw32bitsBlocCount;
extern DWORD __0x26fr__dwDataChannel0[];
extern DWORD __0x26fr__dwDataChannel1[];

extern DWORD __0x26fc__dwCryptKey[];
extern DWORD __0x26fc__dwDataChannelCount;
extern DWORD __0x26fc__dw32bitsBlocCount;
extern DWORD __0x26fc__dwDataChannel0[];
extern DWORD __0x26fc__dwDataChannel1[];

extern DWORD __0x26rc__dwCryptKey[];
extern DWORD __0x26rc__dwDataChannelCount;
extern DWORD __0x26rc__dw32bitsBlocCount;
extern DWORD __0x26rc__dwDataChannel0[];
extern DWORD __0x26rc__dwDataChannel1[];

extern DWORD __0x26rl__dwCryptKey[];
extern DWORD __0x26rl__dwDataChannelCount;
extern DWORD __0x26rl__dw32bitsBlocCount;
extern DWORD __0x26rl__dwDataChannel0[];
extern DWORD __0x26rl__dwDataChannel1[];

extern DWORD __0x26rr__dwCryptKey[];
extern DWORD __0x26rr__dwDataChannelCount;
extern DWORD __0x26rr__dw32bitsBlocCount;
extern DWORD __0x26rr__dwDataChannel0[];
extern DWORD __0x26rr__dwDataChannel1[];
*/