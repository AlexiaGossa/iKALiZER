#include "sam_render.h"

/*

    Headphone rendering

        ITD = Interaural Time Difference
        IIT/ILD = Interaural Intensity/Level Difference
        Bauer delay = Use multiple delay line to simulate HRTF
        Surround spatialisation = Apply a surround+reverd effect to rear sounds

*/

/*

    Simulation de rendu holographique :
    
    Devant et haut-dessus
    ---------------------
    
        fValue = Convolution_GetValue ( Convolution_HighPass_3000, (long)fSampleRate, i ) * 1.6F;
        ConvolutionFilter_Add ( &ConvFilter, i,  fValue * fGain, 0 );
        ConvolutionFilter_Add ( &ConvFilter, i,  fValue * fGain, 1 );

        fValue = Convolution_GetValue ( Convolution_LowPass_6000, (long)fSampleRate, i ) * 1.6F;
        ConvolutionFilter_Add ( &ConvFilter, i + (long)floor( (fSampleRate / 48000.0F) * (0) ), fValue * fGain, 0 );
        ConvolutionFilter_Add ( &ConvFilter, i + (long)floor( (fSampleRate / 48000.0F) * (0) ), fValue * fGain, 1 );
        ConvolutionFilter_Add ( &ConvFilter, i + (long)floor( (fSampleRate / 48000.0F) * (40) ), fValue * fGain * 0.5F, 0 );
        ConvolutionFilter_Add ( &ConvFilter, i + (long)floor( (fSampleRate / 48000.0F) * (60) ), fValue * fGain * 0.5F, 1 );
        ConvolutionFilter_Add ( &ConvFilter, i + (long)floor( (fSampleRate / 48000.0F) * (260) ), fValue * fGain * 0.2F, 0 );
        ConvolutionFilter_Add ( &ConvFilter, i + (long)floor( (fSampleRate / 48000.0F) * (240) ), fValue * fGain * 0.2F, 1 );


    Haut-dessus
    ---------------------
    
        fValue = Convolution_GetValue ( Convolution_HighPass_3000, (long)fSampleRate, i ) * 1.6F;
        ConvolutionFilter_Add ( &ConvFilter, i,  fValue * fGain, 0 );
        ConvolutionFilter_Add ( &ConvFilter, i,  fValue * fGain, 1 );

        fValue = Convolution_GetValue ( Convolution_LowPass_6000, (long)fSampleRate, i ) * 1.6F;
        ConvolutionFilter_Add ( &ConvFilter, i + (long)floor( (fSampleRate / 48000.0F) * (0) ), fValue * fGain, 0 );
        ConvolutionFilter_Add ( &ConvFilter, i + (long)floor( (fSampleRate / 48000.0F) * (0) ), fValue * fGain, 1 );
        ConvolutionFilter_Add ( &ConvFilter, i + (long)floor( (fSampleRate / 48000.0F) * (40) ), fValue * fGain * 0.2F, 0 );
        ConvolutionFilter_Add ( &ConvFilter, i + (long)floor( (fSampleRate / 48000.0F) * (80) ), fValue * fGain * 0.2F, 1 );
        ConvolutionFilter_Add ( &ConvFilter, i + (long)floor( (fSampleRate / 48000.0F) * (380) ), fValue * fGain * 0.3F, 0 );
        ConvolutionFilter_Add ( &ConvFilter, i + (long)floor( (fSampleRate / 48000.0F) * (340) ), fValue * fGain * 0.3F, 1 );


*/



           
T_CONVOLUTION Convolution_LowPass_6000[6] = {
        
        //48000 Hz
        {   {   0.052978516F,
                0.12915039F,
                0.16781616F,
                0.12915039F,
                0.052978516F },
            5, 48000 },
            
        //44100 Hz    
        {   {   0.04699707F,
                0.13189697F,
                0.1661377F,
                0.11621094F,
                0.031707764F },
            5, 44100 },

        //32000 Hz
        {   {   0.021087646F,
                0.1293335F,
                0.15786743F,
                0.051452637F },
            4, 32000 },
        
        //24000 Hz    
        {   {   0.054504395F,
                0.16583252F,
                0.054260254F },
            3, 24000 },
        
        //22050 Hz    
        {   {   0.053131104F,
                0.16189575F,
                0.038421631F },
            3, 22050 },
            
        {   {   0.0F },
            0, 0 },
        
        };
        

T_CONVOLUTION Convolution_SideEar[6] = {
        {   {   0.029817265F,
                0.011171134F,
                -0.044129528F,
                -0.14608847F,
                0.78114569F,
                -0.14608848F,
                -0.044129536F,
                0.011171134F,
                0.029817266F },
            9, 48000 },
            
        {   {   0.032092221F,
                0.020977955F,
                -0.035454862F,
                -0.15009385F,
                0.76777864F,
                -0.15009385F,
                -0.035454862F,
                0.020977955F,
                0.032092221F },
            9, 44100 },
            
        {   {   0.046019763F,
                0.01673755F,
                -0.15582225F,
                0.70623648F,
                -0.15582225F,
                0.01673755F,
                0.046019763F },
            7, 32000 },
            
        {   {   0.074051946F,
                -0.13505632F,
                0.63556004F,
                -0.13505632F,
                0.074051946F },
            5, 24000 },
            
        {   {   0.085540883F,
                -0.12092823F,
                0.6106286F,
                -0.12092823F,
                0.085540891F },
            5, 22050 },
            
        {   {   0.0F },
            0, 0 },
        
        };
        

//Filtre testé et validé.
T_CONVOLUTION Convolution_LowPass_3000[6] = {
        
        {   {   0.03978835F,
                0.048995547F,
                0.056237258F,
                0.060863629F,
                0.06245422F,
                0.060863629F,
                0.056237262F,
                0.048995547F,
                0.03978835F },
            9, 48000 },
            
        {   {   0.039399032F,
                0.050871044F,
                0.060067758F,
                0.066014335F,
                0.068070829F,
                0.066014335F,
                0.060067762F,
                0.050871044F,
                0.039399032F },
            9, 44100 },
            
        {   {   0.052015558F,
                0.073551908F,
                0.088491485F,
                0.093833953F,
                0.088491485F,
                0.073551916F,
                0.052015565F },
            7, 32000 },
            
           
        {   {   0.079577252F,
                0.11256104F,
                0.12503052F,
                0.11256105F,
                0.079577252F },
            5, 24000 },
            
        {   {   0.078798562F,
                0.12013569F,
                0.13614163F,
                0.1201357F,
                0.078798562F },
            5, 22050 },
            
            
        {   {   0.0F },
            0, 0 },
            
        };        
        
T_CONVOLUTION Convolution_HighPass_3000[6] = {
        
        {   {   -0.024810791F,
                -0.024780273F,
                -0.068084717F,
                -0.056793213F,
                0.40621948F,
                -0.056793213F,
                -0.068084717F,
                -0.024780273F,
                -0.024810791F },
            9, 48000 },
            
        {   {   -0.048701357F,
                0.0066843219F,
                -0.13160515F,
                0.11186689F,
                0.30042991F,
                -0.1485216F,
                -0.0083806608F,
                -0.022787193F,
                -0.005326123F },
            9, 44100 },
        
        {   {   0.016982298F,
                -0.10181499F,
                0.069517389F,
                0.20121552F,
                -0.12385329F,
                -0.019621029F },
            6, 32000 },
        
        {   {   -0.019821446F,
                -0.052490387F,
                0.1691421F,
                -0.052490391F,
                -0.024810791F },
            5, 24000 },
            
        {   {   -0.038324267F,
                -0.013467157F,
                0.13938096F,
                -0.076462932F,
                -0.0011909214F },
            5, 22050 },
                
                
            
        {   {   0.0F },
            0, 0 },
        
        };
                

