// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Inventory/MGInventoryItemDefinition.h"
#include "Templates/SubclassOf.h"

#include "InventoryFragment_EquippableItem.generated.h"

class UMGEquipmentDefinition;
class UObject;

UCLASS()
class UInventoryFragment_EquippableItem : public UMGInventoryItemFragment
{
	GENERATED_BODY()

public:
	/*UPROPERTY(EditAnywhere, Category=MG)
	TSubclassOf<UMGEquipmentDefinition> EquipmentDefinition;*/
};
