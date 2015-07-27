/*++
*
* Copyright (c) 2015 - 2016  �����������¿Ƽ����޹�˾
*
* Module Name:
*
*		ProcWhiteList.h
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

#define MOD_PROC_WHITE_LIST			L"���̰�����"
#define MEMORY_TAG_PROC_WHILTE_LIST	'LWCP'			// PCWL

typedef struct _PROC_WHITE_LIST
{
	ULONG		ulPid;	//����ID

	LIST_ENTRY  List; 
} PROC_WHITE_LIST, *PPROC_WHITE_LIST, *LPPROC_WHITE_LIST;

class CProcWhiteList
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
	CProcWhiteList();

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
	~CProcWhiteList();

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
	*		�ж�ָ�������Ƿ��ڰ�������
	*
	* Arguments:
	*
	*		ulPid - ����id
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
		IsIn(
		__in ULONG ulPid
		);

private:
	static LIST_ENTRY		ms_ListHead;
	static ERESOURCE		ms_Lock;
	static KSPIN_LOCK		ms_SpLock;

	KIRQL					m_Irql;
	LONG					m_LockRef;

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
	*		ɾ��ָ�����̵�PROC_WHITE_LIST�ṹ
	*
	* Arguments:
	*
	*		ulPid - ����id
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
		__in ULONG ulPid
		);

	/*++
	*
	* Routine Description:
	*
	*		����ָ�����̵�PROC_WHITE_LIST�ṹ
	*
	* Arguments:
	*
	*		ulPid - ����id
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
		__in ULONG ulPid
		);

	/*++
	*
	* Routine Description:
	*
	*		���ָ�����̵�PROC_WHITE_LIST�ṹ
	*
	* Arguments:
	*
	*		ulPid - ����id
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
	LPPROC_WHITE_LIST
		Get(
		__in ULONG ulPid
		);



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
};
