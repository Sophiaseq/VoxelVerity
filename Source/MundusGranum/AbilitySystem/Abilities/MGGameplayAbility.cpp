// Fill out your copyright notice in the Description page of Project Settings.


#include "MGGameplayAbility.h"

#include "Character/MGCharacter.h"

AMGCharacter* UMGGameplayAbility::GetMGCharacterFromActorInfo() const
{
	return (CurrentActorInfo ? Cast<AMGCharacter>(CurrentActorInfo->AvatarActor.Get()) : nullptr);
}
