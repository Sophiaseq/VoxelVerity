// Fill out your copyright notice in the Description page of Project Settings.


#include "MGPlayerState.h"
#include "Player/MGPlayerController.h"
#include "AbilitySystem/MGAbilitySystemComponent.h"
#include "AbilitySystem/Attributes/MGCombatSet.h"
#include "AbilitySystem/Attributes/MGHealthSet.h"
#include "Character/MGCharacterDefinition.h"
#include "Character/MGPawnExtensionComponent.h"
#include "Components/GameFrameworkComponentManager.h"
#include "GameModes/MGExperienceManagerComponent.h"
#include "GameModes/MGGameModeBase.h"
#include "Net/UnrealNetwork.h"


const FName AMGPlayerState::NAME_MGAbilityReady("MGAbilitiesReady");

AMGPlayerState::AMGPlayerState(const FObjectInitializer& ObjectInitializer) 
     : Super(ObjectInitializer)
{
     AbilitySystemComponent = CreateDefaultSubobject<UMGAbilitySystemComponent>("AbilitySystemComponent");
     AbilitySystemComponent->SetIsReplicated(true);
     AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

     // These attribute sets will be detected by AbilitySystemComponent::InitializeComponent. Keeping a reference so that the sets don't get garbage collected before that.
     HealthSet = CreateDefaultSubobject<UMGHealthSet>(TEXT("HealthSet"));
     CombatSet = CreateDefaultSubobject<UMGCombatSet>(TEXT("CombatSet"));

     // AbilitySystemComponent needs to be updated at a high frequency.
     SetNetUpdateFrequency(100.0f);
}

void AMGPlayerState::ClientInitialize(AController* C)
{
     Super::ClientInitialize(C);
     
     if (UMGPawnExtensionComponent* PawnExtComp = UMGPawnExtensionComponent::FindPawnExtensionComponent(GetPawn()))
     {
          PawnExtComp->CheckDefaultInitialization();
     }
}


void AMGPlayerState::SetPawnData(const UMGCharacterDefinition* InCharacterDefinition)
{
     check(InCharacterDefinition);

     if (GetLocalRole() != ROLE_Authority)
     {
          return;
     }

     if (PawnData)
     {
          UE_LOG(LogTemp, Error, TEXT("Trying to set PawnData [%s] on player state [%s] that already has valid PawnData [%s]."), *GetNameSafe(InCharacterDefinition), *GetNameSafe(this), *GetNameSafe(PawnData));
          return;
     }

     //MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, PawnData, this);
     PawnData = InCharacterDefinition;
     
     //TODO Ability
     /*for (const UMGAbilitySet* AbilitySet : PawnData->AbilitySets)
     {
          if (AbilitySet)
          {
               AbilitySet->GiveToAbilitySystem(AbilitySystemComponent, nullptr);
          }
     }*/

     UGameFrameworkComponentManager::SendGameFrameworkComponentExtensionEvent(this, NAME_MGAbilityReady);
	
     ForceNetUpdate();
}

void AMGPlayerState::OnRep_PawnData()
{
}

void AMGPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
     Super::GetLifetimeReplicatedProps(OutLifetimeProps);

     FDoRepLifetimeParams SharedParams;
     SharedParams.bIsPushBased = true;

     DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, PawnData, SharedParams);
}

void AMGPlayerState::OnExperienceLoaded(const UMGExperienceDefinition* CurrentExperience)
{
     if (AMGGameModeBase* MGGameMode = GetWorld()->GetAuthGameMode<AMGGameModeBase>())
     {
          if (const UMGCharacterDefinition* NewPawnData = MGGameMode->GetPawnDataForController(GetOwningController()))
          {
               SetPawnData(NewPawnData);
          }
          else
          {
               UE_LOG(LogTemp, Error, TEXT("Unable to find PawnData to initialize player state [%s]!"), *GetNameSafe(this));
          }
     }
}

AMGPlayerController* AMGPlayerState::GetMGPlayerController() const
{
     return Cast<AMGPlayerController>(GetOwner());
}

UAbilitySystemComponent* AMGPlayerState::GetAbilitySystemComponent() const
{
     return GetMGAbilitySystemComponent();
}

void AMGPlayerState::PreInitializeComponents()
{
     Super::PreInitializeComponents();
}

void AMGPlayerState::PostInitializeComponents()
{
     Super::PostInitializeComponents();
     check(AbilitySystemComponent);
     AbilitySystemComponent->InitAbilityActorInfo(this, GetPawn());
     
     UWorld* World = GetWorld();
     if (World && World->IsGameWorld() && World->GetNetMode() != NM_Client)
     {
          AGameStateBase* GameState = GetWorld()->GetGameState();
          check(GameState);
          UMGExperienceManagerComponent* ExperienceComponent = GameState->FindComponentByClass<UMGExperienceManagerComponent>();
          check(ExperienceComponent);
          ExperienceComponent->CallOrRegister_OnExperienceLoaded(FOnMGExperienceLoaded::FDelegate::CreateUObject(this, &ThisClass::OnExperienceLoaded));
     }
}

void AMGPlayerState::Reset()
{
     Super::Reset();
}
