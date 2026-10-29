// Fill out your copyright notice in the Description page of Project Settings.

#include "MGInventoryManagerComponent.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "NativeGameplayTags.h"
#include "MGInventoryFragment_Stackable.h"
#include "Net/UnrealNetwork.h"
#include "Tests/ToolMenusTestUtilities.h"


#include UE_INLINE_GENERATED_CPP_BY_NAME(MGInventoryManagerComponent)

UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_MG_Inventory_Message_StackChanged, "MG.Inventory.Message.StackChanged");

void FMGInventoryEntry::AddStackCount(int32 Count)
{
	StackCount += Count;
}

TArray<UMGInventoryItemInstance*> FMGInventoryList::GetAllItems() const
{
	TArray<UMGInventoryItemInstance*> Results;
	Results.Reserve(Entries.Num());
	for (const FMGInventoryEntry& Entry : Entries)
	{
		if (Entry.Instance != nullptr) //@TODO: 最好让Entry中不存在为nullptr的Instance
		{
			Results.Add(Entry.Instance);
		}
	}
	return Results;
}

void FMGInventoryList::PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize)
{
	for (int32 Index : RemovedIndices)
	{
		FMGInventoryEntry& Stack = Entries[Index];
		BroadcastChangeMessage(Stack, /*OldCount=*/ Stack.StackCount, /*NewCount=*/ 0);
		Stack.LastObservedCount = 0;
	}
}

void FMGInventoryList::PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize)
{
	for (int32 Index : AddedIndices)
	{
		FMGInventoryEntry& Stack = Entries[Index];
		BroadcastChangeMessage(Stack, /*OldCount=*/ 0, /*NewCount=*/ Stack.StackCount);
		Stack.LastObservedCount = Stack.StackCount;
	}
}

void FMGInventoryList::PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize)
{
	for (int32 Index : ChangedIndices)
	{
		FMGInventoryEntry& Stack = Entries[Index];
		check(Stack.LastObservedCount != INDEX_NONE);
		BroadcastChangeMessage(Stack, /*OldCount=*/ Stack.LastObservedCount, /*NewCount=*/ Stack.StackCount);
		Stack.LastObservedCount = Stack.StackCount;
	}
}

FMGInventoryEntry* FMGInventoryList::FindStackableItemStackByDefinition(
	const TSubclassOf<UMGInventoryItemDefinition> ItemDef)
{
	const UMGInventoryItemFragment* Fragment = GetDefault<UMGInventoryItemDefinition>(ItemDef)->FindFragmentByClass(UMGInventoryFragment_Stackable::StaticClass());
	const UMGInventoryFragment_Stackable* StackableItem = Cast<UMGInventoryFragment_Stackable>(Fragment);
	for (FMGInventoryEntry& Entry : Entries)
	{
		UMGInventoryItemInstance* Instance = Entry.Instance;
		if (Instance->GetItemDef() == ItemDef)
		{
			if (StackableItem->CanStackWith(Entry.StackCount))
			{
				return &Entry;
			}
		}
	}
	return nullptr;
}

