#include "sam_header.h"
#include "sam_data.h"
#include "sam_voice.h"


void SAM_VOICE_MixSingleGranule_Null ( SAM_VOICE * psamVoice, SAM_SFX * psamSFX, float fGlobalGain, DWORD dwLoopCount, float * pfSoftwareBuffer, DWORD dwSoftwareBufferChannelCount )
{
    DWORD       dwInPlaySamplePosition;
    DWORD       dwInPlaySamplePositionPrevious;
    DWORD       dwInPlayTickPosition;
    DWORD       dwInPlayTickIncrement;
    DWORD       dwInPlayGranulePosition;

    DWORD       dwSamplesCount;

    BYTE        bFormat;
    BYTE        bLoop;

    DWORD       dwTemp;

    DWORD       dwCurrentGranuleID;
    DWORD       dwBytesPerSampleCount;
    DWORD       dwSamplesPerGranuleCount;


    QWORD       qwInPlayTickPosition;
    QWORD       qwInPlayTickIncrement;
    

    //Lecture des données en R/O
    bFormat                                     = psamSFX->bFormat;
    bLoop                                       = (unsigned char)psamVoice->bLoopMode;
    dwInPlayTickIncrement                       = psamVoice->dwInPlayTickIncrement;
    dwSamplesCount                              = psamSFX->dwSamplesCount;


    //Nombre de samples par granule
    SAM_FormatGetBytesCount ( bFormat, &dwBytesPerSampleCount );
    dwSamplesPerGranuleCount                    = sam_GRANULE_BUFFERBYTES / dwBytesPerSampleCount;

    //Lecture des données en R/W
    dwInPlaySamplePosition                      = psamVoice->dwInPlaySamplePosition;
    dwInPlaySamplePositionPrevious              = psamVoice->dwInPlaySamplePositionPrevious;
    dwInPlayTickPosition                        = psamVoice->dwInPlayTickPosition;
    dwInPlayGranulePosition                     = psamVoice->dwInPlayGranulePosition;
    dwCurrentGranuleID                          = psamVoice->dwCurrentGranuleID;
    
    //Position et Incrément sur 64 bits
    qwInPlayTickPosition                        = (QWORD)dwInPlayTickPosition;
    qwInPlayTickIncrement                       = (QWORD)dwInPlayTickIncrement;
    qwInPlayTickIncrement                       *= (QWORD)dwLoopCount;

    //On déplace le pointeur        
    qwInPlayTickPosition                        += qwInPlayTickIncrement;
    dwInPlaySamplePosition                      += (DWORD)(qwInPlayTickPosition>>24);
    dwInPlayGranulePosition                     += (DWORD)(qwInPlayTickPosition>>24);
    dwInPlayTickPosition                        =  (DWORD)(qwInPlayTickPosition&0xFFFFFF);
    
    if (bLoop) //Le SFX utilise une boucle ?
    {
        //Devons-nous boucler ?
        if (dwInPlaySamplePosition>psamSFX->dwLoopEndPositionSample)
        {
            //On se cale sur la nouvelle position
            do {
                dwTemp                  = dwInPlaySamplePosition - psamSFX->dwLoopEndPositionSample - 1;
                dwInPlaySamplePosition  = psamSFX->dwLoopBeginPositionSample + dwTemp;
                dwInPlayGranulePosition = psamSFX->dwLoopBeginGranulePositionSample + dwTemp;
            } while (dwInPlaySamplePosition>psamSFX->dwLoopEndPositionSample);
        }
        
        //Cherche le bon granule
        dwTemp = dwInPlaySamplePosition / dwSamplesPerGranuleCount;
        dwCurrentGranuleID      = psamSFX->dwGranuleFirst;
        if (dwTemp)
        {
            do {
                dwTemp--;
                SAM_GranulesGetNext ( dwCurrentGranuleID, &dwCurrentGranuleID );
            } while (dwTemp);
        }
        
        //Aligne la position dans le granule
        dwInPlayGranulePosition %= dwSamplesPerGranuleCount;
    }
    else //Le SFX n'a pas de boucle !
    {
        //Avons-nous atteint la fin de cet échantillon ?
        if (dwInPlaySamplePosition>=dwSamplesCount)
        {
            //Fin de la lecture
            dwInPlaySamplePosition      = 0;
            dwInPlayGranulePosition     = 0; //dwInPlaySamplePosition;
            psamVoice->bIsPlay          = 0;
            //psamVoice->bIsUsed        = 0;
            
            //Retourne au premier granule
            dwCurrentGranuleID          = psamSFX->dwGranuleFirst;
        }
        else
        {
            //Détermine s'il faut charger un nouveau granule
            if (dwInPlayGranulePosition>=dwSamplesPerGranuleCount)
            {
                do {
                    //On va au début du granule suivant
                    dwInPlayGranulePosition -= dwSamplesPerGranuleCount;
                
                    //Cherche le granule suivant
                    SAM_GranulesGetNext ( dwCurrentGranuleID, &dwCurrentGranuleID );
                    
                } while (dwInPlayGranulePosition>=dwSamplesPerGranuleCount);
            }
        }
    }
    

    /*
    do {
        //On déplace le pointeur d'échantillon
        dwInPlayTickPosition    += dwInPlayTickIncrement;
        dwInPlaySamplePosition  += dwInPlayTickPosition>>24;
        dwInPlayGranulePosition += dwInPlayTickPosition>>24;
        dwInPlayTickPosition    &= 0xFFFFFF;
        
        if (bLoop)
        {
            //Devons-nous boucler ?
            if (dwInPlaySamplePosition>psamSFX->dwLoopEndPositionSample)
            {
                dwTemp                  = dwInPlaySamplePosition - psamSFX->dwLoopEndPositionSample - 1;
                dwInPlaySamplePosition  = psamSFX->dwLoopBeginPositionSample + dwTemp;
                dwInPlayGranulePosition = psamSFX->dwLoopBeginGranulePositionSample + dwTemp;
                dwCurrentGranuleID      = psamSFX->dwLoopBeginGranuleID;
            }
        }
        else
        {
            //Avons-nous atteint la fin de cet échantillon ?
            if (dwInPlaySamplePosition>=dwSamplesCount)
            {
                //Fin de la lecture
                dwInPlaySamplePosition      = 0;
                dwInPlayGranulePosition     = 0; //dwInPlaySamplePosition;
                psamVoice->bIsPlay          = 0;
                //psamVoice->bIsUsed        = 0;
                
                //Retourne au premier granule
                dwCurrentGranuleID          = psamSFX->dwGranuleFirst;
                dwOutputCount               = 1;
                break;
            }
        }

        //Avons-nous atteint la fin de ce granule ?
        if (dwInPlayGranulePosition>=dwSamplesPerGranuleCount)
        {
            //On va au début du granule suivant
            dwInPlayGranulePosition -= dwSamplesPerGranuleCount;
            
            //Cherche le granule suivant
            SAM_GranulesGetNext ( dwCurrentGranuleID, &dwCurrentGranuleID );
        }

    } while (--dwOutputCount);
    */
    
    //Ecriture des données en R/W
    psamVoice->dwInPlaySamplePosition           = dwInPlaySamplePosition;
    psamVoice->dwInPlaySamplePositionPrevious   = dwInPlaySamplePositionPrevious;
    psamVoice->dwInPlayTickPosition             = dwInPlayTickPosition;
    psamVoice->dwInPlayGranulePosition          = dwInPlayGranulePosition;
    psamVoice->dwCurrentGranuleID               = dwCurrentGranuleID;
    psamVoice->fFX_IIR_LowPassValue             = 0;
    
    //Vide les piles...
    memset ( 
        psamVoice->interpolation_fStackValue,
        0,
        sizeof(float) * 16 );
    
}
