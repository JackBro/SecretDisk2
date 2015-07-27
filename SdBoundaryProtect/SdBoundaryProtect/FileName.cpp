/*++
*
* Copyright (c) 2015 - 2016  �����������¿Ƽ����޹�˾
*
* Module Name:
*
*		FileName.cpp
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
#include "FileName.h"


LIST_ENTRY	CFileName::ms_ListHead		= {0};
ERESOURCE	CFileName::ms_Lock			= {0};
KSPIN_LOCK	CFileName::ms_SpLock		= 0;
CKrnlStr*	CFileName::ms_pSystemRoot	= NULL;
CKrnlStr*	CFileName::ms_pPageFileSys	= NULL;
CKrnlStr*	CFileName::ms_pLogFileSys	= NULL;
CKrnlStr*	CFileName::ms_pWmDb			= NULL;


CFileName::CFileName()
{
	RtlZeroMemory(&m_Irql, sizeof(m_Irql));
	m_LockRef = 0;
}

CFileName::~CFileName()
{
	RtlZeroMemory(&m_Irql, sizeof(m_Irql));
	m_LockRef = 0;
}

/*
* ����˵����
*		��ʼ��ģ��
*
* ������
*		��
*
* ����ֵ��
*		��
*
* ��ע��
*		��
*/
BOOLEAN
	CFileName::Init()
{
	BOOLEAN		bRet		= FALSE;

	CKrnlStr	FileName;
	CKrnlStr	VolumeFileName;


	KdPrintKrnl(LOG_PRINTF_LEVEL_INFO, LOG_RECORED_LEVEL_NEED, L"begin");

	__try
	{
		InitializeListHead(&ms_ListHead);
		ExInitializeResourceLite(&ms_Lock);
		KeInitializeSpinLock(&ms_SpLock);

		ms_pSystemRoot = new(FILE_NAME_TAG) CKrnlStr;

		if (!FileName.Set(L"\\systemroot", wcslen(L"\\systemroot")))
		{
			KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"FileName.Set failed");
			__leave;
		}

		if (!SystemRootToDev(&FileName, ms_pSystemRoot))
		{
			KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"SystemRootToDev failed. File(%wZ)",
				FileName.Get());

			__leave;
		}

		KdPrintKrnl(LOG_PRINTF_LEVEL_INFO, LOG_RECORED_LEVEL_NEED, L"ms_pSystemRoot (%wZ)",
			ms_pSystemRoot->Get());

		if (!VolumeFileName.Set(ms_pSystemRoot))
		{
			KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"VolumeFileName.Set failed. File(%wZ)",
				ms_pSystemRoot->Get());

			__leave;
		}

		if (!VolumeFileName.Shorten(ms_pSystemRoot->GetLenCh() - wcslen(L"windows")))
		{
			KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"VolumeFileName.Shorten failed. File(%wZ)",
				VolumeFileName.Get());

			__leave;
		}

		ms_pPageFileSys = new(FILE_NAME_TAG) CKrnlStr;

		if (!ms_pPageFileSys->Set(&VolumeFileName))
		{
			KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"ms_pPageFileSys->Set failed. File(%wZ)",
				VolumeFileName.Get());

			__leave;
		}

		if (!ms_pPageFileSys->Append(L"pagefile.sys", wcslen(L"pagefile.sys")))
		{
			KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"ms_pPageFileSys->Append failed. File(%wZ)",
				ms_pPageFileSys->Get());

			__leave;
		}

		ms_pLogFileSys = new(FILE_NAME_TAG) CKrnlStr;

		if (!ms_pLogFileSys->Set(&VolumeFileName))
		{
			KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"ms_pLogFileSys->Set failed. File(%wZ)",
				VolumeFileName.Get());

			__leave;
		}

		if (!ms_pLogFileSys->Append(L"$LogFile", wcslen(L"$LogFile")))
		{
			KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"ms_pLogFileSys->Append failed. File(%wZ)",
				ms_pLogFileSys->Get());

			__leave;
		}

		ms_pWmDb = new(FILE_NAME_TAG) CKrnlStr;

		bRet = TRUE;
	}
	__finally
	{
		if (!bRet)
		{
			delete ms_pSystemRoot;
			ms_pSystemRoot = NULL;

			delete ms_pPageFileSys;
			ms_pPageFileSys = NULL;

			delete ms_pLogFileSys;
			ms_pLogFileSys = NULL;

			delete ms_pWmDb;
			ms_pWmDb = NULL;
		}
	}

	KdPrintKrnl(LOG_PRINTF_LEVEL_INFO, LOG_RECORED_LEVEL_NEED, L"end");

	return bRet;
}

/*
* ����˵����
*		ж��ģ��,�����ʵ��ʱ������ø�ģ�飬�ͷž���Ϣ����
*
* ������
*		��			
*
* ����ֵ��
*		��
*
* ��ע��
*		����
*/
BOOLEAN 
	CFileName::Unload()
{
	BOOLEAN				bRet			= FALSE;

	LPVOLUME_NAME_INFO	lpVolNameInfo	= NULL;


	KdPrintKrnl(LOG_PRINTF_LEVEL_INFO, LOG_RECORED_LEVEL_NEED, L"begin");

	__try
	{
		GetLock();

		// ����������Դ
		while (!IsListEmpty(&ms_ListHead))
		{
			lpVolNameInfo = CONTAINING_RECORD(ms_ListHead.Flink, VOLUME_NAME_INFO, List);
			if (!lpVolNameInfo)
			{
				KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"CONTAINING_RECORD failed");
				__leave;
			}

			RemoveEntryList(&lpVolNameInfo->List);
			delete lpVolNameInfo;
			lpVolNameInfo = NULL;
		}

		bRet = TRUE;
	}
	__finally
	{
		delete ms_pSystemRoot;
		ms_pSystemRoot = NULL;

		delete ms_pPageFileSys;
		ms_pPageFileSys = NULL;

		delete ms_pLogFileSys;
		ms_pLogFileSys = NULL;

		delete ms_pWmDb;
		ms_pWmDb = NULL;

		FreeLock();

		// ������
		ExDeleteResourceLite(&ms_Lock);
		RtlZeroMemory(&ms_Lock, sizeof(ms_Lock));
	}

	KdPrintKrnl(LOG_PRINTF_LEVEL_INFO, LOG_RECORED_LEVEL_NEED, L"end");

	return bRet;
}

/*
* ����˵����
*		����ģ��
*
* ������
*		��			
*
* ����ֵ��
*		��
*
* ��ע��
*		����
*/
BOOLEAN 
	CFileName::Clear()
{
	BOOLEAN				bRet			= FALSE;

	LPVOLUME_NAME_INFO	lpVolNameInfo	= NULL;


	KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"begin");

	__try
	{
		GetLock();

		// ����������Դ
		while (!IsListEmpty(&ms_ListHead))
		{
			lpVolNameInfo = CONTAINING_RECORD(ms_ListHead.Flink, VOLUME_NAME_INFO, List);
			if (!lpVolNameInfo)
			{
				KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"CONTAINING_RECORD failed");
				__leave;
			}

			RemoveEntryList(&lpVolNameInfo->List);
			delete lpVolNameInfo;
			lpVolNameInfo = NULL;
		}

		bRet = TRUE;
	}
	__finally
	{
		FreeLock();
	}

	KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"end");

	return bRet;
}

/*
* ����˵����
*		�����
*
* ������
*		��
*
* ����ֵ��
*		��
*
* ��ע��
*		��
*/
#pragma warning(push)
#pragma warning(disable: 28167)
VOID
	CFileName::GetLock()
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

/*  
* ����˵����
*		�ͷ���
*
* ������
*		��
*
* ����ֵ��
*		��
*
* ��ע��
*		��
* 
*/
#pragma warning(push)
#pragma warning(disable: 28167)
VOID
	CFileName::FreeLock()
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
	CFileName::GetParentPath(
	__in	CKrnlStr * pPath,
	__out	CKrnlStr * pParentPath
	)
{
	BOOLEAN		bRet				= FALSE;

	PWCHAR		pwchPostion			= NULL;
	PWCHAR		pwchPostionStart	= NULL;
	PWCHAR		pwchPostionEnd		= NULL;
	ULONG		ulCount				= 0;


	__try
	{
		if (!pPath || !pParentPath)
		{
			KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"input parameter error. pPath(%p) pParentPath(%p)",
				pPath, pParentPath);

			__leave;
		}

		if (!pParentPath->Set(pPath))
		{
			KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"pParentPath->Set failed. Path(%wZ)",
				pPath->Get());

			__leave;
		}

		pwchPostionStart = pParentPath->GetString();
		pwchPostionEnd = pwchPostion = pParentPath->GetString() + pParentPath->GetLenCh() - 1;

		for (; pwchPostion >= pwchPostionStart; pwchPostion--)
		{
			if (L'\\' == *pwchPostion && pwchPostion != pwchPostionEnd)
				ulCount++;

			if (1 == ulCount)
			{
				if (!pParentPath->Shorten((USHORT)(pwchPostion - pwchPostionStart)))
				{
					KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"pParentPath->Shorten failed. Path(%wZ)",
						pParentPath->Get());

					__leave;
				}

				bRet = TRUE;
				__leave;
			}
			else
			{
				if (2 <= ulCount)
				{
					KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"ulCount error. ulCount(%d) Path(%wZ)",
						ulCount, pParentPath->Get());

					__leave;
				}
			}
		}
	}
	__finally
	{
		;
	}

	return bRet;
}

