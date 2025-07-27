    // C RunTime Header Files
    #include "sam_win32_dsound_sdk.h"
    
    typedef HRESULT (WINAPI * __DirectSoundCreate_DLLProc)      ( LPGUID lpGUID, LPDIRECTSOUND * lplpDS, IUnknown FAR *pUnkOuter);
    typedef HRESULT (WINAPI * __DirectSoundEnumerate_DLLProc)   ( LPDSENUMCALLBACK lpDSEnumCallback, void * lpContext );


    //typedef HRESULT (WINAPI * _mDES_DirectSoundCreate8_DLLProc) ( LPGUID lpGuidDevice, LPDIRECTSOUND8 * ppDS8, LPUNKNOWN pUnkOuter );
    
    //HRESULT DirectSoundCreate8 ( LPCGUID lpcGuidDevice, LPDIRECTSOUND8 * ppDS8, LPUNKNOWN pUnkOuter );

    typedef struct {
        LPGUID  lpGUID;
        GUID    Guid;
        char    szDesc[256];
        char    szDrvName[256];
    } SAM_DIRECTSOUNDENUM;
    
    #define SAM_DSOUNDENUMCOUNT_MAX     64


    typedef struct {    
        long                                lAlwaysInit;
        
        HMODULE                             hLibrary_DirectSound;
        __DirectSoundCreate_DLLProc         pDirectSoundCreate;
        __DirectSoundEnumerate_DLLProc      pDirectSoundEnumerate;
        LPDIRECTSOUND                       lpDirectSound;
        LPDIRECTSOUNDBUFFER                 lpDirectSoundBuffer;
        //LPDIRECTSOUNDBUFFER                 lpDirectSoundPrimaryBuffer;

        HINSTANCE                           hInstanceDirectSound;
        HWND                                hWnd;

        void                                *pAudioBufferOutput;
        DWORD                               dwLockSizeOutput;
        WAVEFORMATEX                        WaveFormatExOutput;
        DWORD                               dwSampleSizeOutput;
        DWORD                               dwByteSizeOutput;
        
        SAM_DIRECTSOUNDENUM                 samDSoundEnum[SAM_DSOUNDENUMCOUNT_MAX];
        long                                lDSoundEnumCount; //1...64
        
        long                                lCaps_Certified;
        long                                lCaps_Emulated;
        char                                szOutputModel[64];

    } SAM_DIRECTSOUNDDATA;



    long    SAM_DirectSound_Open                    ( DWORD dwDeviceEntry, void * pDeviceParam, DWORD dwSampleRate, DWORD dwChannels, DWORD dwQuerySampleSize, DWORD * pdwSampleSizeAllocated );
    long    SAM_DirectSound_Close                   ( void );
    long    SAM_DirectSound_SecureExit              ( long lPartialMode );
    long    SAM_DirectSound_CloseHelper             ( void );
    long    SAM_DirectSound_OpenHelper              ( DWORD dwDeviceEntry );
    long    SAM_DirectSound_GetEnum                 ( void );
    long    SAM_DirectSound_GetEnumInfo             ( DWORD dwDeviceEntry, char * pszDesc, char * pszDrvName );
    long    SAM_DirectSound_GetOutputModelFlags     ( char * pszOutputModel, char * pszFlags );

    long    SAM_DirectSound_BeginPaint              ( void ** pAudioBufferOutput );
    long    SAM_DirectSound_EndPaint                ( void );
    long    SAM_DirectSound_Refresh                 ( void );

    long    SAM_DirectSound_GetPosition             ( DWORD * pdwSampleWrite, DWORD * pdwSampleRead );

