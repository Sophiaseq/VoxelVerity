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
	
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	
	//~Begin ICombatInterface
	virtual float GetCharacterLevel() override;
	virtual const UMGWeaponItemDefinition* GetCurrentWeapon() const override;
	//~End ICombatInterface
	
	void SetPawnData();
	
protected:
	virtual void OnAbilitySystemInitialized() override;
	virtual void PostInitializeComponents() override;
	virtual void BeginPlay() override;
	
	//~Begin APawn
	virtual void OnRep_Controller() override;
	//~End APawn
	
	//~Begin AController
	virtual void PossessedBy(AController* NewController) override;
	//~End AController
	
	UFUNCTION()
	void OnRep_WeaponDef();
	
	virtual void HitReactTagChanged(const FGameplayTag CallbackTag, int32 NewCount) override;
	
	void PostReplicatedPawnData();
	
private:
	UPROPERTY(VisibleAnywhere, Category = "MundusGranum|NonPlayerState")
	TObjectPtr<UMGAbilitySystemComponent> AbilitySystemComponent;
	
	UPROPERTY(EditAnywhere)
	TObjectPtr<UHealthBarComponent> HealthBarComponent;
	
	UPROPERTY()
	TObjectPtr<UMGHealthSet> HealthSet;
	
	UPROPERTY()
	TObjectPtr<UMGCombatSet> CombatSet;
	
	UPROPERTY(VisibleAnywhere, Category = "MundusGranum|NonPlayerState")
	float NonPlayerLevel = 1;
	
	UPROPERTY(EditAnywhere, Category = "MundusGranum|AI")
	TObjectPtr<UBehaviorTree> BehaviorTree;
	
	UPROPERTY()
	TObjectPtr<AMGAIController> MGAIController;
	
	UPROPERTY(ReplicatedUsing=OnRep_WeaponDef)
	TObjectPtr<const UMGWeaponItemDefinition> WeaponDef;
	
public:
	float GetNonPlayerLevel() const {return NonPlayerLevel;}
	void SetWeaponDef(const UMGWeaponItemDefinition* WeaponItemDefinition) { WeaponDef = WeaponItemDefinition;}
};
