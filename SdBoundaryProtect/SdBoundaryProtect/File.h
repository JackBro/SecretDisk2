/*++
*
* Copyright (c) 2015 - 2016  �����������¿Ƽ����޹�˾
*
* Module Name:
*
*		File.h
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

#define MOD_FILE		L"�ļ�"
#define MEMORY_TAG_FILE	'ELIF'		// FILE

typedef enum _FILE_OBJECT_TYPE
{
	OBJECT_TYPE_NULL,
	OBJECT_TYPE_FILE,
	OBJECT_TYPE_DIR,
	OBJECT_TYPE_VOLUME,
	OBJECT_TYPE_UNKNOWN
} FILE_OBJECT_TYPE, *PFILE_OBJECT_TYPE, *LPFILE_OBJECT_TYPE;

class CFile
{
public:
	/*
	* ����˵����
	*		��ȡ��������
	*
	* ������
	*		pData
	*		pFileName
	*		bRelation	pData��pFileName�Ƿ��й���
	*
	* ����ֵ��
	*		ObjType
	*
	* ��ע��
	*		��
	*/
	static
		FILE_OBJECT_TYPE
		GetObjType(
		__in PFLT_CALLBACK_DATA		pData,
		__in CKrnlStr			*	pFileName,
		__in BOOLEAN				bRelation
		);

private:
	CFile();

	~CFile();

	/*
	* ����˵����
	*		ʹ��pData�еĲ���Create�ļ�pFileName
	*
	* ������
	*		pData	
	*		pFileName
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
		CreateUseOigInfo(
		__in		PFLT_CALLBACK_DATA		pData,
		__in		CKrnlStr			*	pFileName,
		__out_opt	FILE_OBJECT_TYPE			*	ObjType		= NULL
		);

	/*
	* ����˵����
	*		ɾ���ļ�
	*
	* ������
	*		pData	
	*		pFileName		�ļ���
	*		bDelTag			ɾ�����
	*		bUserOrgFileObj	pData��pFileName�Ƿ��й���
	*		ObjType			��������
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
		DelFile(
		__in PFLT_CALLBACK_DATA		pData,
		__in CKrnlStr			*	pFileName,
		__in BOOLEAN				bDelTag,
		__in BOOLEAN				bRelation,
		__in FILE_OBJECT_TYPE			ObjType		
		);

	static
		BOOLEAN
		GetFileAttributes(
		__in PFLT_CALLBACK_DATA	pData,
		__in CKrnlStr*			pFileName,
		__in PULONG				pFileAttributes
		);

	static
		BOOLEAN
		SetFileAttributes(
		__in PFLT_CALLBACK_DATA	pData,
		__in CKrnlStr*			pFileName,
		__in ULONG				ulFileAttributes
		);
};
