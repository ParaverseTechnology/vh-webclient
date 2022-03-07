// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "LarkXRDataChannel.h"
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LarkXRCloudAppCompnent.generated.h"

UENUM(BlueprintType)
enum class LarkDataChannelType : uint8 {
	DATA_STRING = 0,
	DATA_BIN = 1,
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTaskStatus, bool, status, FString, taskId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FDataChannelOnData, LarkDataChannelType, type, FString, dataText);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnAiVoiceResult, bool, isUrl, FString, ngl, FString, url);
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class LARKXRCLOUDAPP_API ULarkXRCloudAppCompnent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	ULarkXRCloudAppCompnent();

	UPROPERTY(EditAnyWhere, BlueprintReadWrite, BlueprintAssignable, Category = "Events|LarkXR")
		FDataChannelOnData p_FDataChannelOnData;	
	
	UPROPERTY(EditAnyWhere, BlueprintReadWrite, BlueprintAssignable, Category = "Events|LarkXR")
		FOnTaskStatus p_FOnTaskStatus;

	UPROPERTY(EditAnyWhere, BlueprintReadWrite, BlueprintAssignable, Category = "Events|LarkXR")
		FOnAiVoiceResult p_FOnAiVoiceResult;
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
	// ��ȡ��ǰ�� TaskID
	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "LarkXR")
		FString TaskID;

	// �Ƿ�����Ϸ�̻߳ص�
	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "LarkXR")
		bool bCallbackOnGameThread = true;

	//************************************
	// Method:    lr_client_register_getTaskId_callback
	// FullName:  lr_client_register_getTaskId_callback
	// Access:    public 
	// Returns:   LARKXR_API void DC_CALL
	// Qualifier: Ӧ��ͨ���˻ص��ӿڻ�ȡ�ͻ�������״̬�Լ�taskid
	// Parameter: on_taskid get_task
	// Parameter: void * user_data
	//************************************
	UFUNCTION(BlueprintCallable, Category = "LarkXR")
		void LR_Client_Register_Taskstatus_Callback();

	/*
		datachannel�ӿڶ����첽��

		lr_client_start ���سɹ�ֻ��˵���ӿڴ����ɹ�

		�Ƿ����ӳɹ���Ҫ���ӿڵ��õĻص����������ʧ���� ��鿴on_disconnected�Ĳ���

	*/
	UFUNCTION(BlueprintCallable, Category = "LarkXR")
		int LR_Client_Start(FString debugID = "");

	/*
		lr_client_send �����ַ��� ʱ�� �뱣֤ �ַ�����utf8����,��������\0
		���ͳɹ� ����ֵΪ���͵ĳ���
		ʧ�� ��鿴����ֵ
	*/
	UFUNCTION(BlueprintCallable, Category = "LarkXR")
		static int LR_Client_SendText(FString data);

	UFUNCTION(BlueprintCallable, Category = "LarkXR")
		static int LR_Client_SendBinary(TArray<uint8> data);

	/*
	* stop
	*/
	UFUNCTION(BlueprintCallable, Category = "LarkXR")
		static void LR_Client_Stop();

	/**
	* ���������������Զ�ע��ص�
	* ���� R_SUCCESS:ע��ɹ�	 XR_ERROR_INTERFACE_FAILED:û�е��� r_client_start XR_ERROR_SERVER_UNSUPPORT:����ͨ����֧��/����������֧��
	* ���ô˺���һ��Ҫ��lr_client_start�ɹ������һص�����Ҳ���سɹ����ٵ���
	* ��ǰ������Ӧ���ú������������񣬲����ں�̨�ϴ�Ӧ��ʱ��ʼ������������
	*/
	UFUNCTION(BlueprintCallable, Category = "LarkXR")
		int LR_Client_AiVoice_Start();

private:
	static void cc_on_connected(void* user_data);

	static void cc_on_data(data_type type, const char* data, int size, void* user_data);

	static void cc_on_disconnected(enum ErrorCode code, void* user_data);

	static void cc_on_taskstatus(bool status/*true:�ͻ������� false:�ͻ��˶Ͽ�*/, const char* taskId, void* user_data);

	static void cc_on_aivoice_callback(struct AiVoicePacket* packet, void* user_data);
};
