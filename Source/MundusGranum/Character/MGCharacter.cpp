// Fill out your copyright notice in the Description page of Project Settings.


#include "MGCharacter.h"

#include "MGHealthComponent.h"
#include "MGLogChannels.h"
#include "MGPawnExtensionComponent.h"
#include "MundusGranum.h"
#include "MundusGranumGameplayTags.h"
#include "AbilitySystem/MGAbilitySystemComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/MGEquipmentComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Player/MGPlayerState.h"

// Sets default values
AMGCharacter::AMGCharacter()
{
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;
	
	bReplicates = true;
	GetCharacterMovement()->SetIsReplicated(true);
	
	UCapsuleComponent* CapsuleComp = GetCapsuleComponent();
	check(CapsuleComp);
	CapsuleComp->InitCapsuleSize(40.0f, 90.0f);
	CapsuleComp->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	CapsuleComp->SetGenerateOverlapEvents(false);

	USkeletalMeshComponent* MeshComp = GetMesh();
	check(MeshComp);
	MeshComp->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
	MeshComp->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	MeshComp->SetCollisionResponseToChannel(ECC_Projectile, ECR_Overlap);
	MeshComp->SetGenerateOverlapEvents(true);

	UCharacterMovementComponent* MoveComp = GetCharacterMovement();
	MoveComp->MaxWalkSpeed = WalkSpeed;
	MoveComp->GravityScale = 1.0f;
	MoveComp->MaxAcceleration = 2400.0f;
	MoveComp->BrakingFrictionFactor = 1.0f;
	MoveComp->BrakingFriction = 6.0f;
	MoveComp->GroundFriction = 8.0f;
	MoveComp->BrakingDecelerationWalking = 1400.0f;
	MoveComp->bUseControllerDesiredRotation = false;
	MoveComp->bOrientRotationToMovement = true;
	MoveComp->RotationRate = FRotator(0.0f, 720.0f, 0.0f);
	MoveComp->bAllowPhysicsRotationDuringAnimRootMotion = false;
	MoveComp->GetNavAgentPropertiesRef().bCanCrouch = true;
	MoveComp->bCanWalkOffLedgesWhenCrouching = true;
	MoveComp->SetCrouchedHalfHeight(65.0f);
	
	PawnExtComponent = CreateDefaultSubobject<UMGPawnExtensionComponent>(TEXT("PawnExtensionComponent"));
	PawnExtComponent->OnAbilitySystemInitialized_RegisterAndCall(FSimpleMulticastDelegate::FDelegate::CreateUObject(this, &ThisClass::OnAbilitySystemInitialized));
	PawnExtComponent->OnAbilitySystemUninitialized_Register(FSimpleMulticastDelegate::FDelegate::CreateUObject(this, &ThisClass::OnAbilitySystemUninitialized));

	HealthComponent = CreateDefaultSubobject<UMGHealthComponent>(TEXT("HealthComponent"));
	HealthComponent->OnDeathStarted.AddDynamic(this, &ThisClass::OnDeathStarted);
	HealthComponent->OnDeathFinished.AddDynamic(this, &ThisClass::OnDeathFinished);
}

void AMGCharacter::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogMG, Warning, TEXT("HasAuthority=%d LocalRole=%d RemoteRole=%d Controller=%s IsLocallyControlled=%d"),
	HasAuthority(),
	(int)GetLocalRole(),
	(int)GetRemoteRole(),
	*GetNameSafe(GetController()),
	IsLocallyControlled());
}

void AMGCharacter::SetSprinting(bool bSprinting)
{
	// 本地立即生效（客户端用于预测，服务器用于权威）
	GetCharacterMovement()->MaxWalkSpeed = bSprinting ? SprintSpeed : WalkSpeed;

	// 客户端把冲刺状态发给服务器
	if (!HasAuthority())
	{
		ServerSetSprinting(bSprinting);
	}
}

void AMGCharacter::ServerSetSprinting_Implementation(bool bSprinting)
{
	GetCharacterMovement()->MaxWalkSpeed = bSprinting ? SprintSpeed : WalkSpeed;
}

void AMGCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	UE_LOG(LogMG, Warning, TEXT("[Init] Character::PossessedBy → HandleControllerChanged"));

	PawnExtComponent->HandleControllerChanged();
}

void AMGCharacter::UnPossessed()
{
	Super::UnPossessed();
	
	PawnExtComponent->HandleControllerChanged();
}

void AMGCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	
	PawnExtComponent->HandlePlayerStateReplicated();
}
void AMGCharacter::OnRep_Controller()
{
	Super::OnRep_Controller();

	PawnExtComponent->HandleControllerChanged();
}

void AMGCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AMGCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	
	PawnExtComponent->SetupPlayerInputComponent();
}

