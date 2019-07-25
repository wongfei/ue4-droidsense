#pragma once

#include "Modules/ModuleManager.h"

class DROIDSENSE_API IDroidSenseModule : public IModuleInterface
{
public:

	static inline IDroidSenseModule& Get()
	{
		return FModuleManager::LoadModuleChecked<IDroidSenseModule>("DroidSense");
	}

	static inline bool IsAvailable()
	{
		return FModuleManager::Get().IsModuleLoaded("DroidSense");
	}
};
