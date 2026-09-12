// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "MGGameplayAbility.generated.h"

class AMGCharacter;
/**
 * 
 */
UCLASS()
class MUNDUSGRANUM_API UMGGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Category = "MundusGranum|Ability")
	AMGCharacter* GetMGCharacterFromActorInfo() const;
};
