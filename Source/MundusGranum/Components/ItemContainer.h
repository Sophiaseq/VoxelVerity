// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "ItemContainer.generated.h"

class UMGItemDefinition;
// This class does not need to be modified.
UINTERFACE()
class UItemContainer : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class MUNDUSGRANUM_API IItemContainer
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Inventory")
	int32 AddItemToContainer(UMGItemDefinition* Item, int32 Count);
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Inventory")
	bool CanAcceptItem(UMGItemDefinition* Item) const;
};
