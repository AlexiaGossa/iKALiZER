#include "sam_header.h"
#include "sam_data.h"
#include "sam_voice.h"


//Tables d'angles pour les modes multi-canaux
typedef struct {
    long lChannel;
    long lAngleLevelMiniA;
    long lAngleLevelMaxi;
    long lAngleLevelMiniB;
    float fGainComp;
} SAM_RENDER_MULTICHANNEL_MATRIX;

void SAM_RENDER_Init_MultiChannel ( SAM_RENDER_MULTICHANNEL_MATRIX *psrmcMatrixUser, SAM_RENDER254 * psamRender, long lTotalChannelCount );

SAM_RENDER_MULTICHANNEL_MATRIX srmcMatrix5dot1[]    = { {  0,    0,   30,  110, 1.0F }, //Left
                                                        {  1, -110,  -30,    0, 1.0F }, //Right
                                                        {  2,  -30,    0,   30, 1.0F }, //Center
                                                        {  3, -180,    0,  180, 1.0F }, //LFE 
                                                        {  4,   30,  110,  250, 1.0F }, //SR-Left
                                                        {  5,  -30, -110, -250, 1.0F }, //SR-Right
                                                        { -1 } };

SAM_RENDER_MULTICHANNEL_MATRIX srmcMatrix5dot0[]    = { {  0,    0,   30,  110, 1.0F }, //Left
                                                        {  1, -110,  -30,    0, 1.0F }, //Right
                                                        {  2,  -30,    0,   30, 1.0F }, //Center
                                                        {  4,   30,  110,  250, 1.0F }, //SR-Left
                                                        {  5,  -30, -110, -250, 1.0F }, //SR-Right
                                                        { -1 } };

SAM_RENDER_MULTICHANNEL_MATRIX srmcMatrixQuad[]     = { {  0,  -30,   30,  135, 1.0F }, //Left
                                                        {  1, -135,  -30,   30, 1.0F }, //Right
                                                        {  2,   30,  135,  225, 1.0F }, //Rear-Left
                                                        {  3,  -30, -135, -225, 1.0F }, //Rear-Right
                                                        { -1 } };

SAM_RENDER_MULTICHANNEL_MATRIX srmcMatrix0x26[]     = { {  2,    0,   55,  110, 1.0F }, //Front-Left
                                                        {  3, -110,  -55,    0, 1.0F }, //Front-Right
                                                        {  4,  -55,    0,   55, 1.0F }, //Front-Center
                                                        {  5,  110,  180,  250, 1.0F }, //Rear-Center 
                                                        {  6,   45,  110,  180, 1.0F }, //Rear-Left
                                                        {  7,  -45, -110, -180, 1.0F }, //Rear-Right
                                                        { -1 } };


void SAM_RENDER_Init_6CH_L_R_C_LFE_SL_SR ( float fDistanceWithinSpeakers, float fSampleRate, SAM_RENDER254 * psamRender )
{
    SAM_RENDER_Init_MultiChannel ( 
        srmcMatrix5dot0,
        psamRender,
        6 );
}








void SAM_RENDER_DecodePrepare_0x26 ( void );

void SAM_RENDER_Init_8CH_0x26 ( float fDistanceWithinSpeakers, float fSampleRate, SAM_RENDER254 * psamRender )
{
    SAM_RENDER_Init_MultiChannel ( 
        srmcMatrix0x26,
        psamRender,
        8 );
        
    SAM_RENDER_DecodePrepare_0x26 ( );
}

void SAM_RENDER_Init_6CH_L_R_C_noLFE_SL_SR ( float fDistanceWithinSpeakers, float fSampleRate, SAM_RENDER254 * psamRender )
{
    SAM_RENDER_Init_MultiChannel ( 
        srmcMatrix5dot0,
        psamRender,
        6 );
}

void SAM_RENDER_Init_4CH_L_R_SL_SR ( float fDistanceWithinSpeakers, float fSampleRate, SAM_RENDER254 * psamRender )
{
    SAM_RENDER_Init_MultiChannel ( 
        srmcMatrixQuad,
        psamRender,
        4 );
}









