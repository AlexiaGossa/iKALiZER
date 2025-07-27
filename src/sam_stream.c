#include "sam_header.h"
#include "sam_data.h"



    
    /*


    */
    
    DLLEXPORT   long    SAM_STREAM_SetState ( BYTE bPlayState )
    {
        _OSI_EnterCriticalSection ( &samData.samStreaming.osiCriticalSection );
        samData.samStreaming.bPlayState = bPlayState;
        _OSI_LeaveCriticalSection ( &samData.samStreaming.osiCriticalSection );
        
        return 0;
    }
    


    DLLEXPORT   long    SAM_STREAM_AddData ( DWORD dwSampleRate, DWORD dwSamplesCount, BYTE bStereo, BYTE bQuantification, void * pAudioData, float fVolume )
    {
        BYTE        bFormat;
        DWORD       dwSampleBytesCount;
        DWORD       dwStreamBytesCount;
        SAM_STREAM  *psamStream;
        DWORD       dwFoundStream;
        DWORD       dwIndex, dwStream;


        //Le format source
        switch (bQuantification)
        {
            case  0: 
            case  8: bFormat = sam_FORMAT_MONO_PCM8; break;
            case  1: 
            case 16: bFormat = sam_FORMAT_MONO_PCM16; break;
            case 32: bFormat = sam_FORMAT_MONO_FLOAT32; break;
        }
        if (bStereo) bFormat|= 0x10;

        //Taille du STREAM avant le stockage
        SAM_FormatGetBytesCount ( bFormat, &dwSampleBytesCount );
        dwStreamBytesCount = dwSamplesCount * dwSampleBytesCount;

        //Recherche un STREAM libre
        dwFoundStream = 0xFFFFFFFF;
        for (dwIndex=0;dwIndex<samData.samStreaming.dwStreamCount;dwIndex++)
        {
            dwStream = dwIndex; //(dwIndex+1+samData.samStreaming.dwCurrentStream)%samData.samStreaming.dwStreamCount;
            psamStream = &(samData.samStreaming.psamStream[dwStream]);
            if (psamStream->bUsed==0)
            {
                dwFoundStream = dwStream;     
                break;
            }
        }

        //Il n'y a plus de stream libre !
        if (dwFoundStream==0xFFFFFFFF)
            return -1;
            
        //C'est ok, on a un stream        
        psamStream = &(samData.samStreaming.psamStream[dwFoundStream]);
        memset ( psamStream, 0, sizeof(SAM_STREAM) );        
        psamStream->bFormat         = sam_FORMAT_STEREO_FLOAT32;//(bFormat&sam_FORMAT_CHANNELMASK) | sam_FORMAT_MONO_FLOAT32;
        psamStream->dwSamplesCount  = dwSamplesCount;
        psamStream->dwSampleRate    = dwSampleRate;
        psamStream->qwPositionID    = samData.samStreaming.qwCurrentPositionID;
        samData.samStreaming.qwCurrentPositionID += 1;

        //_OSI_EnterCriticalSection ( &samData.samStreaming.osiCriticalSection );

        //Allocation et copie
        if (SAM_GranulesAlloc ( 
                psamStream->dwSamplesCount,
                psamStream->bFormat,
                &psamStream->dwGranuleFirst,
                &psamStream->fReplayGain,
                pAudioData,
                bFormat,
                (float)dwSampleRate,
                NULL ))
        {
            //_OSI_LeaveCriticalSection ( &samData.samStreaming.osiCriticalSection );
            memset ( psamStream, 0, sizeof(SAM_STREAM) );
            return -2;
        }

        //_OSI_LeaveCriticalSection ( &samData.samStreaming.osiCriticalSection );

        psamStream->dwCurrentGranuleID = psamStream->dwGranuleFirst;
        psamStream->bUsed           = 1;

        return 0;
    }

    /*
                psamVoice->dwCurrentGranuleID               = psamSFX->dwGranuleFirst;
                psamVoice->dwInPlayGranulePosition          = 0;
                psamVoice->dwInPlaySamplePosition           = 0;
                psamVoice->dwInPlaySamplePositionPrevious   = 0;
                psamVoice->dwInPlayTickPosition             = 0;
                psamVoice->fFX_IIR_LowPassValue             = 0;
    */

    void SAM_STREAM_Mix ( float * pfSoftwareBuffer, DWORD dwSoftwareBufferChannelCount, DWORD dwSoftwareBufferSamplesToMixCount, DWORD dwInternalTimer )
    {
        float           fGlobalGain;

        DWORD           dwTickIncrement;
        long            lRatio;
        //long            bFormatType;
        //DWORD           dwBytesPerSampleCount;
        //DWORD           dwSamplesPerGranuleCount;
        SAM_STREAM      *psamStream;
        float           fValue;

       
        QWORD           qwLowestID;
        DWORD           dwLowestStream;
        DWORD           dwIndex;
        DWORD           dwSamplesCount;
        BYTE            bPlayState;
        
        long            lFin;
        
        float           *pfOutputBuffer;
        
        _OSI_EnterCriticalSection ( &samData.samStreaming.osiCriticalSection );
        bPlayState = samData.samStreaming.bPlayState;
        _OSI_LeaveCriticalSection ( &samData.samStreaming.osiCriticalSection );
        

        
        pfOutputBuffer  = pfSoftwareBuffer;
        
        if (!bPlayState)
            return;
            
        lFin = 0;
        
        do {
            //Recherche le plus petit ID à lire...
            qwLowestID      = 0xFFFFFFFFFFFFFFFF;
            dwLowestStream  = 0xFFFFFFFF;
            psamStream      = samData.samStreaming.psamStream;
            for (dwIndex=0;dwIndex<samData.samStreaming.dwStreamCount;dwIndex++,psamStream++)
            {
                if ((psamStream->bUsed)&&(psamStream->qwPositionID<qwLowestID))
                {
                    qwLowestID      = psamStream->qwPositionID;
                    dwLowestStream  = dwIndex;
                }
            }
            
            //On a trouvé un stream à lire ?
            if (dwLowestStream!=0xFFFFFFFF)
            {
                //Le stream
                psamStream = &(samData.samStreaming.psamStream[dwLowestStream]);
                
                //Le gain global
                fGlobalGain = psamStream->fReplayGain * samData.fGainStreaming;
                
                //Nouveau sample rate ?
                if (psamStream->dwSampleRate!=samData.samStreaming.dwCurrentSampleRate)
                {
                    _SAM_VOICE_GetPlayTick ( 
                        (float)(psamStream->dwSampleRate),
                        &dwTickIncrement ); 

                    samData.samStreaming.dwInPlayTickIncrement          = dwTickIncrement;
                    samData.samStreaming.dwInPlayTickPosition           = 0;
                    samData.samStreaming.dwCurrentSampleRate            = psamStream->dwSampleRate;
                    samData.samStreaming.dwInPlaySamplePosition         = 0;
                    samData.samStreaming.dwInPlayGranulePosition        = 0;
                    samData.samStreaming.dwInPlaySamplePositionPrevious = 0xFFFFFFFF;
                }
                
                //Lecture du stream
                dwSamplesCount = SAM_STREAM_MixSingleGranule (
                        psamStream,
                        fGlobalGain,
                        dwSoftwareBufferSamplesToMixCount,
                        pfOutputBuffer, //pfSoftwareBuffer,
                        dwSoftwareBufferChannelCount );
                        
                pfOutputBuffer += dwSamplesCount * dwSoftwareBufferChannelCount;
                dwSoftwareBufferSamplesToMixCount -= dwSamplesCount;

                //Le stream est fini ?
                if (psamStream->bUsed==2)
                {
                    //Libération du granule
                    SAM_GranulesFree ( psamStream->dwGranuleFirst );

                    //Libération du stream
                    psamStream->bUsed = 0;
                }
           
            }
            
            if (dwSoftwareBufferSamplesToMixCount<=0)
                lFin = 1;
                
            if (dwLowestStream==0xFFFFFFFF)
                lFin = 1;
            
        } while (!lFin);//(dwSoftwareBufferSamplesToMixCount>0)&&(dwLowestStream!=0xFFFFFFFF));
                        
                
                
            
        
        
        
        
        /*
        do {
            psamStream = &(samData.samStreaming.psamStream[samData.samStreaming.dwCurrentStream]);

            if (psamStream->bUsed)
            {
                fGlobalGain = psamStream->fReplayGain;

                //Nouveau sample rate ?
                if (psamStream->dwSampleRate!=samData.samStreaming.dwCurrentSampleRate)
                {
                    float fRatio;
                    //Fixe les nouvelles valeurs du FIR et du IIR de resampling
                    //fValue = (float)(psamStream->dwSampleRate);
                    _SAM_VOICE_GetPlayTickAndFIR ( 
                        (float)(psamStream->dwSampleRate),
                        &dwTickIncrement, 
                        &lRatio );
                    samData.samStreaming.resample_pfFIRCoef             = &(samData.pfFIRFastTableRatio[lRatio]);
                    samData.samStreaming.dwInPlayTickIncrement          = dwTickIncrement;
                    samData.samStreaming.dwInPlayTickPosition           = 0;
                    samData.samStreaming.dwCurrentSampleRate            = psamStream->dwSampleRate;
                    samData.samStreaming.dwInPlaySamplePosition         = 0;
                    samData.samStreaming.dwInPlayGranulePosition        = 0;
                    samData.samStreaming.dwInPlaySamplePositionPrevious = 0xFFFFFFFF;
                }
                
                dwSoftwareBufferSamplesToMixCount -= SAM_STREAM_MixSingleGranule (
                        psamStream,
                        fGlobalGain,
                        dwSoftwareBufferSamplesToMixCount,
                        pfSoftwareBuffer,
                        dwSoftwareBufferChannelCount );
                
                if (psamStream->bUsed==2)
                {
                    //Libération du granule
                    //_OSI_EnterCriticalSection ( &samData.samStreaming.osiCriticalSection );
                    SAM_GranulesFree ( psamStream->dwGranuleFirst );
                    //_OSI_LeaveCriticalSection ( &samData.samStreaming.osiCriticalSection );

                    //On passe au stream suivant
                    samData.samStreaming.dwCurrentStream = (samData.samStreaming.dwCurrentStream+1)%samData.samStreaming.dwStreamCount;
                    
                    psamStream->bUsed = 0;
                }
            }
            else
            {
                //On passe au stream suivant
                samData.samStreaming.dwCurrentStream = (samData.samStreaming.dwCurrentStream+1)%samData.samStreaming.dwStreamCount;
            }
            dwStreamCount -= 1;

        } while ((dwSoftwareBufferSamplesToMixCount>0)&&(dwStreamCount>0));

            
            

        /*            

        psamVoice           = samVoiceData.psamVoice;
        for (dwVoiceIndex=0;dwVoiceIndex<samVoiceData.dwVoiceCount;dwVoiceIndex++,psamVoice++)
        {
            //La voice est utilisée est lue ?
            if ((psamVoice->bIsUsed)&&(psamVoice->bIsPlay))
            {
                //Par défaut, on doit lire la Voice
                bPlayVoice = 1;

                //La Voice a-t-elle un SFX associé ?
                lReturn = SAM_SFX_GetIt ( psamVoice->dwHandleSFX, &psamSFX );
                if (lReturn) bPlayVoice = 0;

                //Lecture de la Voice
                if (bPlayVoice)
                {
                    //Mise à jour de l'état...
                    psamSFX->dwAllocTimeStamp = dwInternalTimer;

                    //Calcul du volume final
                    fGlobalGain = psamVoice->fLevelSFXReplayGain;
                    fGlobalGain *= psamVoice->fFX_LevelDistanceGain;
                    fGlobalGain *= psamVoice->fLevelMasterGain;

                    //Calcul de l'effet doppler
                    //...


                    //Traitement
                    SAM_VOICE_MixSingleGranule ( 
                        psamVoice, 
                        psamSFX,
                        fGlobalGain,
                        dwSoftwareBufferSamplesToMixCount,
                        pfSoftwareBuffer,
                        dwSoftwareBufferChannelCount );

               
                }
            }
        }
        */
    }


