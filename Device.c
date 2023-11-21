/*++

Module Name:

    device.c - Device handling events for example driver.

Abstract:

   This file contains the device entry points and callbacks.
    
Environment:

    Kernel-mode Driver Framework

--*/

#include "driver.h"
#include "device.tmh"

#ifdef ALLOC_PRAGMA
#pragma alloc_text (PAGE, TFA987xampCreateDevice)
#endif

NTSTATUS
UpdateBits(
    PDEVICE_CONTEXT pDevice,
    UINT8 reg,
    UINT16 mask,
    UINT16 val
){
    NTSTATUS ret = 0;
    UINT16 tmp = 0, orig = 0;

    ret = SpbReadDataSynchronously(&pDevice->SpbContext, reg, &orig, sizeof(orig));
    orig = _byteswap_ushort(orig);
    TraceEvents(TRACE_LEVEL_ERROR, TRACE_DEVICE, "%!FUNC! Read Reg 0x%04x: 0x%04x", reg, orig);

    tmp = orig & ~mask;
    tmp |= val & mask;

    tmp = _byteswap_ushort(tmp);
    ret = SpbWriteDataSynchronously(&pDevice->SpbContext, reg, &tmp, sizeof(tmp));

    if (NT_SUCCESS(ret)) {
        //TraceEvents(TRACE_LEVEL_ERROR, TRACE_DEVICE, "%!FUNC! Successfully updated reg: 0x%04x with mask: 0x%04x and val: 0x%04x! %!STATUS!", reg, mask, val, ret);
        SpbReadDataSynchronously(&pDevice->SpbContext, reg, &tmp, 2);
        tmp = _byteswap_ushort(tmp);
        TraceEvents(TRACE_LEVEL_ERROR, TRACE_DEVICE, "%!FUNC! Reg 0x%04x: 0x%04x", reg, tmp);
    }
    else {
        //TraceEvents(TRACE_LEVEL_ERROR, TRACE_DEVICE, "%!FUNC! Failed to update reg: 0x%04x with mask: 0x%04x and val: 0x%04x! %!STATUS!", reg, mask, val, ret);
    }

  
    return ret;
}

