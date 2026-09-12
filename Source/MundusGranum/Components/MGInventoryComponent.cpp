// Fill out your copyright notice in the Description page of Project Settings.


#include "MGInventoryComponent.h"
#include "MGInventorySlot.h"
#include "Items/MGItemDefinition.h"

// Sets default values for this component's properties
UMGInventoryComponent::UMGInventoryComponent()
{
}

void UMGInventoryComponent::BeginPlay()
{
	Super::BeginPlay();
	InitializeSlots();

}

void UMGInventoryComponent::InitializeSlots()
{
	Slots.SetNum(SlotCount);
	for (auto& [Item, Count] : Slots)
	{
		Item = nullptr;
		Count = 0;
	}
}

FMGInventorySlot UMGInventoryComponent::GetSlot(int32 Index) const
{
	if (!Slots.IsValidIndex(Index))
	{
		UE_LOG(LogTemp, Warning, TEXT("GetSlot 索引越界 Index = %d"), Index);
		return FMGInventorySlot{};
	}

	return Slots[Index];
}

bool UMGInventoryComponent::SetSlot(int32 Index, const FMGInventorySlot& NewSlot)
{
	if (!Slots.IsValidIndex(Index))
	{
		UE_LOG(LogTemp, Warning, TEXT("SetSlot 索引越界 Index = %d"), Index);
		return false;
	}

	Slots[Index] = NewSlot;

	//背包数据发生变化，广播通知UI刷新和手持刷新
	OnInventoryChanged.Broadcast(Index);
	return true;
}

int32 UMGInventoryComponent::AddItem(UMGItemDefinition* Item, int32 Count)
{
	if (!Item || Count <= 0) return Count;

	// 先尝试堆叠到已有同类物品的槽位
	for (int32 i = 0; i < Slots.Num(); ++i)
	{
		FMGInventorySlot& Slot = Slots[i];
		if (!Slot.IsEmpty() && Slot.Item == Item)
		{
			int32 Space = Slot.GetRemainingSpace();
			int32 Add = FMath::Min(Count, Space);
			Slot.Count += Add;
			Count -= Add;
			OnInventoryChanged.Broadcast(i);
			if (Count == 0) return 0;
		}
	}

	// 放入空槽位
	for (int32 i = 0; i < Slots.Num(); ++i)
	{
		FMGInventorySlot& Slot = Slots[i];
		if (Slot.IsEmpty())
		{
			int32 Add = FMath::Min(Count, Item->BaseData.MaxStackSize);
			Slot.Item = Item;
			Slot.Count = Add;
			Count -= Add;
			OnInventoryChanged.Broadcast(i);
			if (Count == 0) return 0;
		}
	}
	return Count; // 返回未能放入的数量
}

int32 UMGInventoryComponent::PickupItem(UMGItemDefinition* Item, int32 Count)
{
	if (!Item || Count <= 0) return Count;

	const int32 SelectedIdx = GetSelectedSlotIndex();

	// 1. 优先当前选中槽（空或同类可堆叠），捡到即上手
	if (Slots.IsValidIndex(SelectedIdx))
	{
		FMGInventorySlot& SelectedSlot = Slots[SelectedIdx];
		if (SelectedSlot.IsEmpty() || SelectedSlot.Item == Item)
		{
			const int32 Space = SelectedSlot.IsEmpty() ? Item->BaseData.MaxStackSize : SelectedSlot.GetRemainingSpace();
			const int32 Add = FMath::Min(Count, Space);
			if (Add > 0)
			{
				SelectedSlot.Item = Item;
				SelectedSlot.Count += Add;
				Count -= Add;
				OnInventoryChanged.Broadcast(SelectedIdx); // 选中槽内容变化，角色据此刷新手持
				if (Count == 0) return 0;
			}
		}
	}

	// 2. 快捷栏空槽（不切选中）
	for (int32 i = HotbarStartIndex; i < HotbarStartIndex + HotbarSize; ++i)
	{
		if (!Slots.IsValidIndex(i)) continue;
		FMGInventorySlot& Slot = Slots[i];
		if (Slot.IsEmpty())
		{
			const int32 Add = FMath::Min(Count, Item->BaseData.MaxStackSize);
			Slot.Item = Item;
			Slot.Count = Add;
			Count -= Add;
			OnInventoryChanged.Broadcast(i);
			if (Count == 0) return 0;
		}
	}

	// 3. 剩余走通用 AddItem（堆叠任意同类 + 任意空槽）
	if (Count > 0)
	{
		return AddItem(Item, Count);
	}

	return 0;
}

bool UMGInventoryComponent::RemoveItem(int32 Index, int32 Count)
{
	// 参数合法性校验
	if(Count <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("RemoveItem:无效数量 Count=%d"), Count);
		return false;
	}

	// 索引越界检查
	if (!Slots.IsValidIndex(Index))
	{
		UE_LOG(LogTemp, Warning, TEXT("RemoveItem:索引越界 Index=%d"), Index);
		return false;
	}

	auto& Slot = Slots[Index];
	//结构化绑定拆解槽
	auto& [Item, SlotNum] = Slot;

	//槽本身是空
	if(Item == nullptr || SlotNum <= 0)
	{
		return false;
	}

	if(SlotNum > Count)
	{
		//物品还有剩余，仅扣减数量
		SlotNum -= Count;
	}
	else
	{
		//全部移除，清空槽位(保留格子，不删除数组元素！)
		Item = nullptr;
		SlotNum = 0;
	}

	//背包数据变更标记，用来触发UI刷新和手持刷新
	OnInventoryChanged.Broadcast(Index);
	return true;
}

void UMGInventoryComponent::SwapSlots(int32 IndexA, int32 IndexB)
{
}

void UMGInventoryComponent::SelectSlot(int32 NewIndex)
{
	if (HotbarSize == 0) return;
	// 确保索引在快捷栏范围内（循环）
	int32 NewSelected = (NewIndex % HotbarSize + HotbarSize) % HotbarSize;
	if (NewSelected == SelectedSlotIndex) return;
	SelectedSlotIndex = NewSelected;
	OnSelectedSlotChanged.Broadcast(Slots[SelectedSlotIndex].Item);
}

int32 UMGInventoryComponent::GetSelectedSlotIndex() const
{
	return HotbarStartIndex + SelectedSlotIndex;
}

FMGInventorySlot UMGInventoryComponent::GetSelectedSlot() const
{
	int32 SlotIdx = GetSelectedSlotIndex();
	if (Slots.IsValidIndex(SlotIdx))
		return Slots[SlotIdx];
	return FMGInventorySlot();
}

UMGItemDefinition* UMGInventoryComponent::GetSelectedItem() const
{
	return GetSelectedSlot().Item;
}

void UMGInventoryComponent::UseSelectedItem(AActor* Target)
{
	FMGInventorySlot Slot = GetSelectedSlot();
	if (Slot.IsEmpty()) return;

	// 在此调用物品的使用逻辑，例如触发一个接口，或者调用Target上的函数
	// 示例：如果物品是方块，则生成方块等
	// 使用后可以减少数量
	RemoveItem(GetSelectedSlotIndex(), 1);
}
