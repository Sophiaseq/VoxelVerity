// Fill out your copyright notice in the Description page of Project Settings.


#include "MGPlayer.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "MundusGranum/Input/MGInputComponent.h"
#include "MundusGranum/MundusGranumGameplayTags.h"
#include "EnhancedInputComponent.h"
#include "MGInventorySlot.h"
#include "Camera/CameraComponent.h"
#include "Components/MGHandEquipComponent.h"
#include "Components/MGInventoryComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Items/MGDroppedItemActor.h"
#include "Items/Weapons/MGWeaponItemDefinition.h"

// Sets default values
AMGPlayer::AMGPlayer(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{
 	PrimaryActorTick.bCanEverTick = true;
	TurnRateGamepad = 45.f;
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.f);
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);
	ArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("ArmComponent"));
	ArmComponent->SetupAttachment(RootComponent);
	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
	CameraComponent->SetupAttachment(ArmComponent);
	
	InventoryComp = CreateDefaultSubobject<UMGInventoryComponent>(TEXT("InventoryComponent"));
	HandEquipComp = CreateDefaultSubobject<UMGHandEquipComponent>(TEXT("HandEquipComponent"));
	HandEquipComp->SetupAttachment(GetMesh(), FName("RightHandSocket"));

	/*UStaticMeshComponent* StaticItemMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticItemMesh"));
	StaticItemMeshComp->SetupAttachment(HandEquipComp);
	StaticItemMeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	StaticItemMeshComp->SetVisibility(false);
	HandEquipComp->SetupEquipMeshes(StaticItemMeshComp);
	
	USceneComponent* BoxTraceStart = CreateDefaultSubobject<USceneComponent>(TEXT("BoxTraceStart"));
	USceneComponent* BoxTraceEnd = CreateDefaultSubobject<USceneComponent>(TEXT("BoxTraceEnd"));
	HandEquipComp->SetupBoxTrace(BoxTraceStart, BoxTraceEnd);*/
}

void AMGPlayer::BeginPlay()
{
	Super::BeginPlay();
	if (InventoryComp)
	{
		InventoryComp->OnSelectedSlotChanged.AddDynamic(this, &AMGPlayer::HandleSelectedSlotChanged);
		InventoryComp->OnInventoryChanged.AddDynamic(this, &AMGPlayer::HandleInventoryChanged);
	}
	RefreshEquippedItemFromSelectedSlot();
}

// Called every frame
void AMGPlayer::Tick(const float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AMGPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	
	// 强制转换为自定义输入组件
	InputComp = Cast<UMGInputComponent>(PlayerInputComponent);
	check(InputComp);
	check(GlobalInputConfig);

	// ========== 全部输入绑定（复刻Lyra BindNativeAction逻辑） ==========
	// 移动轴输入
	InputComp->BindNativeAction(
		GlobalInputConfig,
		MundusGranumGameplayTags::InputTag_Move,
		ETriggerEvent::Triggered,
		this,
		&AMGPlayer::Input_Move);

	// 鼠标视角输入
	InputComp->BindNativeAction(
		GlobalInputConfig,
		MundusGranumGameplayTags::InputTag_Look_Mouse,
		ETriggerEvent::Triggered,
		this,
		&AMGPlayer::Input_LookMouse);
	
	InputComp->BindNativeAction(
		GlobalInputConfig,
		MundusGranumGameplayTags::InputTag_Jump,
		ETriggerEvent::Triggered,
		this,
		&AMGPlayer::Input_Jump);
	
	InputComp->BindNativeAction(
		GlobalInputConfig,
		MundusGranumGameplayTags::InputTag_Sprint,
		ETriggerEvent::Started,
		this,
		&AMGPlayer::Input_SprintPressed);
	
	InputComp->BindNativeAction(
		GlobalInputConfig,
		MundusGranumGameplayTags::InputTag_Sprint,
		ETriggerEvent::Completed,
		this,
		&AMGPlayer::Input_SprintReleased);
	
	InputComp->BindNativeAction(
		GlobalInputConfig,
		MundusGranumGameplayTags::InputTag_Pickup,
		ETriggerEvent::Triggered,
		this,
		&AMGPlayer::Input_Pickup);
	
	InputComp->BindNativeAction(
		GlobalInputConfig,
		MundusGranumGameplayTags::InputTag_UseLeftHandItem,
		ETriggerEvent::Triggered,
		this,
		&AMGPlayer::InputTag_UseLeftHandItem);
	
	InputComp->BindNativeAction(
		GlobalInputConfig,
		MundusGranumGameplayTags::InputTag_UseRightHandItem,
		ETriggerEvent::Triggered,
		this,
		&AMGPlayer::InputTag_UseRightHandItem);
	
	InputComp->BindNativeAction(
		GlobalInputConfig,
		MundusGranumGameplayTags::InputTag_SelectItem,
		ETriggerEvent::Triggered,
		this,
		&AMGPlayer::InputTag_SelectItem);
	
	InputComp->BindNativeAction(
		GlobalInputConfig,
		MundusGranumGameplayTags::InputTag_SlowWalk,
		ETriggerEvent::Triggered,
		this,
		&AMGPlayer::InputTag_SlowWalk);
}

