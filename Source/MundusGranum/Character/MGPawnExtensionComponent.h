#pragma once

#include "CoreMinimal.h"
#include "Components/GameFrameworkInitStateInterface.h"
#include "Components/PawnComponent.h"
#include "MGPawnExtensionComponent.generated.h"



class UMGAbilitySystemComponent;
class UMGCharacterDefinition;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class UMGPawnExtensionComponent : public UPawnComponent, public IGameFrameworkInitStateInterface
{
	GENERATED_BODY()

public:
	UMGPawnExtensionComponent(const FObjectInitializer& ObjectInitializer);
	
	static const FName NAME_ActorFeatureName;
	
	//~ Begin IGameFrameworkInitStateInterface interface
	virtual FName GetFeatureName() const override { return NAME_ActorFeatureName; }
	virtual bool CanChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) const override;
	virtual void HandleChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) override;
	virtual void OnActorInitStateChanged(const FActorInitStateChangedParams& Params) override;
	virtual void CheckDefaultInitialization() override;
	//~ End IGameFrameworkInitStateInterface interface
	
	void SetupPlayerInputComponent();
	
	/** Should be called by the owning pawn when the player state has been replicated. */
	void HandlePlayerStateReplicated();

	/** Returns the pawn extension component if one exists on the specified actor. */
	UFUNCTION(BlueprintPure, Category = "MundusGranum|Pawn")
	static UMGPawnExtensionComponent* FindPawnExtensionComponent(const AActor* Actor) { return (Actor ? Actor->FindComponentByClass<UMGPawnExtensionComponent>() : nullptr); }

	/** Gets the pawn data, which is used to specify pawn properties in data */
	template <class T>
	const T* GetPawnData() const { return Cast<T>(PawnData); }

	/** Sets the current pawn data */
	void SetPawnData(const UMGCharacterDefinition* InPawnData);
	
	UFUNCTION(BlueprintPure, Category = "Lyra|Pawn")
	UMGAbilitySystemComponent* GetMGAbilitySystemComponent() const { return AbilitySystemComponent; }
	
	void InitializeAbilitySystem(UMGAbilitySystemComponent* InASC, AActor* InOwnerActor);
	
	/** Should be called by the owning pawn to remove itself as the avatar of the ability system. */
	void UninitializeAbilitySystem();
	
	/** Should be called by the owning pawn when the pawn's controller changes. */
	void HandleControllerChanged();
	
	/** Register with the OnAbilitySystemInitialized delegate and broadcast if our pawn has been registered with the ability system component */
	void OnAbilitySystemInitialized_RegisterAndCall(FSimpleMulticastDelegate::FDelegate Delegate);

	/** Register with the OnAbilitySystemUninitialized delegate fired when our pawn is removed as the ability system's avatar actor */
	void OnAbilitySystemUninitialized_Register(FSimpleMulticastDelegate::FDelegate Delegate);

protected:
	virtual void OnRegister() override;
	virtual void BeginPlay() override;
	
	UFUNCTION()
	void OnRep_PawnData();
	
	/** Delegate fired when our pawn becomes the ability system's avatar actor */
	FSimpleMulticastDelegate OnAbilitySystemInitialized;

	/** Delegate fired when our pawn is removed as the ability system's avatar actor */
	FSimpleMulticastDelegate OnAbilitySystemUninitialized;
	
	UPROPERTY(EditInstanceOnly, ReplicatedUsing = OnRep_PawnData, Category = "MundusGranum|Pawn")
	TObjectPtr<const UMGCharacterDefinition> PawnData;
	
	UPROPERTY(Transient)
	TObjectPtr<UMGAbilitySystemComponent> AbilitySystemComponent;
	
};