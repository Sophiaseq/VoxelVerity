// Copyright Epic Games, Inc. All Rights Reserved.

#include "MGEquipmentDefinition.h"
#include "MGEquipmentInstance.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(MGEquipmentDefinition)

UMGEquipmentDefinition::UMGEquipmentDefinition(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	InstanceType = UMGEquipmentInstance::StaticClass();
}

