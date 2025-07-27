#define sam_VOICEBUFFERSTACK_COUNT  (1024)
#define sam_VOICEBUFFERSTACK_MASK   (sam_VOICEBUFFERSTACK_COUNT-1)

#define sam_VOICEUSERDATACOUNT      256

#define sam_RENDERFREQMIN (30)

/*
sam_ALIGN_SPEC(32) typedef struct {
    sam_ALIGN       float                   interpolation_fStackValue[64][16]; //64 piles de 16 entrées
                    float                   

sam_ALIGN_SPEC(32)  typedef struct {

    sam_ALIGN       float                   interpolation_fStackValue[16];      //16 entrées pour l'interpolation
    
                    float                   fMulLevel;                          //Application du niveau
                    float                   fDataPostLevel[;                     //Le signal après amplification
                    

} IKA_VOICE_SOURCE;

/*
typedef struct {
                    long                    lSourceAudioMixID;
                    long                    lTargetAudioMixID;
                    
                    float                   fParamList[16];
                    float                   fValueList[32];
                    float                   fParamC;
                    float                   fParamD;


/*AudioMixID

    bit 30-31       0 = VoiceGetSample (IKA_VOICE_SOURCE)
                    1 = ProcessEffect (IKA_VOICE_EFFECT)
                    2 = VoiceSampleToRender (IKA_VOICE_RENDER)


*/

sam_ALIGN_SPEC(32)   typedef struct {

                    //Place cette pile au début pour profiter de l'alignement : 32 x 4 = 128 octets
    sam_ALIGN       float                   interpolation_fStackValue[32];   //32 entrées (en fait 2x16 pour mono/stéréo) (en réalité 40 entrées pour faire un alignement 16 octets)

                    //La pile de pré-sortie : (1024+1) * 4 = 4100 octets
    sam_ALIGN       float                   stack_fBufferStackValue[sam_VOICEBUFFERSTACK_COUNT];    //4096 points soit 85ms @ 48KHz
                    DWORD                   stack_dwBufferStackPosition;

                    //General info : 5 x 4 octets = 20 octets
                    long                    bIsUsed;
                    DWORD                   dwHandleSFX;
                    DWORD                   dwCurrentGranuleID;
                    long                    bLoopMode;
                    long                    bVariableRate;                      //La vitesse de lecture est variable (doppler...), le mixage doit le savoir !
                    long                    bIsPlay;

                    //For mixing only : 6 x 4 octets = 24 octets
                    float                   fLevelMasterGain;
                    float                   fLevelSFXReplayGain;
                    float                   fLevelCinemaAGC;                //-10 to +3dB (in mul type : *0.316...*1.414)
                    float                   fDistanceMeters;
                    long                    lAngleDegrees;
                    float                   fOutputGainMul;

                    //Sound Effect for mono only : 3 x 4 octets = 12 octets
                    float                   fFX_LevelDistanceGain;
                    float                   fFX_IIR_LowPassRatio;
                    float                   fFX_IIR_LowPassValue;

                    //For sample processing : 6 x 4 octets = 24 octets
                    DWORD                   dwInPlayTickIncrement;          //Increment in Tick (8+24bits)
                    DWORD                   dwInPlayTickPosition;           //Position in Tick (8+24bits)
                    DWORD                   dwInPlayGranulePosition;        //Position dans le granule
                    DWORD                   dwInPlaySamplePosition;         //Position dans l'échantillon
                    DWORD                   dwInPlaySamplePositionPrevious; //Position dans l'échantillon précédent
                    DWORD                   dwInPlaySampleRate;             //Echantillonnage

                    SAM_RENDER254           *psamRender254;

                    long                    lRenderPositionCurrent_F16;     //0...63 shl 16
                    long                    lRenderIncrement_F16;           //xxxxxx shl 16
                    long                    lRenderPositionTarget;
                    
                    //For direct 2ch output (only if psamRender254 is NULL !!!)
                    float                   fLevelGainL;                    //-1...0...+1
                    float                   fLevelGainR;                    //-1...0...+1
                    long                    lLevelGainEnable;               //0 or 1 (enable 2ch direct output rendering)
                    
                    float                   fSpeedShift;                    // SpeedShift = 0<x<256
                    DWORD                   dwInPlayTickIncrementBackup;    // Un backup bien utile !


    //Le resampling
    //float                   resample_fFIRStackValue[(sam_FIRLEN*2)+4];   //32 entrées (en fait 2x16 pour mono/stéréo) + 4 entrées pour l'alignement sur 16
    //float                   *resample_pfFIRCoef;
    //long                    resample_lFIRStackPosition;
    //long                    resample_lFIRLength;
    //float                   resample_fLastValue[2];
    //float                   resample_fCurrValue[2];
    /*
    float                   reverb_fStackLate[2][8192];
    float                   reverb_fStackEarly[2][2048];
    long                    reverb_lStackAPosition;
    long                    reverb_lStackBPosition;
    float                   reverb_fReinjectStack[2];
    float                   reverb_fCombStackA[2][32];
    long                    reverb_lCombStackAPosition;
    float                   reverb_fCombStackB[2][32];
    long                    reverb_lCombStackBPosition;
    
    float                   reverb_fLowPassValue[2];
    float                   reverb_fHighPassValue[2];
    */

    //long                    lRenderIndexCurrent;            //0...63
    //long                    lRenderIndexTarget;             //0...63
} SAM_VOICE;

