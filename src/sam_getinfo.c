#include "sam_header.h"
#include "sam_data.h"

extern SAM_GRANULESDATA samGranulesData;

DLLEXPORT   long    SAM_GetInfo ( DWORD dwInfoID, DWORD * pdwOutData, char * pszOutData )
{
    DWORD dwData;
    char szTmp[256], szTmp1[256], szTmp2[256];
    char szDesc[256];
    char szDrvName[256];
    
    long lhh, lmm;
    long lYy, lMm, lDd;
    long lBuild;
    
    char szListMonth[64] = "JanFebMarAprMayJunJulAugSepOctNovDec";

    char *psz;

    dwData      = 0;
    szTmp[0]    = 0;
    
    

    switch (dwInfoID)
    {
        /*
            Base information
        */                                              
        
        case SAM_INFO_VERSION_DATA:
            dwData = _SAM_VERSIONL_DATA | (_SAM_VERSIONM_DATA<<16) | (_SAM_VERSIONH_DATA<<24);
            break;
            
        case SAM_INFO_VERSION_BUILD:
        
            //Récupération de "HH:MM:SS"
            strcpy ( szTmp1, _SAM_VERSION_BUILD_TIME );
            
            //Les minutes
            szTmp2[0] = szTmp1[3];
            szTmp2[1] = szTmp1[4];
            szTmp2[2] = 0;
            lmm = (long)atol ( szTmp2 );
            
            //Les heures
            szTmp1[2] = 0;
            lhh = atol ( szTmp1 );
            
            //Récupération de "Mmm dd yyyy"
            strcpy ( szTmp1, _SAM_VERSION_BUILD_DATE );
            
            //L'année
            strcpy ( szTmp2, szTmp1+7 );
            szTmp2[4] = 0;
            lYy = atol ( szTmp2 );
            
            //Le jour
            strcpy ( szTmp2, szTmp1+4 );
            szTmp2[2] = 0;
            lDd = atol ( szTmp2 );
            
            //Le mois
            szTmp1[3] = 0;
            strlwr ( szTmp1 );
            //strcpy ( szListMonth, "JanFebMarAprMayJunJulAugSepOctNovDec" );
            strlwr ( szListMonth );
            psz = strstr ( szListMonth, szTmp1 );
            if (psz!=NULL)
            {
                //psz -= &szListMonth[0];
                lMm = (long)(psz - &szListMonth[0]);
                lMm /= 3;
                lMm += 1;
            }
            else lMm = 0;
            
            //Calcul du numéro de Build
            lBuild = lmm + lhh*60 + (lDd-1)*24*60 + (lMm-1)*24*60*31 + (lYy-2008)*24*60*31*12;
            lBuild /= 10; //Par tranche de 10 minutes
            lBuild -= 0x5D45;
            
            dwData = lBuild;
            break;

        case SAM_INFO_VERSION:
        
            SAM_GetInfo ( SAM_INFO_VERSION_BUILD, &lBuild, NULL );
            
            //Le mode debug
            strcpy ( szTmp2, _SAM_TextDebug );
            
            
            sprintf ( 
                szTmp, 
                "%d.%d.%d TC=0x0%x %s",// *%s*", 
                _SAM_VERSIONH_DATA, 
                _SAM_VERSIONM_DATA,
                _SAM_VERSIONL_DATA,
                lBuild,
                szTmp2 );
            
            _OSI_EnterCriticalSection ( &samData.osiCriticalSection );
            sprintf ( 
                szTmp2, 
                " (%4.1fcps) (%d%% of %d%%) (T=%d)", 
                samData.fCyclesPerSampleMirror, 
                SAM_MATH_lroundf(samData.fMixerAverageUsagePercent), 
                SAM_MATH_lroundf(samData.fMaxProcessorUsagePercent),
                samData.lTimerResolutionMin );
                
            _OSI_LeaveCriticalSection ( &samData.osiCriticalSection );
            
            strcat ( szTmp, szTmp2 );
            break;

        case SAM_INFO_VENDOR:
            strcpy ( szTmp, _SAM_Vendor );
            break;

        case SAM_INFO_COPYRIGHT:
            strcpy ( szTmp, _SAM_Copyright );
            break;

        case SAM_INFO_TITLE:
            strcpy ( szTmp, _SAM_Title );
            break;

        /*
            Memory information (in bytes)
        */
        case SAM_INFO_MEMORY_TOTAL:
            dwData = samData.dwTotalMemoryNeeds;
            break;

        case SAM_INFO_MEMORY_SFXFREE:
            dwData = samGranulesData.dwFreeBytes;
            break;

        case SAM_INFO_MEMORY_SFXUSED:
            dwData = (samGranulesData.dwGranulesCount * samGranulesData.dwRealAudioBytesInGranule) - samGranulesData.dwFreeBytes;
            break;

        case SAM_INFO_MEMORY_SFXTOTAL:
            dwData = samGranulesData.dwGranulesCount * samGranulesData.dwRealAudioBytesInGranule;
            break;

        /*
            
        */
        case SAM_INFO_RENDER_MODE:
            strcpy ( szTmp, samData.szOutputName );
            if (samData.lWithoutStackMode==0) //(samData.dwChannelMode==0x21)||(samData.dwChannelMode==0x22)||(samData.dwChannelMode==0x22))
            {
                switch (samData.dwDynamicDelayLinesCount)
                {
                    case 0:
                        strcat ( szTmp, " (DDL=off)" );
                        break;
                    case 0xFFFFFFFF:
                        strcat ( szTmp, " (DDL=auto)" );                        
                        break;
                    case 0xFFFFFFFE:
                        strcat ( szTmp, " (DDL=DLF processor)" );
                        break;
                    default:
                        sprintf ( szTmp2, " (DDL=%d%%)", samData.dwDynamicDelayLinesCount );
                        strcat ( szTmp, szTmp2 );
                        break;
                }
            }
            //_OSI_EnterCriticalSection ( &samData.osiCriticalSection );
            //sprintf ( szTmp2, " (B/U:%d)", samData.lTotalBufferUnderrunCount );
            //_OSI_LeaveCriticalSection ( &samData.osiCriticalSection );
            //strcat ( szTmp, szTmp2 );
            
            dwData = samData.dwChannelMode;
            break;
            
        /*
            Info sur le système
        */
        case SAM_INFO_SYSTEM_INSTANCES:
            dwData = samData.lInstancesCount;
            break;
            
        case SAM_INFO_SYSTEM_SIMD:
            strcpy ( szTmp, _SAM_SIMD );
            if (samData.lProcessorEnableSSE) strcat ( szTmp, _SAM_SIMD_SSE );
            if (samData.lProcessorEnableSSE2) strcat ( szTmp, _SAM_SIMD_SSE2 );
            if (samData.lProcessorEnableSSE3) strcat ( szTmp, _SAM_SIMD_SSE3 );
            if ((!samData.lProcessorEnableSSE)&&
                (samData.lProcessorEnableSSE2) ) strcat ( szTmp, _SAM_SIMD_ABSENT );
            break;
            
        case SAM_INFO_DEVICE_GETCURRENT:
            /*
            {            
                ikAUDIODEVICEFEATURES adfEntry;
                ikAudioInterfaceEnumQuery ( &samData.aiMain, &adfEntry, samData.dwHardwareDeviceSelected );
                
                sprintf ( szTmp, "%s (%s)", adfEntry.szDesc, adfEntry.szDrvName );
                dwData = samData.dwHardwareDeviceSelected;
            }
            */
                
            SAM_DirectSound_GetEnumInfo (
                samData.dwHardwareDeviceSelected, 
                szDesc,
                szDrvName );
        
            sprintf ( szTmp, "%s (%s)", szDesc, szDrvName );
            dwData = samData.dwHardwareDeviceSelected;
            
            break;
            
        case SAM_INFO_SUPPORT_SELECT_DEVICE:
        case SAM_INFO_SUPPORT_MESSAGE_UNDEFINED:
        case SAM_INFO_SUPPORT_MESSAGE_VOICE_SYNCFREEZE:
        case SAM_INFO_SUPPORT_MESSAGE_VOICE_ISPLAYED:
        case SAM_INFO_SUPPORT_MESSAGE_VOICE_POSITION:
            dwData = 1;
            break;
            
        case SAM_INFO_DEVICE_OUTPUT_MODEL:
            //SAM_DirectSound_GetOutputModelFlags ( szTmp, NULL );
            sprintf ( szTmp, "No model" );
            break;
            
        case SAM_INFO_DEVICE_OUTPUT_FLAGS:
            //SAM_DirectSound_GetOutputModelFlags ( NULL, szTmp );
            sprintf ( szTmp, "No flag" );
            break;
    }
    
    if ( (dwInfoID>=SAM_INFO_DEVICE_ENUM_00) &&
         (dwInfoID<=SAM_INFO_DEVICE_ENUM_63) )
    {
        /*
        ikAUDIODEVICEFEATURES adfEntry;
        dwData = (DWORD)ikAudioInterfaceEnumQuery ( &samData.aiMain, &adfEntry, dwInfoID-SAM_INFO_DEVICE_ENUM_00 );
        sprintf ( szTmp, "%s (%s)", adfEntry.szDesc, adfEntry.szDrvName );
        */
        dwData = (DWORD)SAM_DirectSound_GetEnumInfo (
            dwInfoID-SAM_INFO_DEVICE_ENUM_00, 
            szDesc,
            szDrvName );
    
        sprintf ( szTmp, "%s (%s)", szDesc, szDrvName );
    }

    if (pdwOutData)
        *pdwOutData = dwData;

    if (pszOutData)
    {
        SAM_StringAccentKill ( szTmp );
        strcpy ( pszOutData, szTmp );
    }

    return 0;
}