/*
* ����˵����
*		���ļ��ķ�����������R3��ת���豸��
*
* ������
*		pName		������������R3��	\\??\\C: ���� C:
*		pDevName	�豸��				\\Device\\HarddiskVolume1
*
* ����ֵ��
*		TRUE	�ɹ�
*		FALSE	ʧ��
*
* ��ע��
*		��
*/
BOOLEAN 
	CFileName::ToDev(
	__in		CKrnlStr*		pName,
	__inout		CKrnlStr*		pDevName,
	__in_opt	PFLT_INSTANCE	pFltInstance
	)
{
	BOOLEAN				bRet			= FALSE;

	CKrnlStr			VolAppName;
	CKrnlStr			VolSymName;
	CKrnlStr			VolDevName;
	CKrnlStr			PrePartName;
	CKrnlStr			PostPartName;

	CFileName			FileName;

	BOOLEAN				bDisk			= FALSE;
	PVOLUME_NAME_INFO	pVolNameInfo	= NULL;
	NAME_TYPE			NameType		= TYPE_UNKNOW;
	BOOLEAN				bInsertNew		= FALSE;


	__try
	{
		if (!pName || !pDevName)
		{
			KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"input parameter error");
			__leave;
		}

		if (FileName.IsSystemRootPath(pName))
		{
			if (!FileName.SystemRootToDev(pName, pDevName))
			{
				KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"SystemRootToDev failed. File(%wZ)",
					pName->Get());

				__leave;
			}

			bRet = TRUE;
			__leave;
		}

		if (!ParseAppOrSymName(pName, &PrePartName, &PostPartName, &bDisk, &NameType))
		{
			KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"Parse failed. Name(%wZ) (%d)",
				pName->Get(), NameType);

			__leave;
		}

		if (NameType == TYPE_APP)
		{
			pVolNameInfo = FileName.GetVolNameInfoByVolAppName(&PrePartName);
			if (!pVolNameInfo)
			{
				KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"FileName.GetVolNameInfoByVolAppName failed. Name(%wZ) (%d)",
					PrePartName.Get(), NameType);

				__leave;

// 				if (!pFltInstance)
// 				{
// 					KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"pFltInstance error");
// 					__leave;
// 				}
// 
// 				// ��þ����������
// 				if (!VolSymName.Set(L"\\??\\", wcslen(L"\\??\\")))
// 				{
// 					KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"VolAppName.Set failed");
// 					__leave;
// 
// 				}
// 				if (!VolSymName.Append(&PrePartName))
// 				{
// 					KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"VolAppName.Set failed. Vol(%wZ)", PrePartName.Get());
// 					__leave;
// 				}
// 
// 				if (!GetVolDevNameByQueryObj(&VolSymName, &VolDevName))
// 				{
// 					KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"GetVolDevNameByQueryObj failed. Vol(%wZ)", VolSymName.Get());
// 					__leave;
// 				}
// 
// 				if (!FileName.InsertVolNameInfo(&PrePartName, &VolSymName, &VolDevName, pFltInstance))
// 				{
// 					KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"InsertVolNameInfo failed. Vol(%wZ)", PrePartName.Get());
// 					__leave;
// 				}
// 
// 				bInsertNew = TRUE;
			}
		}
		else if (NameType == TYPE_SYM)
		{
			pVolNameInfo = FileName.GetVolNameInfoByVolSymName(&PrePartName);
			if (!pVolNameInfo)
			{
				KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"FileName.GetVolNameInfoByVolSymName failed. Name(%wZ) (%d)",
					PrePartName.Get(), NameType);

				__leave;

// 				// ��þ�R3��
// 				if (!VolAppName.Set(PrePartName.GetString() + 4, PrePartName.GetLenCh() - 4))
// 				{
// 					KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"VolAppName.Set failed. Vol(%wZ)", PrePartName.Get());
// 					__leave;
// 				}
// 
// 				if (!GetVolDevNameByQueryObj(&PrePartName, &VolDevName))
// 				{
// 					KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"GetVolDevNameByQueryObj failed. Vol(%wZ)", PrePartName.Get());
// 					__leave;
// 				}
// 
// 				if (!FileName.InsertVolNameInfo(&VolAppName, &PrePartName, &VolDevName, pFltInstance))
// 				{
// 					KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"InsertVolNameInfo failed. Vol(%wZ)", VolAppName.Get());
// 					__leave;
// 				}
// 
// 				bInsertNew = TRUE;
			}
		}
		else
			__leave;

		if (bInsertNew)
		{
			if (!pDevName->Set(&VolDevName))
			{
				KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"pDevName->Set failed. Vol(%wZ)", VolDevName.Get());
				__leave;
			}
		}
		else
		{
			if (!pDevName->Set(&pVolNameInfo->DevName))
			{
				KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"pDevName->Set failed. Vol(%wZ)", pVolNameInfo->DevName.Get());
				__leave;
			}
		}

		if (!bDisk)
		{
			if (!pDevName->Append(&PostPartName))
			{
				KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"pDevName->Append failed. Name(%wZ)", PostPartName.Get());
				__leave;
			}
		}

		bRet = TRUE;
	}
	__finally
	{
		;
	}

	return bRet;
}

/*
* ����˵����
*		���ļ����豸�������������ת��R3��
*
* ������
*		Name		�豸�������������
*		AppName		R3��
*
* ����ֵ��
*		TRUE	�ɹ�
*		FALSE	ʧ��
*
* ��ע��
*		��
*/
BOOLEAN 
	CFileName::ToApp(
	__in	CKrnlStr*		pName,
	__inout CKrnlStr*		pAppName,
	__in	PFLT_INSTANCE	pFltInstance
	)
{
	BOOLEAN				bRet			= FALSE;

	CKrnlStr			VolAppName;
	CKrnlStr			VolSymName;
	CKrnlStr			VolDevName;
	CKrnlStr			TmpName;

	CFileName			FileName;

	LPVOLUME_NAME_INFO	lpVolNameInfo	= NULL;
	USHORT				usCutOffset		= 0;
	BOOLEAN				bInsertNew		= FALSE;


	__try
	{
		FileName.GetLock();

		if (!pName || !pAppName || !pFltInstance)
		{
			KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"input parameter error");
			__leave;
		}

		if (!TmpName.Set(pName))
		{
			KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"TmpName.Set failed. File(%wZ)", pName->Get());
			__leave;
		}

		if (!TmpName.Shorten(4))
		{
			KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"TmpName.Shorten failed. File(%wZ)", TmpName.Get());
			__leave;
		}

		if (TmpName.Equal(L"\\??\\", wcslen(L"\\??\\"), FALSE))
		{
			// ����������
			if (!pAppName->Set(pName->GetString() + 4, pName->GetLenCh() - 4))
			{
				KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"pAppName->Set failed. File(%wZ)", pName->Get());
				__leave;
			}

			bRet = TRUE;
			__leave;
		}

		// �豸��
		lpVolNameInfo = FileName.GetVolNameInfo(pName, TYPE_DEV | TYPE_FULL_PATH);
		if (!lpVolNameInfo)
		{
			KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"FileName.GetVolNameInfo failed. Name(%wZ)",
				pName->Get());

			__leave;

// 			// �����ڣ�׼������
// 
// 			// ��þ�R3��
// 			if (!GetVolAppNameByQueryObj(pName, &VolAppName, &usCutOffset))
// 			{
// 				KdPrintKrnl(LOG_PRINTF_LEVEL_WARNING, LOG_RECORED_LEVEL_NEED, L"GetVolAppNameByQueryObj failed. File(%wZ)", pName->Get());
// 				__leave;
// 			}
// 
// 			// ��þ����������
// 			if (!VolSymName.Set(L"\\??\\", wcslen(L"\\??\\")))
// 			{
// 				KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"VolSymName.Set failed");
// 				__leave;
// 			}
// 
// 			if (!VolSymName.Append(&VolAppName))
// 			{
// 				KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"VolSymName.Append failed. Vol(%wZ)", VolAppName.Get());
// 				__leave;
// 			}
// 
// 			// ��þ��豸��
// 			if (!VolDevName.Set(pName))
// 			{
// 				KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"VolDevName.Set failed. Vol(%wZ)", pName->Get());
// 				__leave;
// 			}
// 
// 			if (!VolDevName.Shorten(usCutOffset / sizeof(WCHAR)))
// 			{
// 				KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"VolDevName.Shorten failed. Vol(%wZ)", VolDevName.Get());
// 				__leave;
// 			}
// 
// 			// ����
// 			if (!FileName.InsertVolNameInfo(&VolAppName, &VolSymName, &VolDevName, pFltInstance))
// 			{
// 				KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"FileName.InsertVolName failed. Vol(%wZ)", VolAppName.Get());
// 				__leave;
// 			}
// 
// 			bInsertNew = TRUE;
		}
		else
			usCutOffset = lpVolNameInfo->DevName.GetLenB();

		if (bInsertNew)
		{
			if (!pAppName->Set(&VolAppName))
			{
				KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"pAppName->Set failed. Vol(%wZ)", VolAppName.Get());
				__leave;
			}
		}
		else
		{
			if (!pAppName->Set(&lpVolNameInfo->AppName))
			{
				KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"pAppName->Set failed. Vol(%wZ)", lpVolNameInfo->AppName.Get());
				__leave;
			}
		}

		if (pName->GetLenB() > usCutOffset)
		{
			if (!pAppName->Append(pName->GetString() + usCutOffset / sizeof(WCHAR), pName->GetLenCh() - usCutOffset / sizeof(WCHAR)))
			{
				KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"pAppName->Append failed. File(%wZ)", pName->Get());
				__leave;
			}
		}

		bRet = TRUE;
	}
	__finally
	{
		FileName.FreeLock();
	}

	return bRet;
}

BOOLEAN
	CFileName::GetFileFullPathFromDataAndFltVol(
	__in	PFLT_CALLBACK_DATA		pData,
	__in	PFLT_VOLUME				pFltVol,
	__out	CKrnlStr			*	pName
	)
{
	BOOLEAN		bRet			= FALSE;

	NTSTATUS	ntStatus		= STATUS_UNSUCCESSFUL;
	ULONG		ulVolNameLen	= 0;


	__try
	{
		if (!pData || !pFltVol || !pName)
		{
			KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"input parameter error. pData(%p) pFltVol(%p) pName(%p)",
				pData, pFltVol, pName);

			__leave;
		}

		if (!GetVolDevNameFromFltVol(pFltVol, pName))
		{
			KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"GetDevNameFromFltVol failed");
			__leave;
		}

		if (pData->Iopb->TargetFileObject->FileName.Length)
		{
			if (L'\\' != *(pData->Iopb->TargetFileObject->FileName.Buffer))
			{
				if (!pName->Append(L"\\", wcslen(L"\\")))
				{
					KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"pName->Append failed. Name(%wZ)",
						pName->Get());

					__leave;
				}
			}

			if (!pName->Append(&pData->Iopb->TargetFileObject->FileName))
			{
				KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"pName->Append failed. Name(%wZ)",
					&pData->Iopb->TargetFileObject->FileName);

				__leave;
			}
		}

		if (IRP_MJ_CREATE == pData->Iopb->MajorFunction &&
			FlagOn(pData->Iopb->OperationFlags, SL_OPEN_TARGET_DIRECTORY))
		{
			if (!CFileName::GetParentPath(pName, pName))
			{
				KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"CFileName::GetParentPath failed. Name(%wZ)",
					&pData->Iopb->TargetFileObject->FileName);

				__leave;
			}
		}

		bRet = TRUE;
	}
	__finally
	{
		;
	}

	return bRet;
}

