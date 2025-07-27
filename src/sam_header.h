#include <malloc.h>
#include <string.h>
#include <math.h>
/*
#include "..\\srcika\\ikerror.h"
#include "..\\srcika\\ikhandle.h"
#include "..\\srcika\\ikinstance.h"
#include "..\\srcika\\ikmessage.h"
#include "..\\srcika\\iktickcount.h"
#include "..\\srcika\\ikcriticalsection.h"
#include "..\\srcika\\ikaudiointerface.h"
*/

#ifdef _WIN32
#define _CRT_SECURE_NO_WARNINGS
#pragma warning(disable : 4996)
#endif


#ifdef SAMLIB_DLL_EXPORTS
    #ifndef DLLEXPORT
        #define DLLEXPORT   __declspec(dllexport)
    #endif
#else
    #define DLLEXPORT
#endif

#define _SAM_VERSIONH_DATA      (0)
#define _SAM_VERSIONM_DATA      (30)
#define _SAM_VERSIONL_DATA      (0)
#define _SAM_VERSION_BUILD_TIME (__TIME__) 
#define _SAM_VERSION_BUILD_DATE (__DATE__) 

//#define _SAM_VersionString      "0.14.4 dvlp beta "__DATE__ //" with XD4 codec"
#ifdef _DEBUG                                            
#define _SAM_TextDebug          "Debug"
#else
#define _SAM_TextDebug          ""
#endif



#define _SAM_Vendor             "Alexia Gossa"      
#define _SAM_Copyright          "(c) 2007-2010 Alexia Gossa"
#define _SAM_Title              "iKALiZER"
#define _SAM_SIMD               "SIMD Specs : "
#define _SAM_SIMD_SSE           "SSE "
#define _SAM_SIMD_SSE2          "SSE2 "
#define _SAM_SIMD_SSE3          "SSE3 "
#define _SAM_SIMD_ABSENT        "No"

typedef unsigned char       BYTE;
typedef unsigned short      WORD;
typedef unsigned long       DWORD;
typedef unsigned __int64    QWORD;
typedef float               FLOAT;
typedef float               FLOAT32;
typedef double              FLOAT64;

typedef FLOAT32             VECT;
typedef VECT                VECT2[2];
typedef VECT                VECT3[3];
typedef VECT                VECT4[4];

typedef signed char         INT8;
typedef signed short        INT16;
typedef signed long         INT32;
typedef signed __int64      INT64;
typedef unsigned char       UINT8;
typedef unsigned short      UINT16;
typedef unsigned long       UINT32;
typedef unsigned __int64    UINT64;

typedef unsigned long       BOOL;

#define                     SAM_KERNEL_ALLOC(n)         malloc(n)
#define                     SAM_KERNEL_REALLOC(p,n)     realloc(p,n)
#define                     SAM_KERNEL_FREE(p)          free(p)

#define                     sam_FIR_RECTANGLE           0x00
#define                     sam_FIR_KAISERBESSEL        0x01
#define                     sam_FIR_BLACKMANHARRIS      0x02
#define                     sam_FIR_HAMMING             0x03
#define                     sam_FIR_GOSSAMODKBD         0x04
#define                     sam_FIR_LANCZOS             0x05
#define                     sam_FIR_RIESZ               0x06
#define                     sam_FIR_RIESZ_TO_GOSSAMODKBD    0x07
#define                     sam_PI		                3.1415926535897932384626433832795
#define                     sam_ABS(a)                  ((a<0)?(-a):(a))

#define                     sam_FORMAT_MONO_FLOAT32     0x00
#define                     sam_FORMAT_MONO_XPCM8       0x01
#define                     sam_FORMAT_MONO_PCM8        0x02
#define                     sam_FORMAT_MONO_PCM16       0x03
#define                     sam_FORMAT_MONO_XD4         0x04
#define                     sam_FORMAT_MONO_XD4ADPCM    0x05
#define                     sam_FORMAT_STEREO_FLOAT32   0x10
#define                     sam_FORMAT_STEREO_XPCM8     0x11
#define                     sam_FORMAT_STEREO_PCM8      0x12
#define                     sam_FORMAT_STEREO_PCM16     0x13
#define                     sam_FORMAT_STEREO_XD4       0x14
#define                     sam_FORMAT_STEREO_XD4ADPCM  0x15
#define                     sam_FORMAT_TYPEMASK         0x0F
#define                     sam_FORMAT_CHANNELMASK      0xF0

#define                     sam_GRANULE_BUFFERBYTES     4080

#define                     sam_VOICE_MAXCOUNT          256
#define                     sam_VOICE_VIRTUALCOUNT      1024
#define                     sam_FIRLEN                  32


#define                     sam_ALIGN_SIZE              16
#define                     sam_ALIGN                   __declspec(align(sam_ALIGN_SIZE))
#define                     sam_ALIGN_SPEC(x)           __declspec(align(x))

#define                     ika_ALIGN_SIZE              16
#define                     ika_ALIGN                   __declspec(align(ika_ALIGN_SIZE))
#define                     ika_ALIGN_SPEC(x)           __declspec(align(x))

#define                     sam_LATENCY_DURATION_MIN    2
#define                     sam_LATENCY_DURATION_MAX    50
#define                     sam_BUFFER_DURATION_MIN     250
#define                     sam_BUFFER_DURATION_MAX     1000
#define                     sam_SAMPLINGRATE_MAX        48000



