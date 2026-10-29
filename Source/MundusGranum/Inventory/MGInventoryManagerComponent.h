// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "MGInventoryItemInstance.h"
#include "Components/ActorComponent.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "MGInventoryManagerComponent.generated.h"


class UMGInventoryManagerComponent;
struct FMGInventoryList;

USTRUCT(BlueprintType)
struct FMGInventoryChangeMessage
{
	GENERATED_BODY()

	//@TODO: Tag based names+owning actors for inventories instead of directly exposing the component?
	UPROPERTY(BlueprintReadOnly, Category=Inventory)
	TObjectPtr<UActorComponent> InventoryOwner = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = Inventory)
	TObjectPtr<UMGInventoryItemInstance> Instance = nullptr;

	UPROPERTY(BlueprintReadOnly, Category=Inventory)
	int32 NewCount = 0;

	UPROPERTY(BlueprintReadOnly, Category=Inventory)
	int32 Delta = 0;
};

USTRUCT(BlueprintType)
struct FMGInventoryEntry : public FFastArraySerializerItem
{
	GENERATED_BODY()
	
	FMGInventoryEntry(){};
	
private:
	void AddStackCount(int32 Count);
	
	friend FMGInventoryList;
	friend UMGInventoryManagerComponent;
	
	UPROPERTY()
	TObjectPtr<UMGInventoryItemInstance> Instance = nullptr;
	
	UPROPERTY()
	int32 StackCount = 0;
	
	UPROPERTY(NotReplicated)
	int32 LastObservedCount = INDEX_NONE;
};

USTRUCT(BlueprintType)
struct FMGInventoryList : public FFastArraySerializer
{
	GENERATED_BODY()

	FMGInventoryList()
		: OwnerComponent(nullptr)
	{
	}

	FMGInventoryList(UActorComponent* InOwnerComponent)
		: OwnerComponent(InOwnerComponent)
	{
	}

	TArray<UMGInventoryItemInstance*> GetAllItems() const;

public:
	//~FFastArraySerializer contract
	void PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize);
	void PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize);
	void PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize);
	//~End of FFastArraySerializer contract

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
	{
		return FFastArraySerializer::FastArrayDeltaSerialize<FMGInventoryEntry, FMGInventoryList>(Entries, DeltaParms, *this);
	}
	
	UMGInventoryItemInstance* AddEntry(TSubclassOf<UMGInventoryItemDefinition> ItemDef, int32& StackCount);
	void AddEntry(UMGInventoryItemInstance* Instance);
	void RemoveEntry(UMGInventoryItemInstance* Instance);

private:
	void BroadcastChangeMessage(FMGInventoryEntry& Entry, int32 OldCount, int32 NewCount);
	FMGInventoryEntry* FindStackableItemStackByDefinition(const TSubclassOf<UMGInventoryItemDefinition> ItemDef);
	bool HasReachedMaxEntryCount() const;
	
	friend UMGInventoryManagerComponent;

	//需要硬编码
	UPROPERTY(EditDefaultsOnly, NotReplicated)
	int8 MaxEntryCount = 32; 
	
	//网络复制
	UPROPERTY()
	TArray<FMGInventoryEntry> Entries;

	UPROPERTY(NotReplicated)
	TObjectPtr<UActorComponent> OwnerComponent;
};

template<>
struct TStructOpsTypeTraits<FMGInventoryList> : public TStructOpsTypeTraitsBase2<FMGInventoryList>
{
	enum { WithNetDeltaSerializer = true };
};

UCLASS(BlueprintType)
class MUNDUSGRANUM_API UMGInventoryManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMGInventoryManagerComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category=Inventory)
	bool CanAddItemDefinition(TSubclassOf<UMGInventoryItemDefinition> ItemDef, int32 StackCount = 1);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category=Inventory)
	UMGInventoryItemInstance* AddItemDefinition(TSubclassOf<UMGInventoryItemDefinition> ItemDef, int32 StackCount = 1);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category=Inventory)
	void AddItemInstance(UMGInventoryItemInstance* ItemInstance);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category=Inventory)
	void RemoveItemInstance(UMGInventoryItemInstance* ItemInstance);

	UFUNCTION(BlueprintCallable, Category=Inventory, BlueprintPure=false)
	TArray<UMGInventoryItemInstance*> GetAllItems() const;

	UFUNCTION(BlueprintCallable, Category=Inventory, BlueprintPure)
	UMGInventoryItemInstance* FindStackableItemStackByDefinition(TSubclassOf<UMGInventoryItemDefinition> ItemDef);
	
	UFUNCTION(BlueprintCallable, Category=Inventory, BlueprintPure)
	UMGInventoryItemInstance* FindFirstItemStackByDefinition(TSubclassOf<UMGInventoryItemDefinition> ItemDef) const;

	//统计这个ItemDef在Inventory中占了多少个槽位，不是统计单个Entry中的数量
	int32 GetTotalItemCountByDefinition(TSubclassOf<UMGInventoryItemDefinition> ItemDef) const;
	
	bool ConsumeItemsByDefinition(TSubclassOf<UMGInventoryItemDefinition> ItemDef, int32 NumToConsume);

	//~UObject interface
	virtual bool ReplicateSubobjects(class UActorChannel* Channel, class FOutBunch* Bunch, FReplicationFlags* RepFlags) override;
	virtual void ReadyForReplication() override;
	//~End of UObject interface

private:
	UPROPERTY(Replicated)
	FMGInventoryList InventoryList;
};
