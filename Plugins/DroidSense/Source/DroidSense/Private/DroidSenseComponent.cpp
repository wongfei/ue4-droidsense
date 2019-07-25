#include "DroidSenseComponent.h"
#include "DroidSenseLog.h"

#include "Runtime/Sockets/Public/SocketSubsystem.h"
#include "Runtime/Sockets/Public/Sockets.h"
#include "Runtime/Sockets/Public/IPAddress.h"
#include "Runtime/Core/Public/Serialization/MemoryWriter.h"
#include "Runtime/Core/Public/Serialization/MemoryReader.h"

#include "DroidSenseProtocol.h"

DECLARE_STATS_GROUP(TEXT("DroidSense"), STATGROUP_DroidSense, STATCAT_Advanced);

DECLARE_CYCLE_STAT(TEXT("TickComponent"), STAT_DroidSense_TickComponent, STATGROUP_DroidSense);
DECLARE_CYCLE_STAT(TEXT("OnSensorEvent"), STAT_DroidSense_OnSensorEvent, STATGROUP_DroidSense);

DECLARE_DWORD_ACCUMULATOR_STAT(TEXT("SensorEventHz"), STAT_DroidSense_SensorEventHz, STATGROUP_DroidSense);

#define GUARD_PARAM_RET(Param) if (!(Param)) { PLUGIN_LOG(Error, TEXT("Invalid parameter: "#Param)); return; }

UDroidSenseComponent::UDroidSenseComponent(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	PLUGIN_LOG(Verbose, TEXT("+UDroidSenseComponent %p"), this);
	PrimaryComponentTick.bCanEverTick = true;
}

void UDroidSenseComponent::BeginDestroy()
{
	PLUGIN_LOG(Verbose, TEXT("~UDroidSenseComponent %p"), this);
	Super::BeginDestroy();
}

void UDroidSenseComponent::InitializeComponent()
{
	PLUGIN_LOG(Verbose, TEXT("InitializeComponent %p"), this);
	Super::InitializeComponent();
}

void UDroidSenseComponent::UninitializeComponent()
{
	PLUGIN_LOG(Verbose, TEXT("UninitializeComponent %p"), this);
	Super::UninitializeComponent();
}

void UDroidSenseComponent::BeginPlay()
{
	PLUGIN_LOG(Verbose, TEXT("BeginPlay %p"), this);
	InitSocket();
	Super::BeginPlay();
}

void UDroidSenseComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	PLUGIN_LOG(Verbose, TEXT("EndPlay %p"), this);
	Super::EndPlay(EndPlayReason);
	ShutdownSocket();
}

void UDroidSenseComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	SCOPE_CYCLE_COUNTER(STAT_DroidSense_TickComponent);

	PerfAccum += DeltaTime;
	if (PerfAccum >= 1.0f)
	{
		PerfAccum -= 1.0f;
		if (PerfAccum >= 1.0f) PerfAccum = 0;
		SET_DWORD_STAT(STAT_DroidSense_SensorEventHz, 0);
	}

	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	TickSocket();
}

void UDroidSenseComponent::InitSocket()
{
	PLUGIN_LOG(Display, TEXT("InitSocket"));

	SocketSub = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);
	if (SocketSub)
	{
		Socket = SocketSub->CreateSocket(NAME_DGram, TEXT("DroidSense"), true);
		if (Socket)
		{
			SocketBuffer.SetNumZeroed(1024);

			Socket->SetNonBlocking(true);
			Socket->SetBroadcast(true);
			Socket->SetReuseAddr(true);

			if (Socket->Bind(*SocketSub->CreateInternetAddr(0, Port)))
			{
				RecvAddr = SocketSub->CreateInternetAddr();
				return; // SUCCESS
			}
		}
	}

	PLUGIN_LOG(Error, TEXT("InitSocket failed"));
	ShutdownSocket();
}

void UDroidSenseComponent::ShutdownSocket()
{
	if (Socket)
	{
		CloseDevices();
		DetectedDevices.Empty();

		PLUGIN_LOG(Display, TEXT("ShutdownSocket"));
		SocketSub->DestroySocket(Socket);
		Socket = nullptr;
	}
}

