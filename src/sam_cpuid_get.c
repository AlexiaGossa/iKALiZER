#include "sam_cpuid.h"



PROCESSOR_INFO  ProcessorInfo;
long            lProcessorInit = 0;

CPUID_FEATURESLIST 
cpuidGlobalFeatList[] = {   { 0x00000000, "ebx",  0, 32, "CPUVendor0",              (long *)(ProcessorInfo.szManufacturer) },
                            { 0x00000000, "edx",  0, 32, "CPUVendor1",              (long *)(ProcessorInfo.szManufacturer+4) },
                            { 0x00000000, "ecx",  0, 32, "CPUVendor2",              (long *)(ProcessorInfo.szManufacturer+8) },

                            { 0x00000001, "eax",  0,  4, "Stepping",                &ProcessorInfo.lCPU_Stepping },             //OK Intel+AMD
                            { 0x00000001, "eax",  4,  4, "Model",                   &ProcessorInfo.lCPU_Model },                //OK Intel+AMD
                            { 0x00000001, "eax",  8,  4, "Family",                  &ProcessorInfo.lCPU_Family },               //OK Intel+AMD
                            { 0x00000001, "eax", 16,  4, "ExtModel",                &ProcessorInfo.lCPU_ExtModel },             //OK Intel+AMD
                            { 0x00000001, "eax", 20,  8, "ExtFamily",               &ProcessorInfo.lCPU_ExtFamily },            //OK Intel+AMD

                            { 0x00000001, "ebx", 24,  8, "LocalApicId",             &ProcessorInfo.lCPU_LocalApicID },          //OK Intel+AMD
                            { 0x00000001, "ebx", 16,  8, "LogicalProcessorCount",   &ProcessorInfo.lCPU_LogicalCoreCount },     //OK Intel+AMD

                            { 0x00000001, "ecx",  0,  1, "SSE3",                    &ProcessorInfo.x86_SSE3 },                  //OK Intel+AMD

                            { 0x00000001, "edx", 28,  1, "HTT",                     &ProcessorInfo.x86_HTT },                   //OK Intel+AMD
                            { 0x00000001, "edx", 26,  1, "SSE2",                    &ProcessorInfo.x86_SSE2 },                  //OK Intel+AMD
                            { 0x00000001, "edx", 25,  1, "SSE",                     &ProcessorInfo.x86_SSE },                   //OK Intel+AMD
                            { 0x00000001, "edx", 23,  1, "MMX",                     &ProcessorInfo.x86_MMX },                   //OK Intel+AMD
                            { 0x00000001, "edx", 15,  1, "CMOV",                    &ProcessorInfo.x86_CMOV },                  //OK Intel+AMD
                            { 0x00000001, "edx",  8,  1, "CMPXCHG8",                &ProcessorInfo.x86_CMPXCHG8 },              //OK Intel+AMD
                            { 0x00000001, "edx",  4,  1, "RDTSC",                   &ProcessorInfo.x86_RDTSC },                 //OK Intel+AMD
                            { 0x00000001, "edx",  5,  1, "MSR",                     &ProcessorInfo.x86_MSR },                   //OK Intel+AMD                             

                            { 0x80000001, "edx", 29,  1, "x64-64",                  &ProcessorInfo.x86_X64 },                   //OK Intel+AMD
                            { 0x80000001, "edx", 20,  1, "NX",                      &ProcessorInfo.x86_NX },                    //OK Intel+AMD

                            { 0x80000002, "eax",  0, 32, "CPUName0",                (long *)(ProcessorInfo.szDesignation) },
                            { 0x80000002, "ebx",  0, 32, "CPUName1",                (long *)(ProcessorInfo.szDesignation+4) },
                            { 0x80000002, "ecx",  0, 32, "CPUName2",                (long *)(ProcessorInfo.szDesignation+8) },
                            { 0x80000002, "edx",  0, 32, "CPUName3",                (long *)(ProcessorInfo.szDesignation+12) },
                            { 0x80000003, "eax",  0, 32, "CPUName4",                (long *)(ProcessorInfo.szDesignation+16) },
                            { 0x80000003, "ebx",  0, 32, "CPUName5",                (long *)(ProcessorInfo.szDesignation+20) },
                            { 0x80000003, "ecx",  0, 32, "CPUName6",                (long *)(ProcessorInfo.szDesignation+24) },
                            { 0x80000003, "edx",  0, 32, "CPUName7",                (long *)(ProcessorInfo.szDesignation+28) },
                            { 0x80000004, "eax",  0, 32, "CPUName8",                (long *)(ProcessorInfo.szDesignation+32) },
                            { 0x80000004, "ebx",  0, 32, "CPUName9",                (long *)(ProcessorInfo.szDesignation+36) },
                            { 0x80000004, "ecx",  0, 32, "CPUNameA",                (long *)(ProcessorInfo.szDesignation+40) },
                            { 0x80000004, "edx",  0, 32, "CPUNameB",                (long *)(ProcessorInfo.szDesignation+44) },

                            /*{ 0x80000005, "ecx", 24,  8, "L1DataKiB",               &ProcessorInfo.lCPU_CacheDataLevel1_KiB },
                            { 0x80000005, "edx", 24,  8, "L1CodeKiB",               &ProcessorInfo.lCPU_CacheCodeLevel1_KiB },*/                            
                            /*{ 0x80000008, "ecx",  0,  8, "NC",                      &ProcessorInfo.lCPU_NC },*/
                            { -1 } };

