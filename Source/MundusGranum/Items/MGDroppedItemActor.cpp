// PickupActor.cpp
#include "MGDroppedItemActor.h"

#include "Components/ItemContainer.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Items/Weapons/MGWeaponItemDefinition.h"

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

	bReplicates = true;
}

void AMGDroppedItemActor::BeginPlay()
{
	Super::BeginPlay();
	PickupSphere->OnComponentBeginOverlap.AddDynamic(this,&AMGDroppedItemActor::OnSphereOverlap);
	PickupSphere->OnComponentEndOverlap.AddDynamic(this,&AMGDroppedItemActor::SphereOverlapEnd);
	if (ItemDef && ItemDef->DropMesh)
	{
		MeshComponent->SetStaticMesh(ItemDef->DropMesh);
	}
}

void AMGDroppedItemActor::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor || OtherActor == this) return;

	// 只处理 Pawn（避免掉落物之间、子弹等误触发）
	APawn* Pawn = Cast<APawn>(OtherActor);
	if (!Pawn) return;

	// 拾取是服务器权威逻辑
	if (!HasAuthority()) return;

	// 冷却：背包满时不至于每帧刷
	const double Now = GetWorld()->GetTimeSeconds();
	if (Now - LastPickupAttemptTime < PickupRetryCooldown) return;
	LastPickupAttemptTime = Now;

	// 走接口尝试拾取（实际拾取逻辑在 TryPickup_Implementation 里：找 Picker 身上的 IItemContainer 并 AddItemToContainer）
	IPickupable::Execute_TryPickup(this, Pawn);
}

void AMGDroppedItemActor::SphereOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
}

int32 AMGDroppedItemActor::TryPickup_Implementation(AActor* Picker)
{
	if (!HasAuthority() || !Picker || !ItemDef || Count <= 0) return 0;

	// 从 Picker 上找任意实现了 IItemContainer 的对象
	UObject* ContainerObj = nullptr;

	// 优先找组件
	TArray<UActorComponent*> Components;
	Picker->GetComponents(Components);
	for (UActorComponent* Comp : Components)
	{
		if (Comp && Comp->Implements<UItemContainer>())
		{
			ContainerObj = Comp;
			break;
		}
	}
	// 再退而求其次，看 Picker 自身是不是容器
	if (!ContainerObj && Picker->Implements<UItemContainer>())
	{
		ContainerObj = Picker;
	}

	if (!ContainerObj) return 0;

	const int32 Remain = IItemContainer::Execute_AddItemToContainer(ContainerObj, ItemDef, Count);
	const int32 PickedUp = Count - Remain;

	if (PickedUp <= 0) return 0;

	Count = Remain;
	if (Count <= 0)
	{
		Destroy();
	}
	return PickedUp;
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
