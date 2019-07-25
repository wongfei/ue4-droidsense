#include "DroidSenseDevice.h"

UDroidSenseDevice::UDroidSenseDevice(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer) {}

UDroidSenseSensor* UDroidSenseDevice::FindSensorById(int Id)
{
	for (auto* Sensor : Sensors)
	{
		if (Sensor->Id == Id)
		{
			return Sensor;
		}
	}
	return nullptr;
}

UDroidSenseSensor* UDroidSenseDevice::FindSensorByType(EDroidSenseSensorType Type)
{
	for (auto* Sensor : Sensors)
	{
		if (Sensor->Type == Type)
		{
			return Sensor;
		}
	}
	return nullptr;
}
