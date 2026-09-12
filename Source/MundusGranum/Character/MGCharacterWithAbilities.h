// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MGCharacter.h"
#include "MGCharacterWithAbilities.generated.h"

class UMGHealthSet;
class UMGCombatSet;
class UAbilitySystemComponent;
class UMGAbilitySystemComponent;

UCLASS(Blueprintable)
class MUNDUSGRANUM_API AMGCharacterWithAbilities : public AMGCharacter
{
	GENERATED_BODY()

public:
	AMGCharacterWithAbilities();
	virtual void PostInitializeComponents() override;
	
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	void SetHealthSet(const TObjectPtr<const UMGHealthSet>& InHealthSet){this->HealthSet = InHealthSet;}
	void SetCombatSet(const TObjectPtr<const UMGCombatSet>& InCombatSet){this->CombatSet = InCombatSet;}
	float GetNonPlayerLevel() const {return NonPlayerLevel;}
	
	//~Begin ICombatInterface
	virtual float GetCharacterLevel() override;
	//~End ICombatInterface
	
private:
	// The ability system component sub-object used by player characters.
	UPROPERTY(VisibleAnywhere, Category = "MundusGranum|NonPlayerState")
	TObjectPtr<UMGAbilitySystemComponent> AbilitySystemComponent;
	
	// Health attribute set used by this actor.
	UPROPERTY()
	TObjectPtr<const UMGHealthSet> HealthSet;

	// Combat attribute set used by this actor.
	UPROPERTY()
	TObjectPtr<const UMGCombatSet> CombatSet;
	
	UPROPERTY(VisibleAnywhere, Category = "MundusGranum|NonPlayerState")
	float NonPlayerLevel = 1;
	
};
