// Fill out your copyright notice in the Description page of Project Settings.


#include "MGGameplayAbility_Damage.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Interaction/CombatInterface.h"

UMGGameplayAbility_Damage::UMGGameplayAbility_Damage()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

void UMGGameplayAbility_Damage::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                                const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                                const FGameplayEventData* TriggerEventData)
{
	ICombatInterface* CombatInterface = Cast<ICombatInterface>(GetAvatarActorFromActorInfo());
	const UMGWeaponItemDefinition* WeaponDef = CombatInterface ? CombatInterface->GetCurrentWeapon() : nullptr;
	
	AttackMontage = WeaponDef->Montage;
	WeaponAttributes = WeaponDef->WeaponAttributes;
	
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

bool UMGGameplayAbility_Damage::CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                                   const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags,
                                                   const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	ICombatInterface* CombatInterface = Cast<ICombatInterface>(GetAvatarActorFromActorInfo());
	const UMGWeaponItemDefinition* WeaponDef = CombatInterface ? CombatInterface->GetCurrentWeapon() : nullptr;
	if (!WeaponDef || !WeaponDef->Montage) return false;

	return Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags);
}

void UMGGameplayAbility_Damage::CauseDamage(AActor* TargetActor) const
{
	UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo();
	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
	
	FGameplayEffectSpecHandle DamageSpecHandle = MakeOutgoingGameplayEffectSpec(DamageEffectClass, GetAbilityLevel());
	FGameplayEffectSpec* Spec = DamageSpecHandle.Data.Get();
	
	for (const FGameplayModifierInfo& Mod : Spec->Def->Modifiers)
	{
		if (Mod.ModifierMagnitude.GetMagnitudeCalculationType() == EGameplayEffectMagnitudeCalculation::SetByCaller)
		{
			FGameplayTag SetByCallerTag = Mod.ModifierMagnitude.GetSetByCallerFloat().DataTag;
			if (SetByCallerTag.IsValid())
			{
				Spec->SetSetByCallerMagnitude(SetByCallerTag, 0.0f);
			}
		}
	}

	DamageSpecHandle.Data->SetSetByCallerMagnitude("Damage.Melee.Sharpness", WeaponAttributes.Sharpness);
	DamageSpecHandle.Data->SetSetByCallerMagnitude("Damage.Melee.Quality", WeaponAttributes.Quality);
	for (const TTuple<FGameplayTag, FScalableFloat>& Pair : DamageTypes)
	{
		const float ScaledDamage = Pair.Value.GetValueAtLevel(GetAbilityLevel());
		UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(DamageSpecHandle, Pair.Key, ScaledDamage);
	}
	SourceASC->ApplyGameplayEffectSpecToTarget(*DamageSpecHandle.Data.Get(), TargetASC);
}
