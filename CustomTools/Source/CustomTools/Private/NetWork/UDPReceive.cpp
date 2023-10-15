// Fill out your copyright notice in the Description page of Project Settings.


#include "NetWork/UDPReceive.h"

#include "Common/UdpSocketBuilder.h"

// Sets default values for this component's properties
AUDPReceive::AUDPReceive()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryActorTick.bCanEverTick = true;
	SetActorTickEnabled(true);

	// ...
}


// Called when the game starts
void AUDPReceive::BeginPlay()
{
	Super::BeginPlay();

	// ...
	int32 BufferSize = 2 * 1024 * 1024;

	bool IsValid = false;
	//Create Remote Address.
	RemoteAddr = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
	FIPv4Endpoint Endpoint(FIPv4Address::Any, Port);
	
	ListenSocket = FUdpSocketBuilder(TEXT("Udp_Recv"))
		.AsNonBlocking()
		.AsReusable()
		.BoundToEndpoint(Endpoint)
		.WithReceiveBufferSize(BufferSize);
	;

	FTimespan ThreadWaitTime = FTimespan::FromMilliseconds(100);
	UDPReceiver = new FUdpSocketReceiver(ListenSocket, ThreadWaitTime, TEXT("UDP RECEIVER"));
	UDPReceiver->OnDataReceived().BindUObject(this, &AUDPReceive::Recv);

	UDPReceiver->Start();
}

void AUDPReceive::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	
	delete UDPReceiver;
	UDPReceiver = nullptr;

	if (ListenSocket)
	{
		ListenSocket->Close();
		ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(ListenSocket);
	}
}


// Called every frame
void AUDPReceive::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// ...
	if (!RecvString.IsEmpty())
	{
		FString Message;
		RecvString.Dequeue(Message);
		RecvMessage(Message);
	}
}

void AUDPReceive::Recv(const FArrayReaderPtr& ArrayReaderPtr, const FIPv4Endpoint& EndPt)
{
	static char RecvRawData[2097152] = { 0 };//2Mb\
	
	int32 dataByteNum = ArrayReaderPtr->Num();
	memcpy(RecvRawData, ArrayReaderPtr->GetData(), dataByteNum);
	RecvRawData[dataByteNum] = 0;

	FString tryToString(UTF8_TO_TCHAR(RecvRawData));
	RecvString.Enqueue(MoveTemp(tryToString));
}