typedef struct 
{
    //Données non-bloquantes
    DWORD                   dwLastTimeUpdated;
    DWORD                   dwUserID;
    BYTE                    bUserData[sam_VOICEUSERDATACOUNT];                 //sam_VOICEUSERDATACOUNT x BYTE pour l'utilisateur

    //Données bloquantes
    DWORD                   dwUpdateFlag;

    long                    bIsUsed;
    long                    bIsPlay;
    long                    bIsPlayedState;
    long                    bLoopMode;
    long                    bVariableRate;                      //La vitesse de lecture est variable (doppler...), le mixage doit le savoir !
    
    DWORD                   dwInPlaySamplePosition;             //Position dans l'échantillon
    DWORD                   dwInPlayTickPosition;               //Position in Tick (8+24bits)

    float                   fLevelMasterGain;
    float                   fDistanceMeters;
    long                    lAngleDegrees;

    float                   fFX_LevelDistanceGain;
    float                   fFX_IIR_LowPassRatio;

    //Spécification du SFX à utiliser
    DWORD                   dwHandleSFX;                        //Flag = UPDATE_SFX

    //Pour le contrôle de la vitesse de la lecture
    DWORD                   dwInPlayTickIncrement;              //Flag = UPDATE_PLAYRATE
    DWORD                   dwInPlaySampleRate;

    SAM_RENDER254           *psamRender254;                        //Flag = UPDATE_RENDER
    
    long                    lRenderIndexTarget;                 //0...63
    DWORD                   dwTimeRenderUpdatedPrevious;
    DWORD                   dwTimeRenderUpdatedCurrent;
    long                    lRenderOrderChangeFlag;             //Flag = 1 si on vient de changer le contenu
    
    //For direct 2ch output (only if psamRender254 is NULL !!!)
    float                   fLevelGainL;                    //-1...0...+1
    float                   fLevelGainR;                    //-1...0...+1
    long                    lLevelGainEnable;               //0 or 1
    
    float                   fSpeedShift;                    // SpeedShift = 0<x<256
    
    
    
    //SAM_RENDER              samRenderDLFprocessor;
    
} SAM_VOICE_MIRROR;


typedef struct {
    SAM_VOICE               *psamVoice;
    SAM_VOICE_MIRROR        *psamVoiceMirror;
    DWORD                   dwVoiceVirtualCount;    
    DWORD                   dwVoiceRealCountCurrent;
    DWORD                   dwVoiceRealCountInitial;
    _OSI_CRITICAL_SECTION   osiCSMirror;
    _OSI_CRITICAL_SECTION   osiCSFreezeCopyMirror;

    DWORD                   dwFreezeCopyMirror;

} SAM_VOICE_DATA;

extern SAM_VOICE_DATA samVoiceData;

#define UPDATE_SFX                  0x0001                  //Il faut remettre à zéro la position et plein d'autres données !!!
#define UPDATE_PLAYRATE             0x0002
#define UPDATE_PLAY                 0x0004
#define UPDATE_UNUSED               0x0008
#define UPDATE_POSITION             0x0010
#define UPDATE_POSITIONTICK         0x0020
#define UPDATE_RESET                0x1000



void SAM_VOICE_MixSingleGranule_Null ( SAM_VOICE * psamVoice, SAM_SFX * psamSFX, float fGlobalGain, DWORD dwLoopCount, float * pfSoftwareBuffer, DWORD dwSoftwareBufferChannelCount );
void SAM_VOICE_MixSingleGranule_C ( SAM_VOICE * psamVoice, SAM_SFX * psamSFX, float fGlobalGain, DWORD dwLoopCount, float * pfSoftwareBuffer, DWORD dwSoftwareBufferChannelCount );

//Mixage des effets sonore en mode direct stéréo output
//Fonctionne avec tous les modes
//Anciennement : SAM_VOICE_MixSingleGranule_2ch_SSE
//Existe sans resampling
void SAM_VOICE_MixSingle_DirectStereo                           ( SAM_VOICE * psamVoice, SAM_SFX * psamSFX, float fGlobalGain, DWORD dwLoopCount, float * pfSoftwareBuffer, DWORD dwSoftwareBufferChannelCount );
void SAM_VOICE_MixSingle_DirectStereo_noresamp                  ( SAM_VOICE * psamVoice, SAM_SFX * psamSFX, float fGlobalGain, DWORD dwLoopCount, float * pfSoftwareBuffer, DWORD dwSoftwareBufferChannelCount );

