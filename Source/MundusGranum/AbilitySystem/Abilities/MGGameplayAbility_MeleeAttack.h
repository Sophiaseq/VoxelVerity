// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MGGameplayAbility.h"
#include "Items/Weapons/MeleeCombos.h"
#include "MGGameplayAbility_MeleeAttack.generated.h"

class UAbilityTask_PlayMontageAndWait;
class UMGWeaponItemDefinition;

/*
 *
 */
UCLASS()
class MUNDUSGRANUM_API UMGGameplayAbility_MeleeAttack : public UMGGameplayAbility
{
	GENERATED_BODY()

public:
	UMGGameplayAbility_MeleeAttack();

	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void InputPressed(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
	void PlaySection(const FMeleeComboSection& Section);
	
	UFUNCTION(BlueprintCallable)
	bool TryAdvanceCombo();
	
	TArray<FGameplayTag> MeleeInputTags;
	
	UPROPERTY(Transient);
	TObjectPtr<UMeleeCombos> MeleeCombos;
	
	UPROPERTY(Transient)
	TObjectPtr<UAnimMontage> AttackMontage;
	
	UPROPERTY(Transient)
	TObjectPtr<UAbilityTask_PlayMontageAndWait> MontageTask;
	
	FMeleeComboSection CurrentSection;

	UFUNCTION()
	void HandleMontageBlendOut();

	UFUNCTION()
	void HandleMontageCompleted();

	UFUNCTION()
	void HandleMontageInterrupted();

	UFUNCTION()
	void HandleMontageCancelled();
};