NTSTATUS
StartAmp(PDEVICE_CONTEXT pDevice) {
    NTSTATUS status = STATUS_SUCCESS;
    UINT16 rev = 0;
    UINT16 data = 0;
    //UINT8 tmp = 0;
    // Try to Read Version
    TraceEvents(TRACE_LEVEL_ERROR, TRACE_DEVICE, "Starting TFA Device!");

    SpbReadDataSynchronously(&pDevice->SpbContext, 0x03, &rev, 2);
    rev = _byteswap_ushort(rev);
    TraceEvents(TRACE_LEVEL_ERROR, TRACE_DEVICE, "Revision 0x%04x", rev);

    switch (rev) {
        case 0x1b72:
        case 0x2b72:
        case 0x3b72:
            TraceEvents(TRACE_LEVEL_ERROR, TRACE_DEVICE, "Found TFA9872.");
            break;
        case 0x0c74:
            TraceEvents(TRACE_LEVEL_ERROR, TRACE_DEVICE, "Found TFA9874.");
            break;
        default:
            TraceEvents(TRACE_LEVEL_ERROR, TRACE_DEVICE, "Not TFA987x device ! Exiting...");
            //return STATUS_INVALID_PARAMETER;
            break;
    }
    
    /*
    * All the registers needs to be written:
    *   reg     val_h   val_l
    *   0x00    0x00    0x02        // soft reset it.
    *   0x20    0x08    0x90
    *   0x21    0xc0    0xf1
    *   0x00    0x00    0x00
    *   0x01    0x00    0x04
    *   
    * After that, Read 0x0,
    *    if result is  0x00 0x08, that should mean TFA987x works?
    * 
    * Write 0x0 to register 0x0 to mute the amp.
    * 
    */

    /* Perform soft reset */
    data = 0x0200;
    status |= SpbWriteDataSynchronously(&pDevice->SpbContext, 0x00, &data, sizeof(data));
    
    SpbReadDataSynchronously(&pDevice->SpbContext, 0x21, &data, 2);
    data = _byteswap_ushort(data);
    TraceEvents(TRACE_LEVEL_ERROR, TRACE_DEVICE, "Read 0x21: 0x%04x", data);


    /* Setup DC-DC Converter if we have configuration */
    status |= UpdateBits(pDevice, 0x00, 0x0010, 0x0000);
    
    /* Disable DPSA */
    status |= UpdateBits(pDevice, 0x51, 0x0080, 0x0000);
    
    /* Setup TDM 16 bit 1 slot config */
    status |= UpdateBits(pDevice, 0x20, 0xf000, 0x0000);
    status |= UpdateBits(pDevice, 0x21, 0x01ff, 0x00f1);
    status |= UpdateBits(pDevice, 0x22, 0x007c, 0x003c);
    
    /* No current/voltage sense over TDM */
    status |= UpdateBits(pDevice, 0x23, 0x001b, 0x0001);
    status |= UpdateBits(pDevice, 0x26, 0x000f, 0x0000);
    //status |= UpdateBits(pDevice, 0x64, 0xc000, 0x4000);
    
    /* Turn on this thing */
    status |= UpdateBits(pDevice, 0x00, 0x0001, 0x0000);
    status |= UpdateBits(pDevice, 0x01, 0x0004, 0x0004);
    
    /* Re-check SYS CTRL Reg */
    // Actually, it get 0x0 here, so bypass check.
    //SpbReadDataSynchronously(&pDevice->SpbContext, 0x00, &data, 2);
    //data = _byteswap_ushort(data);
    //if(data != 8)
    //    TraceEvents(TRACE_LEVEL_ERROR, TRACE_DEVICE, "Failed to Start Device! Reg 0x0: 0x%04x, \
    //    status: %!STATUS!", data, status);
    //else
    //    TraceEvents(TRACE_LEVEL_ERROR, TRACE_DEVICE, "Successfully Started Device! %!STATUS!", status);

    return status;
}

NTSTATUS
StopAmp(PDEVICE_CONTEXT pDevice) {
    //
    // Mute Amplifier
    //
    NTSTATUS status;

    status = UpdateBits(pDevice, 0x0000, 0x0008, 0x0000);
    TraceEvents(TRACE_LEVEL_ERROR, TRACE_DEVICE, "Muted Device! Status: %!STATUS!", status);
    //
    // Stop Spb Controller
    //
    //WdfInterruptDisable(pDevice->Interrupt);
    //WdfIoTargetClose(pDevice->SpbContext.SpbIoTarget);
    return status;
}

NTSTATUS
OnD0Entry(
    _In_  WDFDEVICE               FxDevice,
    _In_  WDF_POWER_DEVICE_STATE  FxPreviousState
)
/*++

Routine Description:

This routine allocates objects needed by the driver.

Arguments:

FxDevice - a handle to the framework device object
FxPreviousState - previous power state

Return Value:

Status

--*/
{
    UNREFERENCED_PARAMETER(FxPreviousState);
    PDEVICE_CONTEXT pDevice = DeviceGetContext(FxDevice);
    NTSTATUS status = STATUS_SUCCESS;
    TraceEvents(TRACE_LEVEL_ERROR, TRACE_DEVICE, "%!FUNC! Entering.");

    //
    // Create the SPB target.
    //

    WDF_OBJECT_ATTRIBUTES targetAttributes;
    WDF_OBJECT_ATTRIBUTES_INIT(&targetAttributes);

    //status = WdfIoTargetCreate(
    //    pDevice->Device,
    //    &targetAttributes,
    //    &pDevice->SpbContext.SpbIoTarget);

    StartAmp(pDevice);
    TraceEvents(TRACE_LEVEL_ERROR, TRACE_DEVICE, "%!FUNC! Leaving.");
    return status;
}

