// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Engine/GameInstance.h"

#include "MGGameInstance.generated.h"

UCLASS(Config = Game)
class MUNDUSGRANUM_API UMGGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	virtual void Init() override;
};
