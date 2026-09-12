// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Logging/LogMacros.h"

class UObject;

MUNDUSGRANUM_API DECLARE_LOG_CATEGORY_EXTERN(LogMG, Log, All);
MUNDUSGRANUM_API DECLARE_LOG_CATEGORY_EXTERN(LogMGExperience, Log, All);
MUNDUSGRANUM_API DECLARE_LOG_CATEGORY_EXTERN(LogMGAbilitySystem, Log, All);

MUNDUSGRANUM_API FString GetClientServerContextString(UObject* ContextObject = nullptr);
