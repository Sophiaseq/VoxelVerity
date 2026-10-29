// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "MGInventoryItemDefinition.h"
#include "System/GameplayTagStack.h"
#include "MGInventoryItemInstance.generated.h"

/**
 * 
 */
UCLASS()
class MUNDUSGRANUM_API UMGInventoryItemInstance : public UObject
{
	GENERATED_BODY()
	
public:	
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category=Inventory)
	void AddStatTagStack(FGameplayTag Tag, int32 StackCount);
	
	TSubclassOf<UMGInventoryItemDefinition> GetItemDef() const
	{
		return ItemDef;
	}

	
private:
	friend struct FMGInventoryList;
	
	void SetItemDef(const TSubclassOf<UMGInventoryItemDefinition> InDef);		
	
	UPROPERTY(Replicated)
	TSubclassOf<UMGInventoryItemDefinition> ItemDef;
	
	UPROPERTY(Replicated)
	FGameplayTagStackContainer StatTags;

};
