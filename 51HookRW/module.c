#include "module.h"

ULONG64 getModule32(HANDLE pid, LPSTR dllName, SIZE_T* dllSize)
{
	DbgBreakPoint();

	if (dllName == NULL)
	{
		return 0;
	}

	PEPROCESS process = NULL;
	NTSTATUS status = PsLookupProcessByProcessId(pid, &process);
	if (!NT_SUCCESS(status))
	{
		return 0;
	}

	KAPC_STATE apc = { 0 };
	KeStackAttachProcess(process, &apc);

	//PEB32是32位的，定义时要按4字节对齐，LIST_ENTRY 和 UNICODE_STRING 要换成32位的，VOID* 换成 ULONG，因为VOID*64位下占8字节
	PPEB32 peb32 = PsGetProcessWow64Process(process);
	if (peb32 == NULL)
	{
		KeUnstackDetachProcess(&apc);
		ObDereferenceObject(process);
		return 0;
	}

	SIZE_T retSize = 0;
	MmCopyVirtualMemory(process, peb32, process, peb32, 1, KernelMode, &retSize);

	PPEB_LDR_DATA32 ldr32 = (PPEB_LDR_DATA32)peb32->Ldr;

	PLDR_DATA_TABLE_ENTRY32 list = (PLDR_DATA_TABLE_ENTRY32)&ldr32->InLoadOrderModuleList;
	PLDR_DATA_TABLE_ENTRY32 next = list->InLoadOrderLinks.Flink;

	ANSI_STRING aname = { 0 };
	UNICODE_STRING uname = { 0 };
	RtlInitAnsiString(&aname, dllName);
	RtlAnsiStringToUnicodeString(&uname, &aname, TRUE);

	ULONG64 dllBase = 0;

	while (next != list)
	{
		//这里需要重新申请变量
		UNICODE_STRING tmp = { 0 };
		RtlInitUnicodeString(&tmp, next->BaseDllName.Buffer);

		if (RtlCompareUnicodeString(&tmp, &uname, TRUE) == 0)
		{
			dllBase = next->DllBase;

			if (dllSize)
			{
				*dllSize = next->SizeOfImage;
			}
			
			break;
		}

		next = next->InLoadOrderLinks.Flink;
	}

	KeUnstackDetachProcess(&apc);

	ObDereferenceObject(process);

	RtlFreeUnicodeString(&uname);

	return dllBase;
}

