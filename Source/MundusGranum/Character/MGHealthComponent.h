// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/GameFrameworkComponent.h"
#include "MGHealthComponent.generated.h"

struct FGameplayEffectSpec;
class UMGHealthSet;
class UMGAbilitySystemComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGHealth_DeathEvent, AActor*, OwningActor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FMGHealth_AttributeChanged, UMGHealthComponent*, HealthComponent, float,
                                              OldValue, float, NewValue, AActor*, Instigator);

UENUM(BlueprintType)
enum class EMGDeathState : uint8
{
	NotDead = 0,
	DeathStarted,
	DeathFinished
};

UCLASS(Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MUNDUSGRANUM_API UMGHealthComponent : public UGameFrameworkComponent
{
	GENERATED_BODY()

public:
	UMGHealthComponent(const FObjectInitializer& ObjectInitializer);
	
	// Returns the health component if one exists on the specified actor.
	UFUNCTION(BlueprintPure, Category = "MundusGranum|Health")
	static UMGHealthComponent* FindHealthComponent(const AActor* Actor) { return (Actor ? Actor->FindComponentByClass<UMGHealthComponent>() : nullptr); }
	
	UFUNCTION(BlueprintCallable, Category = "MundusGranum|Health")
	void InitializeWithAbilitySystem(UMGAbilitySystemComponent* InASC);
	
	UFUNCTION(BlueprintCallable, Category = "MundusGranum|Health")
	void UninitializeFromAbilitySystem();

	UFUNCTION(BlueprintPure, Category = "MundusGranum|Health")
	float GetHealth() const;

	UFUNCTION(BlueprintPure, Category = "MundusGranum|Health")
	float GetMaxHealth() const;
	
	UFUNCTION(BlueprintCallable, Category = "MundusGranum|Health")
	EMGDeathState GetDeathState() const { return DeathState; }

	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "MundusGranum|Health", Meta = (ExpandBoolAsExecs = "ReturnValue"))
	bool IsDeadOrDying() const { return (DeathState > EMGDeathState::NotDead); }
	
	// Begins the death sequence for the owner.
	virtual void StartDeath();

	// Ends the death sequence for the owner.
	virtual void FinishDeath();

	// Delegate fired when the health value has changed. This is called on the client but the instigator may not be valid
	UPROPERTY(BlueprintAssignable)
	FMGHealth_AttributeChanged OnHealthChanged;

	// Delegate fired when the max health value has changed. This is called on the client but the instigator may not be valid
	UPROPERTY(BlueprintAssignable)
	FMGHealth_AttributeChanged OnMaxHealthChanged;
	
	UPROPERTY(BlueprintAssignable)
	FMGHealth_AttributeChanged OnStaminaChanged;
	
	UPROPERTY(BlueprintAssignable)
	FMGHealth_AttributeChanged OnMaxStaminaChanged;
	
	// Delegate fired when the death sequence has started.
	UPROPERTY(BlueprintAssignable)
	FMGHealth_DeathEvent OnDeathStarted;

	// Delegate fired when the death sequence has finished.
	UPROPERTY(BlueprintAssignable)
	FMGHealth_DeathEvent OnDeathFinished;
	
protected:
	virtual void OnUnregister() override;
	
	virtual void HandleHealthChanged(AActor* DamageInstigator, AActor* DamageCauser, const FGameplayEffectSpec* DamageEffectSpec, float DamageMagnitude, float OldValue, float NewValue);
	virtual void HandleMaxHealthChanged(AActor* DamageInstigator, AActor* DamageCauser, const FGameplayEffectSpec* DamageEffectSpec, float DamageMagnitude, float OldValue, float NewValue);
	virtual void HandleOutOfHealth(AActor* DamageInstigator, AActor* DamageCauser, const FGameplayEffectSpec* DamageEffectSpec, float DamageMagnitude, float OldValue, float NewValue);
	virtual void HandleStaminaChanged(AActor* DamageInstigator, AActor* DamageCauser, const FGameplayEffectSpec* DamageEffectSpec, float DamageMagnitude, float OldValue, float NewValue);
	virtual void HandleMaxStaminaChanged(AActor* DamageInstigator, AActor* DamageCauser, const FGameplayEffectSpec* DamageEffectSpec, float DamageMagnitude, float OldValue, float NewValue);
	virtual void HandleOutOfStamina(AActor* DamageInstigator, AActor* DamageCauser, const FGameplayEffectSpec* DamageEffectSpec, float DamageMagnitude, float OldValue, float NewValue);
	
	void ClearGameplayTags();
	
	UFUNCTION()
	virtual void OnRep_DeathState(EMGDeathState OldDeathState);

	// Ability system used by this component.
	UPROPERTY()
	TObjectPtr<UMGAbilitySystemComponent> AbilitySystemComponent;

	// Health set used by this component.
	UPROPERTY()
	TObjectPtr<const UMGHealthSet> HealthSet;
	
	// Replicated state used to handle dying.
	UPROPERTY(ReplicatedUsing = OnRep_DeathState)
	EMGDeathState DeathState;
};
