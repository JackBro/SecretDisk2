/*++
*
* Copyright (c) 2015 - 2016  �����������¿Ƽ����޹�˾
*
* Module Name:
*
*		stdafx.h
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

#ifdef __cplusplus
extern "C"
{
#endif

#include <fltKernel.h>
#include <windef.h>
#include <wdm.h>

#define NTSTRSAFE_NO_DEPRECATE
#include <Ntstrsafe.h>

#ifdef __cplusplus
}
#endif

#include "resource.h"
#include "Public.h"
#include "KrnlStr.h"
#include "FileName.h"
#include "Log.h"
#include "Minifilter.h"
