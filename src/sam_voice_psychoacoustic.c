#include "sam_header.h"
#include "sam_data.h"
#include "sam_voice.h"

float fTableAttLevelToGain[256];
long lInit = 0;

void SAM_VOICE_PsychoAcoustic_InitTable ( void )
{
    if (!lInit)
    {
        long i;
        float f1;
        
        lInit = 1;
        
        for (i=0;i<256;i++)
        {
            f1 = -0.5F * (float)i;
            f1 = (float)pow ( 10.0F, f1 * 0.05F );
            fTableAttLevelToGain[i] = f1;
        }
    }
}

long SAM_VOICE_PsychoAcoustic_GetGainData ( SAM_SFX * psamSFX, DWORD dwPosition, float * pfGlobalGain )
{
    
    /*DWORD   dwPositionPsyPoint;
    
    if (psamSFX->psyData.dwPsyPointDuration==0)
    {
        *pfGlobalGain = 1.0F;
    }
    else
    {    
        //dwPositionPsyPoint = dwPosition / psamSFX->psyData.dwPsyPointDuration;    
        dwPositionPsyPoint = ( dwPosition * psamSFX->psyData.dwPsyPointDuration_Mul_Shr10 ) >> 10;
        if (dwPositionPsyPoint>1023) dwPositionPsyPoint = 1023;
        *pfGlobalGain = fTableAttLevelToGain[psamSFX->psyData.bAttenuationLevelSensor[dwPositionPsyPoint]];
    } 
    */
    /*DWORD   dwPositionPsyPoint;
    dwPositionPsyPoint = ( dwPosition * psamSFX->psyData.dwPsyPointDuration_Mul_Shr10 ) >> 10;
    if (dwPositionPsyPoint>1023) dwPositionPsyPoint = 1023;
    *pfGlobalGain = fTableAttLevelToGain[psamSFX->psyData.bAttenuationLevelSensor[dwPositionPsyPoint]];
    */
    __asm {
                mov     esi, psamSFX
                mov     eax, dwPosition
                
                imul    eax, DWORD PTR [esi+SAM_SFX.psyData.dwPsyPointDuration_Mul_Shr10]
                mov     ebx, 1023
                
                shr     eax, 10
                xor     ecx, ecx
                
                cmp     eax, ebx
                cmova   eax, ebx
                
                mov     edi, pfGlobalGain
                mov     cl, BYTE PTR [esi+eax+SAM_SFX.psyData.bAttenuationLevelSensor]
                
                mov     edx, DWORD PTR [ecx*4+fTableAttLevelToGain]
                
                mov     [edi], edx
    }
    
   
    return 0;
}

long SAM_VOICE_PsychoAcoustic_GetLevelData ( SAM_SFX * psamSFX, DWORD dwPosition, float * pfGlobalLevel )
{
    DWORD   dwPositionPsyPoint;
    float   fLevel;
    
    if (psamSFX->psyData.dwPsyPointDuration==0)
    {
        *pfGlobalLevel = 0.0F;
    }
    else
    {    
        //dwPositionPsyPoint = dwPosition / psamSFX->psyData.dwPsyPointDuration;    
        dwPositionPsyPoint = ( dwPosition * psamSFX->psyData.dwPsyPointDuration_Mul_Shr10 ) >> 10;
        if (dwPositionPsyPoint>1023) dwPositionPsyPoint = 1023;
        fLevel          = -0.5F * (float)psamSFX->psyData.bAttenuationLevelSensor[dwPositionPsyPoint];
        *pfGlobalLevel  = fLevel;
    }
   
    return 0;
}


typedef struct {
    //SAM_VOICE       *psamVoice;
    DWORD           dwVoiceIndex;
    float           fGlobalGain;
    //float           fGlobalLevel;
} VOICEPSYLISTEN;


VOICEPSYLISTEN  VoicePsyListen[sam_VOICE_MAXCOUNT];
DWORD           dwVoicePsyListenCount;

/*

    Routine de tri
    
        //   DC -  100Hz
            120 -   90dB    => +15dB
          
        //  100 -  250Hz    
             90 -   83dB    => +7 dB
        
        //  250 -  625Hz
             83 -   80dB    => +2 dB
             
        //  625 - 1560Hz    => +2 dB
             80 -   84dB
             
        // 1560 - 3900Hz    =>  0 dB
             84 -   78dB
             
        // 3900 - fs/2Hz    => +5 dB
             78 -   92dB
    
*/
int SAM_VOICE_PsychoAcoustic_SortListProc ( const void * pElementA, const void * pElementB )
{
    VOICEPSYLISTEN  *pVoicePsyListenA;
    VOICEPSYLISTEN  *pVoicePsyListenB;

    pVoicePsyListenA = (VOICEPSYLISTEN *)pElementA;
    pVoicePsyListenB = (VOICEPSYLISTEN *)pElementB;
    
    
    if (pVoicePsyListenB->fGlobalGain>pVoicePsyListenA->fGlobalGain) return 1;
    else return -1;
    //return ( pVoicePsyListenB->fGlobalLevel - pVoicePsyListenA->fGlobalLevel ) * 10;
}


void SAM_VOICE_PsychoAcoustic_SortList ( DWORD *pdwVoiceSortIndexTable, DWORD *pdwVoiceSortIndexCount )
{
    DWORD dwIndex;
    
    qsort (
        VoicePsyListen,
        dwVoicePsyListenCount,
        sizeof(VOICEPSYLISTEN),
        SAM_VOICE_PsychoAcoustic_SortListProc );
        
    for (dwIndex=0;dwIndex<dwVoicePsyListenCount;dwIndex++)
    {
        pdwVoiceSortIndexTable[dwIndex] = VoicePsyListen[dwIndex].dwVoiceIndex;
    }
    *pdwVoiceSortIndexCount = dwVoicePsyListenCount;
}

