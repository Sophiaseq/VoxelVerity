// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "GameFramework/PlayerState.h"
#include "MGPlayerState.generated.h"

#define UE_API MUNDUSGRANUM_API

class UMGCharacterDefinition;
class UMGAbilitySystemComponent;
class UAbilitySystemComponent;
class AMGPlayerController;
/**
 * 
 */

UCLASS(MinimalAPI, Config = Game)
class AMGPlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()
	
public:
	UE_API AMGPlayerState(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
	UFUNCTION(BlueprintCallable, Category = "MundusGranum|PlayerState")
	UE_API AMGPlayerController* GetMGPlayerController() const;
	
	UFUNCTION(BlueprintCallable, Category = "MundusGranum|PlayerState")
	UMGAbilitySystemComponent* GetMGAbilitySystemComponent() const { return AbilitySystemComponent; }
	UE_API virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	
protected:
	UPROPERTY(VisibleAnywhere, Category = "MundusGranum|PlayerState")
	TObjectPtr<UMGAbilitySystemComponent> AbilitySystemComponent;
	
	UPROPERTY(VisibleAnywhere, Category = "MundusGranum|PlayerState")
	TObjectPtr<const UMGCharacterDefinition> PawnData;
};

#undef UE_API