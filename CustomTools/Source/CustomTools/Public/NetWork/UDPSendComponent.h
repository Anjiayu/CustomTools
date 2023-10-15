// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "UDPSendComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class CUSTOMTOOLS_API UUDPSendComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UUDPSendComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category = "UDPSend")
	bool SendMessage(FString MessageToSend);

	UPROPERTY(BlueprintReadWrite,EditAnywhere, Category = "UDPSetting")
	FString SocketName;

	UPROPERTY(BlueprintReadWrite,EditAnywhere, Category = "UDPSetting")
	FString IP;

	UPROPERTY(BlueprintReadWrite,EditAnywhere, Category = "UDPSetting")
	int32 Port;

	FSocket* SenderSocket;
	TSharedPtr<FInternetAddr> RemoteAddr;
};