void UDroidSenseComponent::TickSocket()
{
	uint32 PendingDataSize = 0;
	if (Socket && Socket->HasPendingData(PendingDataSize))
	{
		for (;;)
		{
			int32 BytesRead = 0;
			Socket->RecvFrom(SocketBuffer.GetData(), SocketBuffer.Max(), BytesRead, *RecvAddr, ESocketReceiveFlags::None);
			if (BytesRead <= 0)
			{
				break;
			}

			FRawBuffer Raw(SocketBuffer.GetData(), BytesRead);
			FResponse Resp(RecvAddr, Raw);

			uint8 PacketId;
			Raw.Read(PacketId);

			PLUGIN_LOG(VeryVerbose, TEXT("Packet Id=%d Size=%d"), (int)PacketId, (int)BytesRead);

			switch (PacketId)
			{
				case PK_RESP_DETECT_DEVICE:
					RespDetectDevice(&Resp);
					break;
				case PK_RESP_PING_DEVICE:
					RespPingDevice(&Resp);
					break;
				case PK_RESP_ENUMERATE_SENSORS:
					RespEnumerateSensors(&Resp);
					break;
				case PK_RESP_ENABLE_SENSOR:
					RespEnableSensor(&Resp);
					break;
				case PK_CB_SENSOR_EVENT:
					RespSensorEvent(&Resp);
					break;
			}
		}
	}
}

// PK_REQ_DETECT_DEVICE

void UDroidSenseComponent::DetectDevices()
{
	PLUGIN_LOG(Display, TEXT("DetectDevices Port=%d"), (int)Port);
	DetectedDevices.Empty();

	uint8 PacketId = PK_REQ_DETECT_DEVICE;
	SendPacket(&PacketId, sizeof(PacketId), nullptr, Port);
}

void UDroidSenseComponent::RespDetectDevice(FResponse* Resp)
{
	if (!FindDeviceByAddr(DetectedDevices, Resp->Sender))
	{
		auto* Device = NewObject<UDroidSenseDevice>();
		Device->Address = Resp->Sender->ToString(false);
		Device->Port = Resp->Sender->GetPort();

		PLUGIN_LOG(Display, TEXT("DEVICE Address=%s"), *Device->Address);
		DetectedDevices.Add(Device);

		if (OnDeviceDetected.IsBound())
		{
			OnDeviceDetected.Broadcast(Device);
		}
	}
}

// PK_REQ_PING_DEVICE

void UDroidSenseComponent::PingDevice(UDroidSenseDevice* Device)
{
	GUARD_PARAM_RET(Device);

	uint8 PacketId = PK_REQ_PING_DEVICE;
	SendPacket(&PacketId, sizeof(PacketId), *Device->Address, Device->Port);
}

void UDroidSenseComponent::RespPingDevice(FResponse* Resp)
{
	auto* Device = FindDeviceByAddr(DetectedDevices, Resp->Sender);
	if (Device)
	{
		if (OnDevicePinged.IsBound())
		{
			OnDevicePinged.Broadcast(Device);
		}
	}
}

// PK_REQ_ENUMERATE_SENSORS

void UDroidSenseComponent::OpenDevice(UDroidSenseDevice* Device, const FString& Password)
{
	GUARD_PARAM_RET(Device);

	PLUGIN_LOG(Display, TEXT("OpenDevice Address=%s"), *Device->Address);
	if (Contains(OpenedDevices, Device))
	{
		PLUGIN_LOG(Error, TEXT("Device already opened: Address=%s"), *Device->Address);
		return;
	}

	Device->Password = Password;
	uint8 PacketId = PK_REQ_ENUMERATE_SENSORS;
	SendPacket(&PacketId, sizeof(PacketId), *Device->Address, Device->Port);
}

