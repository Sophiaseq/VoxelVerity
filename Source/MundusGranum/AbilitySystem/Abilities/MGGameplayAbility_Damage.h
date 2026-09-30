// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MGGameplayAbility.h"
#include "Items/Weapons/MGWeaponItemDefinition.h"
#include "MGGameplayAbility_Damage.generated.h"

/**
 * 
 */
UCLASS()
class MUNDUSGRANUM_API UMGGameplayAbility_Damage : public UMGGameplayAbility
{
	GENERATED_BODY()
	
public:
	UMGGameplayAbility_Damage();
	
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	
	UFUNCTION(BlueprintCallable)
	void CauseDamage(AActor* TargetActor) const;
	
protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<UGameplayEffect> DamageEffectClass;
	
	UPROPERTY(EditDefaultsOnly, Category=Damage)
	TMap<FGameplayTag, FScalableFloat> DamageTypes;
	
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UAnimMontage> AttackMontage;
	
	UPROPERTY(BlueprintReadOnly)
	FWeaponAttributes WeaponAttributes;
};
