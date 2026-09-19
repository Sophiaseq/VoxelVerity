// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectTypes.h"
#include "GameplayTagContainer.h"
#include "Animation/AnimInstance.h"
#include "MGAnimInstance.generated.h"

class AMGCharacter;
class UCharacterMovementComponent;
/**
 * 
 */
UCLASS()
class MUNDUSGRANUM_API UMGAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
public:
	virtual void InitializeWithAbilitySystem(UAbilitySystemComponent* ASC);
	
protected:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaTime) override;
	
	// Gameplay tags that can be mapped to blueprint variables. The variables will automatically update as the tags are added or removed.
	// These should be used instead of manually querying for the gameplay tags.
	UPROPERTY(EditDefaultsOnly, Category = "GameplayTags")
	FGameplayTagBlueprintPropertyMap GameplayTagPropertyMap;
};