BOOLEAN
	CFileName::GetFileFullPath(
	__in	PFLT_CALLBACK_DATA		pData,
	__in	PFLT_VOLUME				pFltVol,
	__out	CKrnlStr			*	pName
	)
{
	BOOLEAN						bRet		= FALSE;

	NTSTATUS					ntStatus	= STATUS_UNSUCCESSFUL;
	PFLT_FILE_NAME_INFORMATION	pNameInfo	= NULL;


	__try
	{
		if (!pData || !pFltVol || !pName)
		{
			KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"input parameter error. pData(%p) pFltVol(%p) pName(%p)",
				pData, pFltVol, pName);

			__leave;
		}

		ntStatus = FltGetFileNameInformation( 
			pData,
			FLT_FILE_NAME_NORMALIZED |
			FLT_FILE_NAME_QUERY_ALWAYS_ALLOW_CACHE_LOOKUP,
			&pNameInfo
			);
		if (!NT_SUCCESS(ntStatus))
		{
// 			if (IRP_MJ_WRITE == pData->Iopb->MajorFunction)
// 				__leave;

			if (!CFileName::GetFileFullPathFromDataAndFltVol(pData, pFltVol, pName))
			{
				KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"FileName.GetNameFromDataAndFltVol failed. Name(%wZ)",
					&pData->Iopb->TargetFileObject->FileName);

				__leave;
			}
		}
		else
		{
			if (!pName->Set(&pNameInfo->Name))
			{
				KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"pName->Set failed. Name(%wZ)",
					&pNameInfo->Name);

				__leave;
			}
		}

		bRet = TRUE;
	}
	__finally
	{
		if (pNameInfo) 
		{
			FltReleaseFileNameInformation(pNameInfo);
			pNameInfo = NULL;
		}
	}

	return bRet;
}

/*
* ����˵����
*		ƴ���ļ���
*
* ������
*		pDirPath		�ļ������ļ���
*		pFileName		�ļ���
*		pFilePath		�ļ�ȫ·��
*
* ����ֵ��
*		TRUE	�ɹ�
*		FALSE	ʧ��
*
* ��ע��
*		��
*/
BOOLEAN
	CFileName::SpliceFilePath(
	__in	CKrnlStr*	pDirPath,
	__in	CKrnlStr*	pFileName,
	__inout	CKrnlStr*	pFilePath
	)
{
	BOOLEAN		bRet			= FALSE;

	CKrnlStr	TmpFileName;


	__try
	{
		if (!pDirPath || !pFileName || !pFilePath)
		{
			KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"input parameter error. pDirPath(%p) pFileName(%p) pFilePath(%p)", pDirPath, pFileName, pFilePath);
			__leave;
		}


		if (!pFilePath->Set(pDirPath))
		{
			KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"pFilePath->Set failed. File(%wZ)", pDirPath->Get());
			__leave;
		}

		// ��ʼ�ж�FileName�Ƿ���'\'����
		if (!TmpFileName.Set(pFilePath->GetString() + pFilePath->GetLenCh() - 1, pFilePath->GetLenCh() - (pFilePath->GetLenCh() - 1)))
		{
			KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"TmpFileName.Set failed. File(%wZ)", pFilePath->Get());
			__leave;
		}

		if (TmpFileName.Equal(L"\\", wcslen(L"\\"), FALSE))
		{
			if (!pFilePath->Shorten(pFilePath->GetLenCh() - 1))
			{
				KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"pFilePath->Shorten failed. File(%wZ)", pFilePath->Get());
				__leave;
			}
		}

		// ��ʼ�ж�pFileName�Ƿ���'\'��ʼ
		if (!TmpFileName.Set(pFileName))
		{
			KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"TmpFileName.Set failed. File(%wZ)", pFileName->Get());
			__leave;
		}

		if (!TmpFileName.Shorten(1))
		{
			KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"TmpFileName.Shorten failed. File(%wZ)", TmpFileName.Get());
			__leave;
		}

		if (!TmpFileName.Equal(L"\\", wcslen(L"\\"), FALSE))
		{
			if (!pFilePath->Append(L"\\", wcslen(L"\\")))
			{
				KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"pFilePath->Append failed. File(%wZ)", pFilePath->Get());
				__leave;
			}
		}

		if (!pFilePath->Append(pFileName))
		{
			KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"pFilePath->Append failed. File(%wZ)", pFileName->Get());
			__leave;
		}

		bRet = TRUE;
	}
	__finally
	{
		;
	}

	return bRet;
}

/*
* ����˵����
*		�ж������ļ����Ƿ���ͬ
*
* ������
*		pFileName1		�ļ�ȫ·��
*		pFileName2		�ļ�ȫ·��
*
* ����ֵ��
*		TRUE	��ͬ
*		FALSE	����ͬ
*
* ��ע��
*		��
*/
BOOLEAN
	CFileName::EqualPureFileName(
	__in CKrnlStr* pFileName1,
	__in CKrnlStr* pFileName2
	)
{
	BOOLEAN bRet		= FALSE;

	PWCHAR	pPosation1	= NULL;
	PWCHAR	pPosation2	= NULL;
	USHORT	usLen1		= 0;
	USHORT	usLen2		= 0;


	__try
	{
		if (!pFileName1 || !pFileName2)
		{
			KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"input parameter error. pFileName1(%p) pFileName2(%p)", pFileName1, pFileName2);
			__leave;
		}

		pPosation1 = pFileName1->GetString() + pFileName1->GetLenCh() - 1;
		while (pPosation1 >= pFileName1->GetString())
		{
			if (RtlEqualMemory(pPosation1, "\\", sizeof(WCHAR)))
			{
				usLen1 = pFileName1->GetLenCh() - (USHORT)(pPosation1 - pFileName1->GetString()) - 1;
				break;
			}

			pPosation1--;
		}

		pPosation2 = pFileName2->GetString() + pFileName2->GetLenCh() - 1;
		while (pPosation2 >= pFileName2->GetString())
		{
			if (RtlEqualMemory(pPosation2, "\\", sizeof(WCHAR)))
			{
				usLen2 = pFileName2->GetLenCh() - (USHORT)(pPosation2 - pFileName2->GetString()) - 1;
				break;
			}

			pPosation2--;
		}

		if (pPosation1 < pFileName1->GetString() && pPosation2 < pFileName2->GetString())
		{
			if (!pFileName1->Equal(pFileName2, TRUE))
				__leave;
		}
		else if (pPosation1 < pFileName1->GetString() && pPosation2 >= pFileName2->GetString())
		{
			if (!RtlEqualMemory(pFileName1->GetString(), pPosation2, sizeof(WCHAR) * usLen2))
				__leave;
		}
		else if (pPosation1 >= pFileName1->GetString() && pPosation2 < pFileName2->GetString())
		{
			if (!RtlEqualMemory(pPosation1, pFileName2->GetString(), sizeof(WCHAR) * usLen1))
				__leave;
		}
		else if (pPosation1 >= pFileName1->GetString() && pPosation2 >= pFileName2->GetString())
		{
			if (!RtlEqualMemory(pPosation1, pPosation2, sizeof(WCHAR) * usLen1))
				__leave;
		}

		bRet = TRUE;
	}
	__finally
	{
		;
	}

	return bRet;
}

BOOLEAN
	CFileName::GetVolDevNameFromFltVol(
	__in	PFLT_VOLUME		pFltVol,
	__out	CKrnlStr	*	pName
	)
{
	BOOLEAN		bRet			= FALSE;

	NTSTATUS	ntStatus		= STATUS_UNSUCCESSFUL;
	ULONG		ulDevNameLen	= 0;


	__try
	{
		if (!pFltVol || !pName)
		{
			KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"input parameter error. pFltVol(%p) pName(%p)",
				pFltVol, pName);

			__leave;
		}

		ntStatus = FltGetVolumeName(
			pFltVol,
			NULL,
			&ulDevNameLen
			);
		if (NT_SUCCESS(ntStatus))
		{
			KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"FltGetVolumeName succeed");
			__leave;
		}

		if (STATUS_BUFFER_TOO_SMALL != ntStatus)
		{
			KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"STATUS_BUFFER_TOO_SMALL != ntStatus. (%x)",
				ntStatus);

			__leave;
		}

		if (!pName->Lengthen((USHORT)ulDevNameLen / sizeof(WCHAR)))
		{
			KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"pName->Lengthen failed.");
			__leave;
		}

		ntStatus = FltGetVolumeName(
			pFltVol,
			pName->Get(),
			NULL
			);
		if (!NT_SUCCESS(ntStatus))
		{
			KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"FltGetVolumeName failed. (%x)",
				ntStatus);

			__leave;
		}

		bRet = TRUE;
	}
	__finally
	{
		;
	}

	return bRet;	
}

