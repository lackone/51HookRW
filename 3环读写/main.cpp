#include <windows.h>
#include <stdio.h>

typedef struct _IO_STATUS_BLOCK {
	union {
		NTSTATUS Status;
		PVOID Pointer;
	};
	ULONG_PTR Information;
} IO_STATUS_BLOCK, * PIO_STATUS_BLOCK;

typedef NTSTATUS(*NtQueryInformationFile)(
	__in HANDLE FileHandle,
	__out PIO_STATUS_BLOCK IoStatusBlock,
	__out_bcount(Length) PVOID FileInformation,
	__in ULONG Length,
	__in ULONG FileInformationClass
);

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

int main()
{
	HMODULE ntdll = GetModuleHandleA("ntdll.dll");

	if (!ntdll)
	{
		return 0;
	}

	NtQueryInformationFile myfunc = (NtQueryInformationFile)GetProcAddress(ntdll, "NtQueryInformationFile");

	HANDLE file = CreateFileA("C:\\test.txt", FILE_ALL_ACCESS, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	IO_STATUS_BLOCK io = { 0 };

	CHAR buf[0xE0] = { 0 };
	PMSG_PACKAGE package = (PMSG_PACKAGE)buf;
	

	CHAR tmp[2] = { 0 };
	MEMORY_RW_INFO info = { 0 };
	info.pid = 3300;
	info.addr = 0x400000;
	info.size = 2;
	info.buf = (ULONG64)tmp;

	package->flag = 123456;
	package->data = (ULONG64)&info;
	package->size = sizeof(info);

	package->funNumber = 'rw1';
	myfunc(file, &io, buf, 0xE0, 0x34);
	printf("%x %x %x \r\n", package->result, tmp[0], tmp[1]);

	tmp[0] = 0;
	tmp[1] = 0;

	package->funNumber = 'rw2';
	myfunc(file, &io, buf, 0xE0, 0x34);
	printf("%x %x %x \r\n", package->result, tmp[0], tmp[1]);

	tmp[0] = 0;
	tmp[1] = 0;

	package->funNumber = 'rw3';
	myfunc(file, &io, buf, 0xE0, 0x34);
	printf("%x %x %x \r\n", package->result, tmp[0], tmp[1]);

	tmp[0] = 0;
	tmp[1] = 0;

	package->funNumber = 'rw4';
	myfunc(file, &io, buf, 0xE0, 0x34);
	printf("%x %x %x \r\n", package->result, tmp[0], tmp[1]);

	system("pause");
	return 0;
}