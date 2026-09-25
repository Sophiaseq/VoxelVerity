// Fill out your copyright notice in the Description page of Project Settings.


#include "MGAnimInstance.h"

#include "AbilitySystemGlobals.h"

void UMGAnimInstance::InitializeWithAbilitySystem(UAbilitySystemComponent* ASC)
{
	check(ASC);

	GameplayTagPropertyMap.Initialize(this, ASC);
	GameplayTagPropertyMap.ApplyCurrentTags();
}

void UMGAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	
	if (const AActor* OwningActor = GetOwningActor())
	{
		if (UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(OwningActor))
		{
			InitializeWithAbilitySystem(ASC);
		}
	}
}

void UMGAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);
}

