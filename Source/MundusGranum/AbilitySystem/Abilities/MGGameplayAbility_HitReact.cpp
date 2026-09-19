// Fill out your copyright notice in the Description page of Project Settings.


#include "MGGameplayAbility_HitReact.h"

#include "MundusGranumGameplayTags.h"
#include "Interaction/CombatInterface.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(MGGameplayAbility_HitReact)

UMGGameplayAbility_HitReact::UMGGameplayAbility_HitReact(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
	
	if (HasAnyFlags(RF_ClassDefaultObject))
	{
		FAbilityTriggerData Trigger;
		Trigger.TriggerTag = MundusGranumGameplayTags::GameplayEvent_HitReact;
		Trigger.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
		AbilityTriggers.Add(Trigger);
	}
}

void UMGGameplayAbility_HitReact::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	if (ICombatInterface* CombatActor = Cast<ICombatInterface>(GetAvatarActorFromActorInfo()))
	{
		HitReactMontage = CombatActor->GetHitReactMontage();
	}
}

void UMGGameplayAbility_HitReact::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	HitReactMontage = nullptr;
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

FName UMGGameplayAbility_HitReact::GetSecNameByHitResult(FGameplayTagContainer TargetTags)
{
	if (TargetTags.HasTag(MundusGranumGameplayTags::HitReact_Direction_Front))
	{
		return TEXT("Front");
	}
	if (TargetTags.HasTag(MundusGranumGameplayTags::HitReact_Direction_Back))
	{
		return TEXT("Back");
	}
	if (TargetTags.HasTag(MundusGranumGameplayTags::HitReact_Direction_Left))
	{
		return TEXT("Left");
	}
	if (TargetTags.HasTag(MundusGranumGameplayTags::HitReact_Direction_Right))
	{
		return TEXT("Right");
	}

	return TEXT("Hit");
}
