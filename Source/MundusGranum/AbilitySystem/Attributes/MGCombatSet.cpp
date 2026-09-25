// Fill out your copyright notice in the Description page of Project Settings.


#include "MGCombatSet.h"

#include "Net/UnrealNetwork.h"

UMGCombatSet::UMGCombatSet()
{
}

void UMGCombatSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UMGCombatSet, AttackPower, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMGCombatSet, DefensePower, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMGCombatSet, CriticalRate, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMGCombatSet, CriticalDamage, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMGCombatSet, ElementResistance, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMGCombatSet, FireElementResistance, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMGCombatSet, LightningElementResistance, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMGCombatSet, PhysicalResistance, COND_None, REPNOTIFY_Always);
}

void UMGCombatSet::OnRep_AttackPower(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMGCombatSet, AttackPower, OldValue);
}

void UMGCombatSet::OnRep_DefensePower(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMGCombatSet, DefensePower, OldValue);
}

void UMGCombatSet::OnRep_CriticalRate(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMGCombatSet, CriticalRate, OldValue);
}

void UMGCombatSet::OnRep_CriticalDamage(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMGCombatSet, CriticalDamage, OldValue);
}

void UMGCombatSet::OnRep_ElementResistance(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMGCombatSet, ElementResistance, OldValue);
}

void UMGCombatSet::OnRep_FireElementResistance(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMGCombatSet, FireElementResistance, OldValue);
}

void UMGCombatSet::OnRep_LightningElementResistance(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMGCombatSet, LightningElementResistance, OldValue);
}

void UMGCombatSet::OnRep_PhysicalResistance(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMGCombatSet, PhysicalResistance, OldValue);
}
