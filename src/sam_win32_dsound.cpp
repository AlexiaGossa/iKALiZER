    #include "sam_win32_dsound.h"

    SAM_DIRECTSOUNDDATA    samDirectSoundData = { 0 };


    //*******************************************************************************************************************
    //DirectSound GUID

    #undef DEFINE_GUID

    #define DEFINE_GUID(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
            EXTERN_C const GUID name \
                    = { l, w1, w2, { b1, b2,  b3,  b4,  b5,  b6,  b7,  b8 } }

    // DirectSound Component GUID {47D4D946-62E8-11CF-93BC-444553540000}
    DEFINE_GUID(CLSID_DirectSound, 0x47d4d946, 0x62e8, 0x11cf, 0x93, 0xbc, 0x44, 0x45, 0x53, 0x54, 0x0, 0x0);

    // DirectSound 8.0 Component GUID {3901CC3F-84B5-4FA4-BA35-AA8172B8A09B}
    DEFINE_GUID(CLSID_DirectSound8, 0x3901cc3f, 0x84b5, 0x4fa4, 0xba, 0x35, 0xaa, 0x81, 0x72, 0xb8, 0xa0, 0x9b);

    DEFINE_GUID(IID_IDirectSound8, 0xC50A7E93, 0xF395, 0x4834, 0x9E, 0xF6, 0x7F, 0xA9, 0x9D, 0xE5, 0x09, 0x66);
    DEFINE_GUID(IID_IDirectSound, 0x279AFA83, 0x4981, 0x11CE, 0xA5, 0x21, 0x00, 0x20, 0xAF, 0x0B, 0xE5, 0x60);

    //#pragma comment ( lib, "dsound.lib" )


    //*******************************************************************************************************************
    long    SAM_DirectSound_OpenHelper ( void )
    {
        //Déjà initialisé ?
        if (samDirectSoundData.lpDirectSound) return -1;

        

        //Chargement de la librairie
        samDirectSoundData.hLibrary_DirectSound = LoadLibrary ( "DSOUND.DLL" );
        if (samDirectSoundData.hLibrary_DirectSound)
        {
            samDirectSoundData.pDirectSoundCreate = (_mDES_DirectSoundCreate_DLLProc) GetProcAddress (
                   samDirectSoundData.hLibrary_DirectSound, "DirectSoundCreate8" );

            if (samDirectSoundData.pDirectSoundCreate)
            {
                if (FAILED(samDirectSoundData.pDirectSoundCreate ( (GUID *)NULL, &(samDirectSoundData.lpDirectSound), NULL )))
                {
                    //( GUID FAR *lpGUID, LPDIRECTSOUND FAR *lplpDS, IUnknown FAR *pUnkOuter);
                    FreeLibrary ( samDirectSoundData.hLibrary_DirectSound ); 
                    samDirectSoundData.lpDirectSound = NULL;
                    return -1;
                }
            }
            else
            {
                FreeLibrary ( samDirectSoundData.hLibrary_DirectSound ); 
                return -1;
            }

        }
        else return -1;

        

        //_DirectSoundData.pDirectSoundCreate = DirectSoundCreate;
        //DirectSoundCreate ( (GUID *)NULL, &(_DirectSoundData.lpDirectSound), NULL );

        return 0;
    }

    long    SAM_DirectSound_CloseHelper ( void )
    {
        //Pas encore initialisé ?
        if (!samDirectSoundData.lpDirectSound) return -1;

        FreeLibrary ( samDirectSoundData.hLibrary_DirectSound ); 
        samDirectSoundData.lpDirectSound = NULL;
        
        return 0;
    }


    //Libération rapide et simple pour éviter de surcharger le code.
    long    SAM_DirectSound_SecureExit           ( long lPartialMode )
    {
        //Libération du tampon secondaire
        if (samDirectSoundData.lpDirectSoundBuffer)
        {
            samDirectSoundData.lpDirectSoundBuffer->Stop ( );
            samDirectSoundData.lpDirectSoundBuffer->Release ( );
            samDirectSoundData.lpDirectSoundBuffer = NULL;
        }

        //Libération partielle du Helper de DirectSound
        if ((samDirectSoundData.lpDirectSound)&&(!lPartialMode))
        {
            //Modification du mode coopératif
            samDirectSoundData.lpDirectSound->SetCooperativeLevel( samDirectSoundData.hWnd, DSSCL_NORMAL );

            //Libération du Helper            
            samDirectSoundData.lpDirectSound->Release ( );
            samDirectSoundData.lpDirectSound = NULL;
        }

        if (samDirectSoundData.hLibrary_DirectSound)
        {
            FreeLibrary ( samDirectSoundData.hLibrary_DirectSound );
            samDirectSoundData.hLibrary_DirectSound = NULL;
        }

        //Vide les données
        memset ( &samDirectSoundData, 0, sizeof(samDirectSoundData) );

        return -1;
    }



    long    SAM_DirectSound_Open ( void * pDeviceParam, DWORD dwSampleRate, DWORD dwChannels, DWORD dwQuerySampleSize, DWORD * pdwSampleSizeAllocated )
    {
        HRESULT         hResult;
        WAVEFORMATEX    WaveFormatEx;
        DSBUFFERDESC    DSBufferDesc;
        DSBCAPS         DSBufferCaps;
        DWORD           dwBufferSize;
        void            *pBuffer;
        HWND            hWnd;
        
        if (!pDeviceParam)
            hWnd = GetFocus ( );
        else
            hWnd = *((HWND *)pDeviceParam);
        

        //Test init
        if (samDirectSoundData.lAlwaysInit) return -1;

        //Initialisation
        memset ( &samDirectSoundData, 0, sizeof(samDirectSoundData) );
        samDirectSoundData.lAlwaysInit    = TRUE;
        samDirectSoundData.hWnd           = hWnd;

        //Ouverture du helper
        if (SAM_DirectSound_OpenHelper()) return SAM_DirectSound_SecureExit(0);

        //Niveau de coopération
        hResult = samDirectSoundData.lpDirectSound->SetCooperativeLevel ( samDirectSoundData.hWnd, DSSCL_PRIORITY );
        if (hResult != DS_OK) return SAM_DirectSound_SecureExit(0);

        //Paramètres du format        
        memset ( &WaveFormatEx, 0, sizeof(WaveFormatEx) );
        WaveFormatEx.wFormatTag         = WAVE_FORMAT_PCM;
        WaveFormatEx.nChannels          = (WORD)dwChannels;
        WaveFormatEx.wBitsPerSample     = 16;
        WaveFormatEx.nSamplesPerSec     = dwSampleRate;
        WaveFormatEx.nBlockAlign        = (WaveFormatEx.wBitsPerSample>>3) * WaveFormatEx.nChannels;
        WaveFormatEx.nAvgBytesPerSec    = WaveFormatEx.nBlockAlign * WaveFormatEx.nSamplesPerSec;

        //Taille du tampon
        dwBufferSize                    = WaveFormatEx.nBlockAlign * dwSampleRate;

        //Paramètres du tampon DirectSound
        memset ( &DSBufferDesc, 0, sizeof(DSBufferDesc) );
        DSBufferDesc.dwSize             = sizeof(DSBufferDesc);
        DSBufferDesc.dwFlags            = DSBCAPS_LOCHARDWARE | DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_GLOBALFOCUS;
        DSBufferDesc.lpwfxFormat        = &WaveFormatEx;
        DSBufferDesc.dwBufferBytes      = dwBufferSize; 

        //Création du buffer secondaire avec les paramètres hardware
        hResult = samDirectSoundData.lpDirectSound->CreateSoundBuffer ( &DSBufferDesc, &(samDirectSoundData.lpDirectSoundBuffer), NULL );
        if (hResult != DS_OK)
        {
            //Création du buffer secondaire avec les paramètres software
            DSBufferDesc.dwFlags            = DSBCAPS_LOCSOFTWARE | DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_GLOBALFOCUS;
            hResult = samDirectSoundData.lpDirectSound->CreateSoundBuffer ( &DSBufferDesc, &(samDirectSoundData.lpDirectSoundBuffer), NULL );
            if (hResult != DS_OK)
            {
                return SAM_DirectSound_SecureExit(0);
            }
        }

        //Activation de la lecture en boucle
        hResult = samDirectSoundData.lpDirectSoundBuffer->Play ( 0, 0, DSBPLAY_LOOPING );
        if (hResult != DS_OK)
        {
            return SAM_DirectSound_SecureExit(0);
        }

        //Récupération des infos sur le buffer
        memset ( &DSBufferCaps, 0, sizeof(DSBufferCaps) );
        DSBufferCaps.dwSize = sizeof(DSBufferCaps);
        hResult = samDirectSoundData.lpDirectSoundBuffer->GetCaps ( &DSBufferCaps );
        if (hResult != DS_OK)
        {
            return SAM_DirectSound_SecureExit(0);
        }

        //Informe sur les données réellement allouées
        if (pdwSampleSizeAllocated)
        {
            *pdwSampleSizeAllocated = DSBufferCaps.dwBufferBytes / WaveFormatEx.nBlockAlign;
        }

        //Copie du format de sortie
        memcpy ( &(samDirectSoundData.WaveFormatExOutput), &WaveFormatEx, sizeof(WAVEFORMATEX) );

        //Taille du tampon de sortie
        samDirectSoundData.dwSampleSizeOutput = DSBufferCaps.dwBufferBytes / WaveFormatEx.nBlockAlign;
        samDirectSoundData.dwByteSizeOutput   = DSBufferCaps.dwBufferBytes;

        //Adresse du tampon de sortie
        samDirectSoundData.pAudioBufferOutput = NULL;

        //Vide le tampon de sortie
        if (!SAM_DirectSound_BeginPaint ( &pBuffer ))
        {
            memset ( pBuffer, 0, samDirectSoundData.dwByteSizeOutput );
            SAM_DirectSound_EndPaint ( );
        }

        
        return 0;
    }

    long    SAM_DirectSound_Close                  ( void )
    {
        //Test init
        if (samDirectSoundData.lAlwaysInit) return -1;

        SAM_DirectSound_SecureExit(0);

        return 0;
    }

    long SAM_DirectSound_BeginPaint ( void ** pAudioBufferOutput )
    {
        DWORD           dwStatus;
        HRESULT         hResult;
        long            lRepeat;
        void            *pBuffer1;
        //void            *pBuffer2;
        DWORD           dwLockSize1;
        //DWORD           dwLockSize2;

        //Initialisé ?
        if (!samDirectSoundData.lAlwaysInit) return -1;

        //Récupération du status
        hResult = samDirectSoundData.lpDirectSoundBuffer->GetStatus ( &dwStatus );
        if (hResult != DS_OK)
        {
            //Erreur mais on ne quitte pas.
            dwStatus = 0;
        }

        //Buffer perdu, il faut le restaurer... (c'est rare)
        if (dwStatus & DSBSTATUS_BUFFERLOST)
            samDirectSoundData.lpDirectSoundBuffer->Restore ( );

        //Buffer arrété, il faut le relancer... (c'est rare)
        if (!(dwStatus & DSBSTATUS_PLAYING))
            samDirectSoundData.lpDirectSoundBuffer->Play ( 0, 0, DSBPLAY_LOOPING );

        //Le buffer n'est pas encore locké
        samDirectSoundData.pAudioBufferOutput = NULL;
        if (pAudioBufferOutput) *pAudioBufferOutput = samDirectSoundData.pAudioBufferOutput;

        lRepeat         = 0;
        pBuffer1        = NULL;
        dwLockSize1     = samDirectSoundData.dwByteSizeOutput;

        while ( ( hResult = samDirectSoundData.lpDirectSoundBuffer->Lock ( 
            0, 0,
            &pBuffer1, &dwLockSize1,
            NULL, NULL, //&pBuffer2, &dwLockSize2,
            DSBLOCK_ENTIREBUFFER ) ) != DS_OK )
        {
            if (hResult != DSERR_BUFFERLOST)
            {
                //Le lock a planté
                return SAM_DirectSound_SecureExit(0);
            }
            else
            {
                samDirectSoundData.lpDirectSoundBuffer->Restore ( );
            }

            if (++lRepeat>2) return -1;
        }

        //Le tampon est locké
        samDirectSoundData.dwLockSizeOutput   = dwLockSize1;
        samDirectSoundData.pAudioBufferOutput = pBuffer1;
        if (pAudioBufferOutput) *pAudioBufferOutput = samDirectSoundData.pAudioBufferOutput;

        return 0;
    }

    long SAM_DirectSound_EndPaint ( void )
    {
        //Initialisé ?
        if (!samDirectSoundData.lAlwaysInit) return -1;

        //Unlock
        samDirectSoundData.lpDirectSoundBuffer->Unlock (
            samDirectSoundData.pAudioBufferOutput, samDirectSoundData.dwLockSizeOutput,
            0, 0 );

        return 0;
    }

    long SAM_DirectSound_Refresh ( void )
    {
        HRESULT hResult;

        //Initialisé ?
        if (!samDirectSoundData.lAlwaysInit) return -1;

        hResult = samDirectSoundData.lpDirectSound->SetCooperativeLevel (
            samDirectSoundData.hWnd, DSSCL_PRIORITY );

        if (hResult != DS_OK)
        {
            return SAM_DirectSound_SecureExit(0);
        }

        return 0;
    }

    long    SAM_DirectSound_GetPosition            ( DWORD * pdwSampleWrite, DWORD * pdwSampleRead )
    {
        DWORD   dwRead;
        DWORD   dwWrite;

        //Initialisé ?
        if (!samDirectSoundData.lAlwaysInit) return -1;

        dwWrite = 0;
        dwRead  = 0;
        samDirectSoundData.lpDirectSoundBuffer->GetCurrentPosition ( &dwRead, &dwWrite );

        if (pdwSampleWrite)
            *pdwSampleWrite = dwWrite / samDirectSoundData.WaveFormatExOutput.nBlockAlign;
        
        if (pdwSampleRead)
            *pdwSampleRead  = dwRead / samDirectSoundData.WaveFormatExOutput.nBlockAlign;
            
        return 0;
    }

