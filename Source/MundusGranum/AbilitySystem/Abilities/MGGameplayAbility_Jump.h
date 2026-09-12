// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "MGGameplayAbility.h"

#include "MGGameplayAbility_Jump.generated.h"

class UObject;
struct FFrame;
struct FGameplayAbilityActorInfo;
struct FGameplayTagContainer;


/**
 * UMGGameplayAbility_Jump
 *
 *	Gameplay ability used for character jumping.
 */
UCLASS(Abstract)
class UMGGameplayAbility_Jump : public UMGGameplayAbility
{
	GENERATED_BODY()

public:

	UMGGameplayAbility_Jump(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:

	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	UFUNCTION(BlueprintCallable, Category = "MG|Ability")
	void CharacterJumpStart();

	UFUNCTION(BlueprintCallable, Category = "MG|Ability")
	void CharacterJumpStop();
};