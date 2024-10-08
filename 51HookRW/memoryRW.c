#include "memoryRW.h"

NTSTATUS readR3Memory(HANDLE pid, PVOID addr, SIZE_T size, PVOID buf)
{
	if (addr >= MmHighestUserAddress ||
		(ULONG64)addr + size >= MmHighestUserAddress
	)
	{
		return STATUS_ACCESS_VIOLATION;
	}

	if (size == 0)
	{
		return STATUS_INVALID_PARAMETER_3;
	}

	if (buf == NULL)
	{
		return STATUS_INVALID_PARAMETER_4;
	}

	PEPROCESS process = NULL;
	NTSTATUS status = PsLookupProcessByProcessId(pid, &process);
	if (!NT_SUCCESS(status))
	{
		return status;
	}

	if (PsGetProcessExitStatus(process) != STATUS_PENDING)
	{
		return STATUS_INVALID_PARAMETER_1;
	}

	//�����ڴ�
	PVOID mem = ExAllocatePoolWithTag(NonPagedPool, size, 0);
	if (mem == NULL)
	{
		ObDereferenceObject(process);
		return STATUS_MEMORY_NOT_ALLOCATED;
	}

	memset(mem, size, 0);

	KAPC_STATE apc = { 0 };

	KeStackAttachProcess(process, &apc);

	if (!(MmIsAddressValid(addr) && MmIsAddressValid((ULONG64)addr + size)))
	{
		KeUnstackDetachProcess(&apc);

		ExFreePoolWithTag(mem, 0);

		ObDereferenceObject(process);

		return STATUS_ACCESS_VIOLATION;
	}

	memcpy(mem, addr, size);

	KeUnstackDetachProcess(&apc);

	memcpy(buf, mem, size);

	ExFreePoolWithTag(mem, 0);

	ObDereferenceObject(process);

	return STATUS_SUCCESS;
}

NTSTATUS readR3MemoryByCr3(HANDLE pid, PVOID addr, SIZE_T size, PVOID buf)
{
	if (addr >= MmHighestUserAddress ||
		(ULONG64)addr + size >= MmHighestUserAddress
		)
	{
		return STATUS_ACCESS_VIOLATION;
	}

	if (size == 0)
	{
		return STATUS_INVALID_PARAMETER_3;
	}

	if (buf == NULL)
	{
		return STATUS_INVALID_PARAMETER_4;
	}

	PEPROCESS process = NULL;
	NTSTATUS status = PsLookupProcessByProcessId(pid, &process);
	if (!NT_SUCCESS(status))
	{
		return status;
	}

	if (PsGetProcessExitStatus(process) != STATUS_PENDING)
	{
		return STATUS_INVALID_PARAMETER_1;
	}

	//�����ڴ�
	PVOID mem = ExAllocatePoolWithTag(NonPagedPool, size, 0);
	if (mem == NULL)
	{
		ObDereferenceObject(process);
		return STATUS_MEMORY_NOT_ALLOCATED;
	}

	memset(mem, size, 0);

	//��ȡ��ǰ����CR3
	ULONG64 cr3 = __readcr3();

	//��ȡĿ�����CR3
	ULONG64 targetCr3 = *(PULONG64)((PUCHAR)process + 0x28);

	//�ر�APC
	KeEnterCriticalRegion();

	//�����ж�
	_disable();

	__writecr3(targetCr3);

	if (!(MmIsAddressValid(addr) && MmIsAddressValid((ULONG64)addr + size)))
	{
		__writecr3(cr3);

		_enable();

		KeLeaveCriticalRegion();

		ExFreePoolWithTag(mem, 0);

		ObDereferenceObject(process);

		return STATUS_ACCESS_VIOLATION;
	}

	memcpy(mem, addr, size);

	__writecr3(cr3);

	_enable();

	KeLeaveCriticalRegion();

	memcpy(buf, mem, size);


	ExFreePoolWithTag(mem, 0);

	ObDereferenceObject(process);

	return STATUS_SUCCESS;
}

