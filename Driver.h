/*++

Module Name:

    driver.h

Abstract:

    This file contains the driver definitions.

Environment:

    Kernel-mode Driver Framework

--*/

#include <ntddk.h>
#include <wdf.h>
#include <initguid.h>
#include <acpiioct.h>
#include <ntstrsafe.h>

#include "spb.h"
#include "device.h"
#include "queue.h"
#include "trace.h"

EXTERN_C_START

//
// WDFDRIVER Events
//

DRIVER_INITIALIZE DriverEntry;
EVT_WDF_DRIVER_DEVICE_ADD TFA987xampEvtDeviceAdd;
EVT_WDF_OBJECT_CONTEXT_CLEANUP TFA987xampEvtDriverContextCleanup;

BOOLEAN
OnInterruptIsr(
    _In_  WDFINTERRUPT FxInterrupt,
    _In_  ULONG        MessageID
);

NTSTATUS
StartAmp(PDEVICE_CONTEXT pDevice);

NTSTATUS
StopAmp(PDEVICE_CONTEXT pDevice);

NTSTATUS
GetDeviceUID(
    _In_ WDFDEVICE FxDevice,
    _In_ PINT32 PUID
);

#define AMP_POOL_TAG    (ULONG) 'TFA9'
EXTERN_C_END