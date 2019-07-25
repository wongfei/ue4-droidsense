#pragma once

#include "Components/SceneComponent.h"
#include "DroidSenseDevice.h"
#include "DroidSenseComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDroidSenseDeviceDetectedCallback, UDroidSenseDevice*, Device);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDroidSenseDevicePingedCallback, UDroidSenseDevice*, Device);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDroidSenseDeviceOpenedCallback, UDroidSenseDevice*, Device);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FDroidSenseSensorEnabledCallback, UDroidSenseDevice*, Device, UDroidSenseSensor*, Sensor, bool, bEnabled);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDroidSenseSensorEventCallback, const FDroidSenseSensorEvent&, Event);

UCLASS(ClassGroup="DroidSense", meta=(BlueprintSpawnableComponent))
class DROIDSENSE_API UDroidSenseComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	UDroidSenseComponent(const FObjectInitializer& ObjectInitializer);

	// UObject
	virtual void BeginDestroy() override;

	// UActorComponent
	virtual void InitializeComponent() override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void UninitializeComponent() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Requests
	UFUNCTION(Category = "DroidSense", BlueprintCallable)
	void DetectDevices();

	UFUNCTION(Category = "DroidSense", BlueprintCallable)
	void PingDevice(UDroidSenseDevice* Device);

	UFUNCTION(Category = "DroidSense", BlueprintCallable)
	void OpenDevice(UDroidSenseDevice* Device, const FString& Password);

	UFUNCTION(Category = "DroidSense", BlueprintCallable)
	void CloseDevice(UDroidSenseDevice* Device);

	UFUNCTION(Category = "DroidSense", BlueprintCallable)
	void CloseDevices();

	UFUNCTION(Category = "DroidSense", BlueprintCallable)
	void EnableSensor(UDroidSenseDevice* Device, UDroidSenseSensor* Sensor, bool Enabled, EDroidSenseSensorRate Rate = EDroidSenseSensorRate::NORMAL);

	UFUNCTION(Category = "DroidSense", BlueprintCallable)
	void EnableSensorByType(UDroidSenseDevice* Device, EDroidSenseSensorType Type, bool Enabled, EDroidSenseSensorRate Rate = EDroidSenseSensorRate::NORMAL);

	UFUNCTION(Category = "DroidSense", BlueprintCallable)
	void DisableAllSensors(UDroidSenseDevice* Device);

	// Events
	UPROPERTY(Category = "DroidSense", BlueprintAssignable)
	FDroidSenseDeviceDetectedCallback OnDeviceDetected;

	UPROPERTY(Category = "DroidSense", BlueprintAssignable)
	FDroidSenseDevicePingedCallback OnDevicePinged;

	UPROPERTY(Category = "DroidSense", BlueprintAssignable)
	FDroidSenseDeviceOpenedCallback OnDeviceOpened;

	UPROPERTY(Category = "DroidSense", BlueprintAssignable)
	FDroidSenseSensorEnabledCallback OnSensorEnabled;

	UPROPERTY(Category = "DroidSense", BlueprintAssignable)
	FDroidSenseSensorEventCallback OnSensorEvent;

	// Helpers
	UFUNCTION(Category = "DroidSense", BlueprintCallable, BlueprintPure)
	static FVector GetV3(const FDroidSenseSensorEvent& SensorEvent);

	UFUNCTION(Category = "DroidSense", BlueprintCallable, BlueprintPure)
	static FVector4 GetV4(const FDroidSenseSensorEvent& SensorEvent);

	// Vars
	UPROPERTY(Category = "DroidSense", BlueprintReadWrite, EditAnywhere)
	int32 Port = 9999;

	UPROPERTY(Category = "DroidSense", BlueprintReadWrite, EditAnywhere)
	bool bDropOlderPackets = true;

	UPROPERTY(Category = "DroidSense", BlueprintReadOnly, VisibleAnywhere)
	TArray<UDroidSenseDevice*> DetectedDevices;

	UPROPERTY(Category = "DroidSense", BlueprintReadOnly, VisibleAnywhere)
	TArray<UDroidSenseDevice*> OpenedDevices;

protected:

	virtual void RespDetectDevice(struct FResponse* Resp);
	virtual void RespPingDevice(struct FResponse* Resp);
	virtual void RespEnumerateSensors(struct FResponse* Resp);
	virtual void RespEnableSensor(struct FResponse* Resp);
	virtual void RespSensorEvent(struct FResponse* Resp);

	void InitSocket();
	void ShutdownSocket();
	void TickSocket();

	void SendPacket(const uint8* Data, int32 DataSize, const TCHAR* Address, int32 Port);
	UDroidSenseDevice* FindDeviceByAddr(TArray<UDroidSenseDevice*>& Devices, class FInternetAddr* Addr);
	bool Contains(TArray<UDroidSenseDevice*>& Devices, UDroidSenseDevice* Device);

	class ISocketSubsystem* SocketSub = nullptr;
	class FSocket* Socket = nullptr;
	TArray<uint8> SocketBuffer;
	TSharedPtr<FInternetAddr> RecvAddr;
	FDroidSenseSensorEvent SensorEvent;
	float PerfAccum = 0;
};
