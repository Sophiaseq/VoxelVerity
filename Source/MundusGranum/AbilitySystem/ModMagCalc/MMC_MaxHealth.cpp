// Fill out your copyright notice in the Description page of Project Settings.


#include "MMC_MaxHealth.h"

#include "AbilitySystem/Attributes/MGHealthSet.h"
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
	
	ICombatInterface* CombatInterface = Cast<ICombatInterface>(Spec.GetContext().GetSourceObject());
	return Constitution*10 + CombatInterface->GetCharacterLevel()*10;
}
