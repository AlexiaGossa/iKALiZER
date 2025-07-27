#include "sam_render.h"

void SAM_RENDER_Init_2CH_Surround_2 ( float fDistanceWithinSpeakers, float fSampleRate, SAM_RENDER254 * psamRender )
{
    long            lPrecalcAngle, lAngleMOD64;
    long            lFrontAngle;
    SAM_RENDER254   *pRender;
    float           f1,f2;
    float           fAngle;
    float           fFrontAngle;
    float           fFrontAngleUnity;
    float           fFrontRearAngle;
    float           fFrontRearAngleUnityAbs;
    float           fFrontAngleUnityTable[2];
    float           fDistanceInSamples;
    long            lDistanceInSamples[2];
    float           fBinauralBaseGain;
    long            lDelayCount;
    long            i;
    float           fGainL, fGainR;
    float           fSineBinauralValue[2][32];
    long            lMaxSamplesDistance;
    
    //En cas d'erreur...
    if (!psamRender)
        return;
    pRender = psamRender;

    f1 = sam_VOICEBUFFERSTACK_MASK;
    f1 = f1 * (fSampleRate / 48000.0F);
    lMaxSamplesDistance = (long)floor(f1);
    if (lMaxSamplesDistance>sam_VOICEBUFFERSTACK_MASK)
        lMaxSamplesDistance = sam_VOICEBUFFERSTACK_MASK;

    // Init the binaural effect LUT for surround sound
    SAM_RENDER_InitBinaural ( fSineBinauralValue, fDistanceWithinSpeakers, fSampleRate, 2 );

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
        fFrontRearAngle         = (float)sam_ABS(lPrecalcAngle);
        fFrontRearAngleUnityAbs = fFrontRearAngle * 0.0055F;     //0 = front ... 1 = rear

        // L/R Gain                
        if (fFrontAngleUnity>=0) f1 =  (float)pow (  fFrontAngleUnity, 2.0 );
        else                     f1 = -(float)pow ( -fFrontAngleUnity, 2.0 );
        f2                          = 0.15F + (float)pow(fFrontRearAngleUnityAbs,4)*0.30F;
        f1                          = (float)pow ( 10, f1 * f2 );  // 0.707 ... 1.413
        fFrontAngleUnityTable[0]    = f1;
        fFrontAngleUnityTable[1]    = 1/f1;
        
        // Convert meters in samples
        fDistanceInSamples = (fDistanceWithinSpeakers / 340)*fSampleRate;
        if (fDistanceInSamples<0) fDistanceInSamples = 0;
        if (fDistanceInSamples>510) fDistanceInSamples = 510;

        // Determine base gain L/R (from front/rear)
        f1 = fFrontRearAngleUnityAbs*fFrontRearAngleUnityAbs; //pow ( f3, 2.0 );
        fGainL = 1.0F - f1;
        fGainR = 1.0F - f1;

        f1 = fFrontAngle * 0.3183F;  //f1 = -0.5 ... +0.5
        f1 = (f1+1)*0.5F;  // 0.0 ... 1.0  (left-right angle)
        lDistanceInSamples[0] = (long)((1-f1) * fDistanceInSamples);
        lDistanceInSamples[1] = (long)(   f1  * fDistanceInSamples);

        fBinauralBaseGain = 0.7F * (fFrontRearAngleUnityAbs * fFrontRearAngleUnityAbs);
        lDelayCount = 0;
        for (i=0;i<4;i++)
        {
            pRender->dwDelayIndex[lDelayCount] = lDistanceInSamples[0]*4;
            pRender->fDelayGain[lDelayCount] = fFrontAngleUnityTable[0] * fGainL;
            //pRender->dwChannelIndex[lDelayCount] = 0; // Left channel
            lDelayCount += 1;
        
            pRender->dwDelayIndex[lDelayCount] = lDistanceInSamples[1]*4;
            pRender->fDelayGain[lDelayCount] = fFrontAngleUnityTable[1] * fGainR;
            //pRender->dwChannelIndex[lDelayCount] = 1; // Right channel
            lDelayCount += 1;

            // Next gain for render rear sound (f3: 0=front; 1=rear)
            fGainL = fBinauralBaseGain * fSineBinauralValue[0][i];// * f3;
            fGainR = fBinauralBaseGain * fSineBinauralValue[1][i];// * f3;

            lDistanceInSamples[0] += (long)fDistanceInSamples;
            lDistanceInSamples[1] += (long)fDistanceInSamples;

            fDistanceInSamples *= 0.95F;

            if ( (lDistanceInSamples[0]>=lMaxSamplesDistance) || (lDistanceInSamples[1]>=lMaxSamplesDistance) )
            {
                i = 32;
            }
            
            //Binaural is below than -40dB... not very usefull for binaural rendering
            if (fBinauralBaseGain<0.01F)
            {
                i = 32;
            }

            if (lDelayCount>=RENDER_MAXCOUNT)
                break;
        }
        pRender->dwEntryCount = lDelayCount;        
    }
}