NTSTATUS readR3MemoryByMMCpy(HANDLE pid, PVOID addr, SIZE_T size, PVOID buf)
{
	if (addr >= MmHighestUserAddress ||
		(ULONG64)addr + size >= MmHighestUserAddress
		)
	{
		return STATUS_ACCESS_VIOLATION;
	}

	if (size == 0)
	{
		return STATUS_INVALID_PARAMETER_3;
	}

	if (buf == NULL)
	{
		return STATUS_INVALID_PARAMETER_4;
	}

	PEPROCESS process = NULL;
	NTSTATUS status = PsLookupProcessByProcessId(pid, &process);
	if (!NT_SUCCESS(status))
	{
		return status;
	}

	if (PsGetProcessExitStatus(process) != STATUS_PENDING)
	{
		return STATUS_INVALID_PARAMETER_1;
	}

	//�����ڴ�
	PVOID mem = ExAllocatePoolWithTag(NonPagedPool, size, 0);
	if (mem == NULL)
	{
		ObDereferenceObject(process);
		return STATUS_MEMORY_NOT_ALLOCATED;
	}

	memset(mem, size, 0);

	SIZE_T ret = 0;

	MmCopyVirtualMemory(process, addr, IoGetCurrentProcess(), mem, size, KernelMode, &ret);

	memcpy(buf, mem, size);

	ExFreePoolWithTag(mem, 0);

	ObDereferenceObject(process);

	return STATUS_SUCCESS;
}

NTSTATUS readR3MemoryByMDL(HANDLE pid, PVOID addr, SIZE_T size, PVOID buf)
{
	DbgBreakPoint();

	if (addr >= MmHighestUserAddress ||
		(ULONG64)addr + size >= MmHighestUserAddress
		)
	{
		return STATUS_ACCESS_VIOLATION;
	}

	if (size == 0)
	{
		return STATUS_INVALID_PARAMETER_3;
	}

	if (buf == NULL)
	{
		return STATUS_INVALID_PARAMETER_4;
	}

	PEPROCESS process = NULL;
	NTSTATUS status = PsLookupProcessByProcessId(pid, &process);
	if (!NT_SUCCESS(status))
	{
		return status;
	}

	if (PsGetProcessExitStatus(process) != STATUS_PENDING)
	{
		return STATUS_INVALID_PARAMETER_1;
	}

	//�����ڴ�
	PVOID mem = ExAllocatePoolWithTag(NonPagedPool, size, 0);
	if (mem == NULL)
	{
		ObDereferenceObject(process);
		return STATUS_MEMORY_NOT_ALLOCATED;
	}

	memset(mem, size, 0);

	KAPC_STATE apc = { 0 };

	KeStackAttachProcess(process, &apc);

	if (!(MmIsAddressValid(addr) && MmIsAddressValid((ULONG64)addr + size)))
	{
		KeUnstackDetachProcess(&apc);

		ExFreePoolWithTag(mem, 0);

		ObDereferenceObject(process);

		return STATUS_ACCESS_VIOLATION;
	}

	//ӳ���ڴ�
	PMDL mdl = NULL;

	PVOID mapAddr = mapMemory(addr, size, &mdl);

	if (mapAddr == NULL)
	{
		KeUnstackDetachProcess(&apc);

		ExFreePoolWithTag(mem, 0);

		ObDereferenceObject(process);

		return STATUS_ACCESS_VIOLATION;
	}

	memcpy(mem, mapAddr, size);

	unMapMemory(mapAddr, mdl);

	KeUnstackDetachProcess(&apc);

	memcpy(buf, mem, size);

	ExFreePoolWithTag(mem, 0);

	ObDereferenceObject(process);

	return STATUS_SUCCESS;
}

/**
 * ӳ���ڴ�
 */
PVOID mapMemory(PVOID addr, ULONG size, PMDL* pmdl)
{
	PVOID mapAddr = NULL;

	//����mdl
	PMDL mdl = IoAllocateMdl(addr, size, FALSE, FALSE, NULL);

	//����ҳ����ҳ�ڴ棬�Ƿ�ҳ�ڴ�
	//������ҳ���ᱻ����������
	__try
	{
		MmProbeAndLockPages(mdl, KernelMode, IoReadAccess);
	}
	__except (1)
	{
		IoFreeMdl(mdl);

		return NULL;
	}
	
	//�� MDL ����������ҳ��ӳ�䵽�����ַ
	__try
	{
		mapAddr = MmMapLockedPagesSpecifyCache(mdl, KernelMode, MmCached, NULL, FALSE, HighPagePriority);
	}
	__except (1)
	{
		MmUnlockPages(mdl);

		IoFreeMdl(mdl);

		return NULL;
	}

	*pmdl = mdl;
	
	return mapAddr;
}

/**
 * ȡ���ڴ�ӳ��
 */
VOID unMapMemory(PVOID addr, PMDL pmdl)
{
	MmUnmapLockedPages(addr, pmdl);

	MmUnlockPages(pmdl);

	IoFreeMdl(pmdl);
}