CPUID_FEATURESLIST 
cpuid_INTEL_FeatList[] = {  { 0x00000001, "ecx",  5,  1, "VM",                      &ProcessorInfo.x86_VT },
                            { 0x00000001, "ecx",  9,  1, "SSSE3",                   &ProcessorInfo.x86_SSSE3 },
                            { 0x00000001, "edx", 30,  1, "IA64",                    &ProcessorInfo.x86_X64 },
                            { 0x00000001, "edx", 22,  1, "ACPI",                    &ProcessorInfo.x86_ACPI },
                            { 0x00000002, "eax",  8,  8, "TLBC_eax",                &ProcessorInfo.lIntel_TLBC[1] },
                            { 0x00000002, "eax", 16,  8, "TLBC_eax",                &ProcessorInfo.lIntel_TLBC[2] },
                            { 0x00000002, "eax", 24,  8, "TLBC_eax",                &ProcessorInfo.lIntel_TLBC[3] },
                            { 0x00000002, "ebx",  0,  8, "TLBC_ebx",                &ProcessorInfo.lIntel_TLBC[4] },
                            { 0x00000002, "ebx",  8,  8, "TLBC_ebx",                &ProcessorInfo.lIntel_TLBC[5] },
                            { 0x00000002, "ebx", 16,  8, "TLBC_ebx",                &ProcessorInfo.lIntel_TLBC[6] },
                            { 0x00000002, "ebx", 24,  8, "TLBC_ebx",                &ProcessorInfo.lIntel_TLBC[7] },
                            { 0x00000002, "ecx",  0,  8, "TLBC_ecx",                &ProcessorInfo.lIntel_TLBC[8] },
                            { 0x00000002, "ecx",  8,  8, "TLBC_ecx",                &ProcessorInfo.lIntel_TLBC[9] },
                            { 0x00000002, "ecx", 16,  8, "TLBC_ecx",                &ProcessorInfo.lIntel_TLBC[10] },
                            { 0x00000002, "ecx", 24,  8, "TLBC_ecx",                &ProcessorInfo.lIntel_TLBC[11] },
                            { 0x00000002, "edx",  0,  8, "TLBC_edx",                &ProcessorInfo.lIntel_TLBC[12] },
                            { 0x00000002, "edx",  8,  8, "TLBC_edx",                &ProcessorInfo.lIntel_TLBC[13] },
                            { 0x00000002, "edx", 16,  8, "TLBC_edx",                &ProcessorInfo.lIntel_TLBC[14] },
                            { 0x00000002, "edx", 24,  8, "TLBC_edx",                &ProcessorInfo.lIntel_TLBC[15] },
                            { 0x00000002, "eax", 31,  1, "TLBC_eneax",              &ProcessorInfo.lIntel_EnableEaxTLBC },
                            { 0x00000002, "ebx", 31,  1, "TLBC_enebx",              &ProcessorInfo.lIntel_EnableEbxTLBC },
                            { 0x00000002, "ecx", 31,  1, "TLBC_enecx",              &ProcessorInfo.lIntel_EnableEcxTLBC },
                            { 0x00000002, "edx", 31,  1, "TLBC_enedx",              &ProcessorInfo.lIntel_EnableEdxTLBC },
                            { 0x00000006, "eax",  0,  1, "DTS feature",             &ProcessorInfo.lIntel_DTS },
                            { 0x80000006, "ecx", 16, 16, "L2DataKiB",               &ProcessorInfo.lCPU_CacheDataLevel2_KiB },
                            { 0x00000004, "eax", 26,  6, "CoresPerPhy",             &ProcessorInfo.lCPU_CoresPerProcessor },

                            { -1 } };


