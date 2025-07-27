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
