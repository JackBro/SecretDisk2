/*++
*
* Copyright (c) 2015 - 2016  �����������¿Ƽ����޹�˾
*
* Module Name:
*
*		ProcWhiteList.cpp
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
#include "ProcWhiteList.h"

FpZwQueryInformationProcess CProcWhiteList::ms_fpZwQueryInformationProcess	= NULL;
LIST_ENTRY					CProcWhiteList::ms_ListHead						= {0};
ERESOURCE					CProcWhiteList::ms_Lock							= {0};
KSPIN_LOCK					CProcWhiteList::ms_SpLock						= 0;

CProcWhiteList::CProcWhiteList()
{
	RtlZeroMemory(&m_Irql, sizeof(m_Irql));
	m_LockRef = 0;
}

CProcWhiteList::~CProcWhiteList()
{
	RtlZeroMemory(&m_Irql, sizeof(m_Irql));
	m_LockRef = 0;
}

BOOLEAN
	CProcWhiteList::Init()
{
	BOOLEAN		bRet	= FALSE;

	CKrnlStr	FuncName;


	KdPrintKrnl(LOG_PRINTF_LEVEL_INFO, LOG_RECORED_LEVEL_NEED, L"begin");

	__try
	{
		InitializeListHead(&ms_ListHead);
		ExInitializeResourceLite(&ms_Lock);
		KeInitializeSpinLock(&ms_SpLock);

		if (!FuncName.Set(L"ZwQueryInformationProcess", wcslen(L"ZwQueryInformationProcess")))
		{
			KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"FuncName.Set failed");
			__leave;
		}

		ms_fpZwQueryInformationProcess = (FpZwQueryInformationProcess)MmGetSystemRoutineAddress(FuncName.Get());
		if (!ms_fpZwQueryInformationProcess)
		{
			KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"MmGetSystemRoutineAddress failed");
			__leave;
		}

		if (!Insert(0))
		{
			KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"Insert failed. Pid(0)");
			__leave;
		}

		bRet = TRUE;
	}
	__finally
	{
		;
	}

	KdPrintKrnl(LOG_PRINTF_LEVEL_INFO, LOG_RECORED_LEVEL_NEED, L"end");

	return bRet;
}

BOOLEAN
	CProcWhiteList::Unload()
{
	BOOLEAN				bRet				= FALSE;

	LPPROC_WHITE_LIST	lpProcProtectInfo	= NULL;
	PLIST_ENTRY			pNode				= NULL;		


	KdPrintKrnl(LOG_PRINTF_LEVEL_INFO, LOG_RECORED_LEVEL_NEED, L"begin");

	__try
	{
		GetLock();

		while (!IsListEmpty(&ms_ListHead))
		{
			lpProcProtectInfo = CONTAINING_RECORD(ms_ListHead.Flink, PROC_WHITE_LIST, List);
			if (!lpProcProtectInfo)
			{
				KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"CONTAINING_RECORD failed");
				__leave;
			}

			KdPrintKrnl(LOG_PRINTF_LEVEL_INFO, LOG_RECORED_LEVEL_NEED, L"Pid(%d)",
				lpProcProtectInfo->ulPid);

			RemoveEntryList(&lpProcProtectInfo->List);
			delete lpProcProtectInfo;
			lpProcProtectInfo = NULL;
		}

		bRet = TRUE;
	}
	__finally
	{
		FreeLock();

		ExDeleteResourceLite(&ms_Lock);
		RtlZeroMemory(&ms_Lock, sizeof(ms_Lock));
	}

	KdPrintKrnl(LOG_PRINTF_LEVEL_INFO, LOG_RECORED_LEVEL_NEED, L"end");

	return bRet;
}

BOOLEAN
	CProcWhiteList::Clear()
{
	BOOLEAN				bRet				= FALSE;

	LPPROC_WHITE_LIST	lpProcProtectInfo	= NULL;


	KdPrintKrnl(LOG_PRINTF_LEVEL_INFO, LOG_RECORED_LEVEL_NEED, L"begin");

	__try
	{
		GetLock();

		while (!IsListEmpty(&ms_ListHead))
		{
			lpProcProtectInfo = CONTAINING_RECORD(ms_ListHead.Flink, PROC_WHITE_LIST, List);
			if (!lpProcProtectInfo)
			{
				KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"CONTAINING_RECORD failed");
				__leave;
			}

			KdPrintKrnl(LOG_PRINTF_LEVEL_INFO, LOG_RECORED_LEVEL_NEED, L"Pid(%d)",
				lpProcProtectInfo->ulPid);

			RemoveEntryList(&lpProcProtectInfo->List);
			delete lpProcProtectInfo;
			lpProcProtectInfo = NULL;
		}
	}
	__finally
	{
		FreeLock();
	}

	KdPrintKrnl(LOG_PRINTF_LEVEL_INFO, LOG_RECORED_LEVEL_NEED, L"end");

	return bRet;
}

#pragma warning(push)
#pragma warning(disable: 28167)
VOID
	CProcWhiteList::GetLock()
{
	__try
	{
		// �жϵ�ǰ�����ü����Ƿ�Ϸ�
		if (m_LockRef < 0)
		{
			KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"m_LockRef < 0 (%d)", m_LockRef);
			__leave;
		}

		// �ж��費��Ҫ����
		if (m_LockRef++)
			__leave;

		// ����
		m_Irql = KeGetCurrentIrql();
		if (m_Irql == DISPATCH_LEVEL)
			KeAcquireSpinLock(&ms_SpLock, &m_Irql);
		else if (m_Irql <= APC_LEVEL)
		{
#pragma warning(push)
#pragma warning(disable: 28103)
			KeEnterCriticalRegion();
			ExAcquireResourceExclusiveLite(&ms_Lock, TRUE);
#pragma warning(pop)
		}
		else
			KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"IRQL > DISPATCH_LEVEL");
	}
	__finally
	{
		;
	}

	return;
}
#pragma warning(pop)

#pragma warning(push)
#pragma warning(disable: 28167)
VOID
	CProcWhiteList::FreeLock()
{
	__try
	{
		// �жϵ�ǰ�����ü����Ƿ�Ϸ�
		if (m_LockRef <= 0)
		{
			KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"m_LockRef <= 0 (%d)", m_LockRef);
			__leave;
		}

		// �ж��費��Ҫ����
		if (--m_LockRef)
			__leave;

		// ����
		if (m_Irql == DISPATCH_LEVEL)
			KeReleaseSpinLock(&ms_SpLock, m_Irql);
		else if (m_Irql <= APC_LEVEL)
		{
#pragma warning(push)
#pragma warning(disable: 28107)
			ExReleaseResourceLite(&ms_Lock);
			KeLeaveCriticalRegion();
#pragma warning(pop)
		}
		else
			KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"IRQL > DISPATCH_LEVEL");
	}
	__finally
	{
		;
	}

	return;
}
#pragma warning(pop)

BOOLEAN
	CProcWhiteList::Insert(
	__in ULONG ulPid
	)
{
	BOOLEAN				bRet				= FALSE;

	LPPROC_WHITE_LIST	lpProcProtectInfo	= NULL;


	__try
	{
		GetLock();

		if (Get(ulPid))
		{
			KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"Already exist. Need not insert");
			__leave;
		}

		lpProcProtectInfo = new(MEMORY_TAG_PROC_WHILTE_LIST) PROC_WHITE_LIST;
		lpProcProtectInfo->ulPid = ulPid;
		InsertTailList(&ms_ListHead, &lpProcProtectInfo->List);

		KdPrintKrnl(LOG_PRINTF_LEVEL_INFO, LOG_RECORED_LEVEL_NEED, L"Pid(%d)",
			ulPid);

		bRet = TRUE;
	}
	__finally
	{
		FreeLock();
	}

	return bRet;
}

BOOLEAN
	CProcWhiteList::Delete(
	__in ULONG ulPid
	)
{
	BOOLEAN				bRet				= FALSE;

	LPPROC_WHITE_LIST	lpProcProtectInfo	= NULL;


	__try
	{
		GetLock();

		lpProcProtectInfo = Get(ulPid);
		if (!lpProcProtectInfo)
		{
			KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"not exist. Need not del. ulPid(%d)",
				ulPid);

			__leave;
		}

		RemoveEntryList(&lpProcProtectInfo->List);
		delete lpProcProtectInfo;
		lpProcProtectInfo = NULL;

		KdPrintKrnl(LOG_PRINTF_LEVEL_INFO, LOG_RECORED_LEVEL_NEED, L"Pid(%d)",
			ulPid);

		bRet = TRUE;
	}
	__finally
	{
		FreeLock();
	}

	return bRet;
}

LPPROC_WHITE_LIST
	CProcWhiteList::Get(
	__in ULONG ulPid
	)
{
	LPPROC_WHITE_LIST	lpProcProtectInfo	= NULL;

	PLIST_ENTRY			pNode				= NULL;


	__try
	{
		GetLock();

		if (IsListEmpty(&ms_ListHead))
		{
// 			KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"list empty. Pid(%d)",
// 				ulPid);

			__leave;
		}

		for (pNode = ms_ListHead.Flink; pNode != &ms_ListHead; pNode = pNode->Flink, lpProcProtectInfo = NULL)
		{
			lpProcProtectInfo = CONTAINING_RECORD(pNode, PROC_WHITE_LIST, List);
			if (!lpProcProtectInfo)
			{
				KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"CONTAINING_RECORD failed");
				__leave;
			}

			if (lpProcProtectInfo->ulPid == ulPid)
				__leave;
		}
	}
	__finally
	{
		FreeLock();
	}

	return lpProcProtectInfo;
}

BOOLEAN
	CProcWhiteList::IsIn(
	__in ULONG ulPid
	)
{
	if (Get(ulPid))
		return TRUE;
	else
		return FALSE;
}

BOOLEAN
	CProcWhiteList::GetProcPath(
	__in ULONG			ulPid,
	__in CKrnlStr	*	pProcPath,
	__in BOOLEAN		bForce
	)
{
	BOOLEAN		bRet													= FALSE;

	NTSTATUS	ntStatus												= STATUS_UNSUCCESSFUL;
	PEPROCESS	pEprocess												= NULL;
	HANDLE		hProcess												= NULL;
	ULONG		ulLen													= 0;
	WCHAR		Buffer[MAX_PATH + 2 * sizeof(ULONG) / sizeof(WCHAR)]	= {0};


	__try
	{
		if (!pProcPath)
		{
			KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"input parameter error");
			__leave;
		}

		if (!ulPid || 4 == ulPid)
		{
			bRet = TRUE;
			__leave;
		}

		if (!bForce)
			__leave;

		// ��ȡ���̶���
		ntStatus = PsLookupProcessByProcessId((HANDLE)ulPid, &pEprocess);
		if (!NT_SUCCESS(ntStatus))
		{
			KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"PsLookupProcessByProcessId failed. (%x) Pid(%d)",
				ntStatus, ulPid);

			__leave;
		}

		// ��ȡ���̾��
		ntStatus = ObOpenObjectByPointer(
			pEprocess,
			OBJ_KERNEL_HANDLE,
			NULL,
			STANDARD_RIGHTS_READ,
			NULL,
			KernelMode,
			&hProcess
			);
		if (!NT_SUCCESS(ntStatus))
		{
			KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"ObOpenObjectByPointer failed. (%x) Pid(%d)",
				ntStatus, ulPid);

			__leave;
		}

		if (!pProcPath->Init())
		{
			KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"pProcPath->Init failed");

			__leave;
		}

		// ��ȡ������Ϣ
		ntStatus = ms_fpZwQueryInformationProcess(
			hProcess,
			ProcessImageFileName,
			Buffer,
			MAX_PATH * sizeof(WCHAR) + 2 * sizeof(ULONG),
			&ulLen
			);
		if (!NT_SUCCESS(ntStatus))
		{
			KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"ms_fpZwQueryInformationProcess failed. (%x) Pid(%d)",
				ntStatus, ulPid);

			__leave;
		}

		if (!pProcPath->Set((PUNICODE_STRING)Buffer))
		{
			KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"pProcPath.Set failed. Path(%wZ)",
				Buffer);

			__leave;
		}

		bRet = TRUE;
	}
	__finally
	{
		if (hProcess)
		{
			ZwClose(hProcess);
			hProcess = NULL;
		}

		if (pEprocess)
		{
			ObDereferenceObject(pEprocess);
			pEprocess = NULL;
		}
	}

	return bRet;
}
