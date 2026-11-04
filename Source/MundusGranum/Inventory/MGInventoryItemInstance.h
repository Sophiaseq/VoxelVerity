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
	UMGInventoryItemInstance(const FObjectInitializer& Initializer = FObjectInitializer::Get());
	
	//~UObject interface
	virtual bool IsSupportedForNetworking() const override { return true; }
	//~End of UObject interface
	
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category=Inventory)
	void AddStatTagStack(FGameplayTag Tag, int32 StackCount);
	
	// Removes a specified number of stacks from the tag (does nothing if StackCount is below 1)
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category= Inventory)
	void RemoveStatTagStack(FGameplayTag Tag, int32 StackCount);

	// Returns the stack count of the specified tag (or 0 if the tag is not present)
	UFUNCTION(BlueprintCallable, Category=Inventory)
	int32 GetStatTagStackCount(FGameplayTag Tag) const;

	// Returns true if there is at least one stack of the specified tag
	UFUNCTION(BlueprintCallable, Category=Inventory)
	bool HasStatTag(FGameplayTag Tag) const;
	
	TSubclassOf<UMGInventoryItemDefinition> GetItemDef() const
	{
		return ItemDef;
	}

	UFUNCTION(BlueprintCallable, BlueprintPure=false, meta=(DeterminesOutputType=FragmentClass))
	const UMGInventoryItemFragment* FindFragmentByClass(TSubclassOf<UMGInventoryItemFragment> FragmentClass) const;

	template <typename ResultClass>
	const ResultClass* FindFragmentByClass() const
	{
		return (ResultClass*)FindFragmentByClass(ResultClass::StaticClass());
	}
	
private:
	friend struct FMGInventoryList;
	
	/** Register all replication fragments */
	virtual void RegisterReplicationFragments(UE::Net::FFragmentRegistrationContext& Context, UE::Net::EFragmentRegistrationFlags RegistrationFlags) override;
	
	void SetItemDef(const TSubclassOf<UMGInventoryItemDefinition> InDef);		
	
	UPROPERTY(Replicated)
	TSubclassOf<UMGInventoryItemDefinition> ItemDef;
	
	UPROPERTY(Replicated)
	FGameplayTagStackContainer StatTags;

};
