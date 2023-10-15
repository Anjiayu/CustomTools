// Fill out your copyright notice in the Description page of Project Settings.


#include "NetWork/UDPSendComponent.h"

#include "Common/UdpSocketBuilder.h"
#include "Common/UdpSocketSender.h"

// Sets default values for this component's properties
UUDPSendComponent::UUDPSendComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UUDPSendComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...

	int32 BufferSize = 2 * 1024 * 1024;
	SenderSocket = FUdpSocketBuilder(SocketName).AsReusable().WithBroadcast();
	SenderSocket->SetSendBufferSize(BufferSize,BufferSize);

	//Create Remote Address.
	RemoteAddr = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
	
}


// Called every frame
void UUDPSendComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

bool UUDPSendComponent::SendMessage(FString MessageToSend)
{
	bool bRst = false;
	if (SenderSocket)
	{
		RemoteAddr->SetIp(*IP, bRst);
		RemoteAddr->SetPort(Port);

		int32 nDataLen = std::strlen(TCHAR_TO_UTF8(*MessageToSend));

		int32 BytesSent = 0;
		SenderSocket->SendTo((uint8*)TCHAR_TO_UTF8(*MessageToSend), nDataLen, BytesSent, *RemoteAddr);

		if (BytesSent <= 0)
		{
			const FString Str = "Socket is valid but the sender sent 0 bytes!";
		}
		else
		{
			bRst = true;
		}
	}

	return bRst;
}

