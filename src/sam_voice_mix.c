#include "sam_header.h"
#include "sam_data.h"
#include "sam_voice.h"

void SAM_VOICE_PsychoAcoustic_InitList ( void );
void SAM_VOICE_PsychoAcoustic_SortList ( DWORD *pdwVoiceSortIndexTable, DWORD *pdwVoiceSortIndexCount );

typedef struct {
    SAM_VOICE   *psamVoice;
    SAM_SFX     *psamSFX;
    float       fGlobalGain;
    long        lMaxDelayLines;
} VOICELEVELING;

VOICELEVELING VoiceLeveling[sam_VOICE_MAXCOUNT];
DWORD         dwVoiceSortIndexTable[sam_VOICE_VIRTUALCOUNT];


int SAM_VOICE_Mix_VoiceLevelingSortByLevelProc ( const void * pElementA, const void * pElementB )
{
    VOICELEVELING   *pVoiceLevelingA;
    VOICELEVELING   *pVoiceLevelingB;
    float           fGainA;
    float           fGainB;
    long            lGainA;
    long            lGainB;

    pVoiceLevelingA = (VOICELEVELING *)pElementA;
    pVoiceLevelingB = (VOICELEVELING *)pElementB;
    
    fGainA = (float)fabs ( pVoiceLevelingA->fGlobalGain );
    fGainB = (float)fabs ( pVoiceLevelingB->fGlobalGain );
    
    lGainA = (long)floor ( fGainA * 16777216.0F );
    lGainB = (long)floor ( fGainB * 16777216.0F );
    
    return lGainB - lGainA;
}


