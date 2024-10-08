#pragma once
#include <ntifs.h>

typedef NTSTATUS(*FileCallBack)(HANDLE FileHandle, PVOID arg1, PVOID arg2, PVOID arg3);

typedef struct _RegisterCallback
{
	FileCallBack QueryFileCallback;
	FileCallBack SetFileCallBack;
} RegisterCallback, * PRegisterCallback;

typedef NTSTATUS(*ExRegisterAttributeInformationCallbackProc)(PRegisterCallback callbacks);

typedef struct _MSG_PACKAGE
{
	ULONG64 funNumber;
	ULONG64 flag;
	ULONG64 data;
	ULONG64 size;
	ULONG64 result;
} MSG_PACKAGE, * PMSG_PACKAGE;

typedef struct _MEMORY_RW_INFO
{
	ULONG64 pid;
	ULONG64 addr;
	ULONG64 size;
	ULONG64 buf;
} MEMORY_RW_INFO, * PMEMORY_RW_INFO;

NTSTATUS regCallback();

VOID unRegCallback();

NTSTATUS dispatchCallback(PMSG_PACKAGE package);

