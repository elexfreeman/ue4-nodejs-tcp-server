// Fill out your copyright notice in the Description page of Project Settings.


#include "NodeSocketCmp.h"

// Sets default values for this component's properties
UNodeSocketCmp::UNodeSocketCmp()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...

	// устанавливаем имя скета
	ClientSocketName = FString(TEXT("ue4-tcp-client"));

	// обнуляем сокет
	ClientSocket = nullptr;

	// максимальный размер буфера
	BufferMaxSize = 2 * 1024 * 1024;	// 2 Mb
}


// Called when the game starts
void UNodeSocketCmp::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
	// подключамся к серверу
	this->ConnectToServer(TEXT("127.0.0.1"), 3007);

}


// Called every frame
void UNodeSocketCmp::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}


void UNodeSocketCmp::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, TEXT("End Play"));

	// отключаемся от сервера
	CloseSocket();
}





/**
 * Подключается к серверу
 * 
 */
void UNodeSocketCmp::ConnectToServer(const FString& InIP, const int32 InPort)
{

	// формируем адрес подключения
	RemoteAdress = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();

	UE_LOG(LogTemp, Error, TEXT("TCP address try to connect <%s:%d>"), *InIP, InPort);

	bool bIsValid;
	RemoteAdress->SetIp(*InIP, bIsValid);
	RemoteAdress->SetPort(InPort);

	// чекаем валидность адреса подключения
	if (!bIsValid)
	{
		UE_LOG(LogTemp, Error, TEXT("TCP address is invalid <%s:%d>"), *InIP, InPort);
		return;
	}

	// Получаем подсистему сокетов
	ClientSocket = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateSocket(NAME_Stream, ClientSocketName, false);

	// Устанавливаем размер буфера отправки/приема
	ClientSocket->SetSendBufferSize(BufferMaxSize, BufferMaxSize);
	ClientSocket->SetReceiveBufferSize(BufferMaxSize, BufferMaxSize);

	// конектимся
	bIsConnected = ClientSocket->Connect(*RemoteAdress);

	// если подключились бродкастим эвент
	if (bIsConnected)
	{
		OnConnected.Broadcast();
	}

	// говорим что готовы получать данные
	bShouldReceiveData = true;

	// Слушатель данных
	ClientConnectionFinishedFuture = Async(EAsyncExecution::Thread, [&]()
		{
			uint32 BufferSize = 0;
			TArray<uint8> ReceiveBuffer;
			FString ResultString;

			// запускаем бесконечный цикл получения данных
			while (bShouldReceiveData)
			{
				// если есть данные
				if (ClientSocket->HasPendingData(BufferSize))
				{
					// устанавливаем размер буфера
					ReceiveBuffer.SetNumUninitialized(BufferSize);

					int32 Read = 0;
					ClientSocket->Recv(ReceiveBuffer.GetData(), ReceiveBuffer.Num(), Read);

					// отправляем буфер в эвент
					OnReceivedBytes.Broadcast(ReceiveBuffer);
					
					// отправляем 
					OnReceivedStr.Broadcast(fBytesToString(ReceiveBuffer));

					// логируем сообщение
					UE_LOG(LogTemp, Log, TEXT("Reserve data: %s"), *(fBytesToString(ReceiveBuffer)));

				}
				// пропускаем 1 тик
				ClientSocket->Wait(ESocketWaitConditions::WaitForReadOrWrite, FTimespan(1));
			}
		}
	);
}

/**
 * Закрывает сокет
 * */
void UNodeSocketCmp::CloseSocket()
{
	// если есть сокет
	if (ClientSocket)
	{
		// прекращаем получть данные
		bShouldReceiveData = false;

		// завершаем соединение
		ClientConnectionFinishedFuture.Get();
		ClientSocket->Close();
		ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(ClientSocket);
		ClientSocket = nullptr;
	}
}

/**
 * Оправляет байты на сервер
 * */
bool UNodeSocketCmp::Emit(const TArray<uint8>& Bytes)
{
	bool resp = false;

	// проверяем есть ли подлючение
	if (ClientSocket && ClientSocket->GetConnectionState() == SCS_Connected)
	{
		int32 BytesSent = 0; // сколько байт отправленно
		resp = ClientSocket->Send(Bytes.GetData(), Bytes.Num(), BytesSent);
	}
	return resp;
}

/**
 * Оправляет байты на сервер
 * */
bool UNodeSocketCmp::EmitStr(FString str)
{
	bool resp = false;

	// проверяем есть ли подлючение
	if (ClientSocket && ClientSocket->GetConnectionState() == SCS_Connected)
	{
		int32 BytesSent = 0; // сколько байт отправленно
		// конвертим байты
		TArray<uint8> Bytes = fStringToBytes(str);
		resp =  ClientSocket->Send(Bytes.GetData(), Bytes.Num(), BytesSent);
	}
	return resp;
}


/**
* Конвентирует байты в строки
*/
FString UNodeSocketCmp::fBytesToString(const TArray<uint8>& InArray)
{
	FString ResultString;
	FFileHelper::BufferToString(ResultString, InArray.GetData(), InArray.Num());
	return ResultString;
}


/**
* Конвентирует строки в байты
*/
TArray<uint8> UNodeSocketCmp::fStringToBytes(FString InString)
{
	TArray<uint8> ResultBytes;
	ResultBytes.Append((uint8*)TCHAR_TO_UTF8(*InString), InString.Len());
	return ResultBytes;
}
