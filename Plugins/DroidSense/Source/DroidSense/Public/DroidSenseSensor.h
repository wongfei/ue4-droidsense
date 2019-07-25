#pragma once

#include "DroidSenseSensor.generated.h"

UENUM(Blueprintable)
enum class EDroidSenseSensorType : uint8
{
	UNKNOWN,
	ACCELEROMETER,
	ACCELEROMETER_UNCALIBRATED,
	AMBIENT_TEMPERATURE,
	GAME_ROTATION_VECTOR,
	GEOMAGNETIC_ROTATION_VECTOR,
	GRAVITY,
	GYROSCOPE,
	GYROSCOPE_UNCALIBRATED,
	HEART_BEAT,
	HEART_RATE,
	LIGHT,
	LINEAR_ACCELERATION,
	LOW_LATENCY_OFFBODY_DETECT,
	MAGNETIC_FIELD,
	MAGNETIC_FIELD_UNCALIBRATED,
	MOTION_DETECT,
	ORIENTATION,
	POSE_6DOF,
	PRESSURE,
	PROXIMITY,
	RELATIVE_HUMIDITY,
	ROTATION_VECTOR,
	SIGNIFICANT_MOTION,
	STATIONARY_DETECT,
	STEP_COUNTER,
	STEP_DETECTOR,
	TEMPERATURE,
};

UENUM(Blueprintable)
enum class EDroidSenseSensorRate : uint8
{
	FASTEST,
    GAME,
    NORMAL,
    UI,
};

UCLASS(ClassGroup="DroidSense", BlueprintType)
class DROIDSENSE_API UDroidSenseSensor : public UObject
{
	GENERATED_BODY()

public:

	UDroidSenseSensor(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(Category="DroidSense", BlueprintReadOnly, VisibleAnywhere)
	int32 Id;
	
	UPROPERTY(Category="DroidSense", BlueprintReadOnly, VisibleAnywhere)
	EDroidSenseSensorType Type;

	UPROPERTY(Category="DroidSense", BlueprintReadOnly, VisibleAnywhere)
	FString Name;

	uint64 LastTimestamp = 0;
};

USTRUCT(BlueprintType)
struct DROIDSENSE_API FDroidSenseSensorEvent
{
	GENERATED_BODY()

	UPROPERTY(Category="DroidSense", BlueprintReadOnly, VisibleAnywhere)
	class UDroidSenseDevice* Device;

	UPROPERTY(Category="DroidSense", BlueprintReadOnly, VisibleAnywhere)
	class UDroidSenseSensor* Sensor;

	UPROPERTY(Category="DroidSense", BlueprintReadOnly, VisibleAnywhere)
	float Timestamp;

	UPROPERTY(Category="DroidSense", BlueprintReadOnly, VisibleAnywhere)
	TArray<float> Values;
};
