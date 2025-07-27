#ifdef _WIN32
    #include <windows.h>
    #include <windowsx.h>
    #include <winuser.h>
    #include <winbase.h>
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
#endif

#include "sam_header.h"


#ifdef _WIN32
   

    typedef struct {
        DWORD           dwThreadID;
        HANDLE          hThread;
        BOOL            bThreadExit;
        void            *pUserParams;
        char            szName[16];
    } _OSI_THREAD_DATA;


    long    _OSI_ThreadOpen ( _OSI_THREAD * pThread, long (*pThreadProc) ( void * pProcData ), unsigned long dwCreationFlags, void * pParams )
    {
        _OSI_THREAD_DATA * pThreadData;
        pThreadData = (_OSI_THREAD_DATA *)pThread;

        memset ( pThreadData, 0, sizeof(_OSI_THREAD_DATA) );
        pThreadData->pUserParams = pParams;

        pThreadData->hThread = CreateThread ( 
                                NULL,                                   //No security attributes
                                0,                                      //Default Stack size
                                (LPTHREAD_START_ROUTINE)pThreadProc,    //Thread proc
                                (void *)pThreadData,                    //Input params
                                dwCreationFlags,                        //Creation flag
                                &(pThreadData->dwThreadID) );           //Used for compatibility with Win95/98/Me

        if (pThreadData->hThread) return 0;
        else                      return -1;
    }

    long    _OSI_ThreadOpenEx ( _OSI_THREAD * pThread, long (*pThreadProc) ( void * pProcData ), unsigned long dwCreationFlags, unsigned long dwStackSizeMB, void * pParams )
    {
         _OSI_THREAD_DATA * pThreadData;
        pThreadData = (_OSI_THREAD_DATA *)pThread;

        memset ( pThreadData, 0, sizeof(_OSI_THREAD_DATA) );
        pThreadData->pUserParams = pParams;

        pThreadData->hThread = CreateThread ( 
                                NULL,                                   //No security attributes
                                dwStackSizeMB<<20,                      //Default Stack size
                                (LPTHREAD_START_ROUTINE)pThreadProc,    //Thread proc
                                (void *)pThread,                        //Input params
                                dwCreationFlags,                        //Creation flag
                                &(pThreadData->dwThreadID) );           //Used for compatibility with Win95/98/Me

        if (pThreadData->hThread) return 0;
        else                      return -1;
    }


    long    _OSI_ThreadClose ( _OSI_THREAD * pThread, long lMaxDelayBeforeKill_ms )
    {
        long    lCount;
        DWORD   dwExitCode;
        DWORD   dwFlags;

        _OSI_THREAD_DATA * pThreadData;
        pThreadData = (_OSI_THREAD_DATA *)pThread;

        if (!pThreadData) return -1;
        if (!pThreadData->hThread) return -1;

        //Destruction de la thread
        lCount = lMaxDelayBeforeKill_ms;
        pThreadData->bThreadExit = 1;
        do {
            _OSI_Sleep ( 10 );
            GetExitCodeThread ( pThreadData->hThread, &dwExitCode );
            lCount-=10;
        } while ((dwExitCode==_OSI_THREAD_STILL_ACTIVE)&&(lCount>0));
        if (dwExitCode==_OSI_THREAD_STILL_ACTIVE) TerminateThread ( pThreadData->hThread, 0 );

        //Fermeture de la Thread
        dwFlags = 0;
        if (GetHandleInformation ( (HANDLE)pThreadData->hThread, &dwFlags ))
        {
            if (!(dwFlags&HANDLE_FLAG_PROTECT_FROM_CLOSE))
                CloseHandle ( (HANDLE)pThreadData->hThread );
        }

        memset ( pThreadData, 0, sizeof(_OSI_THREAD) );

        return 0;
    }

    #define MS_VC_EXCEPTION 0x406d1388

    typedef struct tagTHREADNAME_INFO
    {
        DWORD dwType;        // must be 0x1000
        LPCSTR szName;       // pointer to name (in same addr space)
        DWORD dwThreadID;    // thread ID (-1 caller thread)
        DWORD dwFlags;       // reserved for future use, most be zero
    } THREADNAME_INFO;

    long    _OSI_ThreadSetName ( _OSI_THREAD * pThread, char * pszShortName )
    {
        THREADNAME_INFO info;

        _OSI_THREAD_DATA * pThreadData;
        pThreadData = (_OSI_THREAD_DATA *)pThread;

        strncpy ( pThreadData->szName, pszShortName, 9 );

        info.dwType         = 0x1000;
        info.szName         = pThreadData->szName;
        info.dwThreadID     = pThreadData->dwThreadID;
        info.dwFlags        = 0;

        __try
        {
            RaiseException(MS_VC_EXCEPTION, 0, sizeof(info) / sizeof(DWORD),(DWORD *)&info);
        }
        __except (EXCEPTION_CONTINUE_EXECUTION)
        {

        }

        return 0;
    }

    long    _OSI_ThreadGetProcParams ( void * pProcData, void ** pParams )
    {
        _OSI_THREAD_DATA * pThreadData;
        pThreadData = (_OSI_THREAD_DATA *)pProcData;
        *pParams = pThreadData->pUserParams;
        return 0;
    }

    long    _OSI_ThreadSetPriority ( _OSI_THREAD * pThread, DWORD dwPriority )
    {
        _OSI_THREAD_DATA * pThreadData;
        pThreadData = (_OSI_THREAD_DATA *)pThread;

        if (!pThreadData) return -1;
        if (!pThreadData->hThread) return -1;

        if (SetThreadPriority ( pThreadData->hThread, dwPriority )) return 0;
        else                                                        return -1;
    }

    long    _OSI_ThreadSetAffinity ( _OSI_THREAD * pThread, DWORD dwAffinityMask )
    {
        _OSI_THREAD_DATA * pThreadData;
        pThreadData = (_OSI_THREAD_DATA *)pThread;

        if (!pThreadData) return -1;
        if (!pThreadData->hThread) return -1;
        if (!dwAffinityMask) return -1;

        if (SetThreadAffinityMask ( pThreadData->hThread, dwAffinityMask )) return 0;
        else return -1;
    }

    long    _OSI_ThreadSuspend ( _OSI_THREAD * pThread )
    {
        _OSI_THREAD_DATA * pThreadData;
        pThreadData = (_OSI_THREAD_DATA *)pThread;

        if (!pThreadData) return -1;
        if (!pThreadData->hThread) return -1;

        SuspendThread ( pThreadData->hThread );
        return 0;
    }
        
    long    _OSI_ThreadResume ( _OSI_THREAD * pThread )
    {
        _OSI_THREAD_DATA * pThreadData;
        pThreadData = (_OSI_THREAD_DATA *)pThread;

        if (!pThreadData) return -1;
        if (!pThreadData->hThread) return -1;

        ResumeThread ( pThreadData->hThread );
        return 0;
    }

    long    _OSI_ThreadDoWhileAndWaitExit ( void * pProcData )
    {
        _OSI_THREAD_DATA * pThreadData;
        pThreadData = (_OSI_THREAD_DATA *)pProcData;
        if (pThreadData->bThreadExit) return 0;
        else                          return 1;
    }

#else
    #error Need to implement Platform-dependent Threads.
#endif
