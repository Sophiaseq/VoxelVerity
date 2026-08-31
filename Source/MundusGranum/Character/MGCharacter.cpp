// Fill out your copyright notice in the Description page of Project Settings.


#include "MGCharacter.h"

#include "MGPawnExtensionComponent.h"
#include "AbilitySystem/MGAbilitySystemComponent.h"
#include "Player/MGPlayerController.h"
#include "Player/MGPlayerState.h"
#include "UI/HUD/MGHUD.h"


// Sets default values
AMGCharacter::AMGCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AMGCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

void AMGCharacter::InitAbilityActorInfo()
{
	AMGPlayerState* MGPlayerState = GetPlayerState<AMGPlayerState>();
	check(MGPlayerState);
	MGPlayerState->GetAbilitySystemComponent()->InitAbilityActorInfo(MGPlayerState,this);
	UMGAbilitySystemComponent* ASC = MGPlayerState->GetMGAbilitySystemComponent();
	PawnExtComponent->InitializeAbilitySystem(ASC, this);
	
	if (AMGPlayerController* MGPlayerController = Cast<AMGPlayerController>(GetController()))
	{
		if (AMGHUD* MGHUD = Cast<AMGHUD>(MGPlayerController->GetHUD()))
		{
			MGHUD->InitOverlay(MGPlayerController, MGPlayerState, ASC, MGPlayerState->GetHealth());
		}
	}
}

void AMGCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	
	//Init ability actor info for the Server
	InitAbilityActorInfo();
}

void AMGCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	
	InitAbilityActorInfo();
}

void AMGCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AMGCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
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