//Format interne
#define ika_INTERNAL_AUDIO_FORMAT  (sam_FORMAT_MONO_PCM16)
//#define ika_INTERNAL_AUDIO_FORMAT  (sam_FORMAT_MONO_XD4)


/*


*/
//long    SAM_Open ( void * pDeviceParam );
typedef struct {
    DWORD       dwTotalMemorySoundAllocCount;               //Mémoire totale pour les sons de SAM (en mega octets)
    DWORD       dwHardwareSamplingRate;                     //Fréquence d'échantillonnage du matériel (en Hz) - défaut:48000Hz
    DWORD       dwHardwareChannelsMode;                     //Mode de sortie audio sur 2x4bits => 0xXY, X=ChannelsCount, Y=RenderMode
    DWORD       dwHardwareBufferLatencyDuration;            //Latence du buffer matériel (en ms) - défaut:20ms
    DWORD       dwSoftwareBufferDuration;                   //Taille du buffer logiciel (en ms) - défaut:250ms
    DWORD       dwSoftwareVoicesCount;                      //Nombre de voies logicielles pouvant être lues en simultanées - défaut:64
    DWORD       dwSoftwareSFXCount;                         //Nombre d'effets pouvant être chargés en mémoire - défaut:4096
    DWORD       dwSoftwareResamplingQuality;                //Qualité du ré-échantillonnage 0...3 - défaut:0
    DWORD       dwStreamingBufferDuration;
    DWORD       dwProcessFlag;                              //Flag de process... Threads...
    DWORD       dwHardwareDeviceSelected;
    DWORD       dwDynamicDelayLinesCount;
} SAM_CONFIG;

#define SAM_INFO_VERSION                    0x0000
#define SAM_INFO_VENDOR                     0x0001
#define SAM_INFO_COPYRIGHT                  0x0002
#define SAM_INFO_TITLE                      0x0003
#define SAM_INFO_VERSION_DATA               0x0004
#define SAM_INFO_VERSION_BUILD              0x0005

#define SAM_INFO_MEMORY_TOTAL               0x0100
#define SAM_INFO_MEMORY_SFXFREE             0x0101
#define SAM_INFO_MEMORY_SFXUSED             0x0102
#define SAM_INFO_MEMORY_SFXTOTAL            0x0103

#define SAM_INFO_RENDER_MODE                0x0200

#define SAM_INFO_SYSTEM_INSTANCES           0x0300
#define SAM_INFO_SYSTEM_SIMD                0x0301


#define SAM_INFO_DEVICE_ENUM_00             0x0400
#define SAM_INFO_DEVICE_ENUM_63             0x043F

#define SAM_INFO_DEVICE_OUTPUT_MODEL        0x04FD
#define SAM_INFO_DEVICE_OUTPUT_FLAGS        0x04FE
#define SAM_INFO_DEVICE_GETCURRENT          0x04FF


#define SAM_INFO_SUPPORT_SELECT_DEVICE              0x1000
#define SAM_INFO_SUPPORT_MESSAGE_UNDEFINED          0x1001
#define SAM_INFO_SUPPORT_MESSAGE_VOICE_SYNCFREEZE   0x1002
#define SAM_INFO_SUPPORT_MESSAGE_VOICE_ISPLAYED     0x1003
#define SAM_INFO_SUPPORT_MESSAGE_VOICE_POSITION     0x1004

#define SAM_PROCESS_ENABLESMP               0x0003


#define SAM_MESSAGE_DEVICESETENUM           0x0000
#define SAM_MESSAGE_DYNAMICDELAYLINES       0x0001
#define SAM_MESSAGE_VOICE_SYNCFREEZE        0x0002
#define SAM_MESSAGE_VOICE_ISPLAYED          0x0003
#define SAM_MESSAGE_VOICE_VIRTUALCOUNT      0x0004
#define SAM_MESSAGE_VOICE_POSITION_GET      0x0005
#define SAM_MESSAGE_VOICE_POSITION_SET      0x0006
#define SAM_MESSAGE_VOICE_POSITIONTICK_GET  0x0007
#define SAM_MESSAGE_VOICE_POSITIONTICK_SET  0x0008
#define SAM_MESSAGE_PROCESS_MIXER           0x0009
#define SAM_MESSAGE_OUTPUT_FLUSH            0x000A
#define SAM_MESSAGE_RENDERMODE_SET          0x000B
#define SAM_MESSAGE_MAXUSAGE_SET            0x000C
#define SAM_MESSAGE_SOFTRESTART             0x000D
#define SAM_MESSAGE_SAMPLINGRATE_SET        0x000E
#define SAM_MESSAGE_DEVICESELECT_SET        0x000F
#define SAM_MESSAGE_LATENCYDURATION_SET     0x0010
#define SAM_MESSAGE_BUFFERDURATION_SET      0x0011
#define SAM_MESSAGE_LIMITERLEVEL_SET        0x0012
#define SAM_MESSAGE_SFX_DEFAULTLEVEL_SET    0x0013
#define SAM_MESSAGE_VOICE_SPEEDSHIFT_SET    0x0014
#define SAM_MESSAGE_VOICE_SPEEDSHIFT_GET    0x0015
#define SAM_MESSAGE_LATENCYDURATION_BU_GET  0x0020
#define SAM_MESSAGE_MIXER_OUTPUT_GET        0x0030
#define SAM_MESSAGE_MIXER_OUTPUT_KEEP       0x0031
#define SAM_MESSAGE_MIXER_SET_RECORD2QUAKE  0x0032

