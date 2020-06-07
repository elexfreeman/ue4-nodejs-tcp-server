// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include "Async/Async.h"
#include "Kismet/KismetSystemLibrary.h"
#include "SocketSubsystem.h"
#include "Sockets.h"
#include "Networking.h"

#include "Misc/FileHelper.h"
#include "IPAddress.h"
#include "HAL/ThreadSafeBool.h"

#include "NodeSocketCmp.generated.h"


// когда произошло подкл к серверу
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FNetEventS); 

// когда получили байты
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FNetMsgEvent, const TArray<uint8>&, Bytes);

// когда получили строку
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FNetMsgEventStr, const FString&, Msg);


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UBE_API UNodeSocketCmp : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UNodeSocketCmp();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	// наш сокет подключения
	FSocket * ClientSocket;

	// можно принимать данные безопасная для потока
	FThreadSafeBool bShouldReceiveData;

	// ссылка на поток
	TFuture<void> ClientConnectionFinishedFuture;

	// адрес сервера
	TSharedPtr<FInternetAddr> RemoteAdress;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
		

	// получили байты с сервера
	UPROPERTY(BlueprintReadWrite, BlueprintAssignable, BlueprintCallable)
	FNetMsgEvent OnReceivedBytes;

	// получили строку с сервера
	UPROPERTY(BlueprintReadWrite, BlueprintAssignable, BlueprintCallable)
	FNetMsgEventStr OnReceivedStr;

	// выполняется когда подключились к серверу
	FNetEventS OnConnected;


	/**
	 * Конвентирует байты в строки
	 */
	FString fBytesToString(const TArray<uint8>& InArray); 


	/**
	 * Конвентирует строки в байты
	 */
	TArray<uint8> fStringToBytes(FString InString);



	// имя соединения
	FString ClientSocketName;

	// размер буфера в байтах
	int32 BufferMaxSize;

	// признак что подключен
	bool bIsConnected;


	/**
	 * Подключается по TCP к серверу
	 *
	 * @param InIP ip4 адрес куда подключаемся
	 * @param InPort TCP порт подключения
	 */
	void ConnectToServer(const FString& InIP = TEXT("127.0.0.1"), const int32 InPort = 3000);

	/**
	 * Закрывает соединение
	 */
	void CloseSocket();

	/**
	 * Отправляет байты на сервер
	 *
	 * @param Message	Bytes
	 */
	UFUNCTION(BlueprintCallable, Category = "AA")
	bool Emit(const TArray<uint8>& Bytes);


	/**
	 * Отправляет строку на сервер
	 *
	 * @param Message	Bytes
	 */
	UFUNCTION(BlueprintCallable, Category = "AA")
	bool EmitStr(FString str);
};
