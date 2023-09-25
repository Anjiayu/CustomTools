// Fill out your copyright notice in the Description page of Project Settings.


#include "NetWork/TCPClientComponent.h"

#include "Sockets.h"
#include "SocketSubsystem.h"

UTCPClientComponent::UTCPClientComponent(const FObjectInitializer &init) : UActorComponent(init)
{
	PrimaryComponentTick.bCanEverTick = true;
	bWantsInitializeComponent = true;
	bAutoActivate = true;
	ClientSocketName = FString(TEXT("TCPClientSocket"));
	ClientSocket = nullptr;

	BufferMaxSize = 2097152;	//default roughly 2Mb
}

void UTCPClientComponent::ConnectToServerAsClient(const FString& IP, const int32 Port, const int32 RateSecond)
{
	RemoteAdress = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();

	bool bIsValid;
	RemoteAdress->SetIp(*IP, bIsValid);
	RemoteAdress->SetPort(Port);

	if (!bIsValid)
	{
		UE_LOG(LogTemp, Error, TEXT("TCP address is invalid <%s:%d>"), *IP, Port);
		return;
	}

	ClientSocket = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateSocket(NAME_Stream, ClientSocketName, false);

	//Set Send Buffer Size
	ClientSocket->SetSendBufferSize(BufferMaxSize, BufferMaxSize);
	ClientSocket->SetReceiveBufferSize(BufferMaxSize, BufferMaxSize);

	if (ClientSocket->Connect(*RemoteAdress))
	{
		OnConnected.Broadcast(true, FString(TEXT("与服务器建立连接：成功")));
		bShouldReceiveData = true;
		ClientConnectionFinishedFuture = AsyncThread([this]()
			{
				//int32 RawDataLenLeftRecvNum = 4;//头4个字节表示的整数，表示该信息的总长度，包括头4个字节
				int32 RecvLen_CurMsg = 0;//当前消息已经收到的长度
				int32 TotalLen_CurMsg = 0;//当前消息的总长度
				static char Buf_CurMsg[2097156] = { 0 };//2Mb + 4

				uint32 PendingDataSize = 0;
				TArray<uint8> Buf_ThisRecv;
				while (bShouldReceiveData)
				{
					if (ClientSocket->HasPendingData(PendingDataSize))
					{
						Buf_ThisRecv.SetNumUninitialized(PendingDataSize);

						int32 TotalLen_ThisRecv = 0;
						ClientSocket->Recv(Buf_ThisRecv.GetData(), Buf_ThisRecv.Num(), TotalLen_ThisRecv);

						//UE_LOG(NetParticipant, Log, TEXT("+++++++++++TCP Recv TotalLen_ThisRecv = %d"), TotalLen_ThisRecv);
						int32 LeftLenNeedCopy_ThisRecv = TotalLen_ThisRecv;
						while (LeftLenNeedCopy_ThisRecv)
						{
							//UE_LOG(NetParticipant, Log, TEXT("**********TCP LeftLenNeedCopy_ThisRecv = %d ， TCP RecvLen_CurMsg = %d"), LeftLenNeedCopy_ThisRecv, RecvLen_CurMsg);
							if (RecvLen_CurMsg < 4)//< 4：当前消息尚未接收完表示消息长度的消息头
							{

								//UE_LOG(NetParticipant, Log, TEXT("**********解析消息头"));
								int32 NeedCopyLenForHeader = 4 - RecvLen_CurMsg;//还需几个字节，就可以收完消息头（消息头总计4个字节）
								//UE_LOG(NetParticipant, Log, TEXT("**********TCP NeedCopyLenForHeader = %d"), RecvLen_CurMsg);

								if (LeftLenNeedCopy_ThisRecv > NeedCopyLenForHeader)//当前收到的数据，填满消息头后，还有消息体
								{
									//UE_LOG(NetParticipant, Log, TEXT("**********当前收到的数据，填满消息头后，还有消息体"));
									//填满消息头
									FMemory::Memcpy(Buf_CurMsg + RecvLen_CurMsg, Buf_ThisRecv.GetData() + TotalLen_ThisRecv - LeftLenNeedCopy_ThisRecv, NeedCopyLenForHeader);
									//UE_LOG(NetParticipant, Log, TEXT("**********Memcpy Buf_CurMsg(%d) + RecvLen_CurMsg(%d), Buf_ThisRecv.GetData() + TotalLen_ThisRecv(%d) - LeftLenNeedCopy_ThisRecv(%d), NeedCopyLenForHeader(%d)"), Buf_CurMsg, RecvLen_CurMsg,TotalLen_ThisRecv,LeftLenNeedCopy_ThisRecv,NeedCopyLenForHeader);

									RecvLen_CurMsg = 4;
									LeftLenNeedCopy_ThisRecv -= NeedCopyLenForHeader;
									//UE_LOG(NetParticipant, Log, TEXT("**********LeftLenNeedCopy_ThisRecv(%d) -= NeedCopyLenForHeader(%d)"), LeftLenNeedCopy_ThisRecv,NeedCopyLenForHeader);

									FMemory::Memcpy(&TotalLen_CurMsg, Buf_CurMsg, 4);//转译出消息头，即消息的总长度
									//UE_LOG(NetParticipant, Log, TEXT("TCP Recv Msg's Len = %d"), TotalLen_CurMsg);

									if (LeftLenNeedCopy_ThisRecv < TotalLen_CurMsg - 4)//剩余数据不够填满当前消息，只需要追加存储消息体即可
									{
										FMemory::Memcpy(Buf_CurMsg + RecvLen_CurMsg, Buf_ThisRecv.GetData() + TotalLen_ThisRecv - LeftLenNeedCopy_ThisRecv, LeftLenNeedCopy_ThisRecv);
										//UE_LOG(NetParticipant, Log, TEXT("**********Memcpy Buf_CurMsg(%d) + RecvLen_CurMsg(%d), Buf_ThisRecv.GetData() + TotalLen_ThisRecv(%d) - LeftLenNeedCopy_ThisRecv(%d), LeftLenNeedCopy_ThisRecv(%d)"), Buf_CurMsg,RecvLen_CurMsg,TotalLen_ThisRecv,LeftLenNeedCopy_ThisRecv, LeftLenNeedCopy_ThisRecv);

										RecvLen_CurMsg += LeftLenNeedCopy_ThisRecv;

										//UE_LOG(NetParticipant, Log, TEXT("RecvLen_CurMsg(%d) += LeftLenNeedCopy_ThisRecv(%d)"), RecvLen_CurMsg,LeftLenNeedCopy_ThisRecv);
										LeftLenNeedCopy_ThisRecv = 0;
										//UE_LOG(NetParticipant, Log, TEXT("LeftLenNeedCopy_ThisRecv = 0;"));
									}
									else//剩余数据可以填满当前消息，甚至还可能有剩余
									{
										//先填满当前消息
										FMemory::Memcpy(Buf_CurMsg + RecvLen_CurMsg, Buf_ThisRecv.GetData() + TotalLen_ThisRecv - LeftLenNeedCopy_ThisRecv, TotalLen_CurMsg - 4);
										//UE_LOG(NetParticipant, Log, TEXT("**********Memcpy(Buf_CurMsg(%d) + RecvLen_CurMsg(%d), Buf_ThisRecv.GetData() + TotalLen_ThisRecv(%d) - LeftLenNeedCopy_ThisRecv(%d), TotalLen_CurMsg(%d) - 4)"), Buf_CurMsg,RecvLen_CurMsg,TotalLen_ThisRecv,LeftLenNeedCopy_ThisRecv, TotalLen_CurMsg);
										Buf_CurMsg[TotalLen_CurMsg] = 0;
										
										FString tryToString(UTF8_TO_TCHAR(Buf_CurMsg + 4));
										MsgReceived.Enqueue(MoveTemp(tryToString));
										//UE_LOG(NetParticipant, Log, TEXT("当前消息结束"));
										RecvLen_CurMsg = 0;
										LeftLenNeedCopy_ThisRecv -= (TotalLen_CurMsg - 4);
										//UE_LOG(NetParticipant, Log, TEXT("**********LeftLenNeedCopy_ThisRecv(%d) -= (TotalLen_CurMsg(%d) - 4)"), LeftLenNeedCopy_ThisRecv,TotalLen_CurMsg);
									}
								}
								else//当前收到的消息，最多填满消息头，没有消息体，只需要追加存储即可
								{
									FMemory::Memcpy(Buf_CurMsg + RecvLen_CurMsg, Buf_ThisRecv.GetData() + TotalLen_ThisRecv - LeftLenNeedCopy_ThisRecv, LeftLenNeedCopy_ThisRecv);
									//UE_LOG(NetParticipant, Log, TEXT("**********Memcpy(Buf_CurMsg(%d) + RecvLen_CurMsg(%d), Buf_ThisRecv.GetData() + TotalLen_ThisRecv(%d) - LeftLenNeedCopy_ThisRecv(%d), LeftLenNeedCopy_ThisRecv(%d))"), Buf_CurMsg,RecvLen_CurMsg,TotalLen_ThisRecv, LeftLenNeedCopy_ThisRecv,LeftLenNeedCopy_ThisRecv);
									RecvLen_CurMsg += LeftLenNeedCopy_ThisRecv;
									//UE_LOG(NetParticipant, Log, TEXT("RecvLen_CurMsg(%d) += LeftLenNeedCopy_ThisRecv(%d)"), RecvLen_CurMsg , LeftLenNeedCopy_ThisRecv);
									LeftLenNeedCopy_ThisRecv = 0;

									if (4 == RecvLen_CurMsg)//如果正好填满消息头，则读取到当前消息的长度
									{
										FMemory::Memcpy(&TotalLen_CurMsg, Buf_CurMsg, 4);//转译出消息头，即消息的总长度
										//UE_LOG(NetParticipant, Log, TEXT("TCP Recv Msg's Len = %d"), TotalLen_CurMsg);
									}
								}
							}
							else//准备填充消息体
							{
								if (LeftLenNeedCopy_ThisRecv < TotalLen_CurMsg - RecvLen_CurMsg)//剩余数据不够填满当前消息，只需要追加存储消息体即可
								{
									FMemory::Memcpy(Buf_CurMsg + RecvLen_CurMsg, Buf_ThisRecv.GetData() + TotalLen_ThisRecv - LeftLenNeedCopy_ThisRecv, LeftLenNeedCopy_ThisRecv);
									//UE_LOG(NetParticipant, Log, TEXT("Memcpy(Buf_CurMsg(%d) + RecvLen_CurMsg(%d), Buf_ThisRecv.GetData() + TotalLen_ThisRecv(%d) - LeftLenNeedCopy_ThisRecv(%d), LeftLenNeedCopy_ThisRecv(%d))"),Buf_CurMsg,RecvLen_CurMsg,TotalLen_ThisRecv,LeftLenNeedCopy_ThisRecv, LeftLenNeedCopy_ThisRecv);
									RecvLen_CurMsg += LeftLenNeedCopy_ThisRecv;
									//UE_LOG(NetParticipant, Log, TEXT("RecvLen_CurMsg(%d) += LeftLenNeedCopy_ThisRecv(%d)"), RecvLen_CurMsg , LeftLenNeedCopy_ThisRecv);
									LeftLenNeedCopy_ThisRecv = 0;
								}
								else//剩余数据可以填满当前消息，甚至还可能有剩余
								{
									//先填满当前消息
									FMemory::Memcpy(Buf_CurMsg + RecvLen_CurMsg, Buf_ThisRecv.GetData() + TotalLen_ThisRecv - LeftLenNeedCopy_ThisRecv, TotalLen_CurMsg - RecvLen_CurMsg);
									//UE_LOG(NetParticipant, Log, TEXT("Memcpy(Buf_CurMsg(%d) + RecvLen_CurMsg(%d), Buf_ThisRecv.GetData() + TotalLen_ThisRecv(%d) - LeftLenNeedCopy_ThisRecv(%d), TotalLen_CurMsg(%d) - RecvLen_CurMsg(%d))"), Buf_CurMsg,RecvLen_CurMsg,TotalLen_ThisRecv,LeftLenNeedCopy_ThisRecv, TotalLen_CurMsg, RecvLen_CurMsg);
									Buf_CurMsg[TotalLen_CurMsg] = 0;

									FString tryToString(UTF8_TO_TCHAR(Buf_CurMsg + 4));
									MsgReceived.Enqueue(MoveTemp(tryToString));
									//UE_LOG(NetParticipant, Log, TEXT("当前消息完成"));
									
									LeftLenNeedCopy_ThisRecv -= (TotalLen_CurMsg - RecvLen_CurMsg);
									RecvLen_CurMsg = 0;

									//UE_LOG(NetParticipant, Log, TEXT("LeftLenNeedCopy_ThisRecv(%d) -= (TotalLen_CurMsg(%d) - RecvLen_CurMsg(%d))"), LeftLenNeedCopy_ThisRecv, TotalLen_CurMsg,RecvLen_CurMsg);
								}
							}
						}

						if (!MsgReceived.IsEmpty())
						{
							//Pass the reference to be used on game thread
							AsyncTask(ENamedThreads::GameThread, [this]()
								{
									if (!IsComponentTickEnabled())
									{
										SetComponentTickEnabled(true);
									}
								});
						}
					}
					//sleep until there is data or 10 ticks
					ClientSocket->Wait(ESocketWaitConditions::WaitForReadOrWrite, FTimespan(1));
				}
			});
	}
	else
	{
		OnConnected.Broadcast(false, FString(TEXT("与服务器建立连接：失败")));
	}
}

