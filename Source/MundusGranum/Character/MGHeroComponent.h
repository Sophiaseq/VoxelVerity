#pragma once

#include "CoreMinimal.h"
#include "Components/GameFrameworkInitStateInterface.h"
#include "Components/PawnComponent.h"
#include "MGHeroComponent.generated.h"

struct FInputMappingContextAndPriority;
class UMGInputConfig;
struct FInputActionValue;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class UMGHeroComponent : public UPawnComponent, public IGameFrameworkInitStateInterface
{
	GENERATED_BODY()

public:
	UMGHeroComponent(const FObjectInitializer& ObjectInitializer);
	
	/** Adds mode-specific input config */
	void AddAdditionalInputConfig(const UMGInputConfig* InputConfig);

	/** Removes a mode-specific input config if it has been added */
	void RemoveAdditionalInputConfig(const UMGInputConfig* InputConfig);

	/** True if this is controlled by a real player and has progressed far enough in initialization where additional input bindings can be added */
	bool IsReadyToBindInputs() const;

	/** The name of the extension event sent via UGameFrameworkComponentManager when ability inputs are ready to bind */
	static const FName NAME_BindInputsNow;

	/** The name of this component-implemented feature */
	static const FName NAME_ActorFeatureName;
 
	//~ Begin IGameFrameworkInitStateInterface interface
	virtual FName GetFeatureName() const override { return NAME_ActorFeatureName; }
	virtual bool CanChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) const override;
	virtual void HandleChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) override;
	virtual void OnActorInitStateChanged(const FActorInitStateChangedParams& Params) override;
	virtual void CheckDefaultInitialization() override;
	//~ End IGameFrameworkInitStateInterface interface

protected:
	virtual void OnRegister() override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	void InitializePlayerInput(UInputComponent* PlayerInputComponent);
	
	void Input_Move(const FInputActionValue& Value);
	void Input_LookMouse(const FInputActionValue& Value);
	void Input_Jump();
	void Input_Pickup();
	void Input_SprintPressed();
	void Input_SprintReleased();
	void Input_UseLeftHandItem();
	void Input_UseRightHandItem();
	void Input_SelectItem(const FInputActionValue& Value);
	void Input_SlowWalk();
	
	UPROPERTY(EditAnywhere)
	TArray<FInputMappingContextAndPriority> DefaultInputMappings;
	
	/** True when player input bindings have been applied, will never be true for non - players */
	bool bReadyToBindInputs;
private:	
};