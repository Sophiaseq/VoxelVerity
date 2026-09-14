// Fill out your copyright notice in the Description page of Project Settings.


#include "MGEquipmentComponent.h"

#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Actor.h"
#include "Items/MGItemDefinition.h"
#include "MGInventoryComponent.h"
#include "MGLogChannels.h"

UMGEquipmentComponent::UMGEquipmentComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetIsReplicatedByDefault(true);
	bWantsInitializeComponent = true;
}

void UMGEquipmentComponent::BeginPlay()
{
	Super::BeginPlay();

	// 自动查找并绑定同 Actor 上的 InventoryComponent
	if (AActor* Owner = GetOwner())
	{
		if (UMGInventoryComponent* Inventory = Owner->FindComponentByClass<UMGInventoryComponent>())
		{
			BindToInventory(Inventory);
		}
	}
}

void UMGEquipmentComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (BoundInventory)
	{
		BoundInventory->OnSelectedSlotChanged.RemoveDynamic(this, &UMGEquipmentComponent::HandleSelectedItemChanged);
		BoundInventory->OnInventoryChanged.RemoveDynamic(this, &UMGEquipmentComponent::HandleInventoryChanged);
		BoundInventory = nullptr;
	}
	Super::EndPlay(EndPlayReason);
}

// ------------------------------------------------------------------ //
//  初始化
// ------------------------------------------------------------------ //

void UMGEquipmentComponent::SetupEquipMeshes(UStaticMeshComponent* InStaticMesh)
{
	EquippedStaticMeshComp = InStaticMesh;

	if (EquippedStaticMeshComp)
	{
		if (USkeletalMeshComponent* CharacterMesh = GetOwner()->FindComponentByClass<USkeletalMeshComponent>())
		{
			EquippedStaticMeshComp->AttachToComponent(
				CharacterMesh,
				FAttachmentTransformRules::SnapToTargetNotIncludingScale,
				HandSocketName);
		}
	}
}

void UMGEquipmentComponent::BindToInventory(UMGInventoryComponent* InInventory)
{
	if (BoundInventory == InInventory)
	{
		return;
	}

	// 解绑旧的
	if (BoundInventory)
	{
		BoundInventory->OnSelectedSlotChanged.RemoveDynamic(this, &UMGEquipmentComponent::HandleSelectedItemChanged);
		BoundInventory->OnInventoryChanged.RemoveDynamic(this, &UMGEquipmentComponent::HandleInventoryChanged);
	}

	BoundInventory = InInventory;

	if (BoundInventory)
	{
		BoundInventory->OnSelectedSlotChanged.AddDynamic(this, &UMGEquipmentComponent::HandleSelectedItemChanged);
		BoundInventory->OnInventoryChanged.AddDynamic(this, &UMGEquipmentComponent::HandleInventoryChanged);

		// 同步一次当前选中的物品，保证初始化状态一致
		UpdateEquippedItem(BoundInventory->GetSelectedItem());
	}
}

// ------------------------------------------------------------------ //
//  核心：更新手持显示
// ------------------------------------------------------------------ //

void UMGEquipmentComponent::HandleSelectedItemChanged(UMGItemDefinition* NewItem)
{
	UpdateEquippedItem(NewItem);
}

void UMGEquipmentComponent::HandleInventoryChanged(int32 SlotIndex)
{
	// 只有当前选中槽位内容变化才刷新手持显示
	if (BoundInventory && SlotIndex == BoundInventory->GetSelectedSlotIndex())
	{
		UpdateEquippedItem(BoundInventory->GetSelectedItem());
	}
}

void UMGEquipmentComponent::UpdateEquippedItem(UMGItemDefinition* NewItem)
{
	if (CurrentItem == NewItem)
	{
		return; // 没变化就跳过
	}

	// 1. 清理旧状态
	ClearEquippedDisplay();

	CurrentItem = NewItem;

	// 2. 空手：清理完毕直接返回
	if (!CurrentItem)
	{
		return;
	}

	// 3. 显示新物品
	const FEquipDisplayData& Display = CurrentItem->EquipDisplayData;

	if (Display.SkeletalMesh)
	{
		ShowSkeletalMesh(Display.SkeletalMesh, Display.EquippedTransform);
	}
	else if (Display.StaticMesh)
	{
		ShowStaticMesh(Display.StaticMesh, Display.EquippedTransform);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[Equipment] Item '%s' has no display mesh."), *CurrentItem->GetName());
	}
}

// ------------------------------------------------------------------ //
//  显示辅助
// ------------------------------------------------------------------ //

void UMGEquipmentComponent::ClearEquippedDisplay()
{
	// 不销毁组件，只清空网格并隐藏，便于复用
	if (EquippedMeshComp)
	{
		EquippedMeshComp->SetSkeletalMesh(nullptr);
		EquippedMeshComp->SetVisibility(false);
	}
	if (EquippedStaticMeshComp)
	{
		EquippedStaticMeshComp->SetStaticMesh(nullptr);
		EquippedStaticMeshComp->SetVisibility(false);
	}
}

