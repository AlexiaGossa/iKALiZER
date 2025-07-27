#include "sam_header.h"
#include "sam_data.h"

    // Windows Header Files:
    #include <windows.h>
    #include <windowsx.h>
    #include <winuser.h>
    #include <fcntl.h>

    // C RunTime Header Files
    #include <commctrl.h>
    #include <commdlg.h>
    #include <conio.h>
    #include <direct.h>
    #include <io.h>
    #include <malloc.h>
    #include <math.h>
    #include <memory.h>
    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #include <tchar.h>
    #include <time.h>
    
    #pragma comment(lib,"Winmm.lib")

//#include <windows.h>

//Header: Declared in Mmsystem.h; include Windows.h.
//  Library: Use Winmm.lib.

/* =  {   64,         //64Mo pour les granules
                         2,         //2 canaux (stéréo)
                         48000,     //48000 Hz
                         250,       //Longueur du tampon (en général 1/4s)
                         0,         //n/a
                         16,        //FIR order (32=default)
                         64,        //64 voices
                         4096,      //4096 SFX could be loaded
                         3 };       //Special Render mode
                   */
DLLEXPORT   long    SAM_Close ( void )
{
    //if (samData.dwForceAffinity&SAM_PROCESS_ENABLESMP)
    {
        _OSI_ThreadClose (
            &samData.osiThreadMixer,
            500 );

        _OSI_ThreadClose (
            &samData.osiThreadOutput,
            500 );

    }
    
    
    //Périphérique sonore de sortie
    //ikAudioInterfaceDeviceClose ( &samData.aiMain );
    SAM_DirectSound_Close ( );
    
    //Le timer
#ifdef _WIN32
    if (samData.lTimerResolutionMin)
        timeEndPeriod ( samData.lTimerResolutionMin );
#endif        
    SAM_TIMER_Close ( );


    //Libération de la mémoire
    SAM_KERNEL_FREE ( samData.pGlobalMemoryAllocated );

    #ifdef sam_OUTPUTFILE
    fclose ( samData.pFileDebug );
    #endif

    _OSI_DeleteCriticalSection ( &samData.osiCriticalSection );
    _OSI_DeleteCriticalSection ( &samData.osiCSOutputData );
    _OSI_DeleteCriticalSection ( &samData.samStreaming.osiCriticalSection );
    _OSI_DeleteCriticalSection ( &samData.samMusic.osiCriticalSection );

    /* 
    ===========================
    
        iKALiZER gen 2 !!!    
        
    ===========================
    */
    /*
    ikAudioInterfaceDelete ( &samData.aiMain );
    ikMessageManagerClose ( );
    ikInstanceManagerClose ( );
    ikHandleManagerClose ( );
    ikErrorManagerClose ( );
    */
    

    memset ( &samData, 0, sizeof(SAM_DATA) );
    
    return 0;
}

