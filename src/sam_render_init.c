#include "sam_render.h"

void SAM_RENDER_ValidateChannelsModeValues ( DWORD *pdwChannelMode )
{
    switch (*pdwChannelMode)
    {
        default:
            *pdwChannelMode = 0x20;
        case 0x20:  // 360° => 2.0 stéréo
        case 0x21:  // 360° => 2.0 Headphones : Hybrid HRTF
        case 0x25:  // 360° => 2.0 Headphones : Holographic
        case 0x26:  // 360° => 2.0 Headphones : Virtual Holographic
        case 0x22:  // 360° => 2.0 360 Virtual Sound
        case 0x23:  // 360° => L/R/C/S 4.0 => 2.0 Dolby Pro Logic 
        case 0x24:  // 360° => L/R/C/-/SL/SR 5.0 => 2.0 Dolby Pro Logic II
        case 0x40:  // 360° => L/R/SL/SR 4.0
        case 0x60: //5.1 - LFE off
        case 0x61: //5.1 - LFE on
            break;
    }
}

void SAM_RENDER_InitChannelsModeValues ( float * pfDistanceWithinSpeakers, void ** pRenderInitProc )
{
    float fDistanceWithinSpeakers;
    void    (*pSAM_RENDER_InitProc) ( float fDistanceWithinSpeakers, float fSampleRate, SAM_RENDER254 * psamRender );
    
    switch (samData.dwChannelMode)
    {
        default:
            samData.dwChannelMode           = 0x20;
        case 0x20:  // 360° => 2.0 stéréo
            samData.lWithoutStackMode       = 2;
            samData.dwOutputHardwareChannel = 2;
            samData.dwOutputSoftwareChannel = 2;
            samData.dwOutputEncoder         = 0;
            pSAM_RENDER_InitProc            = SAM_RENDER_Init_2CH_Stereo;
            fDistanceWithinSpeakers         = 0.80F;
            strcpy ( samData.szOutputName, "2.0 - Stereo" );
            //samData.likChannelCountMode     = ikCHANNELCOUNTMODE_2ch_STEREO;
            break;

        case 0x21:  // 360° => 2.0 Headphones : Hybrid HRTF
            samData.dwOutputHardwareChannel = 2;
            samData.dwOutputSoftwareChannel = 2;
            samData.dwOutputEncoder         = 0;
            pSAM_RENDER_InitProc            = SAM_RENDER_Init_2CH_Headphones_Hybrid_HRTF;
            fDistanceWithinSpeakers         = 0.15F;
            strcpy ( samData.szOutputName, "2.0 - Headphone (Hybrid HRTF)" );
            //samData.likChannelCountMode     = ikCHANNELCOUNTMODE_2ch_STEREO;
            break;
            
        case 0x25:  //360° => 2.0 Headphones : Holographic
            samData.dwOutputHardwareChannel = 2;
            samData.dwOutputSoftwareChannel = 2;
            samData.dwOutputEncoder         = 0;
            pSAM_RENDER_InitProc            = SAM_RENDER_Init_2CH_Headphones_Holographic;
            fDistanceWithinSpeakers         = 0.15F;
            strcpy ( samData.szOutputName, "2.0 - Headphone (Holographic)" );
            //samData.likChannelCountMode     = ikCHANNELCOUNTMODE_2ch_STEREO;
            break;
            
        case 0x26:  // 360° => 2.0 Headphones : Virtual Holographic
            samData.lWithoutStackMode       = 8;
            samData.dwOutputHardwareChannel = 2;
            samData.dwOutputSoftwareChannel = 8;
            samData.dwOutputEncoder         = 3; //Realtime Recoder : 6ch to 2ch
            pSAM_RENDER_InitProc            = SAM_RENDER_Init_8CH_0x26;
            fDistanceWithinSpeakers         = 0.15F;
            strcpy ( samData.szOutputName, "2.0 - Headphone (Virtual Holographic)" );
            //samData.likChannelCountMode     = ikCHANNELCOUNTMODE_2ch_STEREO;
            break;
            
        case 0x22:  // 360° => 2.0 360 Virtual Sound
            samData.dwOutputHardwareChannel = 2;
            samData.dwOutputSoftwareChannel = 2;
            samData.dwOutputEncoder         = 0;
            pSAM_RENDER_InitProc            = SAM_RENDER_Init_2CH_360VS;
            fDistanceWithinSpeakers         = 0.95F;//1.10F;
            strcpy ( samData.szOutputName, "2.0 - 360 VirtualSound" );
            //samData.likChannelCountMode     = ikCHANNELCOUNTMODE_2ch_STEREO;
            break;

        case 0x23:  // 360° => L/R/C/S 4.0 => 2.0 Dolby Pro Logic 
            samData.lWithoutStackMode       = 4;
            samData.dwOutputHardwareChannel = 2;
            samData.dwOutputSoftwareChannel = 4;            
            samData.dwOutputEncoder         = 1; //Dolby Pro Logic - RealTime Encoder (with Hilbert transform)
            pSAM_RENDER_InitProc            = SAM_RENDER_Init_4CH_L_R_C_S;
            fDistanceWithinSpeakers         = 1.10F;
            strcpy ( samData.szOutputName, "2.0 - Dolby Pro Logic" );
            //samData.likChannelCountMode     = ikCHANNELCOUNTMODE_2ch_STEREO;
            break;

        case 0x24:  // 360° => L/R/C/-/SL/SR 5.0 => 2.0 Dolby Pro Logic II
            samData.lWithoutStackMode       = 6;
            samData.dwOutputHardwareChannel = 2;
            samData.dwOutputSoftwareChannel = 6;
            samData.dwOutputEncoder         = 2; //Dolby Pro Logic II - RealTime Encoder (with Hilbert transform)
            pSAM_RENDER_InitProc            = SAM_RENDER_Init_6CH_L_R_C_noLFE_SL_SR;
            fDistanceWithinSpeakers         = 1.10F;
            strcpy ( samData.szOutputName, "2.0 - Dolby Pro Logic II" );
            //samData.likChannelCountMode     = ikCHANNELCOUNTMODE_2ch_STEREO;
            break;

        case 0x40: // 360° => L/R/SL/SR 4.0
            samData.lWithoutStackMode       = 4;
            samData.dwOutputHardwareChannel = 4;
            samData.dwOutputSoftwareChannel = 4;
            samData.dwOutputEncoder         = 0;
            pSAM_RENDER_InitProc            = SAM_RENDER_Init_4CH_L_R_SL_SR;
            fDistanceWithinSpeakers         = 1.10F;
            strcpy ( samData.szOutputName, "4.0 - Quadriphonic" );
            //samData.likChannelCountMode     = ikCHANNELCOUNTMODE_4ch_QUAD;
            break;

        case 0x60: //5.1 - LFE off
            samData.lWithoutStackMode       = 6;
            samData.dwOutputHardwareChannel = 6;
            samData.dwOutputSoftwareChannel = 6;
            samData.dwOutputEncoder         = 0;
            pSAM_RENDER_InitProc            = SAM_RENDER_Init_6CH_L_R_C_noLFE_SL_SR;
            fDistanceWithinSpeakers         = 1.10F;
            strcpy ( samData.szOutputName, "5.0 - Surround without LFE" );
            //samData.likChannelCountMode     = ikCHANNELCOUNTMODE_6ch_SURROUND;
            break;

        case 0x61: //5.1 - LFE on
            samData.lWithoutStackMode       = 6;
            samData.dwOutputHardwareChannel = 6;
            samData.dwOutputSoftwareChannel = 6;
            samData.dwOutputEncoder         = 0;
            pSAM_RENDER_InitProc            = SAM_RENDER_Init_6CH_L_R_C_LFE_SL_SR;
            fDistanceWithinSpeakers         = 1.10F;
            strcpy ( samData.szOutputName, "5.1 - Surround with LFE" );
            //samData.likChannelCountMode     = ikCHANNELCOUNTMODE_6ch_SURROUND;
            break;

    }

    *pRenderInitProc            = (void *)pSAM_RENDER_InitProc;
    *pfDistanceWithinSpeakers   = fDistanceWithinSpeakers;
}