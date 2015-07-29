#include "stdafx.h"
#include "Comm.h"

COMM_CONTEXT CComm::ms_CommContext = {INVALID_HANDLE_VALUE, NULL};

VOID
	CComm::Stop()
{
	if (ms_CommContext.hCompletionPort)
	{
		CloseHandle(ms_CommContext.hCompletionPort);
		ms_CommContext.hCompletionPort = NULL;
	}

	if (INVALID_HANDLE_VALUE != ms_CommContext.hServerPort)
	{
		CloseHandle(ms_CommContext.hServerPort);
		ms_CommContext.hServerPort = INVALID_HANDLE_VALUE;
	}

	return;
}

/*
*
*������飺Ӧ�ò����ں˲㷢�Ϳ�������
*
*�������ܣ�ucCmdType -------��������������ͣ������������Ϊ��
*
*          pCmdContent------��ſ�����������׵�ַ�������������Ϊ��
*
*		   usCmdLen---------��������ȣ����ֽ�Ϊ��λ�������������Ϊ��
*
*          pCmdResult-------������������������������������Ϊ��
*                           
*����ֵ�������������ͳɹ�������TRUE�����򷵻�FALSE
*
*/
BOOL
	CComm::SendMsg(
	__in		ULONG	ulType,
	__in		LPVOID	lpInBuffer,
	__in		ULONG	ulInBufferSizeB,
	__in_opt	LPVOID	lpOutBuffer,
	__in_opt	ULONG	ulOutBufferSizeB
	)
{
	BOOL			bRet			= FALSE;

	HRESULT			hResult			= S_FALSE;
 	REQUEST_PACKET	RequstPacket	= {0};
	REPLY_PACKET	ReplyPacket		= {0};
 	DWORD			dwRet			= 0;
 	

	__try
	{
		if (!ulType || !lpInBuffer || !ulInBufferSizeB || (lpOutBuffer && !ulOutBufferSizeB))
		{
			printf("input arguments error. \n");
			__leave;
		}

		RequstPacket.ulVersion = REQUEST_PACKET_VERSION;
		RequstPacket.ulType = ulType;

		if (REQUEST_CONTENT_LENGTH < ulInBufferSizeB)
		{
			printf("ulInBufferSizeB too large. \n");
			__leave;
		}

		CopyMemory(RequstPacket.chContent, (LPSTR)lpInBuffer, ulInBufferSizeB);

		ReplyPacket.ulVersion = REPLY_PACKET_VERSION;
	
		hResult = FilterSendMessage(
			ms_CommContext.hServerPort, 
			&RequstPacket,
			REQUEST_PACKET_SIZE,
			&ReplyPacket,
			REPLY_PACKET_SIZE,
			&dwRet
			);
		if (S_OK != hResult)
		{
			printf("FilterSendMessage failed. (0x%08x) \n", hResult);
			__leave;
		}

		if (lpOutBuffer)
		{
			if (ulOutBufferSizeB < ReplyPacket.ulValidContentSizeB)
			{
				printf("ulValidContentSizeB too large. \n");
				__leave;
			}

			CopyMemory(lpOutBuffer, ReplyPacket.chContent, ReplyPacket.ulValidContentSizeB);
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
*
*������飺Ӧ�ò��ʼ��ͨ��
*
*�������ܣ���
*                           
*����ֵ�������ʼ���ɹ�������TRUE�����򷵻�FALSE
*
*/
BOOL
	CComm::Init()
{
	BOOL	bRet	= FALSE;

	HRESULT	hResult	= S_FALSE;


	__try
	{
		hResult = FilterConnectCommunicationPort(
			g_lpCommPortName,
			0,
			g_lpConnectionCtx,
			wcslen(g_lpConnectionCtx) * sizeof(WCHAR),
			NULL,
			&ms_CommContext.hServerPort
			);
		if (S_OK != hResult)
		{
			printf("FilterConnectCommunicationPort failed. (0x%08x) \n", hResult);
			__leave;
		}

		if (INVALID_HANDLE_VALUE == ms_CommContext.hServerPort)
		{
			printf("m_Context.ServerPort error. \n");
			__leave;
		}

		ms_CommContext.hCompletionPort = CreateIoCompletionPort(
			ms_CommContext.hServerPort,
			NULL,
			0,
			0
			);
		if (!ms_CommContext.hCompletionPort)
		{
			printf("CreateIoCompletionPort failed. (%d) \n", GetLastError);
			__leave;
		}
		
		bRet = TRUE;
	}
	__finally
	{
		if (!bRet)
		{
			if (ms_CommContext.hCompletionPort)
			{
				CloseHandle(ms_CommContext.hCompletionPort);
				ms_CommContext.hCompletionPort = NULL;
			}

			if (INVALID_HANDLE_VALUE != ms_CommContext.hServerPort)
			{
				CloseHandle(ms_CommContext.hServerPort);
				ms_CommContext.hServerPort = INVALID_HANDLE_VALUE;
			}
		}
	}

	return bRet;
}
