/*++
*
* Copyright (c) 2015 - 2016  �����������¿Ƽ����޹�˾
*
* Module Name:
*
*		Minifilter.h
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

#define MOD_MINIFITER									L"Minifilter"
#define MEMORY_TAG_MINIFILTER							'TFNM'			// MNFT
#define	MEMORY_TAG_PRE_CREATE							'TCRP'			// PRCT
#define	MEMORY_TAG_PRE_SET_INFORMATION					'ISRP'			// PRSI
#define	MEMORY_TAG_PRE_DIRECTORY_CONTROL				'CDRP'			// PRDC
#define	MEMORY_TAG_POST_DIRECTORY_CONTROL				'CDOP'			// PODC
#define MEMORY_TAG_INSTANCE_TAG							'TTSI' 			// ISTT
#define	MEMORY_TAG_PRE_POST_DIRECTORY_CONTRL_CONTEXT	'CDPP'			// PPDC
#define CALLBACKS_NUM									5
#define MIN_SECTOR_SIZE									0x200

typedef enum _MINIFILTER_ENV_TYPE
{
	MINIFILTER_ENV_TYPE_NULL			= 0x00000000,
	MINIFILTER_ENV_TYPE_ALL_MODULE_INIT = 0x00000001,
	MINIFILTER_ENV_TYPE_ALLOW_WORK		= 0x00000002,
	MINIFILTER_ENV_TYPE_RUNING			= 0x00000004,
	MINIFILTER_ENV_TYPE_FLT_FILTER		= 0x00000008
} MINIFILTER_ENV_TYPE, *PMINIFILTER_ENV_TYPE, *LPMINIFILTER_ENV_TYPE;

typedef struct _PRE_POST_DIRECTORY_CONTROL_CONTEXT
{
	CKrnlStr FileName;
} PRE_POST_DIRECTORY_CONTROL_CONTEXT, *PPRE_POST_DIRECTORY_CONTROL_CONTEXT, *LPPRE_POST_DIRECTORY_CONTROL_CONTEXT;

extern "C" DRIVER_INITIALIZE DriverEntry;

/*++
*
* Routine Description:
*
*		This is the initialization routine for this miniFilter driver.  This
*		registers with FltMgr and initializes all global data structures.
*
* Arguments:
*
*		DriverObject -	Pointer to driver object created by the system to
*						represent this driver.
*
*		RegistryPath -	Unicode string identifying where the parameters for this
*						driver are located in the registry.
*
* Return Value:
*
*		Returns STATUS_SUCCESS.
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
NTSTATUS
	DriverEntry(
	__in PDRIVER_OBJECT		DriverObject,
	__in PUNICODE_STRING	RegistryPath
	);

class CMinifilter
{
public:
	static CMinifilter *	ms_pMfIns;

	PFLT_FILTER				m_pFltFilter;
	PDEVICE_OBJECT			m_pDevObj;

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
	CMinifilter();

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
	~CMinifilter();

	/*++
	*
	* Routine Description:
	*
	*		ע��Minifilter������Minifilter����ʼ����ģ��
	*
	* Arguments:
	*
	*		pDriverObject
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
		Register(
		__in PDRIVER_OBJECT pDriverObject
		);

	/*++
	*
	* Routine Description:
	*
	*		��黷��
	*
	* Arguments:
	*
	*		ulMinifilterEnvType - �����
	*
	* Return Value:
	*
	*		TURE	- ����
	*		FALSE	- ������
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
		CheckEnv(
		__in ULONG ulMinifilterEnvType
		);

private:
	BOOLEAN						m_AllModuleInit;
	BOOLEAN						m_bUnloading;
	BOOLEAN						m_bAllowFltWork;
	FLT_OPERATION_REGISTRATION	m_Callbacks[CALLBACKS_NUM];
	FLT_REGISTRATION			m_FltRegistration;
	CKrnlStr					m_SymbolicLinkName;

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
	*		This is the unload routine for this miniFilter driver. This is called
	*		when the minifilter is about to be unloaded. We can fail this unload
	*		request if this is not a mandatory unloaded indicated by the Flags
	*		parameter.
	*
	* Arguments:
	*
	*		Flags - Indicating if this is a mandatory unload.
	*
	* Return Value:
	*
	*		Returns the final status of this operation.
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
	static
		NTSTATUS
		Unload(
		__in FLT_FILTER_UNLOAD_FLAGS Flags
		);

	/*++
	*
	* Routine Description:
	*
	*		operation registration
	*
	* Arguments:
	*
	*		��
	*
	* Return Value:
	*
	*		TURE	- �ɹ�
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
		InitCallbacks();

	/*++
	*
	* Routine Description:
	*
	*		This defines what we want to filter with FltMgr
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
		InitRegistration();

	/*++
	*
	* Routine Description:
	*
	*		This routine is called whenever a new instance is created on a volume. This
	*		gives us a chance to decide if we need to attach to this volume or not.
	*
	*		If this routine is not defined in the registration structure, automatic
	*		instances are alwasys created.
	*
	* Arguments:
	*
	*		FltObjects	-	Pointer to the FLT_RELATED_OBJECTS data structure containing
	*						opaque handles to this filter, instance and its associated volume.
	*
	*		Flags		-	Flags describing the reason for this attach request.
	*
	* Return Value:
	*
	*		STATUS_SUCCESS				- attach
	*		STATUS_FLT_DO_NOT_ATTACH	- do not attach
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
	static
		NTSTATUS
		InstanceSetup(
		__in PCFLT_RELATED_OBJECTS		FltObjects,
		__in FLT_INSTANCE_SETUP_FLAGS	Flags,
		__in DEVICE_TYPE				VolumeDeviceType,
		__in FLT_FILESYSTEM_TYPE		VolumeFilesystemType
		);

	/*++
	*
	* Routine Description:
	*
	*		This is called when an instance is being manually deleted by a
	*		call to FltDetachVolume or FilterDetach thereby giving us a
	*		chance to fail that detach request.
	*
	*		If this routine is not defined in the registration structure, explicit
	*		detach requests via FltDetachVolume or FilterDetach will always be
	*		failed.
	*
	* Arguments:
	*
	*		FltObjects	-	Pointer to the FLT_RELATED_OBJECTS data structure containing
	*							opaque handles to this filter, instance and its associated volume.
	*
	*		Flags		-	Indicating where this detach request came from.
	*
	* Return Value:
	*
	*		Returns the status of this operation.
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
	static
		NTSTATUS
		InstanceQueryTeardown(
		__in PCFLT_RELATED_OBJECTS				FltObjects,
		__in FLT_INSTANCE_QUERY_TEARDOWN_FLAGS	Flags
		);

	/*++
	*
	* Routine Description:
	*
	*		This routine is called at the start of instance teardown.
	*
	* Arguments:
	*
	*		FltObjects	-	Pointer to the FLT_RELATED_OBJECTS data structure containing
	*						opaque handles to this filter, instance and its associated volume.
	*
	*		Flags		-	Reason why this instance is been deleted.
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
	static
		VOID
		InstanceTeardownStart(
		__in PCFLT_RELATED_OBJECTS			FltObjects,
		__in FLT_INSTANCE_TEARDOWN_FLAGS	Flags
		);

	/*++
	*
	* Routine Description:
	*
	*		This routine is called at the end of instance teardown.
	*
	* Arguments:
	*
	*		FltObjects	-	Pointer to the FLT_RELATED_OBJECTS data structure containing
	*						opaque handles to this filter, instance and its associated volume.
	*
	*		Flags		-	Reason why this instance is been deleted.
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
	static
		VOID
		InstanceTeardownComplete(
		__in PCFLT_RELATED_OBJECTS			FltObjects,
		__in FLT_INSTANCE_TEARDOWN_FLAGS	Flags
		);

	/*++
	*
	* Routine Description:
	*
	*		This routine is the main pre-operation dispatch routine for this
	*		miniFilter. Since this is just a simple passThrough miniFilter it
	*		does not do anything with the callbackData but rather return
	*		FLT_PREOP_SUCCESS_WITH_CALLBACK thereby passing it down to the next
	*		miniFilter in the chain.
	*
	*		This is non-pageable because it could be called on the paging path
	*
	* Arguments:
	*
	*		Data				-	Pointer to the filter callbackData that is passed to us.
	*
	*		FltObjects			-	Pointer to the FLT_RELATED_OBJECTS data structure containing
	*								opaque handles to this filter, instance, its associated volume and
	*								file object.
	*
	*		CompletionContext	-	The context for the completion routine for this
	*								operation.
	*
	* Return Value:
	*
	*		The return value is the status of the operation.
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
	static
		FLT_PREOP_CALLBACK_STATUS
		PreCreate(
		__inout			PFLT_CALLBACK_DATA			Data,
		__in			PCFLT_RELATED_OBJECTS		FltObjects,
		__deref_out_opt PVOID					*	CompletionContext
		);

	/*++
	*
	* Routine Description:
	*
	*		This routine is the main pre-operation dispatch routine for this
	*		miniFilter. Since this is just a simple passThrough miniFilter it
	*		does not do anything with the callbackData but rather return
	*		FLT_PREOP_SUCCESS_WITH_CALLBACK thereby passing it down to the next
	*		miniFilter in the chain.
	*
	*		This is non-pageable because it could be called on the paging path
	*
	* Arguments:
	*
	*		Data				-	Pointer to the filter callbackData that is passed to us.
	*
	*		FltObjects			-	Pointer to the FLT_RELATED_OBJECTS data structure containing
	*								opaque handles to this filter, instance, its associated volume and
	*								file object.
	*
	*		CompletionContext	-	The context for the completion routine for this
	*								operation.
	*
	* Return Value:
	*
	*		The return value is the status of the operation.
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
	static
		FLT_PREOP_CALLBACK_STATUS
		PreSetInformation(
		__inout			PFLT_CALLBACK_DATA			Data,
		__in			PCFLT_RELATED_OBJECTS		FltObjects,
		__deref_out_opt PVOID					*	CompletionContext
		);

		/*++
	*
	* Routine Description:
	*
	*		This routine is the main pre-operation dispatch routine for this
	*		miniFilter. Since this is just a simple passThrough miniFilter it
	*		does not do anything with the callbackData but rather return
	*		FLT_PREOP_SUCCESS_WITH_CALLBACK thereby passing it down to the next
	*		miniFilter in the chain.
	*
	*		This is non-pageable because it could be called on the paging path
	*
	* Arguments:
	*
	*		Data				-	Pointer to the filter callbackData that is passed to us.
	*
	*		FltObjects			-	Pointer to the FLT_RELATED_OBJECTS data structure containing
	*								opaque handles to this filter, instance, its associated volume and
	*								file object.
	*
	*		CompletionContext	-	The context for the completion routine for this
	*								operation.
	*
	* Return Value:
	*
	*		The return value is the status of the operation.
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
	static
		FLT_PREOP_CALLBACK_STATUS
		PreDeviceControl(
		__inout			PFLT_CALLBACK_DATA			Data,
		__in			PCFLT_RELATED_OBJECTS		FltObjects,
		__deref_out_opt PVOID					*	CompletionContext
		);

	/*++
	*
	* Routine Description:
	*
	*		This routine is the main pre-operation dispatch routine for this
	*		miniFilter. Since this is just a simple passThrough miniFilter it
	*		does not do anything with the callbackData but rather return
	*		FLT_PREOP_SUCCESS_WITH_CALLBACK thereby passing it down to the next
	*		miniFilter in the chain.
	*
	*		This is non-pageable because it could be called on the paging path
	*
	* Arguments:
	*
	*		Data				-	Pointer to the filter callbackData that is passed to us.
	*
	*		FltObjects			-	Pointer to the FLT_RELATED_OBJECTS data structure containing
	*								opaque handles to this filter, instance, its associated volume and
	*								file object.
	*
	*		CompletionContext	-	The context for the completion routine for this
	*								operation.
	*
	* Return Value:
	*
	*		The return value is the status of the operation.
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
	static
		FLT_PREOP_CALLBACK_STATUS
		PreDirectoryControl(
		__inout			PFLT_CALLBACK_DATA			Data,
		__in			PCFLT_RELATED_OBJECTS		FltObjects,
		__deref_out_opt PVOID					*	CompletionContext
		);

	/*++
	*
	* Routine Description:
	*
	*		This routine is the post-operation completion routine for this
	*		miniFilter.
	*
	*		This is non-pageable because it may be called at DPC level.
	*
	* Arguments:
	*
	*		Data				-	Pointer to the filter callbackData that is passed to us.
	*
	*		FltObjects			-	Pointer to the FLT_RELATED_OBJECTS data structure containing
	*								opaque handles to this filter, instance, its associated volume and
	*								file object.
	*
	*		CompletionContext	-	The completion context set in the pre-operation routine.
	*
	*		Flags				-	Denotes whether the completion is successful or is being drained.
	*
	* Return Value:
	*
	*		The return value is the status of the operation.
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
	static
		FLT_POSTOP_CALLBACK_STATUS
		PostDirectoryControl(
		__inout		PFLT_CALLBACK_DATA			Data,
		__in		PCFLT_RELATED_OBJECTS		FltObjects,
		__in_opt	PVOID						CompletionContext,
		__in		FLT_POST_OPERATION_FLAGS	Flags
		);

	VOID
		DisallowFltWork();

	VOID
		AllowFltWork();

	BOOLEAN
		CreateSymbolicLinkName(
		__in PDRIVER_OBJECT pDriverObj
		);
};