void UDroidSenseComponent::RespEnumerateSensors(FResponse* Resp)
{
	auto* Device = FindDeviceByAddr(DetectedDevices, Resp->Sender);
	if (Device && !Contains(OpenedDevices, Device))
	{
		Device->Sensors.Empty();

		uint8 NumSensors;
		if (Resp->Raw->Read(NumSensors))
		{
			for (int i = 0; i < NumSensors; ++i)
			{
				if (Resp->Raw->CanRead(3)) // id, type, nameLen
				{
					uint8 SensorId, SensorType;
					Resp->Raw->Read(SensorId);
					Resp->Raw->Read(SensorType);

					FString SensorName;
					if (Resp->Raw->ReadStr(SensorName))
					{
						auto* Sensor = NewObject<UDroidSenseSensor>();
						Sensor->Id = SensorId;
						Sensor->Type = UE_SensorType(SensorType);
						Sensor->Name = SensorName;
						Sensor->LastTimestamp = 0;
						Device->Sensors.Add(Sensor);

						PLUGIN_LOG(Display, TEXT("SENSOR Address=%s Id=%d Type=%d Name=\"%s\""), *Device->Address, (int)SensorId, (int)SensorType, *SensorName);
					}
				}
			}
		}

		OpenedDevices.Add(Device);

		if (OnDeviceOpened.IsBound())
		{
			OnDeviceOpened.Broadcast(Device);
		}
	}
}

void UDroidSenseComponent::CloseDevice(UDroidSenseDevice* Device)
{
	GUARD_PARAM_RET(Device);
	if (Contains(OpenedDevices, Device))
	{
		PLUGIN_LOG(Display, TEXT("CloseDevice Address=%s"), *Device->Address);
		DisableAllSensors(Device);
		OpenedDevices.Remove(Device);
	}
}

void UDroidSenseComponent::CloseDevices()
{
	PLUGIN_LOG(Display, TEXT("CloseDevices"));
	for (auto* Device : OpenedDevices)
	{
		DisableAllSensors(Device);
	}
	OpenedDevices.Empty();
}

// PK_REQ_ENABLE_SENSOR

void UDroidSenseComponent::EnableSensor(UDroidSenseDevice* Device, UDroidSenseSensor* Sensor, bool Enabled, EDroidSenseSensorRate Rate)
{
	GUARD_PARAM_RET(Device);
	GUARD_PARAM_RET(Sensor);

	PLUGIN_LOG(Display, TEXT("EnableSensor Address=%s Id=%d Name=\"%s\""), *Device->Address, (int)Sensor->Id, *Sensor->Name);
	if (!Contains(OpenedDevices, Device))
	{
		PLUGIN_LOG(Error, TEXT("Device not opened: Address=%s"), *Device->Address);
		return;
	}

	FRawBuffer Raw(SocketBuffer);
	Raw.Write((uint8)PK_REQ_ENABLE_SENSOR);
	Raw.WriteStr(Device->Password);
	Raw.Write((uint8)Sensor->Id);
	Raw.Write((uint8)(Enabled ? 1 : 0));
	Raw.Write((uint8)Native_SensorRate(Rate));
	SendPacket(Raw.GetData(), Raw.GetPos(), *Device->Address, Device->Port);
}

void UDroidSenseComponent::EnableSensorByType(UDroidSenseDevice* Device, EDroidSenseSensorType Type, bool Enabled, EDroidSenseSensorRate Rate)
{
	GUARD_PARAM_RET(Device);

	auto* Sensor = Device->FindSensorByType(Type);
	if (Sensor)
	{
		EnableSensor(Device, Sensor, Enabled, Rate);
	}
	else
	{
		PLUGIN_LOG(Error, TEXT("Sensor not found: Type=%d"), (int)Type);
	}
}

void UDroidSenseComponent::RespEnableSensor(FResponse* Resp)
{
	auto* Device = FindDeviceByAddr(OpenedDevices, Resp->Sender);
	if (Device)
	{
		uint8 Status, SensorId;
		if (Resp->Raw->Read(Status) && Resp->Raw->Read(SensorId))
		{
			auto* Sensor = Device->FindSensorById(SensorId);
			if (Sensor)
			{
				if (OnSensorEnabled.IsBound())
				{
					OnSensorEnabled.Broadcast(Device, Sensor, (Status != 0));
				}
			}
		}
	}
}

// PK_REQ_DISABLE_ALL_SENSORS

