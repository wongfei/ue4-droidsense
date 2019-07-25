#include "DroidSenseModule.h"
#include "DroidSenseLog.h"

DEFINE_LOG_CATEGORY(LogDroidSense);

class FDroidSenseModule : public IDroidSenseModule
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};

void FDroidSenseModule::StartupModule()
{
	PLUGIN_LOG(Display, TEXT("## StartupModule ##"));
}

void FDroidSenseModule::ShutdownModule()
{
	PLUGIN_LOG(Display, TEXT("## ShutdownModule ##"));
}

IMPLEMENT_MODULE(FDroidSenseModule, DroidSense)
