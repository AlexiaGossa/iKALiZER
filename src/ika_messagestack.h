typedef struct {
    DWORD               dwHandle;
    
    IKA_MESSAGE         *pMessage;
   
    DWORD               dwBaseIndex;
    DWORD               dwTotalCount;
    DWORD               dwUsedCount;
    
    _OSI_CRITICAL_SECTION       csHandle;
    _OSI_EVENT                  eventNewMessage;
    _OSI_THREAD                 threadMessageStack;
    
} IKA_MESSAGESTACK;

extern IKA_MESSAGESTACK ikaMessageStack;


long    IKA_MessageStackOpen    ( void );
long    IKA_MessageStackClose   ( void );

long    IKA_MessageStackThreadProc ( void * pProcData );
