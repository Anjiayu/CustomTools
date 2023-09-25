// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Common/UdpSocketReceiver.h"
#include "UDPComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FUDPSocketRecvMsg, const FString&, Text);


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class CUSTOMTOOLS_API UUDPComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UUDPComponent();
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	virtual void InitializeComponent() override;
	virtual void UninitializeComponent() override;
	
	void InitSocketService();
	void UnInitSocketService();


	UFUNCTION(BlueprintCallable, Category = "UDP")
	bool SendMessage(FString ToSend);
	
	UFUNCTION(BlueprintCallable, Category = "UDP")
	bool SendMessageToRecvAddr(FString ToSend, FString IP, int32 Port);
		
	UPROPERTY(BlueprintAssignable)
	FUDPSocketRecvMsg OnRecvMsg;

	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	FString Ip;

	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	int32 Port;
	
	FSocket* SenderSocket;
	TSharedPtr<FInternetAddr> RemoteAddr;

	FSocket* ListenSocket;
	FUdpSocketReceiver* UDPReceiver = nullptr;
	void Recv(const FArrayReaderPtr& ArrayReaderPtr, const FIPv4Endpoint& EndPt);

	TQueue<FString> RecvString;//Recv Thread && Game Thread
};
