// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "K2Node_DecodeBase64.generated.h"

/**
 * 
 */

UENUM(BlueprintType)
enum class EExBase64outputType: uint8
{
	OutBytesArray	UMETA(DisplayName = "uInt8 Array"),
	FString			UMETA(DisplayName = "FString"),
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FBPDecodBase64_Result,EExBase64outputType,OutputType,const TArray<uint8> &,OutBytes,const FString &,OutString);

UCLASS()
class CUSTOMTOOLS_API UK2Node_DecodeBase64 : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()
	UK2Node_DecodeBase64(){};
	~UK2Node_DecodeBase64(){};

	UFUNCTION(BlueprintCallable,meta = (WorldContext = "WorldContextObject"))
	static UK2Node_DecodeBase64* DecodeBase64(UObject* WordContextObject,const FString& InString,EExBase64outputType OutputType);

	FString StringToDecode;
	EExBase64outputType Type;
	
	//实际调用
	virtual void Activate() override;

	//Pin口
	UPROPERTY(BlueprintAssignable)
	FBPDecodBase64_Result Out;

};