void SAM_RENDER_Init_MultiChannel ( SAM_RENDER_MULTICHANNEL_MATRIX *psrmcMatrixUser, SAM_RENDER254 * psamRender, long lTotalChannelCount )
{
    long            lPrecalcAngle;
    long            lAngleMOD64;
    SAM_RENDER254   *pRender;
    float           fGain, fGainIncr;
    long            lAngle;
    long            lTemp;
    
    SAM_RENDER_MULTICHANNEL_MATRIX  *psrmcMatrix;

    long lAngleBase;
    long lAngleIncr;
    long lAngleStop;
    
    //En cas d'erreur...
    if (!psamRender)
        return;
        
    pRender = psamRender;

    // Process -180 to +180 degrees and fill the LUT
    for (lAngleMOD64=0;lAngleMOD64<64;lAngleMOD64++,pRender++)
    {
        lPrecalcAngle = ((lAngleMOD64*45)/8) - 180;
        memset ( pRender, 0, sizeof(SAM_RENDER254) );
        pRender->dwEntryCount = lTotalChannelCount;
    }

    for (psrmcMatrix=psrmcMatrixUser;psrmcMatrix->lChannel!=-1;psrmcMatrix++)
    {
        //Première partie du rendu
        lAngleBase = (psrmcMatrix->lAngleLevelMiniA*8)/45;
        lAngleStop = (psrmcMatrix->lAngleLevelMaxi *8)/45;
        lAngleIncr = (lAngleBase<lAngleStop)?(1):(-1);
        
        fGain      = 0.0F;
        fGainIncr  = 1.0F / (float)abs(lAngleBase-lAngleStop);

        lAngle = lAngleBase;
        do {
            //Position du pRender
            lAngleMOD64 = lAngle;
            lTemp       = -lAngleMOD64;
            if (lTemp<0) lTemp += 64;
            lTemp       = lTemp&63;
            lTemp       = (32-lTemp)&63;
            pRender     = psamRender + lTemp; //((-lAngleMOD64)&63);

            //On applique le gain
            pRender->fDelayGain[psrmcMatrix->lChannel] = fGain * psrmcMatrix->fGainComp;
            
            lAngle += lAngleIncr;
            fGain  += fGainIncr;
        } while ((lAngle-lAngleIncr)!=lAngleStop);
        
        //Deuxième partie du rendu
        lAngleBase = (psrmcMatrix->lAngleLevelMaxi *8)/45;
        lAngleStop = (psrmcMatrix->lAngleLevelMiniB*8)/45;
        lAngleIncr = (lAngleBase<lAngleStop)?(1):(-1);
        
        fGain      = 1.0F;
        fGainIncr  = -1.0F / (float)abs(lAngleBase-lAngleStop);

        lAngle = lAngleBase;        
        do {
            //Position du pRender
            lAngleMOD64 = lAngle;
            lTemp       = -lAngleMOD64;
            if (lTemp<0) lTemp += 64;
            lTemp       = lTemp&63;
            lTemp       = (32-lTemp)&63;
            pRender     = psamRender + lTemp; //+ ((-lAngleMOD64)&63);
            
            //On applique le gain
            pRender->fDelayGain[psrmcMatrix->lChannel] = fGain * psrmcMatrix->fGainComp;
            
            lAngle += lAngleIncr;
            fGain  += fGainIncr;
        } while ((lAngle-lAngleIncr)!=lAngleStop);
    }
}        

void SAM_RENDER_Init_MultiChannel_old ( SAM_RENDER_MULTICHANNEL_MATRIX *psrmcMatrixUser, SAM_RENDER254 * psamRender, long lTotalChannelCount )
{
    long            lPrecalcAngle;
    long            lAngleMOD64;
    SAM_RENDER254   *pRender;
    float           fGain, fGainIncr;
    long            lAngle, lAngleDiv5;
    
    SAM_RENDER_MULTICHANNEL_MATRIX  *psrmcMatrix;

    long lAngleBase;
    long lAngleIncr;
    long lAngleStop;
    
    //En cas d'erreur...
    if (!psamRender)
        return;
        
    pRender = psamRender;

    // Process -180 to +180 degrees and fill the LUT
    for (lAngleMOD64=0;lAngleMOD64<64;lAngleMOD64++,pRender++)
    {
        lPrecalcAngle = ((lAngleMOD64*45)/8) - 180;
        memset ( pRender, 0, sizeof(SAM_RENDER254) );
        pRender->dwEntryCount = lTotalChannelCount;
    }

    for (psrmcMatrix=psrmcMatrixUser;psrmcMatrix->lChannel!=-1;psrmcMatrix++)
    {
        //Première partie du rendu
        lAngleBase = psrmcMatrix->lAngleLevelMiniA;
        lAngleIncr = (psrmcMatrix->lAngleLevelMiniA<psrmcMatrix->lAngleLevelMaxi)?(5):(-5);
        lAngleStop = psrmcMatrix->lAngleLevelMaxi;
        fGain      = 0.0F;
        fGainIncr  = 5.0F / (float)abs(lAngleBase-lAngleStop);

        lAngle = lAngleBase;
        do {
            //Position du pRender
            lAngleDiv5 = lAngle;
            if (lAngleDiv5>= 180) lAngleDiv5 -= 360;
            if (lAngleDiv5< -180) lAngleDiv5 += 360;
            lAngleDiv5 = lAngleDiv5+180;
            //lAngleDiv5 = lAngleDiv5/5;
            lAngleDiv5 = (lAngleDiv5*8)/45;
            pRender = psamRender+(lAngleDiv5&63);
            
            //On applique le gain
            pRender->fDelayGain[psrmcMatrix->lChannel] = fGain * psrmcMatrix->fGainComp;
            
            lAngle += lAngleIncr;
            fGain  += fGainIncr;
        } while ((lAngle-lAngleIncr)!=lAngleStop);
        
        //Deuxième partie du rendu
        lAngleBase = psrmcMatrix->lAngleLevelMaxi;
        lAngleIncr = (psrmcMatrix->lAngleLevelMaxi<psrmcMatrix->lAngleLevelMiniB)?(5):(-5);
        lAngleStop = psrmcMatrix->lAngleLevelMiniB;
        fGain      = 1.0F;
        fGainIncr  = -5.0F / (float)abs(lAngleBase-lAngleStop);

        lAngle = lAngleBase;        
        do {
            //Position du pRender
            lAngleDiv5 = lAngle;
            if (lAngleDiv5>= 180) lAngleDiv5 -= 360;
            if (lAngleDiv5< -180) lAngleDiv5 += 360;
            lAngleDiv5 = lAngleDiv5+180;
            //lAngleDiv5 = lAngleDiv5/5;
            lAngleDiv5 = (lAngleDiv5*8)/45;
            pRender = psamRender+(lAngleDiv5&63);
            
            //On applique le gain
            pRender->fDelayGain[psrmcMatrix->lChannel] = fGain * psrmcMatrix->fGainComp;
            
            lAngle += lAngleIncr;
            fGain  += fGainIncr;
        } while ((lAngle-lAngleIncr)!=lAngleStop);
    }
}        


