#include "MGHeroComponent.h"
#include "MGLogChannels.h"
#include "EnhancedInputSubsystems.h"
#include "MGCharacterDefinition.h"
#include "MGPawnExtensionComponent.h"
#include "MundusGranumGameplayTags.h"
#include "Components/GameFrameworkComponentDelegates.h"
#include "Components/GameFrameworkComponentManager.h"
#include "GameFeatures/GameFeatureAction_AddInputContextMapping.h"
#include "Input/MGInputComponent.h"
#include "Player/MGPlayerController.h"
#include "Player/MGPlayerState.h"
#include "UserSettings/EnhancedInputUserSettings.h"
#include "InputMappingContext.h"
#include "AbilitySystem/MGAbilitySystemComponent.h"
#include "Components/MGEquipmentComponent.h"
#include "Components/MGInventoryComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Misc/UObjectToken.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(MGHeroComponent)

const FName UMGHeroComponent::NAME_BindInputsNow("BindInputsNow");
const FName UMGHeroComponent::NAME_ActorFeatureName("Hero");

UMGHeroComponent::UMGHeroComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bReadyToBindInputs = false;
}

void UMGHeroComponent::BeginPlay()
{
	Super::BeginPlay();
	
	// Listen for when the pawn extension component changes init state
	BindOnActorInitStateChanged(UMGPawnExtensionComponent::NAME_ActorFeatureName, FGameplayTag(), false);

	// Notifies that we are done spawning, then try the rest of initialization
	ensure(TryToChangeInitState(MundusGranumGameplayTags::InitState_Spawned));
	CheckDefaultInitialization();
}

void UMGHeroComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UnregisterInitStateFeature();
	
	Super::EndPlay(EndPlayReason);
}

bool UMGHeroComponent::CanChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) const
{
	check(Manager);

	APawn* Pawn = GetPawn<APawn>();

	if (!CurrentState.IsValid() && DesiredState == MundusGranumGameplayTags::InitState_Spawned)
	{
		// As long as we have a real pawn, let us transition
		if (Pawn)
		{
			return true;
		}
	}
	else if (CurrentState == MundusGranumGameplayTags::InitState_Spawned && DesiredState == MundusGranumGameplayTags::InitState_DataAvailable)
	{
		// The player state is required.
		if (!GetPlayerState<AMGPlayerState>())
		{
			return false;
		}

		// If we're authority or autonomous, we need to wait for a controller with registered ownership of the player state.
		if (Pawn->GetLocalRole() != ROLE_SimulatedProxy)
		{
			AController* Controller = GetController<AController>();

			const bool bHasControllerPairedWithPS = (Controller != nullptr) && \
				(Controller->PlayerState != nullptr) && \
				(Controller->PlayerState->GetOwner() == Controller);

			if (!bHasControllerPairedWithPS)
			{
				return false;
			}
		}

		const bool bIsLocallyControlled = Pawn->IsLocallyControlled();
		const bool bIsBot = Pawn->IsBotControlled();

		if (bIsLocallyControlled && !bIsBot)
		{
			AMGPlayerController* MGPC = GetController<AMGPlayerController>();

			// The input component and local player is required when locally controlled.
			if (!Pawn->InputComponent || !MGPC || !MGPC->GetLocalPlayer())
			{
				return false;
			}
		}

		return true;
	}
	else if (CurrentState == MundusGranumGameplayTags::InitState_DataAvailable && DesiredState == MundusGranumGameplayTags::InitState_DataInitialized)
	{
		// Wait for player state and extension component
		AMGPlayerState* MGPS = GetPlayerState<AMGPlayerState>();

		return MGPS && Manager->HasFeatureReachedInitState(Pawn, UMGPawnExtensionComponent::NAME_ActorFeatureName, MundusGranumGameplayTags::InitState_DataInitialized);
	}
	else if (CurrentState == MundusGranumGameplayTags::InitState_DataInitialized && DesiredState == MundusGranumGameplayTags::InitState_GameplayReady)
	{
		// TODO add ability initialization checks?
		return true;
	}

	return false;
}