void AMGPlayer::Input_Move(const FInputActionValue& InputActionValue)
{
	if (Controller != nullptr)
	{
		const FVector2D MoveValue = InputActionValue.Get<FVector2D>();
		const FRotator MovementRotation(0.0f, Controller->GetControlRotation().Yaw, 0.0f);
 
		if (MoveValue.X != 0.0f)
		{
			const FVector MovementDirection = MovementRotation.RotateVector(FVector::RightVector);
			AddMovementInput(MovementDirection, MoveValue.X);
		}
 
		if (MoveValue.Y != 0.0f)
		{
			const FVector MovementDirection = MovementRotation.RotateVector(FVector::ForwardVector);
			AddMovementInput(MovementDirection, MoveValue.Y);
		}
	}
}

void AMGPlayer::Input_LookMouse(const FInputActionValue& InputActionValue)
{
	if (Controller != nullptr)
	{
		const FVector2D LookValue = InputActionValue.Get<FVector2D>();
 
		if (LookValue.X != 0.0f)
		{
			TurnAtRate(LookValue.X);
		}
 
		if (LookValue.Y != 0.0f)
		{
			LookUpAtRate(LookValue.Y);
		}
	}
}

void AMGPlayer::Input_Jump()
{
	ACharacter::Jump();
}

void AMGPlayer::Input_SprintPressed()
{
	if (bIsSprinting) return;
	bIsSprinting = true;
	GetCharacterMovement()->MaxWalkSpeed =SprintSpeed;
}

void AMGPlayer::Input_SprintReleased()
{
	bIsSprinting = false;
	GetCharacterMovement()->MaxWalkSpeed =WalkSpeed;
}

void AMGPlayer::Input_Pickup()
{
	// 清除已失效的指针
	NearbyDrops.RemoveAll([](const TWeakObjectPtr<AMGDroppedItemActor>& Ptr) { return !Ptr.IsValid(); });

	if (NearbyDrops.Num() == 0) return;

	// 策略：拾取列表中距离最近的掉落物（也可以按优先级拾取第一个）
	AMGDroppedItemActor* Nearest = nullptr;
	float NearestDistSq = MAX_FLT;
	FVector MyLocation = GetActorLocation();   

	for (const TWeakObjectPtr<AMGDroppedItemActor>& DropPtr : NearbyDrops)
	{
		if (AMGDroppedItemActor* Drop = DropPtr.Get())
		{
			float DistSq = FVector::DistSquared(MyLocation, Drop->GetActorLocation());
			if (DistSq < NearestDistSq)
			{
				NearestDistSq = DistSq;
				Nearest = Drop;
			}
		}
	}

	if (!Nearest) return; 
	
	int32 Remain = InventoryComp->PickupItem(Nearest->GetItemDef(), Nearest->GetCount());

	if (Remain == 0)
	{
		NearbyDrops.Remove(Nearest);
		Nearest->Destroy();
	}
	else if (Remain < Nearest->GetCount())
	{
		Nearest->SetCount(Remain);
	}
	// Remain == Count 则什么都没捡，保持原样
}

void AMGPlayer::PlayAttackMontage() const
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && AttackMontage)
	{
		AnimInstance->Montage_Play(AttackMontage);
		static int32 Selection = 0;
		Selection = Selection % 4;
		AnimInstance->Montage_JumpToSection(FName("L"+FString::FromInt(Selection)));
		Selection++;
	}
}

