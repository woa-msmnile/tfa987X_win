/*++

Module Name:

    device.h

Abstract:

    This file contains the device definitions.

Environment:

    Kernel-mode Driver Framework

--*/

#include "public.h"

EXTERN_C_START

//
// The device context performs the same job as
// a WDM device extension in the driver frameworks
//
typedef struct _DEVICE_CONTEXT
{
    WDFDEVICE               Device;
    WDFINTERRUPT            Interrupt;
    SPB_CONTEXT             SpbContext;

    PCALLBACK_OBJECT        AmpAudioAPICallback;
    PVOID                   AmpAudioAPICallbackObj;
    BOOLEAN                 AmpAudioManaged;
} DEVICE_CONTEXT, *PDEVICE_CONTEXT;

//
// This macro will generate an inline function called DeviceGetContext
// which will be used to get a pointer to the device context memory
// in a type safe manner.
//
WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(DEVICE_CONTEXT, DeviceGetContext)

//
// Function to initialize the device and its callbacks
//
NTSTATUS
TFA987xampCreateDevice(
    _Inout_ PWDFDEVICE_INIT DeviceInit
    );

EVT_WDF_DEVICE_D0_ENTRY OnD0Entry;

EVT_WDF_DEVICE_D0_EXIT OnD0Exit;

EVT_WDF_INTERRUPT_ISR OnInterruptIsr;

EVT_WDF_DEVICE_PREPARE_HARDWARE OnPrepareHardware;

EVT_WDF_DEVICE_RELEASE_HARDWARE OnReleaseHardware;
EXTERN_C_END
