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

	//申请内存
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

	//申请内存
	PVOID mem = ExAllocatePoolWithTag(NonPagedPool, size, 0);
	if (mem == NULL)
	{
		ObDereferenceObject(process);
		return STATUS_MEMORY_NOT_ALLOCATED;
	}

	memset(mem, size, 0);

	//获取当前进程CR3
	ULONG64 cr3 = __readcr3();

	//获取目标进程CR3
	ULONG64 targetCr3 = *(PULONG64)((PUCHAR)process + 0x28);

	//关闭APC
	KeEnterCriticalRegion();

	//屏蔽中断
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

	//申请内存
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

	//申请内存
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

	//映射内存
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
 * 映射内存
 */
PVOID mapMemory(PVOID addr, ULONG size, PMDL* pmdl)
{
	PVOID mapAddr = NULL;

	//创建mdl
	PMDL mdl = IoAllocateMdl(addr, size, FALSE, FALSE, NULL);

	//锁定页，分页内存，非分页内存
	//锁定的页不会被交换到磁盘
	__try
	{
		MmProbeAndLockPages(mdl, KernelMode, IoReadAccess);
	}
	__except (1)
	{
		IoFreeMdl(mdl);

		return NULL;
	}
	
	//将 MDL 描述的物理页面映射到虚拟地址
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
 * 取消内存映射
 */
VOID unMapMemory(PVOID addr, PMDL pmdl)
{
	MmUnmapLockedPages(addr, pmdl);

	MmUnlockPages(pmdl);

	IoFreeMdl(pmdl);
}