/*
* ����˵����
*		����һ��VOLUME_NAME_INFO
*
* ������
*		pName		���R3��		C:
*		pSymName	��ķ���������	\\?\\C:
*		pDevName	����豸��		\\Device\\HarddiskVolume1
*
* ����ֵ��
*		TRUE	�ɹ�
*		FALSE	ʧ��
*
* ��ע��
*		��
*/
BOOLEAN
	CFileName::InsertVolNameInfo(
	__in		CKrnlStr*		pAppName,
	__in		CKrnlStr*		pSymName,
	__in		CKrnlStr*		pDevName,
	__in		BOOLEAN			bOnlyDevName,
	__in		BOOLEAN			bRemoveable,
	__in_opt	PFLT_INSTANCE	pFltInstance,
	__in		ULONG			ulSectorSize,
	__in_opt	PBOOLEAN		pbModify,
	__in_opt	CKrnlStr*		pOldDevName
	)
{
	BOOLEAN				bRet			= FALSE;

	LPVOLUME_NAME_INFO	lpVolNameInfo	= NULL;
	BOOLEAN				GetOld			= FALSE;


	__try
	{
		GetLock();

		if (!pAppName || !pSymName || !pDevName)
		{
			KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"input parameter error. pAppName(%p) pSymName(%p) pDevName(%p)",
				pAppName, pSymName, pDevName);

			__leave;
		}

		lpVolNameInfo = GetVolNameInfo(pAppName, TYPE_APP);
		if (lpVolNameInfo)
		{
			GetOld = TRUE;

			if (pFltInstance && lpVolNameInfo->pFltInstance && lpVolNameInfo->pFltInstance != pFltInstance)
			{
				if (pbModify)
					*pbModify = TRUE;

				lpVolNameInfo->pFltInstance = pFltInstance;
			}

			if (!lpVolNameInfo->DevName.Equal(pDevName, TRUE))
			{
				if (pbModify)
					*pbModify = TRUE;

				if (pOldDevName)
				{
					if (!pOldDevName->Set(&lpVolNameInfo->DevName))
					{
						KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"pOldDevName->Set failed. Volume(%wZ)",
							lpVolNameInfo->DevName.Get());

						__leave;
					}
				}

				if (!lpVolNameInfo->DevName.Set(pDevName))
				{
					KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"lpVolNameInfo->DevName.Set failed");
					__leave;
				}

				if (!lpVolNameInfo->DevName.ToUpper())
				{
					KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"lpVolNameInfo->DevName.ToUpper failed");
					__leave;
				}
			}
			else
			{
				if (pbModify && *pbModify && pOldDevName)
				{
					if (!pOldDevName->Set(&lpVolNameInfo->DevName))
					{
						KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"pOldDevName->Set failed. Volume(%wZ)",
							lpVolNameInfo->DevName.Get());

						__leave;
					}
				}
			}

			bRet = TRUE;
			__leave;
		}

		lpVolNameInfo = new(FILE_NAME_TAG) VOLUME_NAME_INFO;

		if (!pFltInstance)
		{
			KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"pFltInstance error");
			__leave;
		}

		lpVolNameInfo->pFltInstance = pFltInstance;
		lpVolNameInfo->ulSectorSize	= ulSectorSize;

		if (!lpVolNameInfo->AppName.Set(pAppName))
		{
			KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"lpVolNameInfo->AppName.Set failed");
			__leave;
		}

		if (!lpVolNameInfo->AppName.ToUpper())
		{
			KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"lpVolNameInfo->AppName.ToUpper failed");
			__leave;
		}

		if (!lpVolNameInfo->SymName.Set(pSymName))
		{
			KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"lpVolNameInfo->SymName.Set failed");
			__leave;
		}

		if (!lpVolNameInfo->SymName.ToUpper())
		{
			KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"lpVolNameInfo->SymName.ToUpper failed");
			__leave;
		}

		if (!lpVolNameInfo->DevName.Set(pDevName))
		{
			KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"lpVolNameInfo->DevName.Set failed");
			__leave;
		}

		if (!lpVolNameInfo->DevName.ToUpper())
		{
			KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"lpVolNameInfo->DevName.ToUpper failed");
			__leave;
		}

		lpVolNameInfo->bRemoveable = bRemoveable;
		lpVolNameInfo->bOnlyDevName = bOnlyDevName;

		InsertTailList(&ms_ListHead, &lpVolNameInfo->List);

		bRet = TRUE;
	}
	__finally
	{
		if (!bRet && !GetOld)
		{
			delete lpVolNameInfo;
			lpVolNameInfo = NULL;
		}

		FreeLock();
	}

	return bRet;
}

/*
* ����˵����
*		ɾ��һ��VOLUME_NAME_INFO
*
* ������
*		pName		����豸��		\\Device\\HarddiskVolume1
*
* ����ֵ��
*		TRUE	�ɹ�
*		FALSE	ʧ��
*
* ��ע��
*		��
*/
BOOLEAN
	CFileName::DelVolNameInfo(
	__in CKrnlStr* pName
	)
{
	BOOLEAN				bRet			= FALSE;

	LPVOLUME_NAME_INFO	lpVolNameInfo	= NULL;


	__try
	{
		GetLock();

		if (IsListEmpty(&ms_ListHead))
		{
			KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"List empty. Can not del");
			__leave;
		}

		lpVolNameInfo = GetVolNameInfoByVolDevName(pName);
		if (!lpVolNameInfo)
		{
			KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"Not exsit. Can not del. Vol(%wZ)", pName->Get());
			__leave;
		}

		RemoveEntryList(&lpVolNameInfo->List);
		delete lpVolNameInfo;
		lpVolNameInfo = NULL;

		bRet = TRUE;
	}
	__finally
	{
		FreeLock();
	}

	return bRet;
}

/*
* ����˵����
*		���ָ�����VOLUME_NAME_INFO
*
* ������
*		pName		���������ļ�ȫ·��
*		VolNameType	����ľ�������
*
* ����ֵ��
*		LPVOLUME_NAME_INFO
*		NULL	ʧ��
*		!NULL	�ɹ�
*
* ��ע��
*		��
*/
LPVOLUME_NAME_INFO 
	CFileName::GetVolNameInfo(
	__in CKrnlStr*		pName,
	__in NAME_TYPE		NameType
	)
{
	LPVOLUME_NAME_INFO	lpVolNameInfo	= NULL;

	CFileName			FileName;

	CKrnlStr			VolNameEx;

	PLIST_ENTRY			pNode			= NULL;


	__try
	{
		FileName.GetLock();

		if (!pName || !pName->Get())
		{
			KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"input parameter error");
			__leave;
		}

		if (IsListEmpty(&FileName.ms_ListHead))
		{
			// ����ӡ��Ӱ����־�鿴
			// KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"List empty. Can not get");
			__leave;
		}

		if (!FlagOn(NameType, TYPE_UNKNOW))
			NameType |= TYPE_UNKNOW;

		for (pNode = FileName.ms_ListHead.Flink; pNode != &FileName.ms_ListHead; pNode = pNode->Flink, lpVolNameInfo = NULL)
		{
			lpVolNameInfo = CONTAINING_RECORD(pNode, VOLUME_NAME_INFO, List);
			if (!lpVolNameInfo)
			{
				KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"CONTAINING_RECORD failed");
				__leave;
			}

			if (!FlagOn(NameType, TYPE_FULL_PATH))
			{
				if (FlagOn(NameType, TYPE_APP))
				{
					if (lpVolNameInfo->AppName.Equal(pName, TRUE))
						__leave;
				}

				if (FlagOn(NameType, TYPE_SYM))
				{
					if (lpVolNameInfo->SymName.Equal(pName, TRUE))
						__leave;
				}

				if (FlagOn(NameType, TYPE_DEV))
				{
					if (lpVolNameInfo->DevName.Equal(pName, TRUE))
						__leave;
				}
			}
			else
			{
				if (FlagOn(NameType, TYPE_APP))
				{
					if (!VolNameEx.Set(&lpVolNameInfo->AppName))
					{
						KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"VolNameEx.Set failed. Name(%wZ)", pName->Get());
						__leave;
					}

					if (!VolNameEx.Append(L"*", wcslen(L"*")))
					{
						KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"VolNameEx.Append failed. Name(%wZ)", VolNameEx.Get());
						__leave;
					}

					if (FsRtlIsNameInExpression(
						VolNameEx.Get(),
						pName->Get(),
						TRUE,
						NULL
						))
						__leave;
				}

				if (FlagOn(NameType, TYPE_SYM))
				{
					if (!VolNameEx.Set(&lpVolNameInfo->SymName))
					{
						KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"VolNameEx.Set failed. Name(%wZ)", pName->Get());
						__leave;
					}

					if (!VolNameEx.Append(L"*", wcslen(L"*")))
					{
						KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"VolNameEx.Append failed. Name(%wZ)", VolNameEx.Get());
						__leave;
					}

					if (FsRtlIsNameInExpression(
						VolNameEx.Get(),
						pName->Get(),
						TRUE,
						NULL
						))
						__leave;
				}

				if (FlagOn(NameType, TYPE_DEV))
				{
					if (!VolNameEx.Set(&lpVolNameInfo->DevName))
					{
						KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"VolNameEx.Set failed. Name(%wZ)", pName->Get());
						__leave;
					}

					if (VolNameEx.GetLenCh() == pName->GetLenCh())
					{
						if (VolNameEx.Equal(pName, TRUE))
							__leave;
					}
					else if(VolNameEx.GetLenCh() < pName->GetLenCh())
					{
						if (!VolNameEx.Append(L"\\*", wcslen(L"\\*")))
						{
							KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"VolNameEx.Append failed. Name(%wZ)", VolNameEx.Get());
							__leave;
						}

						if (FsRtlIsNameInExpression(
							VolNameEx.Get(),
							pName->Get(),
							TRUE,
							NULL
							))
							__leave;
					}
				}
			}
		}
	}
	__finally
	{
		FileName.FreeLock();
	}

	return lpVolNameInfo;
}

