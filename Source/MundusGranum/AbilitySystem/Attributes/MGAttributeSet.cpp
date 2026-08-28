// Fill out your copyright notice in the Description page of Project Settings.


#include "MGAttributeSet.h"

#include "AbilitySystem/MGAbilitySystemComponent.h"

UMGAttributeSet::UMGAttributeSet()
{
}

UWorld* UMGAttributeSet::GetWorld() const
{
	const UObject* Outer = GetOuter();
	check(Outer);

	return Outer->GetWorld();
}

UMGAbilitySystemComponent* UMGAttributeSet::GetMGAbilitySystemComponent() const
{
	return Cast<UMGAbilitySystemComponent>(GetOwningAbilitySystemComponent());
}
