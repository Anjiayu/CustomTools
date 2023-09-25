// Fill out your copyright notice in the Description page of Project Settings.


#include "NetWork/WebSocketActor.h"

#include "WebSocketsModule.h"

// Sets default values
AWebSocketActor::AWebSocketActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AWebSocketActor::BeginPlay()
{
	Super::BeginPlay();
	
}

void AWebSocketActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	//结束时关闭Socket;
	Socket->Close();
}

// Called every frame
void AWebSocketActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}



void AWebSocketActor::CreateWebSocket(FString URL)
{
	Socket = FWebSocketsModule::Get().CreateWebSocket(URL);
	Socket->OnConnected().AddUObject(this,&AWebSocketActor::OnWebSocketConnect);
	Socket->OnMessage().AddUObject(this,&AWebSocketActor::OnMessageReceive);
	Socket->Connect();
}

void AWebSocketActor::OnWebSocketConnect()
{
	
	
	Socket->OnMessageSent().AddUObject(this,&AWebSocketActor::OnMessageSend);
	Socket->OnClosed().AddUObject(this,&AWebSocketActor::OnClosed);
}

void AWebSocketActor::OnMessageReceive(const FString& Message)
{
	//将消息同步至蓝图
	OnMessageReceived.Broadcast(Message);
}

void AWebSocketActor::OnMessageSend(const FString& Message)
{
	OnMessageSent.Broadcast(Message);
}

void AWebSocketActor::OnClosed(int32 StatusCode, const FString& Reason, bool bWasClean)
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1,
			10.f,
			FColor::Blue,
			TEXT("WebSocket is Closed!"));
	}
}

int32 AWebSocketActor::SendMessageByWebsocket(FString SendMessage)
{
	int32 BytesSent = 0;
	//检测位
	BytesSent = std::strlen(TCHAR_TO_UTF8(*SendMessage));
	Socket->Send(SendMessage);
	return BytesSent;
}

