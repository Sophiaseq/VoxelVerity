// Fill out your copyright notice in the Description page of Project Settings.


#include "MGInputConfig.h"

#include "MGLogChannels.h"

UMGInputConfig::UMGInputConfig(const FObjectInitializer& ObjectInitializer)
{
}

const UInputAction* UMGInputConfig::FindAbilityInputActionForTag(const FGameplayTag& InputTag, bool bLogNotFound) const
{
	for (const FMGInputAction& Action : AbilityInputActions)
	{
		if (Action.InputAction && (Action.InputTag == InputTag))
		{
			return Action.InputAction;
		}
	}

	if (bLogNotFound)
	{
		UE_LOG(LogMG, Error, TEXT("Can't find AbilityInputAction for InputTag [%s] on InputConfig [%s]."), *InputTag.ToString(), *GetNameSafe(this));
	}

	return nullptr;
}

const UInputAction* UMGInputConfig::FindActionByTag(const FGameplayTag& InputTag, bool bLogMissing) const
{
	for (const FMGInputAction& TaggedInputAction : TaggedInputActions)
	{
		if (TaggedInputAction.InputAction && TaggedInputAction.InputTag == InputTag)
		{
			return TaggedInputAction.InputAction;
		}
	}
	if (bLogMissing)
	{
		UE_LOG(LogMG, Error, TEXT("InputConfig 缺失Tag: %s"), *InputTag.ToString());
	}
	return nullptr;
}