/*
void SAM_VOICE_PsychoAcoustic_InitList_Old ( void )
{
    DWORD           dwVoiceIndex;    
    SAM_VOICE       *psamVoice;
    SAM_SFX         *psamSFX;
    float           fGlobalGain;
    float           fGlobalLevel;
    float           fTemp;
    long            lReturn;
    
    SAM_VOICE_PsychoAcoustic_InitTable ( );    
    
    dwVoicePsyListenCount = 0;
    for (dwVoiceIndex=0;dwVoiceIndex<samVoiceData.dwVoiceVirtualCount;dwVoiceIndex++)
    {
        psamVoice = &(samVoiceData.psamVoice[dwVoiceIndex]);
        if ( (psamVoice->bIsUsed) && (psamVoice->bIsPlay) )
        {
            //Information sur la voix
            //VoicePsyListen[dwVoicePsyListenCount].psamVoice     = psamVoice;
            VoicePsyListen[dwVoicePsyListenCount].dwVoiceIndex  = dwVoiceIndex;

            //Le SFX de la voix
            lReturn = SAM_SFX_GetIt ( psamVoice->dwHandleSFX, &psamSFX );
            
            //Le SFX n'est pas chargé
            if (!lReturn)
            {
                if (!psamSFX->bIsLoaded) lReturn = -1;
                if (!psamSFX->dwGranuleCount) lReturn = -1;
            }
            
            if (!lReturn)
            {
                //Gain global de la voix
                fGlobalGain = psamVoice->fLevelSFXReplayGain * samData.fGainVoice;
                fGlobalGain *= psamVoice->fFX_LevelDistanceGain;
                fGlobalGain *= psamVoice->fLevelMasterGain;
                fGlobalGain = (float)fabs ( fGlobalGain );
                
                //Niveau global
                if (fGlobalGain<=0.00001F) fGlobalLevel = -100.0F;
                else                       fGlobalLevel = (float)log10 ( fGlobalGain ) * 20.0F;
                
                //Le niveau de la voix
                lReturn = SAM_VOICE_PsychoAcoustic_GetLevelData ( 
                    psamSFX, 
                    psamVoice->dwInPlaySamplePosition,
                    &fTemp );
                    
                //On copie les niveaux
                if (!lReturn)
                    fGlobalLevel += fTemp;
                
                //VoicePsyListen[dwVoicePsyListenCount].fGlobalLevel = fGlobalLevel;
                
                //La voix psycho acoustique est ajoutée
                dwVoicePsyListenCount++;
            }
        }
    }
}
*/
void SAM_VOICE_PsychoAcoustic_InitList ( void )
{
    DWORD           dwVoiceIndex;    
    SAM_VOICE       *psamVoice;
    SAM_SFX         *psamSFX;
    float           fGlobalGain;
    float           fTemp;
    long            lReturn;
    
    SAM_VOICE_PsychoAcoustic_InitTable ( );
    
    dwVoicePsyListenCount = 0;
    for (dwVoiceIndex=0;dwVoiceIndex<samVoiceData.dwVoiceVirtualCount;dwVoiceIndex++)
    {
        psamVoice = &(samVoiceData.psamVoice[dwVoiceIndex]);
        if ( (psamVoice->bIsUsed) && (psamVoice->bIsPlay) )
        {
            //Information sur la voix
            //VoicePsyListen[dwVoicePsyListenCount].psamVoice     = psamVoice;
            VoicePsyListen[dwVoicePsyListenCount].dwVoiceIndex  = dwVoiceIndex;

            //Le SFX de la voix
            lReturn = SAM_SFX_GetIt ( psamVoice->dwHandleSFX, &psamSFX );
            
            //Le SFX n'est pas chargé
            if (!lReturn)
            {
                if (!psamSFX->bIsLoaded) lReturn = -1;
                if (!psamSFX->dwGranuleCount) lReturn = -1;
            }
            
            if (!lReturn)
            {
                //Gain global de la voix
                fGlobalGain     =  psamVoice->fLevelSFXReplayGain;                
                fGlobalGain     *= psamVoice->fFX_LevelDistanceGain;
                fGlobalGain     *= psamVoice->fLevelMasterGain;
                fGlobalGain     *= samData.fGainVoice;
                psamVoice->fOutputGainMul = fGlobalGain;
                fGlobalGain     =  (float)fabs ( fGlobalGain );
                
                //Niveau global
                //if (fGlobalGain<=0.00001F) fGlobalLevel = -100.0F;
                //else                       fGlobalLevel = (float)log10 ( fGlobalGain ) * 20.0F;
                
                //Le niveau de la voix
                SAM_VOICE_PsychoAcoustic_GetGainData (
                    psamSFX, 
                    psamVoice->dwInPlaySamplePosition,
                    &fTemp );
                    
                //On copie les niveaux
                VoicePsyListen[dwVoicePsyListenCount].fGlobalGain = fGlobalGain * fTemp;
/*                
                //Temporary deafness "Pain hearing"
                if (samData.TemporaryDeafness_lEnable)
                {
                    samData.TemporaryDeafness_dwVoiceCount += 1;
                    if (VoicePsyListen[dwVoicePsyListenCount].fGlobalGain>0.03F)
                    {
                        fTemp = VoicePsyListen[dwVoicePsyListenCount].fGlobalGain*50;
                        samData.TemporaryDeafness_dwVoiceCountOverlevel += floor(fTemp*fTemp);
                    }
                }
*/                                
                //La voix psycho acoustique est ajoutée
                dwVoicePsyListenCount++;
            }
        }
    }
}
