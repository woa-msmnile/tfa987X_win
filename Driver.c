/*++

Module Name:

    driver.c

Abstract:

    This file contains the driver entry points and callbacks.

Environment:

    Kernel-mode Driver Framework

--*/

#include "driver.h"
#include "driver.tmh"

#ifdef ALLOC_PRAGMA
#pragma alloc_text (INIT, DriverEntry)
#pragma alloc_text (PAGE, TFA987xampEvtDeviceAdd)
#pragma alloc_text (PAGE, TFA987xampEvtDriverContextCleanup)
#endif

NTSTATUS
OnPrepareHardware(
    _In_  WDFDEVICE     FxDevice,
    _In_  WDFCMRESLIST  FxResourcesRaw,
    _In_  WDFCMRESLIST  FxResourcesTranslated
)
/*++

    Routine Description:

    This routine caches the SPB resource connection ID.

    Arguments:

    FxDevice - a handle to the framework device object
    FxResourcesRaw - list of translated hardware resources that
        the PnP manager has assigned to the device
    FxResourcesTranslated - list of raw hardware resources that
        the PnP manager has assigned to the device

    Return Value:

    Status

--*/
{
    PDEVICE_CONTEXT pDevice = DeviceGetContext(FxDevice);
    BOOLEAN fSpbResourceFound = FALSE;
    BOOLEAN fInterruptResourceFound = FALSE;
    ULONG interruptIndex = 0;
    NTSTATUS status = STATUS_SUCCESS;

    UNREFERENCED_PARAMETER(FxResourcesRaw);

    //
    // Parse the peripheral's resources.
    //

    ULONG resourceCount = WdfCmResourceListGetCount(FxResourcesTranslated);

    for (ULONG i = 0; i < resourceCount; i++)
    {
        PCM_PARTIAL_RESOURCE_DESCRIPTOR pDescriptor;
        UCHAR Class;
        UCHAR Type;

        pDescriptor = WdfCmResourceListGetDescriptor(
            FxResourcesTranslated, i);

        switch (pDescriptor->Type)
        {
        case CmResourceTypeConnection:

            //
            // Look for I2C or SPI resource and save connection ID.
            //

            Class = pDescriptor->u.Connection.Class;
            Type = pDescriptor->u.Connection.Type;

            if ((Class == CM_RESOURCE_CONNECTION_CLASS_SERIAL) &&
                ((Type == CM_RESOURCE_CONNECTION_TYPE_SERIAL_I2C)))
            {
                if (fSpbResourceFound == FALSE)
                {
                    pDevice->SpbContext.I2cResHubId.LowPart =
                        pDescriptor->u.Connection.IdLowPart;
                    pDevice->SpbContext.I2cResHubId.HighPart =
                        pDescriptor->u.Connection.IdHighPart;

                    fSpbResourceFound = TRUE;
                }
            }

            break;

        case CmResourceTypeInterrupt:

            if (fInterruptResourceFound == FALSE)
            {
                fInterruptResourceFound = TRUE;
                interruptIndex = i;
            }
            break;

        default:

            //
            // Ignoring all other resource types.
            //

            break;
        }
    }

    //
    // An SPB resource is required.
    //

    if (fSpbResourceFound == FALSE)
    {
        status = STATUS_NOT_FOUND;
    }

    //
    // Create the interrupt if an interrupt
    // resource was found.
    //

    if (NT_SUCCESS(status))
    {
        if (fInterruptResourceFound == TRUE)
        {
            WDF_INTERRUPT_CONFIG interruptConfig;
            WDF_INTERRUPT_CONFIG_INIT(
                &interruptConfig,
                OnInterruptIsr,
                NULL);

            interruptConfig.PassiveHandling = TRUE;
            interruptConfig.InterruptTranslated = WdfCmResourceListGetDescriptor(
                FxResourcesTranslated,
                interruptIndex);
            interruptConfig.InterruptRaw = WdfCmResourceListGetDescriptor(
                FxResourcesRaw,
                interruptIndex);

            //status = WdfInterruptCreate(
            //    pDevice->Device,
            //    &interruptConfig,
            //    WDF_NO_OBJECT_ATTRIBUTES,
            //    &pDevice->Interrupt);
        }
    }
    status = SpbTargetInitialize(FxDevice, &pDevice->SpbContext);
    return status;
}

BOOLEAN
OnInterruptIsr(
    _In_  WDFINTERRUPT FxInterrupt,
    _In_  ULONG        MessageID
) {
    UNREFERENCED_PARAMETER(FxInterrupt);
    UNREFERENCED_PARAMETER(MessageID);

    BOOLEAN fInterruptRecognized = TRUE;
    return fInterruptRecognized;
}