float hilbert_xcoeffs128[] =
  { +0.0012698413F, +0.0000000000, +0.0013489483F, +0.0000000000,
    +0.0015105196F, +0.0000000000, +0.0017620440F, +0.0000000000,
    +0.0021112899F, +0.0000000000, +0.0025663788F, +0.0000000000,
    +0.0031358856F, +0.0000000000, +0.0038289705F, +0.0000000000,
    +0.0046555545F, +0.0000000000, +0.0056265487F, +0.0000000000,
    +0.0067541562F, +0.0000000000, +0.0080522707F, +0.0000000000,
    +0.0095370033F, +0.0000000000, +0.0112273888F, +0.0000000000,
    +0.0131463382F, +0.0000000000, +0.0153219442F, +0.0000000000,
    +0.0177892941F, +0.0000000000, +0.0205930381F, +0.0000000000,
    +0.0237910974F, +0.0000000000, +0.0274601544F, +0.0000000000,
    +0.0317040029F, +0.0000000000, +0.0366666667F, +0.0000000000,
    +0.0425537942F, +0.0000000000, +0.0496691462F, +0.0000000000,
    +0.0584802574F, +0.0000000000, +0.0697446887F, +0.0000000000,
    +0.0847739823F, +0.0000000000, +0.1060495199F, +0.0000000000,
    +0.1388940865F, +0.0000000000, +0.1971551103F, +0.0000000000,
    +0.3316207267F, +0.0000000000, +0.9994281838F, +0.0000000000,
    -0.9994281838F, -0.0000000000, -0.3316207267F, -0.0000000000,
    -0.1971551103F, -0.0000000000, -0.1388940865F, -0.0000000000,
    -0.1060495199F, -0.0000000000, -0.0847739823F, -0.0000000000,
    -0.0697446887F, -0.0000000000, -0.0584802574F, -0.0000000000,
    -0.0496691462F, -0.0000000000, -0.0425537942F, -0.0000000000,
    -0.0366666667F, -0.0000000000, -0.0317040029F, -0.0000000000,
    -0.0274601544F, -0.0000000000, -0.0237910974F, -0.0000000000,
    -0.0205930381F, -0.0000000000, -0.0177892941F, -0.0000000000,
    -0.0153219442F, -0.0000000000, -0.0131463382F, -0.0000000000,
    -0.0112273888F, -0.0000000000, -0.0095370033F, -0.0000000000,
    -0.0080522707F, -0.0000000000, -0.0067541562F, -0.0000000000,
    -0.0056265487F, -0.0000000000, -0.0046555545F, -0.0000000000,
    -0.0038289705F, -0.0000000000, -0.0031358856F, -0.0000000000,
    -0.0025663788F, -0.0000000000, -0.0021112899F, -0.0000000000,
    -0.0017620440F, -0.0000000000, -0.0015105196F, -0.0000000000,
    -0.0013489483F, -0.0000000000, -0.0012698413F, -0.0000000000 };

void dwAlignOn16 ( DWORD *pdwOffset )
{
    DWORD dwOffset;
    
    dwOffset = *pdwOffset;
    
    if (dwOffset&0x0F) dwOffset = (dwOffset&0xFFFFFFF0)+16;
    *pdwOffset = dwOffset;
}                 


