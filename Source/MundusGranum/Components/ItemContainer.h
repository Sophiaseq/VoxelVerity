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

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Inventory")
	int32 AddItemToContainer(UMGItemDefinition* Item, int32 Count);

	/** 是否还能接收至少 1 个该物品（可选，用于提示） */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Inventory")
	bool CanAcceptItem(UMGItemDefinition* Item) const;
};
