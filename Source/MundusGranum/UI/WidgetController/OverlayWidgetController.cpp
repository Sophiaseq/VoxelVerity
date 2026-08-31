// Fill out your copyright notice in the Description page of Project Settings.


#include "OverlayWidgetController.h"

#include "AbilitySystem/Attributes/MGHealthSet.h"

void UOverlayWidgetController::BroadcastInitialValues()
{
	const UMGHealthSet* HealthSet = Cast<UMGHealthSet>(AttributeSet);
	
	OnHealthChanged.Broadcast(HealthSet->GetHealth());
	OnMaxHealthChanged.Broadcast(HealthSet->GetMaxHealth());
	
}

void UOverlayWidgetController::BindCallbackToDependencies()
{
	const UMGHealthSet* HealthSet = Cast<UMGHealthSet>(AttributeSet);
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
		HealthSet->GetHealthAttribute()).AddUObject(this, &UOverlayWidgetController::HealthChanged);
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
		HealthSet->GetMaxHealthAttribute()).AddUObject(this, &UOverlayWidgetController::MaxHealthChanged);
}

void UOverlayWidgetController::HealthChanged(const FOnAttributeChangeData& Data) const
{
	OnHealthChanged.Broadcast(Data.NewValue);
}

void UOverlayWidgetController::MaxHealthChanged(const FOnAttributeChangeData& Data) const
{
	OnMaxHealthChanged.Broadcast(Data.NewValue);
}
