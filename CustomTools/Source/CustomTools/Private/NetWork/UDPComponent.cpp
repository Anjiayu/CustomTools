// Fill out your copyright notice in the Description page of Project Settings.


#include "NetWork/UDPComponent.h"

#include "CustomTools.h"
#include "Common/UdpSocketBuilder.h"

// Sets default values for this component's properties
UUDPComponent::UUDPComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}

void UUDPComponent::InitializeComponent()
{
    Super::InitializeComponent();
	InitSocketService();
}

void UUDPComponent::UninitializeComponent()
{
    Super::UninitializeComponent();
	UnInitSocketService();
}




// Called every frame
void UUDPComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (!RecvString.IsEmpty())
	{
		FString Message;
		RecvString.Dequeue(Message);
	    //接收消息回调
		OnRecvMsg.Broadcast(Message);
	}
}



bool UUDPComponent::SendMessage(FString ToSend)
{
    bool bRst = false;
    if (SenderSocket && RemoteAddr->IsValid())
    {
        int32 nDataLen = std::strlen(TCHAR_TO_UTF8(*ToSend));

        int32 BytesSent = 0;
        SenderSocket->SendTo((uint8*)TCHAR_TO_UTF8(*ToSend), nDataLen, BytesSent, *RemoteAddr);

        if (BytesSent <= 0)
        {
            const FString Str = "Socket is valid but the sender sent 0 bytes!";
            UE_LOG(NetParticipant, Error, TEXT("%s"), *Str);
        }
        else
        {
            bRst = true;
        }
    }

    return bRst;
}

bool UUDPComponent::SendMessageToRecvAddr(FString ToSend, FString IP, int32 PORT)
{
    bool bRst = false;
    if (SenderSocket)
    {
        RemoteAddr->SetIp(*IP, bRst);
        RemoteAddr->SetPort(PORT);

        int32 nDataLen = std::strlen(TCHAR_TO_UTF8(*ToSend));

        int32 BytesSent = 0;
        SenderSocket->SendTo((uint8*)TCHAR_TO_UTF8(*ToSend), nDataLen, BytesSent, *RemoteAddr);

        if (BytesSent <= 0)
        {
            const FString Str = "Socket is valid but the sender sent 0 bytes!";
            UE_LOG(NetParticipant, Error, TEXT("%s"), *Str);
        }
        else
        {
            bRst = true;
        }
    }

    return bRst;
}

void UUDPComponent::InitSocketService()
{
    //Set Buffer Size
    int32 BufferSize = 2 * 1024 * 1024;

    //////////////////////////////////////////////////////////////////////////
    SenderSocket = FUdpSocketBuilder(TEXT("Udp_Send")).AsReusable().WithBroadcast();
    SenderSocket->SetSendBufferSize(BufferSize, BufferSize);
    SenderSocket->SetReceiveBufferSize(BufferSize, BufferSize);

    //Create Remote Address.
    RemoteAddr = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();

    //////////////////////////////////////////////////////////////////////////
    FIPv4Endpoint Endpoint(FIPv4Address::Any, Port);

    ListenSocket = FUdpSocketBuilder(TEXT("Udp_Recv"))
        .AsNonBlocking()
        .AsReusable()
        .BoundToEndpoint(Endpoint)
        .WithReceiveBufferSize(BufferSize);
    ;

    FTimespan ThreadWaitTime = FTimespan::FromMilliseconds(100);
    UDPReceiver = new FUdpSocketReceiver(ListenSocket, ThreadWaitTime, TEXT("UDP RECEIVER"));
    UDPReceiver->OnDataReceived().BindUObject(this, &UUDPComponent::Recv);

    UDPReceiver->Start();
}

void UUDPComponent::UnInitSocketService()
{
    if (SenderSocket)
    {
        SenderSocket->Close();
        ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(SenderSocket);
    }

    delete UDPReceiver;
    UDPReceiver = nullptr;

    if (ListenSocket)
    {
        ListenSocket->Close();
        ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(ListenSocket);
    }

}

void UUDPComponent::Recv(const FArrayReaderPtr& ArrayReaderPtr, const FIPv4Endpoint& EndPt)
{
    bool bRst = false;
    static char RecvRawData[2097152] = { 0 };//2Mb

    RemoteAddr->SetIp(EndPt.Address.Value);
    RemoteAddr->SetPort(EndPt.Port);
    int32 dataByteNum = ArrayReaderPtr->Num();
    memcpy(RecvRawData, ArrayReaderPtr->GetData(), dataByteNum);
    RecvRawData[dataByteNum] = 0;
    FString tryToString(UTF8_TO_TCHAR(RecvRawData));
    RecvString.Enqueue(MoveTemp(tryToString));
   
   
}