#pragma once

#define MOD_COMM_WITH_DEVICE L"���豸ͨѶ"

// #define DEVICE_NAME L"\\\\.\\HelloWDM"


class CCommWithDevice
{
public:
	BOOL
		SendMsg(
		__in LPWSTR	lpSymbolicLinkName,
		__in ULONG	ulType,
		__in LPVOID	lpInputBuf,
		__in ULONG	ulInputBufLenB
		);
};
