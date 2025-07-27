//#define sam_OUTPUTFILE

#ifdef _WIN32
#define _CRT_SECURE_NO_WARNINGS
#pragma warning(disable : 4996)
#endif

#ifdef sam_OUTPUTFILE
#include <stdio.h>
#endif
    

    typedef struct {
        _OSI_CRITICAL_SECTION   osiCriticalSection;
        
        SAM_SFX_MUSIC           *pSFXMusic;
        DWORD                   dwSFXMusicCount;
        
        DWORD                   dwEntryPlayed;  //0xFFFFFFFF pour inactif
        
        sam_ALIGN float         interpolation_fStackValue[16*2];   //32 entrées (en fait 2x16 pour mono/stéréo)
        
        DWORD                   dwCurrentSampleRate;

        DWORD                   dwInPlayTickIncrement;          //Increment in Tick (8+24bits)
        DWORD                   dwInPlayTickPosition;           //Position in Tick (8+24bits)
        DWORD                   dwInPlayGranulePosition;        //Position dans le granule
        DWORD                   dwInPlaySamplePosition;         //Position dans l'échantillon
        DWORD                   dwInPlaySamplePositionPrevious; //Position dans l'échantillon précédent
        
        DWORD                   dwCodecData[16];                //16 x DWORD pour le codec exclusivement (128 octets)
        
    } SAM_MUSIC;
        
        
        
    

    typedef struct {
        _OSI_CRITICAL_SECTION   osiCriticalSection;

        SAM_STREAM      *psamStream;
        DWORD           dwStreamCount;
        DWORD           dwCurrentStream;

        //float                   resample_fFIRStackValue[sam_FIRLEN*2];   //32 entrées (en fait 2x16 pour mono/stéréo)
        //float                   *resample_pfFIRCoef;
        //long                    resample_lFIRStackPosition;
        //float                   resample_fLastValue[2];
        //float                   resample_fCurrValue[2];
        sam_ALIGN float         interpolation_fStackValue[16*2];   //32 entrées (en fait 2x16 pour mono/stéréo)
        DWORD                   dwCurrentSampleRate;

        DWORD                   dwInPlayTickIncrement;          //Increment in Tick (8+24bits)
        DWORD                   dwInPlayTickPosition;           //Position in Tick (8+24bits)
        DWORD                   dwInPlayGranulePosition;        //Position dans le granule
        DWORD                   dwInPlaySamplePosition;         //Position dans l'échantillon
        DWORD                   dwInPlaySamplePositionPrevious; //Position dans l'échantillon précédent
        
        QWORD                   qwCurrentPositionID;
        
        BYTE                    bPlayState;         //0 = Stopped, 1 = play


        /*    
        float * pfStereoData;                       //Position des données
        DWORD   dwSamplesCount;                     //Taille totale des données
        DWORD   dwCursorRead;                       //Position du curseur de lecture
        DWORD   dwCursorWrite;                      //Position du curseur d'écriture
        DWORD   dwReadyDataCount;                   //Données disponible pour la lecture
        DWORD   dwFreeDataCount;                    //Données libres pouvant être écrites
        float   fVolume;                            //Volume

        float                   resample_fFIRStackValue[256];   //256 entrées (en fait 2x128 pour mono/stéréo)
        float                   *resample_pfFIRCoef;
        long                    resample_lFIRStackPosition;
        long                    resample_lFIRLength;
        float                   resample_fLastValue[2];
        float                   resample_fCurrValue[2];
        */
    } SAM_STREAMING;


