// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "MGPlayerAttributeSet.h"
#include "ModularPlayerState.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "Interaction/CombatInterface.h"
#include "System/GameplayTagStack.h"
#include "MGPlayerState.generated.h"

/**
 * 
 */
template<typename T>
using TAttributeFuncPtr = TBaseStaticDelegateInstance<T, FDefaultDelegateUserPolicy>::FFuncPtr;

class UMGExperienceDefinition;
class UMGPawnData;
class UMGAbilitySystemComponent;
class AMGPlayerController;

UCLASS(Config = Game)
class AMGPlayerState : public AModularPlayerState, public IAbilitySystemInterface, public ICombatInterface
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
	
	virtual float GetCharacterLevel() override { return PlayerLevel; };
	
	//~APlayerState interface
	virtual void Reset() override;
	virtual void ClientInitialize(AController* C) override;
	//~End of APlayerState interface
	
	static const FName NAME_MGAbilityReady;
	
	template <class T>
	const T* GetPawnData() const { return Cast<T>(PawnData); }
	
	void SetPawnData(const UMGPawnData* InCharacterDefinition);
	
	FORCEINLINE [[nodiscard]] float GetPlayerLevel() const { return PlayerLevel; }
	
	//属性标签对应的Get函数，可以移到Experience？
	TMap<FGameplayTag, TAttributeFuncPtr<FGameplayAttribute()>> TagsToAttributes;
	
	// Adds a specified number of stacks to the tag (does nothing if StackCount is below 1)
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category=Teams)
	void AddStatTagStack(FGameplayTag Tag, int32 StackCount);

	// Removes a specified number of stacks from the tag (does nothing if StackCount is below 1)
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category=Teams)
	void RemoveStatTagStack(FGameplayTag Tag, int32 StackCount);

	// Returns the stack count of the specified tag (or 0 if the tag is not present)
	UFUNCTION(BlueprintCallable, Category=Teams)
	int32 GetStatTagStackCount(FGameplayTag Tag) const;

	// Returns true if there is at least one stack of the specified tag
	UFUNCTION(BlueprintCallable, Category=Teams)
	bool HasStatTag(FGameplayTag Tag) const;


protected:
	UFUNCTION()
	void OnRep_PawnData();
	
	UPROPERTY(ReplicatedUsing = OnRep_PawnData)
	TObjectPtr<const UMGPawnData> PawnData;
	
private:
	void OnExperienceLoaded(const UMGExperienceDefinition* CurrentExperience);
	
	// The ability system component sub-object used by player characters.
	UPROPERTY(VisibleAnywhere, Category = "MundusGranum|PlayerState")
	TObjectPtr<UMGAbilitySystemComponent> AbilitySystemComponent;

	//所有的属性
	UPROPERTY()
	FMGPlayerAttributeSet PlayerAttributes;
	
	UPROPERTY(Replicated)
	FGameplayTagStackContainer StatTags;
	
	UPROPERTY(VisibleAnywhere, Category = "MundusGranum|PlayerState")
	float PlayerLevel = 1;
	
public:
	[[nodiscard]] FMGPlayerAttributeSet GetPlayerAttributes() const { return PlayerAttributes; }
	void SetPrimarySet(const TObjectPtr<const UAttributeSet>& InPrimarySet){this->PlayerAttributes.PrimarySet = InPrimarySet;}
	void SetHealthSet(const TObjectPtr<const UAttributeSet>& InHealthSet){this->PlayerAttributes.HealthSet = InHealthSet;}
	void SetCombatSet(const TObjectPtr<const UAttributeSet>& InCombatSet){this->PlayerAttributes.CombatSet = InCombatSet;}
	[[nodiscard]] TObjectPtr<const UAttributeSet> GetHealthSet() const { return PlayerAttributes.HealthSet; }
	[[nodiscard]] TObjectPtr<const UAttributeSet> GetPrimarySet() const { return PlayerAttributes.PrimarySet; }
	[[nodiscard]] TObjectPtr<const UAttributeSet> GetCombatSet() const { return PlayerAttributes.CombatSet; }
	
};