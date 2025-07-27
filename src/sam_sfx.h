
typedef struct {
    SAM_SFX                 *psamSFX;
    DWORD                   dwSFXCount;
    _OSI_CRITICAL_SECTION   osiCS;
} SAM_SFX_DATA;


extern SAM_SFX_DATA samSfxData;