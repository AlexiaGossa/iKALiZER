#include "sam_header.h"
#include "sam_data.h"


    void SAM_MUSIC_ApplyNewSampleRate ( DWORD dwNewSampleRate )
    {
        DWORD   dwTickIncrement;
        
        if (dwNewSampleRate!=samData.samMusic.dwCurrentSampleRate)
        {
            _SAM_VOICE_GetPlayTick ( 
                (float)dwNewSampleRate,
                &dwTickIncrement ); 

            //samData.samMusic.resample_pfFIRCoef             = &(samData.pfFIRFastTableRatio[lRatio]);
            samData.samMusic.dwInPlayTickIncrement          = dwTickIncrement;
            samData.samMusic.dwInPlayTickPosition           = 0;
            samData.samMusic.dwCurrentSampleRate            = dwNewSampleRate;
            samData.samMusic.dwInPlaySamplePosition         = 0;
            samData.samMusic.dwInPlayGranulePosition        = 0;
            samData.samMusic.dwInPlaySamplePositionPrevious = 0xFFFFFFFF;
        }
    }

    void SAM_MUSIC_Mix ( float * pfSoftwareBuffer, DWORD dwSoftwareBufferChannelCount, DWORD dwSoftwareBufferSamplesToMixCount, DWORD dwInternalTimer )
    {
        float           fGlobalGain;

        DWORD           dwTickIncrement;
        long            lRatio;
        float           fValue;
       
        QWORD           qwLowestID;
        DWORD           dwLowestStream;
        DWORD           dwIndex;
        DWORD           dwSamplesCount;
        BYTE            bPlayState;
        
        long            lFin;
        long            lContinue;
        
        float           *pfOutputBuffer;
        SAM_SFX_MUSIC   *psamSFXMusic;
        SAM_SFX         *psamSFX;
        
        
        pfOutputBuffer  = pfSoftwareBuffer;
        
        _OSI_EnterCriticalSection ( &samData.samMusic.osiCriticalSection );
        
        if (samData.samMusic.dwEntryPlayed==0xFFFFFFFF)
        {
            _OSI_LeaveCriticalSection ( &samData.samMusic.osiCriticalSection );
            return;
        }
            
        lFin = 0;
        
        do {
            //Le SFX Music...
            psamSFXMusic = &(samData.samMusic.pSFXMusic[samData.samMusic.dwEntryPlayed]);
            
            
            if (psamSFXMusic->bUsed) //Le SFXMusic existe
            {
                //On recherche le SFX associé
                if (!SAM_SFX_GetIt(psamSFXMusic->dwSFXHandle,&psamSFX))
                {
                    lContinue = 0;
                    if (psamSFX->bIsLoaded) lContinue++;
                    if (psamSFX->dwGranuleCount) lContinue++;
                    
                    if (lContinue==2)
                    {
                        //Les nouveaux paramètres d'échantillonnage
                        SAM_MUSIC_ApplyNewSampleRate ( (DWORD)psamSFX->fSampleRate );
                        
                        //Le SFXMusic est en mode lecture
                        psamSFXMusic->bPlayed = 1;
                        
                        //Mise à jour de l'utilisation du SFX
                        psamSFX->dwLastAccessTimeStamp = dwInternalTimer;
                        
                        fGlobalGain = psamSFX->fReplayGain * psamSFX->fUserDefaultGain * samData.fGainMusic;
                        
                        //Lecture du stream
                        dwSamplesCount = SAM_MUSIC_MixSingleGranule (
                                psamSFXMusic,
                                psamSFX,
                                fGlobalGain,
                                dwSoftwareBufferSamplesToMixCount,
                                pfOutputBuffer, //pfSoftwareBuffer,
                                dwSoftwareBufferChannelCount );
                            
                        pfOutputBuffer += dwSamplesCount * dwSoftwareBufferChannelCount;
                        dwSoftwareBufferSamplesToMixCount -= dwSamplesCount;
                        
                        //Le SFXMusic n'est plus en mode lecture ???
                        if (!psamSFXMusic->bPlayed)
                        {
                            //Oui ! Alors il faut jumper vers un autre SFXMusic
                            samData.samMusic.dwEntryPlayed = psamSFXMusic->dwNextSFXMusic;
                        }
                    }
                    else lFin = 1;
                }
                else //Le SFX n'existe pas
                {
                    //On arrête la lecture   
                    samData.samMusic.dwEntryPlayed=0xFFFFFFFF;
                    
                    //Fin
                    lFin = 1;
                }
            }
            else    //Le SFXMusic n'existe pas
            {
                //On arrête la lecture   
                samData.samMusic.dwEntryPlayed=0xFFFFFFFF;
                
                //Fin
                lFin = 1;
            }
            
            if (dwSoftwareBufferSamplesToMixCount<=0)
                lFin = 1;
                
        } while (!lFin);

        _OSI_LeaveCriticalSection ( &samData.samMusic.osiCriticalSection );
    }




