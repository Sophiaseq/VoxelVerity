// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "MGGameplayAbility.generated.h"

class UMGAbilitySystemComponent;
class AMGPlayerController;
class UMGHeroComponent;
class AMGCharacter;

/**
 * EMGAbilityActivationGroup
 *
 *	Defines how an ability activates in relation to other abilities.
 */
UENUM(BlueprintType)
enum class EMGAbilityActivationGroup : uint8
{
	// Ability runs independently of all other abilities.
	Independent,

	// Ability is canceled and replaced by other exclusive abilities.
	Exclusive_Replaceable,

	// Ability blocks all other exclusive abilities from activating.
	Exclusive_Blocking,

	MAX	UMETA(Hidden)
};

/**
 * 
 */
UENUM(BlueprintType)
enum class EMGAbilityActivationPolicy : uint8
{
	// Try to activate the ability when the input is triggered.
	OnInputTriggered,

	// Continually try to activate the ability while the input is active.
	WhileInputActive,

	// Try to activate the ability when an avatar is assigned.
	OnSpawn
};

UCLASS(Abstract)
class MUNDUSGRANUM_API UMGGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()
	
public:
	UMGGameplayAbility(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
	UFUNCTION(BlueprintCallable, Category = "MundusGranum|Ability")
	UMGAbilitySystemComponent* GetMGAbilitySystemComponentFromActorInfo() const;

	UFUNCTION(BlueprintCallable, Category = "MundusGranum|Ability")
	AMGPlayerController* GetMGPlayerControllerFromActorInfo() const;

	UFUNCTION(BlueprintCallable, Category = "MundusGranum|Ability")
	AController* GetControllerFromActorInfo() const;

	UFUNCTION(BlueprintCallable, Category = "MundusGranum|Ability")
	AMGCharacter* GetMGCharacterFromActorInfo() const;

	UFUNCTION(BlueprintCallable, Category = "MundusGranum|Ability")
	UMGHeroComponent* GetHeroComponentFromActorInfo() const;
	
	EMGAbilityActivationPolicy GetActivationPolicy() const { return ActivationPolicy; }
	
	void TryActivateAbilityOnSpawn(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) const;
	
protected:
	// Defines how this ability is meant to activate.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MG|Ability Activation")
	EMGAbilityActivationPolicy ActivationPolicy;
};
