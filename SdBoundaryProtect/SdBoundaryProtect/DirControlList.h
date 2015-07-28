/*++
*
* Copyright (c) 2015 - 2016  �����������¿Ƽ����޹�˾
*
* Module Name:
*
*		DirControlList.h
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

#define MOD_DIR_CONTROL_LIST		L"Ŀ¼�����б�"
#define MEMORY_TAG_DIR_CONTROL_LIST	'LCTD'								// DTCL
#define OFFSETOF(TYPE, MEMBER)		((size_t) & ((TYPE *)0)->MEMBER)

typedef struct _DIR_CONTROL_LIST
{
	CKrnlStr			RuleEx;				// ����·�����ʽ
	CKrnlStr			ParentDirRuleEx;	// ����·����Ŀ¼������ʽ

	DIR_CONTROL_TYPE	Type;				// ��������

	LIST_ENTRY	List;
} DIR_CONTROL_LIST, *PDIR_CONTROL_LIST, *LPDIR_CONTROL_LIST;

typedef struct _REGISTER_DIR_INFO
{
	CKrnlStr			FileName;

	DIR_CONTROL_TYPE	Type;			// ��������
} REGISTER_DIR_INFO, *PREGISTER_DIR_INFO, *LPREGISTER_DIR_INFO;

class CDirControlList
{
public:
	/*++
	*
	* Routine Description:
	*
	*		����
	*
	* Arguments:
	*
	*		��
	*
	* Return Value:
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
	CDirControlList();

	/*++
	*
	* Routine Description:
	*
	*		����
	*
	* Arguments:
	*
	*		��
	*
	* Return Value:
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
	~CDirControlList();

	/*++
	*
	* Routine Description:
	*
	*		��ʼ��ģ��
	*
	* Arguments:
	*
	*		��
	*
	* Return Value:
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
	VOID
		Init();

	/*++
	*
	* Routine Description:
	*
	*		ж��ģ��
	*
	* Arguments:
	*
	*		��
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
		Unload();

	/*++
	*
	* Routine Description:
	*
	*		��ѯָ���ļ��Ƿ����й���
	*
	* Arguments:
	*
	*		FileName - ������ʽ
	*
	* Return Value:
	*
	*		TRUE	- ����
	*		FALSE	- δ����
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
		IsIn(
		__in CKrnlStr			*	pFileName,
		__in DIR_CONTROL_TYPE		DirControlType
		);

	/*++
	*
	* Routine Description:
	*
	*		�����ļ��ļ���
	*
	*		1�����ڶ���
	*		2���ϵ��ǹ���"Ҫ���ص��ļ����ļ��еĸ�Ŀ¼"�µ���
	*		3�������жϵ����ļ���FileName����û��Ҫ���ص��ļ��ļ��У�����У�������
	*
	* Arguments:
	*
	*		FileName	- ������ʽ
	*		pData		- 
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
		Filter(
		__in	CKrnlStr*			FileName,
		__inout PFLT_CALLBACK_DATA	pData
		);

	/*++
	*
	* Routine Description:
	*
	*		����һ������
	*
	* Arguments:
	*
	*		lpRegisterDirInfo - Ҫ��ӵ��ļ�����Ϣ
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
		Insert(
		__in LPREGISTER_DIR_INFO lpRegisterDirInfo
		);

	/*++
	*
	* Routine Description:
	*
	*		����ģ��
	*
	* Arguments:
	*
	*		��
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
		Clear();

	/*++
	*
	* Routine Description:
	*
	*		ɾ��һ������
	*
	* Arguments:
	*
	*		RuleEx - ������ʽ
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
		Delete(
		__in CKrnlStr* RuleEx
		);

private:
	static LIST_ENTRY	ms_ListHead;
	static ERESOURCE	ms_Lock;
	static KSPIN_LOCK	ms_SpLock;

	KIRQL				m_Irql;
	LONG				m_LockRef;

	/*++
	*
	* Routine Description:
	*
	*		�����
	*
	* Arguments:
	*
	*		��
	*
	* Return Value:
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
	VOID
		GetLock();

	/*++
	*
	* Routine Description:
	*
	*		�ͷ���
	*
	* Arguments:
	*
	*		��
	*
	* Return Value:
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
	VOID
		FreeLock();

	/*++
	*
	* Routine Description:
	*
	*		���һ������
	*
	* Arguments:
	*
	*		RuleEx - ������ʽ
	*
	* Return Value:
	*
	*		!NULL	- �ɹ�
	*		NULL	- ʧ��
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
	LPDIR_CONTROL_LIST
		Get(
		__in CKrnlStr* RuleEx
		);
};
