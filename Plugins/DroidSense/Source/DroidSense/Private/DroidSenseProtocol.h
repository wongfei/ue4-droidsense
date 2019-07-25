#pragma once

#include <string>

enum EDroidSenseProtocol
{
	PK_REQ_DETECT_DEVICE = 0xA0,
	PK_RESP_DETECT_DEVICE = 0xA1,
	PK_REQ_PING_DEVICE = 0xA2,
	PK_RESP_PING_DEVICE = 0xA3,
	PK_REQ_ENUMERATE_SENSORS = 0xB0,
	PK_RESP_ENUMERATE_SENSORS = 0xB1,
	PK_REQ_ENABLE_SENSOR = 0xB2,
	PK_RESP_ENABLE_SENSOR = 0xB3,
	PK_REQ_DISABLE_ALL_SENSORS = 0xB4,
	PK_RESP_DISABLE_ALL_SENSORS = 0xB5,
	PK_CB_SENSOR_EVENT = 0xC0,
};

enum EAndroidSensorType {
	SENSOR_TYPE_ACCELEROMETER = 1,
	SENSOR_TYPE_ACCELEROMETER_UNCALIBRATED = 35,
	SENSOR_TYPE_AMBIENT_TEMPERATURE = 13,
	SENSOR_TYPE_GAME_ROTATION_VECTOR = 15,
	SENSOR_TYPE_GEOMAGNETIC_ROTATION_VECTOR = 20,
	SENSOR_TYPE_GRAVITY = 9,
	SENSOR_TYPE_GYROSCOPE = 4,
	SENSOR_TYPE_GYROSCOPE_UNCALIBRATED = 16,
	SENSOR_TYPE_HEART_BEAT = 31,
	SENSOR_TYPE_HEART_RATE = 21,
	SENSOR_TYPE_LIGHT = 5,
	SENSOR_TYPE_LINEAR_ACCELERATION = 10,
	SENSOR_TYPE_LOW_LATENCY_OFFBODY_DETECT = 34,
	SENSOR_TYPE_MAGNETIC_FIELD = 2,
	SENSOR_TYPE_MAGNETIC_FIELD_UNCALIBRATED = 14,
	SENSOR_TYPE_MOTION_DETECT = 30,
	SENSOR_TYPE_ORIENTATION = 3,
	SENSOR_TYPE_POSE_6DOF = 28,
	SENSOR_TYPE_PRESSURE = 6,
	SENSOR_TYPE_PROXIMITY = 8,
	SENSOR_TYPE_RELATIVE_HUMIDITY = 12,
	SENSOR_TYPE_ROTATION_VECTOR = 11,
	SENSOR_TYPE_SIGNIFICANT_MOTION = 17,
	SENSOR_TYPE_STATIONARY_DETECT = 29,
	SENSOR_TYPE_STEP_COUNTER = 19,
	SENSOR_TYPE_STEP_DETECTOR = 18,
	SENSOR_TYPE_TEMPERATURE = 7,
};

enum EAndroidSensorRate {
	SENSOR_DELAY_FASTEST = 0,
	SENSOR_DELAY_GAME = 1,
	SENSOR_DELAY_NORMAL = 3,
	SENSOR_DELAY_UI = 2,
};

inline EDroidSenseSensorType UE_SensorType(int Value)
{
	#define CASE_UE_SensorType(name) case SENSOR_TYPE_##name: return EDroidSenseSensorType::name;
	switch (Value)
	{
		CASE_UE_SensorType(ACCELEROMETER) // 1
		CASE_UE_SensorType(ACCELEROMETER_UNCALIBRATED) // 35
		CASE_UE_SensorType(AMBIENT_TEMPERATURE) // 13
		CASE_UE_SensorType(GAME_ROTATION_VECTOR) // 15
		CASE_UE_SensorType(GEOMAGNETIC_ROTATION_VECTOR) // 20
		CASE_UE_SensorType(GRAVITY) // 9
		CASE_UE_SensorType(GYROSCOPE) // 4
		CASE_UE_SensorType(GYROSCOPE_UNCALIBRATED) // 16
		CASE_UE_SensorType(HEART_BEAT) // 31
		CASE_UE_SensorType(HEART_RATE) // 21
		CASE_UE_SensorType(LIGHT) // 5
		CASE_UE_SensorType(LINEAR_ACCELERATION) // 10
		CASE_UE_SensorType(LOW_LATENCY_OFFBODY_DETECT) // 34
		CASE_UE_SensorType(MAGNETIC_FIELD) // 2
		CASE_UE_SensorType(MAGNETIC_FIELD_UNCALIBRATED) // 14
		CASE_UE_SensorType(MOTION_DETECT) // 30
		CASE_UE_SensorType(ORIENTATION) // 3
		CASE_UE_SensorType(POSE_6DOF) // 28
		CASE_UE_SensorType(PRESSURE) // 6
		CASE_UE_SensorType(PROXIMITY) // 8
		CASE_UE_SensorType(RELATIVE_HUMIDITY) // 12
		CASE_UE_SensorType(ROTATION_VECTOR) // 11
		CASE_UE_SensorType(SIGNIFICANT_MOTION) // 17
		CASE_UE_SensorType(STATIONARY_DETECT) // 29
		CASE_UE_SensorType(STEP_COUNTER) // 19
		CASE_UE_SensorType(STEP_DETECTOR) // 18
		CASE_UE_SensorType(TEMPERATURE) // 7
	}
	return EDroidSenseSensorType::UNKNOWN;
}

