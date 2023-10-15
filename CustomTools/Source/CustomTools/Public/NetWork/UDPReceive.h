// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Common/UdpSocketReceiver.h"
#include "Components/ActorComponent.h"
#include "UDPReceive.generated.h"


UCLASS()
class CUSTOMTOOLS_API AUDPReceive : public AActor
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	AUDPReceive();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(BlueprintReadWrite,EditAnywhere, Category = "UDPSetting")
	FString SocketName;

	UPROPERTY(BlueprintReadWrite,EditAnywhere, Category = "UDPSetting")
	int32 Port;
	
	UFUNCTION(BlueprintImplementableEvent, Category = "UDP")
	void RecvMessage(const FString& tryToString);


	TSharedPtr<FInternetAddr> RemoteAddr;
	FSocket* ListenSocket;
	FUdpSocketReceiver* UDPReceiver = nullptr;
	void Recv(const FArrayReaderPtr& ArrayReaderPtr, const FIPv4Endpoint& EndPt);
	//线程安全传递介体
	TQueue<FString> RecvString;//Recv Thread && Game Thread
};
