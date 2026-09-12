// Fill out your copyright notice in the Description page of Project Settings.


#include "MGPrimarySet.h"

#include "Net/UnrealNetwork.h"


void UMGPrimarySet::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME_CONDITION_NOTIFY(UMGPrimarySet, Strength, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMGPrimarySet, Dexterity, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMGPrimarySet, Constitution, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMGPrimarySet, Intelligence, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMGPrimarySet, Perception, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMGPrimarySet, Luck, COND_None, REPNOTIFY_Always);
}

void UMGPrimarySet::OnRep_Strength(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMGPrimarySet, Strength, OldValue);
}

void UMGPrimarySet::OnRep_Dexterity(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMGPrimarySet, Dexterity, OldValue);
}

void UMGPrimarySet::OnRep_Constitution(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMGPrimarySet, Constitution, OldValue);
}

void UMGPrimarySet::OnRep_Intelligence(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMGPrimarySet, Intelligence, OldValue);
}

void UMGPrimarySet::OnRep_Perception(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMGPrimarySet, Perception, OldValue);
}

void UMGPrimarySet::OnRep_Luck(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMGPrimarySet, Luck, OldValue);
}