DWORD SAM_MUSIC_MixSingleGranule ( SAM_SFX_MUSIC * psamSFXMusic, SAM_SFX * psamSFX, float fGlobalGain, DWORD dwLoopCount, float * pfSoftwareBuffer, DWORD dwSoftwareBufferChannelCount )
{
    float       *interpolation_fStackValue;
    float       *pfInterpolator;

    float       fProcessValue[2];
    DWORD       dwInPlaySamplePosition;
    DWORD       dwInPlaySamplePositionPrevious;
    DWORD       dwInPlayTickPosition;
    DWORD       dwInPlayTickIncrement;
    DWORD       dwInPlayGranulePosition;

    DWORD       dwSamplesCount;


    BYTE        bFormat;
    BYTE        bLoop;
    DWORD       *pAudioDataD32;

    DWORD       dwTemp;
    DWORD       dwCurrentGranuleID;
    DWORD       dwBytesPerSampleCount;
    DWORD       dwSamplesPerGranuleCount;

    float       *pfBufferOut;

    long        i, j;

    void        *pCodecData;

    DWORD       dwOutputCount;
    DWORD       dwMixedCount;
    
    //Lecture des données en R/O
    bFormat                                     = psamSFX->bFormat;
    bLoop                                       = (unsigned char)psamSFXMusic->bLoop;
    dwInPlayTickIncrement                       = samData.samMusic.dwInPlayTickIncrement;
    dwSamplesCount                              = psamSFX->dwSamplesCount;
    pfBufferOut                                 = pfSoftwareBuffer;
    pCodecData                                  = samData.samMusic.dwCodecData;

    //Nombre de samples par granule
    SAM_FormatGetBytesCount ( bFormat, &dwBytesPerSampleCount );
    dwSamplesPerGranuleCount                    = sam_GRANULE_BUFFERBYTES / dwBytesPerSampleCount;

    //Lecture des données en R/W
    dwInPlaySamplePosition                      = samData.samMusic.dwInPlaySamplePosition;
    dwInPlaySamplePositionPrevious              = samData.samMusic.dwInPlaySamplePositionPrevious;
    dwInPlayTickPosition                        = samData.samMusic.dwInPlayTickPosition;
    dwInPlayGranulePosition                     = samData.samMusic.dwInPlayGranulePosition;
    dwCurrentGranuleID                          = psamSFXMusic->dwCurrentGranuleID;
    
    interpolation_fStackValue                   = samData.samMusic.interpolation_fStackValue;
    

    //Les données du granule en cours...
    SAM_GranulesGetData ( dwCurrentGranuleID, &pAudioDataD32 );
    //pAudioDataF32 = (float *)pAudioDataB8;
    //pAudioDataXD4 = (DWORD *)pAudioDataB8;
    //pAudioDataQ64 = (QWORD *)pAudioDataB8;
    //pAudioDataD32 = (DWORD *)pAudioDataB8;
    dwOutputCount = dwLoopCount;
    dwMixedCount = 0;

    //Traitement avec un son stéréo : Resampling + Volume + Mixage Stéréo
    do {
        dwMixedCount++;
        
        //*******************************************************
        //Devons-nous charger une nouvelle donnée ?
        if (dwInPlaySamplePosition!=dwInPlaySamplePositionPrevious)
        {
            if (bFormat&sam_FORMAT_CHANNELMASK)
            {
                //dwTemp = dwInPlayGranulePosition&3;
                //resample_fCurrValue[0] = SAM_XD4_DecodeValue ( (DWORD)(pAudioDataQ64[dwInPlayGranulePosition>>2]&0xFFFFFFFF), dwTemp );
                //resample_fCurrValue[1] = SAM_XD4_DecodeValue ( (DWORD)(pAudioDataQ64[dwInPlayGranulePosition>>2]>>32), dwTemp );
#if ika_INTERNAL_AUDIO_FORMAT == sam_FORMAT_MONO_PCM16                
                fProcessValue[0] = SAM_LPCM_DecodeValue16 ( pAudioDataD32[dwInPlayGranulePosition], 0 );
                fProcessValue[1] = SAM_LPCM_DecodeValue16 ( pAudioDataD32[dwInPlayGranulePosition], 1 );
#endif

#if ika_INTERNAL_AUDIO_FORMAT == sam_FORMAT_MONO_XD4
                fProcessValue[0] = SAM_XD4_DecodeValue ( pAudioDataD32[((dwInPlayGranulePosition>>1)&0xFFFFFFFE)  ], dwInPlayGranulePosition&3 );
                fProcessValue[1] = SAM_XD4_DecodeValue ( pAudioDataD32[((dwInPlayGranulePosition>>1)&0xFFFFFFFE)+1], dwInPlayGranulePosition&3 );
#endif
            }
            else
            {
                
#if ika_INTERNAL_AUDIO_FORMAT == sam_FORMAT_MONO_PCM16                
                fProcessValue[0] = SAM_LPCM_DecodeValue16 ( pAudioDataD32[dwInPlayGranulePosition>>1], dwInPlayGranulePosition&1 );
#endif

#if ika_INTERNAL_AUDIO_FORMAT == sam_FORMAT_MONO_XD4
                fProcessValue[0] = SAM_XD4_DecodeValue ( pAudioDataD32[dwInPlayGranulePosition>>2], (dwInPlayGranulePosition&3) );
#endif
                
                fProcessValue[1] = fProcessValue[0];
            }
            
            dwInPlaySamplePositionPrevious = dwInPlaySamplePosition;
            
            for (i=0;i<15;i++)
            {
                interpolation_fStackValue[i   ] = interpolation_fStackValue[i+1 ];
                interpolation_fStackValue[i+16] = interpolation_fStackValue[i+17];
            }
            interpolation_fStackValue[15] = fProcessValue[0];
            interpolation_fStackValue[31] = fProcessValue[1];
            /*
            __asm {
            
                mov esi, interpolation_fStackValue
                
                movups  xmm0, [esi+ 4]
                movss   xmm4, fProcessValue
                movups  xmm1, [esi+20]
                movups  xmm2, [esi+36]
                movups  xmm3, [esi+52] //On déborde un peu, mais c'est pas grave car la pile fait plus de 16 points (pour la gestion en stéréo)
                
                movaps  [esi+ 0], xmm0
                movaps  [esi+16], xmm1
                movaps  [esi+32], xmm2
                movaps  [esi+48], xmm3
                movss   [esi+60], xmm4
                
            }
            */    
                
            


        }

        //*******************************************************
        // Génération de la phase sur 10 bits
        dwTemp = (dwInPlayTickPosition>>14)&0x3FF;            

        //*******************************************************
        // Application de l'interpolation
        fProcessValue[0] = 0;
        fProcessValue[1] = 0;
        pfInterpolator = &samData_f32InterpolationData_1024_16[dwTemp<<4];
        for (i=0;i<16;i++,dwTemp++)
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
            //Devons-nous boucler ?
            if (bLoop)
            {
                //Fin du SFX, on retourne au début
                dwInPlaySamplePosition -= dwSamplesCount;
                dwInPlayGranulePosition = dwInPlaySamplePosition;

                //Retourne au premier granule
                dwCurrentGranuleID      = psamSFX->dwGranuleFirst;

                //Mise à jour des pointeurs de données sources
                SAM_GranulesGetData ( dwCurrentGranuleID, &pAudioDataD32 );
            }
            else
            {
                //Fin de la lecture
                //dwOutputCount             = 1;
                dwInPlaySamplePosition  = 0;
                dwInPlayGranulePosition = 0; //dwInPlaySamplePosition;
                psamSFXMusic->bPlayed   = 0;
                //psamVoice->bIsUsed      = 0;

                //Retourne au premier granule
                dwCurrentGranuleID = psamSFX->dwGranuleFirst;

                dwOutputCount           = 1;


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

            //Mise à jour des pointeurs de données sources
            SAM_GranulesGetData ( dwCurrentGranuleID, &pAudioDataD32 );
        }

        pfBufferOut += dwSoftwareBufferChannelCount;
    } while (--dwOutputCount);


    //Ecriture des données en R/W
    samData.samMusic.dwInPlaySamplePosition             = dwInPlaySamplePosition;
    samData.samMusic.dwInPlaySamplePositionPrevious     = dwInPlaySamplePositionPrevious;
    samData.samMusic.dwInPlayTickPosition               = dwInPlayTickPosition;
    samData.samMusic.dwInPlayGranulePosition            = dwInPlayGranulePosition;
    psamSFXMusic->dwCurrentGranuleID                    = dwCurrentGranuleID;

    
    return dwMixedCount;
}