#define SAM_MESSAGE_UNDEFINED               0xFFFFFFFF


#define SAM_VOICE_MASK                      0x7FFFFFFF
#define SAM_VOICE_FASTACCESS                0x80000000

typedef struct {
    DWORD dwSamplingRate;           //0xFFFFFFFF if don't change
    DWORD dwChannelMode;            //0xFFFFFFFF if don't change
    DWORD dwDeviceSelect;           //0xFFFFFFFF if don't change
    DWORD dwLatencyDuration;        //0xFFFFFFFF if don't change
    DWORD dwBufferDuration;         //0xFFFFFFFF if don't change
} SAM_SoftRestart;

DLLEXPORT   long    SAM_Open ( void * pDeviceParam, SAM_CONFIG * psamConfig );
DLLEXPORT   long    SAM_Close ( void );
DLLEXPORT   long    SAM_LimiterSet ( long lMode );
DLLEXPORT   long    SAM_GetInfo ( DWORD dwInfoID, DWORD * pdwOutData, char * pszOutData );

DLLEXPORT   long    SAM_Message ( DWORD dwMessage, DWORD dwParamA, DWORD dwParamB );            

DLLEXPORT   long    SAM_GainSet ( BYTE bSetGainMaskVMS, float fGainVoice, float fGainMusic, float fGainStreaming );
            void    SAM_ForceAffinity ( void );

//void SAM_DEBUG_TEXT_ ( char * pszText, ... );
/*
    Les messages d'erreurs de SAM
*/
long        SAM_ErrorAddMessage ( char * psz );

/*
    Psychoacoustic
*/
typedef struct {
    BYTE                    bAttenuationLevelSensor[1024];
    DWORD                   dwPsyPointDuration;
    DWORD                   dwPsyPointDuration_Mul_Shr10;
} SAM_SFX_PSY;


/*
    Le codec XPCM
*/
void        SAM_XPCM_Init        ( void );
BYTE        SAM_XPCM_EncodeValue ( FLOAT32 f32AbsoluteValue );
FLOAT32     SAM_XPCM_DecodeValue ( BYTE bXPCMValue );

/*
    Le codec XD4
*/
void        SAM_XD4_Init        ( void );
DWORD       SAM_XD4_EncodeValue ( FLOAT32 f32Value1, FLOAT32 f32Value2, FLOAT32 f32Value3, FLOAT32 f32Value4 );
FLOAT32     SAM_XD4_DecodeValue ( DWORD dwXD4Data, DWORD dwValueIndex );

/*
    Le codec XD4ADPCM
*/
void        SAM_XD4ADPCM_Init        ( void );
DWORD       SAM_XD4ADPCM_EncodeValue ( FLOAT32 f32Value1, FLOAT32 f32Value2, FLOAT32 f32Value3, FLOAT32 f32Value4 );
FLOAT32     SAM_XD4ADPCM_DecodeValue ( DWORD dwXD4Data, DWORD dwValueIndex, void * pCodecValue );

/*
    Le codec LPCM-16
*/
void        SAM_LPCM_Init           ( void );
WORD        SAM_LPCM_EncodeValue16  ( FLOAT32 f32AbsoluteValue );
FLOAT32     SAM_LPCM_DecodeValue16  ( DWORD dwLPCMValue, long lEntryValueUsed );


/*
    Transfert audio
*/
//long SAM_CopyAudioData ( void * pAudioDataTarget, BYTE bFormatTarget, void * pAudioDataSource, BYTE bFormatSource, DWORD dwSamplesCount, float fGain );
long SAM_CopyAudioData ( void * pAudioDataTarget, BYTE bFormatTarget, void * pAudioDataSource, BYTE bFormatSource, DWORD dwSamplesCount, float fGain );
long SAM_PreCopyAudioData ( void * pAudioDataSource, BYTE bFormatSource, DWORD dwSamplesCount, float fSampleRate, float * pfGain, SAM_SFX_PSY * ppsyData );

/*
    Les tables
*/
extern unsigned long sam_dwSineInterpolationLUT512[512];
extern unsigned long sam_dwSincDivisorLUT512[512];

/*
    Le FIR
*/
long SAM_FIR_Design         ( long lOrder, float * pf32Coef, float f32RatioCut, long lWindowType, float f32WindowParam );
long SAM_FIR_FFT_Design     ( long lLength, float * pf32Coef, float f32RatioCut );
long SAM_FIR_FFT_DesignEq   ( long lCoefLength, float * pf32Coef, long lEqualizerLenght_pow2, float * f32EqualizerCoef );

/*  ################################################################################################################################################
    
    OSIndependant
    - CriticalSection
    - Event
    - Thread
    - Sleep

*/


typedef struct {
    DWORD               dwData[16];
} _OSI_CRITICAL_SECTION;

typedef struct {
    DWORD               dwData[24];
} _OSI_EVENT;

typedef struct {
    DWORD               dwData[32];
} _OSI_THREAD;