void UDroidSenseComponent::DisableAllSensors(UDroidSenseDevice* Device)
{
	GUARD_PARAM_RET(Device);

	PLUGIN_LOG(Display, TEXT("DisableAllSensors Address=%s"), *Device->Address);
	if (!Contains(OpenedDevices, Device))
	{
		PLUGIN_LOG(Error, TEXT("Device not opened: Address=%s"), *Device->Address);
		return;
	}

	FRawBuffer Raw(SocketBuffer);
	Raw.Write((uint8)PK_REQ_DISABLE_ALL_SENSORS);
	Raw.WriteStr(Device->Password);
	SendPacket(Raw.GetData(), Raw.GetPos(), *Device->Address, Device->Port);
}

// PK_CB_SENSOR_EVENT

void UDroidSenseComponent::RespSensorEvent(FResponse* Resp)
{
	SCOPE_CYCLE_COUNTER(STAT_DroidSense_OnSensorEvent);

	auto* Device = FindDeviceByAddr(OpenedDevices, Resp->Sender);
	if (Device)
	{
		uint8 SensorId;
		if (Resp->Raw->Read(SensorId))
		{
			auto* Sensor = Device->FindSensorById(SensorId);
			if (Sensor)
			{
				uint64 Timestamp;
				if (Resp->Raw->Read(Timestamp))
				{
					Timestamp = SwapBytes(Timestamp);

					uint8 NumValues = 0;
					if (Resp->Raw->Read(NumValues) && Resp->Raw->CanRead(NumValues * sizeof(uint32)))
					{
						// packet validated
						INC_DWORD_STAT(STAT_DroidSense_SensorEventHz);

						if (!bDropOlderPackets || Sensor->LastTimestamp < Timestamp)
						{
							Sensor->LastTimestamp = Timestamp;

							SensorEvent.Device = Device;
							SensorEvent.Sensor = Sensor;
							SensorEvent.Timestamp = (float)((double)Timestamp * 1e-6); // nanosec to millisec
							SensorEvent.Values.SetNum(NumValues);

							for (int i = 0; i < NumValues; ++i)
							{
								uint32 ValU32;
								Resp->Raw->Read(ValU32);
								ValU32 = SwapBytes(ValU32);
								SensorEvent.Values[i] = *(float*)&ValU32;
							}

							if (OnSensorEvent.IsBound())
							{
								OnSensorEvent.Broadcast(SensorEvent);
							}
						}
					}
				}
			}
		}
	}
}

// Utils

void UDroidSenseComponent::SendPacket(const uint8* Data, int32 DataSize, const TCHAR* Address, int32 DstPort)
{
	if (!Socket) return;

	TSharedRef<FInternetAddr> Dst = SocketSub->CreateInternetAddr();
	if (Address)
	{
		bool bIsValid;
		Dst->SetIp(Address, bIsValid);
	}
	else
	{
		Dst->SetBroadcastAddress();
	}
	Dst->SetPort(DstPort);

	int32 BytesSent = 0;
	Socket->SendTo(Data, DataSize, BytesSent, *Dst);
}

UDroidSenseDevice* UDroidSenseComponent::FindDeviceByAddr(TArray<UDroidSenseDevice*>& Devices, FInternetAddr* Addr)
{
	FString AddrStr(Addr->ToString(false));
	for (auto* Iter : Devices)
	{
		if (Iter->Address.Equals(AddrStr))
		{
			return Iter;
		}
	}
	return nullptr;
}

bool UDroidSenseComponent::Contains(TArray<UDroidSenseDevice*>& Devices, UDroidSenseDevice* Device)
{
	for (auto* Iter : Devices)
	{
		if (Iter == Device || Iter->Address.Equals(Device->Address))
		{
			return true;
		}
	}
	return false;
}

FVector UDroidSenseComponent::GetV3(const FDroidSenseSensorEvent& SensorEvent)
{
	return FVector(SensorEvent.Values[0], SensorEvent.Values[1], SensorEvent.Values[2]);
}

FVector4 UDroidSenseComponent::GetV4(const FDroidSenseSensorEvent& SensorEvent)
{
	return FVector4(SensorEvent.Values[0], SensorEvent.Values[1], SensorEvent.Values[2], SensorEvent.Values[3]);
}
