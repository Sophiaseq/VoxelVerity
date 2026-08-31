// Fill out your copyright notice in the Description page of Project Settings.


#include "MGPlayerState.h"
#include "Player/MGPlayerController.h"
#include "AbilitySystem/MGAbilitySystemComponent.h"
#include "AbilitySystem/Attributes/MGCombatSet.h"
#include "AbilitySystem/Attributes/MGHealthSet.h"


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

AMGPlayerController* AMGPlayerState::GetMGPlayerController() const
{
     return Cast<AMGPlayerController>(GetOwner());
}

UAbilitySystemComponent* AMGPlayerState::GetAbilitySystemComponent() const
{
     return GetMGAbilitySystemComponent();
}
