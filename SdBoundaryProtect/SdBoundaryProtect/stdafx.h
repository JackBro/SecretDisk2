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
#include <Wdmsec.h>

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
#include "ProcWhiteList.h"
#include "DirHide.h"
#include "../../Comm/CommPublic.h"
#include "DirControlList.h"
#include "File.h"
#include "Comm.h"
#include "Minifilter.h"
