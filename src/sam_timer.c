/*

        

*/


#include "sam_header.h"


typedef struct {
    _OSI_CRITICAL_SECTION   osiCS;
    DWORD                   dwTimerCount;
} SAM_TIMER_DATA;

SAM_TIMER_DATA samTimerData;


long SAM_TIMER_Open ( void )
{
    memset ( &samTimerData, 0, sizeof(SAM_TIMER_DATA) );
    
    _OSI_InitializeCriticalSectionAndSpinCount ( &samTimerData.osiCS, 0x80001000 );
    
    return 0;
}

long SAM_TIMER_Close ( void )
{
    _OSI_DeleteCriticalSection ( &samTimerData.osiCS );
    
    memset ( &samTimerData, 0, sizeof(SAM_TIMER_DATA) );
    
    return 0;
}

long SAM_TIMER_Increment ( DWORD * pdwTimer )
{
    DWORD dwTimer;
    
    _OSI_EnterCriticalSection ( &samTimerData.osiCS );
    
    samTimerData.dwTimerCount += 1;    
    dwTimer = samTimerData.dwTimerCount;
    
    _OSI_LeaveCriticalSection ( &samTimerData.osiCS );    
    
    if (pdwTimer) *pdwTimer = dwTimer;
    
    return 0;
}

long SAM_TIMER_GetIt ( DWORD * pdwTimer )
{
    DWORD dwTimer;
    
    _OSI_EnterCriticalSection ( &samTimerData.osiCS );
    dwTimer = samTimerData.dwTimerCount;
    _OSI_LeaveCriticalSection ( &samTimerData.osiCS );    
    
    if (pdwTimer) *pdwTimer = dwTimer;
    
    return 0;
}