void SAM_RENDER_Init_4CH_L_R_SL_SR_old ( float fDistanceWithinSpeakers, float fSampleRate, SAM_RENDER254 * psamRender )
{
    long            lPrecalcAngle, lAngleMOD64;
    long            lFrontAngle;
    SAM_RENDER254   *pRender;
    float           fAngle;
    float           fFrontAngle;
    float           fFrontAngleUnity;
    float           fFrontRearAngle;
    float           fFrontRearAngleUnityAbs;
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
        fFrontRearAngle         = (float)sam_ABS(lPrecalcAngle);
        fFrontRearAngleUnityAbs = fFrontRearAngle * 0.0055F;     //0 = front ... 1 = rear

        // L/R Gain                
        //  -1 ... 0.0 ... +1
        //  0  ... 0.5 ... +1
        //  0  ... 0.7 ... +1.4
        fGainL = (fFrontAngleUnity+1)*0.707F;
        fGainR = 1.414F - fGainL;
        
        //0 = left
        //1 = right
        //2 = sr-left
        //3 = sr-right

        //Left        
        pRender->dwDelayIndex[0] = 0;
        pRender->fDelayGain[0] = fGainL * (1-fFrontRearAngleUnityAbs);

        //Right        
        pRender->dwDelayIndex[1] = 0;
        pRender->fDelayGain[1] = fGainR * (1-fFrontRearAngleUnityAbs);

        //SR-Left
        pRender->dwDelayIndex[2] = 0;
        pRender->fDelayGain[2] = fGainL * fFrontRearAngleUnityAbs;

        //SR-Right        
        pRender->dwDelayIndex[3] = 0;
        pRender->fDelayGain[3] = fGainR * fFrontRearAngleUnityAbs;

        pRender->dwEntryCount = 4;
    }
}









void SAM_RENDER_Init_4CH_L_R_C_S ( float fDistanceWithinSpeakers, float fSampleRate, SAM_RENDER254 * psamRender )
{
    long            lPrecalcAngle, lAngleMOD64;
    long            lFrontAngle;
    SAM_RENDER254   *pRender;
    float           fAngle;
    float           fFrontAngle;
    float           fFrontAngleUnity;
    float           fFrontRearAngle;
    float           fFrontRearAngleUnityAbs;
    long            lDelayCount;
    float           fGainL, fGainR, fGainC;
    
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
        fFrontRearAngle         = (float)sam_ABS(lPrecalcAngle);
        fFrontRearAngleUnityAbs = fFrontRearAngle * 0.0055F;     //0 = front ... 1 = rear

        // L/R Gain                
        //  -1 ... 0.0 ... +1
        //  0  ... 0.5 ... +1
        //  0  ... 0.7 ... +1.4
        fGainL = (fFrontAngleUnity+1)*0.707F;
        fGainR = 1.414F - fGainL;
        fGainC = (float)(1.0 - pow ( fabs(fFrontAngleUnity), 0.9 ));
        
        lDelayCount = 0;

        
        pRender->dwDelayIndex[lDelayCount]      = 0;
        pRender->fDelayGain[lDelayCount]        = fGainL * (1-fFrontRearAngleUnityAbs) * (float)sqrt(1-fGainC);
        //pRender->dwChannelIndex[lDelayCount]    = 0; // Front Left channel
        lDelayCount += 1;

        pRender->dwDelayIndex[lDelayCount]      = 0;
        pRender->fDelayGain[lDelayCount]        = fGainR * (1-fFrontRearAngleUnityAbs) * (float)sqrt(1-fGainC);
        //pRender->dwChannelIndex[lDelayCount]    = 1; // Front Right channel
        lDelayCount += 1;
        
        pRender->dwDelayIndex[lDelayCount]      = 0;
        pRender->fDelayGain[lDelayCount]        = 0.5F * (1-fFrontRearAngleUnityAbs) * fGainC;
        //pRender->dwChannelIndex[lDelayCount]    = 2; // Front Center channel
        lDelayCount += 1;
        
        pRender->dwDelayIndex[lDelayCount]      = 0;
        pRender->fDelayGain[lDelayCount]        = (fGainL+fGainR) * 0.5F * fFrontRearAngleUnityAbs;
        //pRender->dwChannelIndex[lDelayCount]    = 3; // Rear Channel
        lDelayCount += 1;
        
        pRender->dwEntryCount = lDelayCount;
    }
}
