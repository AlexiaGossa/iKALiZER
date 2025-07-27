#include "sam_header.h"


#ifdef _WIN32

    //Nécessaire pour utiliser InitializeCriticalSectionAndSpinCount
    //Mais compatible à partir de Windows 2000/XP/Vista
    #define _WIN32_WINNT    0x0500

    #include <windows.h>
    #include <windowsx.h>
    #include <winuser.h>
    #include <tchar.h>
    #include <stdio.h>
    #include <string.h>
    #include <conio.h>
    #include <direct.h>
    #include <stdlib.h>
    #include <commctrl.h>
    #include <fcntl.h>
    #include <io.h>
    #include <time.h>
    #include <math.h>
    #include <commdlg.h>
    #include <malloc.h>
    #include <winbase.h>

    typedef struct {
        CRITICAL_SECTION    win32CriticalSection;
        long                lCreated;
    } _OSI_CRITICAL_SECTION_DATA;

    long _OSI_EnterCriticalSection ( _OSI_CRITICAL_SECTION * pCriticalSection )
    {
        _OSI_CRITICAL_SECTION_DATA * pCriticalSectionData;
        pCriticalSectionData = (_OSI_CRITICAL_SECTION_DATA *)pCriticalSection;

        if (pCriticalSectionData->lCreated==1)
            EnterCriticalSection ( &pCriticalSectionData->win32CriticalSection );
        return 0;
    }

    long _OSI_LeaveCriticalSection ( _OSI_CRITICAL_SECTION * pCriticalSection )
    {
        _OSI_CRITICAL_SECTION_DATA * pCriticalSectionData;
        pCriticalSectionData = (_OSI_CRITICAL_SECTION_DATA *)pCriticalSection;

        if (pCriticalSectionData->lCreated==1)
            LeaveCriticalSection ( &pCriticalSectionData->win32CriticalSection );
        return 0;
    }

    long _OSI_InitializeCriticalSection ( _OSI_CRITICAL_SECTION * pCriticalSection )
    {
        _OSI_CRITICAL_SECTION_DATA * pCriticalSectionData;
        pCriticalSectionData = (_OSI_CRITICAL_SECTION_DATA *)pCriticalSection;

        InitializeCriticalSection ( &pCriticalSectionData->win32CriticalSection );
        pCriticalSectionData->lCreated = 1;
        return 0;
    }

    long _OSI_InitializeCriticalSectionAndSpinCount ( _OSI_CRITICAL_SECTION * pCriticalSection, DWORD dwSpinCount )
    {
        _OSI_CRITICAL_SECTION_DATA * pCriticalSectionData;
        pCriticalSectionData = (_OSI_CRITICAL_SECTION_DATA *)pCriticalSection;

        InitializeCriticalSectionAndSpinCount ( &pCriticalSectionData->win32CriticalSection, dwSpinCount );
        pCriticalSectionData->lCreated = 1;
        return 0;
    }

    long _OSI_DeleteCriticalSection ( _OSI_CRITICAL_SECTION * pCriticalSection )
    {
        _OSI_CRITICAL_SECTION_DATA * pCriticalSectionData;
        pCriticalSectionData = (_OSI_CRITICAL_SECTION_DATA *)pCriticalSection;

        if (pCriticalSectionData->lCreated==1)
        {
            DeleteCriticalSection ( &pCriticalSectionData->win32CriticalSection );
            memset ( pCriticalSectionData, 0, sizeof(_OSI_CRITICAL_SECTION_DATA) );
        }
        return 0;
    }

    /*

    long _OSI_EnterCriticalSection ( _OSI_CRITICAL_SECTION * pCriticalSection )
    {
        RTL_CRITICAL_SECTION * pRCS;
        pRCS = (RTL_CRITICAL_SECTION *)pCriticalSection->pOSDependentData;
        if (pCriticalSection->pOSDependentData) EnterCriticalSection ( (LPCRITICAL_SECTION)pCriticalSection->pOSDependentData );
        return 0;
    }

    long _OSI_LeaveCriticalSection ( _OSI_CRITICAL_SECTION * pCriticalSection )
    {
        if (pCriticalSection->pOSDependentData) LeaveCriticalSection ( (LPCRITICAL_SECTION)pCriticalSection->pOSDependentData );
        return 0;
    }

    long _OSI_InitializeCriticalSection ( _OSI_CRITICAL_SECTION * pCriticalSection )
    {
        pCriticalSection->pOSDependentData = (void *)malloc ( sizeof(CRITICAL_SECTION) );
        InitializeCriticalSection ( (LPCRITICAL_SECTION)(pCriticalSection->pOSDependentData) );
        return 0;
    }

    long _OSI_DeleteCriticalSection ( _OSI_CRITICAL_SECTION * pCriticalSection )
    {
        if (pCriticalSection->pOSDependentData)
        {
            DeleteCriticalSection ( (LPCRITICAL_SECTION)(pCriticalSection->pOSDependentData) );
            free ( pCriticalSection->pOSDependentData );
            pCriticalSection->pOSDependentData = NULL;
        }
        return 0;
    }

    */
#else
    #error Need to implement Platform-dependent CriticalSection.
#endif
