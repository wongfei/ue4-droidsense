#pragma once

#include "CoreMinimal.h"

DECLARE_LOG_CATEGORY_EXTERN(LogDroidSense, Log, All);

#define PLUGIN_LOG(Verbosity, Format, ...) UE_LOG(LogDroidSense, Verbosity, Format, __VA_ARGS__)