/*
* ����˵����
*		����һ��R3���߷���������
*
* ������
*		pName		�ļ�R3���߷���������
*		pVolName	�ļ����ھ�
*		pPartName	�ļ�ȥ��������ʣ�ಿ��
*		pbDisk		��������Ƿ����̷�
*
* ����ֵ��
*		TRUE	�ɹ�
*		FALSE	ʧ��
*
* ��ע��
*		û�н������ں��豸��д������
*		��Ϊ���޷�ֱ�����ֱ���Ӳ�̺ͱ���U�̵��豸��
*		���ж��Ƿ��ǿ��ƶ����ʣ��ٽ�һ�����֣�̫������
*/
BOOLEAN
	CFileName::ParseAppOrSymName(
	__in	CKrnlStr*	pName,
	__inout CKrnlStr*	pVolName,
	__inout CKrnlStr*	pPartName,
	__inout PBOOLEAN	pbDisk,
	__inout	PNAME_TYPE	NameType
	)
{
	BOOLEAN	bRet		= FALSE;

	PWCHAR	pPosition	= NULL;


	__try
	{
		if (!pName || !pVolName || !pPartName || !pbDisk || !NameType)
		{
			KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"input parameter error");
			__leave;
		}

		if (!pVolName->Set(pName))
		{
			KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"pVolName->Set failed. Name(%wZ)",
				pName->Get());

			__leave;
		}

		if ((*pVolName->GetString() >= L'a' && *pVolName->GetString() <= L'z')
			||
			(*pVolName->GetString() >= L'A' && *pVolName->GetString() <= L'Z'))
		{
			// R3
			if (pVolName->GetLenCh() == 1)
			{
				if (!pVolName->Append(L":", wcslen(L":")))
				{
					KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"pVolName->Append failed. Name(%wZ)",
						pVolName->Get());

					__leave;
				}

				*NameType = TYPE_APP;
				*pbDisk = TRUE;
				bRet = TRUE;
				__leave;
			}
			else if (pName->GetLenCh() == 2)
			{
				*NameType = TYPE_APP;
				*pbDisk = TRUE;
				bRet = TRUE;
				__leave;
			}
			else
			{
				if (!pVolName->Shorten(2))
				{
					KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"pVolName->Shorten failed. Name(%wZ)",
						pVolName->Get());

					__leave;
				}

				if (!pPartName->Set(pName->GetString() + 2, pName->GetLenCh() - 2))
				{
					KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"pPartName->Set failed. Name(%wZ)",
						pName->Get());

					__leave;
				}
			}	

			*NameType = TYPE_APP;
			bRet = TRUE;
			__leave;
		}

		if (*(pVolName->GetString() + 1) == L'?')
		{
			// Sym
			if (pVolName->GetLenCh() == 5)
			{
				// \??\c
				if (!pVolName->Append(L":", wcslen(L":")))
				{
					KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"pVolName->Append failed. Name(%wZ)",
						pVolName->Get());

					__leave;
				}

				*pbDisk = TRUE;
			}
			else if (pVolName->GetLenCh() == 6)
			{
				// \??\c:
				*pbDisk = TRUE;
			}
			else
			{
				if (*(pVolName->GetString() + 6) == L'\\')
				{
					// \??\c:\	
					if (!pVolName->Shorten(6))
					{
						KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"pVolName->Shorten failed. Name(%wZ)",
							pVolName->Get());

						__leave;
					}

					if (!pPartName->Set(pName->GetString() + 6, pName->GetLenCh() - 6))
					{
						KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"pPartName->Set failed. Name(%wZ)",
							pName->Get());

						__leave;
					}
				}
				else
				{
					if (pVolName->GetLenCh() <= 7)
					{
						KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"pVolName len error. Name(%wZ)",
							pVolName->Get());

						__leave;
					}

					pPosition = pVolName->SearchCharacter(
						L'\\',
						pVolName->GetString() + 7,
						pVolName->GetString() + pVolName->GetLenCh() - 1
						);
					if (!pPosition)
					{
						// \??\volume{}
						*pbDisk = TRUE;
					}
					else
					{
						// \??\volume{}\	
						if (!pPartName->Set(pPosition, wcslen(pPosition)))
						{
							KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"pPartName->Set failed. Name(%lS)",
								pPosition);

							__leave;
						}
						if (!pVolName->Shorten((USHORT)(pPosition - pVolName->GetString())))
						{
							KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"pVolName->Shorten failed. Name(%wZ)",
								pVolName->Get());

							__leave;
						}
					}
				}
			}

			*NameType = TYPE_SYM;
			bRet = TRUE;
		}
	}
	__finally
	{
		;
	}

	return bRet;
}

/*
* ����˵����
*		���ָ�����VOLUME_NAME_INFO
*
* ������
*		pName	���R3��
*
* ����ֵ��
*		LPVOLUME_NAME_INFO
*		NULL	ʧ��
*		!NULL	�ɹ�
*
* ��ע��
*		��
*/
LPVOLUME_NAME_INFO 
	CFileName::GetVolNameInfoByVolAppName(
	__in CKrnlStr* pName
	)
{
	LPVOLUME_NAME_INFO	lpVolNameInfo	= NULL;

	PLIST_ENTRY			pNode			= NULL;


	__try
	{
		GetLock();

		if (!pName || !pName->Get())
		{
			KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"input parameter error");
			__leave;
		}

		if (IsListEmpty(&ms_ListHead))
		{
			KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"List empty. Can not get. Vol(%wZ)", pName->Get());
			__leave;
		}

		for (pNode = ms_ListHead.Flink; pNode != &ms_ListHead; pNode = pNode->Flink, lpVolNameInfo = NULL)
		{
			lpVolNameInfo = CONTAINING_RECORD(pNode, VOLUME_NAME_INFO, List);
			if (!lpVolNameInfo)
			{
				KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"CONTAINING_RECORD failed. Vol(%wZ)", pName->Get());
				__leave;
			}

			if (lpVolNameInfo->AppName.Equal(pName, TRUE))
				__leave;
		}
	}
	__finally
	{
		FreeLock();
	}

	return lpVolNameInfo;
}

/*
* ����˵����
*		���ָ�����VOLUME_NAME_INFO
*
* ������
*		pName	��ķ���������
*
* ����ֵ��
*		LPVOLUME_NAME_INFO
*		NULL	ʧ��
*		!NULL	�ɹ�
*
* ��ע��
*		��
*/
LPVOLUME_NAME_INFO 
	CFileName::GetVolNameInfoByVolSymName(
	__in CKrnlStr* pName
	)
{
	LPVOLUME_NAME_INFO	lpVolNameInfo	= NULL;

	PLIST_ENTRY			pNode			= NULL;


	__try
	{
		GetLock();

		if (!pName || !pName->Get())
		{
			KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"input parameter error");
			__leave;
		}

		if (IsListEmpty(&ms_ListHead))
		{
			KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"List empty. Can not get. Vol(%wZ)", pName->Get());
			__leave;
		}

		for (pNode = ms_ListHead.Flink; pNode != &ms_ListHead; pNode = pNode->Flink, lpVolNameInfo = NULL)
		{
			lpVolNameInfo = CONTAINING_RECORD(pNode, VOLUME_NAME_INFO, List);
			if (!lpVolNameInfo)
			{
				KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"CONTAINING_RECORD failed. Vol(%wZ)", pName->Get());
				__leave;
			}

			if (lpVolNameInfo->SymName.Equal(pName, TRUE))
				__leave;
		}
	}
	__finally
	{
		FreeLock();
	}

	return lpVolNameInfo;
}

/*
* ����˵����
*		���ָ�����VOLUME_NAME_INFO
*
* ������
*		pName	����豸��
*
* ����ֵ��
*		LPVOLUME_NAME_INFO
*		NULL	ʧ��
*		!NULL	�ɹ�
*
* ��ע��
*		��
*/
LPVOLUME_NAME_INFO 
	CFileName::GetVolNameInfoByVolDevName(
	__in CKrnlStr* pName
	)
{
	LPVOLUME_NAME_INFO	lpVolNameInfo	= NULL;

	PLIST_ENTRY			pNode			= NULL;


	__try
	{
		GetLock();

		if (!pName || !pName->Get())
		{
			KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"input parameter error");
			__leave;
		}

		if (IsListEmpty(&ms_ListHead))
		{
			KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"List empty. Can not get. Vol(%wZ)", pName->Get());
			__leave;
		}

		for (pNode = ms_ListHead.Flink; pNode != &ms_ListHead; pNode = pNode->Flink, lpVolNameInfo = NULL)
		{
			lpVolNameInfo = CONTAINING_RECORD(pNode, VOLUME_NAME_INFO, List);
			if (!lpVolNameInfo)
			{
				KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"CONTAINING_RECORD failed. Vol(%wZ)", pName->Get());
				__leave;
			}

			if (lpVolNameInfo->DevName.Equal(pName, TRUE))
				__leave;
		}
	}
	__finally
	{
		FreeLock();
	}

	return lpVolNameInfo;
}

BOOLEAN
	CFileName::GetVolDevNameByQueryObj(
	__in	CKrnlStr * pName,
	__out	CKrnlStr * pDevName
	)
{
	BOOLEAN				bRet		= FALSE;

	OBJECT_ATTRIBUTES	Oa			= {0};
	NTSTATUS			ntStatus	= STATUS_UNSUCCESSFUL;
	HANDLE				Handle		= NULL;


	__try
	{
		if (!pName || !pDevName)
		{
			KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"input parameter error. pName(%p) pDevName(%p)",
				pName, pDevName);

			__leave;
		}

		InitializeObjectAttributes(
			&Oa, 
			pName->Get(),
			OBJ_CASE_INSENSITIVE ,
			NULL, 
			NULL
			);

		ntStatus = ZwOpenSymbolicLinkObject(
			&Handle, 
			GENERIC_READ, 
			&Oa
			);
		if (!NT_SUCCESS(ntStatus))
		{
			KdPrintKrnl(LOG_PRINTF_LEVEL_WARNING, LOG_RECORED_LEVEL_NEED, L"ZwOpenSymbolicLinkObject failed. (%x) Name(%wZ)",
				ntStatus, pName->Get());

			__leave;
		}

		if (!pDevName->Init())
		{
			KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"pDevName.Init failed");
			__leave;
		}

		ntStatus = ZwQuerySymbolicLinkObject(
			Handle, 
			pDevName->Get(), 
			NULL
			);
		if (!NT_SUCCESS(ntStatus))
		{
			KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"ZwQuerySymbolicLinkObject failed. (%x) Name(%wZ)",
				ntStatus, pName->Get());

			__leave;
		}

		bRet = TRUE;
	}
	__finally
	{
		if (Handle)
		{
			ZwClose(Handle);
			Handle = NULL;
		}
	}

	return bRet;
}

