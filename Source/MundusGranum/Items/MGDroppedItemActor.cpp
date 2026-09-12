// PickupActor.cpp
#include "MGDroppedItemActor.h"

#include "Interaction/MGInteractionReceiver.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Items/Weapons/MGWeaponItemDefinition.h"
#include "Net/UnrealNetwork.h"

AMGDroppedItemActor::AMGDroppedItemActor()
{
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	RootComponent = MeshComponent;

	PickupSphere = CreateDefaultSubobject<USphereComponent>(TEXT("TriggerComp"));
	PickupSphere->SetupAttachment(RootComponent);
	PickupSphere->SetSphereRadius(100.0f);

	// 默认禁用物理模拟（可以在初始化时按需开启）
	MeshComponent->SetSimulatePhysics(false);
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

	// 重要：为了网络复制
	//bReplicates = true;
}

void AMGDroppedItemActor::BeginPlay()
{
	Super::BeginPlay();
	PickupSphere->OnComponentBeginOverlap.AddDynamic(this,&AMGDroppedItemActor::OnSphereOverlap);
	PickupSphere->OnComponentEndOverlap.AddDynamic(this,&AMGDroppedItemActor::SphereOverlapEnd);
	if (ItemDef)
	{
		InitializeDroppedItemActor(ItemDef);
	}
}

void AMGDroppedItemActor::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (IMGInteractionReceiver* InteractionReceiver = Cast<IMGInteractionReceiver>(OtherActor))
	{
		InteractionReceiver->AddNearbyDrop(this, OtherActor);
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(1,30.f,FColor::Blue,FString("Ending Overlap with") + OtherActor->GetName());
		}
	}
	//直接拾取的逻辑
	/*int32 Remain = IMGInteractionReceiver::Execute_AttemptPickup(OtherActor, ItemDef, Count);
	if (Remain == 0)
	{
		Destroy(); // 全部拾取，销毁自身www
	}
	else if (Remain < Count)
	{
		Count = Remain; // 只捡走一部分（背包满了），更新数量
		// 可在此更新 3D 悬浮文字显示 "x5"
	}
	// Remain == Count 时什么都没发生，留着*/
}

void AMGDroppedItemActor::SphereOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (IMGInteractionReceiver* InteractionReceiver = Cast<IMGInteractionReceiver>(OtherActor))
	{
		InteractionReceiver->RemoveNearbyDrop(this, OtherActor);
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(1,30.f,FColor::Blue,FString("Ending Overlap with") + OtherActor->GetName());
		}
	}
}

void AMGDroppedItemActor::InitializeDroppedItemActor(UMGItemDefinition* InItemDef)
{
	if (!InItemDef) return;

	ItemDef = InItemDef;
	if (ItemDef->DropMesh)
	{
		MeshComponent->SetStaticMesh(ItemDef->DropMesh);
	}
}

#if WITH_EDITOR
void AMGDroppedItemActor::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	if (!bAutoUpdatePreview)return;
	// 获取被修改的属性
	const FProperty* ChangedProp = PropertyChangedEvent.Property;
	if (!ChangedProp) return;
	
	// 只监听 PreviewWeaponMesh 的改动
	if (const FName PropName = ChangedProp->GetFName(); PropName == GET_MEMBER_NAME_CHECKED(AMGDroppedItemActor, ItemDef))
		UpdatePreviewFromDataAsset();
}

void AMGDroppedItemActor::UpdatePreviewFromDataAsset()
{

	if (!ItemDef)
	{
		if (MeshComponent && BoxTraceStart && BoxTraceEnd)
		{
			MeshComponent->SetStaticMesh(nullptr);
			BoxTraceStart->DestroyComponent();
			BoxTraceEnd->DestroyComponent();
		}
		return;
	}
	
	if (!MeshComponent)
	{
		MeshComponent = NewObject<UStaticMeshComponent>(this, TEXT("PreviewWeaponMesh"));
		MeshComponent->SetupAttachment(MeshComponent);
		MeshComponent->RegisterComponent();
		MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		MeshComponent->SetHiddenInGame(true);
	}

	// 应用武器模型
	MeshComponent->SetStaticMesh(ItemDef->EquipDisplayData.StaticMesh);

	// 把Trace点挂载到预览武器上，应用DataAsset里的偏移
	if (const UMGWeaponItemDefinition* CurrentWeapon = Cast<UMGWeaponItemDefinition>(ItemDef))
	{
		BoxTraceStart = NewObject<USceneComponent>(this, TEXT("BoxTraceStart"));
		BoxTraceStart->AttachToComponent(MeshComponent, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		BoxTraceStart->RegisterComponent();
		BoxTraceStart->SetRelativeLocation(CurrentWeapon->CollisionTransform.TraceStartOffset);
			
		BoxTraceEnd = NewObject<USceneComponent>(this, TEXT("BoxTraceEnd"));
		BoxTraceEnd->AttachToComponent(MeshComponent, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		BoxTraceStart->RegisterComponent();
		BoxTraceEnd->SetRelativeLocation(CurrentWeapon->CollisionTransform.TraceStartOffset);
	}
}

void AMGDroppedItemActor::SaveTraceOffsetsToDataAsset() const
{
	if (UMGWeaponItemDefinition* CurrentWeapon = Cast<UMGWeaponItemDefinition>(ItemDef))
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
		if (ItemDef->MarkPackageDirty())
			UE_LOG(LogTemp, Log, TEXT("Trace偏移已保存到 %s"), *ItemDef->GetName());
	}
}
#endif
/*void AMGDroppedItemActor::OnRep_ItemDef()
{
	// 客户端收到复制后，更新自己的视觉表现
	if (ItemDef && ItemDef->DropMesh)
	{
		MeshComponent->SetStaticMesh(ItemDef->DropMesh);
		MeshComponent->SetWorldScale3D(ItemDef->DropMeshScale);
	}
}

void AMGDroppedItemActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AMGDroppedItemActor, ItemDef);
}*/
