// Fill out your copyright notice in the Description page of Project Settings.


#include "MGAnimInstance.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "Character/MGPawnExtensionComponent.h"

void UMGAnimInstance::InitializeWithAbilitySystem(UAbilitySystemComponent* ASC)
{
	check(ASC);

	GameplayTagPropertyMap.Initialize(this, ASC);
	GameplayTagPropertyMap.ApplyCurrentTags();
}

void UMGAnimInstance::OnAbilitySystemInitialized()
{
	if (UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(GetOwningActor()))
	{
		InitializeWithAbilitySystem(ASC);
	}
}

void UMGAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	if (const AActor* OwningActor = GetOwningActor())
	{	
		if (UMGPawnExtensionComponent* PawnExtComp = UMGPawnExtensionComponent::FindPawnExtensionComponent(OwningActor))
		{
			PawnExtComp->OnAbilitySystemInitialized_RegisterAndCall(
				FSimpleMulticastDelegate::FDelegate::CreateUObject(this, &ThisClass::OnAbilitySystemInitialized)
			);
		}
	}
}

void UMGAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);
}