CPUID_FEATURESLIST 
cpuid_AMD_FeatList[] = {    { 0x80000001, "ecx",  1,  1, "CmpLegacy",               &ProcessorInfo.lCPU_CmpLegacy },
                            { 0x80000001, "edx", 31,  1, "3DNow!",                  &ProcessorInfo.x86_3DNow },
                            { 0x80000001, "edx", 30,  1, "3DNow!Ext",               &ProcessorInfo.x86_3DNowExtended },
                            { 0x80000001, "edx", 22,  1, "MmxExt",                  &ProcessorInfo.x86_MmxExt },
                            { 0x80000005, "ecx", 24,  8, "L1DataKiB",               &ProcessorInfo.lCPU_CacheDataLevel1_KiB },
                            { 0x80000005, "edx", 24,  8, "L1CodeKiB",               &ProcessorInfo.lCPU_CacheCodeLevel1_KiB },
                            { 0x80000006, "ecx", 16, 16, "L2DataKiB",               &ProcessorInfo.lCPU_CacheDataLevel2_KiB },
                            { 0x80000008, "ecx",  0,  8, "NC",                      &ProcessorInfo.lCPU_NC },
                            { 0x80000007, "edx",  0,  1, "DTS feature",             &ProcessorInfo.lAMD_DTS },
                            { -1 } };


