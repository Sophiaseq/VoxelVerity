// Fill out your copyright notice in the Description page of Project Settings.


#include "MGHandEquipComponent.h"

#include "Items/MGItemDefinition.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Interaction/HitInterface.h"
#include "Items/Weapons/MGWeaponItemDefinition.h"
#include "Kismet/KismetSystemLibrary.h"


UMGHandEquipComponent::UMGHandEquipComponent()
{
}

void UMGHandEquipComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UMGHandEquipComponent::InitializeComponent()
{
	Super::InitializeComponent();
	EquippedStaticMeshComp = NewObject<UStaticMeshComponent>(GetOwner(),TEXT("ItemMesh"));
	EquippedStaticMeshComp->RegisterComponent();
	EquippedStaticMeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	EquippedStaticMeshComp->SetVisibility(false);
	EquippedStaticMeshComp->AttachToComponent(this,FAttachmentTransformRules::SnapToTargetNotIncludingScale);

	BoxTraceStart = NewObject<USceneComponent>(GetOwner(),TEXT("BoxTraceStart"));
	BoxTraceStart->RegisterComponent();
	BoxTraceStart->AttachToComponent(this,FAttachmentTransformRules::SnapToTargetNotIncludingScale);

	BoxTraceEnd = NewObject<USceneComponent>(GetOwner(),TEXT("BoxTraceEnd"));
	BoxTraceEnd->RegisterComponent();
	BoxTraceEnd->AttachToComponent(this,FAttachmentTransformRules::SnapToTargetNotIncludingScale);
}

void UMGHandEquipComponent::SetupEquipMeshes(UStaticMeshComponent* InStatic)
{
	EquippedStaticMeshComp = InStatic;
}

UMeshComponent* UMGHandEquipComponent::GetPriorityMeshComponent() const
{
	if(IsValid(EquippedMeshComp))
	{
		return EquippedMeshComp;
	}
	if(IsValid(EquippedStaticMeshComp))
	{
		return EquippedStaticMeshComp;
	}
	return nullptr;
}

void UMGHandEquipComponent::SetupBoxTrace(USceneComponent* TraceStart, USceneComponent* TraceEnd)
{
	BoxTraceStart = TraceStart;
	BoxTraceEnd = TraceEnd;
}

