/*++
*
* Copyright (c) 2015 - 2016  �����������¿Ƽ����޹�˾
*
* Module Name:
*
*		DirHide.cpp
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
#include "DirHide.h"

/*
* ����˵����
*		�����ļ����ļ�
*
* ������
*		pData
*		Rule			����
*		ParentPath		��Ŀ¼
*
* ����ֵ��
*		��
*
* ��ע��
*		��
*/
BOOLEAN 
	CDirHide::BreakLink(
	__inout PFLT_CALLBACK_DATA	pData,
	__in	CKrnlStr*			RuleEx,
	__in	CKrnlStr*			ParentPath
	)
{
	BOOLEAN					bRet					= TRUE;

	CKrnlStr				Name;
	CKrnlStr				Path;
	CKrnlStr				Rule;
	PWCHAR					pWcharName				= NULL;

	//FILE_INFORMATION_CLASS �����ƫ����
	ULONG_PTR				ulNextOffset			= 0;
	ULONG_PTR				ulNameOffset			= 0;
	ULONG_PTR				ulNameLengthOffset		= 0;

	FILE_INFORMATION_CLASS	FileInfoClass;

	LPVOID					lpStartFileInfo			= NULL;
	LPVOID					lpCurrentFileInfo		= NULL;
	LPVOID					lpPreviousFileInfo		= NULL;
	LPVOID					lpNextFileInfo 			= NULL;
	ULONG_PTR				ulFileInfoSize			= 0;


	if (!pData || !RuleEx || !RuleEx->Get() || !ParentPath || !ParentPath->Get())
	{
		KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"[CDirProtect::BreakLink : input argument error");
		return FALSE;
	}

	// ��ù����ַ���
	if (!Rule.Set(RuleEx))
	{
		KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"Rule.Set failed");
		return FALSE;
	}

	if (!Rule.Shorten(Rule.GetLenCh() - 2))
	{
		KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"Rule.Shorten failed");
		return FALSE;
	}

	// ��û����ַ
	if (pData->Iopb->Parameters.DirectoryControl.QueryDirectory.MdlAddress)
		lpCurrentFileInfo = MmGetSystemAddressForMdlSafe(
		pData->Iopb->Parameters.DirectoryControl.QueryDirectory.MdlAddress,
		NormalPagePriority
		);
	else
		lpCurrentFileInfo = pData->Iopb->Parameters.DirectoryControl.QueryDirectory.DirectoryBuffer;

	if (!lpCurrentFileInfo)
	{
		KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"pCurrentFileInfo is NULL");
		return FALSE;
	}

	//�ж�FileInformationClass������
	FileInfoClass = pData->Iopb->Parameters.DirectoryControl.QueryDirectory.FileInformationClass;
	if (!GetDirInfoOffset(FileInfoClass, &ulNameOffset, &ulNameLengthOffset, &ulFileInfoSize))
	{
		KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"GetDirInfoOffset failed");
		return FALSE;
	}

	lpStartFileInfo = lpCurrentFileInfo;
	lpPreviousFileInfo = lpCurrentFileInfo;

	do 
	{
		// Byte offset of the next FILE_BOTH_DIR_INFORMATION entry
		ulNextOffset = *(PULONG)lpCurrentFileInfo;

		// ��̽��ָ�� 
		lpNextFileInfo = (LPVOID)((ULONG_PTR)lpCurrentFileInfo + ulNextOffset);  

		// ��õ�ǰ�ļ�(��)������
		pWcharName = (PWCHAR)new(MEMORY_TAG_DIR_HIDE) CHAR[*((PUSHORT)(((PCHAR)lpCurrentFileInfo) + ulNameLengthOffset)) + sizeof(WCHAR)];
		RtlCopyMemory(pWcharName, (PWCHAR)(((PCHAR)lpCurrentFileInfo) + ulNameOffset), *((PUSHORT)(((PCHAR)lpCurrentFileInfo) + ulNameLengthOffset)));

		if (!Name.Set(pWcharName, wcslen(pWcharName)))
		{
			KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"Name.Set failed");
			bRet = FALSE;
			break;
		}

		// ��õ�ǰ�ļ�(��)��·��
		if (!Path.Set(ParentPath))
		{
			KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"Path.Set failed");
			bRet = FALSE;
			break;
		}

		if (*(Path.GetString() + Path.GetLenCh() - 1) != L'\\')
		{
			if (!Path.Append(L"\\", wcslen(L"\\")))
			{
				KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"Path.Append failed");
				bRet = FALSE;
				break;
			}
		}

		if (!Path.Append(&Name))
		{
			KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"Path.Append failed");
			bRet = FALSE;
			break;
		}

		// �ж��費��Ҫ����
		if ((FsRtlIsNameInExpression(RuleEx->Get(), Path.Get(), TRUE, NULL) || Rule.Equal(&Path, TRUE))
			&&
			!Name.Equal(L".", wcslen(L"."), FALSE)
			&&
			!Name.Equal(L"..", wcslen(L".."), FALSE))
		{
			// ���������ز���

			if (lpCurrentFileInfo == lpStartFileInfo)
			{
				if (ulNextOffset)
				{
					// �̷���Ŀ¼���ж���ļ������ҵ�һ����Ҫ����
					RtlCopyMemory((PWCHAR)(((PCHAR)lpCurrentFileInfo) + ulNameOffset), L".", sizeof(WCHAR));
					*((PUSHORT)(((PCHAR)lpCurrentFileInfo) + ulNameLengthOffset)) = sizeof(WCHAR);
				}
				else
				{
					// �̷���Ŀ¼ֻ��1���ļ����ļ��У�����ǡ������Ҫ���ص�
					pData->IoStatus.Status = STATUS_NO_MORE_FILES;
				}
			}
			else
			{
				// ����ǰ�������ָ����һ����ƫ�������Թ�Ҫ���ص��ļ����ļ���㣬�ﵽ����Ŀ��
				*(PULONG_PTR)lpPreviousFileInfo += ulNextOffset;
			}

			// ������ѯ���
			if (pData->IoStatus.Information > ulFileInfoSize + (*((PUSHORT)(((PCHAR)lpCurrentFileInfo) + ulNameLengthOffset)) - sizeof(WCHAR)))
				pData->IoStatus.Information -= (ulFileInfoSize + (*((PUSHORT)(((PCHAR)lpCurrentFileInfo) + ulNameLengthOffset)) - sizeof(WCHAR)));
		}
		else
			lpPreviousFileInfo = lpCurrentFileInfo;

		// ��ǰָ�����      
		lpCurrentFileInfo = lpNextFileInfo;

		delete[] pWcharName;
		pWcharName = NULL;
	} while (ulNextOffset);

	if (lpPreviousFileInfo != lpNextFileInfo)
		*(PULONG_PTR)lpPreviousFileInfo = 0;

	delete[] pWcharName;
	pWcharName = NULL;

	return bRet;
}

