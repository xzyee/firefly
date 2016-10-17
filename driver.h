//------------------------------------------
//    FireFly.h
//------------------------------------------
#include <ntddk.h>
#include <wdf.h>
#define NTSTRSAFE_LIB
#include <ntstrsafe.h>
#include <initguid.h>
#include <wdmguid.h>

//
// Our drivers generated include from firefly.mof
// See makefile.inc for wmi commands
//
#include "fireflymof.h"

// Our drivers modules includes
#include "device.h"
#include "wmi.h"
#include "vfeature.h"
#include "magic.h"

DRIVER_INITIALIZE DriverEntry;

EVT_WDF_DRIVER_DEVICE_ADD FireFlyEvtDeviceAdd;

//------------------------------------------
//  device.h
//------------------------------------------
//
// The device context performs the same job as
// a WDM device extension in the driver framework
//
typedef struct _DEVICE_CONTEXT
{
    // Our WMI data generated from firefly.mof
    FireflyDeviceInformation WmiInstance;

    UNICODE_STRING PdoName;

} DEVICE_CONTEXT, *PDEVICE_CONTEXT;

WDF_DECLARE_CONTEXT_TYPE(DEVICE_CONTEXT)

WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(FireflyDeviceInformation, InstanceGetInfo)

EVT_WDF_DEVICE_CONTEXT_CLEANUP EvtDeviceContextCleanup;

//------------------------------------------
//  vfeature.h
//------------------------------------------
#if !defined(_VFEATURE_H_)
#define _VFEATURE_H_

NTSTATUS
FireflySetFeature(
    IN  PDEVICE_CONTEXT DeviceContext,
    IN  UCHAR           PageId,
    IN  USHORT          FeatureId,
    IN  BOOLEAN         EnableFeature
    );

#endif // _VFEATURE_H

//------------------------------------------
magic.h
//------------------------------------------
#define TAILLIGHT_PAGE      0xFF
#define TAILLIGHT_FEATURE   0x02


//------------------------------------------
//  wmi.h
//------------------------------------------
//
// Where they are described.
//
#define MOFRESOURCENAME L"FireflyWMI"

//
// Initialize the FireFly drivers WMI support
//
NTSTATUS
WmiInitialize(
    WDFDEVICE       Device,
    PDEVICE_CONTEXT DeviceContext
    );

EVT_WDF_WMI_INSTANCE_QUERY_INSTANCE EvtWmiInstanceQueryInstance;
EVT_WDF_WMI_INSTANCE_SET_INSTANCE EvtWmiInstanceSetInstance;
EVT_WDF_WMI_INSTANCE_SET_ITEM EvtWmiInstanceSetItem;





