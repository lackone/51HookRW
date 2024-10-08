#include <ntifs.h>
#include <ntddk.h>
#include <intrin.h>
#include "utils.h"
#include "module.h"
#include "sysModule.h"

VOID DriverUnload(PDRIVER_OBJECT DriverObject)
{
	DbgPrintEx(77, 0, "DriverUnload \r\n");

	//unRegCallback();

	LARGE_INTEGER time = { 0 };
	time.QuadPart = -10000 * 1000;
	KeDelayExecutionThread(KernelMode, FALSE, &time);
}

/**
 * 
 * 
 * 
win7下的通信

typedef NTSTATUS(*FileCallBack)(HANDLE FileHandle, PVOID arg1, PVOID arg2, PVOID arg3);

typedef struct _RegisterCallback
{
	FileCallBack QueryFileCallback;
	FileCallBack SetFileCallBack;
} RegisterCallback, * PRegisterCallback;

typedef NTSTATUS(*ExRegisterAttributeInformationCallbackProc)(PRegisterCallback callbacks);

win10下的通信

NtConvertBetweenAuxiliaryCounterAndPerformanceCounter

 */
NTSTATUS DriverEntry(PDRIVER_OBJECT pDriver, PUNICODE_STRING pReg)
{
	//regCallback();

	//SIZE_T dllsize = 0;
	//ULONG64 dllbase = getModule32(3156, "kernel32.dll", &dllsize);
	//DbgPrintEx(77, 0, "%llx %llx \r\n", dllbase, dllsize);

	SIZE_T moduleSize = 0;
	ULONG64 moduleBase = getModuleBase("ntoskrnl.exe", &moduleSize);
	DbgPrintEx(77, 0, "%llx %llx \r\n", moduleBase, moduleSize);

	pDriver->DriverUnload = DriverUnload;
	return STATUS_SUCCESS;
}
