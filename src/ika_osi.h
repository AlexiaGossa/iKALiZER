#include "ika_types.h"
#include <malloc.h>
#include <string.h>
#include <math.h>




/*
#ifdef SAMLIB_DLL_EXPORTS
    #ifndef DLLEXPORT
        #define DLLEXPORT   __declspec(dllexport)
    #endif
#else
    #define DLLEXPORT
#endif

#define _SAM_VERSIONH_DATA      (0)
#define _SAM_VERSIONM_DATA      (22)
#define _SAM_VERSIONL_DATA      (1)
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

*/

/*  ################################################################################################################################################
    
    OSIndependant
    - CriticalSection
    - Event
    - Thread
    - Sleep

*/

#   ifndef _IKA_OSI_H_
#       define _IKA_OSI_H_


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
            #ifndef THREAD_BASE_PRIORITY_LOWRT
                #define THREAD_BASE_PRIORITY_LOWRT  15  // value that gets a thread to LowRealtime-1
                #define THREAD_BASE_PRIORITY_MAX    2   // maximum thread base priority boost
                #define THREAD_BASE_PRIORITY_MIN    -2  // minimum thread base priority boost
                #define THREAD_BASE_PRIORITY_IDLE   -15 // value that gets a thread to idle
            #endif

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

#   endif
/*
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
*/    