// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "GameFramework/PlayerState.h"
#include "MGPlayerState.generated.h"

/**
 * 
 */

class UMGExperienceDefinition;
class UMGCharacterDefinition;
class UMGAbilitySystemComponent;
class AMGPlayerController;

UCLASS(Config = Game)
class AMGPlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()
	
public:
	AMGPlayerState(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
	UFUNCTION(BlueprintCallable, Category = "MundusGranum|PlayerState")
	AMGPlayerController* GetMGPlayerController() const;
	
	UFUNCTION(BlueprintCallable, Category = "MundusGranum|PlayerState")
	UMGAbilitySystemComponent* GetMGAbilitySystemComponent() const { return AbilitySystemComponent; }
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	
	//~AActor interface
	virtual void PreInitializeComponents() override;
	virtual void PostInitializeComponents() override;
	//~End of AActor interface
	
	//~APlayerState interface
	virtual void Reset() override;
	virtual void ClientInitialize(AController* C) override;
	//~End of APlayerState interface
	
	static const FName NAME_MGAbilityReady;
	
	template <class T>
	const T* GetPawnData() const { return Cast<T>(PawnData); }
	
	void SetPawnData(const UMGCharacterDefinition* InCharacterDefinition);
	
	void SetHealthSet(const TObjectPtr<UAttributeSet>& InHealthSet){this->HealthSet = InHealthSet;}
	void SetCombatSet(const TObjectPtr<const UAttributeSet>& InCombatSet){this->CombatSet = InCombatSet;}
	[[nodiscard]] TObjectPtr<UAttributeSet> GetHealth() const{return HealthSet;}

protected:
	UFUNCTION()
	void OnRep_PawnData();
	
	UPROPERTY(ReplicatedUsing = OnRep_PawnData)
	TObjectPtr<const UMGCharacterDefinition> PawnData;
	
private:
	void OnExperienceLoaded(const UMGExperienceDefinition* CurrentExperience);
	
	// The ability system component sub-object used by player characters.
	UPROPERTY(VisibleAnywhere, Category = "MundusGranum|PlayerState")
	TObjectPtr<UMGAbilitySystemComponent> AbilitySystemComponent;

	// Health attribute set used by this actor.
	//HACK 临时取消const
	UPROPERTY()
	TObjectPtr<UAttributeSet> HealthSet;

	// Combat attribute set used by this actor.
	UPROPERTY()
	TObjectPtr<const UAttributeSet> CombatSet;
	
};