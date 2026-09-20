// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "MGAttributeSet.h"
#include "MGCombatSet.generated.h"

struct MGDamageStatics;
/**
 * 
 */
UCLASS()
class MUNDUSGRANUM_API UMGCombatSet : public UMGAttributeSet
{
	GENERATED_BODY()
	
public:
	UMGCombatSet();
	ATTRIBUTE_ACCESSORS_BASIC(UMGCombatSet, AttackPower)
	ATTRIBUTE_ACCESSORS_BASIC(UMGCombatSet, DefensePower)
	ATTRIBUTE_ACCESSORS_BASIC(UMGCombatSet, CriticalRate)
	ATTRIBUTE_ACCESSORS_BASIC(UMGCombatSet, CriticalDamage)
	
protected:
	UFUNCTION()
	virtual void OnRep_AttackPower(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	virtual void OnRep_DefensePower(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	virtual void OnRep_CriticalRate(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	virtual void OnRep_CriticalDamage(const FGameplayAttributeData& OldValue);
	
private:
	/** 攻击力 — 影响所有伤害类技能的最终伤害值 */
	UPROPERTY(VisibleAnywhere, Category = "Combat", ReplicatedUsing = OnRep_AttackPower)
	FGameplayAttributeData AttackPower;
	
	/** 防御力 — 减少受到的物理伤害 */
	UPROPERTY(VisibleAnywhere, Category = "Combat", ReplicatedUsing = OnRep_DefensePower)
	FGameplayAttributeData DefensePower;

	/** 暴击率 — 0.0~1.0，0.05=5%暴击率 */
	UPROPERTY(VisibleAnywhere, Category = "Combat", ReplicatedUsing = OnRep_CriticalRate)
	FGameplayAttributeData CriticalRate;

	/** 暴击伤害倍率 — 1.5=暴击时造成150%伤害 */
	UPROPERTY(VisibleAnywhere, Category = "Combat", ReplicatedUsing = OnRep_CriticalDamage)
	FGameplayAttributeData CriticalDamage;
	
	friend MGDamageStatics;
};