NTSTATUS
OnD0Exit(
    _In_  WDFDEVICE               FxDevice,
    _In_  WDF_POWER_DEVICE_STATE  FxPreviousState
)
/*++

Routine Description:

This routine destroys objects needed by the driver.

Arguments:

FxDevice - a handle to the framework device object
FxPreviousState - previous power state

Return Value:

Status

--*/
{
    UNREFERENCED_PARAMETER(FxPreviousState);

    PDEVICE_CONTEXT pDevice = DeviceGetContext(FxDevice);
    NTSTATUS status = STATUS_SUCCESS;
    TraceEvents(TRACE_LEVEL_ERROR, TRACE_DEVICE, "%!FUNC! Entering.");

    StopAmp(pDevice);

    if (pDevice->SpbContext.SpbIoTarget != WDF_NO_HANDLE)
    {
        WdfObjectDelete(pDevice->SpbContext.SpbIoTarget);
        pDevice->SpbContext.SpbIoTarget = WDF_NO_HANDLE;
    }
    TraceEvents(TRACE_LEVEL_ERROR, TRACE_DEVICE, "%!FUNC! Leaving.");

    return status;
}

NTSTATUS
TFA987xampCreateDevice(
    _Inout_ PWDFDEVICE_INIT DeviceInit
    )
/*++

Routine Description:

    Worker routine called to create a device and its software resources.

Arguments:

    DeviceInit - Pointer to an opaque init structure. Memory for this
                    structure will be freed by the framework when the WdfDeviceCreate
                    succeeds. So don't access the structure after that point.

Return Value:

    NTSTATUS

--*/
{
    WDF_OBJECT_ATTRIBUTES deviceAttributes;
    PDEVICE_CONTEXT deviceContext;
    WDFDEVICE device;
    NTSTATUS status;

    PAGED_CODE();
    TraceEvents(TRACE_LEVEL_ERROR, TRACE_DEVICE, "%!FUNC! Entering.");

    
    {
        TraceEvents(TRACE_LEVEL_ERROR, TRACE_DEVICE, "Registering PnpCallbacks.");
        WDF_PNPPOWER_EVENT_CALLBACKS pnpCallbacks;
        WDF_PNPPOWER_EVENT_CALLBACKS_INIT(&pnpCallbacks);
        pnpCallbacks.EvtDevicePrepareHardware = OnPrepareHardware;
        pnpCallbacks.EvtDeviceReleaseHardware = OnReleaseHardware;
        //pnpCallbacks.EvtDeviceSelfManagedIoInit = OnSelfManagedIoInit;
        pnpCallbacks.EvtDeviceD0Entry = OnD0Entry;
        pnpCallbacks.EvtDeviceD0Exit = OnD0Exit;
        WdfDeviceInitSetPnpPowerEventCallbacks(DeviceInit, &pnpCallbacks);
    }


    WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(&deviceAttributes, DEVICE_CONTEXT);

    status = WdfDeviceCreate(&DeviceInit, &deviceAttributes, &device);

    if (NT_SUCCESS(status)) {
        //
        // Get a pointer to the device context structure that we just associated
        // with the device object. We define this structure in the device.h
        // header file. DeviceGetContext is an inline function generated by
        // using the WDF_DECLARE_CONTEXT_TYPE_WITH_NAME macro in device.h.
        // This function will do the type checking and return the device context.
        // If you pass a wrong object handle it will return NULL and assert if
        // run under framework verifier mode.
        //

        {
            WDF_DEVICE_STATE deviceState;
            WDF_DEVICE_STATE_INIT(&deviceState);

            deviceState.NotDisableable = WdfFalse;
            WdfDeviceSetDeviceState(device, &deviceState);
        }

        deviceContext = DeviceGetContext(device);

        //
        // Initialize the context.
        //
        //deviceContext->PrivateDeviceData = 0;

        //
        // Create a device interface so that applications can find and talk
        // to us.
        //
        status = WdfDeviceCreateDeviceInterface(
            device,
            &GUID_DEVINTERFACE_TFA987xamp,
            NULL // ReferenceString
            );

        if (NT_SUCCESS(status)) {
            //
            // Initialize the I/O Package and any Queues
            //
            status = TFA987xampQueueInitialize(device);
        }
    }
    TraceEvents(TRACE_LEVEL_ERROR, TRACE_DEVICE, "%!FUNC! Leaving.");
    return status;
}
