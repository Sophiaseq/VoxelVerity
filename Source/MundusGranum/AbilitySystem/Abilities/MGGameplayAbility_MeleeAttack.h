// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MGGameplayAbility.h"
#include "MGGameplayAbility_MeleeAttack.generated.h"

/**
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
	
	
	/*// 连招窗口事件
	UFUNCTION()
	void OnComboWindowEvent(FGameplayTag EventTag, FGameplayEventData EventData);

	// 命中窗口事件
	UFUNCTION()
	void OnHitWindowEvent(FGameplayTag EventTag, FGameplayEventData EventData);

	// 蒙太奇结束
	UFUNCTION()
	void OnMontageCompleted();

	UFUNCTION()
	void OnMontageInterrupted();

	// 执行近战检测
	void PerformMeleeTrace();

	// 应用伤害
	void ApplyDamageToTarget(AActor* TargetActor, const FHitResult& Hit);*/
	
protected:
	UPROPERTY(BlueprintReadOnly, Category = "Melee")
	FGameplayTagContainer MeleeInputTags;

};
