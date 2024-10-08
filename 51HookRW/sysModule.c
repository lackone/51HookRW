#include "sysModule.h"

ULONG64 getModuleBase(PCHAR moduleName, SIZE_T* moduleSize)
{
	ULONG64 testSize = 0;
	ULONG retLen = 0;
	ULONG64 imageBase = 0;

	SIZE_T len = strlen(moduleName) + 2;
	PCHAR newModuleName = ExAllocatePool(PagedPool, len);

	if (!newModuleName)
	{
		return 0;
	}

	memset(newModuleName, 0, len);
	memcpy(newModuleName, moduleName, len - 2);
	_strupr(newModuleName);

	NTSTATUS status = ZwQuerySystemInformation(SystemModuleInformation, &testSize, sizeof(ULONG64), &retLen);
	if (status == STATUS_INFO_LENGTH_MISMATCH)
	{
		PRTL_PROCESS_MODULES buf = (PRTL_PROCESS_MODULES)ExAllocatePoolWithTag(NonPagedPool, retLen, 0);
		if (!buf)
		{
			ExFreePool(newModuleName);
			return 0;
		}

		memset(buf, 0, retLen);

		status = ZwQuerySystemInformation(SystemModuleInformation, buf, retLen, &retLen);

		if (!NT_SUCCESS(status))
		{
			ExFreePoolWithTag(buf, 0);
			ExFreePool(newModuleName);
			return 0;
		}

		ULONG size = buf->NumberOfModules;

		for (SIZE_T i = 0; i < size; i++)
		{
			PRTL_PROCESS_MODULE_INFORMATION module = &buf->Modules[i];

			_strupr(module->FullPathName);

			DbgPrintEx(77, 0, "%s \r\n", module->FullPathName);

			if (strstr(module->FullPathName, newModuleName))
			{
				if (moduleSize)
				{
					*moduleSize = module->ImageSize;
				}

				imageBase = module->ImageBase;

				break;
			}
		}

		ExFreePoolWithTag(buf, 0);
	}

	if (newModuleName)
	{
		ExFreePool(newModuleName);
	}

	return imageBase;
}