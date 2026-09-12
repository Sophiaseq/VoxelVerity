// Fill out your copyright notice in the Description page of Project Settings.


#include "AttributeMenuWidgetController.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystem/Attributes/AttributeInfo.h"
#include "Player/MGPlayerState.h"

void UAttributeMenuWidgetController::BroadcastInitialValues()
{
	AMGPlayerState* PS = CastChecked<AMGPlayerState>(PlayerState);
	check(AttributeInfo)
	
	for (auto& Pair : PS->TagsToAttributes)
	{
		FMGAttributeInfo Info = AttributeInfo->FindAttributeInfoForTag(Pair.Key);
		FGameplayAttribute Attribute = Pair.Value();
		Info.AttributeValue = AbilitySystemComponent->GetNumericAttribute(Attribute); 
		AttributeInfoDelegate.Broadcast(Info);
	}
}

void UAttributeMenuWidgetController::BindCallbackToDependencies()
{
	AMGPlayerState* PS = CastChecked<AMGPlayerState>(PlayerState);
	
	for (auto& Pair : PS->TagsToAttributes)
	{
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(Pair.Value()).AddLambda(
			[this, Pair](const FOnAttributeChangeData& Data)
			{
				FMGAttributeInfo Info = AttributeInfo->FindAttributeInfoForTag(Pair.Key);
				FGameplayAttribute Attribute = Pair.Value();
				Info.AttributeValue = AbilitySystemComponent->GetNumericAttribute(Attribute); 
				AttributeInfoDelegate.Broadcast(Info);
			});
	}
}
