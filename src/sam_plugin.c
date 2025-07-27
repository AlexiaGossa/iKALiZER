#include "sam_header.h"

typedef struct {
    float   *pfAudioDataIn;
    float   *pfAudioDataOut;
    DWORD   dwChannelsCount;
    DWORD   dwSamplesCount;
    DWORD   dwSampleRate;
} IKA_PLUGIN_AUDIO_COMM;

typedef struct {
    char    szName[64];
    char    szAuthor[64];
    char    szCopyrights[64];
    char    szAddInfo[64];
    
    DWORD   dwSupportSampleRateMin;
    DWORD   dwSupportSampleRateMax;
    DWORD   dwSupportChannelsMin;
    DWORD   dwSupportChannelsMax;
} IKA_PLUGIN_INFO;

    

typedef struct {
    DWORD   *pdwParamData;
    DWORD   dwParamCount;
    
    


} IKA_PLUGIN_PARAMS;

typedef struct {
    IKA_PLUGIN_INFO         ikaPlugInInformation;
//    long                    (*Open) ( IKA_PLUGIN * pikaPlugIn );

} IKA_PLUGIN;

/*

    Fonctionne des plug-ins
    
    Le moteur de plug-ins

*/