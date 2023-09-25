// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TCPClientComponent.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTCPClientRecvMsg, const FString&, Text);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FTCPClientConnectState, bool, bConnectState, const FString&, Text);


UCLASS( ClassGroup= "NetWorking", meta=(BlueprintSpawnableComponent) )
class CUSTOMTOOLS_API UTCPClientComponent : public UActorComponent
{
	GENERATED_BODY()

public:	


	UPROPERTY(BlueprintAssignable, Category = "TCP Events")
	FTCPClientRecvMsg OnReceived;

	UPROPERTY(BlueprintAssignable, Category = "TCP Events")
	FTCPClientConnectState OnConnected;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TCP Connection Properties")
	FString ClientSocketName;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TCP Connection Properties")
	int32 BufferMaxSize;


	UFUNCTION(BlueprintCallable, Category = "TCP Functions")
	void ConnectToServerAsClient(const FString& IP = TEXT("127.0.0.1"), const int32 Port = 3000, const int32 RateSecond = 5);

	UFUNCTION(BlueprintCallable, Category = "TCP Functions")
	void CloseSocket();

	UFUNCTION(BlueprintCallable, Category = "TCP Functions")
	bool Send(const FString& Text);
protected:
	
	FSocket* ClientSocket;
	FThreadSafeBool bShouldReceiveData;
	TFuture<void> ClientConnectionFinishedFuture;

	//FTCPSocketReceiver* TCPReceiver;
	FString SocketDescription;
	TSharedPtr<FInternetAddr> RemoteAdress;

	TQueue<FString> MsgReceived;

public:
	// Called when the game starts
	virtual void BeginPlay() override;
	// Sets default values for this component's properties
	UTCPClientComponent(const FObjectInitializer &init);
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	virtual void InitializeComponent() override;

	virtual void UninitializeComponent() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
		
};