void UMGHandEquipComponent::UpdateBoxTraceBasedOnItem(const UMGWeaponItemDefinition* ItemDefinition) const
{
	BoxTraceStart->AttachToComponent(GetPriorityMeshComponent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	BoxTraceEnd->AttachToComponent(GetPriorityMeshComponent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	BoxTraceStart->SetRelativeLocation(ItemDefinition->CollisionTransform.TraceStartOffset);
	BoxTraceEnd->SetRelativeLocation(ItemDefinition->CollisionTransform.TraceEndOffset);
}

void UMGHandEquipComponent::CreateBoxComponentToTrace(const UMGWeaponItemDefinition* WeaponNewItem)
{
	AActor* Outer = this->GetOwner();
	check(Outer);
	BoxComp = NewObject<UBoxComponent>(Outer, TEXT("WeaponCollisionBox"));
	BoxComp->SetupAttachment(EquippedStaticMeshComp);
	BoxComp->RegisterComponent();
	BoxComp->SetBoxExtent(WeaponNewItem->CollisionTransform.BoxExtent);
	BoxComp->SetRelativeLocation(WeaponNewItem->CollisionTransform.Offset);
	BoxComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BoxComp->SetCollisionResponseToAllChannels(ECR_Overlap);
	BoxComp->SetCollisionResponseToChannel(ECC_Pawn,ECR_Ignore);
}

void UMGHandEquipComponent::UpdateEquippedItem(UMGItemDefinition* NewItem)
{
	CurrentItem = NewItem;
	const UMGWeaponItemDefinition* WeaponNewItem = Cast<UMGWeaponItemDefinition>(CurrentItem);

	// 空手：隐藏两个网格
	if (!CurrentItem)
	{
		if (EquippedMeshComp) EquippedMeshComp->DestroyComponent();
		if (EquippedStaticMeshComp) EquippedStaticMeshComp->SetVisibility(false);
		if (WeaponNewItem) BoxComp->DestroyComponent();
		CurrentBehavior = nullptr;
		return;
	}

	const FEquipDisplayData& Display = CurrentItem->EquipDisplayData;
    
	if (Display.SkeletalMesh && EquippedMeshComp)
	{
		AActor* Outer = this->GetOwner();
     	check(Outer);
		EquippedMeshComp = NewObject<USkeletalMeshComponent>(Outer, TEXT("WeaponMesh"));
		EquippedMeshComp->SetupAttachment(this);
		EquippedMeshComp->RegisterComponent();
		EquippedMeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		// 武器/骨骼网格
		EquippedMeshComp->SetSkeletalMesh(Display.SkeletalMesh);
		EquippedMeshComp->SetRelativeTransform(Display.EquippedTransform);
		if (EquippedStaticMeshComp) EquippedStaticMeshComp->SetVisibility(false);
	}
	else if (Display.StaticMesh && EquippedStaticMeshComp)
	{
		// 普通物品/静态网格
		EquippedStaticMeshComp->SetStaticMesh(Display.StaticMesh);
		EquippedStaticMeshComp->SetRelativeTransform(Display.EquippedTransform);
		EquippedStaticMeshComp->SetVisibility(true);
	}
	else
	{
		// 物品没有配置手持网格，隐藏
		if (EquippedMeshComp) EquippedMeshComp->SetVisibility(false);
		if (EquippedStaticMeshComp) EquippedStaticMeshComp->SetVisibility(false);
		return;
	}
	if (WeaponNewItem)
	{
		CreateBoxComponentToTrace(WeaponNewItem);
		BoxTraceStart->AttachToComponent(GetPriorityMeshComponent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		BoxTraceEnd->AttachToComponent(GetPriorityMeshComponent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		BoxTraceStart->SetRelativeLocation(WeaponNewItem->CollisionTransform.TraceStartOffset);
		BoxTraceEnd->SetRelativeLocation(WeaponNewItem->CollisionTransform.TraceEndOffset);
		BoxComp->OnComponentBeginOverlap.AddDynamic(this, &UMGHandEquipComponent::OnBoxOverlap);
	}
}

void UMGHandEquipComponent::OnBoxOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	const FVector Start = BoxTraceStart->GetComponentLocation();
	const FVector End = BoxTraceEnd->GetComponentLocation();
	FHitResult BoxHit;
	TArray<AActor*> ActorsToIgnore;
	for (AActor* Actor :IgnoreActors)
	{
		ActorsToIgnore.AddUnique(Actor);
	}
	UKismetSystemLibrary::BoxTraceSingle(
		this,
		Start,
		End,
		FVector(5.0f, 5.0f, 5.0f),
		BoxTraceStart->GetComponentRotation(),
		TraceTypeQuery1,
		false,
		ActorsToIgnore,
		EDrawDebugTrace::ForDuration,
		BoxHit,
		true);
	if(BoxHit.GetActor())
	{
		if (IHitInterface* HitInterface = Cast<IHitInterface>(BoxHit.GetActor()))
		{
			HitInterface->Execute_GetHit(BoxHit.GetActor(), BoxHit.ImpactPoint);
		}
		IgnoreActors.AddUnique(BoxHit.GetActor());
		//CurrentBehavior->CreateFields(BoxHit.ImpactPoint);
	}
}

#if WITH_EDITOR
void UMGHandEquipComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	if (!bAutoUpdatePreview)return;
	// 获取被修改的属性
	const FProperty* ChangedProp = PropertyChangedEvent.Property;
	
	// 只监听 PreviewWeaponMesh 的改动
	if (const FName PropName = ChangedProp->GetFName(); PropName == GET_MEMBER_NAME_CHECKED(UMGHandEquipComponent, PreviewWeaponMesh) || PropName == GET_MEMBER_NAME_CHECKED(UMGHandEquipComponent, CurrentItem))
		UpdatePreviewFromDataAsset();
}

void UMGHandEquipComponent::UpdatePreviewFromDataAsset()
{

	if (!CurrentItem)
	{
		if (PreviewWeaponMesh) PreviewWeaponMesh->DestroyComponent();
		return;
	}
	
	if (!PreviewWeaponMesh)
	{
		PreviewWeaponMesh = NewObject<UStaticMeshComponent>(this, TEXT("PreviewWeaponMesh"));
		PreviewWeaponMesh->SetupAttachment(this);
		PreviewWeaponMesh->RegisterComponent();
		PreviewWeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		PreviewWeaponMesh->SetHiddenInGame(true);
	}

	// 应用武器模型
	PreviewWeaponMesh->SetStaticMesh(CurrentItem->EquipDisplayData.StaticMesh);

	// 把Trace点挂载到预览武器上，应用DataAsset里的偏移
	if (BoxTraceStart && BoxTraceEnd)
	{
		if (const UMGWeaponItemDefinition* CurrentWeapon = Cast<UMGWeaponItemDefinition>(CurrentItem))
		{
			BoxTraceStart->AttachToComponent(PreviewWeaponMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
			BoxTraceStart->SetRelativeLocation(CurrentWeapon->CollisionTransform.TraceStartOffset);
            
			BoxTraceEnd->AttachToComponent(PreviewWeaponMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
			BoxTraceEnd->SetRelativeLocation(CurrentWeapon->CollisionTransform.TraceStartOffset);
		}
	}
}

void UMGHandEquipComponent::SaveTraceOffsetsToDataAsset() const
{
	if (UMGWeaponItemDefinition* CurrentWeapon = Cast<UMGWeaponItemDefinition>(CurrentItem))
	{
		FVector &TraceStart = CurrentWeapon->CollisionTransform.TraceStartOffset;
		FVector &TraceEnd = CurrentWeapon->CollisionTransform.TraceEndOffset;

		// 获取当前Trace点相对于武器Mesh的相对变换
		const FVector StartRel = BoxTraceStart->GetRelativeLocation();
		const FVector EndRel = BoxTraceEnd->GetRelativeLocation();

		// 写入DataAsset
		TraceStart = StartRel;
		TraceEnd = EndRel;

		// 标记资产已修改，编辑器会提示保存
		if (CurrentItem->MarkPackageDirty())
			UE_LOG(LogTemp, Log, TEXT("Trace偏移已保存到 %s"), *CurrentItem->GetName());
	}
}
#endif