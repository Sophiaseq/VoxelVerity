// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/PawnComponent.h"
#include "MGEquipmentComponent.generated.h"

class UMGInventoryComponent;
class UMGItemDefinition;
class USkeletalMeshComponent;
class UStaticMeshComponent;

/*
 *
 */
UCLASS(BlueprintType, ClassGroup=(Custom), Const)
class UMGEquipmentComponent : public UPawnComponent
{
	GENERATED_BODY()

public:
	UMGEquipmentComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** 由角色初始化时调用，传入预创建的静态网格组件 */
	UFUNCTION(BlueprintCallable)
	void SetupEquipMeshes(UStaticMeshComponent* InStaticMesh);

	/** 绑定到 InventoryComponent */
	UFUNCTION(BlueprintCallable)
	void BindToInventory(UMGInventoryComponent* InInventory);

	/** 手动更新手持物品（一般由 Inventory 事件驱动） */
	UFUNCTION(BlueprintCallable)
	void UpdateEquippedItem(UMGItemDefinition* NewItem);

	// ========== 对外访问器 ==========
	UFUNCTION(BlueprintPure)
	USkeletalMeshComponent* GetEquippedMeshComp() const { return EquippedMeshComp; }

	UFUNCTION(BlueprintPure)
	UMGItemDefinition* GetCurrentItem() const { return CurrentItem; }

	/** 返回当前优先使用的网格组件（骨骼优先，其次静态） */
	UFUNCTION(BlueprintPure)
	UMeshComponent* GetPriorityMeshComponent() const;

protected:
	/** Inventory 选中物品变化回调 */
	UFUNCTION()
	void HandleSelectedItemChanged(UMGItemDefinition* NewItem);

	// ========== 内部辅助 ==========
	void ClearEquippedDisplay();
	void ShowSkeletalMesh(USkeletalMesh* Mesh, const FTransform& Transform);
	void ShowStaticMesh(UStaticMesh* Mesh, const FTransform& Transform);

private:
	// ========== 显示组件（懒创建，复用）==========
	UPROPERTY(Transient)
	TObjectPtr<USkeletalMeshComponent> EquippedMeshComp;

	UPROPERTY(Transient)
	TObjectPtr<UStaticMeshComponent> EquippedStaticMeshComp;

	// ========== 当前状态 ==========
	UPROPERTY(Transient)
	TObjectPtr<UMGItemDefinition> CurrentItem;

	UPROPERTY(Transient)
	TObjectPtr<UMGInventoryComponent> BoundInventory;

	// ========== 配置 ==========
	/** 手部插槽名称（通常是角色骨骼上的 HandGrip_R / HandGrip_L） */
	UPROPERTY(EditDefaultsOnly, Category = "Equipment")
	FName HandSocketName = TEXT("HandGrip_R");

#if WITH_EDITORONLY_DATA
	// ========== 编辑器预览 ==========
	UPROPERTY(Transient)
	TObjectPtr<UStaticMeshComponent> PreviewWeaponMesh;

	UPROPERTY(EditDefaultsOnly, Category = "Editor Preview")
	bool bAutoUpdatePreview = false;

	UPROPERTY(EditDefaultsOnly, Category = "Editor Preview")
	TObjectPtr<UMGItemDefinition> PreviewItem;
#endif

#if WITH_EDITOR

public:
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;

	UFUNCTION(CallInEditor, Category = "Editor Preview")
	void RefreshPreview();

private:
	void UpdatePreviewFromDataAsset();
#endif
};