UMGInventoryItemInstance* FMGInventoryList::AddEntry(TSubclassOf<UMGInventoryItemDefinition> ItemDef,
                                                     int32& StackCount)
{
	UMGInventoryItemInstance* Result = nullptr;

	check(ItemDef != nullptr);
	check(OwnerComponent);

	AActor* OwningActor = OwnerComponent->GetOwner();
	check(OwningActor->HasAuthority());

	const UMGInventoryItemDefinition* ItemCDO = GetDefault<UMGInventoryItemDefinition>(ItemDef);
	const UMGInventoryItemFragment* ItemFragment = ItemCDO->FindFragmentByClass(UMGInventoryFragment_Stackable::StaticClass());
	const UMGInventoryFragment_Stackable* ItemStack = Cast<UMGInventoryFragment_Stackable>(ItemFragment);
	
	auto NewEntryToStack = [this, &Result](const UMGInventoryFragment_Stackable* ItemStack, TSubclassOf<UMGInventoryItemDefinition> ItemDef, int32& Count)
	{
		for (int32 i = 0; i < Count/ItemStack->GetMaxStackCount()+1; i++)
		{
			if (!HasReachedMaxEntryCount())
			{
				FMGInventoryEntry& NewEntry = Entries.AddDefaulted_GetRef();
				NewEntry.Instance = NewObject<UMGInventoryItemInstance>(OwnerComponent->GetOwner());
				NewEntry.Instance->SetItemDef(ItemDef);
				for (UMGInventoryItemFragment* Fragment : GetDefault<UMGInventoryItemDefinition>(ItemDef)->Fragments)
				{
					if (Fragment != nullptr)
					{
						Fragment->OnInstanceCreated(NewEntry.Instance);
					}
				}
				NewEntry.StackCount = Count > ItemStack->GetMaxStackCount() ? ItemStack->GetMaxStackCount() : Count;
				Result = NewEntry.Instance;
				MarkItemDirty(NewEntry);
				Count -= ItemStack->GetMaxStackCount();
			}
		}
	};

	FMGInventoryEntry* Entry = FindStackableItemStackByDefinition(ItemDef);
	if (Entry->Instance != nullptr && Entry->StackCount + StackCount <= ItemStack->GetMaxStackCount())
	{
		Entry->AddStackCount(StackCount);
	}
	else if (Entry->Instance == nullptr)
	{
		NewEntryToStack(ItemStack, ItemDef, StackCount);
	}
	else
	{
		Entry->AddStackCount(ItemStack->GetMaxStackCount() - Entry->StackCount);
		StackCount -= ItemStack->GetMaxStackCount() - Entry->StackCount;
		NewEntryToStack(ItemStack, ItemDef, StackCount);
	}
	//const UMGInventoryItemDefinition* ItemCDO = GetDefault<UMGInventoryItemDefinition>(ItemDef);

	return Result;
}

bool FMGInventoryList::HasReachedMaxEntryCount() const
{
	if (Entries.Num() == MaxEntryCount) return true;
	return false;
}

void FMGInventoryList::AddEntry(UMGInventoryItemInstance* Instance)
{
	unimplemented();
}

void FMGInventoryList::RemoveEntry(UMGInventoryItemInstance* Instance)
{
	for (auto EntryIt = Entries.CreateIterator(); EntryIt; ++EntryIt)
	{
		FMGInventoryEntry& Entry = *EntryIt;
		if (Entry.Instance == Instance)
		{
			EntryIt.RemoveCurrent();
			MarkArrayDirty();
		}
	}
}

void FMGInventoryList::BroadcastChangeMessage(FMGInventoryEntry& Entry, int32 OldCount, int32 NewCount)
{
	FMGInventoryChangeMessage Message;
	Message.InventoryOwner = OwnerComponent;
	Message.Instance = Entry.Instance;
	Message.NewCount = NewCount;
	Message.Delta = NewCount - OldCount;

	UGameplayMessageSubsystem& MessageSystem = UGameplayMessageSubsystem::Get(OwnerComponent->GetWorld());
	MessageSystem.BroadcastMessage(TAG_MG_Inventory_Message_StackChanged, Message);
}

UMGInventoryManagerComponent::UMGInventoryManagerComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, InventoryList(this)
{
	SetIsReplicatedByDefault(true);
}

bool UMGInventoryManagerComponent::CanAddItemDefinition(TSubclassOf<UMGInventoryItemDefinition> ItemDef,
	int32 StackCount)
{
	const UMGInventoryItemFragment* Fragment = GetDefault<UMGInventoryItemDefinition>(ItemDef)->FindFragmentByClass(UMGInventoryFragment_Stackable::StaticClass());
	if (ItemDef == nullptr || Fragment == nullptr) return false;
	if (InventoryList.HasReachedMaxEntryCount())
	{
		if (!FindStackableItemStackByDefinition(ItemDef))
		{
			return false;
		}
	}
	
	return true;
}

