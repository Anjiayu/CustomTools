// Fill out your copyright notice in the Description page of Project Settings.


#include "DataConversion/K2Node_DecodeBase64.h"

UK2Node_DecodeBase64* UK2Node_DecodeBase64::DecodeBase64(UObject* WordContextObject,const FString& InString,EExBase64outputType outputType)
{
	UK2Node_DecodeBase64* Ins = NewObject<UK2Node_DecodeBase64>();
	Ins->StringToDecode = InString;
	Ins->Type = outputType;
	return Ins;
}

void UK2Node_DecodeBase64::Activate()
{
	FString outString;
	TArray<uint8> outBytesArray;
	Async(EAsyncExecution::ThreadPool,[&]()
	{
		switch (Type)
		{
		case EExBase64outputType::FString:
			FBase64::Decode(StringToDecode,outString);
			break;
		case EExBase64outputType::OutBytesArray:
			FBase64::Decode(StringToDecode,outBytesArray);
			break;
		}
		Out.Broadcast(EExBase64outputType::OutBytesArray,outBytesArray,outString);
	});
	
}


