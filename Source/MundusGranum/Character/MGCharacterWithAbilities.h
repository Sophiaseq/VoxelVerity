// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "MGCharacter.h"
#include "AI/MGAIController.h"
#include "UI/Widget/HealthBarComponent.h"
#include "MGCharacterWithAbilities.generated.h"

class UBehaviorTree;
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
	virtual const UMGWeaponItemDefinition* GetCurrentWeapon() const override;
	//~End ICombatInterface
	
	void SetPawnData();
	
protected:
	//~Begin AController
	virtual void PossessedBy(AController* NewController) override;
	//~End AController
	
	virtual void HitReactTagChanged(const FGameplayTag CallbackTag, int32 NewCount) override;
	
private:
	// The ability system component sub-object used by player characters.
	UPROPERTY(VisibleAnywhere, Category = "MundusGranum|NonPlayerState")
	TObjectPtr<UMGAbilitySystemComponent> AbilitySystemComponent;
	
	UPROPERTY(EditAnywhere)
	TObjectPtr<UHealthBarComponent> HealthBarComponent;
	
	UPROPERTY(VisibleAnywhere, Category = "MundusGranum|NonPlayerState")
	float NonPlayerLevel = 1;
	
	UPROPERTY(EditAnywhere, Category = "MundusGranum|AI")
	TObjectPtr<UBehaviorTree> BehaviorTree;
	
	UPROPERTY()
	TObjectPtr<AMGAIController> MGAIController;
	
	UPROPERTY()
	TObjectPtr<const UMGWeaponItemDefinition> WeaponDef;
	
public:
	float GetNonPlayerLevel() const {return NonPlayerLevel;}
	void SetWeaponDef(const UMGWeaponItemDefinition* WeaponItemDefinition) { WeaponDef = WeaponItemDefinition;}
};