DLLEXPORT   long    SAM_MUSIC_AddSFX ( DWORD dwHandleSFX, DWORD * pdwAllocatedEntry )
{
    DWORD           dwIndex, dwFoundIndex;
    SAM_SFX_MUSIC   *psamSFXMusic;
    SAM_SFX         *psamSFX;

    //Le SFX existe ?    
    if (SAM_SFX_GetIt(dwHandleSFX,&psamSFX))
        return -1;

    //Il faut pouvoir renvoyer l'entrée        
    if (!pdwAllocatedEntry)
        return -1;
        
    _OSI_EnterCriticalSection ( &samData.samMusic.osiCriticalSection );

    //Recherche un emplacement de libre
    dwFoundIndex = 0xFFFFFFFF;    
    psamSFXMusic = samData.samMusic.pSFXMusic;
    for (dwIndex=0;dwIndex<samData.samMusic.dwSFXMusicCount;dwIndex++,psamSFXMusic++)
    {
        if (!psamSFXMusic->bUsed)
        {
            dwFoundIndex = dwIndex;
            break;
        }
    }
    
    if (dwFoundIndex==0xFFFFFFFF)
    {
        _OSI_LeaveCriticalSection ( &samData.samMusic.osiCriticalSection );
        return -1;
    }
        
    psamSFXMusic = &(samData.samMusic.pSFXMusic[dwFoundIndex]);
    memset ( psamSFXMusic, 0, sizeof(SAM_SFX_MUSIC) );
    psamSFXMusic->dwSFXHandle           = dwHandleSFX;
    psamSFXMusic->dwNextSFXMusic        = 0xFFFFFFFF;
    psamSFXMusic->dwCurrentGranuleID    = psamSFX->dwGranuleFirst;
    psamSFXMusic->bUsed                 = 1;
    
    *pdwAllocatedEntry = dwFoundIndex;
    
    _OSI_LeaveCriticalSection ( &samData.samMusic.osiCriticalSection );
    
    return 0;
}

