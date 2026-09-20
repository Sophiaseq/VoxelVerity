// Fill out your copyright notice in the Description page of Project Settings.


#include "MGAbilitySystemGlobals.h"

#include "MGGameplayEffectContext.h"

FGameplayEffectContext* UMGAbilitySystemGlobals::AllocGameplayEffectContext() const
{
	return new FMGGameplayEffectContext();
}