void SAM_VOICE_Mix ( float * pfSoftwareBuffer, DWORD dwSoftwareBufferChannelCount, DWORD dwSoftwareBufferSamplesToMixCount, DWORD dwInternalTimer  )
{
    DWORD           dwVoiceIndex, dwVoiceMapped;    
    SAM_VOICE       *psamVoice;
    SAM_SFX         *psamSFX;
    //long            bPlayVoice;
    long            lReturn;
    float           fGlobalGain;
    long            lIndex, lTemp, i;
    long            lVoiceLevelingCount;
    long            lTotalDelayLines;
    VOICELEVELING   *pVoiceLeveling;
    long            lStateDelayLinesA;
    long            lStateDelayLinesB;
    long            lStateDelayLinesC;
    DWORD           dwTemp;
    DWORD           dwActiveVirtualVoiceCount;
    DWORD           dwActivePhysicalVoiceCount;
    //long            bFormatType;
    //DWORD           dwBytesPerSampleCount;
    //DWORD           dwSamplesPerGranuleCount;
    long            lBiasMaxDelayLine;
    long            lBiasMaxDelayLineA;
    long            lBiasMaxDelayLineB;
    long            lBiasMaxDelayLineC;
    
    long            lTotalInterpolationSamples;
    long            lRenderPositionTarget_F16;
    long            lDistanceVariation_F16;
    long            lSensVariation;
    long            lMaxProcessorUsage;
    static char     cHistoryVoiceUse[92][64];
    static long     lHistoryVoicePos = -1;
    long            lAverageProcessorUse;
    
    float           fOldValueIIR;
    float           fNewValueIIR;
    
    
    
    void            (*pMixSingleGranule) ( SAM_VOICE * psamVoice, SAM_SFX * psamSFX, float fGlobalGain, DWORD dwLoopCount, float * pfSoftwareBuffer, DWORD dwSoftwareBufferChannelCount, DWORD dwParam );
    void            (*pMixSingleGranuleNoResamp) ( SAM_VOICE * psamVoice, SAM_SFX * psamSFX, float fGlobalGain, DWORD dwLoopCount, float * pfSoftwareBuffer, DWORD dwSoftwareBufferChannelCount, DWORD dwParam );
    
   
    pMixSingleGranule           = SAM_VOICE_MixSingleGranule_C;
    pMixSingleGranuleNoResamp   = SAM_VOICE_MixSingleGranule_C;
    
/*
#if ika_INTERNAL_AUDIO_FORMAT == sam_FORMAT_MONO_PCM16
    //if (samData.lProcessorEnableSSE)
    switch (samData.lWithoutStackMode)
    {
        case 0: //Modes : 0x21, 0x22, 0x25
            pMixSingleGranule           = SAM_VOICE_MixSingle_RenderStack;
            pMixSingleGranuleNoResamp   = SAM_VOICE_MixSingle_RenderStack_noresamp;
            break;
            
        case 2: //Modes : 0x20
            pMixSingleGranule           = SAM_VOICE_MixSingle_Render2ch;
            pMixSingleGranuleNoResamp   = SAM_VOICE_MixSingle_Render2ch_noresamp;
            break;

        case 4: //Modes : 0x23, 0x40
            pMixSingleGranule           = SAM_VOICE_MixSingle_Render4ch;
            pMixSingleGranuleNoResamp   = SAM_VOICE_MixSingle_Render4ch_noresamp;
            break;

        case 6: //Modes : 0x24, 0x60, 0x61
            pMixSingleGranule           = SAM_VOICE_MixSingle_Render6ch;
            pMixSingleGranuleNoResamp   = SAM_VOICE_MixSingle_Render6ch_noresamp;
            break;
            
        case 8: //Modes : 0x26
            pMixSingleGranule           = SAM_VOICE_MixSingle_Render8ch;
            pMixSingleGranuleNoResamp   = SAM_VOICE_MixSingle_Render8ch_noresamp;
            break;

    }
#endif
*/
    
    
    /*
        Transformation de 1024 voix virtuelles en "N" voix réelles.
        
        Pour chaque SFX, on dispose d'une table de niveaux grave, medium, aigus permettant de trier efficacement les sons.
        Les 1024 voix sont ainsi triées du son le plus fort au son le plus faible. Et on en garde que "N".
        Les voix non-traîtées réellement seront traîtées virtuellement : avancement du pointeur, bouclage...
    */
    

    //####################################################################################################
    //
    //  Calcul de l'interpolation du render
    //      Permet d'éviter certains artefacts "clic" dû à l'absence de douceur dans la
    //      variation de l'angle, spécialement pour les machines lentes.
    //  Mise à jour du SpeedShift
    lTotalInterpolationSamples = (long)dwSoftwareBufferSamplesToMixCount;
    psamVoice = samVoiceData.psamVoice;
    for (dwVoiceIndex=0;dwVoiceIndex<samVoiceData.dwVoiceVirtualCount;dwVoiceIndex++,psamVoice++)
    {
        psamVoice->bVariableRate = 0;
        
        if (!psamVoice->bIsPlay)
            continue;
            
        if ( (!psamVoice->bIsUsed) || (!psamVoice->bIsPlay) )
            continue;
            
        //La Voice a-t-elle un SFX associé ?
        lReturn = SAM_SFX_GetIt ( psamVoice->dwHandleSFX, &psamSFX );
        if (lReturn)
            continue;
            
        if (!psamSFX->bIsLoaded)
            continue;
            
        if (psamVoice->fSpeedShift>0.0F)
        {
            double f64PlayTickIncrement;
            
            psamVoice->bVariableRate = 1;
            psamVoice->dwInPlayTickIncrementBackup = psamVoice->dwInPlayTickIncrement;
            
            f64PlayTickIncrement = (double)(psamVoice->dwInPlayTickIncrement) * (double)psamVoice->fSpeedShift;
            
            if (f64PlayTickIncrement>4294967295) f64PlayTickIncrement = 4294967295;
            psamVoice->dwInPlayTickIncrement = (DWORD)SAM_MATH_lroundd ( f64PlayTickIncrement );
        }
        
            
        //La position de destination
        lRenderPositionTarget_F16 = (psamVoice->lRenderPositionTarget)<<16;
        
        //Mesure la distance de variation
        lDistanceVariation_F16 = lRenderPositionTarget_F16 - psamVoice->lRenderPositionCurrent_F16;
        
        //Choisi le sens de variation
        if (lDistanceVariation_F16==0)
        {
            //Pas de variation
            psamVoice->lRenderIncrement_F16         = 0;
            psamVoice->lRenderPositionCurrent_F16   = (psamVoice->lRenderPositionTarget)<<16;
        }
        else if (sam_ABS(lDistanceVariation_F16)<=(32<<16))
        {
            //Variation minimale par défaut
            if (lDistanceVariation_F16>0)   lSensVariation =  1;
            else                            lSensVariation = -1;
            
            psamVoice->lRenderIncrement_F16 = sam_ABS(lDistanceVariation_F16) / lTotalInterpolationSamples;
            psamVoice->lRenderIncrement_F16 <<= 4;
            psamVoice->lRenderIncrement_F16 *= lSensVariation;
        }
        else
        {
            //Exemple A : Current =  7, Target = 52     => Sens = -1
            //Exemple B : Current = 40, Target = 1      => Sens =  1
            
            if (psamVoice->lRenderPositionCurrent_F16 < lRenderPositionTarget_F16 )
            {
                //On est dans l'exemple A
                lSensVariation          = -1;    
                lDistanceVariation_F16  = psamVoice->lRenderPositionCurrent_F16 + ( 0x3FFFFF - lRenderPositionTarget_F16 );
            }
            else
            {
                //On est dans l'exemple B
                lSensVariation          =  1;
                lDistanceVariation_F16  = ( 0x3FFFFF - psamVoice->lRenderPositionCurrent_F16 ) + lRenderPositionTarget_F16;
            }
            
            psamVoice->lRenderIncrement_F16 = lDistanceVariation_F16 / lTotalInterpolationSamples;
            psamVoice->lRenderIncrement_F16 <<= 4;
            psamVoice->lRenderIncrement_F16 *= lSensVariation;
        }
    }
    
    

    
    SAM_VOICE_PsychoAcoustic_InitList ( );
    SAM_VOICE_PsychoAcoustic_SortList ( dwVoiceSortIndexTable, &dwActiveVirtualVoiceCount );
    
    //Modifie le nombre de voix physique dynamiquement si le DDL est en marche et qu'il n'y a qu'un seul CPU
    /*if ( (samData.lInstancesCount==1) && 
         (samData.dwDynamicDelayLinesCount!=0) )*/

    //Historique de l'occupation processeur des voix
    lAverageProcessorUse = SAM_MATH_lroundf ( samData.fMixerCurrentUsagePercent );
    if (lAverageProcessorUse<1) lAverageProcessorUse = 1;
    if (lHistoryVoicePos==-1)
    {
        memset ( cHistoryVoiceUse, 0, 92 * 64 );
        lHistoryVoicePos = 0;
    }
    lIndex = samVoiceData.dwVoiceRealCountCurrent;
    if (lIndex>91) lIndex=91;
    for (i=63;i>0;i--)
    {
        cHistoryVoiceUse[lIndex][i] = cHistoryVoiceUse[lIndex][i-1];
    }
    cHistoryVoiceUse[lIndex][0] = (char *)lAverageProcessorUse;
         
    //Spécifie l'occupation maximale         
    lMaxProcessorUsage = 0;
    if (samData.fMaxProcessorUsagePercent>0)
        lMaxProcessorUsage = SAM_MATH_lroundf(samData.fMaxProcessorUsagePercent);
    else
        lMaxProcessorUsage = 70;
    
    //On module le nombre de voix...    
    if (lAverageProcessorUse<lMaxProcessorUsage)
        samVoiceData.dwVoiceRealCountCurrent += 1;

    if (lAverageProcessorUse>lMaxProcessorUsage)
        samVoiceData.dwVoiceRealCountCurrent -= 1;
        
    if (samVoiceData.dwVoiceRealCountCurrent<4)
        samVoiceData.dwVoiceRealCountCurrent = 4;
        
    if (samVoiceData.dwVoiceRealCountCurrent>samVoiceData.dwVoiceRealCountInitial)
        samVoiceData.dwVoiceRealCountCurrent = samVoiceData.dwVoiceRealCountInitial;
        
    //samVoiceData.dwVoiceRealCountCurrent = 2;
    
    //Nombre de voix physique actives
    dwActivePhysicalVoiceCount = samVoiceData.dwVoiceRealCountCurrent;
    if (dwActivePhysicalVoiceCount>dwActiveVirtualVoiceCount)
        dwActivePhysicalVoiceCount = dwActiveVirtualVoiceCount;
        
    //Détermine l'occupation processeur prévisible
    lIndex = dwActivePhysicalVoiceCount;
    if (lIndex>91) lIndex=91;
    lTemp = 0;
    for (i=0;i<64;i++)
    {
        lTemp += (long)cHistoryVoiceUse[lIndex][i];
    }
    lTemp = lTemp>>6;
    
    if (lTemp>lMaxProcessorUsage)
    {
        lIndex = ( lMaxProcessorUsage * dwActivePhysicalVoiceCount ) / lTemp;
        if (lIndex<3) lIndex = 3;
        samVoiceData.dwVoiceRealCountCurrent    = lIndex;
        dwActivePhysicalVoiceCount              = lIndex;
        if (dwActivePhysicalVoiceCount>dwActiveVirtualVoiceCount)      
            dwActivePhysicalVoiceCount = dwActiveVirtualVoiceCount;
    }
    dwActivePhysicalVoiceCount = dwActiveVirtualVoiceCount;
#ifdef _DEBUG
//    _SAM_DEBUG_TEXT_ ( "Voice = %d", dwActivePhysicalVoiceCount );
#endif    
    
    /*{
    static long lCount = 0;
    lCount++;
    if (lCount>4096)
    {
        lCount = 0;
    }
    }*/
    
        
    
        
    //Application du traîtement CinemaAGC sur les voix physiques
    //SAM_VOICE_CinemaAGC_Process ( dwVoiceSortIndexTable, dwActivePhysicalVoiceCount );

    
    //samVoiceData.dwVoiceVirtualCount );
    
    
    //List les voix sans faire de tri...
    /*for (dwVoiceIndex=0;dwVoiceIndex<samVoiceData.dwVoiceVirtualCount;dwVoiceIndex++)
    {
        dwVoiceSortIndexTable[dwVoiceIndex] = dwVoiceIndex;
    }*/
    
    
    
    //if (samData.lProcessorEnableSSE2)
    //    pMixSingleGranule = SAM_VOICE_MixSingleGranule_SSE2;
    
    //Nous sommes en mode Stack... Donc Hybrid HRTF ou Holographic, on va diminuer le nombre de Delay Lines des sons lointains et laisser les proches identiques
    if ((!samData.lWithoutStackMode)&&((samData.dwDynamicDelayLinesCount)&&(samData.dwDynamicDelayLinesCount!=0xFFFFFFFE)))
    {
        lIndex    = 0;

        //Module le nombre de lignes à retard...
        if (samData.dwDynamicDelayLinesCount==0xFFFFFFFF)
        {
            dwTemp = samData.dwTotalRealVoicesCount;
            if (dwTemp<96) dwTemp = 96;
            lStateDelayLinesA = (long)(dwTemp * 1.34F);
            lStateDelayLinesB = (long)(dwTemp * 2.00F);
            lStateDelayLinesC = (long)(dwTemp * 2.67F);
        }
        else
        {
            dwTemp = samData.dwDynamicDelayLinesCount;
            if (dwTemp>100) dwTemp=100;
            lStateDelayLinesA = samData.dwTotalRealVoicesCount + (samData.dwTotalRealVoicesCount*dwTemp)/10;
            lStateDelayLinesB = (long)(lStateDelayLinesA * 1.5F);
            lStateDelayLinesC = (long)(lStateDelayLinesA * 2.0F);
        }
        
        for (dwVoiceMapped=0;dwVoiceMapped<dwActivePhysicalVoiceCount;dwVoiceMapped++)
        {
            dwVoiceIndex = dwVoiceSortIndexTable[dwVoiceMapped];
            psamVoice = &(samVoiceData.psamVoice[dwVoiceIndex]);
            
            //La Voice a-t-elle un SFX associé ?
            lReturn = SAM_SFX_GetIt ( psamVoice->dwHandleSFX, &psamSFX );
            if (lReturn)
                continue;
                
            if (!psamSFX->bIsLoaded)
                continue;                

            //Lecture de la Voice
            //Calcul du volume final
            /*
            fGlobalGain     = psamVoice->fLevelSFXReplayGain;
            fGlobalGain     *= psamVoice->fFX_LevelDistanceGain;
            fGlobalGain     *= psamVoice->fLevelMasterGain;
            //fGlobalGain     *= psamVoice->fLevelCinemaAGC;
            fGlobalGain     *= samData.fGainVoice;
            
            
            */
            VoiceLeveling[lIndex].fGlobalGain       = psamVoice->fOutputGainMul;
            VoiceLeveling[lIndex].psamVoice         = psamVoice;
            VoiceLeveling[lIndex].psamSFX           = psamSFX;
            VoiceLeveling[lIndex].lMaxDelayLines    = psamVoice->psamRender254->dwEntryCount;
            lIndex++;
        }
        
        lVoiceLevelingCount = lIndex;
        //Les données sont déjà triées
        /*qsort (
            VoiceLeveling,
            lVoiceLevelingCount,
            sizeof(VOICELEVELING),
            SAM_VOICE_Mix_VoiceLevelingSortByLevelProc );*/
            
        if (samData.dwChannelMode==0x25) lBiasMaxDelayLine = 4;
        else                             lBiasMaxDelayLine = 1;
        lBiasMaxDelayLineA = 24 * lBiasMaxDelayLine;
        lBiasMaxDelayLineB = 16 * lBiasMaxDelayLine;
        lBiasMaxDelayLineC =  8 * lBiasMaxDelayLine;
            
        lTotalDelayLines = 0;
        pVoiceLeveling = VoiceLeveling;
        for (lIndex=0;lIndex<lVoiceLevelingCount;lIndex++,pVoiceLeveling++)
        {
            //Mise à jour de l'état...
            pVoiceLeveling->psamSFX->dwAllocTimeStamp = dwInternalTimer;
                
            if (pVoiceLeveling->psamSFX->bFormat&sam_FORMAT_CHANNELMASK)
            {
                //Mode stéréo, on utilise la version en C
                SAM_VOICE_MixSingleGranule_C (
                    pVoiceLeveling->psamVoice, 
                    pVoiceLeveling->psamSFX,
                    pVoiceLeveling->fGlobalGain,
                    dwSoftwareBufferSamplesToMixCount,
                    pfSoftwareBuffer,
                    dwSoftwareBufferChannelCount );
            }
            else
            {                
                if (lTotalDelayLines>lStateDelayLinesA)
                {
                    if (pVoiceLeveling->lMaxDelayLines>lBiasMaxDelayLineA)
                        pVoiceLeveling->lMaxDelayLines = lBiasMaxDelayLineA;
                }
                
                if (lTotalDelayLines>lStateDelayLinesB)
                {
                    if (pVoiceLeveling->lMaxDelayLines>lBiasMaxDelayLineB)
                        pVoiceLeveling->lMaxDelayLines = lBiasMaxDelayLineB;
                }
                
                if (lTotalDelayLines>lStateDelayLinesC)
                {
                    if (pVoiceLeveling->lMaxDelayLines>lBiasMaxDelayLineC)
                        pVoiceLeveling->lMaxDelayLines = lBiasMaxDelayLineC;
                }
                
                //Mise à jour du nombre total de delay lines !            
                lTotalDelayLines += pVoiceLeveling->lMaxDelayLines;

                //Gestion de la douleur auditive (STEP1)
/*                
                if (samData.TemporaryDeafness_lEnable)
                {
                    fOldValueIIR = pVoiceLeveling->psamVoice->fFX_IIR_LowPassRatio;                                
                    if (samData.TemporaryDeafness_lEarPainCount>TEMPORARYDEAFNESS_MINPAIN)
                    {
                        fNewValueIIR = TEMPORARYDEAFNESS_MINPAIN / (float)samData.TemporaryDeafness_lEarPainCount;
                        //fNewValueIIR *= fNewValueIIR;
                        if (fNewValueIIR<0.1F)
                            fNewValueIIR = 0.1F;
                            
                        if (pVoiceLeveling->psamVoice->fFX_IIR_LowPassRatio>fNewValueIIR)
                            pVoiceLeveling->psamVoice->fFX_IIR_LowPassRatio = fNewValueIIR;
                    }
                }    
*/            
                pMixSingleGranule (
                    pVoiceLeveling->psamVoice,
                    pVoiceLeveling->psamSFX,
                    pVoiceLeveling->fGlobalGain,                                                
                    dwSoftwareBufferSamplesToMixCount,
                    pfSoftwareBuffer,
                    dwSoftwareBufferChannelCount,
                    pVoiceLeveling->lMaxDelayLines );
                    
                //Gestion de la douleur auditive (STEP2)                    
/*                
                if (samData.TemporaryDeafness_lEnable)
                    pVoiceLeveling->psamVoice->fFX_IIR_LowPassRatio = fOldValueIIR;    
*/
            }
        } 
    }
    else
    {
        for (dwVoiceMapped=0;dwVoiceMapped<dwActivePhysicalVoiceCount;dwVoiceMapped++)
        {
            dwVoiceIndex = dwVoiceSortIndexTable[dwVoiceMapped];
            psamVoice = &(samVoiceData.psamVoice[dwVoiceIndex]);
    
            //La Voice a-t-elle un SFX associé ?
            lReturn = SAM_SFX_GetIt ( psamVoice->dwHandleSFX, &psamSFX );
            if (lReturn)
                continue;
                
            if (!psamSFX->bIsLoaded)
                continue;
                
            //Mise à jour de l'état...
            psamSFX->dwAllocTimeStamp = dwInternalTimer;

            //Le format-type
            //bFormatType     = psamSFX->bFormat&sam_FORMAT_TYPEMASK;

            //Nombre d'octets par échantillon
            //SAM_FormatGetBytesCount ( psamSFX->bFormat, &dwBytesPerSampleCount );

            //Nombre d'échantillons par granule
            //dwSamplesPerGranuleCount = sam_GRANULE_BUFFERBYTES / dwBytesPerSampleCount;

            //Calcul de l'effet doppler
            //...

            //Traitement
            if (psamSFX->bFormat&sam_FORMAT_CHANNELMASK)
            {
                //Mode stéréo, on utilise la version en C
                SAM_VOICE_MixSingleGranule_C (
                    psamVoice, 
                    psamSFX,
                    psamVoice->fOutputGainMul,
                    dwSoftwareBufferSamplesToMixCount,
                    pfSoftwareBuffer,
                    dwSoftwareBufferChannelCount );
            }
            else
            {
            
                //Gestion de la douleur auditive (STEP1)
/*                
                if (samData.TemporaryDeafness_lEnable)
                {
                    fOldValueIIR = psamVoice->fFX_IIR_LowPassRatio;                                
                    if (samData.TemporaryDeafness_lEarPainCount>TEMPORARYDEAFNESS_MINPAIN)
                    {
                        fNewValueIIR = TEMPORARYDEAFNESS_MINPAIN / (float)samData.TemporaryDeafness_lEarPainCount;
                        //fNewValueIIR *= fNewValueIIR;
                        if (fNewValueIIR<0.1F)
                            fNewValueIIR = 0.1F;
                            
                        if (psamVoice->fFX_IIR_LowPassRatio>fNewValueIIR)
                            psamVoice->fFX_IIR_LowPassRatio = fNewValueIIR;
                    }
                }    
*/            
                            
                //Mode directstereo, on prend la version rapide
                
#if ika_INTERNAL_AUDIO_FORMAT == sam_FORMAT_MONO_PCM16                            
                if (psamVoice->lLevelGainEnable)
#else           
                if (0)
#endif     
                {
                    SAM_VOICE_MixSingle_DirectStereo (
                        psamVoice, 
                        psamSFX,
                        psamVoice->fOutputGainMul,
                        dwSoftwareBufferSamplesToMixCount,
                        pfSoftwareBuffer,
                        dwSoftwareBufferChannelCount );
                }
                else
                {                    
                    pMixSingleGranule ( 
                        psamVoice, 
                        psamSFX,
                        psamVoice->fOutputGainMul,
                        dwSoftwareBufferSamplesToMixCount,
                        pfSoftwareBuffer,
                        dwSoftwareBufferChannelCount,
                        0 );
                }
/*                
                //Gestion de la douleur auditive (STEP2)                    
                if (samData.TemporaryDeafness_lEnable)
                    psamVoice->fFX_IIR_LowPassRatio = fOldValueIIR;    
*/                    
            }
        }
    }
    
    //Traîtement des autres voix (non lues) en mode virtuel
    for (dwVoiceMapped=dwActivePhysicalVoiceCount;dwVoiceMapped<dwActiveVirtualVoiceCount;dwVoiceMapped++)
    {
        dwVoiceIndex = dwVoiceSortIndexTable[dwVoiceMapped];
        psamVoice = &(samVoiceData.psamVoice[dwVoiceIndex]);

        //La Voice a-t-elle un SFX associé ?
        lReturn = SAM_SFX_GetIt ( psamVoice->dwHandleSFX, &psamSFX );
        if (lReturn)
            continue;
            
        if (!psamSFX->bIsLoaded)
            continue;

        //Mise à jour de l'état...
        psamSFX->dwAllocTimeStamp = dwInternalTimer;
    
        SAM_VOICE_MixSingleGranule_Null (
                        psamVoice, 
                        psamSFX,
                        0,
                        dwSoftwareBufferSamplesToMixCount,
                        pfSoftwareBuffer,
                        dwSoftwareBufferChannelCount );
    }

    //####################################################################################################
    //
    //  On fixe la nouvelle position du Render...
    //
    psamVoice = samVoiceData.psamVoice;
    for (dwVoiceIndex=0;dwVoiceIndex<samVoiceData.dwVoiceVirtualCount;dwVoiceIndex++,psamVoice++)
    {
        psamVoice->lRenderIncrement_F16       = 0;
        psamVoice->lRenderPositionCurrent_F16 = (psamVoice->lRenderPositionTarget)<<16;
        
        if (psamVoice->bVariableRate)
        {
            psamVoice->dwInPlayTickIncrement = psamVoice->dwInPlayTickIncrementBackup;
        }
    }
}


