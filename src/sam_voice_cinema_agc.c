#include "sam_header.h"
#include "sam_data.h"
#include "sam_voice.h"

static  float   SortProc_fConst_100 = 100.0F;
static  long    SortProc_lValue;

int SAM_VOICE_CinemaAGC_SortProc_ASM ( const void * pElementA, const void * pElementB )
{
    
    

    __asm {
        mov     eax, pElementA
        mov     ebx, pElementB
        mov     esi, samVoiceData
        
        mov     ecx, [eax]          //ecx = dwIndexA
        mov     edx, [ebx]          //edx = dwIndexB
        
        imul    ecx, SIZE SAM_VOICE
        imul    edx, SIZE SAM_VOICE
        
        //add     ecx, esi
        //add     edx, esi
        fld     dword ptr SortProc_fConst_100
        
        fld     dword ptr [ecx+esi+SAM_VOICE.fDistanceMeters]
        fsub    dword ptr [edx+esi+SAM_VOICE.fDistanceMeters]
        fmulp   st(1), st(0)
        
        fistp   dword ptr SortProc_lValue
        
        mov     eax, SortProc_lValue
    }
    
}

        
        



int SAM_VOICE_CinemaAGC_SortProc_C ( const void * pElementA, const void * pElementB )
{
    SAM_VOICE *psamVoiceA, *psamVoiceB;
    DWORD dwIndexA, dwIndexB;
    
    dwIndexA = *((DWORD *)pElementA);
    dwIndexB = *((DWORD *)pElementB);
    
    psamVoiceA = &samVoiceData.psamVoice[dwIndexA];
    psamVoiceB = &samVoiceData.psamVoice[dwIndexB];

    //Convertion en cm
    return (int)floor ( (psamVoiceA->fDistanceMeters - psamVoiceB->fDistanceMeters) * 100.0F );
}

long    lTableLog2_dm[1024];
float   fTableLevelToGain[209];
long    lInitTables = 0;


void SAM_VOICE_CinemaAGC_Process ( DWORD * pdwVoiceSortIndexTable, DWORD dwActivePhysicalVoiceCount )
{
    SAM_VOICE   *psamVoice;    
    DWORD       dwIndex;
    float       fCurrentLevel;
    float       fGain;
    float       fPrevDistance;
    float       fCurrDistance;
    float       fCalcDistance;
    DWORD       dwIndexVoiceMap[256];
    long        lCurrentLevel;
    long        lCurrDistance;
    long        lPrevDistance;
    long        lCalcDistance;
    

    if (!lInitTables)
    {
        float f1;
        long i;
        
        lInitTables = 1;
        
        for (i=0;i<1024;i++)
        {
            f1 = (float)i;
            f1 *= 0.1F;
            lTableLog2_dm[i] = (long)(( log(f1+1.0F) / log(2.0F) ) * 16.0F);
        }
        
        for (i=0;i<209;i++)
        {
            f1 = (float)i;
            f1 /= 16.0F;
            f1 -= 10.0F;
            fTableLevelToGain[i] = (float)pow ( 10.0F, f1 * 0.05F );
        }
    }

    //Tri des voix dans l'ordre de distance
    //memcpy ( dwIndexVoiceMap, pdwVoiceSortIndexTable, sizeof(DWORD)*dwActivePhysicalVoiceCount );
    /*qsort (
        dwIndexVoiceMap,
        dwActivePhysicalVoiceCount,
        sizeof(DWORD),
        SAM_VOICE_CinemaAGC_SortProc_ASM );*/

    /*
    fCurrentLevel = 3.0F;     
    fCurrDistance = 1.0F;
    fPrevDistance = 1.0F;
    */
    lCurrentLevel = 3*16;    
    lCurrDistance = 20;
    lPrevDistance = 20;
            
    //Applique la rêgle
    for (dwIndex=0;dwIndex<dwActivePhysicalVoiceCount;dwIndex++)
    {
        //psamVoice = &samVoiceData.psamVoice[dwIndexVoiceMap[dwIndex]];
        psamVoice = &samVoiceData.psamVoice[pdwVoiceSortIndexTable[dwIndex]];
        
        if (psamVoice->fDistanceMeters<=2.0F)
            psamVoice->fLevelCinemaAGC = 1.0F;
        else
        {
            
            lPrevDistance = lCurrDistance;
            lCurrDistance = (long)(psamVoice->fDistanceMeters*10.0F);
            
            lCalcDistance = lCurrDistance-lPrevDistance;
            if (lCalcDistance<0) lCalcDistance = 0;
            lCalcDistance = (lCalcDistance<1024)?(lTableLog2_dm[lCalcDistance]):(lTableLog2_dm[1023]);

            //Utilise le gain précédent            
            fGain = fTableLevelToGain[lCurrentLevel+160];

            //Calcule le nouveau gain
            lCurrentLevel -= (8+lCalcDistance);
            if (lCurrentLevel<-160) lCurrentLevel = -160;
            
            if (fGain<1.0F) fGain = 0;
            
            psamVoice->fLevelCinemaAGC = fGain;
            

            /*
            fPrevDistance = fCurrDistance;
            fCurrDistance = psamVoice->fDistanceMeters;
            
            fCalcDistance = fCurrDistance-fPrevDistance;
            if (fCalcDistance<0) fCalcDistance = 0.0F;
            fCalcDistance = log(fCalcDistance+1.0F) / log(2.0F);

            //Utilise le gain précédent        
            fGain = (float)pow ( 10.0F, fCurrentLevel * 0.05F );
            
            //Calcule le nouveau niveau
            fCurrentLevel -= (0.5F+fCalcDistance);
            if (fCurrentLevel<-10.0F) fCurrentLevel = -10.0F;
            
            //if (fGain<0.316F) fGain = 0.316F;
            //if (fGain>1.414F) fGain = 1.414F;
            psamVoice->fLevelCinemaAGC = fGain;
            */
        }
    }
}