DLLEXPORT   long    SAM_Open ( void * pDeviceParam, SAM_CONFIG * psamConfig )
{
    float   fTemp, fSum;
    long    lInterpolationLenght;
    long    lError;
    long    i;
    DWORD   dwTempA, dwTempB;

    DWORD   dwGranulesMemoryNeeds;
    DWORD   dwSFXMemoryNeeds;
    DWORD   dwVoicesMemoryNeeds;
    DWORD   dwRenderMemoryNeeds;
    DWORD   dwInterpolationMemoryNeeds;
    DWORD   dwFIRMemoryNeeds;
    DWORD   dwSoftwareBufferMemoryNeeds;
    DWORD   dwStreamMemoryNeeds;
    DWORD   dwMusicMemoryNeeds;
    DWORD   dwInterpolatorsMemoryNeeds;
    DWORD   dwMixerOutputMemoryNeeds;
    BYTE    *pGlobalMemoryAllocated;
    BYTE    *pGranulesAllocatedMemory;
    BYTE    *pSFXAllocatedMemory;
    BYTE    *pVoiceAllocatedMemory;
    BYTE    *pRenderAllocatedMemory;
    BYTE    *pInterpolationAllocatedMemory;
    BYTE    *pFIRAllocatedMemory;
    BYTE    *pSoftwareBufferAllocatedMemory;
    BYTE    *pStreamAllocatedMemory;
    BYTE    *pMusicAllocatedMemory;
    BYTE    *pInterpolatorsAllocatedMemory;
    BYTE    *pMixerOutputAllocatedMemory;

    float   fDistanceWithinSpeakers;
    void    (*pSAM_RENDER_InitProc) ( float fDistanceWithinSpeakers, float fSampleRate, SAM_RENDER254 * psamRender );
    
    //Remplissage et initialisation de la structure samData
    memset ( &samData, 0, sizeof(SAM_DATA) );
    
    //Détection du processeur...
    CPUID_Get ( CPUID_GET_SSE, &samData.lProcessorEnableSSE );
    CPUID_Get ( CPUID_GET_SSE2, &samData.lProcessorEnableSSE2 );
    CPUID_Get ( CPUID_GET_SSE3, &samData.lProcessorEnableSSE3 );
    if (!samData.lProcessorEnableSSE)
        return -2;

    _OSI_InitializeCriticalSectionAndSpinCount ( &samData.osiCriticalSection, 0x80001000 );
    _OSI_InitializeCriticalSectionAndSpinCount ( &samData.osiCSOutputData,    0x80001000 );

    #ifdef sam_OUTPUTFILE
    samData.pFileDebug = fopen ( "C:\\sam_outfile_st16.pcm", "wb" );
    #endif
    
    
    /* 
    ===========================
    
        iKALiZER gen 2 !!!    
        
    ===========================
    */
    /*
    ikErrorManagerOpen ( );
    ikHandleManagerOpen ( 262144 );
    ikInstanceManagerOpen ( );
    ikMessageManagerOpen ( );
    ikAudioInterfaceCreate ( &samData.aiMain, ikAUDIOINTERFACE_DIRECTSOUND_PLAY );
    ikAudioInterfaceEnumDo ( &samData.aiMain );
    */
    
    

    /*DWORD       dwTotalMemorySoundAllocCount;               //Mémoire totale pour les sons de SAM (en mega octets)
    DWORD       dwHardwareSamplingRate;                     //Fréquence d'échantillonnage du matériel (en Hz) - défaut:48000Hz
    DWORD       dwHardwareChannelsMode;                     //Mode de sortie audio canaux/rendu
    DWORD       dwHardwareBufferLatencyDuration;            //Latence du buffer matériel (en ms) - défaut:20ms
    DWORD       dwSoftwareBufferDuration;                   //Taille du buffer logiciel (en ms) - défaut:250ms
    DWORD       dwSoftwareVoicesCount;                      //Nombre de voies logicielles pouvant être lues en simultanées - défaut:64
    DWORD       dwSoftwareSFXCount;                         //Nombre d'effets pouvant être chargés en mémoire - défaut:4096
    DWORD       dwSoftwareResamplingQuality;                //Qualité du ré-échantillonnage 0...3 - défaut:0
    */

    samData.dwHardwaremixSampleRate         = psamConfig->dwHardwareSamplingRate;
    samData.dwSoundMegaBytes                = psamConfig->dwTotalMemorySoundAllocCount;
    samData.dwTotalRealVoicesCount          = psamConfig->dwSoftwareVoicesCount;
    samData.dwTotalVirtualVoicesCount       = sam_VOICE_VIRTUALCOUNT;
    samData.dwTotalSFXCount                 = psamConfig->dwSoftwareSFXCount;
    samData.dwChannelMode                   = psamConfig->dwHardwareChannelsMode;
    samData.dwHardwareDeviceSelected        = psamConfig->dwHardwareDeviceSelected;
    samData.dwDynamicDelayLinesCount        = 0xFFFFFFFF;
    samData.fGainVoice                      = 1.0F;
    samData.fGainMusic                      = 1.0F;
    samData.fGainStreaming                  = 1.0F;
    
    //Voices checking
    if (samData.dwTotalRealVoicesCount>64) //sam_VOICE_MAXCOUNT)
        samData.dwTotalRealVoicesCount = 64; //sam_VOICE_MAXCOUNT;

    //Memory checking... 4 to 1024 MiB
    if (samData.dwSoundMegaBytes<4)
        samData.dwSoundMegaBytes = 4;
    if (samData.dwSoundMegaBytes>1024)
        samData.dwSoundMegaBytes = 1024;
    
    //###########################################################################################
    //
    //  Sample rate checking... 22050 to 48000 Hz    
    //
    switch (samData.dwHardwaremixSampleRate)
    {
        case 22050:
        case 24000:
        case 32000:
        case 44100:
        case 48000:
            break;
            
        default:
            samData.dwHardwaremixSampleRate = 24000;
            break;
    }

    //###########################################################################################
    //
    //  Le streaming (8ms par stream)
    //
    samData.samStreaming.dwStreamCount = (psamConfig->dwStreamingBufferDuration / 8 );
    if (samData.samStreaming.dwStreamCount>2048) samData.samStreaming.dwStreamCount = 2048;
    if (samData.samStreaming.dwStreamCount<  64) samData.samStreaming.dwStreamCount =   64;

    //###########################################################################################
    //
    //  Transformation de Hilbert
    //
    memset ( samData.fHilbertFIRCoef, 0, 512*sizeof(float) );
    fSum = 0;
    for (i=0;i<32;i++)
    {   
        fTemp = (float)i;
        fTemp = (float)(sam_PI * ( 2 * fTemp - 1 ));
        samData.fHilbertFIRCoef[64+(2*i-1)] = 2/fTemp;
        samData.fHilbertFIRCoef[64-(2*i-1)] =-2/fTemp;
    }

    for (i=0;i<128;i++)
        samData.fHilbertFIRCoef[i] = hilbert_xcoeffs128[i];

    memcpy ( samData.fHilbertFIRCoef+128, samData.fHilbertFIRCoef, 128*sizeof(float) );
    
    /*
    for (i=0;i<128;i++)
    {
        samData.fHilbertFIRCoef[i] = hilbert_xcoeffs128[i];
        samData.fHilbertFIRCoef[i+128] = hilbert_xcoeffs128[i];
    }
    */

    //###########################################################################################
    //
    //  Le mode de rendu
    //
    SAM_RENDER_InitChannelsModeValues ( 
        &fDistanceWithinSpeakers, 
        (void **)&pSAM_RENDER_InitProc );

    //###########################################################################################
    //
    //  Latence minimale = 5ms / maximale = 100 ms
    //
    dwTempA = psamConfig->dwHardwareBufferLatencyDuration;
    if (dwTempA<sam_LATENCY_DURATION_MIN) dwTempA = sam_LATENCY_DURATION_MIN;
    if (dwTempA>sam_LATENCY_DURATION_MAX) dwTempA = sam_LATENCY_DURATION_MAX;
    samData.dwHardwareBufferLatencySamplesCount = (samData.dwHardwaremixSampleRate * dwTempA)/1000;

    //###########################################################################################
    //
    //  Buffer de mixage matériel et logiciel (au moins 100ms de plus que la latence minimale et 500ms maxi)
    //
    dwTempB = psamConfig->dwSoftwareBufferDuration;
    if (dwTempB<(dwTempA+sam_BUFFER_DURATION_MIN)) dwTempB = dwTempA+sam_BUFFER_DURATION_MIN;
    if (dwTempB>sam_BUFFER_DURATION_MAX) dwTempB = sam_BUFFER_DURATION_MAX;
    samData.dwHardwareAndSoftwareBufferSamplesCount = (samData.dwHardwaremixSampleRate * dwTempB)/1000;

    //###########################################################################################
    //
    //  Tentative d'ouverture de DirectSound (il nous faut la taille du buffer utilisé dans DirectSound)
    //
    /*
    lError = ikAudioInterfaceDeviceOpen ( 
        &samData.aiMain, 
        samData.dwHardwareDeviceSelected, 
        samData.dwHardwaremixSampleRate, 
        samData.likChannelCountMode,
        1 );
    */
    
    lError = SAM_DirectSound_Open ( 
        samData.dwHardwareDeviceSelected,
        pDeviceParam,
        samData.dwHardwaremixSampleRate,
        samData.dwOutputHardwareChannel,
        samData.dwHardwareAndSoftwareBufferSamplesCount,
        &samData.dwHardwareAndSoftwareBufferSamplesCount );
    
    //Traîtement d'une éventuelle erreur
    if (lError<0) //!=ikAUDIOCONTROL_NOERROR)
    {   
        memset ( &samData, 0, sizeof(SAM_DATA) );
        return -1;
    }
    samData.pDeviceParam = pDeviceParam;
    
    //Récupération du périphérique sélectionné
    samData.dwHardwareDeviceSelected = (DWORD)lError;

    //Espace nécessaire pour les granules
    dwGranulesMemoryNeeds = (samData.dwSoundMegaBytes)<<20;

    //Espace nécessaire pour les SFX
    SAM_SFX_PreOpenMemoryNeeds ( 
        psamConfig->dwSoftwareSFXCount, 
        &dwSFXMemoryNeeds );

    //Espace nécessaire pour les VOICE
    SAM_VOICE_PreOpenMemoryNeeds (
        samData.dwTotalVirtualVoicesCount,
        &dwVoicesMemoryNeeds );

    //Espace nécessaire pour le RENDER (300Ko)
    dwRenderMemoryNeeds     = 64 * sizeof(SAM_RENDER254) * 2;
    
    //Espace nécessaire pour la table d'interpolation (2*2Ko)
    lInterpolationLenght = 256;
    dwInterpolationMemoryNeeds = 2 * lInterpolationLenght * 2 * sizeof(float);

    //Espace nécessaire pour les FIR (256Ko)
    dwFIRMemoryNeeds = (sam_FIRLEN*16) * 256 * sizeof(float);
    
    //Espace nécessaire pour le SOFTWARE_BUFFER (toujours 8 canaux à 48KHz)
    dwSoftwareBufferMemoryNeeds = 
        sizeof(float) * 8 * ( ( sam_SAMPLINGRATE_MAX * sam_BUFFER_DURATION_MAX ) / 1000 );
/*        sizeof(float) * 
        ((samData.dwOutputSoftwareChannel<8)?(8):(samData.dwOutputSoftwareChannel)) * 
        samData.dwHardwareAndSoftwareBufferSamplesCount;*/

    //Espace nécessaire pour le STREAM
    dwStreamMemoryNeeds = sizeof(SAM_STREAM) * samData.samStreaming.dwStreamCount;
    
    //Espace nécessaire pour MUSIC
    samData.samMusic.dwSFXMusicCount = 256;
    dwMusicMemoryNeeds = sizeof(SAM_SFX_MUSIC) * samData.samMusic.dwSFXMusicCount;
    
    //Espace nécessaire pour les interpolateurs
    dwInterpolatorsMemoryNeeds = ( 16 * 1024 ) * sizeof(float);
    
    //Espace nécessaire pour le buffer "pré-out"
    dwMixerOutputMemoryNeeds = 65536 * sizeof(INT16);

    //Alignement de l'allocation générale
    dwAlignOn16 ( &dwGranulesMemoryNeeds );
    dwAlignOn16 ( &dwSFXMemoryNeeds );
    dwAlignOn16 ( &dwVoicesMemoryNeeds );
    dwAlignOn16 ( &dwRenderMemoryNeeds );
    dwAlignOn16 ( &dwInterpolationMemoryNeeds );
    dwAlignOn16 ( &dwFIRMemoryNeeds );
    dwAlignOn16 ( &dwSoftwareBufferMemoryNeeds );
    dwAlignOn16 ( &dwStreamMemoryNeeds );
    dwAlignOn16 ( &dwMusicMemoryNeeds );
    dwAlignOn16 ( &dwInterpolatorsMemoryNeeds );
    dwAlignOn16 ( &dwMixerOutputMemoryNeeds );

    //Allocation générale !
    samData.dwTotalMemoryNeeds = 
        dwGranulesMemoryNeeds +
        dwSFXMemoryNeeds +
        dwVoicesMemoryNeeds +
        dwRenderMemoryNeeds +
        dwInterpolationMemoryNeeds +
        dwFIRMemoryNeeds +
        dwSoftwareBufferMemoryNeeds +
        dwStreamMemoryNeeds +
        dwMusicMemoryNeeds +
        dwInterpolatorsMemoryNeeds +
        dwMixerOutputMemoryNeeds;


    pGlobalMemoryAllocated          = (BYTE *)SAM_KERNEL_ALLOC ( samData.dwTotalMemoryNeeds );
    memset ( pGlobalMemoryAllocated, 0, samData.dwTotalMemoryNeeds );

    //Distribution des zones allouées
    samData.pGlobalMemoryAllocated  = (void *)pGlobalMemoryAllocated;
    pGranulesAllocatedMemory        = pGlobalMemoryAllocated;
    pSFXAllocatedMemory             = pGranulesAllocatedMemory          +dwGranulesMemoryNeeds;
    pVoiceAllocatedMemory           = pSFXAllocatedMemory               +dwSFXMemoryNeeds;
    pRenderAllocatedMemory          = pVoiceAllocatedMemory             +dwVoicesMemoryNeeds;
    pInterpolationAllocatedMemory   = pRenderAllocatedMemory            +dwRenderMemoryNeeds;
    pFIRAllocatedMemory             = pInterpolationAllocatedMemory     +dwInterpolationMemoryNeeds;
    pSoftwareBufferAllocatedMemory  = pFIRAllocatedMemory               +dwFIRMemoryNeeds;
    pStreamAllocatedMemory          = pSoftwareBufferAllocatedMemory    +dwSoftwareBufferMemoryNeeds;
    pMusicAllocatedMemory           = pStreamAllocatedMemory            +dwStreamMemoryNeeds;
    pInterpolatorsAllocatedMemory   = pMusicAllocatedMemory             +dwMusicMemoryNeeds;
    pMixerOutputAllocatedMemory     = pInterpolatorsAllocatedMemory     +dwInterpolatorsMemoryNeeds;
    
    //Le timer
    SAM_TIMER_Open ( );
    samData.lTimerResolutionMin = 0;
#ifdef _WIN32
    {
        TIMECAPS TimeCaps;
        
        if (timeGetDevCaps(&TimeCaps,sizeof(TimeCaps))==TIMERR_NOERROR)
        {
            samData.lTimerResolutionMin = TimeCaps.wPeriodMin;
        }
        if (samData.lTimerResolutionMin)
            timeBeginPeriod ( samData.lTimerResolutionMin );
    }
#endif        
        
    
    //Granules MMU
    SAM_GranulesOpen ( dwGranulesMemoryNeeds, pGranulesAllocatedMemory );
    
    //XPCM - Initialise LUT
    SAM_XPCM_Init ( );

    //XD4 Initiatise LUT
    SAM_XD4_Init ( );
    SAM_XD4ADPCM_Init ( );
    SAM_LPCM_Init ( );

    //SFX
    SAM_SFX_Open ( samData.dwTotalSFXCount, pSFXAllocatedMemory );

    //Voice
    SAM_VOICE_Open ( samData.dwTotalVirtualVoicesCount, samData.dwTotalRealVoicesCount, pVoiceAllocatedMemory );

    //Render
    samData.psamRender254Table  = (SAM_RENDER254 *)pRenderAllocatedMemory;
    pSAM_RENDER_InitProc ( fDistanceWithinSpeakers, (float)samData.dwHardwaremixSampleRate, samData.psamRender254Table );
    for (i=0;i<64;i++)
    {
        //Maximisation
        SAM_RENDER_MaximizeLevel ( &samData.psamRender254Table[i], samData.dwOutputSoftwareChannel, samData.dwHardwaremixSampleRate );
    }
    samData.dwChannelIndexLeft  = 0;
    samData.dwChannelIndexRight = 1;
    
    //Génération des polynomes d'interpolation
    //samData.pfInterpolationFIR_1024_16 = (float *)pInterpolatorsAllocatedMemory;
    SAM_INTERPOLATOR_Init ( );

    //Stream
    samData.samStreaming.psamStream             = (SAM_STREAM *)pStreamAllocatedMemory;
    _OSI_InitializeCriticalSectionAndSpinCount ( &samData.samStreaming.osiCriticalSection, 0x80001000 );
    
    //MUSIC
    samData.samMusic.pSFXMusic              = (SAM_SFX_MUSIC *)pMusicAllocatedMemory;
    samData.samMusic.dwEntryPlayed          = 0xFFFFFFFF;
    _OSI_InitializeCriticalSectionAndSpinCount ( &samData.samMusic.osiCriticalSection, 0x80001000 );

    //Tampon logiciel
    samData.pfSoftwareBuffer = (float *)pSoftwareBufferAllocatedMemory;
    
    //Le pré-out
    samData.MixerOutput_pi16Buffer              = (INT16 *)pMixerOutputAllocatedMemory;
    samData.MixerOutput_dwSamplesCount          = dwMixerOutputMemoryNeeds / sizeof(INT16);
    samData.MixerOutput_dwMCSamplesCount        = 0;
    samData.MixerOutput_dwMCSamplesOffsetRead   = 0;
    samData.MixerOutput_dwMCSamplesOffsetWrite  = 0;
    samData.MixerOutput_dwMCSamplesReadyToRead  = 0;

    //Post process
    samData.lLimiterMode = 0;
    
    //La gestion de la douleur auditive (Pain Hearing)
/*    
    samData.TemporaryDeafness_dwVoiceCount = 0;
    samData.TemporaryDeafness_dwVoiceCountOverlevel = 0;
    samData.TemporaryDeafness_lEarPainCount = 0;
    samData.TemporaryDeafness_lEnable = 1;
*/
    //***********************************************************************************
    // Gestion de l'affinité et ça marche !!! Même TaskInfo est dans les choux !
    samData.hCallerProcess      = (DWORD)GetCurrentProcess();
    samData.hCallerThread       = (DWORD)GetCurrentThread();
    samData.dwForceAffinityMode = (psamConfig->dwProcessFlag)&SAM_PROCESS_ENABLESMP;
    
    //Détection du nombre de processeurs réels
    {
        DWORD dwCurrentProcessAffinity;
        DWORD dwCurrentSystemAffinity;
        DWORD dwSetAffinity;
        DWORD dwGetAffinity;
        long  lInstancesCount;
        
        //Affinité courante...
        GetProcessAffinityMask ( 
            (HANDLE)samData.hCallerProcess, 
            &dwCurrentProcessAffinity, 
            &dwCurrentSystemAffinity );
            
        lInstancesCount = 0;            
        for (i=0;i<32;i++)
        {
            dwSetAffinity = 1<<i;
            SetProcessAffinityMask ( (HANDLE)samData.hCallerProcess, dwSetAffinity );
            GetProcessAffinityMask ( (HANDLE)samData.hCallerProcess, &dwGetAffinity, &dwTempA );
            if (dwSetAffinity==dwGetAffinity) lInstancesCount++;
        }
        
        SetProcessAffinityMask (
            (HANDLE)samData.hCallerProcess, 
            dwCurrentProcessAffinity );
            
        samData.lInstancesCount = lInstancesCount;
    }
    
    //Si on doit forcer l'affinité, il nous faut le nombre de processeurs réels !
    switch (samData.dwForceAffinityMode)
    {
        case 0:
            //Laisse le système gérer
            break;
            
        case 1:
            //Force le mode dualcore :
            // Si un seul processeur, passe au mode 2
            if (samData.lInstancesCount>1)
            {
                //ioQuake passe sur l'affinité #0
                samData.dwAffinityCaller = 1;
                
                //SAM passe sur l'affinité #1 !
                samData.dwAffinityLibrary = 2; //dwTempA;
                
                break;
            }
            //Ne pas ajouter de break, car si un seul processeur, on passe au mode 2
        case 2:
            samData.dwForceAffinityMode = 2;
            
            //ioQuake passe sur l'affinité #0
            samData.dwAffinityCaller = 1;
            
            //SAM passe sur l'affinité #0 !
            samData.dwAffinityLibrary = 1; //dwTempA;
            break;
    }
            
    /*        
    if (samData.dwForceAffinity&SAM_PROCESS_ENABLESMP)
    {
        if (samData.lInstancesCount<=1)
            samData.dwForceAffinity = 0;
        else
        {
            if (samData.lInstancesCount<32) dwTempA = (1<<samData.lInstancesCount)-1;
            else                            dwTempA = 0xFFFFFFFF;
        
            //ioQuake passe sur l'affinité #0
            samData.dwAffinityCaller = 1;
            
            //SAM passe sur l'affinité #1 !
            samData.dwAffinityLibrary = 2; //dwTempA;
        }
    }
    */
    
    SAM_ProcessProc_Mixer ( 0, NULL );
    
    //if (samData.dwForceAffinity&SAM_PROCESS_ENABLESMP)
    {    
        _OSI_ThreadOpen ( 
            &samData.osiThreadMixer,
            SAM_ThreadProc_Mixer,
            0,
            NULL );

        _OSI_ThreadSetPriority ( 
            &samData.osiThreadMixer,
            _OSI_THREAD_PRIORITY_TIME_CRITICAL );

        /*_OSI_ThreadOpen ( 
            &samData.osiThreadOutput,
            SAM_ThreadProc_Output,
            0,
            NULL );
        */
            
        _OSI_ThreadSetPriority ( 
            &samData.osiThreadOutput,
            _OSI_THREAD_PRIORITY_TIME_CRITICAL );
        
            
        SAM_ForceAffinity ( );
    }    

    
    
    return 0;
}

