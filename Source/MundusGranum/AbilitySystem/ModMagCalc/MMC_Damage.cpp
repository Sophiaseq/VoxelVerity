// Fill out your copyright notice in the Description page of Project Settings.


#include "MMC_Damage.h"

#include "AbilitySystem/Attributes/MGCombatSet.h"

UMMC_Damage::UMMC_Damage()
{
	WeaponAttackDef.AttributeSource = EGameplayEffectAttributeCaptureSource::Source;
	WeaponAttackDef.AttributeToCapture = UMGCombatSet::GetAttackPowerAttribute();
	WeaponAttackDef.bSnapshot = false;
	RelevantAttributesToCapture.Add(WeaponAttackDef);
	
	DefenseDef.AttributeSource = EGameplayEffectAttributeCaptureSource::Target;
	DefenseDef.AttributeToCapture = UMGCombatSet::GetDefensePowerAttribute();
	DefenseDef.bSnapshot = false;
	RelevantAttributesToCapture.Add(DefenseDef);
}

float UMMC_Damage::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();
	
	FAggregatorEvaluateParameters EvaluateParameters;
	EvaluateParameters.SourceTags = SourceTags;
	EvaluateParameters.TargetTags = TargetTags;
	
	float AttackPower = 0.0f;
	float Defense = 0.0f;
	GetCapturedAttributeMagnitude(WeaponAttackDef, Spec, EvaluateParameters, AttackPower);
	GetCapturedAttributeMagnitude(DefenseDef, Spec, EvaluateParameters, Defense);
	AttackPower = FMath::Max(AttackPower, 0.0f);
	Defense = FMath::Max(Defense, 0.0f);

	const float Sharpness =  Spec.GetSetByCallerMagnitude("Damage.Melee.Sharpness");
	const float Quality =  Spec.GetSetByCallerMagnitude("Damage.Melee.Quality");

	// 遍历所有类型的伤害
	
	const float X = FMath::Clamp(Defense-Sharpness, 1, Defense-Sharpness);
	
	return AttackPower*Quality / X;
}