void UMGHeroComponent::HandleChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState)
{
	UE_LOG(LogMG, Warning, TEXT("[Init] Hero HandleChangeInitState: %s → %s"), *CurrentState.ToString(), *DesiredState.ToString());
	if (CurrentState == MundusGranumGameplayTags::InitState_DataAvailable && DesiredState == MundusGranumGameplayTags::InitState_DataInitialized)
	{
		APawn* Pawn = GetPawn<APawn>();
		AMGPlayerState* MGPS = GetPlayerState<AMGPlayerState>();
		if (!ensure(Pawn && MGPS))
		{
			return;
		}

		const UMGCharacterDefinition* PawnData = nullptr;

		if (UMGPawnExtensionComponent* PawnExtComp = UMGPawnExtensionComponent::FindPawnExtensionComponent(Pawn))
		{
			PawnData = PawnExtComp->GetPawnData<UMGCharacterDefinition>();

			// The player state holds the persistent data for this player (state that persists across deaths and multiple pawns).
			// The ability system component and attribute sets live on the player state.
			PawnExtComp->InitializeAbilitySystem(MGPS->GetMGAbilitySystemComponent(), MGPS);
		}

		if (AMGPlayerController* MGPC = GetController<AMGPlayerController>())
		{
			if (Pawn->InputComponent != nullptr)
			{
				InitializePlayerInput(Pawn->InputComponent);
			}
		}

		// Hook up the delegate for all pawns, in case we spectate later
		/*if (PawnData)
		{
			if (UMGCameraComponent* CameraComponent = UMGCameraComponent::FindCameraComponent(Pawn))
			{
				CameraComponent->DetermineCameraModeDelegate.BindUObject(this, &ThisClass::DetermineCameraMode);
			}
		}*/
	}
}

void UMGHeroComponent::OnActorInitStateChanged(const FActorInitStateChangedParams& Params)
{
	if (Params.FeatureName == UMGPawnExtensionComponent::NAME_ActorFeatureName)
	{
		if (Params.FeatureState == MundusGranumGameplayTags::InitState_DataInitialized)
		{
			// If the extension component says all all other components are initialized, try to progress to next state
			CheckDefaultInitialization();
		}
	}
}

void UMGHeroComponent::CheckDefaultInitialization()
{
	static const TArray<FGameplayTag> StateChain = { MundusGranumGameplayTags::InitState_Spawned, MundusGranumGameplayTags::InitState_DataAvailable, MundusGranumGameplayTags::InitState_DataInitialized, MundusGranumGameplayTags::InitState_GameplayReady };

	// This will try to progress from spawned (which is only set in BeginPlay) through the data initialization stages until it gets to gameplay ready
	
	ContinueInitStateChain(StateChain);
}

void UMGHeroComponent::OnRegister()
{
	Super::OnRegister();
	if (!GetPawn<APawn>())
	{
		UE_LOG(LogTemp, Error, TEXT("[UMGHeroComponent::OnRegister] This component has been added to a blueprint whose base class is not a Pawn. To use this component, it MUST be placed on a Pawn Blueprint."));

#if WITH_EDITOR
		if (GIsEditor)
		{
			static const FText Message = NSLOCTEXT("MGHeroComponent", "NotOnPawnError", "has been added to a blueprint whose base class is not a Pawn. To use this component, it MUST be placed on a Pawn Blueprint. This will cause a crash if you PIE!");
			static const FName HeroMessageLogName = TEXT("MGHeroComponent");
			
			FMessageLog(HeroMessageLogName).Error()
				->AddToken(FUObjectToken::Create(this, FText::FromString(GetNameSafe(this))))
				->AddToken(FTextToken::Create(Message));
				
			FMessageLog(HeroMessageLogName).Open();
		}
#endif
	}
	else
	{
		// Register with the init state system early, this will only work if this is a game world
		RegisterInitStateFeature();
	}
}

void UMGHeroComponent::AddAdditionalInputConfig(const UMGInputConfig* InputConfig)
{
	TArray<uint32> BindHandles;

	const APawn* Pawn = GetPawn<APawn>();
	if (!Pawn)
	{
		return;
	}
	
	const APlayerController* PC = GetController<APlayerController>();
	check(PC);

	const ULocalPlayer* LP = PC->GetLocalPlayer();
	check(LP);

	UEnhancedInputLocalPlayerSubsystem* Subsystem = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	check(Subsystem);

	if (const UMGPawnExtensionComponent* PawnExtComp = UMGPawnExtensionComponent::FindPawnExtensionComponent(Pawn))
	{
		UMGInputComponent* MGIC = Pawn->FindComponentByClass<UMGInputComponent>();
		if (ensureMsgf(MGIC, TEXT("Unexpected Input Component class! The Gameplay Abilities will not be bound to their inputs. Change the input component to UMGInputComponent or a subclass of it.")))
		{
			//TODO
			//MGIC->BindAbilityActions(InputConfig, this, &ThisClass::Input_AbilityInputTagPressed, &ThisClass::Input_AbilityInputTagReleased, /*out*/ BindHandles);
		}
	}
}

