// Fill out your copyright notice in the Description page of Project Settings.


#include "..\..\Public\AudioClass\K2Node_PCMToWav.h"

UK2Node_PCMToWav* UK2Node_PCMToWav::ChangePCMToWav(UObject* WordContextObject, TArray<uint8> PCMData,FString WavPathToSave)
{
	UK2Node_PCMToWav* Ins = NewObject<UK2Node_PCMToWav>();
	Ins->m_PCMData = PCMData;
	Ins->m_WavPathToSave = WavPathToSave;
	return Ins;
}

void UK2Node_PCMToWav::Activate()
{
	Async(EAsyncExecution::ThreadPool,[&]()
	{
		wave_pcm_hdr wav_hdr = default_wav_hdr;
	
		int size = m_PCMData.Num();

		// 补充单字节
		// if(size%2 != 0)
		// {
		// 	m_PCMData.Add('A');
		// }

		// FString savePCMPath = FPaths::ProjectContentDir() + "Movies/CachedWav/0.pcm"; 
		
		// errno_t err;
		// err = fopen_s(&fp,TCHAR_TO_UTF8(*savePCMPath),"wb");
		// fwrite(m_PCMData.GetData(),m_PCMData.Num(),1,fp);
		// fclose(fp);
		
		int realsize = m_PCMData.Num();
		wav_hdr.size_8 = m_PCMData.Num()+36;
	
		wav_hdr.data_size = realsize;
	
		
		FString savePath = m_WavPathToSave;
		FILE* fp = NULL;
		errno_t err;
		err = fopen_s(&fp, TCHAR_TO_UTF8(*savePath), "wb");
		if (0 != err)
		{
			//报错
			UE_LOG(LogTemp,Warning,TEXT("Open File Error"))
			m_WavPathToSave = "";
			return;
		}

		fwrite(&wav_hdr, sizeof(wav_hdr), 1, fp);

		fseek(fp, 44, 0);
		fwrite(m_PCMData.GetData(), m_PCMData.Num(), 1, fp);

		fclose(fp);

		fp = NULL;

		OnWavFileWriteComplete.Broadcast(m_WavPathToSave);
	});
}