/*
* ����˵����
*		��þ��R3��
*
* ������
*		pName			�ļ����豸��
*		pAppName		���R3��
*		pulCutOffset	����豸������
*
* ����ֵ��
*		TRUE	�ɹ�
*		FALSE	ʧ��
*
* ��ע��
*		��
*/
BOOLEAN
	CFileName::GetVolAppNameByQueryObj(
	__in	CKrnlStr*	pName,
	__inout CKrnlStr*	pAppName,
	__inout PUSHORT		pusCutOffset
	)
{
	BOOLEAN			bRet			=FALSE;

	PWCHAR			pVolume			= NULL;
	CKrnlStr		VolumeSym;
	CKrnlStr		VolumeDevEx;	


	__try
	{
		if (!pName || !pName->Get() || !pAppName || !pusCutOffset)
		{
			KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"[ToApp] : input parameter error");
			__leave;
		}

		// \\??\\A:
		if (!VolumeSym.Set(L"\\??\\A:", wcslen(L"\\??\\A:")))
		{
			KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"[ToApp] : VolumeSym.Set failed");
			__leave;
		}

		pVolume = VolumeSym.GetString() + 4;
		for (; *pVolume <= L'Z'; (*pVolume)++)
		{
			if (!CFileName::GetVolDevNameByQueryObj(&VolumeSym, &VolumeDevEx))
				continue;

			// \\Device\\HarddiskVolume1\\*
			if (!VolumeDevEx.Append(L"\\*", wcslen(L"\\*")))
			{
				KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"[ToApp] : VolumeDevEx.Append failed. Vol(%wZ)", VolumeDevEx.Get());
				__leave;
			}

			// ת��д
			if (!VolumeDevEx.ToUpper())
			{
				KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"[ToApp] : VolumeDevEx.ToUpper failed");
				__leave;
			}

			if (FsRtlIsNameInExpression(
				VolumeDevEx.Get(),
				pName->Get(),
				TRUE,
				NULL
				))
			{
				// ��þ��R3��
				if (!pAppName->Set(pVolume, wcslen(pVolume)))
				{
					KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"[ToApp] : AppName->Set failed. Vol(%lS)", pVolume);
					__leave;
				}

				*pusCutOffset = VolumeDevEx.GetLenCh() - 2;
				bRet = TRUE;
				break;
			}

			if (!VolumeDevEx.Clean())
				KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"[ToApp] : VolumeDevEx.Clean failed. Vol(%wZ)", VolumeDevEx.Get());
		}
	}
	__finally
	{
		;
	}

	return bRet;
}

BOOLEAN
	CFileName::GetPureFileName(
	__in CKrnlStr* pFileName,
	__in CKrnlStr* pPureName
	)
{
	BOOLEAN bRet		 = FALSE;

	PWCHAR	pPosition	= NULL;


	__try
	{
		if (!pFileName || !pPureName)
		{
			KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"input parameter error. pFileName(%p) pPureName(%p)",
				pFileName, pPureName);

			__leave;
		}

		pPosition = pFileName->GetString() + pFileName->GetLenCh() - 1;
		while (pPosition >= pFileName->GetString())
		{
			if (RtlEqualMemory(pPosition, "\\", sizeof(WCHAR)))
			{
				if (!pPureName->Set(pPosition + 1, wcslen(pPosition) - 1))
				{
					KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"pPureName->Set failed. File(%wZ)",
						pFileName->Get());

					__leave;
				}

				bRet = TRUE;
				__leave;
			}

			pPosition--;
		}
	}
	__finally
	{
		;
	}

	return bRet;
}

BOOLEAN
	CFileName::GetExt(
	__in CKrnlStr* pFileName,
	__in CKrnlStr* pExt
	)
{
	BOOLEAN bRet		 = FALSE;

	PWCHAR	pPosition	= NULL;


	__try
	{
		if (!pFileName || !pExt)
		{
			KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"input parameter error. pFileName(%p) pExt(%p)",
				pFileName, pExt);

			__leave;
		}

		pPosition = pFileName->GetString() + pFileName->GetLenCh() - 1;
		while (pPosition >= pFileName->GetString())
		{
			if (RtlEqualMemory(pPosition, ".", sizeof(WCHAR)))
			{
				if (!pExt->Set(pPosition + 1, wcslen(pPosition) - 1))
				{
					KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"pExt->Set failed. File(%wZ)",
						pFileName->Get());

					__leave;
				}

				bRet = TRUE;
				__leave;
			}

			pPosition--;
		}
	}
	__finally
	{
		;
	}

	return bRet;
}

BOOLEAN
	CFileName::IsVolume(
	__in PFLT_CALLBACK_DATA		pData,
	__in CKrnlStr			*	pFileName
	)
{
	BOOLEAN				bRet			= FALSE;

	LPVOLUME_NAME_INFO	lpVolNameInfo	= NULL;
	PLIST_ENTRY			pNode			= NULL;

	CKrnlStr			FileName;


	__try
	{
		GetLock();

		if (!pData || !pFileName)
		{
			KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"input parameter error. pData(%p) pFileName(%p)",
				pData, pFileName);

			__leave;
		}
		 
		if (IRP_MJ_CREATE == pData->Iopb->MajorFunction &&
			FlagOn(pData->Iopb->TargetFileObject->Flags, FO_VOLUME_OPEN))
		{
			bRet = TRUE;
			__leave;
		}

		if (IsListEmpty(&ms_ListHead))
		{
			KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"List empty. File(%wZ)",
				pFileName->Get());

			__leave;
		}

		if (!FileName.Set(pFileName))
		{
			KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"FileName.Set. File(%wZ)",
				pFileName->Get());

			__leave;
		}

		if (L'\\' == *(FileName.GetString() + FileName.GetLenCh() - 1))
		{
			if (!FileName.Shorten(FileName.GetLenCh() - 1))
			{
				KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"FileName.Shorten. File(%wZ)",
					FileName.Get());

				__leave;
			}
		}

		for (pNode = ms_ListHead.Flink; pNode != &ms_ListHead; pNode = pNode->Flink, lpVolNameInfo = NULL)
		{
			lpVolNameInfo = CONTAINING_RECORD(pNode, VOLUME_NAME_INFO, List);
			if (!lpVolNameInfo)
			{
				KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"CONTAINING_RECORD failed. File(%wZ)",
					pFileName->Get());
				__leave;

			}

			if (lpVolNameInfo->DevName.GetLenCh() == FileName.GetLenCh() &&
				lpVolNameInfo->DevName.Equal(&FileName, TRUE))
			{
				bRet = TRUE;
				__leave;
			}
		}
	}
	__finally
	{
		FreeLock();
	}

	return bRet;
}

BOOLEAN
	CFileName::IsSystemRootPath(
	__in CKrnlStr* pFileName
	)
{
	BOOLEAN		bRet		= FALSE;

	CKrnlStr	FileName;


	__try
	{
		if (!pFileName)
		{
			KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"CFileName::IsSystemRootPath input parameter error");
			__leave;
		}

		if (pFileName->GetLenCh() < 11)
			__leave;

		if (!FileName.Set(pFileName))
		{
			KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"CFileName::IsSystemRootPath FileName.Set failed. File(%wZ)",
				pFileName->Get());

			__leave;
		}

		if (!FileName.Shorten(11))
		{
			KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"CFileName::IsSystemRootPath FileName.Shorten failed. File(%wZ)",
				pFileName->Get());

			__leave;
		}

		bRet = FileName.Equal(L"\\systemroot", wcslen(L"\\systemroot"), TRUE);
	}
	__finally
	{
		;
	}

	return bRet;
}

BOOLEAN
	CFileName::SystemRootToDev(
	__in	CKrnlStr* pFileName,
	__inout CKrnlStr* pFileNameDev
	)
{
	BOOLEAN bRet = FALSE;

	CKrnlStr FileNameSystemRoot;
	CKrnlStr FileNameTmpLong;
	CKrnlStr FileNameTmpShort;
	CKrnlStr FileNameWindows;

	PWCHAR	pPosition = NULL;


	__try
	{
		GetLock();

		if (!pFileName || !pFileNameDev)
		{
			KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"input parameter error. pFileName(%p) pFileNameDev(%p)",
				pFileName, pFileNameDev);

			__leave;
		}

		if (!FileNameSystemRoot.Set(pFileName))
		{
			KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"FileNameSystemRoot.Set failed. File(%wZ)",
				pFileName->Get());

			__leave;
		}

		if (!FileNameSystemRoot.Shorten(11))
		{
			KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"FileNameSystemRoot.Shorten failed. File(%wZ)",
				FileNameSystemRoot.Get());

			__leave;
		}

		if (!ConvertByZwQuerySymbolicLinkObject(&FileNameSystemRoot, &FileNameTmpLong))
		{
			KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"ConvertByZwQuerySymbolicLinkObject failed. File(%wZ)",
				FileNameSystemRoot.Get());

			__leave;
		}

		if (!FileNameTmpShort.Set(&FileNameTmpLong))
		{
			KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"FileNameTmpShort.Set failed. File(%wZ)",
				FileNameTmpLong.Get());

			__leave;
		}

		for (pPosition = FileNameTmpShort.GetString() + FileNameTmpShort.GetLenCh() - 1; pPosition >= FileNameTmpShort.GetString(); pPosition--)
		{
			if (*pPosition == L'\\')
				break;
		}

		if (!FileNameWindows.Set(pPosition, FileNameTmpShort.GetLenCh() - (USHORT)(pPosition - FileNameTmpShort.GetString())))
		{
			KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"FileNameWindows.Set failed. Windows(%lS)",
				pPosition);

			__leave;
		}

		if (!FileNameTmpShort.Shorten((USHORT)(pPosition - FileNameTmpShort.GetString())))
		{
			KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"FileNameTmpShort.Shorten failed. File(%wZ)",
				FileNameTmpShort.Get());

			__leave;
		}

		if (!ConvertByZwQuerySymbolicLinkObject(&FileNameTmpShort, pFileNameDev))
		{
			KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"ConvertByZwQuerySymbolicLinkObject failed. File(%wZ)",
				FileNameTmpShort.Get());

			__leave;
		}

		if (!pFileNameDev->Append(&FileNameWindows))
		{
			KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"pFileNameDev->Append failed. File(%wZ)",
				pFileNameDev->Get());

			__leave;
		}

		if (!pFileNameDev->Append(pFileName->GetString() + 11, pFileName->GetLenCh() - 11))
		{
			KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"pFileNameDev->Append failed. File(%wZ) File(%wZ)",
				pFileNameDev->Get(), pFileName->Get());

			__leave;
		}

		bRet = TRUE;
	}
	__finally
	{
		FreeLock();
	}

	return bRet;
}