DLLEXPORT   long    SAM_MUSIC_Delete ( DWORD dwEntry )
{
    DWORD           dwIndex, dwJumpEntry;
    SAM_SFX_MUSIC   *psamSFXMusic;
    
    //On se protège    
    _OSI_EnterCriticalSection ( &samData.samMusic.osiCriticalSection );

    //Recherche l'entrée
    if ((dwEntry>=samData.samMusic.dwSFXMusicCount)&&(dwEntry!=0xFFFFFFFF))
    {
        _OSI_LeaveCriticalSection ( &samData.samMusic.osiCriticalSection );
        return -1;
    }

    if (dwEntry==0xFFFFFFFF)
    {
        memset ( samData.samMusic.pSFXMusic, 0, sizeof(SAM_SFX_MUSIC) * samData.samMusic.dwSFXMusicCount );
        samData.samMusic.dwEntryPlayed = 0xFFFFFFFF;
    }
    else
    {
        psamSFXMusic = &(samData.samMusic.pSFXMusic[dwEntry]);
        
        //Vide l'entrée
        dwJumpEntry = psamSFXMusic->dwNextSFXMusic;
        memset ( psamSFXMusic, 0, sizeof(SAM_SFX_MUSIC) );
        if (dwJumpEntry==dwEntry)
            dwJumpEntry = 0xFFFFFFFF;
        
        //Recherche un jump pour remplacement
        psamSFXMusic = samData.samMusic.pSFXMusic;
        for (dwIndex=0;dwIndex<samData.samMusic.dwSFXMusicCount;dwIndex++,psamSFXMusic++)
        {
            if ((psamSFXMusic->bUsed)&&(psamSFXMusic->dwNextSFXMusic==dwEntry))
                psamSFXMusic->dwNextSFXMusic = dwJumpEntry;
        }
        
        if (samData.samMusic.dwEntryPlayed==dwEntry)
            samData.samMusic.dwEntryPlayed = 0xFFFFFFFF;    
    }
    
    //Fin de la section critique
    _OSI_LeaveCriticalSection ( &samData.samMusic.osiCriticalSection );    
    
    return 0;
}

