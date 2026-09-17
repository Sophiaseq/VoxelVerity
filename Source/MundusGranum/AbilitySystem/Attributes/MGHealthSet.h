// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "MGAttributeSet.h"
#include "MGHealthSet.generated.h"

/**
 * 
 */
UCLASS()
class MUNDUSGRANUM_API UMGHealthSet : public UMGAttributeSet
{
	GENERATED_BODY()
	
public:
	UMGHealthSet();
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
	// ========== 生命/法力/体力（ ==========
	ATTRIBUTE_ACCESSORS_BASIC(UMGHealthSet, Health)
	ATTRIBUTE_ACCESSORS_BASIC(UMGHealthSet, MaxHealth)
	ATTRIBUTE_ACCESSORS_BASIC(UMGHealthSet, Mana)
	ATTRIBUTE_ACCESSORS_BASIC(UMGHealthSet, MaxMana)
	ATTRIBUTE_ACCESSORS_BASIC(UMGHealthSet, Stamina)
	ATTRIBUTE_ACCESSORS_BASIC(UMGHealthSet, MaxStamina)
	ATTRIBUTE_ACCESSORS_BASIC(UMGHealthSet, IncomingDamage)
	ATTRIBUTE_ACCESSORS_BASIC(UMGHealthSet, IncomingHealing)
	
	// Delegate when health changes due to damage/healing, some information may be missing on the client
	mutable FMGAttributeEvent OnHealthChanged;

	// Delegate when max health changes
	mutable FMGAttributeEvent OnMaxHealthChanged;

	// Delegate to broadcast when the health attribute reaches zero
	mutable FMGAttributeEvent OnOutOfHealth;
	
	mutable FMGAttributeEvent OnStaminaChanged;
	mutable FMGAttributeEvent OnMaxStaminaChanged;
	mutable FMGAttributeEvent OnOutOfStamina;

protected:
	// 属性修改前回调 —— 用于Clamp（限制值域）
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;

	virtual bool PreGameplayEffectExecute(struct FGameplayEffectModCallbackData& Data) override;
	// GE执行后回调 —— 用于处理"死亡的连锁反应"等
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;

	// 属性应用后回调（所有GE计算完成后）
	virtual void PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const override;

	virtual void PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) override;
	
	void ResetMetaAttributes();
	
	/** 钳制主要属性：确保当前值不超过最大值，且不低于0 */
	static void ClampVitalAttribute(const FGameplayAttribute& Attribute, float& NewValue,
		const FGameplayAttributeData& MaxValueAttribute);
	
	UFUNCTION()
	virtual void OnRep_Health(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	virtual void OnRep_MaxHealth(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	virtual void OnRep_Mana(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	virtual void OnRep_MaxMana(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	virtual void OnRep_Stamina(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	virtual void OnRep_MaxStamina(const FGameplayAttributeData& OldValue);
	
private:
	UPROPERTY(BlueprintReadOnly, Category = "Vital|Health", ReplicatedUsing = OnRep_Health, meta = (AllowPrivateAccess))
	FGameplayAttributeData Health;
	
	UPROPERTY(BlueprintReadOnly, Category = "Vital|Health", ReplicatedUsing = OnRep_MaxHealth, meta = (AllowPrivateAccess))
	FGameplayAttributeData MaxHealth;
	
	UPROPERTY(BlueprintReadOnly, Category = "Vital|Mana", ReplicatedUsing = OnRep_Mana, meta = (AllowPrivateAccess))
	FGameplayAttributeData Mana;
	
	UPROPERTY(BlueprintReadOnly, Category = "Vital|Mana", ReplicatedUsing = OnRep_MaxMana, meta = (AllowPrivateAccess))
	FGameplayAttributeData MaxMana;
	
	UPROPERTY(BlueprintReadOnly, Category = "Vital|Stamina", ReplicatedUsing = OnRep_Stamina, meta = (AllowPrivateAccess))
	FGameplayAttributeData Stamina;
	
	UPROPERTY(BlueprintReadOnly, Category = "Vital|Stamina", ReplicatedUsing = OnRep_MaxStamina, meta = (AllowPrivateAccess))
	FGameplayAttributeData MaxStamina;
	
	// Used to track when the health reaches 0.
	bool bOutOfHealth;

	// Store the health before any changes 
	float MaxHealthBeforeAttributeChange;
	float HealthBeforeAttributeChange;
	
	bool bOutOfStamina;

	// Store the health before any changes 
	float MaxStaminaBeforeAttributeChange;
	float StaminaBeforeAttributeChange;
		
	UPROPERTY(BlueprintReadOnly, Category = "Meta", meta = (AllowPrivateAccess))
	FGameplayAttributeData IncomingDamage;
	
	UPROPERTY(BlueprintReadOnly, Category = "Meta", meta = (AllowPrivateAccess))
	FGameplayAttributeData IncomingHealing;
};