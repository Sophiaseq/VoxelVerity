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
	ATTRIBUTE_ACCESSORS_BASIC(UMGHealthSet, Health)  // 自动生成Getter/Setter/Init函数
	ATTRIBUTE_ACCESSORS_BASIC(UMGHealthSet, MaxHealth)
	ATTRIBUTE_ACCESSORS_BASIC(UMGHealthSet, Mana)
	ATTRIBUTE_ACCESSORS_BASIC(UMGHealthSet, MaxMana)
	ATTRIBUTE_ACCESSORS_BASIC(UMGHealthSet, Stamina)
	ATTRIBUTE_ACCESSORS_BASIC(UMGHealthSet, MaxStamina)
	ATTRIBUTE_ACCESSORS_BASIC(UMGHealthSet, IncomingDamage)
	ATTRIBUTE_ACCESSORS_BASIC(UMGHealthSet, IncomingHealing)
	
protected:
	// 属性修改前回调 —— 用于Clamp（限制值域）
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute,float& NewValue) override;

	// GE执行后回调 —— 用于处理"死亡的连锁反应"等
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;

	// 属性应用后回调（所有GE计算完成后）
	virtual void PreAttributeBaseChange(const FGameplayAttribute& Attribute,float& NewValue) const override;

	virtual void PostAttributeChange(const FGameplayAttribute& Attribute,float OldValue,float NewValue) override;
	
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
	/** 当前生命值 — 降到0时角色死亡 */
	UPROPERTY(VisibleAnywhere, Category = "Vital|Health", ReplicatedUsing = OnRep_Health)
	FGameplayAttributeData Health;
	
	/** 最大生命值 — 由装备、等级、Buff等影响 */
	UPROPERTY(VisibleAnywhere, Category = "Vital|Health", ReplicatedUsing = OnRep_MaxHealth)
	FGameplayAttributeData MaxHealth;
	
	/** 当前法力值 — 施放技能消耗法力 */
	UPROPERTY(VisibleAnywhere, Category = "Vital|Mana", ReplicatedUsing = OnRep_Mana)
	FGameplayAttributeData Mana;
	
	/** 最大法力值 */
	UPROPERTY(VisibleAnywhere, Category = "Vital|Mana", ReplicatedUsing = OnRep_MaxMana)
	FGameplayAttributeData MaxMana;
	
	/** 当前体力值 — 闪避、冲刺消耗体力 */
	UPROPERTY(VisibleAnywhere, Category = "Vital|Stamina", ReplicatedUsing = OnRep_Stamina)
	FGameplayAttributeData Stamina;
	
	/** 最大体力值 */
	UPROPERTY(VisibleAnywhere, Category = "Vital|Stamina", ReplicatedUsing = OnRep_MaxStamina)
	FGameplayAttributeData MaxStamina;
		
	UPROPERTY(VisibleAnywhere, Category = "Meta")
	FGameplayAttributeData IncomingDamage;
	
	/** 受到的最终治疗值（临时属性） */
	UPROPERTY(VisibleAnywhere, Category = "Meta")
	FGameplayAttributeData IncomingHealing;
};
