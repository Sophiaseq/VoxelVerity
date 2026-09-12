// Fill out your copyright notice in the Description page of Project Settings.


#include "MGGameplayAbility_MeleeAttack.h"

#include "Character/MGCharacter.h"

UMGGameplayAbility_MeleeAttack::UMGGameplayAbility_MeleeAttack()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

bool UMGGameplayAbility_MeleeAttack::CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags,
	const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!ActorInfo || !ActorInfo->AvatarActor.IsValid())
	{
		return false;
	}

	const AMGCharacter* MGCharacter = Cast<AMGCharacter>(ActorInfo->AvatarActor.Get());
	if (!MGCharacter)
	{
		return false;
	}

	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	return true;
}

void UMGGameplayAbility_MeleeAttack::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	MeleeInputTags.Reset();

	if (const FGameplayAbilitySpec* Spec = GetCurrentAbilitySpec())
		MeleeInputTags.AppendTags(Spec->GetDynamicSpecSourceTags());

	//上面的替代方案，只有通过 SendGameplayEventToActor 触发能力时，TriggerEventData 才不为空
	/*if (TriggerEventData)
		MeleeInputTags.AppendTags(TriggerEventData->InstigatorTags);*/
}

