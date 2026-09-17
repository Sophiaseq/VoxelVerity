#pragma once
#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "MGItemDefinition.generated.h"

class UStaticMesh;
class USkeletalMesh;
class UAnimInstance;
class AMGItemBehavior;

USTRUCT(BlueprintType)
struct FItemBaseData
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly)
	FText ItemName;

	UPROPERTY(EditDefaultsOnly)
	int32 MaxStackSize = 64;

	UPROPERTY(EditDefaultsOnly)
	TSoftObjectPtr<UTexture2D> InventoryIcon;
};

// 手持时的装备显示数据：静态网格（工具/方块/消耗品）或骨骼网格（武器），按需二选一填写
USTRUCT(BlueprintType)
struct FEquipDisplayData
{
	GENERATED_BODY()

	// 手持时的静态网格
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UStaticMesh> StaticMesh;

	// 手持时的骨骼网格
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<USkeletalMesh> SkeletalMesh;

	// 手持偏移
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FTransform EquippedTransform = FTransform::Identity;

	// 该物品专用的动画蓝图（暂不使用）
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UAnimInstance> EquipAnimClass;
};

UCLASS(BlueprintType)
class MUNDUSGRANUM_API UMGItemDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<AMGItemBehavior> ItemBehavior;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FItemBaseData BaseData;

	// 丢在地上时显示的 3D 网格体（静态模型，如石头、木头）
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UStaticMesh> DropMesh;
	
	// 手持时的显示数据（静态或骨骼网格 + 偏移 + 动画类）
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FEquipDisplayData EquipDisplayData;

	// 放置到世界（如果Category是Block）——不是网格体，而是Actor类（因为方块可能有交互逻辑）
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(EditConditionExpression="ItemTags.HasTag(ItemCategory.Block)"))
	TSubclassOf<AActor> PlacedActorClass;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tags")
	FGameplayTagContainer ItemTags;
	
	UFUNCTION(BlueprintCallable, BlueprintPure)
	bool HasTag(FGameplayTag Tag) const
	{
		return ItemTags.HasTag(Tag);
	}
};