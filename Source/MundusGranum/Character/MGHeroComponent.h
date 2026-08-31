#pragma once

#include "CoreMinimal.h"
#include "Components/GameFrameworkInitStateInterface.h"
#include "Components/PawnComponent.h"
#include "MGHeroComponent.generated.h"

#define UE_API MUNDUSGRANUM_API

struct FInputMappingContextAndPriority;
class UMGInputConfig;
struct FInputActionValue;

UCLASS(MinimalAPI, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class UMGHeroComponent : public UPawnComponent, public IGameFrameworkInitStateInterface
{
	GENERATED_BODY()

public:
	UE_API UMGHeroComponent(const FObjectInitializer& ObjectInitializer);
	
	/** Adds mode-specific input config */
	UE_API void AddAdditionalInputConfig(const UMGInputConfig* InputConfig);

	/** Removes a mode-specific input config if it has been added */
	UE_API void RemoveAdditionalInputConfig(const UMGInputConfig* InputConfig);

	/** True if this is controlled by a real player and has progressed far enough in initialization where additional input bindings can be added */
	UE_API bool IsReadyToBindInputs() const;

	/** The name of the extension event sent via UGameFrameworkComponentManager when ability inputs are ready to bind */
	static UE_API const FName NAME_BindInputsNow;

	/** The name of this component-implemented feature */
	static UE_API const FName NAME_ActorFeatureName;
 
	//~ Begin IGameFrameworkInitStateInterface interface
	UE_API virtual FName GetFeatureName() const override { return NAME_ActorFeatureName; }
	UE_API virtual bool CanChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) const override;
	UE_API virtual void HandleChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) override;
	UE_API virtual void OnActorInitStateChanged(const FActorInitStateChangedParams& Params) override;
	UE_API virtual void CheckDefaultInitialization() override;
	//~ End IGameFrameworkInitStateInterface interface

protected:
	UE_API virtual void BeginPlay() override;
	UE_API void InitializePlayerInput(UInputComponent* PlayerInputComponent);
	
	UE_API void Input_Move(const FInputActionValue& Value);
	UE_API void Input_LookMouse(const FInputActionValue& Value);
	UE_API void Input_Jump();
	UE_API void Input_Pickup();
	UE_API void Input_SprintPressed();
	UE_API void Input_SprintReleased();
	UE_API void Input_UseLeftHandItem();
	UE_API void Input_UseRightHandItem();
	UE_API void Input_SelectItem(const FInputActionValue& Value);
	UE_API void Input_SlowWalk();
	
	UPROPERTY(EditAnywhere)
	TArray<FInputMappingContextAndPriority> DefaultInputMappings;
	
	/** True when player input bindings have been applied, will never be true for non - players */
	bool bReadyToBindInputs;
private:	
};

#undef UE_API