void UMGHeroComponent::RemoveAdditionalInputConfig(const UMGInputConfig* InputConfig)
{
	//@TODO: Implement me!
}

bool UMGHeroComponent::IsReadyToBindInputs() const
{
	return bReadyToBindInputs;
}

void UMGHeroComponent::InitializePlayerInput(UInputComponent* PlayerInputComponent)
{
	check(PlayerInputComponent);

	const APawn* Pawn = GetPawn<APawn>();
	if (!Pawn)
	{
		return;
	}

	const APlayerController* PC = GetController<APlayerController>();
	check(PC);

	//TODO UMGLocalPlayer
	const ULocalPlayer* LP = Cast<ULocalPlayer>(PC->GetLocalPlayer());
	check(LP);

	UEnhancedInputLocalPlayerSubsystem* Subsystem = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	check(Subsystem);

	Subsystem->ClearAllMappings();

	if (const UMGPawnExtensionComponent* PawnExtComp = UMGPawnExtensionComponent::FindPawnExtensionComponent(Pawn))
	{
		if (const UMGCharacterDefinition* PawnData = PawnExtComp->GetPawnData<UMGCharacterDefinition>())
		{
			if (const UMGInputConfig* InputConfig = PawnData->InputConfig)
			{
				for (const FInputMappingContextAndPriority& Mapping : DefaultInputMappings)
				{
					if (UInputMappingContext* IMC = Mapping.InputMapping.LoadSynchronous())
					{
						if (Mapping.bRegisterWithSettings)
						{
							if (UEnhancedInputUserSettings* Settings = Subsystem->GetUserSettings())
							{
								Settings->RegisterInputMappingContext(IMC);
							}
							
							FModifyContextOptions Options = {};
							Options.bIgnoreAllPressedKeysUntilRelease = false;
							// Actually add the config to the local player							
							Subsystem->AddMappingContext(IMC, Mapping.Priority, Options);
						}
					}
				}

				// The MG Input Component has some additional functions to map Gameplay Tags to an Input Action.
				// If you want this functionality but still want to change your input component class, make it a subclass
				// of the UMGInputComponent or modify this component accordingly.
				UMGInputComponent* MGIC = Cast<UMGInputComponent>(PlayerInputComponent);
				if (ensureMsgf(MGIC, TEXT("Unexpected Input Component class! The Gameplay Abilities will not be bound to their inputs. Change the input component to UMGInputComponent or a subclass of it.")))
				{
					// Add the key mappings that may have been set by the player
					MGIC->AddInputMappings(InputConfig, Subsystem);

					// This is where we actually bind and input action to a gameplay tag, which means that Gameplay Ability Blueprints will
					// be triggered directly by these input actions Triggered events. 
					TArray<uint32> BindHandles;
					MGIC->BindAbilityActions(InputConfig, this, &ThisClass::Input_AbilityInputTagPressed, &ThisClass::Input_AbilityInputTagReleased, /*out*/ BindHandles);

					MGIC->BindNativeAction(InputConfig, MundusGranumGameplayTags::InputTag_Move, ETriggerEvent::Triggered, this, &ThisClass::Input_Move, /*bLogIfNotFound=*/ false);
					MGIC->BindNativeAction(InputConfig, MundusGranumGameplayTags::InputTag_Look_Mouse, ETriggerEvent::Triggered, this, &ThisClass::Input_LookMouse, /*bLogIfNotFound=*/ false);
					MGIC->BindNativeAction(InputConfig, MundusGranumGameplayTags::InputTag_Sprint, ETriggerEvent::Started, this, &ThisClass::Input_SprintPressed, /*bLogIfNotFound=*/ false);
					MGIC->BindNativeAction(InputConfig, MundusGranumGameplayTags::InputTag_Sprint, ETriggerEvent::Completed, this, &ThisClass::Input_SprintReleased, /*bLogIfNotFound=*/ false);
					MGIC->BindNativeAction(InputConfig, MundusGranumGameplayTags::InputTag_SlowWalk, ETriggerEvent::Triggered, this, &ThisClass::Input_SlowWalk, /*bLogIfNotFound=*/ false);
					MGIC->BindNativeAction(InputConfig, MundusGranumGameplayTags::InputTag_SelectItem, ETriggerEvent::Triggered, this, &ThisClass::InputTag_SelectItem, /*bLogIfNotFound=*/ false);
				}
			}
		}
	}
	UE_LOG(LogMG, Warning, TEXT("[Init] Hero InitializePlayerInput → 广播 BindInputsNow"));

	if (ensure(!bReadyToBindInputs))
	{
		bReadyToBindInputs = true;
	}
 
	UGameFrameworkComponentManager::SendGameFrameworkComponentExtensionEvent(const_cast<APlayerController*>(PC), NAME_BindInputsNow);
	UGameFrameworkComponentManager::SendGameFrameworkComponentExtensionEvent(const_cast<APawn*>(Pawn), NAME_BindInputsNow);
}

