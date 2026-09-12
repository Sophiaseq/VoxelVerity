// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Delegates/Delegate.h"
#include "MGInventoryComponent.generated.h"


struct FMGInventorySlot;
class UMGItemDefinition;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGInventoryChanged, int32, SlotIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGSelectedSlotChanged, UMGItemDefinition*, NewItem);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class UMGInventoryComponent : public UActorComponent
{
	GENERATED_BODY()
public:
	UMGInventoryComponent();
	virtual void BeginPlay() override;
	
	// 初始化所有槽位为空
	void InitializeSlots();

	// 获取/设置槽位
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	FMGInventorySlot GetSlot(int32 Index) const;

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool SetSlot(int32 Index, const FMGInventorySlot& NewSlot);

	// 添加物品（自动堆叠，返回剩余数量）
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	int32 AddItem(UMGItemDefinition* Item, int32 Count);

	// 拾取落位：优先当前选中槽，其次快捷栏空槽，最后走通用 AddItem（返回剩余数量）
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	int32 PickupItem(UMGItemDefinition* Item, int32 Count);

	// 移除指定槽位中的物品
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool RemoveItem(int32 Index, int32 Count);

	// 交换两个槽位
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void SwapSlots(int32 IndexA, int32 IndexB);

	// 快捷栏操作
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void SelectSlot(int32 NewIndex); // 循环取模

	// 获取当前选中槽的实际索引（HotbarStartIndex + SelectedSlotIndex）
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory")
	int32 GetSelectedSlotIndex() const;

	// 获取当前主手物品（当前选中的快捷栏槽位）
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory")
	FMGInventorySlot GetSelectedSlot() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory")
	UMGItemDefinition* GetSelectedItem() const;

	// 使用当前主手物品（触发使用逻辑）
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void UseSelectedItem(AActor* Target = nullptr);

	// 某个槽位内容发生变化时广播（参数为槽位索引）
	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FMGInventoryChanged OnInventoryChanged;

	// 选中槽位切换时广播（参数为新的快捷栏索引）
	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FMGSelectedSlotChanged OnSelectedSlotChanged;

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Inventory")
	TArray<FMGInventorySlot> Slots;

	// 总槽位数（例如 36）
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory")
	int32 SlotCount = 36;

	// 快捷栏起始索引（假设前9格为快捷栏）
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory")
	int32 HotbarStartIndex = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory")
	int32 HotbarSize = 9;

	// 当前选中的快捷栏槽位索引（0~HotbarSize-1）
	UPROPERTY(BlueprintReadOnly, Category = "Inventory")
	int32 SelectedSlotIndex = 0;
};
