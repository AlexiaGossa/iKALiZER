#include "sam_render.h"

void SAM_RENDER_Init_2CH_Stereo ( float fDistanceWithinSpeakers, float fSampleRate, SAM_RENDER254 * psamRender )
{
    long            lPrecalcAngle;
    long            lAngleMOD64;
    long            lFrontAngle;
    SAM_RENDER254   *pRender;
    float           fAngle;
    float           fFrontAngle;
    float           fFrontAngleUnity;
    long            lDelayCount;
    float           fGainL, fGainR;
    
    //En cas d'erreur...
    if (!psamRender)
        return;
    pRender = psamRender;

    // Process -180 to +180 degrees and fill the LUT
    for (lAngleMOD64=0;lAngleMOD64<64;lAngleMOD64++,pRender++)
    {
        lPrecalcAngle = ((lAngleMOD64*45)/8) - 180;

        // Angle process
        lFrontAngle = lPrecalcAngle;
        if (lFrontAngle> 90) lFrontAngle = 180-lFrontAngle;
        if (lFrontAngle<-90) lFrontAngle = -180-lFrontAngle;
        fAngle                  = ((float)lPrecalcAngle) * 0.0174532F;
        fFrontAngle             = ((float)lFrontAngle  ) * 0.0174532F;   //-PI/2 ... +PI/2
        fFrontAngleUnity        = ((float)lFrontAngle  ) * 0.0111111F;    //-1 ... +1

        // L/R Gain                
        //  -1 ... 0.0 ... +1
        //  0  ... 0.5 ... +1
        //  0  ... 0.7 ... +1.4
        fGainL = (fFrontAngleUnity+1)*0.707F;
        fGainR = 1.414F - fGainL;
        
        lDelayCount = 0;
        pRender->dwDelayIndex[lDelayCount] = 0;
        pRender->fDelayGain[lDelayCount] = fGainL;
        //pRender->dwChannelIndex[lDelayCount] = 0; // Left channel
        lDelayCount += 1;
        
        pRender->dwDelayIndex[lDelayCount] = 0;
        pRender->fDelayGain[lDelayCount] = fGainR;
        //pRender->dwChannelIndex[lDelayCount] = 1; // Right channel
        lDelayCount += 1;

        pRender->dwEntryCount = lDelayCount;        
    }
}
