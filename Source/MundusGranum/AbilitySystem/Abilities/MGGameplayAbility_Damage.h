// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MGGameplayAbility.h"
#include "MGGameplayAbility_Damage.generated.h"

/**
 * 
 */
UCLASS()
class MUNDUSGRANUM_API UMGGameplayAbility_Damage : public UMGGameplayAbility
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable)
	void CauseDamage(AActor* TargetActor);
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<UGameplayEffect> DamageEffectClass;
	
	UPROPERTY(EditDefaultsOnly, Category=Damage)
	TMap<FGameplayTag, FScalableFloat> DamageTypes;
};
