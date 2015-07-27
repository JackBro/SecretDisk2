/*++
*
* Copyright (c) 2015 - 2016  �����������¿Ƽ����޹�˾
*
* Module Name:
*
*		DirHide.h
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

#define MOD_DIR_HIDE		L"Ŀ¼����"
#define MEMORY_TAG_DIR_HIDE	'DHTD'		// DTHD

class CDirHide
{
public:
	static
		BOOLEAN 
		BreakLink(
		__inout PFLT_CALLBACK_DATA	pData,
		__in	CKrnlStr*			RuleEx,
		__in	CKrnlStr*			ParentPath
		);

private:
	static
		BOOLEAN
		GetDirInfoOffset(
		__in	FILE_INFORMATION_CLASS	FileInfoClass,
		__inout	PULONG_PTR				pNameOffset,
		__inout	PULONG_PTR				pNameLengthOffset,
		__inout	PULONG_PTR				pFileInfoSize
		);
};
