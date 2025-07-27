#include "sam_header.h"
#include "sam_data.h"


                        SAM_DATA        samData;

sam_ALIGN_SPEC(32)      FLOAT32         samData_f32InterpolationData_1024_16[1024*16];



void SAM_DATA_SaveState ( SAM_DATASTATE * psamDataState )
{
    psamDataState->lWithoutStackMode                                = samData.lWithoutStackMode;
    psamDataState->dwOutputHardwareChannel                          = samData.dwOutputHardwareChannel;
    psamDataState->dwOutputSoftwareChannel                          = samData.dwOutputSoftwareChannel;
    psamDataState->dwOutputEncoder                                  = samData.dwOutputEncoder;
    psamDataState->dwChannelMode                                    = samData.dwChannelMode;
    psamDataState->dwHardwaremixSampleRate                          = samData.dwHardwaremixSampleRate;
    psamDataState->dwHardwareBufferLatencySamplesCount              = samData.dwHardwareBufferLatencySamplesCount;
    psamDataState->dwHardwareAndSoftwareBufferSamplesCount          = samData.dwHardwareAndSoftwareBufferSamplesCount;
    psamDataState->dwDeviceSelect                                   = samData.dwHardwareDeviceSelected;
}

void SAM_DATA_RestoreState ( SAM_DATASTATE * psamDataState )
{
    samData.lWithoutStackMode                                = psamDataState->lWithoutStackMode;
    samData.dwOutputHardwareChannel                          = psamDataState->dwOutputHardwareChannel;
    samData.dwOutputSoftwareChannel                          = psamDataState->dwOutputSoftwareChannel;
    samData.dwOutputEncoder                                  = psamDataState->dwOutputEncoder;
    samData.dwChannelMode                                    = psamDataState->dwChannelMode;
    samData.dwHardwaremixSampleRate                          = psamDataState->dwHardwaremixSampleRate;
    samData.dwHardwareBufferLatencySamplesCount              = psamDataState->dwHardwareBufferLatencySamplesCount;
    samData.dwHardwareAndSoftwareBufferSamplesCount          = psamDataState->dwHardwareAndSoftwareBufferSamplesCount;
    samData.dwHardwareDeviceSelected                         = psamDataState->dwDeviceSelect;
}
