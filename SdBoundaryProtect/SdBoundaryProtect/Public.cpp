/*++
*
* Copyright (c) 2015 - 2016  �����������¿Ƽ����޹�˾
*
* Module Name:
*
*		Public.cpp
*
* Abstract:
*
*		��
*
* Environment:
*
*		Kernel mode
*
* Version:
*
*		��
*
* Author:
*
*		����
*
* Complete Time:
*
*		��
*
* Modify Record:
*
*		��
*
--*/

#include "stdafx.h"
#include "Public.h"

/*
* ����һ��ָ��
*
* ������
*		size		Ҫ����Ĵ�С
*		ulPoolTag	Ҫ�����ָ��ı��
*
* ����ֵ��
*		��
*
* ��ע��
*
*/
LPVOID
	__cdecl operator
	new(
	size_t	size,
	ULONG	ulPoolTag
	)
{
	LPVOID			lpPointer		= NULL;

	LARGE_INTEGER	ShortTime		= {0};


	__try
	{
		if (!size)
		{
			ASSERT(FALSE);
			__leave;
		}

		ShortTime.QuadPart = -5000;

		while (TRUE)
		{
			lpPointer = ExAllocatePoolWithTag(NonPagedPool, size, ulPoolTag);
			if (lpPointer)
			{
				RtlZeroMemory(lpPointer, size);
				break;
			}

			KeDelayExecutionThread(KernelMode, FALSE, &ShortTime);
		}
	}
	__finally
	{
		;
	}

	return lpPointer;
}

/*
* �ͷ�һ��ָ��
*
* ������
*		pPointer	Ҫ�ͷŵ�ָ��
*
* ����ֵ��
*		��
*
* ��ע��
*
*/
VOID 
	__cdecl operator
	delete(
	LPVOID lpPointer
	)
{
	if (lpPointer)
	{
		ExFreePool(lpPointer);
		lpPointer = NULL;
	}
}

/*
* �ͷ�һ��ָ��
*
* ������
*		lpPointer	Ҫ�ͷŵ�ָ��
*
* ����ֵ��
*		��
*
* ��ע��
*
*/
VOID 
	__cdecl operator
	delete[](
	LPVOID lpPointer
	)
{
	if (lpPointer)
	{
		ExFreePool(lpPointer);	
		lpPointer = NULL;
	}
}

