// Copyright Epic Games, Inc. All Rights Reserved.

#include "MGGameplayAbility_Jump.h"

#include "AbilitySystem/Abilities/MGGameplayAbility.h"
#include "Character/MGCharacter.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(MGGameplayAbility_Jump)

struct FGameplayTagContainer;


UMGGameplayAbility_Jump::UMGGameplayAbility_Jump(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
	ActivationPolicy = EMGAbilityActivationPolicy::OnInputTriggered;
}

bool UMGGameplayAbility_Jump::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!ActorInfo || !ActorInfo->AvatarActor.IsValid())
	{
		return false;
	}

	const AMGCharacter* MGCharacter = Cast<AMGCharacter>(ActorInfo->AvatarActor.Get());
	if (!MGCharacter || !MGCharacter->CanJump())
	{
		return false;
	}

	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	return true;
}

void UMGGameplayAbility_Jump::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	// Stop jumping in case the ability blueprint doesn't call it.
	CharacterJumpStop();

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UMGGameplayAbility_Jump::CharacterJumpStart()
{
	if (AMGCharacter* MGCharacter = GetMGCharacterFromActorInfo())
	{
		if (MGCharacter->IsLocallyControlled() && !MGCharacter->bPressedJump)
		{
			MGCharacter->UnCrouch();
			MGCharacter->Jump();
		}
	}
}

void UMGGameplayAbility_Jump::CharacterJumpStop()
{
	if (AMGCharacter* MGCharacter = GetMGCharacterFromActorInfo())
	{
		if (MGCharacter->IsLocallyControlled() && MGCharacter->bPressedJump)
		{
			MGCharacter->StopJumping();
		}
	}
}