void SAM_ForceAffinity ( void )
{
    if (samData.dwForceAffinityMode)
    {
        //Affinité du process
        SetProcessAffinityMask ( 
            (HANDLE)samData.hCallerProcess,
            samData.dwAffinityCaller | samData.dwAffinityLibrary );
            
        //Affinité du thread appelant
        SetThreadAffinityMask (
            (HANDLE)samData.hCallerThread,
            samData.dwAffinityCaller );
            
        //Affinité de la thread de SAM
        _OSI_ThreadSetAffinity ( 
            &samData.osiThreadMixer,
            samData.dwAffinityLibrary );
    }
}

long    SAM_LimiterSet ( long lMode )
{
    _OSI_EnterCriticalSection ( &samData.osiCriticalSection );

    switch (lMode)
    {
        case 0:
        default:
            samData.lLimiterMode = 0;
            break;

        case 1:
            samData.lLimiterMode = 1;
            break;

        case 2:
            samData.lLimiterMode = 2;
            break;

        case 3:
            samData.lLimiterMode = 3;
            break;
    }

    _OSI_LeaveCriticalSection ( &samData.osiCriticalSection );

    return 0;
}

/*
long    SAM_InternalTimerGet ( DWORD * pdwInternalTimer )
{
    DWORD dwInternalTimer;

    if (!pdwInternalTimer)
        return -1;

    _OSI_EnterCriticalSection ( &samData.osiCriticalSection );
    dwInternalTimer = samData.dwProtectedInternalTimer;
    _OSI_LeaveCriticalSection ( &samData.osiCriticalSection );
    *pdwInternalTimer = dwInternalTimer;

    return 0;
}
*/


DLLEXPORT   long    SAM_GainSet ( BYTE bSetGainMaskVMS, float fGainVoice, float fGainMusic, float fGainStreaming )
{
    if (bSetGainMaskVMS&1) samData.fGainStreaming   = fGainStreaming;
    if (bSetGainMaskVMS&2) samData.fGainMusic       = fGainMusic;
    if (bSetGainMaskVMS&4) samData.fGainVoice       = fGainVoice;
    return 0;
}




void SAM_DEBUG_TEXT_ ( char * pszText, ... )
{
    char        szTmp[1024];
    va_list     vaList;

    if (!pszText) return;

    //Conversion...
    va_start ( vaList, pszText );
    _vsnprintf ( szTmp, 1023, pszText, vaList );
    strcat ( szTmp, "\n" );
    OutputDebugString ( szTmp );
}
