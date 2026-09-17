// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "MGCharacter.h"
#include "UI/UI_MVVM/HealthBarComponent.h"
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
	virtual void BeginPlay() override;
	
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	
	//~Begin ICombatInterface
	virtual float GetCharacterLevel() override;
	//~End ICombatInterface
	
	void SetPawnData() const;
	
private:
	// The ability system component sub-object used by player characters.
	UPROPERTY(VisibleAnywhere, Category = "MundusGranum|NonPlayerState")
	TObjectPtr<UMGAbilitySystemComponent> AbilitySystemComponent;
	
	UPROPERTY(EditAnywhere)
	TObjectPtr<UHealthBarComponent> HealthBarComponent;
	
	UPROPERTY(VisibleAnywhere, Category = "MundusGranum|NonPlayerState")
	float NonPlayerLevel = 1;
	
public:
	float GetNonPlayerLevel() const {return NonPlayerLevel;}
};