void AMGPlayer::InputTag_UseLeftHandItem()
{
	if (ActionState == MundusGranumGameplayTags::ActionState_Unoccupied && CharacterState != MundusGranumGameplayTags::CharacterState_Unequipped)
	{
		PlayAttackMontage();
		ActionState = MundusGranumGameplayTags::ActionState_Occupied;
	}
}

void AMGPlayer::InputTag_UseRightHandItem()
{
}

void AMGPlayer::InputTag_SelectItem(const FInputActionValue& Value)
{
	if (!InventoryComp) return;
	const float Axis = Value.Get<float>();
	if (FMath::IsNearlyZero(Axis)) return;
	const int32 Delta = Axis > 0.f ? 1 : -1;
	InventoryComp->SelectSlot(InventoryComp->SelectedSlotIndex + Delta);
}

void AMGPlayer::InputTag_SlowWalk()
{
	GetCharacterMovement()->MaxWalkSpeed = WanderingSpeed;
}

void AMGPlayer::RefreshEquippedItemFromSelectedSlot()
{
	if (!InventoryComp || !HandEquipComp) return;
	UMGItemDefinition* Item = InventoryComp->GetSelectedItem();
	HandEquipComp->UpdateEquippedItem(Item);
	CharacterState = Item ? MundusGranumGameplayTags::CharacterState_OneHandedEquipped : MundusGranumGameplayTags::CharacterState_Unequipped;
}

void AMGPlayer::HandleInventoryChanged(int32 SlotIndex)
{
	if (InventoryComp && SlotIndex == InventoryComp->GetSelectedSlotIndex())
	{
		RefreshEquippedItemFromSelectedSlot();
	}
}

void AMGPlayer::HandleSelectedSlotChanged(int32 NewIndex)
{
	RefreshEquippedItemFromSelectedSlot();
}

void AMGPlayer::SetWeaponCollisionEnabled(const ECollisionEnabled::Type CollisionEnabled)
{
	if (HandEquipComp && HandEquipComp->GetBoxComp())
	{
		HandEquipComp->GetBoxComp()->SetCollisionEnabled(CollisionEnabled);
		HandEquipComp->IgnoreActors.Empty();
	}
}

void AMGPlayer::TurnAtRate(const float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * TurnRateGamepad * GetWorld()->GetDeltaSeconds());
}
 
void AMGPlayer::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * TurnRateGamepad * GetWorld()->GetDeltaSeconds());
}

int32 AMGPlayer::AttemptPickup(UMGItemDefinition* Item, int32 Count)
{
	if (!InventoryComp) return Count; 
	
	int32 Remain = InventoryComp->AddItem(Item, Count);
    
	// 可在此添加拾取音效/粒子，完全解耦
	if (Remain < Count)
	{
		// 播放拾取特效（不关心掉落实体）
	}
	return Remain;
}

void AMGPlayer::AddNearbyDrop(AMGDroppedItemActor* Drop, AActor* OtherActor)
{
	if (Drop && !NearbyDrops.Contains(Drop))
	{
		NearbyDrops.Add(Drop);
	}
}

void AMGPlayer::RemoveNearbyDrop(AMGDroppedItemActor* Drop, AActor* OtherActor)
{
	NearbyDrops.Remove(Drop);
}

float AMGPlayer::GetMovementDirection() const
{
	if (!GetCharacterMovement()) return 0.f;

	// 1. 获取当前速度（或输入向量），平摊到地面（忽略 Z 轴）
	FVector Velocity = GetCharacterMovement()->Velocity;
	Velocity.Z = 0.f;

	// 如果速度太小（静止），返回 0 保持当前方向，防止抖动
	if (Velocity.IsNearlyZero(1.f)) return 0.f;

	// 2. 获取角色面朝方向（即 +X 轴）
	FVector Forward = GetActorForwardVector();
	Forward.Z = 0.f;

	// 3. 核心计算：速度朝向相对于角色面朝方向的旋转差值
	FRotator ForwardRot = Forward.Rotation();
	FRotator VelocityRot = Velocity.Rotation();
    
	// DeltaRotator 自动计算出最短路径，并归一化到 -180~180
	FRotator DeltaRot = (VelocityRot - ForwardRot).GetNormalized();
    
	// 返回 Yaw（偏航角），这是你在混合空间水平轴（Direction）需要的值
	return DeltaRot.Yaw;
}

