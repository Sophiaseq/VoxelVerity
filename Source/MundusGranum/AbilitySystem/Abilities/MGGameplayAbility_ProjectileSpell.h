// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MGGameplayAbility_Damage.h"
#include "MGGameplayAbility_ProjectileSpell.generated.h"

class AMGProjectile;
/**
 * 
 */
UCLASS()
class MUNDUSGRANUM_API UMGGameplayAbility_ProjectileSpell : public UMGGameplayAbility_Damage
{
	GENERATED_BODY()
	
protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	
	UFUNCTION(BlueprintCallable, Category=Projectile)
	void SpawnProjectile(const FVector& ProjectileTargetLocation);
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<AMGProjectile> ProjectileClass;
 };
