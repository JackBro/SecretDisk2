/*++
*
* Copyright (c) 2015 - 2016  �����������¿Ƽ����޹�˾
*
* Module Name:
*
*		Log.h
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

#define MOD_LOG						L"��־"
#define MEMORY_TAG_LOG				'GOL'				// LOG
#define	MAX_EVERY_TIME_LOG_COUNT	1
#define MAX_LOG_FILE_SIZE			(9 * 1024 * 1024)

typedef struct _LOG_INFO
{
	CKrnlStr	Log;

	LIST_ENTRY	List;
} LOG_INFO, *PLOG_INFO, *LPLOG_INFO;

class CLog
{
public:
	static PFLT_INSTANCE	ms_pFltInstance;

	CLog();

	~CLog();

	/*
	* ����˵����
	*		��ʼ��
	*
	* ������
	*		��
	*
	* ����ֵ��
	*		TRUE	�ɹ�
	*		FALSE	ʧ��
	*
	* ��ע��
	*		��
	*/
	BOOLEAN
		Init();

	/*
	* ����˵����
	*		ж��
	*
	* ������
	*		��
	*
	* ����ֵ��
	*		TRUE	�ɹ�
	*		FALSE	ʧ��
	*
	* ��ע��
	*		��
	*/
	BOOLEAN
		Unload();

	/*
	* ����˵����
	*		��ʼ����־�ļ�
	*
	* ������
	*		bReset	�Ƿ�������־�ļ�
	*
	* ����ֵ��
	*		TRUE	�ɹ�
	*		FALSE	ʧ��
	*
	* ��ע��
	*		��
	*/
	BOOLEAN
		InitLogFile(
		__in BOOLEAN bReset = FALSE
		);

	BOOLEAN
		Insert(
		__in WCHAR	*	pLog,
		__in USHORT		usLenCh
		);

	VOID
		GetLock();

	VOID
		FreeLock();

	/*
	* ����˵����
	*		�ͷ���־�ļ�
	*
	* ������
	*		��
	*
	* ����ֵ��
	*		TRUE	�ɹ�
	*		FALSE	ʧ��
	*
	* ��ע��
	*		��
	*/
	BOOLEAN
		ReleaseLogFile();

	/*
	* ����˵����
	*		������־�ļ�����Ŀ¼
	*
	* ������
	*		pLogDir	��־�ļ�����Ŀ¼
	*
	* ����ֵ��
	*		TRUE	�ɹ�
	*		FALSE	ʧ��

	* ��ע��
	*		��
	*/
	BOOLEAN
		SetLogDir(
		__in CKrnlStr * pLogDir
		);

private:
	static LIST_ENTRY		ms_ListHead;
	static ERESOURCE		ms_Lock;
	static KSPIN_LOCK		ms_SpLock;
	static KEVENT			ms_UnloadEvent;
	static CKrnlStr*		ms_pLogFile;
	static CKrnlStr*		ms_pLogDir;
	static HANDLE			ms_hLogFile;
	static PFILE_OBJECT		ms_pLogFileObj;
	static LARGE_INTEGER	ms_liByteOffset;
	static PETHREAD			ms_pEThread;
	static BOOLEAN			ms_bCanInsertLog;
	static ULONG			ms_ulSectorSize;

	static KSTART_ROUTINE	ThreadStart;

	KIRQL					m_Irql;
	LONG					m_LockRef;

	/*
	* ����˵����
	*		��ȡ��־
	*
	* ������
	*		pLog	��־
	*
	* ����ֵ��
	*		TRUE	�ɹ�
	*		FALSE	ʧ��
	*
	* ��ע��
	*		��
	*/
	BOOLEAN
		Pop(
		__out CKrnlStr * pLog
		);

	/*
	* ����˵����
	*		д��־
	*
	* ������
	*		pLog	��־
	*
	* ����ֵ��
	*		TRUE	�ɹ�
	*		FALSE	ʧ��
	*
	* ��ע��
	*		��
	*/
	BOOLEAN
		Write(
		__in CKrnlStr * pLog
		);

	/*
	* ����˵����
	*		�ж���־�ļ��Ƿ��Ѿ�׼�����
	*
	* ������
	*		��
	*
	* ����ֵ��
	*		TRUE	׼�����
	*		FALSE	��δ׼�����
	*
	* ��ע��
	*		��
	*/
	BOOLEAN
		LogFileReady();

	/*
	* ����˵����
	*		��ʼ����־�ļ���
	*
	* ������
	*		��
	*
	* ����ֵ��
	*		TRUE	�ɹ�
	*		FALSE	ʧ��	
	*
	* ��ע��
	*		��
	*/
	BOOLEAN
		InitLogFileName();
};
