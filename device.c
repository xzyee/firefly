/*++
    device.c
    This module contains the Windows Driver Framework Device object
    handlers for the firefly filter driver.
    Kernel mode
--*/

#include "FireFly.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, FireFlyEvtDeviceAdd)
#endif

NTSTATUS
FireFlyEvtDeviceAdd(
    WDFDRIVER Driver, //driverentry创建的，未用
    PWDFDEVICE_INIT DeviceInit
    )
/*++
    EvtDeviceAdd is called by the framework in response to AddDevice
    call from the PnP manager. We create and initialize a device object to
    represent to be part of the device stack as a filter.
--*/    
{
    WDF_OBJECT_ATTRIBUTES           attributes;
    NTSTATUS                        status;
    PDEVICE_CONTEXT                 pDeviceContext;
    WDFDEVICE                       device;
    WDFMEMORY                       memory;
    size_t                          bufferLength;

    UNREFERENCED_PARAMETER(Driver);

    PAGED_CODE();
	//----------------------------------------
	//第一步：声明这是个filter驱动
	//----------------------------------------
    //
    // Configure the device as a filter driver
    //
    WdfFdoInitSetFilter(DeviceInit);

	//----------------------------------------
	//第二步：创建device，wdf牌
	//----------------------------------------
    WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(&attributes, DEVICE_CONTEXT);

    status = WdfDeviceCreate(&DeviceInit, &attributes, &device);
...
    //
    // Driver Framework always zero initializes an objects context memory
    //
    pDeviceContext = WdfObjectGet_DEVICE_CONTEXT(device);//马上获得设备扩展

	//----------------------------------------
	//第三步：初始化WMI的支持
	//----------------------------------------
    //
    // Initialize our WMI support
    //
    status = WmiInitialize(device, pDeviceContext);
...

	//----------------------------------------
	//第四步：创建一块内存，用于保存名字（打开文件的名字）
	//----------------------------------------
    //
    // In order to send ioctls to our PDO, we have open to open it
    // by name so that we have a valid filehandle (fileobject).
    // When we send ioctls using the IoTarget, framework automatically 
    // sets the filobject in the stack location.
    //
    WDF_OBJECT_ATTRIBUTES_INIT(&attributes);
    //
    // By parenting it to device, we don't have to worry about
    // deleting explicitly. It will be deleted along witht the device.
    //
    attributes.ParentObject = device;//设置父亲的好处，不需要管什么时候删除之

    status = WdfDeviceAllocAndQueryProperty(device, //刚刚创建的
                                    DevicePropertyPhysicalDeviceObjectName,//=0xb,设备属性（在注册表中的）
                                    NonPagedPoolNx,
                                    &attributes,//可以为WDF_NO_OBJECT_ATTRIBUTES，不过这里有父亲
                                    &memory);//输出,这块内存只管用，不需要你亲自管理释放等复杂的东西

...
	//保存这块内存到设备扩展中
    pDeviceContext->PdoName.Buffer = WdfMemoryGetBuffer(memory, &bufferLength);//内存挂在设备扩展里
...

    pDeviceContext->PdoName.MaximumLength = (USHORT) bufferLength;
    pDeviceContext->PdoName.Length = (USHORT) bufferLength-sizeof(UNICODE_NULL);//最后保留一个0

    return status;
}
