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
	ATTRIBUTE_ACCESSORS_BASIC(UMGCombatSet, ElementResistance)
	ATTRIBUTE_ACCESSORS_BASIC(UMGCombatSet, FireElementResistance)
	ATTRIBUTE_ACCESSORS_BASIC(UMGCombatSet, LightningElementResistance)
	ATTRIBUTE_ACCESSORS_BASIC(UMGCombatSet, PhysicalResistance)
	
protected:
	UFUNCTION()
	virtual void OnRep_AttackPower(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	virtual void OnRep_DefensePower(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	virtual void OnRep_CriticalRate(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	virtual void OnRep_CriticalDamage(const FGameplayAttributeData& OldValue);
	
	UFUNCTION()
	virtual void OnRep_ElementResistance(const FGameplayAttributeData& OldValue);
	
	UFUNCTION()
	virtual void OnRep_FireElementResistance(const FGameplayAttributeData& OldValue);
	
	UFUNCTION()
	virtual void OnRep_LightningElementResistance(const FGameplayAttributeData& OldValue);
	
	UFUNCTION()
	virtual void OnRep_PhysicalResistance(const FGameplayAttributeData& OldValue);
	
private:
	UPROPERTY(VisibleAnywhere, Category = "Combat", ReplicatedUsing = OnRep_AttackPower)
	FGameplayAttributeData AttackPower;
	
	UPROPERTY(VisibleAnywhere, Category = "Combat", ReplicatedUsing = OnRep_DefensePower)
	FGameplayAttributeData DefensePower;
	
	UPROPERTY(VisibleAnywhere, Category = "Combat", ReplicatedUsing = OnRep_CriticalRate)
	FGameplayAttributeData CriticalRate;
	
	UPROPERTY(VisibleAnywhere, Category = "Combat", ReplicatedUsing = OnRep_CriticalDamage)
	FGameplayAttributeData CriticalDamage;
	
	UPROPERTY(VisibleAnywhere, Category = "Combat", ReplicatedUsing = OnRep_ElementResistance)
	FGameplayAttributeData ElementResistance;
	
	UPROPERTY(VisibleAnywhere, Category = "Combat", ReplicatedUsing = OnRep_FireElementResistance)
	FGameplayAttributeData FireElementResistance;
	
	UPROPERTY(VisibleAnywhere, Category = "Combat", ReplicatedUsing = OnRep_LightningElementResistance)
	FGameplayAttributeData LightningElementResistance;
	
	UPROPERTY(VisibleAnywhere, Category = "Combat", ReplicatedUsing = OnRep_PhysicalResistance)
	FGameplayAttributeData PhysicalResistance;
	
	friend MGDamageStatics;
};