inline int Native_SensorRate(EDroidSenseSensorRate Value)
{
	#define CASE_Native_SensorRate(name) case EDroidSenseSensorRate::name: return SENSOR_DELAY_##name;
	switch (Value)
	{
		CASE_Native_SensorRate(FASTEST)
		CASE_Native_SensorRate(GAME)
		CASE_Native_SensorRate(NORMAL)
		CASE_Native_SensorRate(UI)
	}
	return SENSOR_DELAY_NORMAL;
}

inline uint16 SwapBytes(uint16 value)
{
	return ((value & 0xFFU) << 8 | (value & 0xFF00U) >> 8);
}

inline uint32 SwapBytes(uint32 value)
{
	return
		(value & 0x000000FFU) << 24 | (value & 0x0000FF00U) << 8 |
		(value & 0x00FF0000U) >> 8  | (value & 0xFF000000U) >> 24;
}

inline uint64 SwapBytes(uint64 value)
{
	return
		(value & 0x00000000000000FFUL) << 56 | (value & 0x000000000000FF00UL) << 40 |
		(value & 0x0000000000FF0000UL) << 24 | (value & 0x00000000FF000000UL) << 8  |
		(value & 0x000000FF00000000UL) >> 8  | (value & 0x0000FF0000000000UL) >> 24 |
		(value & 0x00FF000000000000UL) >> 40 | (value & 0xFF00000000000000UL) >> 56;
}

class FRawBuffer
{
public:
	inline FRawBuffer(void* data, size_t limit) : 
		data_((uint8*)data), 
		limit_(limit), 
		pos_(0) 
	{}

	inline FRawBuffer(TArray<uint8>& buf) : 
		data_(buf.GetData()), 
		limit_(buf.Max()), 
		pos_(0) 
	{}

	template<typename T>
	inline bool Read(T& elem) { 
		if (pos_ + sizeof(T) <= limit_) {
			elem = *(const T*)(data_ + pos_);
			pos_ += sizeof(T);
			return true;
		}
		return false;
	}

	template<typename T>
	inline bool Write(const T& elem) { 
		if (pos_ + sizeof(T) <= limit_) {
			*(T*)(data_ + pos_) = elem;
			pos_ += sizeof(T);
			return true;
		}
		return false;
	}

	inline bool ReadStr(FString& elem) { 
		uint8 n;
		if (Read(n)) {
			if (!n) {
				elem.Empty();
				return true;
			}
			if (pos_ + n <= limit_) {
				// FUTF8ToTCHAR appends junk character without seeing null terminator
				std::string str((const ANSICHAR*)(data_ + pos_), n);
				FUTF8ToTCHAR conv(str.c_str());
				elem = conv.Get();
				pos_ += n;
				return true;
			}
		}
		return false;
	}

	inline bool WriteStr(const FString& elem) { 
		FTCHARToUTF8 conv(*elem);
		const uint8 n = (uint8)FMath::Min(conv.Length(), 0xFF);
		if (pos_ + 1 + n <= limit_) {
			Write(n);
			if (n) {
				FMemory::Memcpy(data_ + pos_, conv.Get(), n);
				pos_ += n;
			}
			return true;
		}
		return false;
	}

	inline bool CanRead(size_t count) const { return (pos_ + count <= limit_); }
	inline uint8* GetData() { return data_; }
	inline size_t GetPos() const { return pos_; }

private:
	uint8* data_;
	size_t limit_;
	size_t pos_;
};

struct FResponse
{
	FInternetAddr* Sender;
	FRawBuffer* Raw;
	FResponse(TSharedRef<FInternetAddr>& addr, FRawBuffer& raw) : Sender(&addr.Get()), Raw(&raw) {}
	FResponse(TSharedPtr<FInternetAddr>& addr, FRawBuffer& raw) : Sender(addr.Get()), Raw(&raw) {}
};
