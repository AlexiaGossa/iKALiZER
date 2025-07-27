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
#endif
    
#include "ika_osi.h"


#ifdef _WIN32

    long    _OSI_Sleep                                       ( unsigned long dwMilliseconds )
    {
        Sleep ( dwMilliseconds );
        return 0;
    }

    long    _OSI_GetTickCount                                ( unsigned long * pdwTick )
    {
        *pdwTick = GetTickCount ( );
        return 0;
    }


#else
    #error Need to implement Platform-dependent Threads.
#endif
