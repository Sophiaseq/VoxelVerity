// Fill out your copyright notice in the Description page of Project Settings.


#include "MMC_MaxHealth.h"

#include "AbilitySystem/Attributes/MGPrimarySet.h"
#include "Interaction/CombatInterface.h"

UMMC_MaxHealth::UMMC_MaxHealth()
{
	ConstitutionDef.AttributeSource = EGameplayEffectAttributeCaptureSource::Target;
	ConstitutionDef.AttributeToCapture = UMGPrimarySet::GetConstitutionAttribute();
	ConstitutionDef.bSnapshot = false;
	
	RelevantAttributesToCapture.Add(ConstitutionDef);
}

float UMMC_MaxHealth::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	//Gather tags from source and target
	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();
	
	FAggregatorEvaluateParameters EvaluateParameters;
	EvaluateParameters.SourceTags = SourceTags;
	EvaluateParameters.TargetTags = TargetTags;
	
	float Constitution = 0.0f;
	GetCapturedAttributeMagnitude(ConstitutionDef, Spec, EvaluateParameters, Constitution);
	Constitution = FMath::Max(Constitution, 0.0f);
	
	/*
	 *MGPlayerState中的SetPawnData调用AbilitySet->GiveToAbilitySystem(AbilitySystemComponent, nullptr)，
	 *此时InstigatorAbilitySystemComponent的Owner为MGPlayerState，但AvatarActor为null,只能从PlayerState中获取MGCharacterLevel
	 *这样让MGPlayerState也继承了ICombatInterface
	 */
	//TODO: 或许有不需要MGPlayerState继承ICombatInterface的方法
	float Level = 0.0f;
	if (ICombatInterface* CombatInterface = Cast<ICombatInterface>(Spec.GetContext().GetInstigator()))
	{
		Level = CombatInterface->GetCharacterLevel();
	}
	return Constitution*10 + Level*10;
}
