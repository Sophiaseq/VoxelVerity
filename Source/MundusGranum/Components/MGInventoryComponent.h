// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ItemContainer.h"
#include "Components/PawnComponent.h"
#include "Delegates/Delegate.h"
#include "MGInventoryComponent.generated.h"


struct FMGInventorySlot;
class UMGItemDefinition;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGInventoryChanged, int32, SlotIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGSelectedSlotChanged, UMGItemDefinition*, NewItem);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class UMGInventoryComponent : public UPawnComponent, public IItemContainer
{
	GENERATED_BODY()
public:
	UMGInventoryComponent(const FObjectInitializer& ObjectInitializer);
	virtual void BeginPlay() override;
	
	void InitializeSlots();
	
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	FMGInventorySlot GetSlot(int32 Index) const;

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool SetSlot(int32 Index, const FMGInventorySlot& NewSlot);

	// ~Begin IItemContainer
	virtual int32 AddItemToContainer_Implementation(UMGItemDefinition* Item, int32 Count) override;
	virtual bool CanAcceptItem_Implementation(UMGItemDefinition* Item) const override;
	// ~End IItemContainer
	
	int32 AddItem(UMGItemDefinition* Item, int32 Count);
	int32 PickupItem(UMGItemDefinition* Item, int32 Count);
	bool RemoveItem(int32 Index, int32 Count);

	// 交换两个槽位
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void SwapSlots(int32 IndexA, int32 IndexB);

	// 快捷栏操作
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void SelectSlot(int32 NewIndex); // 循环取模

	// 客户端请求切换选中槽位（服务器权威，经复制广播给所有客户端）
	UFUNCTION(Server, Reliable)
	void ServerSelectSlot(int32 NewIndex);

	//~ Begin UActorComponent interface
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	//~ End UActorComponent interface

	int32 GetSelectedSlotIndex() const;
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory")
	FMGInventorySlot GetSelectedSlot() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory")
	UMGItemDefinition* GetSelectedItem() const;
	
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void UseSelectedItem(AActor* Target = nullptr);
	
	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FMGInventoryChanged OnInventoryChanged;
	
	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FMGSelectedSlotChanged OnSelectedSlotChanged;

protected:
	UFUNCTION()
	void OnRep_Slots();

	UFUNCTION()
	void OnRep_SelectedSlot();

	UPROPERTY(ReplicatedUsing = OnRep_Slots, BlueprintReadOnly, Category = "Inventory")
	TArray<FMGInventorySlot> Slots;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory")
	int32 SlotCount = 36;

	// 快捷栏起始索引
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory")
	int32 HotbarStartIndex = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory")
	int32 HotbarSize = 9;
	
	UPROPERTY(ReplicatedUsing = OnRep_SelectedSlot, BlueprintReadOnly, Category = "Inventory")
	int32 SelectedSlotIndex = 0;

};
