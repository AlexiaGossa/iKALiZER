#include "sam_header.h"
#include "sam_data.h"

typedef struct {
    char szVendor[32];
    char szDesignation[64];
    long lFamily;
    long lModel;
    long lStepping;
    long lType;
    long lCacheL1Size;
    long lCacheL2Size;
    long lInvalidTSC;
} PROCESSOR_LIST;


typedef struct {
    char szManufacturer[256];       //Fabricant
    char szDesignation[256];        //Designation
    long lGeneration;               //>=5 => Pentium Class

    long x86_MMX;                   //Support MMX
    long x86_MmxExt;                //Support MmxExt
    long x86_SSE;                   //Support SSE
    long x86_SSE2;                  //Support SSE2
    long x86_SSE3;                  //Support SSE3
    long x86_SSSE3;                 //Support SSSE3
    long x86_3DNow;                 //Support 3DNow!
    long x86_3DNowExtended;         //Support Extended 3DNow!
    long x86_X64;                   //Support x64
    long x86_HTT;                   //Support HyperThreadingTechnology
    long x86_VT;                    //Support VirtualisationTechnology
    long x86_NX;                    //Support No-execute
    long x86_Prefetch;              //Support pour la fonction prefetch
    long x86_CMOV;                  //Support CMOV
    long x86_CMPXCHG8;              //Support CMPXCHG8
    long x86_RDTSC;                 //Support RDTSC
    long x86_MSR;                   //Support MSR
    long x86_ACPI;                  //Support ACPI (Thermal Monitor + Software Controlled Clock)
    long x86_Channels;              //Nombre de canaux

    long lInvalidTSC;               //Informe que le TSC n'est plus en relation avec la fréquence/les cycles

    long    lCPU_CacheDataLevel1_KiB;
    long    lCPU_CacheCodeLevel1_KiB;
    long    lCPU_CacheDataLevel2_KiB;
    long    lCPU_CacheDataLevel3_KiB;

    long    lCPU_LocalApicID;
    long    lCPU_LogicalCoreCount;  //LogicalProcessorCount. AMD only ?
    long    lCPU_CmpLegacy;         //AMD only ?
    long    lCPU_NC;                //Number of CPU cores-1. AMD only ?

    long    lCPU_Family;
    long    lCPU_Model;
    long    lCPU_Stepping;
    long    lCPU_ExtFamily;
    long    lCPU_ExtModel;

    long    lCPU_ThreadsPerCore;
    long    lCPU_CoresPerProcessor;

    long    lIntel_TLBC[16];
    long    lIntel_EnableEaxTLBC;
    long    lIntel_EnableEbxTLBC;
    long    lIntel_EnableEcxTLBC;
    long    lIntel_EnableEdxTLBC;
    long    lIntel_NeedToResolveL2L3_0x40;
    long    lIntel_NeedToResolveL2L3_0x49;
    long    lIntel_DTS;

    long    lAMD_DTS;

} PROCESSOR_INFO;



extern PROCESSOR_LIST ProcessorList[];


void CPUID_Init ( void );



typedef struct {
    long lDetectedValue;
    long lWriteValue;
    long *plObjectToWrite;
} VALUEDETECTION;

typedef struct {
    DWORD dwFnCPUID;
    char szRegister[4];
    long lBitStart;
    long lBitCount;
    char szName[64];
    long *plOutToStore;
} CPUID_FEATURESLIST;