long    _OSI_EnterCriticalSection                               ( _OSI_CRITICAL_SECTION * pCriticalSection );
long    _OSI_InitializeCriticalSectionAndSpinCount              ( _OSI_CRITICAL_SECTION * pCriticalSection, DWORD dwSpinCount );
long    _OSI_LeaveCriticalSection                               ( _OSI_CRITICAL_SECTION * pCriticalSection );
long    _OSI_InitializeCriticalSection                          ( _OSI_CRITICAL_SECTION * pCriticalSection );
long    _OSI_DeleteCriticalSection                              ( _OSI_CRITICAL_SECTION * pCriticalSection );

long    _OSI_InitializeEvent                                    ( _OSI_EVENT * pEvent, BOOL bEnableManualReset, BOOL bInitialState );
long    _OSI_DeleteEvent                                        ( _OSI_EVENT * pEvent );
long    _OSI_SetEvent                                           ( _OSI_EVENT * pEvent );
long    _OSI_ResetEvent                                         ( _OSI_EVENT * pEvent );
long    _OSI_WaitForEvent                                       ( _OSI_EVENT * pEvent, unsigned long dwMilliseconds, long * plReturnWaitState );

long    _OSI_ThreadOpen                                         ( _OSI_THREAD * pThread, long (*pThreadProc) ( void * pProcData ), unsigned long dwCreationFlags, void * pInputParams );
long    _OSI_ThreadOpenEx                                       ( _OSI_THREAD * pThread, long (*pThreadProc) ( void * pProcData ), unsigned long dwCreationFlags, unsigned long dwStackSizeMB, void * pParams );
long    _OSI_ThreadClose                                        ( _OSI_THREAD * pThread, long lMaxDelayBeforeKill_ms );
long    _OSI_ThreadGetProcParams                                ( void * pProcData, void ** pParams );
long    _OSI_ThreadSetPriority                                  ( _OSI_THREAD * pThread, DWORD dwPriority );
long    _OSI_ThreadSetName                                      ( _OSI_THREAD * pThread, char * pszShortName );
long    _OSI_ThreadSuspend                                      ( _OSI_THREAD * pThread );
long    _OSI_ThreadResume                                       ( _OSI_THREAD * pThread );
long    _OSI_ThreadDoWhileAndWaitExit                           ( void * pProcData );
long    _OSI_ThreadSetAffinity                                  ( _OSI_THREAD * pThread, DWORD dwAffinityMask );
#ifdef _WIN32
    /*
    #ifndef THREAD_BASE_PRIORITY_LOWRT
        #define THREAD_BASE_PRIORITY_LOWRT  15  // value that gets a thread to LowRealtime-1
        #define THREAD_BASE_PRIORITY_MAX    2   // maximum thread base priority boost
        #define THREAD_BASE_PRIORITY_MIN    -2  // minimum thread base priority boost
        #define THREAD_BASE_PRIORITY_IDLE   -15 // value that gets a thread to idle
    #endif
    */
    #define _OSI_THREAD_BASE_PRIORITY_LOWRT     (THREAD_BASE_PRIORITY_LOWRT)
    #define _OSI_THREAD_BASE_PRIORITY_MAX       (THREAD_BASE_PRIORITY_MAX)
    #define _OSI_THREAD_BASE_PRIORITY_MIN       (THREAD_BASE_PRIORITY_MIN)
    #define _OSI_THREAD_BASE_PRIORITY_IDLE      (THREAD_BASE_PRIORITY_IDLE)
    #define _OSI_CREATE_SUSPENDED               (CREATE_SUSPENDED)
    #define _OSI_STILL_ACTIVE                   (STILL_ACTIVE)
#endif
#define _OSI_THREAD_PRIORITY_LOWEST             (_OSI_THREAD_BASE_PRIORITY_MIN)
#define _OSI_THREAD_PRIORITY_BELOW_NORMAL       (_OSI_THREAD_PRIORITY_LOWEST+1)
#define _OSI_THREAD_PRIORITY_NORMAL             (0)        
#define _OSI_THREAD_PRIORITY_HIGHEST            (_OSI_THREAD_BASE_PRIORITY_MAX)
#define _OSI_THREAD_PRIORITY_ABOVE_NORMAL       (_OSI_THREAD_PRIORITY_HIGHEST-1)
#define _OSI_THREAD_PRIORITY_TIME_CRITICAL      (_OSI_THREAD_BASE_PRIORITY_LOWRT)
#define _OSI_THREAD_PRIORITY_IDLE               (_OSI_THREAD_BASE_PRIORITY_IDLE)
#define _OSI_THREAD_CREATE_SUSPENDED            (_OSI_CREATE_SUSPENDED)
#define _OSI_THREAD_STILL_ACTIVE                (_OSI_STILL_ACTIVE)

long    _OSI_Sleep                                              ( unsigned long dwMilliseconds );
long    _OSI_GetTickCount                                       ( unsigned long * pdwTick );

