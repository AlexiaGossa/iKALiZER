#include "sam_render.h"

                

void SAM_RENDER_ConvolutionFilter_Init ( T_CONVOLUTION_FILTER_LR * pConvFilter )
{
    memset ( pConvFilter, 0, sizeof(T_CONVOLUTION_FILTER_LR) );
}

void SAM_RENDER_ConvolutionFilter_Add ( T_CONVOLUTION_FILTER_LR * pConvFilter, DWORD dwIndex, float fGain, long lChannel )
{
    if (dwIndex<2048)
    {
        if (!lChannel) pConvFilter->fConvolutionLeft[dwIndex]  += fGain;
        else           pConvFilter->fConvolutionRight[dwIndex] += fGain;
    }
}