DWORD SAM_STREAM_MixSingleGranule ( SAM_STREAM * psamStream, float fGlobalGain, DWORD dwLoopCount, float * pfSoftwareBuffer, DWORD dwSoftwareBufferChannelCount )
{
    float       fProcessValue[2];
    DWORD       dwInPlaySamplePosition;
    DWORD       dwInPlaySamplePositionPrevious;
    DWORD       dwInPlayTickPosition;
    DWORD       dwInPlayTickIncrement;
    DWORD       dwInPlayGranulePosition;
    float       *interpolation_fStackValue;
    float       *pfInterpolator;


    DWORD       dwSamplesCount;
    BYTE        bFormat;

    DWORD       dwTemp;
    float       *pAudioDataF32;

    
    DWORD       dwCurrentGranuleID;
    DWORD       dwBytesPerSampleCount;
    DWORD       dwSamplesPerGranuleCount;

    float       *pfBufferOut;

    long        i, j;

    DWORD       dwOutputCount;
    DWORD       dwMixedCount;

    //Lecture des données en R/O
    dwInPlayTickIncrement                       = samData.samStreaming.dwInPlayTickIncrement;
    dwSamplesCount                              = psamStream->dwSamplesCount;
    bFormat                                     = psamStream->bFormat;
    pfBufferOut                                 = pfSoftwareBuffer;

    //Nombre de samples par granule
    SAM_FormatGetBytesCount ( bFormat, &dwBytesPerSampleCount );
    dwSamplesPerGranuleCount                    = sam_GRANULE_BUFFERBYTES / dwBytesPerSampleCount;

    //Lecture des données en R/W
    dwInPlaySamplePosition                      = samData.samStreaming.dwInPlaySamplePosition;
    dwInPlaySamplePositionPrevious              = samData.samStreaming.dwInPlaySamplePositionPrevious;
    dwInPlayTickPosition                        = samData.samStreaming.dwInPlayTickPosition;
    dwInPlayGranulePosition                     = samData.samStreaming.dwInPlayGranulePosition;
    dwCurrentGranuleID                          = psamStream->dwCurrentGranuleID;
    interpolation_fStackValue                   = samData.samStreaming.interpolation_fStackValue;

    //Les données du granule en cours...
    SAM_GranulesGetData ( dwCurrentGranuleID, &pAudioDataF32 );
    dwOutputCount = dwLoopCount;
    dwMixedCount = 0;
    

    //Traitement avec un son stéréo : Resampling + Volume + Mixage Stéréo
    do {
        dwMixedCount+= 1;

        //*******************************************************
        //Devons-nous charger une nouvelle donnée ?
        if (dwInPlaySamplePosition!=dwInPlaySamplePositionPrevious)
        {
            for (i=0;i<15;i++)
            {
                interpolation_fStackValue[i   ] = interpolation_fStackValue[i+1 ];
                interpolation_fStackValue[i+16] = interpolation_fStackValue[i+17];
            }
            interpolation_fStackValue[15] = pAudioDataF32[(dwInPlayGranulePosition<<1)  ];
            interpolation_fStackValue[31] = pAudioDataF32[(dwInPlayGranulePosition<<1)+1];
            
            dwInPlaySamplePositionPrevious = dwInPlaySamplePosition;
        }
        
        //*******************************************************
        // Génération de la phase sur 10 bits
        dwTemp = (dwInPlayTickPosition>>14)&0x3FF;

        //*******************************************************
        // Application de l'interpolation
        fProcessValue[0] = 0;
        fProcessValue[1] = 0;
        pfInterpolator = &samData_f32InterpolationData_1024_16[dwTemp<<4];
        for (i=0;i<16;i++)
        {
            fProcessValue[0] += interpolation_fStackValue[i   ] * pfInterpolator[i];
            fProcessValue[1] += interpolation_fStackValue[i+16] * pfInterpolator[i];
        }
        
        //*******************************************************
        //Application du gain final et stockage
        pfBufferOut[samData.dwChannelIndexLeft ] += fProcessValue[0] * fGlobalGain;
        pfBufferOut[samData.dwChannelIndexRight] += fProcessValue[1] * fGlobalGain;

        //On déplace le pointeur d'échantillon
        dwInPlayTickPosition    += dwInPlayTickIncrement;
        dwInPlaySamplePosition  += dwInPlayTickPosition>>24;
        dwInPlayGranulePosition += dwInPlayTickPosition>>24;
        dwInPlayTickPosition    &= 0xFFFFFF;

        //Avons-nous atteint la fin de cet échantillon ?
        if (dwInPlaySamplePosition>=dwSamplesCount)
        {
            //Fin de la lecture
            dwInPlaySamplePosition          = 0;
            dwInPlayGranulePosition         = 0;
            //dwInPlaySamplePositionPrevious  = 0xFFFFFFFF;

            //Le stream a été lu... il peut être détruit
            psamStream->bUsed       = 2;

            dwOutputCount           = 1;
            break;
        }

        //Avons-nous atteint la fin de ce granule ?
        if (dwInPlayGranulePosition>=dwSamplesPerGranuleCount)
        {
            //On va au début du granule suivant
            dwInPlayGranulePosition -= dwSamplesPerGranuleCount;
            
            //Cherche le granule suivant
            SAM_GranulesGetNext ( dwCurrentGranuleID, &dwCurrentGranuleID );

            //Mise à jour des pointeurs de données sources
            SAM_GranulesGetData ( dwCurrentGranuleID, &pAudioDataF32 );
        }

        pfBufferOut += dwSoftwareBufferChannelCount;
    } while (--dwOutputCount);


    //Ecriture des données en R/W
    samData.samStreaming.dwInPlaySamplePosition             = dwInPlaySamplePosition;
    samData.samStreaming.dwInPlaySamplePositionPrevious     = dwInPlaySamplePositionPrevious;
    samData.samStreaming.dwInPlayTickPosition               = dwInPlayTickPosition;
    samData.samStreaming.dwInPlayGranulePosition            = dwInPlayGranulePosition;
    psamStream->dwCurrentGranuleID                          = dwCurrentGranuleID;

    return dwMixedCount;
}