void UTCPClientComponent::CloseSocket()
{
	if (ClientSocket)
	{
		bShouldReceiveData = false;
		if (SCS_Connected == ClientSocket->GetConnectionState())
		{
			ClientSocket->Close();
		}
		if (ClientConnectionFinishedFuture.IsValid())
		{
			ClientConnectionFinishedFuture.Get();
		}

		ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(ClientSocket);
		ClientSocket = nullptr;
	}
}


bool UTCPClientComponent::Send(const FString& Text)
{
	static uint8 TmpSend[2097152] = { 0 };//2Mb

	bool bRst = false;
	if (ClientSocket && ClientSocket->GetConnectionState() == SCS_Connected)
	{
		int32 TotalBytesLen = std::strlen(TCHAR_TO_UTF8(*Text)) + 4;

		FMemory::Memcpy(TmpSend, &TotalBytesLen, 4);//转译出消息头，即消息的总长度

		FMemory::Memcpy(TmpSend + 4, TCHAR_TO_UTF8(*Text), TotalBytesLen - 4);//转译出消息头，即消息的总长度

		int32 TotalBytesSent = 0;
		do 
		{
			int32 BytesSent = 0;
			if (!ClientSocket->Send(TmpSend + TotalBytesSent, TotalBytesLen - TotalBytesSent, BytesSent))
			{
				break;
			}

			TotalBytesSent += BytesSent;

		} while (TotalBytesSent < TotalBytesLen);

		bRst = (TotalBytesSent >= TotalBytesLen);
	}

	if (!bRst)
	{
		OnConnected.Broadcast(false, Text);
	}

	return bRst;
}

void UTCPClientComponent::InitializeComponent()
{
	Super::InitializeComponent();
}

void UTCPClientComponent::UninitializeComponent()
{
	Super::UninitializeComponent();
}

void UTCPClientComponent::BeginPlay()
{
	Super::BeginPlay();

}


void UTCPClientComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (MsgReceived.IsEmpty())
	{
		SetComponentTickEnabled(false);
	}
	else
	{
		FString Text;
		MsgReceived.Dequeue(Text);

		OnReceived.Broadcast(Text);
	}

	if (ClientSocket && ClientSocket->GetConnectionState() == SCS_Connected)
	{

	}
	else
	{

	}
}

void UTCPClientComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	CloseSocket();

	Super::EndPlay(EndPlayReason);
}