DLLEXPORT   long    SAM_MUSIC_SetLoop ( DWORD dwEntry, BYTE bLoop )
{
    DWORD           dwIndex, dwJumpEntry;
    SAM_SFX_MUSIC   *psamSFXMusic;
    
    //On se protège    
    _OSI_EnterCriticalSection ( &samData.samMusic.osiCriticalSection );

    //Recherche l'entrée
    if (dwEntry>=samData.samMusic.dwSFXMusicCount)
    {
        _OSI_LeaveCriticalSection ( &samData.samMusic.osiCriticalSection );
        return -1;
    }
        
    psamSFXMusic = &(samData.samMusic.pSFXMusic[dwEntry]);

    if (psamSFXMusic->bUsed)
        psamSFXMusic->bLoop = bLoop;        

    //Fin de la section critique
    _OSI_LeaveCriticalSection ( &samData.samMusic.osiCriticalSection );    

    return 0;
}

DLLEXPORT   long    SAM_MUSIC_SetJump ( DWORD dwEntry, DWORD dwJumpEntry )
{
    DWORD           dwIndex;
    SAM_SFX_MUSIC   *psamSFXMusic;
    
    //On se protège    
    _OSI_EnterCriticalSection ( &samData.samMusic.osiCriticalSection );

    //Recherche l'entrée
    if (dwEntry>=samData.samMusic.dwSFXMusicCount)
    {
        _OSI_LeaveCriticalSection ( &samData.samMusic.osiCriticalSection );
        return -1;
    }

    //Recherche l'entrée
    if (dwJumpEntry>=samData.samMusic.dwSFXMusicCount)
    {
        _OSI_LeaveCriticalSection ( &samData.samMusic.osiCriticalSection );
        return -1;
    }

    psamSFXMusic = &(samData.samMusic.pSFXMusic[dwEntry]);

    if (psamSFXMusic->bUsed)
        psamSFXMusic->dwNextSFXMusic = dwJumpEntry;        

    //Fin de la section critique
    _OSI_LeaveCriticalSection ( &samData.samMusic.osiCriticalSection );    

    return 0;
}

DLLEXPORT   long    SAM_MUSIC_Play ( DWORD dwEntry )
{
    DWORD           dwIndex, dwJumpEntry;
    SAM_SFX_MUSIC   *psamSFXMusic;


    //On se protège    
    _OSI_EnterCriticalSection ( &samData.samMusic.osiCriticalSection );
    
    //Recherche l'entrée
    if (dwEntry<samData.samMusic.dwSFXMusicCount)
    {   
        psamSFXMusic = &(samData.samMusic.pSFXMusic[dwEntry]);
        
        if (psamSFXMusic->bUsed)    
            samData.samMusic.dwEntryPlayed = dwEntry;
    }
    else samData.samMusic.dwEntryPlayed = 0xFFFFFFFF;
    

    //Fin de la section critique
    _OSI_LeaveCriticalSection ( &samData.samMusic.osiCriticalSection );    

    return 0;
}


