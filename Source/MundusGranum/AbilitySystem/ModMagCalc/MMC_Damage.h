// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayModMagnitudeCalculation.h"
#include "MMC_Damage.generated.h"

/**
 * 
 */
UCLASS()
class MUNDUSGRANUM_API UMMC_Damage : public UGameplayModMagnitudeCalculation
{
	GENERATED_BODY()
	
public:
	UMMC_Damage();
	
	float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;
	
private:
	FGameplayEffectAttributeCaptureDefinition WeaponAttackDef;
	FGameplayEffectAttributeCaptureDefinition DefenseDef;
};
