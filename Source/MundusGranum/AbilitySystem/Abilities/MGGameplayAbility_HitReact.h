// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MGGameplayAbility.h"
#include "MGGameplayAbility_HitReact.generated.h"

/**
 * 
 */
UCLASS(Abstract)
class MUNDUSGRANUM_API UMGGameplayAbility_HitReact : public UMGGameplayAbility
{
	GENERATED_BODY()
	
public:
	UMGGameplayAbility_HitReact(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	
	UFUNCTION(BlueprintPure)
	FName GetSecNameByHitResult(FGameplayTagContainer TargetTags);

	UPROPERTY(BlueprintReadOnly, Transient)
	TSoftObjectPtr<UAnimMontage> HitReactMontage;
};
