#include "sam_header.h"

#ifdef _WIN32
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
        HANDLE      hEvent;
        char        szName[64];
    } _OSI_EVENT_DATA;



    long _OSI_InitializeEvent ( _OSI_EVENT * pEvent, BOOL bEnableManualReset, BOOL bInitialState )
    {
        _OSI_EVENT_DATA * pEventData;
        pEventData = (_OSI_EVENT_DATA *)pEvent;

        if (!pEventData) return -1;

        pEventData->hEvent = CreateEvent ( NULL, bEnableManualReset, bInitialState, NULL );
        if (!pEventData->hEvent) return -2;
        return 0;
    }

    long _OSI_DeleteEvent ( _OSI_EVENT * pEvent )
    {
        _OSI_EVENT_DATA * pEventData;
        pEventData = (_OSI_EVENT_DATA *)pEvent;

        if (!pEventData) return -1;

        CloseHandle ( pEventData->hEvent ) ;
        memset ( pEventData, 0, sizeof(_OSI_EVENT_DATA) );
        return 0;
    }

    long _OSI_SetEvent ( _OSI_EVENT * pEvent )
    {
        _OSI_EVENT_DATA * pEventData;
        pEventData = (_OSI_EVENT_DATA *)pEvent;

        if (!pEventData) return -1;

        SetEvent ( pEventData->hEvent );
        return 0;
    }

    long _OSI_ResetEvent ( _OSI_EVENT * pEvent )
    {
        _OSI_EVENT_DATA * pEventData;
        pEventData = (_OSI_EVENT_DATA *)pEvent;

        if (!pEventData) return -1;

        ResetEvent ( pEventData->hEvent );
        return 0;
    }

    long _OSI_WaitForEvent ( _OSI_EVENT * pEvent, unsigned long dwMilliseconds, long * plReturnWaitState )
    {
        unsigned long dwResult;

        _OSI_EVENT_DATA * pEventData;
        pEventData = (_OSI_EVENT_DATA *)pEvent;

        if (!pEventData) return -1;

        //Possible risk of bug under Windows NT (needed SYNCHRONIZE access), see MSDN
        dwResult = WaitForSingleObject ( pEventData->hEvent, dwMilliseconds );

        if ((dwResult==WAIT_ABANDONED)||(dwResult==WAIT_FAILED)) return -1;

        if (plReturnWaitState)
        {
            switch (dwResult)
            {
                case WAIT_TIMEOUT:      *plReturnWaitState = 0;    break;
                case WAIT_OBJECT_0:     *plReturnWaitState = 1;    break;
            }
        }
        return 0;
    }

#else
    #error Need to implement Platform-dependent Event.
#endif