/* ################################################################################################################################################

    Audio Driver
    - DirectSound
*/
long    SAM_DirectSound_Open                    ( DWORD dwDeviceEntry, void * pDeviceParam, DWORD dwSampleRate, DWORD dwChannels, DWORD dwQuerySampleSize, DWORD * pdwSampleSizeAllocated );
long    SAM_DirectSound_Close                   ( void );
long    SAM_DirectSound_BeginPaint              ( void ** pAudioBufferOutput );
long    SAM_DirectSound_EndPaint                ( void );
long    SAM_DirectSound_Refresh                 ( void );
long    SAM_DirectSound_GetPosition             ( DWORD * pdwSampleWrite, DWORD * pdwSampleRead );
long    SAM_DirectSound_GetEnum                 ( void );
long    SAM_DirectSound_GetEnumInfo             ( DWORD dwDeviceEntry, char * pszDesc, char * pszDrvName );

// ################################################################################################################################################



/* ################################################################################################################################################

    Granules
    Enhanced memory management for highest memory usage
*/
typedef struct {
    BYTE        bAudioData[sam_GRANULE_BUFFERBYTES];
    DWORD       dwIdGranuleCurrent;
    DWORD       dwIdGranulePrevious;
    DWORD       dwIdGranuleNext;
    WORD        wSamplesCount;
    WORD        wBytesCount;
} SAM_GRANULE;

typedef struct {
    DWORD           dwRealAudioBytesInGranule;
    DWORD           dwFirstFreeIdGranule;
    DWORD           dwGranulesCount;
    DWORD           dwFreeBytes;
    SAM_GRANULE     *pGranule;
} SAM_GRANULESDATA;

long    SAM_GranulesOpen ( DWORD dwTotalNeedsBytesCount, void * pAllocatedMemory );
long    SAM_GranulesClose ( void );

long    SAM_GranulesAlloc ( DWORD dwTotalSamplesCount, BYTE bFormat, DWORD * pdwIdGranule, float *pfInvertedGain, void * pAudioDataSource, BYTE bFormatSource, float fSampleRate, SAM_SFX_PSY * ppsyData );
long    SAM_GranulesFree ( DWORD dwIdGranule );
long    SAM_GranulesGetData ( DWORD dwIdGranule, void ** pData );
long    SAM_GranulesGetNext ( DWORD dwIdGranule, DWORD * pdwIdGranuleNext );

/*

    Le mixer

*/
long SAM_ProcessProc_Mixer ( long lMode, DWORD * pdwProcessedSamples );
long SAM_ThreadProc_Mixer ( void * pProcData );
void SAM_PostProcessCopy ( FLOAT32 * pf32SourceData, INT16 * pi16TargetData, long lSamplesCopyCount, long lTargetOffset, long lLimiterMode );
void SAM_PostProcessCopy_Init ( void );
void SAM_PostProcessCopy_0x26_Init ( void );

void SAM_EnterProtectThreadMixer ( void );
void SAM_LeaveProtectThreadMixer ( void );

long IKA_MixerFreeze ( long bFreeze );





long    SAM_FormatGetBytesCount ( BYTE bFormat, DWORD * pdwBytesCount );

/*
    Les SFX
*/
typedef struct {
    long                    bProtected;             //bit0 = 1 if protected against SAM_SFX_GetOldest, bit1 = temporary protection
    long                    bIsUsed;                //0 if used
    long                    bIsLoaded;              //1 if sound data are loaded
    char                    szName[256];
    DWORD                   dwGranuleFirst;
    DWORD                   dwGranuleCount;
    
    SAM_SFX_PSY             psyData;
    
    //DWORD                   dwGranulePsychoAcoustic;
    //DWORD                   *pdwPsychoAcousticData;
    
    BYTE                    bFormat;                //Audio format
    DWORD                   dwSamplesCount;
    float                   fSampleRate;
    float                   fReplayGain;
    float                   fUserDefaultGain;
    
    long                    lInUseCount;
    DWORD                   dwAllocTimeStamp;
    DWORD                   dwLastAccessTimeStamp;
    
    DWORD                   dwLoopBeginPositionSample;
    DWORD                   dwLoopBeginGranuleID;
    DWORD                   dwLoopBeginGranulePositionSample;
    DWORD                   dwLoopEndPositionSample;
} SAM_SFX;

void SAM_SFX_PreOpenMemoryNeeds ( DWORD dwSFXCount, DWORD *pdwBytesNeeds );
long SAM_SFX_Open ( DWORD dwSFXCount, void * pAllocatedMemory );
long SAM_SFX_Close ( void );
long SAM_SFX_GetIt ( DWORD dwHandle, SAM_SFX ** psamSFX );
void SAM_SFX_EnterLock ( void );
void SAM_SFX_LeaveLock ( void );


DLLEXPORT   long SAM_SFX_Load ( DWORD dwHandle, char * pszSFXName, DWORD dwSampleRate, DWORD dwSamplesCount, BYTE bStereo, BYTE b16bits, void * pAudioData, DWORD * pdwAllocatedHandle );
DLLEXPORT   long SAM_SFX_Unload ( DWORD dwHandle );
DLLEXPORT   long SAM_SFX_Free ( DWORD dwHandle );
DLLEXPORT   long SAM_SFX_IsLoaded ( char * pszSFXName, DWORD * pdwAllocatedHandle );
DLLEXPORT   long SAM_SFX_GetLoadedMemoryState ( DWORD dwHandle, DWORD * pdwLoadedMemoryState );
DLLEXPORT   long SAM_SFX_GetOldest ( DWORD * pdwOldestHandle, DWORD * pdwReservedHandle, DWORD dwReservedHandleCount );
DLLEXPORT   long SAM_SFX_GetName ( DWORD dwHandle, char * pszSFXName );
            long SAM_SFX_SetUserDefaultLevel ( DWORD dwHandle, float fDefaultLevel );



