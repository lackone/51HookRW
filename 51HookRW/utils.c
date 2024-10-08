#include "utils.h"
#include "memoryRW.h"

FileCallBack g_oldQuery = NULL;
FileCallBack g_oldSet = NULL;
PULONG_PTR g_ExpDisQueryAttributeInformation = NULL;

NTSTATUS queryFileCallback(HANDLE FileHandle, PVOID arg1, PVOID arg2, PVOID arg3)
{
	//arg1 就是 buf 的地址
	if (MmIsAddressValid(arg1))
	{
		PMSG_PACKAGE package = (PMSG_PACKAGE)arg1;
		if (package->flag == 123456)
		{
			DbgPrintEx(77, 0, "收到3环数据 \r\n");

			package->result = dispatchCallback(package);

			return STATUS_SUCCESS;
		}
		else
		{
			if (g_oldQuery)
			{
				return g_oldQuery(FileHandle, arg1, arg2, arg3);
			}
		}
	}

	return STATUS_SUCCESS;
}

NTSTATUS setFileCallback(HANDLE FileHandle, PVOID arg1, PVOID arg2, PVOID arg3)
{
	//arg1 就是 buf 的地址
	if (MmIsAddressValid(arg1))
	{
		PMSG_PACKAGE package = (PMSG_PACKAGE)arg1;
		if (package->flag == 123456)
		{
			DbgPrintEx(77, 0, "收到3环数据 \r\n");

			package->result = dispatchCallback(package);

			return STATUS_SUCCESS;
		}
		else
		{
			if (g_oldSet)
			{
				return g_oldSet(FileHandle, arg1, arg2, arg3);
			}
		}
	}

	return STATUS_SUCCESS;
}

NTSTATUS dispatchCallback(PMSG_PACKAGE package)
{
	switch (package->funNumber)
	{
	case 'rw1':
	{
		PMEMORY_RW_INFO data = (PMEMORY_RW_INFO)package->data;

		DbgPrintEx(77, 0, "readR3Memory \r\n");

		return readR3Memory(data->pid, data->addr, data->size, data->buf);
	}
	break;
	case 'rw2':
	{
		PMEMORY_RW_INFO data = (PMEMORY_RW_INFO)package->data;

		DbgPrintEx(77, 0, "readR3MemoryByCr3 \r\n");

		return readR3MemoryByCr3(data->pid, data->addr, data->size, data->buf);
	}
	break;
	case 'rw3':
	{
		PMEMORY_RW_INFO data = (PMEMORY_RW_INFO)package->data;

		DbgPrintEx(77, 0, "readR3MemoryByMMCpy \r\n");

		return readR3MemoryByMMCpy(data->pid, data->addr, data->size, data->buf);
	}
	break;
	case 'rw4':
	{
		PMEMORY_RW_INFO data = (PMEMORY_RW_INFO)package->data;

		DbgPrintEx(77, 0, "readR3MemoryByMDL \r\n");

		return readR3MemoryByMDL(data->pid, data->addr, data->size, data->buf);
	}
	break;
	default:
		break;
	}

	return STATUS_SUCCESS;
}

/**
 * 注册我们的回调
 */
NTSTATUS regCallback()
{
	//ExpDisQueryAttributeInformation
	//ExpDisSetAttributeInformation

	//获取ExRegisterAttributeInformationCallback函数地址
	UNICODE_STRING name = { 0 };
	RtlInitUnicodeString(&name, L"ExRegisterAttributeInformationCallback");
	PUCHAR func = MmGetSystemRoutineAddress(&name);

	PULONG_PTR ExpDisQueryAttributeInformation = 0;
	PULONG_PTR ExpDisSetAttributeInformation = 0;

	INT findCnt = 0;

	for (INT i = 0; i < 100; i++)
	{
		if (func[i] == 0x48 && func[i + 1] == 0x39 && func[i + 2] == 0x05)
		{
			if (findCnt == 0)
			{
				LONG offset = *(PLONG)(func + i + 3);
				ExpDisQueryAttributeInformation = (func + i + 7) + offset;
				//保存旧值
				g_oldQuery = *ExpDisQueryAttributeInformation;
				//清空
				*ExpDisQueryAttributeInformation = 0;
			}
			else if (findCnt == 1)
			{
				LONG offset = *(PLONG)(func + i + 3);
				ExpDisSetAttributeInformation = (func + i + 7) + offset;
				//保存旧值
				g_oldSet = *ExpDisSetAttributeInformation;
				//清空
				*ExpDisSetAttributeInformation = 0;
				break;
			}
			findCnt++;
		}
	}

	//注册我们自已的回调
	ExRegisterAttributeInformationCallbackProc regCallbackFunc = func;

	RegisterCallback reg = {
		queryFileCallback,
		setFileCallback,
	};

	NTSTATUS status = regCallbackFunc(&reg);

	if (!NT_SUCCESS(status))
	{
		return status;
	}

	g_ExpDisQueryAttributeInformation = ExpDisQueryAttributeInformation;

	return STATUS_SUCCESS;
}

VOID unRegCallback()
{
	//恢复原callback
	if (g_ExpDisQueryAttributeInformation)
	{
		g_ExpDisQueryAttributeInformation[0] = g_oldQuery;
		g_ExpDisQueryAttributeInformation[1] = g_oldSet;
	}
}