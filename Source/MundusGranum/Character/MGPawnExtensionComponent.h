#pragma once

#include "CoreMinimal.h"
#include "Components/GameFrameworkInitStateInterface.h"
#include "Components/PawnComponent.h"
#include "MGPawnExtensionComponent.generated.h"

#define UE_API MUNDUSGRANUM_API

class UMGAbilitySystemComponent;
class UMGCharacterDefinition;

UCLASS(MinimalAPI, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class UMGPawnExtensionComponent : public UPawnComponent, public IGameFrameworkInitStateInterface
{
	GENERATED_BODY()

public:
	UE_API UMGPawnExtensionComponent(const FObjectInitializer& ObjectInitializer);
	
	static UE_API const FName NAME_ActorFeatureName;
	
	//~ Begin IGameFrameworkInitStateInterface interface
	UE_API virtual FName GetFeatureName() const override { return NAME_ActorFeatureName; }
	UE_API virtual bool CanChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) const override;
	UE_API virtual void HandleChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) override;
	UE_API virtual void OnActorInitStateChanged(const FActorInitStateChangedParams& Params) override;
	UE_API virtual void CheckDefaultInitialization() override;
	//~ End IGameFrameworkInitStateInterface interface

	/** Returns the pawn extension component if one exists on the specified actor. */
	UFUNCTION(BlueprintPure, Category = "MundusGranum|Pawn")
	static UMGPawnExtensionComponent* FindPawnExtensionComponent(const AActor* Actor) { return (Actor ? Actor->FindComponentByClass<UMGPawnExtensionComponent>() : nullptr); }

	/** Gets the pawn data, which is used to specify pawn properties in data */
	template <class T>
	const T* GetPawnData() const { return Cast<T>(PawnData); }

	/** Sets the current pawn data */
	UE_API void SetPawnData(const UMGCharacterDefinition* InPawnData);
	
	UFUNCTION(BlueprintPure, Category = "Lyra|Pawn")
	UMGAbilitySystemComponent* GetMGAbilitySystemComponent() const { return AbilitySystemComponent; }
	
	UE_API void InitializeAbilitySystem(UMGAbilitySystemComponent* InASC, AActor* InOwnerActor);

private:
	UPROPERTY(EditAnywhere, Category = "MundusGranum|Pawn")
	TObjectPtr<const UMGCharacterDefinition> PawnData;
	
	UPROPERTY(Transient)
	TObjectPtr<UMGAbilitySystemComponent> AbilitySystemComponent;
	
};

#undef UE_API