BOOLEAN
	CFileName::ConvertByZwQuerySymbolicLinkObject(
	__in	CKrnlStr* pFileName,
	__inout CKrnlStr* pNewFileName
	)
{
	BOOLEAN				bRet		= FALSE;

	OBJECT_ATTRIBUTES	Oa			= {0};
	NTSTATUS			ntStatus	= STATUS_UNSUCCESSFUL;
	HANDLE				Handle		= NULL;


	__try
	{
		if (!pFileName || !pNewFileName)
		{
			KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"input parameter error. pFileName(%p) pNewFileName(%p)",
				pFileName, pNewFileName);

			__leave;
		}

		// ��󲻴�"\\"
		InitializeObjectAttributes(
			&Oa, 
			pFileName->Get(),
			OBJ_CASE_INSENSITIVE,
			NULL, 
			NULL
			);

		ntStatus = ZwOpenSymbolicLinkObject(
			&Handle, 
			GENERIC_READ, 
			&Oa
			);
		if (!NT_SUCCESS(ntStatus))
		{
			KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"ZwOpenSymbolicLinkObject failed. File(%wZ) (%x)",
				pFileName->Get(), ntStatus);

			__leave;
		}

		if (!pNewFileName->Init())
		{
			KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"pNewFileName->Init failed");
			__leave;
		}

		ntStatus = ZwQuerySymbolicLinkObject(
			Handle, 
			pNewFileName->Get(), 
			NULL
			);
		if (!NT_SUCCESS(ntStatus))
		{
			KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"ZwQuerySymbolicLinkObject failed. File(%wZ) (%x)",
				pFileName->Get(), ntStatus);

			__leave;
		}

		bRet = TRUE;
	}
	__finally
	{
		if (Handle)
		{
			ZwClose(Handle);
			Handle = NULL;
		}
	}

	return bRet;
}

BOOLEAN
	CFileName::GetPathByHandle(
	__in	PFLT_CALLBACK_DATA		pData,
	__in	PCFLT_RELATED_OBJECTS	pFltObjects,
	__in	HANDLE					hFile,
	__out	CKrnlStr			*	pFileName
	)
{
	BOOLEAN					bRet			= FALSE;

	NTSTATUS				ntStatus		= STATUS_UNSUCCESSFUL;
	PFILE_OBJECT			pFileObj		= NULL;
	PFILE_NAME_INFORMATION	pFileNameInfo	= NULL;
	PWCHAR					pName			= NULL;

	CKrnlStr				RelativeFileName;


	__try
	{
		if (!pData || !hFile || !pFileName)
		{
			KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"input parameter error. pData(%p) hFile(%p) pFileName(%p)",
				pData, hFile, pFileName);

			__leave;
		}

		ntStatus = ObReferenceObjectByHandle(
			hFile,
			0,
			NULL,
			KernelMode,
			(PVOID *)&pFileObj,
			NULL
			);
		if (!NT_SUCCESS(ntStatus))
		{
			KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"ObReferenceObjectByHandle failed. (%x) File(%wZ)",
				ntStatus, &pData->Iopb->TargetFileObject->FileName);

			__leave;
		}

		pFileNameInfo = (PFILE_NAME_INFORMATION)new(FILE_NAME_TAG) CHAR[sizeof(FILE_NAME_INFORMATION) + (MAX_PATH - 1) * sizeof(WCHAR)];

		ntStatus = FltQueryInformationFile(
			pData->Iopb->TargetInstance,
			pFileObj,
			pFileNameInfo,
			sizeof(FILE_NAME_INFORMATION) + (MAX_PATH - 1) * sizeof(WCHAR),
			FileNameInformation,
			NULL
			);
		if (!NT_SUCCESS(ntStatus))
		{
			// ����������STATUS_BUFFER_OVERFLOW����ulRetΪ0������������ǰ�ͷ���һ���㹻���buffer
			// Comments
			// FltQueryInformationFile returns zero in any member of a FILE_XXX_INFORMATION structure that is not supported by a particular file system. 
			KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"FltQueryInformationFile failed. (%x) File(%wZ)",
				ntStatus, &pData->Iopb->TargetFileObject->FileName);

			__leave;
		}

		pName = (PWCHAR)new(FILE_NAME_TAG) CHAR[pFileNameInfo->FileNameLength + sizeof(WCHAR)];
		RtlCopyMemory(pName, pFileNameInfo->FileName, pFileNameInfo->FileNameLength);

		if (!pFileName->Set(pName, wcslen(pName)))
		{
			KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"pFileName->Set failed. File(%lS)",
				pName);

			__leave;
		}

		if (L'\\' == *(pFileName->GetString() + pFileName->GetLenCh() - 1))
		{
			if (!pFileName->Shorten(pFileName->GetLenCh() - 1))
			{
				KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"pFileName->Shorten failed. File(%wZ)",
					pFileName->Get());

				__leave;
			}
		}

		if (!RelativeFileName.Set(pFileName))
		{
			KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"RelativeFileName.Set failed. File(%wZ)",
				pFileName->Get());

			__leave;
		}

		if (!GetVolDevNameFromFltVol(pFltObjects->Volume, pFileName))
		{
			KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"GetVolDevNameFromFltVol failed. File(%wZ)",
				RelativeFileName.Get());

			__leave;
		}

		if (!pFileName->Append(&RelativeFileName))
		{
			KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"pFileName->Append failed. Volume(%wZ) File(%wZ)",
				pFileName->Get(), RelativeFileName.Get());

			__leave;
		}

		bRet = TRUE;
	}
	__finally
	{
		delete[] pFileNameInfo;
		pFileNameInfo = NULL;

		delete[] pName;
		pName = NULL;

		if (pFileObj)
		{
			ObDereferenceObject(pFileObj);
			pFileObj = NULL;
		}
	}

	return bRet;
}

BOOLEAN
	CFileName::IsExpression(
	__in CKrnlStr* pFileName
	)
{
	BOOLEAN bRet		= FALSE;

	PWCHAR	pPosition	= NULL;


	__try
	{
		if (!pFileName)
		{
			KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"input parameter error");
			__leave;
		}

		for(pPosition = pFileName->GetString(); pPosition < pFileName->GetString() + pFileName->GetLenCh(); pPosition++)
		{
			if (*pPosition == L'*')
			{
				bRet = TRUE;
				__leave;
			}
		}
	}
	__finally
	{
		;
	}

	return bRet;
}

BOOLEAN
	CFileName::ParseDevName(
	__in	PFLT_VOLUME	pFltVol,
	__in	CKrnlStr*	pDevName,
	__inout CKrnlStr*	pVolName,
	__inout CKrnlStr*	pPartName
	)
{
	BOOLEAN bRet = FALSE;


	__try
	{
		if (!pFltVol || !pDevName || !pVolName || !pPartName)
		{
			KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"input parameter error. pFltVol(%p) pDevName(%p) pVolName(%p) pPartName(%p)",
				pFltVol, pDevName, pVolName, pPartName);

			__leave;
		}

		if (!GetVolDevNameFromFltVol(pFltVol, pVolName))
		{
			KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"GetVolDevNameFromFltVol failed. File(%wZ)",
				pDevName->Get());

			__leave;
		}

		if (!pPartName->Set(pDevName->GetString() + pVolName->GetLenCh(), pDevName->GetLenCh() - pVolName->GetLenCh()))
		{
			KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"pPartName->Set failed. File(%wZ)",
				pDevName->Get());

			__leave;
		}

		bRet = TRUE;
	}
	__finally
	{
		;
	}


	return bRet;
}

BOOLEAN
	CFileName::IsPageFileSys(
	__in CKrnlStr* pFileName
	)
{
	BOOLEAN		bRet		= FALSE;

	CFileName	FileName;


	__try
	{
		FileName.GetLock();

		if (!pFileName)
		{
			KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"input parameter error");
			__leave;
		}

		bRet = (pFileName->Equal(ms_pPageFileSys, TRUE) || pFileName->Equal(ms_pLogFileSys, TRUE));
	}
	__finally
	{
		FileName.FreeLock();
	}

	return bRet;
}

BOOLEAN
	CFileName::IsDisMountStandard(
	__in		CKrnlStr*		pVolDevName,
	__in		PFLT_INSTANCE	pFltInstance,
	__in_opt	CKrnlStr*		pVolAppName
	)
{
	BOOLEAN				bRet			= FALSE;

	LPVOLUME_NAME_INFO	lpVolNameInfo	= NULL;


	__try
	{
		GetLock();

		if (!pVolDevName || !pFltInstance)
		{
			KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"input parameter error. pVolDevName(%p) pFltInstance(%p)",
				pVolDevName, pFltInstance);

			__leave;
		}

		lpVolNameInfo = GetVolNameInfo(pVolDevName, TYPE_DEV);
		if (!lpVolNameInfo)
		{
			KdPrintKrnl(LOG_PRINTF_LEVEL_WARNING, LOG_RECORED_LEVEL_NEED, L"GetVolNameInfo failed. Volume(%wZ)",
				pVolDevName->Get());

			__leave;
		}

		if (lpVolNameInfo->pFltInstance == pFltInstance)
		{
			if (pVolAppName)
			{
				if (!pVolAppName->Set(&lpVolNameInfo->AppName))
				{
					KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"pVolAppName->Set failed. Volume(%wZ)",
						pVolDevName->Get());

					__leave;
				}
			}

			bRet = TRUE;
		}
	}
	__finally
	{
		FreeLock();
	}

	return bRet;
}

BOOLEAN
	CFileName::ParseDevNameFromList(
	__in		CKrnlStr*		pFileName,
	__inout		CKrnlStr*		pVolName,
	__inout		CKrnlStr*		pPartName,
	__out_opt	PFLT_INSTANCE*	pPFltInstance
	)
{
	BOOLEAN				bRet			= FALSE;

	LIST_ENTRY*			pNode			= NULL;
	LPVOLUME_NAME_INFO	lpVolNameInfo	= NULL;

	CKrnlStr			VolNameEx;


	__try
	{
		GetLock();

		if (!pFileName || !pVolName || !pPartName)
		{
			KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"input parameter error. pFileName(%p) pVolName(%p) pPartName(%p)",
				pFileName, pVolName, pPartName);

			__leave;
		}

		if (IsListEmpty(&ms_ListHead))
		{
			KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"list empty. File(%wZ)",
				pFileName->Get());

			__leave;
		}

		for (pNode = ms_ListHead.Flink; pNode != &ms_ListHead; pNode = pNode->Flink, lpVolNameInfo = NULL)
		{
			lpVolNameInfo = CONTAINING_RECORD(pNode, VOLUME_NAME_INFO, List);
			if (!lpVolNameInfo)
			{
				KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"CONTAINING_RECORD failed. File(%wZ)",
					pFileName->Get());

				__leave;
			}

			if (!VolNameEx.Set(&lpVolNameInfo->DevName))
			{
				KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"VolNameEx.Set failed. File(%wZ) Vol(%wZ)",
					pFileName->Get(), lpVolNameInfo->DevName.Get());

				__leave;
			}

			if (!VolNameEx.Append(L"\\*", wcslen(L"\\*")))
			{
				KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"VolNameEx.Append failed. File(%wZ) Vol(%wZ)",
					pFileName->Get(), VolNameEx.Get());

				__leave;
			}

			if (FsRtlIsNameInExpression(
				VolNameEx.Get(),
				pFileName->Get(),
				TRUE,
				NULL
				))
			{
				if (!pVolName->Set(&lpVolNameInfo->DevName))
				{
					KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"pVolName->Set failed. File(%wZ) Vol(%wZ)",
						pFileName->Get(), lpVolNameInfo->DevName.Get());

					__leave;
				}

				if (!pPartName->Set(pFileName->GetString() + pVolName->GetLenCh(), pFileName->GetLenCh() - pVolName->GetLenCh()))
				{
					KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"pPartName->Set failed. File(%wZ) Vol(%wZ)",
						pFileName->Get(), pVolName->Get());

					__leave;
				}

				if (pPFltInstance)
					*pPFltInstance = lpVolNameInfo->pFltInstance;

				bRet = TRUE;
				__leave;
			}
		}
	}
	__finally
	{
		FreeLock();
	}

	return bRet;
}

