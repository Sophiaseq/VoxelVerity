// Fill out your copyright notice in the Description page of Project Settings.


#include "MGEquipmentComponent.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Actor.h"
#include "Items/MGItemDefinition.h"
#include "MGInventoryComponent.h"
#include "MGLogChannels.h"
#include "Character/MGHealthComponent.h"

UMGEquipmentComponent::UMGEquipmentComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetIsReplicatedByDefault(true);
	bWantsInitializeComponent = true;
}

void UMGEquipmentComponent::HandleCharacterDeath(AActor* OwningActor)
{
	if (EquippedStaticMeshComp)
	{
		EquippedStaticMeshComp->DetachFromComponent(FDetachmentTransformRules(EDetachmentRule::KeepWorld, true));
		EquippedStaticMeshComp->SetSimulatePhysics(true);
		EquippedStaticMeshComp->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
	}
	else if (EquippedMeshComp)
	{
		EquippedMeshComp->DetachFromComponent(FDetachmentTransformRules(EDetachmentRule::KeepWorld, true));
		EquippedMeshComp->SetSimulatePhysics(true);
		EquippedMeshComp->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
	}
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
	
	if (AActor* Owner = GetOwner())
	{
		if (UMGHealthComponent* HealComp = Owner->FindComponentByClass<UMGHealthComponent>())
		{
			HealComp->OnDeathStarted.AddDynamic(this, &UMGEquipmentComponent::HandleCharacterDeath);
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
		return;
	}

	// 清理旧状态
	ClearEquippedDisplay();

	CurrentItem = NewItem;

	const IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(GetOwner());
	
	// 空手：清理完毕直接返回
	if (!CurrentItem)
	{
		ASI->GetAbilitySystemComponent()->RemoveLooseGameplayTags(EquippedTags, 1, EGameplayTagReplicationState::TagOnly);
		EquippedTags.Reset();
		return;
	}

	// 显示新物品
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
	
	ASI->GetAbilitySystemComponent()->RemoveLooseGameplayTags(EquippedTags, 1, EGameplayTagReplicationState::TagOnly);
	ASI->GetAbilitySystemComponent()->AddLooseGameplayTags(CurrentItem->ItemTags, 1, EGameplayTagReplicationState::TagOnly);
	EquippedTags = CurrentItem->ItemTags;
}

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
	
	if (!EquippedMeshComp)
	{
		EquippedMeshComp = NewObject<USkeletalMeshComponent>(GetOuter(), TEXT("EquippedSkeletalMesh"));
		EquippedMeshComp->SetupAttachment(GetOwner()->GetRootComponent(), HandSocketName);
		EquippedMeshComp->RegisterComponent();
		EquippedMeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		EquippedStaticMeshComp->ComponentTags.Add(FName("Component.Mesh.Weapon"));

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
		EquippedStaticMeshComp->ComponentTags.Add(FName("Component.Mesh.Weapon"));

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