/*
    
    VOICE

*/
void    SAM_VOICE_PreOpenMemoryNeeds ( DWORD dwVirtualVoicesCount, DWORD *pdwBytesNeeds );
long    SAM_VOICE_Open ( DWORD dwVirtualVoicesCount, DWORD dwRealVoicesCount, void * pAllocatedMemory );
long    SAM_VOICE_Close ( void );
void    SAM_VOICE_Mirror ( DWORD dwInternalTimer );
void    SAM_VOICE_Mix ( float * pfSoftwareBuffer, DWORD dwSoftwareBufferChannelCount, DWORD dwSoftwareBufferSamplesToMixCount, DWORD dwInternalTimer  );
long    _SAM_VOICE_GetPlayTick ( float fPlayRate, DWORD * pdwTickIncrement );

DLLEXPORT   long    SAM_VOICE_Alloc ( DWORD * pdwVoiceHandle, long lForceAlloc, DWORD * pdwKilledVoiceHandle );
DLLEXPORT   long    SAM_VOICE_AllocByVoiceHandle ( DWORD dwVoiceHandle, long lForceAlloc );
DLLEXPORT   long    SAM_VOICE_AllocByUserID ( DWORD dwUserID, long lForceAlloc );
DLLEXPORT   long    SAM_VOICE_Free ( DWORD dwVoiceHandle );
DLLEXPORT   long    SAM_VOICE_FreeByUserID ( DWORD dwUserID );
DLLEXPORT   long    SAM_VOICE_GetHandleByUserID ( DWORD * pdwVoiceHandle, DWORD dwStartVoiceHandle, DWORD dwUserID );


DLLEXPORT   long    SAM_VOICE_SetSFX ( DWORD dwVoiceHandle, DWORD dwHandleSFX );
DLLEXPORT   long    SAM_VOICE_SetSampleRate ( DWORD dwVoiceHandle, DWORD dwSampleRate_Hz );
DLLEXPORT   long    SAM_VOICE_SetLoop ( DWORD dwVoiceHandle, long bLoop );
DLLEXPORT   long    SAM_VOICE_SetMasterLevel ( DWORD dwVoiceHandle, float fMasterLevel_dB );
DLLEXPORT   long    SAM_VOICE_SetDistanceLevel ( DWORD dwVoiceHandle, float fDistanceLevel_dB );
DLLEXPORT   long    SAM_VOICE_SetRatioIIR ( DWORD dwVoiceHandle, float fRatioIIR );
DLLEXPORT   long    SAM_VOICE_SetPlay ( DWORD dwVoiceHandle, long bPlay );
DLLEXPORT   long    SAM_VOICE_SetOrigin ( DWORD dwVoiceHandle, long lAngleDegrees, float fDistanceMeters );

DLLEXPORT   long    SAM_VOICE_GetUserData ( DWORD dwVoiceHandle, BYTE * pbReceiveBuffer, DWORD dwByteCount );
DLLEXPORT   long    SAM_VOICE_SetUserData ( DWORD dwVoiceHandle, BYTE * pbSendBuffer, DWORD dwByteCount );
DLLEXPORT   long    SAM_VOICE_GetUserID ( DWORD dwVoiceHandle, DWORD * pdwUserID );
DLLEXPORT   long    SAM_VOICE_SetUserID ( DWORD dwVoiceHandle, DWORD dwUserID );

DLLEXPORT   long    SAM_VOICE_GetVoiceUsedCount ( DWORD * pdwVoiceTotalUsedCount, DWORD * pdwVoiceUsedCountUnlooped, DWORD * pdwVoiceUsedCountLooped );


/*

    STREAM

*/
    typedef struct {
        BYTE        bUsed;
        BYTE        bFormat;
        QWORD       qwPositionID;           //Identifiant 64 bits de comptage
        DWORD       dwSampleRate;
        DWORD       dwSamplesCount;
        float       fReplayGain;
        DWORD       dwGranuleFirst;
        DWORD       dwCurrentGranuleID;
    } SAM_STREAM;

void SAM_STREAM_Mix ( float * pfSoftwareBuffer, DWORD dwSoftwareBufferChannelCount, DWORD dwSoftwareBufferSamplesToMixCount, DWORD dwInternalTimer );
DWORD SAM_STREAM_MixSingleGranule ( SAM_STREAM * psamStream, float fGlobalGain, DWORD dwLoopCount, float * pfSoftwareBuffer, DWORD dwSoftwareBufferChannelCount );

DLLEXPORT   long    SAM_STREAM_SetState ( BYTE bPlayState );
DLLEXPORT   long    SAM_STREAM_AddData ( DWORD dwSampleRate, DWORD dwSamplesCount, BYTE bStereo, BYTE bQuantification, void * pAudioData, float fVolume );

/*

    MUSIC

*/
    typedef struct {
        BYTE    bUsed;
        BYTE    bPlayed;
        
        DWORD   dwSFXHandle;
        BYTE    bLoop;
        
        DWORD   dwNextSFXMusic;
        DWORD   dwCurrentGranuleID;
    
    } SAM_SFX_MUSIC;