//Mixage des effets sonore avec le render sur pile
//Ne fonctionne que pour les modes 2 canaux (utilisé dans les modes 0x21, 0x22 et 0x25)
//Anciennement : SAM_VOICE_MixSingleGranule_WithStackSSE
//Existe sans resampling
void SAM_VOICE_MixSingle_RenderStack                            ( SAM_VOICE * psamVoice, SAM_SFX * psamSFX, float fGlobalGain, DWORD dwLoopCount, float * pfSoftwareBuffer, DWORD dwSoftwareBufferChannelCount, DWORD dwParam );
void SAM_VOICE_MixSingle_RenderStack_noresamp                   ( SAM_VOICE * psamVoice, SAM_SFX * psamSFX, float fGlobalGain, DWORD dwLoopCount, float * pfSoftwareBuffer, DWORD dwSoftwareBufferChannelCount, DWORD dwParam );

//Mixage des effets sonore avec le render en mode 2 canaux
//Ne fonctionne que pour les modes 2 canaux (utilisé dans le mode 0x20)
//Anciennement : SAM_VOICE_MixSingleGranule_WithoutStackSSE_2ch
//Existe sans resampling
void SAM_VOICE_MixSingle_Render2ch                              ( SAM_VOICE * psamVoice, SAM_SFX * psamSFX, float fGlobalGain, DWORD dwLoopCount, float * pfSoftwareBuffer, DWORD dwSoftwareBufferChannelCount, DWORD dwParam );
void SAM_VOICE_MixSingle_Render2ch_noresamp                     ( SAM_VOICE * psamVoice, SAM_SFX * psamSFX, float fGlobalGain, DWORD dwLoopCount, float * pfSoftwareBuffer, DWORD dwSoftwareBufferChannelCount, DWORD dwParam );

//Mixage des effets sonore avec le render en mode 2 canaux
//Ne fonctionne que pour les modes 4 canaux (utilisé dans le mode 0x23, 0x40)
//Anciennement : SAM_VOICE_MixSingleGranule_WithoutStackSSE_2ch
//Existe sans resampling
void SAM_VOICE_MixSingle_Render4ch                              ( SAM_VOICE * psamVoice, SAM_SFX * psamSFX, float fGlobalGain, DWORD dwLoopCount, float * pfSoftwareBuffer, DWORD dwSoftwareBufferChannelCount, DWORD dwParam );
void SAM_VOICE_MixSingle_Render4ch_noresamp                     ( SAM_VOICE * psamVoice, SAM_SFX * psamSFX, float fGlobalGain, DWORD dwLoopCount, float * pfSoftwareBuffer, DWORD dwSoftwareBufferChannelCount, DWORD dwParam );

//Mixage des effets sonore avec le render en mode 6 canaux
//Ne fonctionne que pour les modes 6 canaux (utilisé dans le mode 0x24, 0x60, 0x61)
//Anciennement : SAM_VOICE_MixSingleGranule_WithoutStackSSE_8ch
//Existe sans resampling
void SAM_VOICE_MixSingle_Render6ch                              ( SAM_VOICE * psamVoice, SAM_SFX * psamSFX, float fGlobalGain, DWORD dwLoopCount, float * pfSoftwareBuffer, DWORD dwSoftwareBufferChannelCount, DWORD dwParam );
void SAM_VOICE_MixSingle_Render6ch_noresamp                     ( SAM_VOICE * psamVoice, SAM_SFX * psamSFX, float fGlobalGain, DWORD dwLoopCount, float * pfSoftwareBuffer, DWORD dwSoftwareBufferChannelCount, DWORD dwParam );

//Mixage des effets sonore avec le render en mode 8 canaux
//Ne fonctionne que pour les modes 8 canaux (utilisé dans le mode 0x26)
//Anciennement : SAM_VOICE_MixSingleGranule_WithoutStackSSE_8ch
//Existe sans resampling
void SAM_VOICE_MixSingle_Render8ch                              ( SAM_VOICE * psamVoice, SAM_SFX * psamSFX, float fGlobalGain, DWORD dwLoopCount, float * pfSoftwareBuffer, DWORD dwSoftwareBufferChannelCount, DWORD dwParam );
void SAM_VOICE_MixSingle_Render8ch_noresamp                     ( SAM_VOICE * psamVoice, SAM_SFX * psamSFX, float fGlobalGain, DWORD dwLoopCount, float * pfSoftwareBuffer, DWORD dwSoftwareBufferChannelCount, DWORD dwParam );



void SAM_VOICE_Mix ( float * pfSoftwareBuffer, DWORD dwSoftwareBufferChannelCount, DWORD dwSoftwareBufferSamplesToMixCount, DWORD dwInternalTimer );


long    _SAM_VOICE_GetPlayTick ( float fPlayRate, DWORD * pdwTickIncrement );


long    SAM_VOICE_LockEnter ( DWORD dwAccess );
long    SAM_VOICE_LockLeave ( DWORD dwAccess );

long    SAM_VOICE_MIRROR_FreezeEnable ( void );
long    SAM_VOICE_MIRROR_FreezeDisable ( void );





extern              SAM_GRANULESDATA    samGranulesData;
extern              FLOAT32             f32LPCM_Decode16;
extern  sam_ALIGN   DWORD               sam_voice_mixsingle_dwMaskPS_0111[4];