VALUEDETECTION
vd_INTEL_TBLC[] = {     {   0x00,       0,      NULL },
                        {   0x06,       8,      &ProcessorInfo.lCPU_CacheCodeLevel1_KiB  },
                        {   0x08,      16,      &ProcessorInfo.lCPU_CacheCodeLevel1_KiB  }, 
                        {   0x0A,       8,      &ProcessorInfo.lCPU_CacheDataLevel1_KiB  },
                        {   0x0C,      16,      &ProcessorInfo.lCPU_CacheDataLevel1_KiB  }, 
                        {   0x22,     512,      &ProcessorInfo.lCPU_CacheDataLevel3_KiB  },
                        {   0x23,    1024,      &ProcessorInfo.lCPU_CacheDataLevel3_KiB  }, 
                        {   0x25,    2048,      &ProcessorInfo.lCPU_CacheDataLevel3_KiB  },
                        {   0x29,    4096,      &ProcessorInfo.lCPU_CacheDataLevel3_KiB  },
                        {   0x2C,      32,      &ProcessorInfo.lCPU_CacheDataLevel1_KiB  },
                        {   0x30,      32,      &ProcessorInfo.lCPU_CacheCodeLevel1_KiB  },
                        {   0x39,     128,      &ProcessorInfo.lCPU_CacheDataLevel2_KiB  },
                        {   0x3A,     192,      &ProcessorInfo.lCPU_CacheDataLevel2_KiB  },
                        {   0x3B,     128,      &ProcessorInfo.lCPU_CacheDataLevel2_KiB  },
                        {   0x3C,     256,      &ProcessorInfo.lCPU_CacheDataLevel2_KiB  },
                        {   0x3D,     384,      &ProcessorInfo.lCPU_CacheDataLevel2_KiB  },
                        {   0x3E,     512,      &ProcessorInfo.lCPU_CacheDataLevel2_KiB  },
                        {   0x40,       1,      &ProcessorInfo.lIntel_NeedToResolveL2L3_0x40 },
                        {   0x41,     128,      &ProcessorInfo.lCPU_CacheDataLevel2_KiB  },
                        {   0x42,     256,      &ProcessorInfo.lCPU_CacheDataLevel2_KiB  },
                        {   0x43,     512,      &ProcessorInfo.lCPU_CacheDataLevel2_KiB  },
                        {   0x44,    1024,      &ProcessorInfo.lCPU_CacheDataLevel2_KiB  },
                        {   0x45,    2048,      &ProcessorInfo.lCPU_CacheDataLevel2_KiB  },
                        {   0x46,    4096,      &ProcessorInfo.lCPU_CacheDataLevel3_KiB  },
                        {   0x47,    8192,      &ProcessorInfo.lCPU_CacheDataLevel3_KiB  },
                        //{   0x49,    4096,      &ProcessorInfo.lCPU_CacheDataLevel3_KiB  }, //Intel Xeon processor MP, Family F, Model 6
                        //{   0x49,    4096,      &ProcessorInfo.lCPU_CacheDataLevel2_KiB  },
                        {   0x49,       1,      &ProcessorInfo.lIntel_NeedToResolveL2L3_0x49 },
                        {   0x4A,    6144,      &ProcessorInfo.lCPU_CacheDataLevel3_KiB  },
                        {   0x4B,    8192,      &ProcessorInfo.lCPU_CacheDataLevel3_KiB  },
                        {   0x4C,   12288,      &ProcessorInfo.lCPU_CacheDataLevel3_KiB  },
                        {   0x4D,   16384,      &ProcessorInfo.lCPU_CacheDataLevel3_KiB  },

                        {   0x60,      16,      &ProcessorInfo.lCPU_CacheDataLevel1_KiB  },
                        {   0x66,       8,      &ProcessorInfo.lCPU_CacheDataLevel1_KiB  },
                        {   0x67,      16,      &ProcessorInfo.lCPU_CacheDataLevel1_KiB  },
                        {   0x68,      32,      &ProcessorInfo.lCPU_CacheDataLevel1_KiB  },

                        {   0x70,     -12,      &ProcessorInfo.lCPU_CacheCodeLevel1_KiB  }, //Trace cache / K-uops
                        {   0x71,     -16,      &ProcessorInfo.lCPU_CacheCodeLevel1_KiB  }, //Trace cache / K-uops
                        {   0x72,     -32,      &ProcessorInfo.lCPU_CacheCodeLevel1_KiB  }, //Trace cache / K-uops
                        {   0x73,     -64,      &ProcessorInfo.lCPU_CacheCodeLevel1_KiB  }, //Trace cache / K-uops

                        {   0x78,    1024,      &ProcessorInfo.lCPU_CacheDataLevel2_KiB  },
                        {   0x79,     128,      &ProcessorInfo.lCPU_CacheDataLevel2_KiB  },
                        {   0x7A,     256,      &ProcessorInfo.lCPU_CacheDataLevel2_KiB  },
                        {   0x7B,     512,      &ProcessorInfo.lCPU_CacheDataLevel2_KiB  },
                        {   0x7C,    1024,      &ProcessorInfo.lCPU_CacheDataLevel2_KiB  },
                        {   0x7D,    2048,      &ProcessorInfo.lCPU_CacheDataLevel2_KiB  },
                        {   0x7F,     512,      &ProcessorInfo.lCPU_CacheDataLevel2_KiB  },
                        {   0x82,     256,      &ProcessorInfo.lCPU_CacheDataLevel2_KiB  },
                        {   0x83,     512,      &ProcessorInfo.lCPU_CacheDataLevel2_KiB  },
                        {   0x84,    1024,      &ProcessorInfo.lCPU_CacheDataLevel2_KiB  },
                        {   0x85,    2048,      &ProcessorInfo.lCPU_CacheDataLevel2_KiB  },
                        {   0x86,     512,      &ProcessorInfo.lCPU_CacheDataLevel2_KiB  },
                        {   0x87,    1024,      &ProcessorInfo.lCPU_CacheDataLevel2_KiB  },

                        {     -1 } };


