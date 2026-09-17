// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/GameFrameworkComponent.h"
#include "MGHealthComponent.generated.h"

struct FGameplayEffectSpec;
class UMGHealthSet;
class UMGAbilitySystemComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FMGHealth_AttributeChanged, UMGHealthComponent*, HealthComponent, float,
                                              OldValue, float, NewValue, AActor*, Instigator);

UCLASS(Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MUNDUSGRANUM_API UMGHealthComponent : public UGameFrameworkComponent
{
	GENERATED_BODY()

public:
	UMGHealthComponent(const FObjectInitializer& ObjectInitializer);
	
	UFUNCTION(BlueprintCallable, Category = "MundusGranum|Health")
	void InitializeWithAbilitySystem(UMGAbilitySystemComponent* InASC);
	
	UFUNCTION(BlueprintCallable, Category = "MundusGranum|Health")
	void UninitializeFromAbilitySystem();

	UFUNCTION(BlueprintPure, Category = "MundusGranum|Health")
	float GetHealth() const;

	UFUNCTION(BlueprintPure, Category = "MundusGranum|Health")
	float GetMaxHealth() const;

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
	
protected:
	virtual void OnUnregister() override;
	
	virtual void HandleHealthChanged(AActor* DamageInstigator, AActor* DamageCauser, const FGameplayEffectSpec* DamageEffectSpec, float DamageMagnitude, float OldValue, float NewValue);
	virtual void HandleMaxHealthChanged(AActor* DamageInstigator, AActor* DamageCauser, const FGameplayEffectSpec* DamageEffectSpec, float DamageMagnitude, float OldValue, float NewValue);
	virtual void HandleOutOfHealth(AActor* DamageInstigator, AActor* DamageCauser, const FGameplayEffectSpec* DamageEffectSpec, float DamageMagnitude, float OldValue, float NewValue);
	virtual void HandleStaminaChanged(AActor* DamageInstigator, AActor* DamageCauser, const FGameplayEffectSpec* DamageEffectSpec, float DamageMagnitude, float OldValue, float NewValue);
	virtual void HandleMaxStaminaChanged(AActor* DamageInstigator, AActor* DamageCauser, const FGameplayEffectSpec* DamageEffectSpec, float DamageMagnitude, float OldValue, float NewValue);
	virtual void HandleOutOfStamina(AActor* DamageInstigator, AActor* DamageCauser, const FGameplayEffectSpec* DamageEffectSpec, float DamageMagnitude, float OldValue, float NewValue);
	
	void ClearGameplayTags();

	// Ability system used by this component.
	UPROPERTY()
	TObjectPtr<UMGAbilitySystemComponent> AbilitySystemComponent;

	// Health set used by this component.
	UPROPERTY()
	TObjectPtr<const UMGHealthSet> HealthSet;
};