/*
void SAM_VOICE_Mix ( DWORD dwSamplesToMixCount )
{
    DWORD           dwIndex;
    DWORD           dwIndexEnd;
    SAM_VOICE       *psamVoice;
    SAM_SFX         *psamSFX;
    float           fLastestValue[2];
    float           fCurrentValue[2];
    float           fProcessValue[2];
    DWORD           dwInPlaySamplePosition;         //Position dans l'échantillon
    DWORD           dwInPlaySamplePositionPrevious; //Position dans l'échantillon précédent
    BYTE            bFormatType;
    BYTE            *pAudioDataB8;
    float           *pAudioDataF32;
    DWORD           *pAudioDataXD4;

    float                   fFX_IIR_LowPassRatio;
    float                   fFX_IIR_LowPassValue;

    float                   fGlobalGain;

    float                   *stack_fBufferStackValue[sam_VOICEBUFFERSTACK_COUNT];
    DWORD                   stack_dwBufferStackPosition;
    

    DWORD           dwTotalSamplesToMixCount;
    DWORD           dwTotalVoiceToMixCount;

    DWORD           dwSamplesBytesCount;


    DWORD           dwTempA, dwTempB;
    QWORD           qwTempA, qwTempB;

    
    dwSamplesToMixOut = dwSamplesToMixCount;




    for (dwVoiceIndex=0;dwVoiceIndex<dwTotalVoiceToMixCount;dwVoiceIndex++)
    {
        //Nombre de samples à mixer sur la sortie
        dwSamplesToMixOut = dwSamplesToMixCount;

        //Sélection de la voie
        //...

        //Sélection du SFX
        lError = SAM_SFX_GetIt ( psamVoice->dwHandleSFX, &psamSFX );

        //Le SFX existe
        if (!lError)
        {
            //Le format-type
            bFormatType     = psamSFX->bFormat&sam_FORMAT_TYPEMASK;

            //Taille d'un échantillon
            SAM_FormatGetBytesCount ( psamVoice->dwHandleSFX, &dwSamplesBytesCount );

            //Nombre d'échantillons par granule
            dwGranuleSamplesCount = sam_GRANULE_BUFFERBYTES / dwSamplesBytesCount;

            //Calcule le nombre total de samples du SFX que l'on doit utiliser
            qwTempA = (QWORD)dwInPlayTickIncrement;
            qwTempA *= (QWORD)dwSamplesToMixOut;
            dwSamplesSFXToMix = (DWORD)qwTempA>>24;

            if (!psamVoice->bLoopMode)
            {
                //Le son sera-t-il arrété avant la fin du mixage car on arrive à la fin du SFX ?
                if (psamVoice->dwInPlaySamplePosition+dwSamplesSFXToMix>psamSFX->dwSamplesCount)
                {
                    //Détermine le nombre d'échantillons pour la sortie
                    dwSamplesSFXToMix   = psamSFX->dwSamplesCount - psamVoice->dwInPlaySamplePosition; //Nombre d'échantillons du SFX encore disponibles
                    qwTempA             = (QWORD)dwSamplesSFXToMix;
                    qwTempA             = (qwTempA<<24)/(QWORD)dwInPlayTickIncrement; //Convertion en Tick
                    dwSamplesToMixOut   = (DWORD)qwTempA;
                }
            }

            do {
                //Position courante dans le granule
                dwInPlayGranulePosition = psamVoice->dwInPlayGranulePosition;

                //Nombre de samples jusqu'à la fin de ce granule
                dwGranuleSamplesRemainCount = dwGranuleSamplesCount - dwInPlayGranulePosition;

                //Nombre de samples qui vont être utilisés dans ce granule
                if (dwSamplesSFXToMix>dwGranuleSamplesRemainCount) dwGranuleSamplesToMix = dwGranuleSamplesRemainCount;
                else                                               dwGranuleSamplesToMix = dwSamplesSFXToMix;

                //Détermine le nombre de Samples de sortie à mixer pour le granule
                dwSamplesToMixOutForGranule = (DWORD)((((QWORD)dwGranuleSamplesToMix)<<24)/(QWORD)dwInPlayTickIncrement);



/*
    Cette routine permet de faire un mixage sans aucun test de fin de granule ou de SFX
    C'est à l'appelant de bien calculer le "dwLoopCount"

*/