void CPUID_GetFeatures ( CPUID_FEATURESLIST *pFeatList )
{
    long lExtendedCPUID;
    DWORD dwCPUID_Fn;
    long lRegister;
    DWORD dwGR[4], dwMask, dwValue;
    long i;

    //Support de l'extended CPUID ?
    __asm { 
            push ebx
            pushfd
            mov eax, 0x80000000
            cpuid
            xor ebx, ebx
            cmp eax, 0x80000000
            setne bl
            mov lExtendedCPUID, ebx
            popfd
            pop ebx
    }

    dwCPUID_Fn = -1;
    for (i=0;pFeatList[i].dwFnCPUID!=0xFFFFFFFF;i++)
    {
        //On évite les appels multiples...
        if (dwCPUID_Fn!=pFeatList[i].dwFnCPUID)
        {
            dwCPUID_Fn = pFeatList[i].dwFnCPUID;

            __asm { 
                push ebx
                pushfd
                mov eax, dwCPUID_Fn
                xor ecx, ecx
                cpuid
                mov dword ptr dwGR,    eax
                mov dword ptr dwGR+4,  ebx
                mov dword ptr dwGR+8,  ecx
                mov dword ptr dwGR+12, edx
                popfd
                pop ebx
            }
        }

        if (strcmp(pFeatList[i].szRegister,"eax")==0) lRegister = 0;
        if (strcmp(pFeatList[i].szRegister,"ebx")==0) lRegister = 1;
        if (strcmp(pFeatList[i].szRegister,"ecx")==0) lRegister = 2;
        if (strcmp(pFeatList[i].szRegister,"edx")==0) lRegister = 3;

        if (pFeatList[i].lBitCount==32) dwMask = 0xFFFFFFFF;
        else dwMask =  ( 1 << pFeatList[i].lBitCount ) - 1;
        dwValue = ( dwGR[lRegister] >> pFeatList[i].lBitStart ) & dwMask;

        if (pFeatList[i].plOutToStore)
            *pFeatList[i].plOutToStore = dwValue;
    }
}
                            

  
void CPUID_Init ( void ) 
{
    long lSupportedCPUID;
    long i,j;

    if (lProcessorInit==0)
    {
        memset ( &ProcessorInfo, 0, sizeof(PROCESSOR_INFO) );
        lProcessorInit = 1;
    }
    else return;

    
    #ifndef _DEBUG
        //OSSetExclusiveMode ( 1 );
    #endif


    


    //Initialisation des chaines

    __asm {
            // Est-ce que l'instruction CPU-ID est présente
            push ebx
            pushfd
            pop       eax
            mov       ecx,eax
            xor       eax,0x200000
            push      eax
            popfd
            pushfd
            pop       eax
            cmp       ecx,eax
            mov       lSupportedCPUID, -1
            je        _opsys_get_cpu_type_no_cpuid
            mov       lSupportedCPUID, 0

        _opsys_get_cpu_type_no_cpuid:
            pop ebx
    }

    if (lSupportedCPUID==0)
    {
        CPUID_GetFeatures ( cpuidGlobalFeatList );

        //Ajustement pour les CPUs AMD
        if (memcmp(ProcessorInfo.szManufacturer,"AuthenticAMD",12)==0)
        {
            //Caractéristiques spécifiques
            CPUID_GetFeatures ( cpuid_AMD_FeatList );

            //LogicalProcessorCount, CmpLegacy, HTT and NC
            switch ((ProcessorInfo.lCPU_CmpLegacy<<1)|(ProcessorInfo.x86_HTT))
            {
                case 0:
                    ProcessorInfo.lCPU_LogicalCoreCount     = 1;
                    ProcessorInfo.lCPU_ThreadsPerCore       = 1;
                    ProcessorInfo.lCPU_CoresPerProcessor    = 1;
                    break;

                case 1:
                    ProcessorInfo.lCPU_CoresPerProcessor    = ProcessorInfo.lCPU_NC+1;
                    ProcessorInfo.lCPU_ThreadsPerCore       = ProcessorInfo.lCPU_LogicalCoreCount / ProcessorInfo.lCPU_CoresPerProcessor;
                    break;
                                        
                case 3:
                    ProcessorInfo.lCPU_CoresPerProcessor    = ProcessorInfo.lCPU_NC+1;
                    ProcessorInfo.lCPU_ThreadsPerCore       = ProcessorInfo.lCPU_LogicalCoreCount / ProcessorInfo.lCPU_CoresPerProcessor;
                    break;

                case 2:
                    ProcessorInfo.lCPU_CoresPerProcessor    = ProcessorInfo.lCPU_NC+1;
                    ProcessorInfo.lCPU_ThreadsPerCore       = ProcessorInfo.lCPU_LogicalCoreCount / ProcessorInfo.lCPU_CoresPerProcessor;
                    break;
            }

            if (ProcessorInfo.lCPU_ThreadsPerCore==1)
                ProcessorInfo.x86_HTT = 0;
        }

        //Flags Prefetch
        if (((ProcessorInfo.x86_SSE)||(ProcessorInfo.x86_SSE2)||(ProcessorInfo.x86_3DNow)))
            ProcessorInfo.x86_Prefetch = 1;

        //Ajustement pour les CPUs INTEL
        if (memcmp(ProcessorInfo.szManufacturer,"GenuineIntel",12)==0)
        {
            //Caractéristiques spécifiques
            CPUID_GetFeatures ( cpuid_INTEL_FeatList );

            //Gestion du TBL / Cache
            if (ProcessorInfo.lIntel_EnableEaxTLBC)
                for (i=1;i<4;i++) ProcessorInfo.lIntel_TLBC[i] = 0;

            if (ProcessorInfo.lIntel_EnableEbxTLBC)
                for (i=0;i<4;i++) ProcessorInfo.lIntel_TLBC[i+4] = 0;

            if (ProcessorInfo.lIntel_EnableEcxTLBC)
                for (i=0;i<4;i++) ProcessorInfo.lIntel_TLBC[i+8] = 0;

            if (ProcessorInfo.lIntel_EnableEdxTLBC)
                for (i=0;i<4;i++) ProcessorInfo.lIntel_TLBC[i+12] = 0;

            for (j=1;j<16;j++)
            {
                for (i=0;vd_INTEL_TBLC[i].lDetectedValue!=-1;i++)
                {
                    if (vd_INTEL_TBLC[i].lDetectedValue==ProcessorInfo.lIntel_TLBC[j])
                    {
                        if (vd_INTEL_TBLC[i].plObjectToWrite)
                        {
                            *vd_INTEL_TBLC[i].plObjectToWrite = vd_INTEL_TBLC[i].lWriteValue;
                        }
                    }
                }
            }

            ProcessorInfo.lCPU_CoresPerProcessor        += 1;

            if (ProcessorInfo.x86_HTT)
            {
                if (ProcessorInfo.lCPU_LogicalCoreCount>ProcessorInfo.lCPU_CoresPerProcessor)
                {
                    ProcessorInfo.lCPU_ThreadsPerCore = ProcessorInfo.lCPU_LogicalCoreCount / ProcessorInfo.lCPU_CoresPerProcessor;
                }
                else
                {
                    ProcessorInfo.x86_HTT = 0;
                    ProcessorInfo.lCPU_ThreadsPerCore = 1;
                }
            }
            else
            {
                ProcessorInfo.lCPU_ThreadsPerCore       = 1;
                ProcessorInfo.lCPU_CoresPerProcessor    = 1;
            }

            //Résolution de l'ambiguité L2/L3
            if (ProcessorInfo.lIntel_NeedToResolveL2L3_0x49)
            {
                if ((ProcessorInfo.lCPU_Family==0x0F)&&(ProcessorInfo.lCPU_Model==0x06))
                    ProcessorInfo.lCPU_CacheDataLevel3_KiB = 4096;
                else
                    ProcessorInfo.lCPU_CacheDataLevel2_KiB = 4096;
            }

            if (ProcessorInfo.lIntel_NeedToResolveL2L3_0x40)
                ProcessorInfo.lCPU_CacheDataLevel3_KiB = 0;
            
        }
        
        //La famille et le modèle
        ProcessorInfo.lCPU_ExtFamily = (ProcessorInfo.lCPU_ExtFamily<<4) + ProcessorInfo.lCPU_Family;
        ProcessorInfo.lCPU_ExtModel  = (ProcessorInfo.lCPU_ExtModel<<4)  + ProcessorInfo.lCPU_Model;

        //Le nom du processeur
        //strremovedupp ( ProcessorInfo.szDesignation, 32 );

        //Génération
        ProcessorInfo.lGeneration    = ProcessorInfo.lCPU_Family;

        //TSC invalide
        ProcessorInfo.lInvalidTSC    = 0;

        //Designation
        /*
        i = 0;
        do {
            //Test du Vendor
            if (strcmp(szCPUVendor,ProcessorList[i].szVendor)==0)
            {
                if ( ((ProcessorList[i].lFamily!=-1)&&(ProcessorList[i].lFamily==lCPUFamily)) || (ProcessorList[i].lFamily==-1) )
                {
                    if ( ((ProcessorList[i].lModel!=-1)&&(ProcessorList[i].lModel==lCPUModel)) || (ProcessorList[i].lModel==-1) )
                    {
                        if ( ((ProcessorList[i].lStepping!=-1)&&(ProcessorList[i].lStepping==lCPUStepping)) || (ProcessorList[i].lStepping==-1) )
                        {
                            if ( ((ProcessorList[i].lType!=-1)&&(ProcessorList[i].lType==lCPUType)) || (ProcessorList[i].lType==-1) )
                            {
                                //strcpy ( ProcessorInfo.szDesignation, ProcessorList[i].szDesignation );
                                ProcessorInfo.bInvalidTSC    = ProcessorList[i].bInvalidTSC;
                                //ProcessorInfo.lCacheSize[0]  = ProcessorList[i].lCacheL1Size<<10;
                                //ProcessorInfo.lCacheSize[1]  = ProcessorList[i].lCacheL2Size<<10;
                                i = -2;
                            }
                        }
                    }
                }
            }
            else if (ProcessorList[i].szVendor[0]==0)
            {
                //strcpy ( ProcessorInfo.szDesignation, szCPUVendor );
                i = -3;
            }
            i++;
        } while (i>0);
        */
        //Remplissage de la spécification
        //ProcessorInfo.lCPU_Type     =   lCPUType;
        /*ProcessorInfo.lCPU_Family   =   lCPUFamily;
        ProcessorInfo.lCPU_Model    =   lCPUModel;
        ProcessorInfo.lCPU_Stepping =   lCPUStepping;*/

        //Nom du processeur
        //if (strlen(szCPUName)) strcpy ( ProcessorInfo.szDesignation, szCPUName );




        /*if (lMMX==1)    strcat ( pmProcessor->szSpec, " MMX" );
        if (lSSE==1)    strcat ( pmProcessor->szSpec, " SSE" );
        if (lSSE2==1)   strcat ( pmProcessor->szSpec, " SSE2" );
        if (l3DNOW==1)  strcat ( pmProcessor->szSpec, " 3DNow!" );
        if (lCMOV==1)   strcat ( pmProcessor->szSpec, " CMOV" );*/
    }
        




    #ifndef _DEBUG
        //OSSetExclusiveMode ( 0 );
    #endif
    return;
}


