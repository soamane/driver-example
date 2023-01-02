#include <ntifs.h>
#include <wdm.h>

#include "driver.hpp"
#include "memory.hpp"
void UnloadDriver(PDRIVER_OBJECT DriverObject) {
	UNREFERENCED_PARAMETER(DriverObject);

	UNICODE_STRING SymbolLink = RTL_CONSTANT_STRING(DEVICE_LINK);
	IoDeleteSymbolicLink(&SymbolLink);
	IoDeleteDevice(DriverObject->DeviceObject);
}

NTSTATUS CreateCall(PDEVICE_OBJECT DeviceObject, PIRP Irp) {
	UNREFERENCED_PARAMETER(DeviceObject);
	UNREFERENCED_PARAMETER(Irp);

	Irp->IoStatus.Status = STATUS_SUCCESS;
	Irp->IoStatus.Information = 0;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);

	return STATUS_SUCCESS;
}

NTSTATUS IoDeviceControl(PDEVICE_OBJECT DeviceObject, PIRP Irp) {
	UNREFERENCED_PARAMETER(DeviceObject);
	UNREFERENCED_PARAMETER(Irp);

	PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(Irp);
	ULONG IoControlCode = stack->Parameters.DeviceIoControl.IoControlCode;

	KERNEL_CLIENT_READ* RequestRead;
	KERNEL_CLIENT_WRITE* RequestWrite;
	KERNEL_CLIENT_TERMINATE* RequestTerminate;

	PEPROCESS hProcess;
	switch (IoControlCode) {
	case WRITE_REQUEST:
		RequestWrite = (KERNEL_CLIENT_WRITE*)Irp->AssociatedIrp.SystemBuffer;
		PsLookupProcessByProcessId((HANDLE)RequestWrite->ProcessId, &hProcess);
		KeWriteVirtualMemory(hProcess, RequestWrite->Buff, (PVOID)RequestWrite->Address, RequestWrite->Size);
		break;

	case READ_REQUEST:
		RequestRead = (KERNEL_CLIENT_READ*)Irp->AssociatedIrp.SystemBuffer;
		PsLookupProcessByProcessId((HANDLE)RequestRead->ProcessId, &hProcess);
		KeReadVirtualMemory(hProcess, (PVOID)RequestRead->Address, RequestRead->Buff, RequestRead->Size);
		break;

	case TERMINATE_REQUEST:
		RequestTerminate = (KERNEL_CLIENT_TERMINATE*)Irp->AssociatedIrp.SystemBuffer;
		KeTerminateProcess(RequestTerminate);
		break;
	default:
		break;
	}

	Irp->IoStatus.Status = STATUS_SUCCESS;
	Irp->IoStatus.Information = 0;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);

	return STATUS_SUCCESS;
}

NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING PUniString) {
	UNREFERENCED_PARAMETER(DriverObject);
	UNREFERENCED_PARAMETER(PUniString);

	DriverObject->DriverUnload = UnloadDriver;
	DriverObject->MajorFunction[IRP_MJ_CREATE] = CreateCall;
	DriverObject->MajorFunction[IRP_MJ_CLOSE] = CreateCall;
	DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = IoDeviceControl;

	UNICODE_STRING DeviceName = RTL_CONSTANT_STRING(DEVICE_NAME);
	RtlInitUnicodeString(&DeviceName, DEVICE_NAME);

	PDEVICE_OBJECT DeviceObject;
	IoCreateDevice(DriverObject, NULL, &DeviceName, FILE_DEVICE_UNKNOWN, NULL, FALSE, &DeviceObject);

	UNICODE_STRING SymbolLink = RTL_CONSTANT_STRING(DEVICE_LINK);
	IoCreateSymbolicLink(&SymbolLink, &DeviceName);

	return STATUS_SUCCESS;
}