UMGInventoryItemInstance* UMGInventoryManagerComponent::AddItemDefinition(
	TSubclassOf<UMGInventoryItemDefinition> ItemDef, int32 StackCount)
{
	UMGInventoryItemInstance* Result = nullptr;
	if (CanAddItemDefinition(ItemDef))
	{
		Result = InventoryList.AddEntry(ItemDef, StackCount);
		
		if (IsUsingRegisteredSubObjectList() && IsReadyForReplication() && Result)
		{
			AddReplicatedSubObject(Result);
		}
	}
	return Result;
}

void UMGInventoryManagerComponent::AddItemInstance(UMGInventoryItemInstance* ItemInstance)
{
	InventoryList.AddEntry(ItemInstance);
	if (IsUsingRegisteredSubObjectList() && IsReadyForReplication() && ItemInstance)
	{
		AddReplicatedSubObject(ItemInstance);
	}
}

void UMGInventoryManagerComponent::RemoveItemInstance(UMGInventoryItemInstance* ItemInstance)
{
	InventoryList.RemoveEntry(ItemInstance);

	if (ItemInstance && IsUsingRegisteredSubObjectList())
	{
		RemoveReplicatedSubObject(ItemInstance);
	}
}

TArray<UMGInventoryItemInstance*> UMGInventoryManagerComponent::GetAllItems() const
{
	return InventoryList.GetAllItems();
}

UMGInventoryItemInstance* UMGInventoryManagerComponent::FindStackableItemStackByDefinition(
	TSubclassOf<UMGInventoryItemDefinition> ItemDef)
{
	if (const FMGInventoryEntry* Entry = InventoryList.FindStackableItemStackByDefinition(ItemDef))
	{
		return Entry->Instance;
	}
	return nullptr;
}

UMGInventoryItemInstance* UMGInventoryManagerComponent::FindFirstItemStackByDefinition(
	TSubclassOf<UMGInventoryItemDefinition> ItemDef) const
{
	for (const FMGInventoryEntry& Entry : InventoryList.Entries)
	{
		UMGInventoryItemInstance* Instance = Entry.Instance;

		if (IsValid(Instance))
		{
			if (Instance->GetItemDef() == ItemDef)
			{
				return Instance;
			}
		}
	}

	return nullptr;
}

int32 UMGInventoryManagerComponent::GetTotalItemCountByDefinition(TSubclassOf<UMGInventoryItemDefinition> ItemDef) const
{
	int32 TotalCount = 0;
	for (const FMGInventoryEntry& Entry : InventoryList.Entries)
	{
		const UMGInventoryItemInstance* Instance = Entry.Instance;

		if (IsValid(Instance))
		{
			if (Instance->GetItemDef() == ItemDef)
			{
				++TotalCount;
			}
		}
	}

	return TotalCount;
}

bool UMGInventoryManagerComponent::ConsumeItemsByDefinition(TSubclassOf<UMGInventoryItemDefinition> ItemDef,
	int32 NumToConsume)
{
	AActor* OwningActor = GetOwner();
	if (!OwningActor || !OwningActor->HasAuthority())
	{
		return false;
	}

	//@TODO: N squared right now as there's no acceleration structure
	int32 TotalConsumed = 0;
	while (TotalConsumed < NumToConsume)
	{
		if (UMGInventoryItemInstance* Instance = UMGInventoryManagerComponent::FindFirstItemStackByDefinition(ItemDef))
		{
			InventoryList.RemoveEntry(Instance);
			++TotalConsumed;
		}
		else
		{
			return false;
		}
	}

	return TotalConsumed == NumToConsume;
}

bool UMGInventoryManagerComponent::ReplicateSubobjects(class UActorChannel* Channel, class FOutBunch* Bunch,
	FReplicationFlags* RepFlags)
{
	return Super::ReplicateSubobjects(Channel, Bunch, RepFlags);
}

void UMGInventoryManagerComponent::ReadyForReplication()
{
	Super::ReadyForReplication();
}

void UMGInventoryManagerComponent::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, InventoryList);
}


