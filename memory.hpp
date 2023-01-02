#pragma once

extern "C" NTSTATUS MmCopyVirtualMemory(
	PEPROCESS Process, PVOID Address,
	PEPROCESS Target, PVOID TargetAddress,
	SIZE_T Buffer, KPROCESSOR_MODE Mode,
	PSIZE_T Return
);
extern POBJECT_TYPE* PsProcessType;

NTSTATUS KeReadVirtualMemory(PEPROCESS Process, PVOID Address, PVOID TargetAddress, SIZE_T Size) {
	SIZE_T Bytes;
	return MmCopyVirtualMemory(Process, Address, PsGetCurrentProcess(), TargetAddress, Size, KernelMode, &Bytes);
}

NTSTATUS KeWriteVirtualMemory(PEPROCESS Process, PVOID Address, PVOID TargetAddress, SIZE_T Size) {
	SIZE_T Bytes;
	return MmCopyVirtualMemory(PsGetCurrentProcess(), Address, Process, TargetAddress, Size, KernelMode, &Bytes);
}

NTSTATUS KeTerminateProcess(KERNEL_CLIENT_TERMINATE* request) {
	PEPROCESS process;
	HANDLE hProcess = NULL;
	PsLookupProcessByProcessId((HANDLE)request->ProcessId, &process);

	ObOpenObjectByPointer(process, NULL, NULL, 25, *PsProcessType, KernelMode, &hProcess);
	ZwTerminateProcess(hProcess, 0);
	ZwClose(hProcess);

	return STATUS_SUCCESS;
}