void SAM_MUSIC_ApplyNewSampleRate ( DWORD dwNewSampleRate );
DWORD SAM_MUSIC_MixSingleGranule ( SAM_SFX_MUSIC * psamSFXMusic, SAM_SFX * psamSFX, float fGlobalGain, DWORD dwLoopCount, float * pfSoftwareBuffer, DWORD dwSoftwareBufferChannelCount );
void SAM_MUSIC_Mix ( float * pfSoftwareBuffer, DWORD dwSoftwareBufferChannelCount, DWORD dwSoftwareBufferSamplesToMixCount, DWORD dwInternalTimer );
DLLEXPORT   long    SAM_MUSIC_AddSFX ( DWORD dwHandleSFX, DWORD * pdwAllocatedEntry );
DLLEXPORT   long    SAM_MUSIC_Delete ( DWORD dwEntry );
DLLEXPORT   long    SAM_MUSIC_SetLoop ( DWORD dwEntry, BYTE bLoop );
DLLEXPORT   long    SAM_MUSIC_SetJump ( DWORD dwEntry, DWORD dwJumpEntry );
DLLEXPORT   long    SAM_MUSIC_Play ( DWORD dwEntry );

/*

    RENDER
    
    Ne surtout pas modifier la valeur de 254, cela permet d'avoir 2048 octets pour une entrée SAM_RENDER254

*/
sam_ALIGN typedef struct
{
    DWORD                   dwDelayIndex[254];       // [254] Index in the delay line
    DWORD                   dwTmpA[2];
    float                   fDelayGain[254];         // [254] Gain
    //DWORD                   dwChannelIndex[64];     // [64] Channel index
    DWORD                   dwEntryCount;           // 0...64
    DWORD                   dwTmpB;
} SAM_RENDER254;

typedef struct
{
    DWORD                   dwDelayIndex[2048];       // [64] Index in the delay line    
    float                   fDelayGain[2048];         // [64] Gain
    //DWORD                   dwChannelIndex[64];     // [64] Channel index
    DWORD                   dwEntryCount;           // 0...64
    DWORD                   dwTmp[3];
} SAM_RENDER2K;

