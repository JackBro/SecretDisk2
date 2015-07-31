/*++
*
* Copyright (c) 2015 - 2016  �����������¿Ƽ����޹�˾
*
* Module Name:
*
*		FileName.h
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

#pragma once

#define MOD_FILE_NAME			L"�ļ���"
#define MEMORY_TAG_FILE_NAME	'MNLF'		// FLNM

typedef enum _NAME_TYPE
{
	NAME_TYPE_NULL		= 0x00000000,
	TYPE_APP			= 0x00000001,
	TYPE_SYM			= 0x00000002,
	TYPE_DEV			= 0x00000004,
	TYPE_UNKNOW			= 0x00000007,
	TYPE_FULL_PATH		= 0x00000008
} NAME_TYPE, *PNAME_TYPE, *LPNAME_TYPE;

typedef struct _VOLUME_NAME_INFO
{
	CKrnlStr		AppName;		// like "C:"
	CKrnlStr		SymName;		// like "\\??\\C:"
	CKrnlStr		DevName;		// like "\\Device\\HarddiskVolume12" consider volumes more than 10

	PFLT_INSTANCE	pFltInstance;
	ULONG			ulSectorSize;
	BOOLEAN			bRemoveable;
	BOOLEAN			bOnlyDevName;

	LIST_ENTRY		List;			// struct EntryPointer
} VOLUME_NAME_INFO, *PVOLUME_NAME_INFO, *LPVOLUME_NAME_INFO;

class CFileName
{
public:
	CFileName();

	~CFileName();

	VOID
		Init();

	BOOLEAN
		Unload();

	static
		BOOLEAN
		ToDev(
		__in	CKrnlStr * pName,
		__inout	CKrnlStr * pDevName
		);

	static
		BOOLEAN
		ToApp(
		__in	CKrnlStr * pName,
		__inout CKrnlStr * pAppName
		);

	/*
	* ����˵����
	*		��ȡ�ļ���
	*
	* ������
	*		pData
	*		pFltVol	
	*		pName		�ļ���
	*
	* ����ֵ��
	*		TRUE	�ɹ�
	*		FALSE	ʧ��
	*
	* ��ע��
	*		��
	*
	* ����޸�ʱ�䣺
	*		2015/7/3-16:25
	*/
	static
		BOOLEAN
		GetFileFullPath(
		__in	PFLT_CALLBACK_DATA		pData,
		__in	PFLT_VOLUME				pFltVol,
		__out	CKrnlStr			*	pDevName
		);

	/*
	* ����˵����
	*		��ø�·��
	*
	* ������
	*		pPath			�ļ�·��
	*		pParentPath		�ļ���·��
	*
	* ����ֵ��
	*		TRUE	�ɹ�
	*		FALSE	ʧ��
	*
	* ��ע��
	*		��
	*
	* ����޸�ʱ�䣺
	*		2015/7/3-16:25
	*/
	static
		BOOLEAN
		GetParentPath(
		__in	CKrnlStr * pPath,
		__out	CKrnlStr * pParentPath
		);

	/*
	* ����˵����
	*		��PFLT_VOLUME�л�ȡ����豸��
	*
	* ������
	*		pFltVol
	*		pName		����豸��
	*
	* ����ֵ��
	*		TRUE	�ɹ�
	*		FALSE	ʧ��
	*
	* ��ע��
	*		��
	*
	* ����޸�ʱ�䣺
	*		2015/7/3-16:29
	*/
	static
		BOOLEAN
		GetVolDevNameFromFltVol(
		__in	PFLT_VOLUME		pFltVol,
		__out	CKrnlStr	*	pDevName
		);

	/*++
	*
	* Routine Description:
	*
	*		����һ��VOLUME_NAME_INFO
	*
	* Arguments:
	*
	*		pAppName
	*
	*		pSymName
	*
	*		pDevName
	*
	*		bOnlyDevName
	*
	*		bRemoveable
	*
	*		pFltInstance
	*
	*		ulSectorSize
	*
	* Return Value:
	*
	*		TRUE	- �ɹ�
	*		FALSE	- ʧ��
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
	BOOLEAN
		InsertVolNameInfo(
		__in_opt	CKrnlStr		*	pAppName,
		__in_opt	CKrnlStr		*	pSymName,
		__in		CKrnlStr		*	pDevName,
		__in		BOOLEAN				bOnlyDevName,
		__in		BOOLEAN				bRemoveable,
		__in		PFLT_INSTANCE		pFltInstance,
		__in		ULONG				ulSectorSize
		);

	BOOLEAN
		DelVolNameInfo(
		__in CKrnlStr * pDevName
		);

	static
		BOOLEAN
		SpliceFilePath(
		__in	CKrnlStr * pDirPath,
		__in	CKrnlStr * pFileName,
		__inout	CKrnlStr * pFilePath
		);

	LPVOLUME_NAME_INFO
		GetVolNameInfo(
		__in CKrnlStr	*	pName,
		__in NAME_TYPE		NameType
		);

	/*
	* ����˵����
	*		�ж϶����Ƿ��Ǿ�
	*
	* ������
	*		pData
	*		pFileName	������
	*
	* ����ֵ��
	*		TRUE	�Ǿ�
	*		FALSE	���Ǿ�
	*
	* ��ע��
	*		��
	*/
	BOOLEAN
		IsVolume(
		__in PFLT_CALLBACK_DATA		pData,
		__in CKrnlStr			*	pDevName
		);

	/*
	* ����˵����
	*		�����ļ������ȡ�ļ�·��
	*
	* ������
	*		pData
	*		hFile		�ļ����
	*		pFileName	�ļ�·��
	*
	* ����ֵ��
	*		TRUE	�ɹ�
	*		FALSE	ʧ��
	*
	* ��ע��
	*		��
	*/
	static
		BOOLEAN
		GetPathByHandle(
		__in	PFLT_CALLBACK_DATA		pData,
		__in	PCFLT_RELATED_OBJECTS	pFltObjects,
		__in	HANDLE					hFile,
		__out	CKrnlStr			*	pDevName
		);

	static
		BOOLEAN
		IsExpression(
		__in CKrnlStr * pFileName
		);

	static
		BOOLEAN
		ParseAppOrSymName(
		__in	CKrnlStr	*	pName,
		__inout CKrnlStr	*	pVolName,
		__inout CKrnlStr	*	pPartName,
		__inout PBOOLEAN		pbDisk,
		__inout PNAME_TYPE		pNameType
		);

	/*++
	*
	* Routine Description:
	*
	*		�Ƿ��Ǳ�׼����
	*
	* Arguments:
	*
	*		pVolDevName
	*
	*		pFltInstance
	*
	* Return Value:
	*
	*		TRUE	- ��
	*		FALSE	- ����
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
	BOOLEAN
		IsDisMountStandard(
		__in CKrnlStr		*	pVolDevName,
		__in PFLT_INSTANCE		pFltInstance
		);

	static
		BOOLEAN
		GetVolAppNameByQueryObj(
		__in	CKrnlStr *	pName,
		__inout CKrnlStr *	pAppName,
		__inout PUSHORT		pusCutOffset
		);


	BOOLEAN
		GetFltInstance(
		__in	CKrnlStr		*	pFileName,
		__out	PFLT_INSTANCE	*	pPFltInstance,
		__in	NAME_TYPE			NameType
		);

	BOOLEAN
		GetSectorSize(
		__in	CKrnlStr	*	pDevName,
		__inout ULONG		*	pUlSectorSize
		);

private:
	static LIST_ENTRY	ms_ListHead;	
	static ERESOURCE	ms_Lock;
	static KSPIN_LOCK	ms_SpLock;

	KIRQL				m_Irql;
	LONG				m_LockRef;

	VOID 
		GetLock();

	VOID 
		FreeLock();

	/*
	* ����˵����
	*		��PFLT_CALLBACK_DATA��PFLT_VOLUME�н��ļ���ƴ�ӳ���
	*
	* ������
	*		pData
	*		pFltVol	
	*		pName		�ļ���	
	*
	* ����ֵ��
	*		TRUE	�ɹ�
	*		FALSE	ʧ��
	*
	* ��ע��
	*		��
	*
	* ����޸�ʱ�䣺
	*		2015/7/3-16:27
	*/
	static
		BOOLEAN
		GetFileFullPathFromDataAndFltVol(
		__in	PFLT_CALLBACK_DATA		pData,
		__in	PFLT_VOLUME				pFltVol,
		__out	CKrnlStr			*	pName
		);

	LPVOLUME_NAME_INFO
		GetVolNameInfoByVolAppName(
		__in CKrnlStr * pAppName
		);

	LPVOLUME_NAME_INFO
		GetVolNameInfoByVolSymName(
		__in CKrnlStr * pSymName
		);

	LPVOLUME_NAME_INFO
		GetVolNameInfoByVolDevName(
		__in CKrnlStr * pDevName
		);

	/*
	* ����˵����
	*		��þ���豸��
	*
	* ������
	*		pName		��ķ���������
	*		pDevName	����豸��
	*
	* ����ֵ��
	*		TRUE	�ɹ�
	*		FALSE	ʧ��
	*
	* ��ע��
	*		��
	*/
	static
		BOOLEAN
		GetVolDevNameByQueryObj(
		__in	CKrnlStr * pName,
		__out	CKrnlStr * pDevName
		);

};
