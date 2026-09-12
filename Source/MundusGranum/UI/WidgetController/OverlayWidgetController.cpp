// Fill out your copyright notice in the Description page of Project Settings.


#include "OverlayWidgetController.h"

#include "AbilitySystem/MGAbilitySystemComponent.h"
#include "AbilitySystem/Attributes/MGHealthSet.h"

void UOverlayWidgetController::BroadcastInitialValues()
{
	const UMGHealthSet* HealthSet = Cast<UMGHealthSet>(Attributes.HealthSet);
	
	OnHealthChanged.Broadcast(HealthSet->GetHealth());
	OnMaxHealthChanged.Broadcast(HealthSet->GetMaxHealth());
	OnStaminaChanged.Broadcast(HealthSet->GetStamina());
	OnMaxStaminaChanged.Broadcast(HealthSet->GetMaxStamina());
}

void UOverlayWidgetController::BindCallbackToDependencies()
{
	const UMGHealthSet* HealthSet = Cast<UMGHealthSet>(Attributes.HealthSet);
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
		HealthSet->GetHealthAttribute()).AddLambda(
			[this](const FOnAttributeChangeData& Data)
			{
				OnHealthChanged.Broadcast(Data.NewValue);
			});
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
		HealthSet->GetMaxHealthAttribute()).AddLambda(
		[this](const FOnAttributeChangeData& Data)
		{
			OnMaxHealthChanged.Broadcast(Data.NewValue);
		});
	
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
	HealthSet->GetStaminaAttribute()).AddLambda(
		[this](const FOnAttributeChangeData& Data)
		{
			OnStaminaChanged.Broadcast(Data.NewValue);
		});
	
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
	HealthSet->GetMaxStaminaAttribute()).AddLambda(
		[this](const FOnAttributeChangeData& Data)
		{
			OnMaxStaminaChanged.Broadcast(Data.NewValue);
		});
	
	Cast<UMGAbilitySystemComponent>(AbilitySystemComponent)->EffectAssetTags.AddLambda(
		[this](FGameplayTagContainer& AssetTags)
	{
		for (const FGameplayTag& Tag : AssetTags)
		{
			FGameplayTag MessageTag = FGameplayTag::RequestGameplayTag(FName("Message"));
			const FUIWidgetRow* Row = GetDataTableRowByTag<FUIWidgetRow>(MessageWidgetDataTable, Tag);
			if (Tag.MatchesTag(MessageTag) && Row)
			{
				MessageWidgetRowSignature.Broadcast(*Row);
			}
		}
	});
}