void SAM_RENDER_Init_2CH_Headphones_Holographic ( float fDistanceWithinSpeakers, float fSampleRate, SAM_RENDER254 * psamRender );
void SAM_RENDER_Init_2CH_Headphones_Hybrid_HRTF ( float fDistanceWithinSpeakers, float fSampleRate, SAM_RENDER254 * psamRender );
//void SAM_RENDER_Init_2CH_Headphones ( float fDistanceWithinSpeakers, float fSampleRate, SAM_RENDER * psamRender );
void SAM_RENDER_Init_2CH_Stereo ( float fDistanceWithinSpeakers, float fSampleRate, SAM_RENDER254 * psamRender );
void SAM_RENDER_Init_2CH_360VS ( float fDistanceWithinSpeakers, float fSampleRate, SAM_RENDER254 * psamRender );
void SAM_RENDER_Init_2CH_DolbySurround_ProLogic ( float fDistanceWithinSpeakers, float fSampleRate, SAM_RENDER254 * psamRender );
void SAM_RENDER_Init_2CH_DolbySurround_ProLogicII ( float fDistanceWithinSpeakers, float fSampleRate, SAM_RENDER254 * psamRender );
void SAM_RENDER_Init_4CH_L_R_C_S ( float fDistanceWithinSpeakers, float fSampleRate, SAM_RENDER254 * psamRender );
void SAM_RENDER_Init_4CH_L_R_SL_SR ( float fDistanceWithinSpeakers, float fSampleRate, SAM_RENDER254 * psamRender );
void SAM_RENDER_Init_6CH_L_R_C_LFE_SL_SR ( float fDistanceWithinSpeakers, float fSampleRate, SAM_RENDER254 * psamRender );
void SAM_RENDER_Init_6CH_L_R_C_noLFE_SL_SR ( float fDistanceWithinSpeakers, float fSampleRate, SAM_RENDER254 * psamRender );
void SAM_RENDER_Init_8CH_0x26 ( float fDistanceWithinSpeakers, float fSampleRate, SAM_RENDER254 * psamRender );
void SAM_RENDER_GetAngleToRender ( SAM_RENDER254 * psamRenderTable, long lAngleDegrees, SAM_RENDER254 ** psamRenderSet, long * plIndexRender );
void SAM_RENDER_MaximizeLevel ( SAM_RENDER254 * pRender, long lChannelCount, long lSampleRate );
long SAM_RENDER_Processor_CreateRender ( float fSampleRate, SAM_RENDER254 * psamRender, long lAngle, float fDistanceMeters );
void SAM_RENDER_InitChannelsModeValues ( float * pfDistanceWithinSpeakers, void ** pRenderInitProc );
void SAM_RENDER_CorrectOffsetDC ( SAM_RENDER254 * pRender, DWORD dwChannelCount );
void SAM_VOICE_CinemaAGC_Process ( DWORD * pdwVoiceSortIndexTable, DWORD dwActivePhysicalVoiceCount );
    void SAM_RENDER_ValidateChannelsModeValues ( DWORD *pdwChannelMode );


    //#################################################################################################################################
    //
    //  CPUID
    //
    #define             CPUID_GET_FAMILY                        0x00000002
    #define             CPUID_GET_MODEL                         0x00000003
    #define             CPUID_GET_STEPPING                      0x00000004
    #define             CPUID_GET_EXTFAMILY                     0x00000005
    #define             CPUID_GET_EXTMODEL                      0x00000006
    #define             CPUID_GET_VENDOR_STR                    0x00000010
    #define             CPUID_GET_NAME_STR                      0x00000011
    #define             CPUID_GET_OEMNAME_STR                   0x00000012
    #define             CPUID_GET_MMX                           0x00000020
    #define             CPUID_GET_SSE                           0x00000021
    #define             CPUID_GET_SSE2                          0x00000022
    #define             CPUID_GET_SSE3                          0x00000023
    #define             CPUID_GET_SSSE3                         0x00000024
    #define             CPUID_GET_3DNOW                         0x00000030
    #define             CPUID_GET_3DNOWEXT                      0x00000031
    #define             CPUID_GET_X64                           0x00000040
    #define             CPUID_GET_HTT                           0x00000041
    #define             CPUID_GET_VT                            0x00000042
    #define             CPUID_GET_NX                            0x00000043
    #define             CPUID_GET_MSR                           0x00000044
    #define             CPUID_GET_CACHEDATAL1                   0x00000050
    #define             CPUID_GET_CACHECODEL1                   0x00000051
    #define             CPUID_GET_CACHEDATAL2                   0x00000053
    #define             CPUID_GET_CACHEDATAL3                   0x00000054
    #define             CPUID_GET_LOCALAPICID                   0x00000060
    #define             CPUID_GET_LOGICALCORECOUNT              0x00000061
    #define             CPUID_GET_THREADSPERCORE                0x00000062
    #define             CPUID_GET_CORESPERPROCESSOR             0x00000063
    #define             CPUID_GET_DTS_INTEL                     0x00000070
    #define             CPUID_GET_DTS_AMD                       0x00000071
    #define             CPUID_GET_DIODE_SENSOR                  0x00000072
    #define             CPUID_GET_ACPI                          0x00000073


    long                CPUID_Get                               ( long lSpecificationType, void * pReturnedSpecification );


    //#################################################################################################################################
    
    void SAM_StringAccentKill ( char * psz );
    
    
    //#################################################################################################################################
    //
    //  Design de filtre IIR
    //
    void    SAM_IIR_Design ( long lOrder, float *pf32ABCoef, long * plABCoefCount, float f32RatioCut, long lEnableHighPass );
    
    // 
    //  Le timer
    long SAM_TIMER_Open ( void );
    long SAM_TIMER_Close ( void );
    long SAM_TIMER_Increment ( DWORD * pdwTimer );
    long SAM_TIMER_GetIt ( DWORD * pdwTimer );
    
    //#################################################################################################################################
    //
    //  Maths
    //
    long SAM_MATH_lroundf ( float f );
    long SAM_MATH_lroundd ( double d );
    
    
    long        SAM_INTERPOLATOR_Init ( void );


    //#################################################################################################################################
    //
    //  L'état de SAM_DATA pour le soft-restart
    //

    typedef struct {
        long    lWithoutStackMode;
        DWORD   dwOutputHardwareChannel;
        DWORD   dwOutputSoftwareChannel;
        DWORD   dwOutputEncoder;
        DWORD   dwChannelMode;
        DWORD   dwHardwaremixSampleRate;
        DWORD   dwHardwareBufferLatencySamplesCount;
        DWORD   dwHardwareAndSoftwareBufferSamplesCount;
        DWORD   dwDeviceSelect;
    } SAM_DATASTATE;
    
    void        SAM_DATA_SaveState ( SAM_DATASTATE * psamDataState );
    void        SAM_DATA_RestoreState ( SAM_DATASTATE * psamDataState );


    //Le débug
    void    _SAM_DEBUG_TEXT_ ( char * pszText, ... );
    
    //#################################################################################################################################
    //  IKA_Handle : Gestion des handles pour iKALiZER et l'appelant
    long IKA_HandleCreate ( DWORD * pdwHandle );
    long IKA_HandleDelete ( DWORD dwHandle );

    long IKA_HandleOpen ( DWORD * pdwTotalNeedsBytesCount, void * pAllocatedMemory );
    long IKA_HandleClose ( void );
    long IKA_HandleDataSet ( DWORD dwHandle, DWORD dwDescription, DWORD dwParamA, DWORD dwParamB );
    long IKA_HandleDataGet ( DWORD dwHandle, DWORD *pdwDescription, DWORD *pdwParamA, DWORD *pdwParamB );
    long IKA_HandleFreeProcSet ( DWORD dwHandle, void (*pFreeProc) (DWORD dwHandle) );
    long IKA_HandleRawAlloc ( DWORD dwHandle, DWORD dwBytesSize );
    long IKA_HandleRawFree ( DWORD dwHandle );
    long IKA_HandleRawGet ( DWORD dwHandle, void **pRaw );
    
    //#################################################################################################################################    
    //Le gestionnaire de sortie audio
    long SAM_OutputGetAverageDeltaRW ( void );
    long SAM_ThreadProc_Output ( void * pProcData );
    