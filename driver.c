/*++
    driver.c
    This modules contains the Windows Driver Framework Driver object
    handlers for the firefly filter driver.
    Kernel mode
--*/

#include "FireFly.h"

NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT  DriverObject,
    IN PUNICODE_STRING RegistryPath
    )
{
    WDF_DRIVER_CONFIG params;
    NTSTATUS  status;

	...
    WDF_DRIVER_CONFIG_INIT(
                        &params,
                        FireFlyEvtDeviceAdd
                        );

    //
    // Create the framework WDFDRIVER object, with the handle
    // to it returned in Driver.
    //
    status = WdfDriverCreate(DriverObject, //透明的
                             RegistryPath, //透明的
                             WDF_NO_OBJECT_ATTRIBUTES, //必须为NULL
                             &params, //初始化了FireFlyEvtDeviceAdd函数
                             WDF_NO_HANDLE);//不需要
    ...
    return status;
}