/*
                //On récupère les données
                fLastestValue[0]                    = psamVoice->resample_fLastValue[0];
                fLastestValue[1]                    = psamVoice->resample_fLastValue[1];
                dwInPlaySamplePosition              = psamVoice->dwInPlaySamplePosition;
                dwInPlaySamplePositionPrevious      = psamVoice->dwInPlaySamplePositionPrevious;
                dwInPlayTickPosition                = psamVoice->dwInPlayTickPosition;
                resample_lFIRStackPosition          = psamVoice->resample_lFIRStackPosition;
                resample_lFIRLength                 = psamVoice->resample_lFIRLength;
                fFX_IIR_LowPassRatio                = psamVoice->fFX_IIR_LowPassRatio;
                fFX_IIR_LowPassValue                = psamVoice->fFX_IIR_LowPassValue;
                stack_fBufferStackValue             = psamVoice->stack_fBufferStackValue;
                stack_dwBufferStackPosition         = psamVoice->stack_dwBufferStackPosition;

            

            if (psamSFX->bFormat&sam_FORMAT_CHANNELMASK)
            {
                //Traitement avec un son stéréo : Resampling + Volume + Mixage Stéréo
                //...
            }
            else
            {
                
                for (dwIndex=0;dwIndex<dwIndexEnd;dwIndex++)
                {






                    //On envoie sur la sortie avec le générateur de rendu
                    //...

                    //On déplace le pointeur d'échantillon
                    dwInPlayTickPosition    += dwInPlayTickIncrement;
                    dwInPlaySamplePosition  += dwInPlayTickPosition>>24;
                    dwInPlayGranulePosition += dwInPlayTickPosition>>24;
                    dwInPlayTickPosition    &= 0xFFFFFF;
                }
            }

            //On sauve les données
            psamVoice->resample_fLastValue[0]           = fLastestValue[0];
            psamVoice->resample_fLastValue[1]           = fLastestValue[1];
            psamVoice->dwInPlaySamplePosition           = dwInPlaySamplePosition;
            psamVoice->dwInPlaySamplePositionPrevious   = dwInPlaySamplePositionPrevious;
            psamVoice->resample_lFIRStackPosition       = resample_lFIRStackPosition;
            psamVoice->fFX_IIR_LowPassValue             = fFX_IIR_LowPassValue;


/*
typedef struct
{
    long                    bIsFree;                // This channel is free
    long                    bIsPlayed;              // Currently played if true
    long                    bNeedToBeKilled;        // This channel need to be killed
    long                    bNeedToSlotUpdate;      // This channel will be played on the next update slot
    long                    lAllocTime;             // Time when this sfx was created in ms
    sam_sfx_t               *pSFX;                  // SFX data
    sfxHandle_t             sfxHandle;              // Handle of the sfx used

    long                    lLoopMode;              // 0 = noloop, -1 = infinite loop
    int                     iEntityNumber;
    int                     iEntityNumberStandAlone;
    //int                     iEntityChannel;

    long                    bIsOrigin;              // vec3Origin is used if true
    long                    bIsVelocity;            // vec3Velocity is used if true
    vec3_t		            vec3Origin;
	vec3_t		            vec3Velocity;

    float                   fLevelMasterGain;       // Master level gain in dB
    float                   fLevelDistanceGain;     // Distance level gain in dB (-oo to 0dB)
    long                    lAngleView;             // Angle of sfx

	long                    bIsDoppler;             // Doppler is used if true
	float		            fDopplerScale;
	float		            fOldDopplerScale;


    uint32                  ui32TickPositionDecimal;    // Position in decimal part (24 bits used)
    float                   fTickPositionIncrement;     // Increment of the position on each sample process
    long                    lTickPosition;              // Position in the sfx
    float                   fAdditionnalLPRatio;        // Additionnal low pass ratio for FIR
    float                   fDistanceLowPassRatio;      // Low pass ratio filter (for distance filtering)
    float                   fDistanceLowPassValue;      // Low pass value filter (for distance filtering)

    float                   fBufferSaveDataLast[sam_CHBUFFER_SAVE_COUNT];  // saved samples (sam_CHBUFFER_SAVE_COUNT points)
    long                    lBufferSaveCurrentPosition; // Current save position


    float                   fFIRCoef[256];              // Coef of FIR A filter (128coef)
    float                   fFIRLast[128];              // Last data for A filter
    long                    lFIRPosition;
    float                   fFIRRatio;
    long                    lFIRTableRatioIndex;        // Ratio index

    sam_render_t            *psamRender;

} sam_channel_t;


    long                    lState;                                 // 0 = unused, 1 = allocated, 2 = inplay
    long                    bLock;                                  // 0 = unlocked, 1 = locked
    long                    lLooped;                                // 0 = no-loop, 1 = looped
    
    // Source data
    long                    lSourceFormat;                          // 0 = standard 32 bit float, 1 = XPCM
    void                    *pSourceData;
    uint32                  ui32SourceSampleCount;                     // Count of source in samples


    // FIR filter    
    float                   *pfFIRMulValues;                        // All multiplier values for the FIR filter
    long                    lFIRLength;                             // Length of the FIR filter (max = 128)
    
    // Low-pass IIR filter
    float                   fIIRLowPassRatio;                       // Low pass ratio from fixing frequency cut
    
    // Global level
    float                   fLevelGlobal;                           // Global level (in dB)
    float                   fLevelDistanceAttenuation;              // Distance attenuation level (in dB)
    
    
    // Output rendering
    sam_render_t            *psamRender;                            // Output rendering command orders
    
    
    uint32                  ui32TimeAllocated;                      // Allocated time
*/