void AMGCharacter::SetupAttributeByLevel(TSubclassOf<UGameplayEffect> AttributeEffect, float Level) const
{
	check(IsValid(GetAbilitySystemComponent()));
	if (AttributeEffect)
	{
		FGameplayEffectContextHandle ContextHandle = GetAbilitySystemComponent()->MakeEffectContext();
		ContextHandle.AddSourceObject(this);
		const FGameplayEffectSpecHandle EffectSpecHandle = GetAbilitySystemComponent()->MakeOutgoingSpec(AttributeEffect, Level, ContextHandle);
		GetAbilitySystemComponent()->ApplyGameplayEffectSpecToTarget(*EffectSpecHandle.Data.Get(), GetAbilitySystemComponent());
	}
}

UMGAbilitySystemComponent* AMGCharacter::GetMGAbilitySystemComponent() const
{
	return Cast<UMGAbilitySystemComponent>(GetAbilitySystemComponent());
}

UAbilitySystemComponent* AMGCharacter::GetAbilitySystemComponent() const
{
	if (PawnExtComponent == nullptr)
	{
		return nullptr;
	}

	return PawnExtComponent->GetMGAbilitySystemComponent();
}

float AMGCharacter::GetCharacterLevel()
{
	if (const AMGPlayerState* MGPlayerState = Cast<AMGPlayerState>(GetPlayerState()))
		return MGPlayerState->GetPlayerLevel();
	return 0.0f;
}

UMGWeaponItemDefinition* AMGCharacter::GetCurrentWeapon() const
{
	return FindComponentByClass<UMGEquipmentComponent>() ? FindComponentByClass<UMGEquipmentComponent>()->GetCurrentWeapon() : nullptr;
}

FVector AMGCharacter::GetSocketLocation(FName TagName, FName SocketName) const
{
	if (UActorComponent* Comp = FindComponentByTag(UPrimitiveComponent::StaticClass(), TagName))
 		if (const USceneComponent* SceneComp = Cast<USceneComponent>(Comp))
 			return SceneComp->GetSocketLocation(SocketName);
 	return FVector::ZeroVector;
}

UAnimMontage* AMGCharacter::GetHitReactMontage() const
{
	return PawnExtComponent->GetPawnData<UMGPawnData>()->HitReactMontage;
}

void AMGCharacter::OnDeathStarted(AActor* OwningActor)
{
	DisableMovementAndCollision();
	
	GetMesh()->SetSimulatePhysics(true);
	GetMesh()->SetEnableGravity(true);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
	GetMesh()->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
}

void AMGCharacter::OnDeathFinished(AActor* OwningActor)
{
	GetWorld()->GetTimerManager().SetTimerForNextTick(this, &ThisClass::DestroyDueToDeath);
}

void AMGCharacter::DisableMovementAndCollision()
{
	if (GetController())
	{
		GetController()->SetIgnoreMoveInput(true);
	}

	UCapsuleComponent* CapsuleComp = GetCapsuleComponent();
	check(CapsuleComp);
	CapsuleComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CapsuleComp->SetCollisionResponseToAllChannels(ECR_Ignore);

	UCharacterMovementComponent* MoveComp = GetCharacterMovement();
	MoveComp->StopMovementImmediately();
	MoveComp->DisableMovement();
}

void AMGCharacter::DestroyDueToDeath()
{
	K2_OnDeathFinished();

	UninitAndDestroy();
}

void AMGCharacter::UninitAndDestroy()
{
	if (GetLocalRole() == ROLE_Authority)
	{
		DetachFromControllerPendingDestroy();
		SetLifeSpan(1.0f);
	}

	// Uninitialize the ASC if we're still the avatar actor (otherwise another pawn already did it when they became the avatar actor)
	if (UMGAbilitySystemComponent* MGASC = GetMGAbilitySystemComponent())
	{
		if (MGASC->GetAvatarActor() == this)
		{
			PawnExtComponent->UninitializeAbilitySystem();
		}
	}

	SetActorHiddenInGame(true);
}

void AMGCharacter::InitializeGameplayTags()
{
	if (UMGAbilitySystemComponent* MGASC = GetMGAbilitySystemComponent())
	{
		for (const TPair<uint8, FGameplayTag>& TagMapping : MundusGranumGameplayTags::MovementModeTagMap)
		{
			if (TagMapping.Value.IsValid())
			{
				MGASC->SetLooseGameplayTagCount(TagMapping.Value, 0);
			}
		}

		//暂时不需要
		/*for (const TPair<uint8, FGameplayTag>& TagMapping : MundusGranumGameplayTags::CustomMovementModeTagMap)
		{
			if (TagMapping.Value.IsValid())
			{
				MGASC->SetLooseGameplayTagCount(TagMapping.Value, 0);
			}
		}*/

		/*UMGCharacterMovementComponent* MGMoveComp = CastChecked<UMGCharacterMovementComponent>(GetCharacterMovement());
		SetMovementModeTag(MGMoveComp->MovementMode, MGMoveComp->CustomMovementMode, true);*/
	}
}

void AMGCharacter::OnAbilitySystemInitialized()
{
	UMGAbilitySystemComponent* MGASC = GetMGAbilitySystemComponent();
	check(MGASC);

	HealthComponent->InitializeWithAbilitySystem(MGASC);

	InitializeGameplayTags();
}

void AMGCharacter::OnAbilitySystemUninitialized()
{
	HealthComponent->UninitializeFromAbilitySystem();
}

void AMGCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	
}

