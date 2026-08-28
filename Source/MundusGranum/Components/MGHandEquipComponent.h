// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "Components/SceneComponent.h"
#include "Items/Weapons/MGWeaponItemDefinition.h"
#include "MGHandEquipComponent.generated.h"

class AMGItemBehavior;
class USkeletalMeshComponent;
class UStaticMeshComponent;
class UMGItemDefinition;

UCLASS(Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class UMGHandEquipComponent : public USceneComponent
{
	GENERATED_BODY()
public:
	UMGHandEquipComponent();
	virtual void BeginPlay() override;
	virtual void InitializeComponent() override;
	
	void SetupEquipMeshes(UStaticMeshComponent* InStatic);
	void CreateBoxComponentToTrace(const UMGWeaponItemDefinition* WeaponNewItem);
	UMeshComponent* GetPriorityMeshComponent() const;
	void SetupBoxTrace(USceneComponent* TraceStart, USceneComponent* TraceEnd);
	void UpdateBoxTraceBasedOnItem(const UMGWeaponItemDefinition* ItemDefinition) const;

	// 根据当前背包选中的物品更新手持模型（NewItem 为 nullptr 表示空手）
	UFUNCTION(BlueprintCallable)
	void UpdateEquippedItem(UMGItemDefinition* NewItem);
	
#if WITH_EDITOR
	UFUNCTION(CallInEditor, Category = "Editor Preview")
	void SaveTraceOffsetsToDataAsset() const;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	void UpdatePreviewFromDataAsset();
#endif

	UPROPERTY()
	TArray<AActor*> IgnoreActors;
	
protected:
	UFUNCTION(BlueprintCallable)
	void OnBoxOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
#if WITH_EDITORONLY_DATA
	UPROPERTY(Transient, EditDefaultsOnly, Category = "Editor Preview")
	TObjectPtr<UStaticMeshComponent> PreviewWeaponMesh;
	
	UPROPERTY(EditDefaultsOnly, Category = "Editor Preview")
	bool bAutoUpdatePreview = false;
#endif
	
	UPROPERTY(VisibleAnywhere, Category = HandedItem)
	TObjectPtr<AMGItemBehavior> CurrentBehavior;
	
	// 手持武器的骨骼网格（武器等）
	UPROPERTY(VisibleAnywhere, Category = HandedItem)
	TObjectPtr<USkeletalMeshComponent> EquippedMeshComp;

	// 手持普通物品的静态网格（工具/方块/消耗品等）
	UPROPERTY(VisibleAnywhere, Category = HandedItem)
	TObjectPtr<UStaticMeshComponent> EquippedStaticMeshComp;

	UPROPERTY(EditDefaultsOnly, Category = HandedItem)
	TObjectPtr<UMGItemDefinition> CurrentItem;
	
	UPROPERTY(VisibleAnywhere, Category = HandedItem)
	TObjectPtr<UBoxComponent> BoxComp;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = HandedItem)
	TObjectPtr<USceneComponent> BoxTraceStart;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = HandedItem)
	TObjectPtr<USceneComponent> BoxTraceEnd;

public:
	UFUNCTION(BlueprintCallable)
	FORCEINLINE USkeletalMeshComponent* GetEquippedMeshComp() const{return EquippedMeshComp;}
	FORCEINLINE [[nodiscard]] TObjectPtr<UMGItemDefinition> GetCurrentItem() const {return CurrentItem;}
	FORCEINLINE void SetCurrentItem(const TObjectPtr<UMGItemDefinition>& Item){CurrentItem = Item;}
	FORCEINLINE [[nodiscard]] TObjectPtr<UBoxComponent> GetBoxComp() const {return BoxComp;}
};