void UMGEquipmentComponent::ShowSkeletalMesh(USkeletalMesh* Mesh, const FTransform& Transform)
{
	if (!Mesh)
	{
		return;
	}

	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	// 懒创建（仅第一次）
	if (!EquippedMeshComp)
	{
		EquippedMeshComp = NewObject<USkeletalMeshComponent>(GetOuter(), TEXT("EquippedSkeletalMesh"));
		EquippedMeshComp->SetupAttachment(GetOwner()->GetRootComponent(), HandSocketName);
		EquippedMeshComp->RegisterComponent();
		EquippedMeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		// 挂到手部插槽
		if (USkeletalMeshComponent* CharacterMesh = Owner->FindComponentByClass<USkeletalMeshComponent>())
		{
			EquippedMeshComp->AttachToComponent(
				CharacterMesh,
				FAttachmentTransformRules::SnapToTargetNotIncludingScale,
				HandSocketName);
			// 武器网格跟随角色动画
			EquippedMeshComp->SetLeaderPoseComponent(CharacterMesh);
		}
	}

	EquippedMeshComp->SetSkeletalMesh(Mesh);
	EquippedMeshComp->SetRelativeTransform(Transform);
	EquippedMeshComp->SetVisibility(true);

	if (EquippedStaticMeshComp)
	{
		EquippedStaticMeshComp->SetVisibility(false);
	}
}

void UMGEquipmentComponent::ShowStaticMesh(UStaticMesh* Mesh, const FTransform& Transform)
{
	if (!Mesh)
	{
		return;
	}

	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	// 懒创建（仅第一次，避免依赖外部 SetupEquipMeshes）
	if (!EquippedStaticMeshComp)
	{
		EquippedStaticMeshComp = NewObject<UStaticMeshComponent>(GetOuter(), TEXT("EquippedStaticMesh"));
		EquippedStaticMeshComp->SetupAttachment(Owner->GetRootComponent());
		EquippedStaticMeshComp->RegisterComponent();
		EquippedStaticMeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		// 挂到手部插槽
		if (USkeletalMeshComponent* CharacterMesh = Owner->FindComponentByClass<USkeletalMeshComponent>())
		{
			EquippedStaticMeshComp->AttachToComponent(
				CharacterMesh,
				FAttachmentTransformRules::SnapToTargetNotIncludingScale,
				HandSocketName);
		}
	}

	EquippedStaticMeshComp->SetStaticMesh(Mesh);
	EquippedStaticMeshComp->SetRelativeTransform(Transform);
	EquippedStaticMeshComp->SetVisibility(true);

	if (EquippedMeshComp)
	{
		EquippedMeshComp->SetVisibility(false);
	}
}

// ------------------------------------------------------------------ //
//  访问器
// ------------------------------------------------------------------ //

UMeshComponent* UMGEquipmentComponent::GetPriorityMeshComponent() const
{
	if (IsValid(EquippedMeshComp) && EquippedMeshComp->IsVisible())
	{
		return EquippedMeshComp;
	}
	if (IsValid(EquippedStaticMeshComp) && EquippedStaticMeshComp->IsVisible())
	{
		return EquippedStaticMeshComp;
	}
	return nullptr;
}

void UMGEquipmentComponent::InitializeComponent()
{
	Super::InitializeComponent();
	
	UE_LOG(LogMG, Log, TEXT("InitializeEquipmentComponent, Owner is [%s]"), *GetOwner()->GetName());
}

// ------------------------------------------------------------------ //
//  编辑器预览
// ------------------------------------------------------------------ //
#if WITH_EDITOR

void UMGEquipmentComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (!bAutoUpdatePreview)
	{
		return;
	}

	const FName PropName = PropertyChangedEvent.Property
		                       ? PropertyChangedEvent.Property->GetFName()
		                       : NAME_None;

	if (PropName == GET_MEMBER_NAME_CHECKED(UMGEquipmentComponent, PreviewItem))
	{
		UpdatePreviewFromDataAsset();
	}
}

void UMGEquipmentComponent::RefreshPreview()
{
	UpdatePreviewFromDataAsset();
}

void UMGEquipmentComponent::UpdatePreviewFromDataAsset()
{
	if (!PreviewItem)
	{
		if (PreviewWeaponMesh)
		{
			PreviewWeaponMesh->DestroyComponent();
		}
		return;
	}

	if (!PreviewWeaponMesh)
	{
		EquippedMeshComp = NewObject<USkeletalMeshComponent>(GetOuter(), TEXT("EquippedSkeletalMesh"));
		EquippedMeshComp->SetupAttachment(GetOwner()->GetRootComponent(), HandSocketName);
		EquippedMeshComp->RegisterComponent();
		EquippedMeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	PreviewWeaponMesh->SetStaticMesh(PreviewItem->EquipDisplayData.StaticMesh);
}

#endif // WITH_EDITOR
