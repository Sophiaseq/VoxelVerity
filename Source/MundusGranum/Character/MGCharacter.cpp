// Fill out your copyright notice in the Description page of Project Settings.


#include "MGCharacter.h"
#include "MGLogChannels.h"

#include "MGPawnExtensionComponent.h"
#include "AbilitySystem/MGAbilitySystemComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerState.h"
#include "Player/MGPlayerState.h"

// Sets default values
AMGCharacter::AMGCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	
	UCapsuleComponent* CapsuleComp = GetCapsuleComponent();
	check(CapsuleComp);
	CapsuleComp->InitCapsuleSize(40.0f, 90.0f);
	//CapsuleComp->SetCollisionProfileName(NAME_LyraCharacterCollisionProfile_Capsule);

	USkeletalMeshComponent* MeshComp = GetMesh();
	check(MeshComp);
	MeshComp->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));  // Rotate mesh to be X forward since it is exported as Y forward.
	//MeshComp->SetCollisionProfileName(NAME_LyraCharacterCollisionProfile_Mesh);

	UCharacterMovementComponent* MoveComp = GetCharacterMovement();
	MoveComp->MaxWalkSpeed = 230.0f;
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

}

void AMGCharacter::BeginPlay()
{
	Super::BeginPlay();
	
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

void AMGCharacter::SetupInitialAttribute() const
{
	if (!DefaultAttributes.IsEmpty())
	{
		for (TSubclassOf<UGameplayEffect> DefaultAttribute : DefaultAttributes)
		SetupAttributeByLevel(DefaultAttribute, 1.f);
	}
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

void AMGCharacter::OnAbilitySystemInitialized()
{
	UE_LOG(LogMGAbilitySystem, Warning, TEXT("[Init] Character::OnAbilitySystemInitialized → 应用初始属性"));

	SetupInitialAttribute();
	//HealthComponent
}

void AMGCharacter::OnAbilitySystemUninitialized()
{
	//HealthComponent
}

void AMGCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	
}