typedef struct {
    DWORD   dwSoundMegaBytes;                   //Megabytes for granules (used by sfx)
//    DWORD   dwHardwaremixChannelsCount;         //Channels count (default=2)
    DWORD   dwHardwaremixSampleRate;            //Sample rate (default=48000)
    DWORD   dwHardwareDeviceSelected;

    DWORD   dwHardwareBufferLatencySamplesCount;
    DWORD   dwHardwareAndSoftwareBufferSamplesCount;

    DWORD   dwFIRLength;                        //FIR quality 16 to 128 (default=32)
    
    DWORD   dwTotalVirtualVoicesCount;          //Total Voices in virtual mixer (always 1024)
    DWORD   dwTotalRealVoicesCount;             //Total Voices in real mixer (default=64)
    //DWORD   dwTotalVoicesCount;                 //Total Voices (default=64)
    DWORD   dwTotalSFXCount;                    //Total SFX could be loaded (default=4096)
    DWORD   dwDynamicDelayLinesCount;           //First step of the dynamic delay line (-1=auto, 0 = disable... 1...n * dwTotalVoicesCount)

    //DWORD   dwRenderChannelMode;

    DWORD   dwChannelMode;                      

    DWORD   dwOutputHardwareChannel;            //Nombre de canaux matériel en sortie
    DWORD   dwOutputSoftwareChannel;            //Nombre de canaux logiciel en sortie
    DWORD   dwOutputEncoder;                    //Encodeur à utiliser pour le mappage des canaux logiciel=>matériel
    char    szOutputName[256];

    sam_ALIGN float   fHilbertFIRCoef[512];               //2x128 point for the Hilbert FIR... ideal for +90/-90 phase shift


    float   *pfSoftwareBuffer;                  //Software buffer for mixing process

    float   *pfInterpolationTable512;           //Interpolation tables of 512 entry (2x256)
    float   *pfInterpolationTable512PackedBy2;  //Interpolation tables of 512 entry (256x2)
    
    _OSI_THREAD             osiThreadMixer;
    _OSI_CRITICAL_SECTION   osiCriticalSection;
    
    _OSI_CRITICAL_SECTION   osiCSOutputData;
    
    

    //float   fLimiterReduceMul[4][1024];         //Limiter with 4 levels : 0dB, +6dB, +12dB, +20dB
    //float   fLimiterSpeed[4];
    //float   fLimiterGainMax[4];
    //

    long    lLimiterMode;                       //0...3
    long    lTotalBufferUnderrunCount;          //Total buffer underrun count from start
    long    lActualLatencyInSamples;            //Actual latency in samples


    //float   fLimiterSpeedIncrease[4][1024];
    //float   fLimiterPeakValue;

    float   fLimiterCurrentGain;
    long    lLimiterConvertTodB_8_24[1024];
    long    lLimiterPeakLevel;
    long    lLimiterGain;
    long    lLimiterGainMax[4];
    long    lLimiterDelayBeforeGrowing;

    float   fLimiterGainTable[4096];

    SAM_RENDER254   *psamRender254Table;               //La table de rendu
//    SAM_RENDER      psamRenderTableSpecial[16];     //16 entrées dans la table de rendu "spéciale"
//    SAM_RENDER8     psamRenderTableSpecial[16];     //16 entrées dans la table de rendu "spéciale"
/*
    Liste des entrées spéciales pour le rendu
    0 = L'auditeur
    1 = Devant/haut-dessus
    2 = Derrière/haut-dessus
*/    
    

    DWORD   dwChannelIndexLeft;                 //Canal gauche (pour les échantillons stéréo)
    DWORD   dwChannelIndexRight;                //Canal droit (pour les échantillons stéréo)

    #ifdef sam_OUTPUTFILE
    FILE    *pFileDebug;
    #endif

    void    *pGlobalMemoryAllocated;
    DWORD   dwTotalMemoryNeeds;

    SAM_STREAMING   samStreaming;
    
    SAM_MUSIC       samMusic;
    
    INT16       *MixerOutput_pi16Buffer;     //Buffer de sortie stocké en mémoire (pour un repiquage avant envoi sur le périph de sortie)
    DWORD       MixerOutput_dwSamplesCount;
    DWORD       MixerOutput_dwMCSamplesCount;
    DWORD       MixerOutput_dwMCSamplesOffsetWrite; //Write est toujours en avance sur read
    DWORD       MixerOutput_dwMCSamplesOffsetRead;  //Read est toujours en retard sur write
    DWORD       MixerOutput_dwMCSamplesReadyToRead; //Nombre d'échantillons écrit, prêt à être lus
    
    float       fGainVoice;
    float       fGainMusic;
    float       fGainStreaming;
    
    //SMP for Window
    DWORD       hCallerProcess;
    DWORD       hCallerThread;
    
    DWORD       dwAffinityCaller;
    DWORD       dwAffinityLibrary;
    
    DWORD       dwForceAffinityMode;            //0 = keep system, 1 = dualcore, 2 = singlecore
    long        lInstancesCount;
    
    long        lProcessorEnableSSE;
    long        lProcessorEnableSSE2;
    long        lProcessorEnableSSE3;
    
    long        lWithoutStackMode;
    
    float       fCyclesPerSample;
    float       fCyclesPerSampleMirror;
    
    long        bInfiniteLoopWait;
    float       fMixerAverageUsagePercent;
    float       fMixerCurrentUsagePercent;
    
    float       fMaxProcessorUsagePercent;
    
    void        *pDeviceParam;
    
    long        bReInitMixer;                   //signal changeant
    long        bFreezeMixer;                   //Mixer thread : read-only, other threads read/write/modify
    long        bFrozenMixer;                   //Mixer thread : r/w/m, other thread read-only
    long        bQuakeRecordAudio;              //Only used if we want to record audio at a fixed FPS (like ioquake)
    long        lQuakeRecordAudioLen;           //Audio record len (for each mixer call)
    
    long        lTimerResolutionMin;            //Resolution mini du timer
    
    /*
        Fonctionnement de la surdite temporaire
        
        Pour chaque voix traitee, on va incremente un compteur...
        lorsque le son est trop fort, un deuxieme compteur est incremente (son taux d'incrementation depend du "sur-niveau")
        
        
        Toutes les 250 ms:
        - Si le taux depasse 80%, un indicateur de douleur est incremente (+1000)
        - on remet a zero ces 2 compteurs...
        
        Toutes les 20 ms :
        - L'indicateur de douleur diminu (-10)
        
        
        
        
        
        
        
    
    */
    /*
    DWORD       TemporaryDeafness_dwVoiceCount;
    DWORD       TemporaryDeafness_dwVoiceCountOverlevel;
    long        TemporaryDeafness_lEarPainCount;            //A partir de 20000, les filtres passe-bas s'activent !
    long        TemporaryDeafness_lEnable;                  //L'effet est actif
    */
    
    
    
    _OSI_THREAD             osiThreadOutput;
    
    /* 
    ===========================
    
        iKALiZER gen 2 !!!    
        
    ===========================
    */
    //ikAUDIOINTERFACE        aiMain;
    //long                    likChannelCountMode;

} SAM_DATA;

#define TEMPORARYDEAFNESS_MINPAIN       20000
#define TEMPORARYDEAFNESS_MAXPAIN       70000
#define TEMPORARYDEAFNESS_MAXRATIO      80
#define TEMPORARYDEAFNESS_PAIN_INCR     500//125
#define TEMPORARYDEAFNESS_PAIN_DECR     250






extern SAM_DATA     samData;
extern FLOAT32      samData_f32InterpolationData_1024_16[];