#include "sam_header.h"
//#include "zwdebug.h"
#include "Windows.h"

void _SAM_DEBUG_TEXT_ ( char * pszText, ... )
{
    char        szTmp[1024];
    va_list     vaList;
    
    if (!pszText) return;

    //Conversion...
    va_start ( vaList, pszText );
    _vsnprintf ( szTmp, 1023, pszText, vaList );
    strcat ( szTmp, "\n" );
    OutputDebugString ( szTmp );
}