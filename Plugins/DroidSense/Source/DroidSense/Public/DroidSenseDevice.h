#pragma once

#include "DroidSenseSensor.h"
#include "DroidSenseDevice.generated.h"

UCLASS(ClassGroup="DroidSense", BlueprintType)
class DROIDSENSE_API UDroidSenseDevice : public UObject
{
	GENERATED_BODY()

public:

	UDroidSenseDevice(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(Category="DroidSense", BlueprintReadOnly, VisibleAnywhere)
	FString Address;

	UPROPERTY(Category="DroidSense", BlueprintReadOnly, VisibleAnywhere)
	int32 Port;

	UPROPERTY(Category="DroidSense", BlueprintReadOnly, VisibleAnywhere)
	FString Password;

	UPROPERTY(Category="DroidSense", BlueprintReadOnly, VisibleAnywhere)
	TArray<class UDroidSenseSensor*> Sensors;

	UFUNCTION(Category = "DroidSense", BlueprintCallable)
	UDroidSenseSensor* FindSensorById(int Id);

	UFUNCTION(Category = "DroidSense", BlueprintCallable)
	UDroidSenseSensor* FindSensorByType(EDroidSenseSensorType Type);
};