void SAM_RENDER_Init_2CH_Headphones_Holographic ( float fDistanceWithinSpeakers, float fSampleRate, SAM_RENDER254 * psamRender )
{
    SAM_RENDER254   *pRender;
    SAM_RENDER2K    Render2K, *pRender2K;
    long            lMaxSamplesDistance;
    float           f1,f2, f3, f4;
    long            lDistance;
    long            lDistanceL;
    long            lDistanceR;

    long            lAngleMOD64;
    long            lPrecalcAngle, lIndexAngle;
    long            lFrontAngle;
    float           fAngle;
    float           fFrontAngle;
    float           fFrontAngleUnity;
    float           fFrontRearAngle;
    float           fFrontRearAngleUnityAbs;
    //float           fFrontAngleUnityTable[2];
    float           fDistanceInSamples;
    long            lDistanceInSamples[2];
    float           fBinauralBaseGain;
    //long            lDelayCount;
    long            i, j, lCount;
    float           fGain, fGainL, fGainR, fValue, fValueL, fValueR;
    //float           fSineBinauralValue[2][32];
    float           fGainCompensation;
    UINT32          ui32Random;
    //float           fDistanceL, fDistanceR;
    float           fDistanceIncr, fDistanceMul;
    float           fDistanceBaseL, fDistanceBaseR;
    float           fDistanceRearL, fDistanceRearR;
    float fGainConvolutionL, fGainConvolutionR;
    float fGainDirectL, fGainDirectR;
    
    DWORD           dwLDelayIndex[2048];
    DWORD           dwRDelayIndex[2048];
    float           fLDelayIndex[2048];
    float           fRDelayIndex[2048];
    
    T_CONVOLUTION_FILTER_LR ConvFilter;  
        
    

    
    //En cas d'erreur...
    if (!psamRender)
        return;
    pRender = psamRender;


    f1 = sam_VOICEBUFFERSTACK_MASK;
    f1 = f1 * (fSampleRate / 48000.0F);
    lMaxSamplesDistance = (long)floor(f1);
    if (lMaxSamplesDistance>sam_VOICEBUFFERSTACK_MASK)
        lMaxSamplesDistance = sam_VOICEBUFFERSTACK_MASK;


    // Process -180 to +180 degrees and fill the LUT
    for (lAngleMOD64=0;lAngleMOD64<64;lAngleMOD64++,pRender++)
    {
        lIndexAngle = ((lAngleMOD64*45)/8) - 180;

        f1 = (float)fabs ( lIndexAngle );
        if (f1>90.0F)
        {
            f2 = 1.2F;
            f1 -= 90.0F;
            f1 = (float)( pow ( f1, f2 ) / ( pow ( 90, f2-1.0F ) ) );
            f1 += 90.0F;
        }
        //f1 = pow ( fabs(lIndexAngle), 1.2 ) / 2.83F;
        if (f1>180) f1 = 180;
        if (lIndexAngle<0) f1 = -f1;
        
        
        lPrecalcAngle = (long)floor(f1);
        
        //lPrecalcAngle = lIndexAngle;
        
        // Angle process
        lFrontAngle = lPrecalcAngle;
        if (lFrontAngle> 90) lFrontAngle =  180-lFrontAngle;
        if (lFrontAngle<-90) lFrontAngle = -180-lFrontAngle;
        fAngle                  = ((float)lPrecalcAngle) * 0.0174532F;
        fFrontAngle             = ((float)lFrontAngle  ) * 0.0174532F;      //-PI/2 ... +PI/2
        fFrontAngleUnity        = ((float)lFrontAngle  ) * 0.0111111F;      //   -1 ... +1      Right(-1)...Center(0)...Left(+1)
        fFrontRearAngle         = (float)sam_ABS(lPrecalcAngle);
        fFrontRearAngleUnityAbs = fFrontRearAngle * 0.0055F;                //    0 ... +1      front(0)...rear(1)

        //
        //  -1...0...+1
        //  -6...0...+6
        //  +6...0...-1
        
        // L/R Gain                
        if (fFrontAngleUnity>=0) f1 = (float) pow (  fFrontAngleUnity, 2.0F );
        else                     f1 = (float)-pow ( -fFrontAngleUnity, 2.0F );
        f2                          = 0.15F + (float)pow(fFrontRearAngleUnityAbs,4.0F)*0.30F;
        f1                          = (float)pow ( 10.0, f1 * f2 );  // 0.707 ... 1.413
        fGainL    = f1;
        fGainR    = 1.0F/f1;
        
        fGainL = (1.0F+fGainL) * 0.5F;
        fGainR = (1.0F+fGainR) * 0.5F;
        
        //fGainL = pow ( 10.0F, ((  fFrontAngleUnity)*6.0F)/20.0F );
        //fGainR = pow ( 10.0F, (( -fFrontAngleUnity)*6.0F)/20.0F );
        
        //Initialisation du filtre de convolution
        SAM_RENDER_ConvolutionFilter_Init ( &ConvFilter );
        
        
        
        //lDelayCount = 0;
        //memset ( pRender, 0, sizeof(SAM_RENDER) );
        //memset ( &Render2K, 0, sizeof(SAM_RENDER2K) );
        pRender2K = &Render2K;
        
        //***********************************************************************
        //Le son vient de devant
        //Unités utilisées : 2x3
        fGain = (float)pow ( 1.0F-fFrontRearAngleUnityAbs, 2 );
        //fGain = 0;        
        if (fGain>=0.1F) //Jusqu'à -20dB
        {
            //fGain *= 0.3F;
            fDistanceBaseL = -20 * fFrontAngleUnity;
            fDistanceBaseR =  20 * fFrontAngleUnity;
            if (fDistanceBaseL<0)
            {
                fDistanceBaseL -= fDistanceBaseL;
                fDistanceBaseR -= fDistanceBaseL;
            }
            if (fDistanceBaseR<0)
            {
                fDistanceBaseL -= fDistanceBaseR;
                fDistanceBaseR -= fDistanceBaseR;
            }          
            
            f1 = fDistanceBaseL;
            f2 = fDistanceBaseR;

            //La convolution secondaire pour perturber la localisation centrale
            fDistanceRearL = 100 + 0;//(ui32Random&4096)?(38):(0);
            fDistanceRearR = 100 + 0; //(ui32Random&4096)?(0):(38);
            lCount = Convolution_GetCount ( Convolution_HighPass_3000, (long)fSampleRate );
            for (i=0;i<lCount;i++)
            {
                fValue = Convolution_GetValue ( Convolution_HighPass_3000, (long)fSampleRate, i ) * 1.6F; //+4 dB pour compenser le niveau
                //fValue *= 0.7F;
                
                SAM_RENDER_ConvolutionFilter_Add ( &ConvFilter, i + (long)floor( (fSampleRate / 48000.0F) * (f1) ),  fValue * fGain, 0 );
                SAM_RENDER_ConvolutionFilter_Add ( &ConvFilter, i + (long)floor( (fSampleRate / 48000.0F) * (f2) ),  fValue * fGain, 1 );
                
                //ConvolutionFilter_Add ( &ConvFilter, i + (long)floor( (fSampleRate / 48000.0F) * (f1+40) ),  fValue * fGain, 0 );
                //ConvolutionFilter_Add ( &ConvFilter, i + (long)floor( (fSampleRate / 48000.0F) * (f2+60) ),  fValue * fGain, 1 );
                
                //ConvolutionFilter_Add ( &ConvFilter, i + (long)floor( (fSampleRate / 48000.0F) * (f1+fDistanceRearL) ),  fValue * fGain, 0 );
                //ConvolutionFilter_Add ( &ConvFilter, i + (long)floor( (fSampleRate / 48000.0F) * (f2+fDistanceRearR) ),  fValue * fGain, 1 );
                /*ConvolutionFilter_Add ( &ConvFilter, i + (long)floor( (fSampleRate / 48000.0F) * (f1+fDistanceRearL) ),  fValue * fGain * (-0.4F), 0 );
                ConvolutionFilter_Add ( &ConvFilter, i + (long)floor( (fSampleRate / 48000.0F) * (f2+fDistanceRearR) ),  fValue * fGain * ( 0.4F), 1 );
                ConvolutionFilter_Add ( &ConvFilter, i + (long)floor( (fSampleRate / 48000.0F) * (f1+fDistanceRearL+40) ),  fValue * fGain * ( 0.3F), 0 );
                ConvolutionFilter_Add ( &ConvFilter, i + (long)floor( (fSampleRate / 48000.0F) * (f2+fDistanceRearR+40) ),  fValue * fGain * (-0.3F), 1 );
                ConvolutionFilter_Add ( &ConvFilter, i + (long)floor( (fSampleRate / 48000.0F) * (f1+fDistanceRearL+120) ),  fValue * fGain * (-0.2F), 0 );
                ConvolutionFilter_Add ( &ConvFilter, i + (long)floor( (fSampleRate / 48000.0F) * (f2+fDistanceRearR+120) ),  fValue * fGain * ( 0.2F), 1 );
                ConvolutionFilter_Add ( &ConvFilter, i + (long)floor( (fSampleRate / 48000.0F) * (f1+fDistanceRearL+220) ),  fValue * fGain * (-0.2F), 0 );
                ConvolutionFilter_Add ( &ConvFilter, i + (long)floor( (fSampleRate / 48000.0F) * (f2+fDistanceRearR+220) ),  fValue * fGain * ( 0.2F), 1 );
                */    
                /*pRender2K->dwDelayIndex[lDelayCount  ] = i + (long)floor( (fSampleRate / 48000.0F) * (f1+fDistanceRearL) );
                pRender2K->fDelayGain[lDelayCount  ]   = -fValue * fGain;
                pRender2K->dwDelayIndex[lDelayCount+1] = i + (long)floor( (fSampleRate / 48000.0F) * (f2+fDistanceRearR) );
                pRender2K->fDelayGain[lDelayCount+1]   = fValue * fGain;
                lDelayCount += 2;
                */
            }
            
            fDistanceRearL = 0;//(ui32Random&4096)?(38):(0);
            fDistanceRearR = 0; //(ui32Random&4096)?(0):(38);
            lCount = Convolution_GetCount ( Convolution_LowPass_6000, (long)fSampleRate );
            for (i=0;i<lCount;i++)
            {
                fValue = Convolution_GetValue ( Convolution_LowPass_6000, (long)fSampleRate, i ) * 1.6F; //+4 dB pour compenser le niveau
                //fValue *= 0.3F;

                SAM_RENDER_ConvolutionFilter_Add ( &ConvFilter, i + (long)floor( (fSampleRate / 48000.0F) * (f1+fDistanceRearL) ), fValue * fGain, 0 );
                SAM_RENDER_ConvolutionFilter_Add ( &ConvFilter, i + (long)floor( (fSampleRate / 48000.0F) * (f2+fDistanceRearR) ), fValue * fGain, 1 );

                SAM_RENDER_ConvolutionFilter_Add ( &ConvFilter, i + (long)floor( (fSampleRate / 48000.0F) * (f1+fDistanceRearL+490) ), fValue * fGain * 0.10F, 0 );
                SAM_RENDER_ConvolutionFilter_Add ( &ConvFilter, i + (long)floor( (fSampleRate / 48000.0F) * (f2+fDistanceRearR+500) ), fValue * fGain * 0.10F, 1 );

                SAM_RENDER_ConvolutionFilter_Add ( &ConvFilter, i + (long)floor( (fSampleRate / 48000.0F) * (f1+fDistanceRearL+400) ), fValue * fGain * 0.12F, 0 );
                SAM_RENDER_ConvolutionFilter_Add ( &ConvFilter, i + (long)floor( (fSampleRate / 48000.0F) * (f2+fDistanceRearR+410) ), fValue * fGain * 0.12F, 1 );

                SAM_RENDER_ConvolutionFilter_Add ( &ConvFilter, i + (long)floor( (fSampleRate / 48000.0F) * (f1+fDistanceRearL+310) ), fValue * fGain * 0.14F, 0 );
                SAM_RENDER_ConvolutionFilter_Add ( &ConvFilter, i + (long)floor( (fSampleRate / 48000.0F) * (f2+fDistanceRearR+300) ), fValue * fGain * 0.14F, 1 );

                SAM_RENDER_ConvolutionFilter_Add ( &ConvFilter, i + (long)floor( (fSampleRate / 48000.0F) * (f1+fDistanceRearL+210) ), fValue * fGain * 0.15F, 0 );
                SAM_RENDER_ConvolutionFilter_Add ( &ConvFilter, i + (long)floor( (fSampleRate / 48000.0F) * (f2+fDistanceRearR+200) ), fValue * fGain * 0.15F, 1 );
                
                //ConvolutionFilter_Add ( &ConvFilter, i + (long)floor( (fSampleRate / 48000.0F) * (f1+fDistanceRearL) ), fValue * fGain, 0 );
                //ConvolutionFilter_Add ( &ConvFilter, i + (long)floor( (fSampleRate / 48000.0F) * (f2+fDistanceRearR) ), fValue * fGain, 1 );
                
                /*pRender2K->dwDelayIndex[lDelayCount  ] = i + (long)floor( (fSampleRate / 48000.0F) * (f1+fDistanceRearL) );
                pRender2K->fDelayGain[lDelayCount  ]   = fValue * fGain;
                pRender2K->dwDelayIndex[lDelayCount+1] = i + (long)floor( (fSampleRate / 48000.0F) * (f2+fDistanceRearR) );
                pRender2K->fDelayGain[lDelayCount+1]   = fValue * fGain;
                lDelayCount += 2;
                */
            }
            
        }
        
        //***********************************************************************
        //Le son vient d'un côté
        //Unités utilisées : 2x2
        f1 = (fFrontRearAngleUnityAbs*2.0F)-1.0F;       // -1...0...+1 => front...center...rear
        fGain = 1.0F - (float)fabs ( f1 );                        // 0...+1      => front/rear...side
        if (fGain>=0.1F) //Jusqu'à -20dB
        {
        
            if (fFrontAngleUnity<0)
            {
                //Le signal est à droite, on applique le filtre à convolution à gauche et pas à droite
                fGainConvolutionL =   (float)fabs(fFrontAngleUnity);
                fGainConvolutionR = 1-(float)fabs(fFrontAngleUnity);
            }
            else
            {
                //Le signal est à gauche, on applique le filtre à convolution à droite et pas à gauche
                fGainConvolutionL = 1-(float)fabs(fFrontAngleUnity);
                fGainConvolutionR =   (float)fabs(fFrontAngleUnity);
            }
            //fGainConvolutionL = pow ( fGainConvolutionL, 4 );
            //fGainConvolutionR = pow ( fGainConvolutionR, 4 );
            fGainDirectL      = 1-fGainConvolutionL;
            fGainDirectR      = 1-fGainConvolutionR;
            
            f1 = (fFrontRearAngleUnityAbs-0.5F)*2.0F;
            if (f1<0.0F) f1 = 0.0F;
            f1 = (float)pow ( f1, 3.0F );
            
            f2 = 15 + ( 60 * f1 );
            fDistanceBaseL = -f2 * fFrontAngleUnity;
            fDistanceBaseR =  f2 * fFrontAngleUnity;
            if (fDistanceBaseL<0)
            {
                fDistanceBaseL -= fDistanceBaseL;
                fDistanceBaseR -= fDistanceBaseL;
            }
            if (fDistanceBaseR<0)
            {
                fDistanceBaseL -= fDistanceBaseR;
                fDistanceBaseR -= fDistanceBaseR;
            }          
            
            f1 = fDistanceBaseL;
            f2 = fDistanceBaseR;
            
            //ConvolutionFilter_Add ( &ConvFilter, (long)floor( (fSampleRate / 48000.0F) * f1 ), fGain * fGainDirectL, 0 );
            //ConvolutionFilter_Add ( &ConvFilter, (long)floor( (fSampleRate / 48000.0F) * f2 ), fGain * fGainDirectR, 1 );
            
            /*pRender2K->dwDelayIndex[lDelayCount  ]        = (long)floor( (fSampleRate / 48000.0F) * f1 );
            pRender2K->fDelayGain[lDelayCount    ]        = fGain * fGainDirectL;
            pRender2K->dwDelayIndex[lDelayCount+1]        = (long)floor( (fSampleRate / 48000.0F) * f2 );
            pRender2K->fDelayGain[lDelayCount+1  ]        = fGain * fGainDirectR;
            lDelayCount += 2;
            */
            
            lCount = Convolution_GetCount ( Convolution_SideEar, (long)fSampleRate );
            for (i=0;i<lCount;i++)
            {
                fValue = Convolution_GetValue ( Convolution_SideEar, (long)fSampleRate, i ) * 2.0F; //+4 dB pour compenser le niveau
                SAM_RENDER_ConvolutionFilter_Add ( &ConvFilter, i + (long)floor( (fSampleRate / 48000.0F) * (f1) ),  fGain * fGainDirectL * fValue, 0 );
                SAM_RENDER_ConvolutionFilter_Add ( &ConvFilter, i + (long)floor( (fSampleRate / 48000.0F) * (f2) ),  fGain * fGainDirectR * fValue, 1 );
            }
            
            
            lCount = Convolution_GetCount ( Convolution_LowPass_3000, (long)fSampleRate );
            for (i=0;i<lCount;i++)
            {
                fValue = Convolution_GetValue ( Convolution_LowPass_3000, (long)fSampleRate, i ) * 2.0F; //+4 dB pour compenser le niveau
                SAM_RENDER_ConvolutionFilter_Add ( &ConvFilter, i + (long)floor( (fSampleRate / 48000.0F) * f1 ), fGain * fGainConvolutionL * fValue, 0 );
                SAM_RENDER_ConvolutionFilter_Add ( &ConvFilter, i + (long)floor( (fSampleRate / 48000.0F) * f2 ), fGain * fGainConvolutionR * fValue, 1 );
                //ConvolutionFilter_Add ( &ConvFilter, 1 + i + (long)floor( (fSampleRate / 48000.0F) * f2 ), fGain * fGainConvolutionR * fValue, 1 );
                
                /*pRender2K->dwDelayIndex[lDelayCount  ]        = 1 + i + (long)floor( (fSampleRate / 48000.0F) * f1 );
                pRender2K->fDelayGain[lDelayCount    ]        = fGain * fGainConvolutionL * fValue;
                pRender2K->dwDelayIndex[lDelayCount+1]        = 1 + i + (long)floor( (fSampleRate / 48000.0F) * f2 );
                pRender2K->fDelayGain[lDelayCount+1  ]        = fGain * fGainConvolutionR * fValue;
                lDelayCount += 2;
                */
            }
            //ConvolutionFilter_Add ( &ConvFilter, (lCount/2) + (long)floor( (fSampleRate / 48000.0F) * f1 ), fGain * fGainConvolutionL, 1 );
            
        }    
        
        //Il reste 2x27 unités de libres
        
        //***********************************************************************
        //Le son vient de derrière
        fGain = (float)pow ( fFrontRearAngleUnityAbs, 4 );
        if (fGain>=0.1F) //Jusqu'à -20dB
        {   
            //fGain *= 0.3F;
            fDistanceBaseL = -8 * fFrontAngleUnity;
            fDistanceBaseR =  8 * fFrontAngleUnity;
            if (fDistanceBaseL<0)
            {
                fDistanceBaseL -= fDistanceBaseL;
                fDistanceBaseR -= fDistanceBaseL;
            }
            if (fDistanceBaseR<0)
            {
                fDistanceBaseL -= fDistanceBaseR;
                fDistanceBaseR -= fDistanceBaseR;
            }
            
            lCount = Convolution_GetCount ( convolutionHeadphonesRearL, (long)fSampleRate );
            for (i=0;i<lCount;i++)
            {
                fValueL     = Convolution_GetValue ( convolutionHeadphonesRearL, (long)fSampleRate, i ) * 7.0F; //+4 dB pour compenser le niveau
                fValueR     = Convolution_GetValue ( convolutionHeadphonesRearR, (long)fSampleRate, i ) * 7.0F; //+4 dB pour compenser le niveau
                lDistanceL  = i + (long)floor( (fSampleRate / 48000.0F) * (fDistanceBaseL) );
                lDistanceR  = i + (long)floor( (fSampleRate / 48000.0F) * (fDistanceBaseR) );
                
                SAM_RENDER_ConvolutionFilter_Add ( &ConvFilter, lDistanceL, fValueL * fGain * fGainL, 0 );
                SAM_RENDER_ConvolutionFilter_Add ( &ConvFilter, lDistanceR, fValueR * fGain * fGainR, 1 );
                
                /*
                lDistance   = (lDistanceL>lDistanceR)?(lDistanceL):(lDistanceR);
                if (lDistance<=(sam_VOICEBUFFERSTACK_MASK-2))
                {
                    pRender2K->dwDelayIndex[lDelayCount  ] = lDistanceL;
                    pRender2K->fDelayGain[lDelayCount  ]   = fValueL * fGain * fGainL;
                    pRender2K->dwDelayIndex[lDelayCount+1] = lDistanceR;
                    pRender2K->fDelayGain[lDelayCount+1]   = fValueR * fGain * fGainR;
                    lDelayCount += 2;
                }
                */
                
                /*
                if (lMode==0)
                {
                    f1 *= 1.02F;
                    if (f1>0.9F) lMode = 1;
                }
                else
                {
                    f1 *= 0.97F;
                }
                */
            }
        }        

        fGain = (float)pow ( fFrontRearAngleUnityAbs, 2 );
        fGain = 0;
        if (fGain>=0.1F) //Jusqu'à -20dB
        {
            //fGain *= 0.3F;
            fDistanceBaseL = -38 * fFrontAngleUnity;
            fDistanceBaseR =  38 * fFrontAngleUnity;
            if (fDistanceBaseL<0)
            {
                fDistanceBaseL -= fDistanceBaseL;
                fDistanceBaseR -= fDistanceBaseL;
            }
            if (fDistanceBaseR<0)
            {
                fDistanceBaseL -= fDistanceBaseR;
                fDistanceBaseR -= fDistanceBaseR;
            }          
            
            ui32Random = lPrecalcAngle;
            ui32Random = ( 1664525 * ui32Random ) + 1013904223;
            
            //Retard primaire de 4,17ms
            fDistanceBaseL += 200 + 0;
            fDistanceBaseR += 200 + 0;
            
            //La convolution primaire pour localiser le signal au centre
            lCount = Convolution_GetCount ( Convolution_HighPass_3000, (long)fSampleRate );
            
            for (i=0;i<lCount;i++)
            {
                fValue = Convolution_GetValue ( Convolution_HighPass_3000, (long)fSampleRate, i ) * 1.0F; //+4 dB pour compenser le niveau
                //fValue *= 0.2F;
                
                SAM_RENDER_ConvolutionFilter_Add ( &ConvFilter, i + (long)floor( (fSampleRate / 48000.0F) * (fDistanceBaseL) ), fValue * fGain, 0 );
                SAM_RENDER_ConvolutionFilter_Add ( &ConvFilter, i + (long)floor( (fSampleRate / 48000.0F) * (fDistanceBaseR) ), fValue * fGain, 1 );
                
                /*
                pRender2K->dwDelayIndex[lDelayCount  ] = i + (long)floor( (fSampleRate / 48000.0F) * (fDistanceBaseL) );
                pRender2K->fDelayGain[lDelayCount  ]   = fValue * fGain;
                pRender2K->dwDelayIndex[lDelayCount+1] = i + (long)floor( (fSampleRate / 48000.0F) * (fDistanceBaseR) );
                pRender2K->fDelayGain[lDelayCount+1]   = fValue * fGain;
                lDelayCount += 2;
                */
            }
            /*
            for (i=0;i<15;i++)
            {
                f1 = fLP_Convolution[i] * 0.5F;
                pRender256->dwDelayIndex[lDelayCount  ] = i + (long)floor( (fSampleRate / 48000.0F) * fDistanceBaseL );
                pRender256->fDelayGain[lDelayCount  ]   = f1 * fGain;
                pRender256->dwDelayIndex[lDelayCount+1] = i + (long)floor( (fSampleRate / 48000.0F) * fDistanceBaseR );
                pRender256->fDelayGain[lDelayCount+1]   = f1 * fGain;
                lDelayCount += 2;
            }
            */
            /*
                 80+ 0 /  80+30     => Pas assez réaliste
                100+ 0 / 100+20     => Derrière en hauteur
                120+ 0 / 120+20     => Plus loin mais moins haut
                
                80/90/192
            */
            
            
            //La convolution No2 pour perturber la localisation centrale : 1,67ms+375us
            fDistanceRearL = 80 + 0;
            fDistanceRearR = 80 + 18;
            lCount = Convolution_GetCount ( Convolution_LowPass_6000, (long)fSampleRate );
            for (i=0;i<lCount;i++)
            {
                fValue = Convolution_GetValue ( Convolution_LowPass_6000, (long)fSampleRate, i ) * 1.6F; //+4 dB pour compenser le niveau
                //fValue *= 0.5F;
                
                SAM_RENDER_ConvolutionFilter_Add ( &ConvFilter, i + (long)floor( (fSampleRate / 48000.0F) * (fDistanceBaseL+fDistanceRearL) ), fValue * fGain, 0 );
                SAM_RENDER_ConvolutionFilter_Add ( &ConvFilter, i + (long)floor( (fSampleRate / 48000.0F) * (fDistanceBaseR+fDistanceRearR) ), fValue * fGain, 1 );
                
                /*pRender2K->dwDelayIndex[lDelayCount  ] = i + (long)floor( (fSampleRate / 48000.0F) * (fDistanceBaseL+fDistanceRearL) );
                pRender2K->fDelayGain[lDelayCount  ]   = fValue * fGain;
                pRender2K->dwDelayIndex[lDelayCount+1] = i + (long)floor( (fSampleRate / 48000.0F) * (fDistanceBaseR+fDistanceRearR) );
                pRender2K->fDelayGain[lDelayCount+1]   = fValue * fGain;
                lDelayCount += 2;*/
            }

            //La convolution No3 pour perturber la localisation centrale : 2,92ms+542us
            fDistanceRearL = 140 + 38;
            fDistanceRearR = 140 + 0;
            lCount = Convolution_GetCount ( Convolution_LowPass_6000, (long)fSampleRate );
            for (i=0;i<lCount;i++)
            {
                fValue = Convolution_GetValue ( Convolution_LowPass_6000, (long)fSampleRate, i ) * 1.6F; //+4 dB pour compenser le niveau
                //fValue *= 0.7F;
                
                SAM_RENDER_ConvolutionFilter_Add ( &ConvFilter, i + (long)floor( (fSampleRate / 48000.0F) * (fDistanceBaseL+fDistanceRearL) ), fValue * fGain, 0 );
                SAM_RENDER_ConvolutionFilter_Add ( &ConvFilter, i + (long)floor( (fSampleRate / 48000.0F) * (fDistanceBaseR+fDistanceRearR) ), fValue * fGain, 1 );
//Bug détecté ici...L,R,L,L ############
                /*pRender2K->dwDelayIndex[lDelayCount  ] = i + (long)floor( (fSampleRate / 48000.0F) * (fDistanceBaseL+fDistanceRearL) );
                pRender2K->fDelayGain[lDelayCount  ]   = fValue * fGain;
                pRender2K->dwDelayIndex[lDelayCount+1] = i + (long)floor( (fSampleRate / 48000.0F) * (fDistanceBaseL+fDistanceRearR) );
                pRender2K->fDelayGain[lDelayCount+1]   = fValue * fGain;
                lDelayCount += 2;
                */
            }   
            /*
            //La convolution No4 pour perturber la localisation centrale : 3,96ms+625us
            fDistanceRearL = 190 + 30;
            fDistanceRearR = 190 + 0;
            lCount = Convolution_GetCount ( Convolution_LowPass_6000, (long)fSampleRate );
            for (i=0;i<lCount;i++)
            {
                fValue = Convolution_GetValue ( Convolution_LowPass_6000, (long)fSampleRate, i ) * 1.6F; //+4 dB pour compenser le niveau
                fValue *= 0.9F;
                pRender256->dwDelayIndex[lDelayCount  ] = i + (long)floor( (fSampleRate / 48000.0F) * (fDistanceBaseL+fDistanceRearL) );
                pRender256->fDelayGain[lDelayCount  ]   = fValue * fGain;
                pRender256->dwDelayIndex[lDelayCount+1] = i + (long)floor( (fSampleRate / 48000.0F) * (fDistanceBaseL+fDistanceRearR) );
                pRender256->fDelayGain[lDelayCount+1]   = fValue * fGain;
                lDelayCount += 2;
            }
            /*
            fDistanceRearL = 230 + 0;//(ui32Random&4096)?(38):(0);
            fDistanceRearR = 230 + 12; //(ui32Random&4096)?(0):(38);
            lCount = Convolution_GetCount ( Convolution_LowPass_6000, (long)fSampleRate );
            for (i=0;i<lCount;i++)
            {
                fValue = Convolution_GetValue ( Convolution_LowPass_6000, (long)fSampleRate, i ) * 1.6F; //+4 dB pour compenser le niveau
                fValue *= 0.7F;
                pRender256->dwDelayIndex[lDelayCount  ] = i + (long)floor( (fSampleRate / 48000.0F) * (fDistanceBaseL+fDistanceRearL) );
                pRender256->fDelayGain[lDelayCount  ]   = fValue * fGain;
                pRender256->dwDelayIndex[lDelayCount+1] = i + (long)floor( (fSampleRate / 48000.0F) * (fDistanceBaseL+fDistanceRearR) );
                pRender256->fDelayGain[lDelayCount+1]   = fValue * fGain;
                lDelayCount += 2;
            }
            */
            /*
            fDistanceRearL = 152 + 18;//(ui32Random&4096)?(38):(0);
            fDistanceRearR = 152 + 0; //(ui32Random&4096)?(0):(38);
            lCount = Convolution_GetCount ( Convolution_LowPass_6000, (long)fSampleRate );
            for (i=0;i<lCount;i++)
            {
                fValue = - Convolution_GetValue ( Convolution_LowPass_6000, (long)fSampleRate, i );
                pRender256->dwDelayIndex[lDelayCount  ] = i + (long)floor( (fSampleRate / 48000.0F) * (fDistanceBaseL+fDistanceRearL) );
                pRender256->fDelayGain[lDelayCount  ]   = -fValue * fGain;
                pRender256->dwDelayIndex[lDelayCount+1] = i + (long)floor( (fSampleRate / 48000.0F) * (fDistanceBaseL+fDistanceRearR) );
                pRender256->fDelayGain[lDelayCount+1]   = -fValue * fGain;
                lDelayCount += 2;
            }
            */
            /*
            for (i=0;i<15;i++)
            {
                f1 = fLP_Convolution[i];                
                pRender256->dwDelayIndex[lDelayCount  ] = i + (long)floor( (fSampleRate / 48000.0F) * (fDistanceBaseL+fDistanceRearL) );
                pRender256->fDelayGain[lDelayCount  ]   = f1 * fGain;
                pRender256->dwDelayIndex[lDelayCount+1] = i + (long)floor( (fSampleRate / 48000.0F) * (fDistanceBaseR+fDistanceRearR) );
                pRender256->fDelayGain[lDelayCount+1]   = f1 * fGain;
                lDelayCount += 2;
            }
            */
                          
                
            
            /*
            fDistanceBaseL = -18 * fFrontAngleUnity;
            fDistanceBaseR =  18 * fFrontAngleUnity;
            if (fDistanceBaseL<0)
            {
                fDistanceBaseL -= fDistanceBaseL;
                fDistanceBaseR -= fDistanceBaseL;
            }
            if (fDistanceBaseR<0)
            {
                fDistanceBaseL -= fDistanceBaseR;
                fDistanceBaseR -= fDistanceBaseR;
            }

            for (i=0;i<9;i++)
            {
                f1 = lRear_ConvolutionPositionL[i];                
                f1 += fDistanceBaseL;
                pRender->dwDelayIndex[lDelayCount  ] = (long)floor( (fSampleRate / 44100.0F) * f1 );
                pRender->fDelayGain[lDelayCount  ]   = fRear_ConvolutionValueL[i] * fGain * 0.707F;

                f1 = lRear_ConvolutionPositionR[i];
                f1 += fDistanceBaseR;
                pRender->dwDelayIndex[lDelayCount+1] = (long)floor( (fSampleRate / 44100.0F) * f1 );
                pRender->fDelayGain[lDelayCount+1]   = fRear_ConvolutionValueR[i] * fGain * 0.707F;
                
                lDelayCount += 2;
            }
            /*
            fDistanceBaseL = -20 * fFrontAngleUnity;
            fDistanceBaseR =  20 * fFrontAngleUnity;
            if (fDistanceBaseL<0)
            {
                fDistanceBaseL -= fDistanceBaseL;
                fDistanceBaseR -= fDistanceBaseL;
            }
            if (fDistanceBaseR<0)
            {
                fDistanceBaseL -= fDistanceBaseR;
                fDistanceBaseR -= fDistanceBaseR;
            }
            
            f1 = fDistanceBaseL;
            f2 = fDistanceBaseR;            
            pRender->dwDelayIndex[lDelayCount]          = (long)floor( (fSampleRate / 48000.0F) * f1 );
            pRender->fDelayGain[lDelayCount  ]          = fGain;
            pRender->dwDelayIndex[lDelayCount+1]        = (long)floor( (fSampleRate / 48000.0F) * f2 );
            pRender->fDelayGain[lDelayCount+1  ]        = fGain;
            */
            
            
            /*
            for (i=0;lDelayCount<64;i++)
            {
                f1 = fDistanceBaseL;
                f2 = fDistanceBaseR;            
                pRender->dwDelayIndex[lDelayCount]          = i + (long)floor( (fSampleRate / 48000.0F) * f1 );
                pRender->fDelayGain[lDelayCount  ]          = fGain * 0.16F;
                pRender->dwDelayIndex[lDelayCount+1]        = i + (long)floor( (fSampleRate / 48000.0F) * f2 );
                pRender->fDelayGain[lDelayCount+1  ]        = fGain * 0.16F;
                
                lDelayCount += 2;
                if (i>=32) break;
            }
            */
            
            
        }
        /*
        //Pour l'écoute avant, il faut baisser :
        // T=-2 à 0.80
        // T=-3 à 0.64
        
        f1 = 2.0F;
        lDistance   = (long)floor( (fSampleRate / 48000.0F) * f1 );
        fGain       = pow ( (1.0F - fFrontRearAngleUnityAbs), 10 );
        pRender->dwDelayIndex[lDelayCount]          = lDistance;
        pRender->fDelayGain[lDelayCount  ]          = fGainL * fGain * 0.40F;
        pRender->dwDelayIndex[lDelayCount+1]        = lDistance;
        pRender->fDelayGain[lDelayCount+1  ]        = fGainR * fGain * 0.40F;
        lDelayCount += 2;

        f1 = 3.0F;
        lDistance   = (long)floor( (fSampleRate / 48000.0F) * f1 );
        fGain       = pow ( (1.0F - fFrontRearAngleUnityAbs), 10 );
        pRender->dwDelayIndex[lDelayCount]          = lDistance;
        pRender->fDelayGain[lDelayCount  ]          = fGainL * fGain * 0.30F;
        pRender->dwDelayIndex[lDelayCount+1]        = lDistance;
        pRender->fDelayGain[lDelayCount+1  ]        = fGainR * fGain * 0.30F;
        lDelayCount += 2;

        fGain       = pow ( fFrontRearAngleUnityAbs, 3 );
        for (i=0;i<8;i++)
        {
            f1 = lRear_ConvolutionPositionL[i];
            pRender->dwDelayIndex[lDelayCount  ] = (long)floor( (fSampleRate / 44100.0F) * f1 );
            pRender->fDelayGain[lDelayCount  ]   = fRear_ConvolutionValueL[i] * fGainL * fGain;
            lDelayCount += 1;
            
            f1 = lRear_ConvolutionPositionR[i];
            pRender->dwDelayIndex[lDelayCount  ] = (long)floor( (fSampleRate / 44100.0F) * f1 );
            pRender->fDelayGain[lDelayCount  ]   = fRear_ConvolutionValueR[i] * fGainR * fGain;
            lDelayCount += 1;
        }


        // Filtre à T=-9
        f1 = 9.0F;
        lDistance   = (long)floor( (fSampleRate / 48000.0F) * f1 );
        fGain       = pow ( fabs(fFrontAngleUnity), 4 );
        if (fFrontAngleUnity<0)
        {
            pRender->dwDelayIndex[lDelayCount]          = lDistance;
            pRender->fDelayGain[lDelayCount  ]          = fGainL * fGain * 0.8F;
            pRender->dwDelayIndex[lDelayCount+1]        = 0;
            pRender->fDelayGain[lDelayCount+1  ]        = 0;
            
        }
        else
        {        
            pRender->dwDelayIndex[lDelayCount]          = 0;
            pRender->fDelayGain[lDelayCount  ]          = 0;
            pRender->dwDelayIndex[lDelayCount+1]        = lDistance;
            pRender->fDelayGain[lDelayCount+1  ]        = fGainR * fGain * 0.8F;
        }
        lDelayCount += 2;
        
        
        fGain = pow ( fFrontRearAngleUnityAbs, 4 );
        for (i=0;lDelayCount<64;i++)
        {
            f1 = (float)(i+160-fFrontAngleUnity*30);
            lDistance   = (long)floor( (fSampleRate / 48000.0F) * f1 );

            pRender->dwDelayIndex[lDelayCount]          = lDistance;
            pRender->fDelayGain[lDelayCount  ]          = fGainL * fGain * 0.06F;
            
            f1 = (float)(i+160+fFrontAngleUnity*30);
            lDistance   = (long)floor( (fSampleRate / 48000.0F) * f1 );
            
            pRender->dwDelayIndex[lDelayCount+1]        = lDistance;
            pRender->fDelayGain[lDelayCount+1  ]        = fGainR * fGain * 0.06F;
            
            lDelayCount += 2;
            if (i>=32) break;
        }
        */

/*
        
        

        
        
        ui32Random      = 0;    
        for (i=0;i<8;i++)
        {
            ui32Random = ( 1664525 * ui32Random ) + 1013904223;
            f2 = (float)((ui32Random>>16)&65535);
            f2 /= 65535.0F;
            f2 -= 0.5F;
            
            // Filtre à T=-126
            f1 = 108.0F + (-fFrontAngleUnity) * 8;
            f1 += f2*8;
            lDistance = (long)floor( (fSampleRate / 48000.0F) * f1 );
            fGain       = pow ( fFrontRearAngleUnityAbs, 4 );        
            pRender->dwDelayIndex[lDelayCount]          = lDistance;
            pRender->fDelayGain[lDelayCount  ]          = fGainL * fGain * 0.60F;
            lDelayCount += 1;

            ui32Random = ( 1664525 * ui32Random ) + 1013904223;
            f2 = (float)((ui32Random>>16)&65535);
            f2 /= 65535.0F;
            f2 -= 0.5F;
            
            // Filtre à T=-126
            f1 = 108.0F + (fFrontAngleUnity) * 8;
            f1 += f2*8;
            lDistance = (long)floor( (fSampleRate / 48000.0F) * f1 );
            fGain       = pow ( fFrontRearAngleUnityAbs, 4 );
            pRender->dwDelayIndex[lDelayCount]          = lDistance;
            pRender->fDelayGain[lDelayCount  ]          = fGainR * fGain * 0.60F;
            lDelayCount += 1;
        }
        
        fDistanceL = 350.0F;
        fDistanceR = 350.0F;
        fDistanceIncr = 20.0F;
        fDistanceMul = 0.97F;
        fGain = pow ( fFrontRearAngleUnityAbs, 2 ) * 0.3F;
        for (;lDelayCount<64;lDelayCount+=4)
        {
            //La distance gauche
            ui32Random = ( 1664525 * ui32Random ) + 1013904223;
            f1 = (float)((ui32Random>>16)&65535);
            f1 /= 65535.0F; //0...1
            f2 = fDistanceL + (fDistanceIncr*0.5F*f1);
            lDistance = (long)floor( (fSampleRate / 48000.0F) * f2 );
            if (lDistance>1022) break;

            //Le niveau gauche            
            ui32Random = ( 1664525 * ui32Random ) + 1013904223;
            f1 = (float)((ui32Random>>16)&65535);
            f1 /= 65535.0F; //0...1
            if (f1<0.5F) f1-=1.0F;
            
            pRender->dwDelayIndex[lDelayCount  ]          = lDistance;
            pRender->fDelayGain[lDelayCount    ]          = fGainL * fGain * f1;
            
            //La distance gauche +8
            lDistance += (long)floor( (fSampleRate / 48000.0F) * 8 );
            if (lDistance>1022) break;            
            pRender->dwDelayIndex[lDelayCount+1]          = lDistance;
            pRender->fDelayGain[lDelayCount+1  ]          = fGainL * fGain * f1;
            
            
            //La distance droite
            ui32Random = ( 1664525 * ui32Random ) + 1013904223;
            f1 = (float)((ui32Random>>16)&65535);
            f1 /= 65535.0F; //0...1
            f2 = fDistanceR + (fDistanceIncr*0.5F*f1);
            lDistance = (long)floor( (fSampleRate / 48000.0F) * f2 );                    
            if (lDistance>1022) break;
            
            //Le niveau droite
            ui32Random = ( 1664525 * ui32Random ) + 1013904223;
            f1 = (float)((ui32Random>>16)&65535);
            f1 /= 65535.0F; //0...1
            if (f1<0.5F) f1-=1.0F;
            
            pRender->dwDelayIndex[lDelayCount+2]          = lDistance;
            pRender->fDelayGain[lDelayCount+2  ]          = fGainR * fGain * f1;
            
            //La distance droite +8
            lDistance += (long)floor( (fSampleRate / 48000.0F) * 8 );
            if (lDistance>1022) break;            
            pRender->dwDelayIndex[lDelayCount+3]          = lDistance;
            pRender->fDelayGain[lDelayCount+3  ]          = fGainR * fGain * f1;
            
            fDistanceL += fDistanceIncr; 
            fDistanceR += fDistanceIncr;
            fDistanceIncr *= fDistanceMul;
            
            //ui32Random = ( 1664525 * ui32Random ) + 1013904223;
            //if (ui32Random&1024) fGainL = -fGainL;
            //if (ui32Random&2048) fGainR = -fGainR;
            //if (ui32Random&4096) fGain  = -fGain;
            
                
            f1 += f2;
            f2 *= 0.9F;
        }
*/        
//        pRender->dwEntryCount = (DWORD)lDelayCount;

/*
        // L/R Gain                
        if (fFrontAngleUnity>=0) f1 = (float) pow (  fFrontAngleUnity, 2.0F );
        else                     f1 = (float)-pow ( -fFrontAngleUnity, 2.0F );
        f2                          = 0.15F + (float)pow(fFrontRearAngleUnityAbs,4.0F)*0.30F;
        f1                          = (float)pow ( 10.0, f1 * f2 );  // 0.707 ... 1.413
        fFrontAngleUnityTable[0]    = f1;
        fFrontAngleUnityTable[1]    = 1.0F/f1;
        
        // Convert meters in samples
        fDistanceInSamples = (fDistanceWithinSpeakers / 340)*fSampleRate;
        if (fDistanceInSamples<0) fDistanceInSamples = 0;
        if (fDistanceInSamples>510) fDistanceInSamples = 510;

        // Determine base gain L/R (from front/rear)
        f1 = fFrontRearAngleUnityAbs*fFrontRearAngleUnityAbs; //pow ( f3, 2.0 );
        fGainL = 1.0F - f1;
        fGainR = 1.0F - f1;

        //Lorsque le signal est derrière, on augmente légèrement le niveau
        fGainCompensation = (float)(1+pow(fFrontRearAngleUnityAbs,2));// *8);

        f1 = fFrontAngle * 0.3183F;  //f1 = -0.5 ... +0.5
        f1 = (f1+1.0F)*0.5F;  // 0.0 ... 1.0  (left-right angle)
        lDistanceInSamples[0] = (long)((1-f1) * fDistanceInSamples);
        lDistanceInSamples[1] = (long)(   f1  * fDistanceInSamples);

        fBinauralBaseGain = 0.7F * (fFrontRearAngleUnityAbs * fFrontRearAngleUnityAbs);
        lDelayCount = 0;
        
        memset ( pRender, 0, sizeof(SAM_RENDER) );
        
        for (i=0;i<32;i++)
        {
            pRender->dwDelayIndex[lDelayCount]      = lDistanceInSamples[0];
            pRender->fDelayGain[lDelayCount]        = fFrontAngleUnityTable[0] * fGainL * fGainCompensation;
            //pRender->dwChannelIndex[lDelayCount]    = 0; // Left channel
            //f1 = log10 ( fabs ( pRender->fDelayGain[lDelayCount] ) ) * 20.0F;
            lDelayCount += 1;
        
            pRender->dwDelayIndex[lDelayCount]      = lDistanceInSamples[1];
            pRender->fDelayGain[lDelayCount]        = fFrontAngleUnityTable[1] * fGainR * fGainCompensation;
            //pRender->dwChannelIndex[lDelayCount]    = 1; // Right channel
            //f2 = log10 ( fabs ( pRender->fDelayGain[lDelayCount] ) ) * 20.0F;
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
        */
        
        //Remplissage de pRender2K

        for (i=0;i<sam_VOICEBUFFERSTACK_COUNT-1;i++)
        {
            j = (i<<1);
            pRender2K->dwDelayIndex[j  ] = i;
            pRender2K->fDelayGain[j  ]   = ConvFilter.fConvolutionLeft[i];
            pRender2K->dwDelayIndex[j+1] = i;
            pRender2K->fDelayGain[j+1]   = ConvFilter.fConvolutionRight[i];
        }
        pRender2K->dwEntryCount = sam_VOICEBUFFERSTACK_COUNT-1;
        
        /*
        
        pRender2K->dwDelayIndex[0] = 0;
        pRender2K->dwDelayIndex[1] = 0;
        pRender2K->fDelayGain[0]   = 1.0F;
        pRender2K->fDelayGain[1]   = 1.0F;
        pRender2K->dwEntryCount = 2;//sam_VOICEBUFFERSTACK_COUNT-1;
        
        */    
        //if (lDelayCount==0) lDelayCount = 2;
        //pRender2K->dwEntryCount = (DWORD)lDelayCount;
        
        
        SAM_RENDER_LimitTotalAmountDelayCount_2Kto64 ( pRender, pRender2K, 254 );
        
        //Convertion des 256 entrées en 64
        /*
        pRender->dwDelayIndex[0] = 0;
        pRender->dwDelayIndex[1] = 0;
        pRender->fDelayGain[0]   = 1.0F;
        pRender->fDelayGain[1]   = 1.0F;
        pRender->dwEntryCount = 2;//sam_VOICEBUFFERSTACK_COUNT-1;
        continue;
        */
    }


}


