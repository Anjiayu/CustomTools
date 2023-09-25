// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "IWebSocket.h"
#include "WebSocketActor.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMessageReceived,FString,Message);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMessageSent,FString,Message);

UCLASS()
class CUSTOMTOOLS_API AWebSocketActor : public AActor
{
	GENERATED_BODY()
public:	
	// Sets default values for this actor's properties
	AWebSocketActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

public:
	//回调委托
	UPROPERTY(BlueprintAssignable)
	FOnMessageReceived OnMessageReceived;
	UPROPERTY(BlueprintAssignable)
	FOnMessageSent OnMessageSent;
	
public:
	TSharedPtr<IWebSocket> Socket = nullptr;
	//蓝图调用函数
	//1.创建websocket
	UFUNCTION(BlueprintCallable)
	void CreateWebSocket(FString URL);
	//2.通过websocket发送Message
	UFUNCTION(BlueprintCallable)
	int32 SendMessageByWebsocket(FString SendMessage);


	//创建websocket并连接成功后，绑定回调事件
	void OnWebSocketConnect();
	
	//消息回调绑定的方法
	void OnMessageReceive(const FString& Message);
	void OnMessageSend(const FString& Message);
	void OnClosed(int32 StatusCode, const FString& Reason, bool bWasClean);
	

};
