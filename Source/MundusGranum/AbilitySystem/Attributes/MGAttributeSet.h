// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "MGAttributeSet.generated.h"

#define UE_API MUNDUSGRANUM_API

class AActor;
class UMGAbilitySystemComponent;
class UObject;
class UWorld;
struct FGameplayEffectSpec;

/**
 * 
 */

/** 
 * Delegate used to broadcast attribute events, some of these parameters may be null on clients: 
 * @param EffectInstigator	The original instigating actor for this event
 * @param EffectCauser		The physical actor that caused the change
 * @param EffectSpec		The full effect spec for this change
 * @param EffectMagnitude	The raw magnitude, this is before clamping
 * @param OldValue			The value of the attribute before it was changed
 * @param NewValue			The value after it was changed
*/
DECLARE_MULTICAST_DELEGATE_SixParams(FMGAttributeEvent, AActor* /*EffectInstigator*/, AActor* /*EffectCauser*/, const FGameplayEffectSpec* /*EffectSpec*/, float /*EffectMagnitude*/, float /*OldValue*/, float /*NewValue*/);


UCLASS(MinimalAPI)
class UMGAttributeSet : public UAttributeSet
{
	GENERATED_BODY()
	
public:

	UE_API UMGAttributeSet();

	UE_API UWorld* GetWorld() const override;

	UE_API UMGAbilitySystemComponent* GetMGAbilitySystemComponent() const;
};

#undef UE_API