NTSTATUS
OnReleaseHardware(
    _In_  WDFDEVICE     FxDevice,
    _In_  WDFCMRESLIST  FxResourcesTranslated
)
/*++

    Routine Description:

    Arguments:

    FxDevice - a handle to the framework device object
    FxResourcesTranslated - list of raw hardware resources that
        the PnP manager has assigned to the device

    Return Value:

    Status

--*/
{
    PDEVICE_CONTEXT pDevice = DeviceGetContext(FxDevice);
    NTSTATUS status = STATUS_SUCCESS;

    UNREFERENCED_PARAMETER(FxResourcesTranslated);

    //if (pDevice->Interrupt != NULL)
    //{
    //    WdfObjectDelete(pDevice->Interrupt);
    //}
    SpbTargetDeinitialize (FxDevice, &pDevice->SpbContext);

    if (pDevice->AmpAudioAPICallbackObj) {
        ExUnregisterCallback(pDevice->AmpAudioAPICallbackObj);
        pDevice->AmpAudioAPICallbackObj = NULL;
    }

    if (pDevice->AmpAudioAPICallback) {
        ObfDereferenceObject(pDevice->AmpAudioAPICallback);
        pDevice->AmpAudioAPICallback = NULL;
    }

    return status;
}

NTSTATUS
DriverEntry(
    _In_ PDRIVER_OBJECT  DriverObject,
    _In_ PUNICODE_STRING RegistryPath
    )
/*++

Routine Description:
    DriverEntry initializes the driver and is the first routine called by the
    system after the driver is loaded. DriverEntry specifies the other entry
    points in the function driver, such as EvtDevice and DriverUnload.

Parameters Description:

    DriverObject - represents the instance of the function driver that is loaded
    into memory. DriverEntry must initialize members of DriverObject before it
    returns to the caller. DriverObject is allocated by the system before the
    driver is loaded, and it is released by the system after the system unloads
    the function driver from memory.

    RegistryPath - represents the driver specific path in the Registry.
    The function driver can use the path to store driver related data between
    reboots. The path does not store hardware instance specific data.

Return Value:

    STATUS_SUCCESS if successful,
    STATUS_UNSUCCESSFUL otherwise.

--*/
{
    WDF_DRIVER_CONFIG config;
    NTSTATUS status;
    WDF_OBJECT_ATTRIBUTES attributes;

    //
    // Initialize WPP Tracing
    //
    WPP_INIT_TRACING(DriverObject, RegistryPath);

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "%!FUNC! Entry");

    //
    // Register a cleanup callback so that we can call WPP_CLEANUP when
    // the framework driver object is deleted during driver unload.
    //
    WDF_OBJECT_ATTRIBUTES_INIT(&attributes);
    attributes.EvtCleanupCallback = TFA987xampEvtDriverContextCleanup;

    WDF_DRIVER_CONFIG_INIT(&config,
                           TFA987xampEvtDeviceAdd
                           );

    status = WdfDriverCreate(DriverObject,
                             RegistryPath,
                             &attributes,
                             &config,
                             WDF_NO_HANDLE
                             );

    if (!NT_SUCCESS(status)) {
        TraceEvents(TRACE_LEVEL_ERROR, TRACE_DRIVER, "WdfDriverCreate failed %!STATUS!", status);
        WPP_CLEANUP(DriverObject);
        return status;
    }

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "%!FUNC! Exit");

    return status;
}

NTSTATUS
TFA987xampEvtDeviceAdd(
    _In_    WDFDRIVER       Driver,
    _Inout_ PWDFDEVICE_INIT DeviceInit
    )
/*++
Routine Description:

    EvtDeviceAdd is called by the framework in response to AddDevice
    call from the PnP manager. We create and initialize a device object to
    represent a new instance of the device.

Arguments:

    Driver - Handle to a framework driver object created in DriverEntry

    DeviceInit - Pointer to a framework-allocated WDFDEVICE_INIT structure.

Return Value:

    NTSTATUS

--*/
{
    NTSTATUS status;

    UNREFERENCED_PARAMETER(Driver);

    PAGED_CODE();

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "%!FUNC! Entry");

    status = TFA987xampCreateDevice(DeviceInit);


    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "%!FUNC! Exit");

    return status;
}

VOID
TFA987xampEvtDriverContextCleanup(
    _In_ WDFOBJECT DriverObject
    )
/*++
Routine Description:

    Free all the resources allocated in DriverEntry.

Arguments:

    DriverObject - handle to a WDF Driver object.

Return Value:

    VOID.

--*/
{
    UNREFERENCED_PARAMETER(DriverObject);

    PAGED_CODE();

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "%!FUNC! Entry");

    //
    // Stop WPP Tracing
    //
    WPP_CLEANUP(WdfDriverWdmGetDriverObject((WDFDRIVER)DriverObject));
}