VOID
	PrintKrnl(
	__in							LOG_PRINTF_LEVEL	PrintfLevel,
	__in							LOG_RECORED_LEVEL	RecoredLevel,
	__in							PCHAR				pFuncName,
	__in __drv_formatString(printf)	PWCHAR				Fmt,
	...
	)
{
	LARGE_INTEGER	snow				= {0};
	LARGE_INTEGER	now					= {0};
	TIME_FIELDS		nowFields			= {0};
	WCHAR			Time[64]			= {0};
	PCHAR			pchProcName			= NULL;
	WCHAR			wchProcName[20]		= {0};
	UNICODE_STRING	ustrProcName		= {0};
	ANSI_STRING		astrProcName		= {0};
	WCHAR*			pContent			= NULL;
	NTSTATUS		ntStatus			= STATUS_UNSUCCESSFUL;
	CHAR			ProcNameError[20]	= {0};
	PEPROCESS		pEprocess			= NULL;
	ULONG			ulPid				= 0;
	ULONG			ulTid				= 0;
	WCHAR*			pLogInfo			= NULL;
	WCHAR			wchPrintfLevel[10]	= {0};

	CLog			Log;

	va_list			Args;


	__try
	{
		va_start(Args, Fmt);

		// ��ñ�׼ʱ��
		KeQuerySystemTime(&snow);

		// ת��Ϊ����ʱ��
		ExSystemTimeToLocalTime(&snow, &now);

		// ת��Ϊ���ǿ�������ʱ���ʽ
		RtlTimeToTimeFields(&now, &nowFields);

		// ��ӡ���ַ�����
		ntStatus = RtlStringCchPrintfW(
			Time,
			64,
			L"[%04d-%02d-%02d][%02d:%02d:%02d.%03d]",
			nowFields.Year,
			nowFields.Month,
			nowFields.Day,
			nowFields.Hour,
			nowFields.Minute,
			nowFields.Second,
			nowFields.Milliseconds
			);
		if (!NT_SUCCESS(ntStatus))
			RtlCopyMemory(Time, L"[Date Error][Time Error]", wcslen(L"[Date Error][Time Error]") * sizeof(WCHAR));

		ulPid = (ULONG)PsGetCurrentProcessId();

		ulTid = (ULONG)PsGetCurrentThreadId();

		pEprocess = PsGetCurrentProcess();
		if (pEprocess)
			pchProcName = (PCHAR)PsGetProcessImageFileName(pEprocess);

		if (pchProcName)
		{
			astrProcName.Length = strlen(pchProcName) * sizeof(CHAR);
			astrProcName.MaximumLength = astrProcName.Length;
			astrProcName.Buffer = pchProcName;

			do 
			{
				ntStatus = RtlAnsiStringToUnicodeString(
					&ustrProcName,
					&astrProcName,
					TRUE
					);
			} while (STATUS_NO_MEMORY == ntStatus);
			
			if (!NT_SUCCESS(ntStatus))
				RtlCopyMemory(wchProcName, L"Proc Error", wcslen(L"Proc Error") * sizeof(WCHAR));
			else
				RtlCopyMemory(wchProcName, ustrProcName.Buffer, ustrProcName.Length);
		}
		else
			RtlCopyMemory(wchProcName, L"Proc Error", wcslen(L"Proc Error") * sizeof(WCHAR));

		switch (PrintfLevel)
		{
		case LOG_PRINTF_LEVEL_ERROR:
			{
				RtlCopyMemory(wchPrintfLevel, L"ERRO", wcslen(L"ERRO") * sizeof(WCHAR));
				break;
			}
		case LOG_PRINTF_LEVEL_INFO:
			{
				RtlCopyMemory(wchPrintfLevel, L"INFO", wcslen(L"INFO") * sizeof(WCHAR));
				break;
			}
		case LOG_PRINTF_LEVEL_WARNING:
			{
				RtlCopyMemory(wchPrintfLevel, L"WARN", wcslen(L"WARN") * sizeof(WCHAR));
				break;
			}
		default:
			{
				RtlCopyMemory(wchPrintfLevel, L"UNKN", wcslen(L"UNKN") * sizeof(WCHAR));
				break;
			}
		}

		pContent = (PWCHAR)new(MEMORY_TAG_PUBLIC) CHAR[MAX_PATH * 4 * sizeof(WCHAR)];
		pLogInfo = (PWCHAR)new(MEMORY_TAG_PUBLIC) CHAR[MAX_PATH * 4 * sizeof(WCHAR)];
		ntStatus = RtlStringCchVPrintfW(pContent, MAX_PATH * 4, Fmt, Args);
		if (NT_SUCCESS(ntStatus))
			ntStatus = RtlStringCchPrintfW(
			pLogInfo,
			MAX_PATH * 4,
			L"[%lS]%lS[%05d][%05d][%lS][%hs]%lS",
			wchPrintfLevel,
			Time,
			ulPid,
			ulTid,
			wchProcName,
			pFuncName,
			pContent
			);
		else
			ntStatus = RtlStringCchPrintfW(
			pLogInfo,
			MAX_PATH * 4,
			L"[%lS]%lS[%05d][%05d][%lS][%hs]%x",
			wchPrintfLevel,
			Time,
			ulPid,
			ulTid,
			wchProcName,
			pFuncName,
			ntStatus
			);

		if (!NT_SUCCESS(ntStatus))
		{
			ntStatus = RtlStringCchPrintfW(
				pLogInfo,
				MAX_PATH * 4,
				L"RtlStringCchPrintfW for pLogInfo error. (%x)",
				ntStatus
				);
		}

		if (!NT_SUCCESS(ntStatus))
		{
			DbgPrintEx(
				DPFLTR_IHVDRIVER_ID,
				DPFLTR_ERROR_LEVEL,
				"RtlStringCchPrintfW failed. (%x) \n",
				ntStatus
				);

			__leave;
		}

		if (RecoredLevel == LOG_RECORED_LEVEL_NEED)
			Log.Insert(pLogInfo, wcslen(pLogInfo));

		ntStatus = DbgPrintEx(
			DPFLTR_IHVDRIVER_ID,
			DPFLTR_ERROR_LEVEL,
			"%lS \n",
			pLogInfo
			);
		if (!NT_SUCCESS(ntStatus))
		{
			DbgPrintEx(
				DPFLTR_IHVDRIVER_ID,
				DPFLTR_ERROR_LEVEL,
				"DbgPrintEx failed. (%x) \n",
				ntStatus
				);
		}
	}
	__finally
	{
		va_end(Args);

		if (pContent)
		{
			delete[] pContent;
			pContent = NULL;
		}

		if (pLogInfo)
		{
			delete[] pLogInfo;
			pLogInfo = NULL;
		}

		if (ustrProcName.Buffer)
			RtlFreeUnicodeString(&ustrProcName);
	}

	return;
}
