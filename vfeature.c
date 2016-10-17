/*++
    vfeature.c
    This module sets the state of a vendor specific HID feature.
    Kernel mode
--*/

#include "firefly.h"

#pragma warning(disable:4201)  // nameless struct/union
#pragma warning(disable:4214)  // bit field types other than int

#include <hidpddi.h>
#include <hidclass.h>

#pragma warning(default:4201)
#pragma warning(default:4214)

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, FireflySetFeature)
#endif

NTSTATUS
FireflySetFeature(
    IN  PDEVICE_CONTEXT DeviceContext,
    IN  UCHAR           PageId, //usage page,灯控制的
    IN  USHORT          FeatureId, //usage id
    IN  BOOLEAN         EnableFeature //灯开关
    )
/*++
    This routine sets the HID feature by sending HID ioctls to our device.
    These IOCTLs will be handled by HIDUSB and converted into USB requests
    and send to the device.
    DeviceContext - Context for our device
    PageID  - UsagePage of the light control feature.
    FeatureId - Usage ID of the feature.
    EnanbleFeature - True to turn the light on, Falst to turn if off.

Return Value:

    NT Status code

--*/
{
    WDF_MEMORY_DESCRIPTOR       inputDescriptor, outputDescriptor;
    NTSTATUS                    status;
    HID_COLLECTION_INFORMATION  collectionInformation = {0};//全部初始化为0
    PHIDP_PREPARSED_DATA        preparsedData;
    HIDP_CAPS                   caps;
    USAGE                       usage;
    ULONG                       usageLength;
    PCHAR                       report;
    WDFIOTARGET                 hidTarget; //句柄
    WDF_IO_TARGET_OPEN_PARAMS   openParams;

    PAGED_CODE();

    //
    // Preinit for error.
    //
    preparsedData = NULL;
    report = NULL;
    hidTarget = NULL;
    
	//-------------------------------------------------------------
	// 创建WDFIOTARGET对象，一个 remote I/O target
	//-------------------------------------------------------------

    status = WdfIoTargetCreate(WdfObjectContextGetObject(DeviceContext), // returns a handle to a framework object.
                            WDF_NO_OBJECT_ATTRIBUTES, 
                            &hidTarget); //输出，句柄   
...
	//-------------------------------------------------------------
	// 打开WDFIOTARGET对象
	//-------------------------------------------------------------

    //
    // Open it up, write access only!
    //
    WDF_IO_TARGET_OPEN_PARAMS_INIT_OPEN_BY_NAME(
                                    &openParams, //被初始化
                                    &DeviceContext->PdoName, //用这个名
                                    FILE_WRITE_ACCESS);//权限

    //
    // We will let the framework to respond automatically to the pnp
    // state changes of the target by closing and opening the handle.
    //
    openParams.ShareAccess = FILE_SHARE_WRITE | FILE_SHARE_READ; //这种对应关系要学习

    status = WdfIoTargetOpen(hidTarget, &openParams);
    ...
    
	//-------------------------------------------------------------
	// 发送一个控制请求，读collection信息,目的是要collection描述符长度
	//-------------------------------------------------------------

    WDF_MEMORY_DESCRIPTOR_INIT_BUFFER(&outputDescriptor,//输出，现在可以describes a specified buffer.
                                      (PVOID) &collectionInformation, //真实buffer，本地变量
                                      sizeof(HID_COLLECTION_INFORMATION));//bufferlength

    //
    // Now get the collection information for this device
    //
    status = WdfIoTargetSendIoctlSynchronously(hidTarget,
                                  NULL, //request，可选
                                  IOCTL_HID_GET_COLLECTION_INFORMATION,//IoctlCode
                                  NULL,//InputBuffer,可选	
                                  &outputDescriptor,//OutputBuffer,读的东西到此，collectionInformation信息完整了
                                  NULL,//RequestOptions
                                  NULL);//BytesReturned

    
	//-------------------------------------------------------------
	// 发送一个控制请求，读collection描述符，得到preparsedData
	//-------------------------------------------------------------

    preparsedData = (PHIDP_PREPARSED_DATA) ExAllocatePoolWithTag(
        NonPagedPoolNx, collectionInformation.DescriptorSize, 'ffly');
...

    WDF_MEMORY_DESCRIPTOR_INIT_BUFFER(&outputDescriptor,//输出，outputDescriptor重用不是原来的意思
                                      (PVOID) preparsedData,//真实buffer
                                      collectionInformation.DescriptorSize);//bufferlength，上面刚刚读到的

    status = WdfIoTargetSendIoctlSynchronously(hidTarget,//句柄
                                  NULL,
                                  IOCTL_HID_GET_COLLECTION_DESCRIPTOR,
                                  NULL,
                                  &outputDescriptor,//输出，注意现在代表preparsedData，句柄
                                  NULL,
                                  NULL);

...

	//-------------------------------------------------------------
	// 获得caps,在有preparsedData的前提下
	//-------------------------------------------------------------
    //
    // Now get the capabilities.
    //
    RtlZeroMemory(&caps, sizeof(HIDP_CAPS));

    status = HidP_GetCaps(preparsedData/*上一步刚刚得到*/, &caps);

...

	//-------------------------------------------------------------
	// 创建report，长度来自caps.FeatureReportByteLength
	//-------------------------------------------------------------
    //
    // Create a report to send to the device.
    //
    report = (PCHAR) ExAllocatePoolWithTag(
        NonPagedPoolNx, caps.FeatureReportByteLength, 'ffly');

...
//===============================================================
// 上面的应该可以优化，否则每次这么做太浪费时间了
//===============================================================
    //
    // Start with a zeroed report. If we are disabling the feature, this might
    // be all we need to do.
    //
    RtlZeroMemory(report, caps.FeatureReportByteLength);//所有的灯缺省是灭的
    status = STATUS_SUCCESS;

	//-------------------------------------------------------------
	// 修改report
	//-------------------------------------------------------------
    if (EnableFeature) { 

        // 打开灯的情况，灭灯的情况不用
        // Edit the report to reflect the enabled feature
        //
        usage = FeatureId;
        usageLength = 1;

        status = HidP_SetUsages(//sets specified HID control buttons ON (1) in a HID report.
            HidP_Feature,//枚举，ReportType
            PageId, //usage page
            0,//LinkCollection=0,the routine sets the first usage for each specified usage in the top-level collection associated with PreparsedData.
            &usage, // array of usages.
            &usageLength, // number of usages in the usage list
            preparsedData,
            report,//地址
            caps.FeatureReportByteLength
            );
		...
    }

	//-------------------------------------------------------------
	// 最后一步：发送！把report发送出去
	//-------------------------------------------------------------
    WDF_MEMORY_DESCRIPTOR_INIT_BUFFER(&inputDescriptor,//把report转化成wdf内存对象
                                      report,
                                      caps.FeatureReportByteLength);
    status = WdfIoTargetSendIoctlSynchronously(hidTarget,
                                  NULL,
                                  IOCTL_HID_SET_FEATURE,
                                  &inputDescriptor,//必须是wdf牌
                                  NULL,
                                  NULL,
                                  NULL);
...

ExitAndFree:

    if (preparsedData != NULL) {
        ExFreePool(preparsedData);
        preparsedData = NULL;
    }

    if (report != NULL) {
        ExFreePool(report);
        report = NULL;
    }

    if (hidTarget != NULL) {
        WdfObjectDelete(hidTarget);
    }

    return status;
}