BOOLEAN
	CFileName::GetFltInstance(
	__in	CKrnlStr*		pFileName,
	__out	PFLT_INSTANCE*	pPFltInstance,
	__in	NAME_TYPE		NameType
	)
{
	BOOLEAN				bRet			= FALSE;

	LIST_ENTRY*			pNode			= NULL;
	LPVOLUME_NAME_INFO	lpVolNameInfo	= NULL;

	CKrnlStr			VolNameEx;


	__try
	{
		GetLock();

		if (!pFileName || !pPFltInstance)
		{
			KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"input parameter error. pFileName(%p) pPFltInstance(%p)",
				pFileName, pPFltInstance);

			__leave;
		}

		if (IsListEmpty(&ms_ListHead))
		{
			KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"list empty. File(%wZ)",
				pFileName->Get());

			__leave;
		}

		for (pNode = ms_ListHead.Flink; pNode != &ms_ListHead; pNode = pNode->Flink, lpVolNameInfo = NULL)
		{
			lpVolNameInfo = CONTAINING_RECORD(pNode, VOLUME_NAME_INFO, List);
			if (!lpVolNameInfo)
			{
				KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"CONTAINING_RECORD failed. File(%wZ)",
					pFileName->Get());

				__leave;
			}

			switch (NameType)
			{
			case TYPE_DEV:
				{
					if (!VolNameEx.Set(&lpVolNameInfo->DevName))
					{
						KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"VolNameEx.Set failed. File(%wZ) Vol(%wZ)",
							pFileName->Get(), lpVolNameInfo->DevName.Get());

						__leave;
					}

					break;
				}
			case TYPE_APP:
				{
					if (!VolNameEx.Set(&lpVolNameInfo->AppName))
					{
						KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"VolNameEx.Set failed. File(%wZ) Vol(%wZ)",
							pFileName->Get(), lpVolNameInfo->AppName.Get());

						__leave;
					}

					break;
				}
			default:
				{
					KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"NameType error. NameType(%d) File(%wZ)",
						NameType, pFileName->Get());

					__leave;
				}
			}

			if (!VolNameEx.Append(L"\\*", wcslen(L"\\*")))
			{
				KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"VolNameEx.Append failed. File(%wZ) Vol(%wZ)",
					pFileName->Get(), VolNameEx.Get());

				__leave;
			}

			if (FsRtlIsNameInExpression(
				VolNameEx.Get(),
				pFileName->Get(),
				TRUE,
				NULL
				))
			{
				*pPFltInstance = lpVolNameInfo->pFltInstance;

				bRet = TRUE;
				__leave;
			}
		}
	}
	__finally
	{
		FreeLock();
	}

	return bRet;
}

BOOLEAN
	CFileName::GetSectorSize(
	__in	CKrnlStr*	pFileName,
	__inout ULONG*		pUlSectorSize
	)
{
	BOOLEAN				bRet			= FALSE;

	LIST_ENTRY*			pNode			= NULL;
	LPVOLUME_NAME_INFO	lpVolNameInfo	= NULL;

	CKrnlStr			VolNameEx;


	__try
	{
		GetLock();

		if (!pFileName || !pUlSectorSize)
		{
			KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"input parameter error. pFileName(%p) pUlSectorSize(%p)",
				pFileName, pUlSectorSize);

			__leave;
		}

		if (IsListEmpty(&ms_ListHead))
		{
			KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"list empty. File(%wZ)",
				pFileName->Get());

			__leave;
		}

		for (pNode = ms_ListHead.Flink; pNode != &ms_ListHead; pNode = pNode->Flink, lpVolNameInfo = NULL)
		{
			lpVolNameInfo = CONTAINING_RECORD(pNode, VOLUME_NAME_INFO, List);
			if (!lpVolNameInfo)
			{
				KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"CONTAINING_RECORD failed. File(%wZ)",
					pFileName->Get());

				__leave;
			}

			if (!VolNameEx.Set(&lpVolNameInfo->DevName))
			{
				KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"VolNameEx.Set failed. File(%wZ) Vol(%wZ)",
					pFileName->Get(), lpVolNameInfo->DevName.Get());

				__leave;
			}

			if (!VolNameEx.Append(L"\\*", wcslen(L"\\*")))
			{
				KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"VolNameEx.Append failed. File(%wZ) Vol(%wZ)",
					pFileName->Get(), VolNameEx.Get());

				__leave;
			}

			if (FsRtlIsNameInExpression(
				VolNameEx.Get(),
				pFileName->Get(),
				TRUE,
				NULL
				))
			{
				*pUlSectorSize = lpVolNameInfo->ulSectorSize;

				bRet = TRUE;
				__leave;
			}
		}
	}
	__finally
	{
		FreeLock();
	}

	return bRet;
}

BOOLEAN
	CFileName::IsShadowCopy(
	__in CKrnlStr * pFileName
	)
{
	BOOLEAN		bRet		= FALSE;

	CKrnlStr	FileName;
	CKrnlStr	FileNameCmp;


	__try
	{
		if (!pFileName)
		{
			KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"input parameter error");
			__leave;
		}

		if (!FileName.Set(L"\\Device\\HarddiskVolumeShadowCopy", wcslen(L"\\Device\\HarddiskVolumeShadowCopy")))
		{
			KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"FileName.Set failed");
			__leave;
		}

		if (pFileName->GetLenCh() < FileName.GetLenCh())
			__leave;
		else if (pFileName->GetLenCh() > FileName.GetLenCh())
		{
			if (!FileNameCmp.Set(pFileName))
			{
				KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"FileNameCmp.Set failed. File(%wZ)",
					pFileName->Get());

				__leave;
			}

			if (!FileNameCmp.Shorten(FileName.GetLenCh()))
			{
				KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"FileNameCmp.Shorten failed");
				__leave;
			}

			bRet = FileNameCmp.Equal(&FileName, TRUE);
		}
		else
			bRet = pFileName->Equal(&FileName, TRUE);
	}
	__finally
	{
		;
	}

	return bRet;
}

BOOLEAN
	CFileName::IsRemoveable(
	__in CKrnlStr * pFileName
	)
{
	BOOLEAN				bRet			= FALSE;

	LIST_ENTRY*			pNode			= NULL;
	LPVOLUME_NAME_INFO	lpVolNameInfo	= NULL;

	CKrnlStr			VolNameEx;


	__try
	{
		GetLock();

		if (!pFileName)
		{
			KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"input parameter error");
			__leave;
		}

		if (IsListEmpty(&ms_ListHead))
		{
			KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"list empty. File(%wZ)",
				pFileName->Get());

			__leave;
		}

		for (pNode = ms_ListHead.Flink; pNode != &ms_ListHead; pNode = pNode->Flink, lpVolNameInfo = NULL)
		{
			lpVolNameInfo = CONTAINING_RECORD(pNode, VOLUME_NAME_INFO, List);
			if (!lpVolNameInfo)
			{
				KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"CONTAINING_RECORD failed. File(%wZ)",
					pFileName->Get());

				__leave;
			}

			if (!VolNameEx.Set(&lpVolNameInfo->DevName))
			{
				KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"VolNameEx.Set failed. File(%wZ) Vol(%wZ)",
					pFileName->Get(), lpVolNameInfo->DevName.Get());

				__leave;
			}

			if (!VolNameEx.Append(L"\\*", wcslen(L"\\*")))
			{
				KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"VolNameEx.Append failed. File(%wZ) Vol(%wZ)",
					pFileName->Get(), VolNameEx.Get());

				__leave;
			}

			if (pFileName->Equal(&lpVolNameInfo->DevName, TRUE) ||
				FsRtlIsNameInExpression(
				VolNameEx.Get(),
				pFileName->Get(),
				TRUE,
				NULL
				))
			{
				bRet = lpVolNameInfo->bRemoveable;
				__leave;
			}
		}
	}
	__finally
	{
		FreeLock();
	}

	return bRet;
}

BOOLEAN
	CFileName::IsWmDb(
	__in CKrnlStr* pFileName
	)
{
	BOOLEAN		bRet		= FALSE;

	CFileName	FileName;


	__try
	{
		FileName.GetLock();

		if (!pFileName)
		{
			KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"input parameter error");
			__leave;
		}

		if (ms_pWmDb->GetLenCh())
			bRet = pFileName->Equal(ms_pWmDb, TRUE);
	}
	__finally
	{
		FileName.FreeLock();
	}

	return bRet;
}

BOOLEAN
	CFileName::SetWmDb(
	__in CKrnlStr* pDir
	)
{
	BOOLEAN		bRet		= FALSE;

	CFileName	FileName;


	__try
	{
		FileName.GetLock();

		if (!pDir)
		{
			KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"input parameter error");
			__leave;
		}

		if (!ms_pWmDb->Set(pDir))
		{
			KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"ms_pWmDb->Set failed. Dir(%wZ)",
				pDir->Get());

			__leave;
		}

		if (!ms_pWmDb->Append(L"\\Wm.db", wcslen(L"\\Wm.db")))
		{
			KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"ms_pWmDb->Append failed. File(%wZ)",
				ms_pWmDb->Get());

			__leave;
		}

		bRet = TRUE;
	}
	__finally
	{
		FileName.FreeLock();
	}

	return bRet;
}