/*
* ����˵����
*		�жϵ�ǰFileInformationClass�ľ�������
*
* ������
*		FileInfoClass		���뵱ǰ��FileInformationClass
*		ulNameOffset		������ǰFileInformationClass���͵�FileName��ƫ��
*		ulNameLengthOffset	������ǰFileInformationClass���͵�FileNameLength��ƫ��
*
* ����ֵ��
*		STATUS_INVALID_PARAMETER	ʧ��
*		STATUS_SUCCESS				�ɹ�
*
* ��ע��
*		�˺���Ŀ����Ϊ��ϵͳ����
*		XP�л�õ�FileInformationClass��������FileBothDirectoryInformation
*		����ϵͳ��δ����
*/
BOOLEAN
	CDirHide::GetDirInfoOffset(
	__in	FILE_INFORMATION_CLASS	FileInfoClass,
	__inout	PULONG_PTR				pNameOffset,
	__inout	PULONG_PTR				pNameLengthOffset,
	__inout	PULONG_PTR				pFileInfoSize
	)
{
	BOOLEAN bRet = TRUE;


	__try
	{
		if (!pNameOffset || !pNameLengthOffset)
		{
			KdPrintKrnl(LOG_PRINTF_LEVEL_ERROR, LOG_RECORED_LEVEL_NEED, L"input argument error");
			__leave;
		}

		switch (FileInfoClass)
		{
		case FileBothDirectoryInformation:
			{
				*pNameOffset = OFFSETOF(FILE_BOTH_DIR_INFORMATION, FileName);
				*pNameLengthOffset = OFFSETOF(FILE_BOTH_DIR_INFORMATION, FileNameLength);
				*pFileInfoSize = sizeof(FILE_BOTH_DIR_INFORMATION);
				break;
			}
		case FileDirectoryInformation:
			{
				*pNameOffset = OFFSETOF(FILE_DIRECTORY_INFORMATION, FileName);
				*pNameLengthOffset = OFFSETOF(FILE_DIRECTORY_INFORMATION, FileNameLength);
				*pFileInfoSize = sizeof(FILE_DIRECTORY_INFORMATION);
				break;
			}
		case FileFullDirectoryInformation:
			{
				*pNameOffset = OFFSETOF(FILE_FULL_DIR_INFORMATION, FileName);
				*pNameLengthOffset = OFFSETOF(FILE_FULL_DIR_INFORMATION, FileNameLength);
				*pFileInfoSize = sizeof(FILE_FULL_DIR_INFORMATION);
				break;
			}
		case FileNamesInformation:
			{
				*pNameOffset = OFFSETOF(FILE_NAMES_INFORMATION, FileName);
				*pNameLengthOffset = OFFSETOF(FILE_NAMES_INFORMATION, FileNameLength);
				*pFileInfoSize = sizeof(FILE_NAMES_INFORMATION);
				break;
			}
		case FileIdBothDirectoryInformation:
			{
				*pNameOffset = OFFSETOF(FILE_ID_BOTH_DIR_INFORMATION, FileName);
				*pNameLengthOffset = OFFSETOF(FILE_ID_BOTH_DIR_INFORMATION, FileNameLength);
				*pFileInfoSize = sizeof(FILE_ID_BOTH_DIR_INFORMATION);
				break;
			}
		case FileIdFullDirectoryInformation:
			{
				*pNameOffset = OFFSETOF(FILE_ID_FULL_DIR_INFORMATION, FileName);
				*pNameLengthOffset = OFFSETOF(FILE_ID_FULL_DIR_INFORMATION, FileNameLength);
				*pFileInfoSize = sizeof(FILE_ID_FULL_DIR_INFORMATION);
				break;
			}
		default:
			bRet = FALSE;
		} 
	}
	__finally
	{
		;
	}

	return bRet;
}