void UMGHeroComponent::Input_AbilityInputTagPressed(FGameplayTag InputTag)
{
	if (const APawn* Pawn = GetPawn<APawn>())
	{
		if (const UMGPawnExtensionComponent* PawnExtComp = UMGPawnExtensionComponent::FindPawnExtensionComponent(Pawn))
		{
			if (UMGAbilitySystemComponent* MGASC = PawnExtComp->GetMGAbilitySystemComponent())
			{
				MGASC->AbilityInputTagPressed(InputTag);
			}
		}	
	}
}

void UMGHeroComponent::Input_AbilityInputTagReleased(FGameplayTag InputTag)
{
	const APawn* Pawn = GetPawn<APawn>();
	if (!Pawn)
	{
		return;
	}

	if (const UMGPawnExtensionComponent* PawnExtComp = UMGPawnExtensionComponent::FindPawnExtensionComponent(Pawn))
	{
		if (UMGAbilitySystemComponent* MGASC = PawnExtComp->GetMGAbilitySystemComponent())
		{
			MGASC->AbilityInputTagReleased(InputTag);
		}
	}
}

void UMGHeroComponent::Input_Move(const FInputActionValue& InputActionValue)
{
	UAbilitySystemComponent* ASC = GetPlayerState<AMGPlayerState>()->GetAbilitySystemComponent();
	if (ASC && ASC->HasMatchingGameplayTag(MundusGranumGameplayTags::CharacterState_Rigidity))
	{
		return;
	}
	APawn* Pawn = GetPawn<APawn>();
	AController* Controller = Pawn ? Pawn->GetController() : nullptr;
	if (Controller != nullptr)
	{
		const FVector2D MoveValue = InputActionValue.Get<FVector2D>();
		const FRotator MovementRotation(0.0f, Controller->GetControlRotation().Yaw, 0.0f);
 
		if (MoveValue.X != 0.0f)
		{
			const FVector MovementDirection = MovementRotation.RotateVector(FVector::RightVector);
			Pawn->AddMovementInput(MovementDirection, MoveValue.X);
		}
 
		if (MoveValue.Y != 0.0f)
		{
			const FVector MovementDirection = MovementRotation.RotateVector(FVector::ForwardVector);
			Pawn->AddMovementInput(MovementDirection, MoveValue.Y);
		}
	}
}

void UMGHeroComponent::Input_LookMouse(const FInputActionValue& InputActionValue)
{
	APawn* Pawn = GetPawn<APawn>();
	if (!Pawn)
	{
		return;
	}
	
	const FVector2D Value = InputActionValue.Get<FVector2D>();

	if (Value.X != 0.0f)
	{
		Pawn->AddControllerYawInput(Value.X);
	}

	if (Value.Y != 0.0f)
	{
		Pawn->AddControllerPitchInput(Value.Y);
	}
}

void UMGHeroComponent::Input_SprintPressed()
{
	ACharacter* Character = Cast<ACharacter>(GetPawn<APawn>());
	if (!Character) return;
	Character->GetCharacterMovement()->MaxWalkSpeed = 600;
}

void UMGHeroComponent::Input_SprintReleased()
{
	ACharacter* Character = Cast<ACharacter>(GetPawn<APawn>());
	if (!Character) return;
	Character->GetCharacterMovement()->MaxWalkSpeed = 230;
}

void UMGHeroComponent::Input_SlowWalk()
{
}

void UMGHeroComponent::InputTag_SelectItem(const FInputActionValue& Value)
{
	UMGInventoryComponent* InventoryComponent = GetOwner()->FindComponentByClass<UMGInventoryComponent>();
	if (!InventoryComponent) return;
	const float Axis = Value.Get<float>();
	if (FMath::IsNearlyZero(Axis)) return;
	const int32 Delta = Axis > 0.f ? 1 : -1;
	InventoryComponent->SelectSlot(InventoryComponent->GetSelectedSlotIndex() + Delta);
}



