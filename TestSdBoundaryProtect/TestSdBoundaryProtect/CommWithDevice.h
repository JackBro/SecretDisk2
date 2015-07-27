/*

// TestWDMDriver.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <windows.h>

#define DEVICE_NAME L"\\\\.\\HelloWDM"

int _tmain(int argc, _TCHAR* argv[])
{
	HANDLE hDevice = CreateFile(DEVICE_NAME,GENERIC_READ | GENERIC_WRITE,0,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);

	if (hDevice != INVALID_HANDLE_VALUE)
	{
		char* inbuf = "hello world";
		char outbuf[12] = {0};
		DWORD dwBytes = 0;
		BOOL b = DeviceIoControl(hDevice, CTL_CODE(FILE_DEVICE_UNKNOWN, 0x800, METHOD_IN_DIRECT, FILE_ANY_ACCESS), 
			inbuf, 11, outbuf, 11, &dwBytes, NULL);

		for (int i = 0; i < 11; i++)
		{
			outbuf[i] = outbuf[i] ^ 'm';
		}

		printf("DeviceIoControl, ret: %d, outbuf: %s, outbuf address: %x operated: %d bytes\n", b, outbuf, outbuf, dwBytes);

		CloseHandle(hDevice);

	}
	else
		printf("CreateFile failed, err: %x\n", GetLastError());

	return 0;
}

#define IOCTL_ENCODE CTL_CODE(FILE_DEVICE_UNKNOWN, 0x800, METHOD_IN_DIRECT, FILE_ANY_ACCESS)

NTSTATUS HelloWDMIOControl(IN PDEVICE_OBJECT fdo, IN PIRP Irp)
{
	KdPrint(("Enter HelloWDMIOControl\n"));

	PDEVICE_EXTENSION pdx = (PDEVICE_EXTENSION)fdo->DeviceExtension;
	PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(Irp);

	//�õ����뻺������С
	ULONG cbin = stack->Parameters.DeviceIoControl.InputBufferLength;

	//�õ������������С
	ULONG cbout = stack->Parameters.DeviceIoControl.OutputBufferLength;

	//�õ�IOCTRL��
	ULONG code = stack->Parameters.DeviceIoControl.IoControlCode;

	NTSTATUS status;
	ULONG info = 0;
	switch (code)
	{
	case IOCTL_ENCODE:
		{
			//��ȡ���뻺������IRP_MJ_DEVICE_CONTROL�����붼��ͨ��buffered io�ķ�ʽ
			char* inBuf = (char*)Irp->AssociatedIrp.SystemBuffer;
			for (ULONG i = 0; i < cbin; i++)//�����뻺���������ÿ���ֽں�m���
			{
				inBuf[i] = inBuf[i] ^ 'm';
			}

			//��ȡ���������������ʹ����ֱ�ӷ�ʽ����CTL_CODE�Ķ��壬ʹ����METHOD_IN_DIRECT��������Ҫͨ��ֱ�ӷ�ʽ��ȡout buffer
			KdPrint(("user address: %x, this address should be same to user mode addess.\n", MmGetMdlVirtualAddress(Irp->MdlAddress)));
			//��ȡ�ں�ģʽ�µĵ�ַ�������ַһ��> 0x7FFFFFFF,�����ַ��������û�ģʽ��ַ��Ӧͬһ�������ڴ�
			char* outBuf = (char*)MmGetSystemAddressForMdlSafe(Irp->MdlAddress, NormalPagePriority);

			ASSERT(cbout >= cbin);
			RtlCopyMemory(outBuf, inBuf, cbin);
			info = cbin;
			status = STATUS_SUCCESS;
		}
		break;
	default:
		status = STATUS_INVALID_VARIANT;
		break;
	}

	Irp->IoStatus.Status = status;
	Irp->IoStatus.Information = info;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);

	KdPrint(("Leave HelloWDMIOControl\n"));
	return status;
}

*/