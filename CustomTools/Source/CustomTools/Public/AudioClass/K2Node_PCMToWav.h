// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "K2Node_PCMToWav.generated.h"

/**
 * 
 */

//Wav文件头，注意格式
//Wav与PCM数据转换关系：https://zhuanlan.zhihu.com/p/565403601?utm_id=0
struct wave_pcm_hdr
{
	char            riff[4];                // = "RIFF"
	int				size_8;                 // = FileSize - 8
	char            wave[4];                // = "WAVE"
	char            fmt[4];                 // = "fmt "
	int				fmt_size;				// = 下一个结构体的大小 : 16

	short int       format_tag;             // = PCM : 1
	short int       channels;               // = 通道数 : 1
	int				samples_per_sec;        // = 采样率 : 8000 | 6000 | 11025 | 16000
	int				avg_bytes_per_sec;      // = 每秒字节数 : samples_per_sec * bits_per_sample / 8
	short int       block_align;            // = 每采样点字节数 : wBitsPerSample / 8
	short int       bits_per_sample;        // = 量化比特数: 8 | 16

	char            data[4];                // = "data";
	int				data_size;              // = 纯数据长度 : FileSize - 44 
} ;



DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FBPPCMToWav_Result,const FString &,WavFileName);


UCLASS()
class CUSTOMTOOLS_API UK2Node_PCMToWav : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

	//将需要写入的PCM数据格式头编辑好
	wave_pcm_hdr default_wav_hdr =
		{
		{ 'R', 'I', 'F', 'F' },
		0,
		{'W', 'A', 'V', 'E'},
		{'f', 'm', 't', ' '},
		16,
		1,
		1,
		16000,
		32000,
		2,
		16,
		{'d', 'a', 't', 'a'},
		0
	};
	
	UK2Node_PCMToWav(){};
	~UK2Node_PCMToWav(){};

	UFUNCTION(BlueprintCallable,meta = (WorldContext = "WorldContextObject"))
	static UK2Node_PCMToWav* ChangePCMToWav(UObject* WordContextObject,TArray<uint8> PCMData,FString WavPathToSave);

	TArray<uint8> m_PCMData;
	FString m_WavPathToSave;
	
	UPROPERTY(BlueprintAssignable)
	FBPPCMToWav_Result OnWavFileWriteComplete;
	
	virtual void Activate() override;
};