void SAM_RENDER_Init_2CH_Headphones_Old ( float fDistanceWithinSpeakers, float fSampleRate, SAM_RENDER254 * psamRender )
{
    long            lPrecalcAngle;
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

    // Kemar HRTF research found 1,225ms... try to have same value with 15cm or 5.9"
    fDistanceWithinSpeakers     *= 2.66F;

    // Init the binaural effect LUT for surround sound
    SAM_RENDER_InitBinaural ( fSineBinauralValue, fDistanceWithinSpeakers, fSampleRate, 1 );
    
    f1 = sam_VOICEBUFFERSTACK_MASK;
    f1 = f1 * (fSampleRate / 48000.0F);
    lMaxSamplesDistance = floor(f1);
    if (lMaxSamplesDistance>sam_VOICEBUFFERSTACK_MASK)
        lMaxSamplesDistance = sam_VOICEBUFFERSTACK_MASK;
    
    // Process -180 to +180 degrees and fill the LUT
    for (lPrecalcAngle=-180;
        lPrecalcAngle<=180;lPrecalcAngle+=5,pRender++)
    {
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
        if (fFrontAngleUnity>=0) f1 = (float) pow (  fFrontAngleUnity, 2.0F );
        else                     f1 = (float)-pow ( -fFrontAngleUnity, 2.0F );
        f2                          = 0.15F + (float)pow(fFrontRearAngleUnityAbs,4.0F)*0.30F;
        f1                          = (float)pow ( 10.0, f1 * f2 );  // 0.707 ... 1.413
        fFrontAngleUnityTable[0]    = f1;
        fFrontAngleUnityTable[1]    = 1.0F/f1;
        
        // Convert meters in samples
        fDistanceInSamples = (fDistanceWithinSpeakers / 340)*fSampleRate;
        if (fDistanceInSamples<0) fDistanceInSamples = 0;
        if (fDistanceInSamples>510) fDistanceInSamples = 510;

        // Determine base gain L/R (from front/rear)
        f1 = fFrontRearAngleUnityAbs*fFrontRearAngleUnityAbs; //pow ( f3, 2.0 );
        fGainL = 1.0F - f1;
        fGainR = 1.0F - f1;

        //Lorsque le signal est derrière, on augmente légèrement le niveau
        fGainCompensation = (float)(1+pow(fFrontRearAngleUnityAbs,2));// *8);

        f1 = fFrontAngle * 0.3183F;  //f1 = -0.5 ... +0.5
        f1 = (f1+1.0F)*0.5F;  // 0.0 ... 1.0  (left-right angle)
        lDistanceInSamples[0] = (long)((1-f1) * fDistanceInSamples);
        lDistanceInSamples[1] = (long)(   f1  * fDistanceInSamples);

        fBinauralBaseGain = 0.7F * (fFrontRearAngleUnityAbs * fFrontRearAngleUnityAbs);
        lDelayCount = 0;
        
        memset ( pRender, 0, sizeof(SAM_RENDER254) );
        
        for (i=0;i<32;i++)
        {
            pRender->dwDelayIndex[lDelayCount]      = lDistanceInSamples[0];
            pRender->fDelayGain[lDelayCount]        = fFrontAngleUnityTable[0] * fGainL * fGainCompensation;
            //pRender->dwChannelIndex[lDelayCount]    = 0; // Left channel
            //f1 = log10 ( fabs ( pRender->fDelayGain[lDelayCount] ) ) * 20.0F;
            lDelayCount += 1;
        
            pRender->dwDelayIndex[lDelayCount]      = lDistanceInSamples[1];
            pRender->fDelayGain[lDelayCount]        = fFrontAngleUnityTable[1] * fGainR * fGainCompensation;
            //pRender->dwChannelIndex[lDelayCount]    = 1; // Right channel
            //f2 = log10 ( fabs ( pRender->fDelayGain[lDelayCount] ) ) * 20.0F;
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
        pRender->dwEntryCount = (DWORD)lDelayCount;        
    }
}


