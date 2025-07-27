#include "sam_render.h"

void SAM_RENDER_Init_2CH_360VS ( float fDistanceWithinSpeakers, float fSampleRate, SAM_RENDER254 * psamRender )
{
    long            lPrecalcAngle;
    long            lAngleMOD64;
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
    float           fGainCompensation;
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
    SAM_RENDER_InitBinaural ( fSineBinauralValue, fDistanceWithinSpeakers, fSampleRate, 1 );

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

        //Lorsque le signal est derrière, on augmente légèrement le niveau
        fGainCompensation = (float)(1+pow(fFrontRearAngleUnityAbs,2)*1);

        f1 = fFrontAngle * 0.3183F;  //f1 = -0.5 ... +0.5
        f1 = (f1+1)*0.5F;  // 0.0 ... 1.0  (left-right angle)
        lDistanceInSamples[0] = (long)((1-f1) * fDistanceInSamples);
        lDistanceInSamples[1] = (long)(   f1  * fDistanceInSamples);
        
        memset ( pRender, 0, sizeof(SAM_RENDER254) );
        
        fDistanceInSamples *= 0.5F;
        fBinauralBaseGain = 0.7F * (fFrontRearAngleUnityAbs * fFrontRearAngleUnityAbs);
        lDelayCount = 0;
        for (i=0;i<32;i++)
        {
            pRender->dwDelayIndex[lDelayCount]      = lDistanceInSamples[0];
            pRender->fDelayGain[lDelayCount]        = fFrontAngleUnityTable[0] * fGainL * fGainCompensation;
            //pRender->dwChannelIndex[lDelayCount]    = 0; // Left channel
            lDelayCount += 1;
        
            pRender->dwDelayIndex[lDelayCount]      = lDistanceInSamples[1];
            pRender->fDelayGain[lDelayCount]        = fFrontAngleUnityTable[1] * fGainR * fGainCompensation;
            //pRender->dwChannelIndex[lDelayCount]    = 1; // Right channel
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



