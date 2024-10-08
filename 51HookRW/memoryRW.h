#pragma once
#include <ntifs.h>
#include <intrin.h>

NTSTATUS MmCopyVirtualMemory(
	IN PEPROCESS FromProcess,
	IN CONST VOID* FromAddress,
	IN PEPROCESS ToProcess,
	OUT PVOID ToAddress,
	IN SIZE_T BufferSize,
	IN KPROCESSOR_MODE PreviousMode,
	OUT PSIZE_T NumberOfBytesCopied
);

NTSTATUS readR3Memory(HANDLE pid, PVOID addr, SIZE_T size, PVOID buf);

NTSTATUS readR3MemoryByCr3(HANDLE pid, PVOID addr, SIZE_T size, PVOID buf);

NTSTATUS readR3MemoryByMMCpy(HANDLE pid, PVOID addr, SIZE_T size, PVOID buf);

NTSTATUS readR3MemoryByMDL(HANDLE pid, PVOID addr, SIZE_T size, PVOID buf);

PVOID mapMemory(PVOID addr, ULONG size, PMDL* pmdl);

VOID unMapMemory(PVOID addr, PMDL pmdl);