    #undef __cplusplus
    #include "sam_win32_dsound.h"
    //#include "ks.h"
    //#include "ksmedia.h"

    static SAM_DIRECTSOUNDDATA    samDirectSoundData = { 0 };


    //*******************************************************************************************************************
    //DirectSound GUID
    /*
    #undef DEFINE_GUID

    #define DEFINE_GUID(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
            EXTERN_C const GUID name \
                    = { l, w1, w2, { b1, b2,  b3,  b4,  b5,  b6,  b7,  b8 } }

    // DirectSound Component GUID {47D4D946-62E8-11CF-93BC-444553540000}
    //DEFINE_GUID(CLSID_DirectSound, 0x47d4d946, 0x62e8, 0x11cf, 0x93, 0xbc, 0x44, 0x45, 0x53, 0x54, 0x0, 0x0);

    // DirectSound 8.0 Component GUID {3901CC3F-84B5-4FA4-BA35-AA8172B8A09B}
    //DEFINE_GUID(CLSID_DirectSound8, 0x3901cc3f, 0x84b5, 0x4fa4, 0xba, 0x35, 0xaa, 0x81, 0x72, 0xb8, 0xa0, 0x9b);

    //DEFINE_GUID(IID_IDirectSound8, 0xC50A7E93, 0xF395, 0x4834, 0x9E, 0xF6, 0x7F, 0xA9, 0x9D, 0xE5, 0x09, 0x66);
    //DEFINE_GUID(IID_IDirectSound, 0x279AFA83, 0x4981, 0x11CE, 0xA5, 0x21, 0x00, 0x20, 0xAF, 0x0B, 0xE5, 0x60);
    
    //DSDEVID_DefaultPlayback
    DEFINE_GUID(DSDEVID_DEFAULTPLAYBACK_DEVICE, 0xdef00000, 0x9c6d, 0x47ed, 0xaa, 0xf1, 0x4d, 0xda, 0x8f, 0x2b, 0x5c, 0x03);
    */
    //#pragma comment ( lib, "dsound.lib" )

    // Speaker Positions:
    #define SPEAKER_FRONT_LEFT              0x1
    #define SPEAKER_FRONT_RIGHT             0x2
    #define SPEAKER_FRONT_CENTER            0x4
    #define SPEAKER_LOW_FREQUENCY           0x8
    #define SPEAKER_BACK_LEFT               0x10
    #define SPEAKER_BACK_RIGHT              0x20
    #define SPEAKER_FRONT_LEFT_OF_CENTER    0x40
    #define SPEAKER_FRONT_RIGHT_OF_CENTER   0x80
    #define SPEAKER_BACK_CENTER             0x100
    #define SPEAKER_SIDE_LEFT               0x200
    #define SPEAKER_SIDE_RIGHT              0x400
    #define SPEAKER_TOP_CENTER              0x800
    #define SPEAKER_TOP_FRONT_LEFT          0x1000
    #define SPEAKER_TOP_FRONT_CENTER        0x2000
    #define SPEAKER_TOP_FRONT_RIGHT         0x4000
    #define SPEAKER_TOP_BACK_LEFT           0x8000
    #define SPEAKER_TOP_BACK_CENTER         0x10000
    #define SPEAKER_TOP_BACK_RIGHT          0x20000

    //
    //  New wave format development should be based on the
    //  WAVEFORMATEXTENSIBLE structure. WAVEFORMATEXTENSIBLE allows you to
    //  avoid having to register a new format tag with Microsoft. Simply
    //  define a new GUID value for the WAVEFORMATEXTENSIBLE.SubFormat field
    //  and use WAVE_FORMAT_EXTENSIBLE in the
    //  WAVEFORMATEXTENSIBLE.Format.wFormatTag field.
    //
    #ifndef _WAVEFORMATEXTENSIBLE_
    #define _WAVEFORMATEXTENSIBLE_
    typedef struct {
        WAVEFORMATEX    Format;
        union {
            WORD wValidBitsPerSample;       /* bits of precision  */
            WORD wSamplesPerBlock;          /* valid if wBitsPerSample==0 */
            WORD wReserved;                 /* If neither applies, set to zero. */
        } Samples;
        DWORD           dwChannelMask;      /* which channels are */
                                            /* present in stream  */
        GUID            SubFormat;
    } WAVEFORMATEXTENSIBLE, *PWAVEFORMATEXTENSIBLE;
    #endif // !_WAVEFORMATEXTENSIBLE_

    #if !defined(WAVE_FORMAT_EXTENSIBLE)
    #define  WAVE_FORMAT_EXTENSIBLE                 0xFFFE
    #endif // !defined(WAVE_FORMAT_EXTENSIBLE)
    
    //quick hack to avoid dx8 sdk headers
    //#if defined(__cplusplus) && _MSC_VER >= 1100
        #define DEFINE_GUIDSTRUCT(g, n) struct __declspec(uuid(g)) n
        #define DEFINE_GUIDNAMED(n) __uuidof(struct n)
    //#else // !defined(__cplusplus)
    //    #define DEFINE_GUIDSTRUCT(g, n) DEFINE_GUIDEX(n)
    //    #define DEFINE_GUIDNAMED(n) n
    //#endif // !defined(__cplusplus)


    #if !defined( DEFINE_WAVEFORMATEX_GUID )
        #define DEFINE_WAVEFORMATEX_GUID(x) (USHORT)(x), 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71
    #endif
    
    /*
    #define STATIC_KSDATAFORMAT_SUBTYPE_PCM\
        DEFINE_WAVEFORMATEX_GUID(WAVE_FORMAT_PCM)
    DEFINE_GUIDSTRUCT("00000001-0000-0010-8000-00aa00389b71", KSDATAFORMAT_SUBTYPE_PCM);
    #define KSDATAFORMAT_SUBTYPE_PCM DEFINE_GUIDNAMED(KSDATAFORMAT_SUBTYPE_PCM)
    */
    
    #define STATIC_KSDATAFORMAT_SUBTYPE_PCM             DEFINE_WAVEFORMATEX_GUID(WAVE_FORMAT_PCM)
    const GUID KSDATAFORMAT_SUBTYPE_PCM = { 0x00000001, 0x0000, 0x0010, { 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71 } };

    #define STATIC_KSDATAFORMAT_SUBTYPE_IEEE_FLOAT      DEFINE_WAVEFORMATEX_GUID(WAVE_FORMAT_IEEE_FLOAT)
    const GUID KSDATAFORMAT_SUBTYPE_IEEE_FLOAT = { 0x00000003, 0x0000, 0x0010, { 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71 } };
    
    /*
    #if defined(_INC_MMREG)
    #define STATIC_KSDATAFORMAT_SUBTYPE_IEEE_FLOAT\
        DEFINE_WAVEFORMATEX_GUID(WAVE_FORMAT_IEEE_FLOAT)
    DEFINE_GUIDSTRUCT("00000003-0000-0010-8000-00aa00389b71", KSDATAFORMAT_SUBTYPE_IEEE_FLOAT);
    #define KSDATAFORMAT_SUBTYPE_IEEE_FLOAT DEFINE_GUIDNAMED(KSDATAFORMAT_SUBTYPE_IEEE_FLOAT)
    #endif // defined(_INC_MMREG)    
    */
    

    //*******************************************************************************************************************
    long    SAM_DirectSound_OpenHelper ( DWORD dwDeviceEntry )
    {
        LPGUID  lpGUID;
        DWORD   dwSelectedDevice;
        
        lpGUID              = NULL;
        dwSelectedDevice    = 0;
        
        //Déjà initialisé ?
        if (samDirectSoundData.lpDirectSound) return -1;

        //Chargement de la librairie
        samDirectSoundData.hLibrary_DirectSound = LoadLibrary ( "DSOUND.DLL" );
        if (samDirectSoundData.hLibrary_DirectSound)
        {
            samDirectSoundData.pDirectSoundCreate = (__DirectSoundCreate_DLLProc) GetProcAddress (
                   samDirectSoundData.hLibrary_DirectSound, "DirectSoundCreate8" );
                   
            samDirectSoundData.pDirectSoundEnumerate = (__DirectSoundEnumerate_DLLProc) GetProcAddress (
                   samDirectSoundData.hLibrary_DirectSound, "DirectSoundEnumerateA" );
                   
            //Tentative d'énumération
            if (samDirectSoundData.pDirectSoundEnumerate)
            {
                SAM_DirectSound_GetEnum ( );
                dwSelectedDevice = dwDeviceEntry;
                
                //Sélection du pilote
                if (dwSelectedDevice>=samDirectSoundData.lDSoundEnumCount)
                    dwSelectedDevice = 0;
                    
                lpGUID = samDirectSoundData.samDSoundEnum[dwSelectedDevice].lpGUID;
            }
                

            if (samDirectSoundData.pDirectSoundCreate)
            {
                if (FAILED(samDirectSoundData.pDirectSoundCreate ( lpGUID, &(samDirectSoundData.lpDirectSound), NULL )))
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

        return dwDeviceEntry;
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
        //Libération partielle du Helper de DirectSound
        if ((samDirectSoundData.lpDirectSound)&&(!lPartialMode))
        {
            //Modification du mode coopératif
            //IDirectSound_SetCooperativeLevel ( samDirectSoundData.lpDirectSound, samDirectSoundData.hWnd, DSSCL_PRIORITY );
        }
    
        //Libération du tampon secondaire
        if (samDirectSoundData.lpDirectSoundBuffer)
        {
            IDirectSoundBuffer_Stop ( samDirectSoundData.lpDirectSoundBuffer );
            IDirectSoundBuffer_Release ( samDirectSoundData.lpDirectSoundBuffer );
            samDirectSoundData.lpDirectSoundBuffer = NULL;
        }

        //Libération partielle du Helper de DirectSound
        if ((samDirectSoundData.lpDirectSound)&&(!lPartialMode))
        {
            //Modification du mode coopératif
            //IDirectSound_SetCooperativeLevel ( samDirectSoundData.lpDirectSound, samDirectSoundData.hWnd, DSSCL_NORMAL );

            //Libération du Helper
            IDirectSound_Release ( samDirectSoundData.lpDirectSound );
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



    long    SAM_DirectSound_Open ( DWORD dwDeviceEntry, void * pDeviceParam, DWORD dwSampleRate, DWORD dwChannels, DWORD dwQuerySampleSize, DWORD * pdwSampleSizeAllocated )
    {
        HRESULT                 hResult;
        WAVEFORMATEX            WaveFormatEx;
        WAVEFORMATEXTENSIBLE    WaveFormatExtensible;
        DSBUFFERDESC            DSBufferDesc;
        DSBCAPS                 DSBufferCaps;
        DSCAPS                  DSCaps;
        DWORD                   dwBufferSize;
        void                    *pBuffer;
        HWND                    hWnd;
        DWORD                   dwSelectedDevice;
        long                    lEntryTest;
        DWORD                   dwSpeakersConfig;
        
        if (!pDeviceParam)
        {
            hWnd = GetFocus ( );
            if (!hWnd) hWnd = GetForegroundWindow ( );
        }
        else
            hWnd = *((HWND *)pDeviceParam);
        
        //dwSampleRate /= 16;

        //Test init
        if (samDirectSoundData.lAlwaysInit) return -1;

        //Initialisation
        memset ( &samDirectSoundData, 0, sizeof(samDirectSoundData) );
        samDirectSoundData.lAlwaysInit    = TRUE;
        samDirectSoundData.hWnd           = hWnd;

        //Ouverture du helper
        dwSelectedDevice = (DWORD)SAM_DirectSound_OpenHelper(dwDeviceEntry);
        hResult          = dwSelectedDevice;
        if (hResult<0) return SAM_DirectSound_SecureExit(0);
        
        //Initialisation
        //pDS->lpVtbl->Initialize( pDS, NULL);
        //IDirectSound_Initialize ( samDirectSoundData.lpDirectSound, NULL );
        
        //Les capacités du périphérique
        memset ( &DSCaps, 0, sizeof(DSCaps) );
        DSCaps.dwSize = sizeof(DSCaps);
        hResult = IDirectSound_GetCaps ( samDirectSoundData.lpDirectSound, &DSCaps );
        samDirectSoundData.lCaps_Certified = (DSCaps.dwFlags&DSCAPS_CERTIFIED )?(1):(0);
        samDirectSoundData.lCaps_Emulated  = (DSCaps.dwFlags&DSCAPS_EMULDRIVER)?(1):(0);
        
        //La configuration du périphérique
        dwSpeakersConfig = 0;
        IDirectSound_GetSpeakerConfig ( samDirectSoundData.lpDirectSound, &dwSpeakersConfig );

        //Paramètres du format Etendu       
        memset ( &WaveFormatEx, 0, sizeof(WaveFormatEx) );
        WaveFormatEx.cbSize             = sizeof(WaveFormatEx);
        WaveFormatEx.wFormatTag         = WAVE_FORMAT_PCM;
        WaveFormatEx.nChannels          = (WORD)dwChannels;
        WaveFormatEx.wBitsPerSample     = 16;
        WaveFormatEx.nSamplesPerSec     = dwSampleRate;
        WaveFormatEx.nBlockAlign        = (WaveFormatEx.wBitsPerSample>>3) * WaveFormatEx.nChannels;
        WaveFormatEx.nAvgBytesPerSec    = WaveFormatEx.nBlockAlign * WaveFormatEx.nSamplesPerSec;
        
        //Paramètres du format Extensible
        memset ( &WaveFormatExtensible, 0, sizeof(WaveFormatExtensible) );
        switch (dwChannels)
        {
            case 2:
                WaveFormatExtensible.dwChannelMask = 
                    SPEAKER_FRONT_LEFT | 
                    SPEAKER_FRONT_RIGHT;
                break;
                
            case 4:
                WaveFormatExtensible.dwChannelMask = 
                    SPEAKER_FRONT_LEFT | 
                    SPEAKER_FRONT_RIGHT |
                    SPEAKER_BACK_LEFT |
                    SPEAKER_BACK_RIGHT;
                break;

            case 6:
                WaveFormatExtensible.dwChannelMask = 
                    SPEAKER_FRONT_LEFT | 
                    SPEAKER_FRONT_RIGHT |
                    SPEAKER_FRONT_CENTER |
                    SPEAKER_LOW_FREQUENCY |
                    SPEAKER_BACK_LEFT |
                    SPEAKER_BACK_RIGHT;
                break;
        }
        memcpy ( &WaveFormatExtensible.Format, &WaveFormatEx, sizeof(WaveFormatEx) );
        WaveFormatExtensible.Format.cbSize                  = 22;
        WaveFormatExtensible.Format.wFormatTag              = WAVE_FORMAT_EXTENSIBLE;
        WaveFormatExtensible.Samples.wValidBitsPerSample    = WaveFormatEx.wBitsPerSample;
        WaveFormatExtensible.SubFormat                      = KSDATAFORMAT_SUBTYPE_PCM;

        //Taille du tampon
        dwBufferSize                    = WaveFormatEx.nBlockAlign * dwQuerySampleSize; //dwSampleRate;

        //Paramètres du tampon DirectSound
        memset ( &DSBufferDesc, 0, sizeof(DSBufferDesc) );
        DSBufferDesc.dwSize             = sizeof(DSBufferDesc);
        DSBufferDesc.dwFlags            = DSBCAPS_LOCHARDWARE | DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_GLOBALFOCUS;
        DSBufferDesc.lpwfxFormat        = (WAVEFORMATEX *)&WaveFormatExtensible;
        DSBufferDesc.dwBufferBytes      = dwBufferSize; 
        DSBufferDesc.guid3DAlgorithm    = GUID_NULL;
        
        //Niveau de coopération
        hResult = IDirectSound_SetCooperativeLevel ( samDirectSoundData.lpDirectSound, samDirectSoundData.hWnd, DSSCL_PRIORITY );
        if (hResult != DS_OK)
            return SAM_DirectSound_SecureExit(0);    
        
        
        /*
            Pourquoi le hardware fonctionne alors que le software ne fonctionne pas, et vice-versa ?
            
            Selon la demande DSBufferDesc, il est possible que le soft ou le hard ne fonctionne pas.
            Par exemple, on veut fixer un mode en 6 canaux et le soft ne fonctionne pas... C'est assez
            logique si la carte est en mode 2 canaux. En même temps, c'est bizarre, mais le hard
            fonctionne. C'est la carte qui fait la conversion 6 canaux vers 2 canaux.
        
        */
        
        lEntryTest = 0;
        do {
            switch (lEntryTest)
            {
                case 0:
                    //Création du buffer secondaire avec les paramètres hardware
                    DSBufferDesc.lpwfxFormat = (WAVEFORMATEX *)&WaveFormatExtensible;
                    DSBufferDesc.dwFlags     = DSBCAPS_LOCHARDWARE | DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_GLOBALFOCUS;
                    hResult = IDirectSound_CreateSoundBuffer ( samDirectSoundData.lpDirectSound, &DSBufferDesc, &(samDirectSoundData.lpDirectSoundBuffer), NULL );
                    strcpy ( samDirectSoundData.szOutputModel, "DirectSound secondary hardware buffer (extensible)" );
                    break;                   

                case 1:
                    //Création du buffer secondaire avec les paramètres hardware
                    DSBufferDesc.lpwfxFormat = (WAVEFORMATEX *)&WaveFormatExtensible;
                    DSBufferDesc.dwFlags     = DSBCAPS_LOCSOFTWARE | DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_GLOBALFOCUS;
                    hResult = IDirectSound_CreateSoundBuffer ( samDirectSoundData.lpDirectSound, &DSBufferDesc, &(samDirectSoundData.lpDirectSoundBuffer), NULL );
                    strcpy ( samDirectSoundData.szOutputModel, "DirectSound secondary software buffer (extensible)" );
                    break;                   

                case 2:
                    //Création du buffer secondaire avec les paramètres hardware
                    DSBufferDesc.lpwfxFormat = (WAVEFORMATEX *)&WaveFormatEx;
                    DSBufferDesc.dwFlags     = DSBCAPS_LOCHARDWARE | DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_GLOBALFOCUS;
                    hResult = IDirectSound_CreateSoundBuffer ( samDirectSoundData.lpDirectSound, &DSBufferDesc, &(samDirectSoundData.lpDirectSoundBuffer), NULL );
                    strcpy ( samDirectSoundData.szOutputModel, "DirectSound secondary hardware buffer (extended)" );
                    break;
                    
                case 3:
                    //Création du buffer secondaire avec les paramètres software
                    DSBufferDesc.lpwfxFormat = (WAVEFORMATEX *)&WaveFormatEx;
                    DSBufferDesc.dwFlags     = DSBCAPS_LOCSOFTWARE | DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_GLOBALFOCUS;
                    hResult = IDirectSound_CreateSoundBuffer ( samDirectSoundData.lpDirectSound, &DSBufferDesc, &(samDirectSoundData.lpDirectSoundBuffer), NULL );
                    strcpy ( samDirectSoundData.szOutputModel, "DirectSound secondary software buffer (extended)" );
                    break;
                    
                default:
                    hResult     = DS_OK;
                    lEntryTest  = -1;
                    break;
            }
            
            if (hResult!=DS_OK)
                lEntryTest += 1;
            
        } while ((hResult!=DS_OK)&&(lEntryTest!=-1));
        
        //Nous ne somme pas parvenu à initialiser un périphérique
        if (lEntryTest==-1)
            return SAM_DirectSound_SecureExit(0);
            
        //Spécification du format
        hResult = IDirectSoundBuffer_SetFormat ( samDirectSoundData.lpDirectSoundBuffer, &WaveFormatEx );
        //if (hResult != DS_OK )
        //    return SAM_DirectSound_SecureExit(0);

            
            
        //---------------------------------------------------------------------------------------------------------
        //Activation de la lecture en boucle
        hResult = IDirectSoundBuffer_Play ( samDirectSoundData.lpDirectSoundBuffer, 0, 0, DSBPLAY_LOOPING );
        if (hResult != DS_OK)
            return SAM_DirectSound_SecureExit(0);
        
        //---------------------------------------------------------------------------------------------------------
        //Récupération des infos sur le buffer
        memset ( &DSBufferCaps, 0, sizeof(DSBufferCaps) );
        DSBufferCaps.dwSize = sizeof(DSBufferCaps);
        hResult = IDirectSoundBuffer_GetCaps ( samDirectSoundData.lpDirectSoundBuffer, &DSBufferCaps );
        if (hResult != DS_OK)
            return SAM_DirectSound_SecureExit(0);

        //Copie du format de sortie
        memcpy ( &(samDirectSoundData.WaveFormatExOutput), &WaveFormatEx, sizeof(WAVEFORMATEX) );
        
        //Informe sur les données réellement allouées
        if (pdwSampleSizeAllocated)
        {
            *pdwSampleSizeAllocated = DSBufferCaps.dwBufferBytes / WaveFormatEx.nBlockAlign;
        }        

        //Taille du tampon de sortie
        samDirectSoundData.dwSampleSizeOutput = DSBufferCaps.dwBufferBytes / WaveFormatEx.nBlockAlign;
        samDirectSoundData.dwByteSizeOutput   = DSBufferCaps.dwBufferBytes;

        //Adresse du tampon de sortie
        samDirectSoundData.pAudioBufferOutput = NULL;
        
        //---------------------------------------------------------------------------------------------------------
        //Vide le tampon de sortie
        if (!SAM_DirectSound_BeginPaint ( &pBuffer ))
        {
            memset ( pBuffer, 0, samDirectSoundData.dwByteSizeOutput );
            SAM_DirectSound_EndPaint ( );
        }

        return dwSelectedDevice;
    }

    long    SAM_DirectSound_Close                  ( void )
    {
        //Test init
        if (!samDirectSoundData.lAlwaysInit) return -1;

        SAM_DirectSound_SecureExit(0);
        
        memset ( &samDirectSoundData, 0, sizeof(samDirectSoundData) );

        return 0;
    }

    long SAM_DirectSound_BeginPaint ( void ** pAudioBufferOutput )
    {
        DWORD           dwStatus;
        HRESULT         hResult;
        long            lRepeat;
        void            *pBuffer1;
        void            *pBuffer2;
        DWORD           dwLockSize1;
        DWORD           dwLockSize2;

        //Initialisé ?
        if (!samDirectSoundData.lAlwaysInit) return -1;

        //Récupération du status
        dwStatus = 0;
        hResult = IDirectSoundBuffer_GetStatus ( samDirectSoundData.lpDirectSoundBuffer, &dwStatus );
        if (hResult != DS_OK)
        {
            //Fix 2010/03/15
            if (dwStatus&DSBSTATUS_BUFFERLOST)
            {
                IDirectSoundBuffer_Restore ( samDirectSoundData.lpDirectSoundBuffer );
                IDirectSoundBuffer_GetStatus ( samDirectSoundData.lpDirectSoundBuffer, &dwStatus );
            }
            
            //Erreur mais on ne quitte pas.
            dwStatus = 0;
        }

        //Buffer perdu, il faut le restaurer... (c'est rare)
        if (dwStatus & DSBSTATUS_BUFFERLOST)
        {
            IDirectSoundBuffer_Restore ( samDirectSoundData.lpDirectSoundBuffer );
            
            //Fix 2010/03/15
            IDirectSoundBuffer_GetStatus ( samDirectSoundData.lpDirectSoundBuffer, &dwStatus );
        }

        //Buffer arrété, il faut le relancer... (c'est rare)
        if (!(dwStatus & DSBSTATUS_PLAYING))
        {
            IDirectSoundBuffer_Play ( samDirectSoundData.lpDirectSoundBuffer, 0, 0, DSBPLAY_LOOPING );
        }

        //Le buffer n'est pas encore locké
        samDirectSoundData.pAudioBufferOutput = NULL;
        if (pAudioBufferOutput) *pAudioBufferOutput = samDirectSoundData.pAudioBufferOutput;

        lRepeat         = 0;
        pBuffer1        = NULL;
        dwLockSize1     = samDirectSoundData.dwByteSizeOutput;

        while ( ( hResult = IDirectSoundBuffer_Lock ( 
            samDirectSoundData.lpDirectSoundBuffer, 
            0, 0,
            &pBuffer1, &dwLockSize1,
            NULL, NULL, //&pBuffer2, &dwLockSize2,
            DSBLOCK_ENTIREBUFFER ) ) != DS_OK )
            /*samDirectSoundData.lpDirectSoundBuffer,     //DSB
            0,                                          //Offset
            samDirectSoundData.dwByteSizeOutput,        //Size
            &pBuffer1, &dwLockSize1,
            &pBuffer2, &dwLockSize2,
            0 ) ) != DS_OK )*/
        {
            if (hResult != DSERR_BUFFERLOST)
            {
                //Le lock a planté
                return SAM_DirectSound_SecureExit(0);
            }
            else
            {
                IDirectSoundBuffer_Restore ( samDirectSoundData.lpDirectSoundBuffer );
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
        IDirectSoundBuffer_Unlock ( 
            samDirectSoundData.lpDirectSoundBuffer,
            samDirectSoundData.pAudioBufferOutput, samDirectSoundData.dwLockSizeOutput,
            0, 0 );

        return 0;
    }

    long SAM_DirectSound_Refresh ( void )
    {
        HRESULT hResult;

        //Initialisé ?
        if (!samDirectSoundData.lAlwaysInit) return -1;

        hResult = IDirectSound_SetCooperativeLevel ( 
            samDirectSoundData.lpDirectSound,
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
        IDirectSoundBuffer_GetCurrentPosition ( samDirectSoundData.lpDirectSoundBuffer, &dwRead, &dwWrite );

        if (pdwSampleWrite)
            *pdwSampleWrite = dwWrite / samDirectSoundData.WaveFormatExOutput.nBlockAlign;
        
        if (pdwSampleRead)
            *pdwSampleRead  = dwRead / samDirectSoundData.WaveFormatExOutput.nBlockAlign;
            
        return 0;
    }

    


    BOOL CALLBACK SAM_DSEnumProc ( LPGUID lpGUID, 
                                LPCTSTR lpszDesc,
                                LPCTSTR lpszDrvName, 
                                LPVOID lpContext )
    {
        SAM_DIRECTSOUNDENUM * psamDSoundEnum;
        
        if ((lpGUID!=NULL)&&(samDirectSoundData.lDSoundEnumCount<SAM_DSOUNDENUMCOUNT_MAX))
        {
            psamDSoundEnum = &samDirectSoundData.samDSoundEnum[samDirectSoundData.lDSoundEnumCount];
            
            //Le GUID
            memcpy ( &psamDSoundEnum->Guid, lpGUID, sizeof(GUID) );
            psamDSoundEnum->lpGUID = &psamDSoundEnum->Guid;
            
            //Les infos diverses
            strncpy ( psamDSoundEnum->szDesc, lpszDesc, 256 );
            strncpy ( psamDSoundEnum->szDrvName, lpszDrvName, 256 );
        
            samDirectSoundData.lDSoundEnumCount+=1;
        }
        return TRUE;
    }        

    long    SAM_DirectSound_GetEnum                ( void )
    {
        SAM_DIRECTSOUNDENUM * psamDSoundEnum;
        
        //Vide le contenu
        memset ( samDirectSoundData.samDSoundEnum, 0, sizeof(SAM_DIRECTSOUNDENUM)*SAM_DSOUNDENUMCOUNT_MAX );
        
        //Génération du périphérique par défaut (0)
        psamDSoundEnum = samDirectSoundData.samDSoundEnum;
        strcpy ( psamDSoundEnum->szDesc, "Default DirectSound device" );
        strcpy ( psamDSoundEnum->szDrvName, "dsound.dll" );
        samDirectSoundData.lDSoundEnumCount = 1;
        
        samDirectSoundData.pDirectSoundEnumerate ( SAM_DSEnumProc, NULL ); 
     
        return 0;   
    }
    
    long    SAM_DirectSound_GetOutputModelFlags     ( char * pszOutputModel, char * pszFlags )
    {
        if (pszOutputModel)
            strcpy ( pszOutputModel, samDirectSoundData.szOutputModel );
            
        if (pszFlags)            
            sprintf ( pszFlags, "Emulated:%d  Certified:%d", samDirectSoundData.lCaps_Emulated, samDirectSoundData.lCaps_Certified );
            
        return 0;
    }
    
    long    SAM_DirectSound_GetEnumInfo             ( DWORD dwDeviceEntry, char * pszDesc, char * pszDrvName )
    {
        if (dwDeviceEntry>=samDirectSoundData.lDSoundEnumCount)
            return -1;
        
        if (pszDesc)
            strcpy ( pszDesc, samDirectSoundData.samDSoundEnum[dwDeviceEntry].szDesc );
        
        if (pszDrvName)
            strcpy ( pszDrvName, samDirectSoundData.samDSoundEnum[dwDeviceEntry].szDrvName );
        
        return 0;
    }
    /*
BOOL CALLBACK DSEnumProc(LPGUID lpGUID, 
                         LPCTSTR lpszDesc,
                         LPCTSTR lpszDrvName, 
                         LPVOID lpContext )
{
    HWND   hCombo = *(HWND *)lpContext;
    LPGUID lpTemp = NULL;
 
    if ( lpGUID != NULL )
    {
        if (( lpTemp = malloc( sizeof(GUID))) == NULL )
        return( TRUE );
 
        memcpy( lpTemp, lpGUID, sizeof(GUID));
    }
 
    ComboBox_AddString( hCombo, lpszDesc );
    ComboBox_SetItemData( hCombo,
                ComboBox_FindString( hCombo, 0, lpszDesc ),
                lpTemp );
    return( TRUE );
}
    
    */