long    CPUID_Get ( long lSpecificationType, void * pReturnedSpecification )
{
    if (!pReturnedSpecification) return -1;

    CPUID_Init ( );

    switch (lSpecificationType)
    {
        case CPUID_GET_FAMILY:
            *((long *)pReturnedSpecification) = ProcessorInfo.lCPU_Family;
            break;

        case CPUID_GET_MODEL:
            *((long *)pReturnedSpecification) = ProcessorInfo.lCPU_Model;
            break;

        case CPUID_GET_STEPPING:
            *((long *)pReturnedSpecification) = ProcessorInfo.lCPU_Stepping;
            break;

        case CPUID_GET_EXTFAMILY:
            *((long *)pReturnedSpecification) = ProcessorInfo.lCPU_ExtFamily;
            break;

        case CPUID_GET_EXTMODEL:
            *((long *)pReturnedSpecification) = ProcessorInfo.lCPU_ExtModel;
            break;


        case CPUID_GET_MMX:
            *((long *)pReturnedSpecification) = ProcessorInfo.x86_MMX;
            break;

        case CPUID_GET_SSE:
            *((long *)pReturnedSpecification) = ProcessorInfo.x86_SSE;
            break;

        case CPUID_GET_SSE2:
            *((long *)pReturnedSpecification) = ProcessorInfo.x86_SSE2;
            break;

        case CPUID_GET_SSE3:
            *((long *)pReturnedSpecification) = ProcessorInfo.x86_SSE3;
            break;

        case CPUID_GET_SSSE3:
            *((long *)pReturnedSpecification) = ProcessorInfo.x86_SSSE3;
            break;

        case CPUID_GET_X64:
            *((long *)pReturnedSpecification) = ProcessorInfo.x86_X64;
            break;

        case CPUID_GET_HTT:
            *((long *)pReturnedSpecification) = ProcessorInfo.x86_HTT;
            break;

        case CPUID_GET_VT:
            *((long *)pReturnedSpecification) = ProcessorInfo.x86_VT;
            break;

        case CPUID_GET_NX:
            *((long *)pReturnedSpecification) = ProcessorInfo.x86_NX;
            break;

        case CPUID_GET_3DNOW:
            *((long *)pReturnedSpecification) = ProcessorInfo.x86_3DNow;
            break;

        case CPUID_GET_3DNOWEXT:
            *((long *)pReturnedSpecification) = ProcessorInfo.x86_3DNowExtended;
            break;

        case CPUID_GET_MSR:
            *((long *)pReturnedSpecification) = ProcessorInfo.x86_MSR;
            break;

        case CPUID_GET_VENDOR_STR:
            strcpy ( ((char *)pReturnedSpecification), ProcessorInfo.szManufacturer );
            break;

        case CPUID_GET_NAME_STR:
            strcpy ( ((char *)pReturnedSpecification), ProcessorInfo.szDesignation );
            break;

        case CPUID_GET_CACHEDATAL1:
            *((long *)pReturnedSpecification) = ProcessorInfo.lCPU_CacheDataLevel1_KiB;
            break;

        case CPUID_GET_CACHECODEL1:
            *((long *)pReturnedSpecification) = ProcessorInfo.lCPU_CacheCodeLevel1_KiB;
            break;

        case CPUID_GET_CACHEDATAL2:
            *((long *)pReturnedSpecification) = ProcessorInfo.lCPU_CacheDataLevel2_KiB;
            break;

        case CPUID_GET_CACHEDATAL3:
            *((long *)pReturnedSpecification) = ProcessorInfo.lCPU_CacheDataLevel3_KiB;
            break;

        case CPUID_GET_LOCALAPICID:
            *((long *)pReturnedSpecification) = ProcessorInfo.lCPU_LocalApicID;
            break;

        case CPUID_GET_LOGICALCORECOUNT:
            *((long *)pReturnedSpecification) = ProcessorInfo.lCPU_LogicalCoreCount;
            break;

        case CPUID_GET_THREADSPERCORE:
            *((long *)pReturnedSpecification) = ProcessorInfo.lCPU_ThreadsPerCore;
            break;

        case CPUID_GET_CORESPERPROCESSOR:
            *((long *)pReturnedSpecification) = ProcessorInfo.lCPU_CoresPerProcessor;
            break;

        case CPUID_GET_DTS_INTEL:
            *((long *)pReturnedSpecification) = ProcessorInfo.lIntel_DTS;
            break;

        case CPUID_GET_DTS_AMD:
            *((long *)pReturnedSpecification) = ProcessorInfo.lAMD_DTS;
            break;

        case CPUID_GET_DIODE_SENSOR:
            *((long *)pReturnedSpecification) = 0;
            break;

        case CPUID_GET_ACPI:
            *((long *)pReturnedSpecification) = ProcessorInfo.x86_ACPI;
            break;

        default:
            break;
    }

    return 0;
}
