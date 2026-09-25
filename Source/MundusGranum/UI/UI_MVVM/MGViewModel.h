// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MVVMViewModelBase.h"

#include "MGViewModel.generated.h"

class UAbilitySystemComponent;
class UMGHealthSet;
/**
 * 
 */
UCLASS()
class MUNDUSGRANUM_API UMGViewModel : public UMVVMViewModelBase
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Category = "MVVM")
	void Initialize(AActor* Owner);
	
	UFUNCTION(BlueprintSetter)
	void SetHealth(float InHealth);
	
	UFUNCTION(BlueprintSetter)
	void SetMaxHealth(float InMaxHealth);
	
	UFUNCTION(BlueprintSetter)
	void SetStamina(float InHealth);
	
	UFUNCTION(BlueprintSetter)
	void SetMaxStamina(float InMaxHealth);
	
	float GetHealth() const { return Health; }
	float GetMaxHealth() const { return MaxHealth; }
	float GetStamina() const { return Stamina; }
	float GetMaxStamina() const { return MaxStamina; }
	
    UFUNCTION(BlueprintPure, FieldNotify)
	float GetStaminaPercent() const;
	
	UFUNCTION(BlueprintPure, FieldNotify)
	float GetHealthPercent() const;
	
protected:
	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter, Setter, meta = (AllowPrivateAccess = "true"))
	float Health = 0.f;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter, Setter, meta = (AllowPrivateAccess = "true"))
	float MaxHealth = 0.f;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter, Setter, meta = (AllowPrivateAccess = "true"))
	float Stamina = 0.f;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter, Setter, meta = (AllowPrivateAccess = "true"))
	float MaxStamina = 0.f;

private:
	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> ASC;

	UPROPERTY()
	TObjectPtr<const UMGHealthSet> AttributeSet;
	
	void OnHealthChanged(const struct FOnAttributeChangeData& Data);
	void OnMaxHealthChanged(const struct FOnAttributeChangeData& Data);
	void OnStaminaChanged(const struct FOnAttributeChangeData& Data);
	void OnMaxStaminaChanged(const struct FOnAttributeChangeData& Data);
};
