// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameplayModMagnitudeCalculation.h"
#include "MMC_MaxHealth.generated.h"

/**
 * 
 */
UCLASS()
class MUNDUSGRANUM_API UMMC_MaxHealth : public UGameplayModMagnitudeCalculation
{
	GENERATED_BODY()
	
public:
	UMMC_MaxHealth();
	
	float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;
	
protected:
	
private:
	
	FGameplayEffectAttributeCaptureDefinition ConstitutionDef;
};
