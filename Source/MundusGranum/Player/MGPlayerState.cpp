// Fill out your copyright notice in the Description page of Project Settings.


#include "MGPlayerState.h"
#include "Player/MGPlayerController.h"
#include "AbilitySystem/MGAbilitySystemComponent.h"



AMGPlayerState::AMGPlayerState(const FObjectInitializer& ObjectInitializer) 
     : Super(ObjectInitializer)
{
}

AMGPlayerController* AMGPlayerState::GetMGPlayerController() const
{
     return Cast<AMGPlayerController>(GetOwner());
}

UAbilitySystemComponent* AMGPlayerState::GetAbilitySystemComponent() const
{
     return GetMGAbilitySystemComponent();
}
