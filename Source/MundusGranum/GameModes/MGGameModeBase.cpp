// Fill out your copyright notice in the Description page of Project Settings.


#include "MGGameModeBase.h"

#include "GameMapsSettings.h"
#include "MGExperienceManagerComponent.h"
#include "MGLogChannels.h"
#include "MGGameState.h"
#include "Character/MGCharacter.h"
#include "Character/MGPawnExtensionComponent.h"
#include "Engine/AssetManager.h"
#include "Kismet/GameplayStatics.h"
#include "Player/MGPlayerController.h"
#include "Player/MGPlayerState.h"
#include "System/MGAssetManager.h"
#include "UI/HUD/MGHUD.h"

AMGGameModeBase::AMGGameModeBase()
{
	GameStateClass = AMGGameState::StaticClass();
	PlayerControllerClass = AMGPlayerController::StaticClass();
	PlayerStateClass = AMGPlayerState::StaticClass();
	DefaultPawnClass = AMGCharacter::StaticClass();
	HUDClass = AMGHUD::StaticClass();
}

const UMGCharacterDefinition* AMGGameModeBase::GetPawnDataForController(const AController* InController) const
{
	// See if pawn data is already set on the player state
	if (InController != nullptr)
	{
		if (const AMGPlayerState* MGPS = InController->GetPlayerState<AMGPlayerState>())
		{
			if (const UMGCharacterDefinition* PawnData = MGPS->GetPawnData<UMGCharacterDefinition>())
			{
				return PawnData;
			}
		}
	}

	// If not, fall back to the the default for the current experience
	check(GameState);
	UMGExperienceManagerComponent* ExperienceComponent = GameState->FindComponentByClass<UMGExperienceManagerComponent>();
	check(ExperienceComponent);

	if (ExperienceComponent->IsExperienceLoaded())
	{
		const UMGExperienceDefinition* Experience = ExperienceComponent->GetCurrentExperienceChecked();
		if (Experience->DefaultPawnData != nullptr)
		{
			return Experience->DefaultPawnData;
		}

		// Experience is loaded and there's still no pawn data, fall back to the default for now
		return UMGAssetManager::Get().GetDefaultPawnData();
	}

	// Experience not loaded yet, so there is no pawn data to be had
	return nullptr;
}

void AMGGameModeBase::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);

	UE_LOG(LogMGExperience, Warning, TEXT("[Init] GameMode::InitGame → 下帧 HandleMatchAssignmentIfNotExpectingOne"));

	// Wait for the next frame to give time to initialize startup settings
	GetWorld()->GetTimerManager().SetTimerForNextTick(this, &ThisClass::HandleMatchAssignmentIfNotExpectingOne);
}

UClass* AMGGameModeBase::GetDefaultPawnClassForController_Implementation(AController* InController)
{
	if (const UMGCharacterDefinition* PawnData = GetPawnDataForController(InController))
	{
		if (PawnData->PawnClass)
		{
			return PawnData->PawnClass;
		}
	}

	return Super::GetDefaultPawnClassForController_Implementation(InController);
}

APawn* AMGGameModeBase::SpawnDefaultPawnAtTransform_Implementation(AController* NewPlayer,
                                                                   const FTransform& SpawnTransform)
{
	FActorSpawnParameters SpawnInfo;
	SpawnInfo.Instigator = GetInstigator();
	SpawnInfo.ObjectFlags |= RF_Transient;	// Never save the default player pawns into a map.
	SpawnInfo.bDeferConstruction = true;

	if (UClass* PawnClass = GetDefaultPawnClassForController(NewPlayer))
	{
		UE_LOG(LogMG, Warning, TEXT("[Init] GameMode::SpawnDefaultPawnAtTransform → SpawnActor(%s)"), *GetNameSafe(PawnClass));

		if (APawn* SpawnedPawn = GetWorld()->SpawnActor<APawn>(PawnClass, SpawnTransform, SpawnInfo))
		{
			if (UMGPawnExtensionComponent* PawnExtComp = UMGPawnExtensionComponent::FindPawnExtensionComponent(SpawnedPawn))
			{
				if (const UMGCharacterDefinition* PawnData = GetPawnDataForController(NewPlayer))
				{
					PawnExtComp->SetPawnData(PawnData);
				}
				else
				{
					UE_LOG(LogTemp, Error, TEXT("Game mode was unable to set PawnData on the spawned pawn [%s]."), *GetNameSafe(SpawnedPawn));
				}
			}

			SpawnedPawn->FinishSpawning(SpawnTransform);

			return SpawnedPawn;
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Game mode was unable to spawn Pawn of class [%s] at [%s]."), *GetNameSafe(PawnClass), *SpawnTransform.ToHumanReadableString());
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Game mode was unable to spawn Pawn due to NULL pawn class."));
	}

	return nullptr;
}

void AMGGameModeBase::InitGameState()
{
	Super::InitGameState();
	
	// 获取 ExperienceManager 组件
	UMGExperienceManagerComponent* ExperienceComponent = GameState->FindComponentByClass<UMGExperienceManagerComponent>();
	check(ExperienceComponent);
	
	ExperienceComponent->CallOrRegister_OnExperienceLoaded(FOnMGExperienceLoaded::FDelegate::CreateUObject(this, &ThisClass::OnExperienceLoaded));
}

void AMGGameModeBase::OnExperienceLoaded(const UMGExperienceDefinition* CurrentExperience)
{
	UE_LOG(LogMGExperience, Warning, TEXT("[Init] GameMode::OnExperienceLoaded → 遍历无 Pawn 的 PC RestartPlayer"));

	// Spawn any players that are already attached
	//@TODO: Here we're handling only *player* controllers, but in GetDefaultPawnClassForController_Implementation we skipped all controllers
	// GetDefaultPawnClassForController_Implementation might only be getting called for players anyways
	for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		APlayerController* PC = Cast<APlayerController>(*Iterator);
		if ((PC != nullptr) && (PC->GetPawn() == nullptr))
		{
			if (PlayerCanRestart(PC))
			{
				RestartPlayer(PC);
			}
		}
	}
}

bool AMGGameModeBase::IsExperienceLoaded() const
{
	check(GameState);
	UMGExperienceManagerComponent* ExperienceComponent = GameState->FindComponentByClass<UMGExperienceManagerComponent>();
	check(ExperienceComponent);
	return ExperienceComponent->IsExperienceLoaded();
}


void AMGGameModeBase::OnMatchAssignmentGiven(FPrimaryAssetId ExperienceId, const FString& ExperienceIdSource)
{
	if (ExperienceId.IsValid())
	{
		UE_LOG(LogGameMode, Log, TEXT("Identified experience %s (Source: %s)"), *ExperienceId.ToString(), *ExperienceIdSource);

		UMGExperienceManagerComponent* ExperienceComponent = GameState->FindComponentByClass<UMGExperienceManagerComponent>();
		check(ExperienceComponent);
		ExperienceComponent->SetCurrentExperience(ExperienceId);
	}
	else
	{
		UE_LOG(LogGameMode, Error, TEXT("Failed to identify experience, loading screen will stay up forever"));
	}
}

void AMGGameModeBase::HandleMatchAssignmentIfNotExpectingOne()
{
	FPrimaryAssetId ExperienceId;
	FString ExperienceIdSource;

	// Precedence order (highest wins)
	//  - Matchmaking assignment (if present)
	//  - URL Options override
	//  - Developer Settings (PIE only)
	//  - Command Line override
	//  - World Settings
	//  - Dedicated server
	//  - Default experience

	UWorld* World = GetWorld();

	if (!ExperienceId.IsValid() && UGameplayStatics::HasOption(OptionsString, TEXT("Experience")))
	{
		const FString ExperienceFromOptions = UGameplayStatics::ParseOption(OptionsString, TEXT("Experience"));
		ExperienceId = FPrimaryAssetId(FPrimaryAssetType(UMGExperienceDefinition::StaticClass()->GetFName()), FName(*ExperienceFromOptions));
		ExperienceIdSource = TEXT("OptionsString");
	}

	/*if (!ExperienceId.IsValid() && World->IsPlayInEditor())
	{
		ExperienceId = GetDefault<ULyraDeveloperSettings>()->ExperienceOverride;
		ExperienceIdSource = TEXT("DeveloperSettings");
	}*/

	// see if the command line wants to set the experience
	if (!ExperienceId.IsValid())
	{
		FString ExperienceFromCommandLine;
		if (FParse::Value(FCommandLine::Get(), TEXT("Experience="), ExperienceFromCommandLine))
		{
			ExperienceId = FPrimaryAssetId::ParseTypeAndName(ExperienceFromCommandLine);
			if (!ExperienceId.PrimaryAssetType.IsValid())
			{
				ExperienceId = FPrimaryAssetId(FPrimaryAssetType(UMGExperienceDefinition::StaticClass()->GetFName()), FName(*ExperienceFromCommandLine));
			}
			ExperienceIdSource = TEXT("CommandLine");
		}
	}

	// see if the world settings has a default experience
	/*if (!ExperienceId.IsValid())
	{
		if (ALyraWorldSettings* TypedWorldSettings = Cast<ALyraWorldSettings>(GetWorldSettings()))
		{
			ExperienceId = TypedWorldSettings->GetDefaultGameplayExperience();
			ExperienceIdSource = TEXT("WorldSettings");
		}
	}*/

	UAssetManager& AssetManager = UAssetManager::Get();
	FAssetData Dummy;
	if (ExperienceId.IsValid() && !AssetManager.GetPrimaryAssetData(ExperienceId, /*out*/ Dummy))
	{
		UE_LOG(LogTemp, Error, TEXT("EXPERIENCE: Wanted to use %s but couldn't find it, falling back to the default)"), *ExperienceId.ToString());
		ExperienceId = FPrimaryAssetId();
	}

	// Final fallback to the default experience
	if (!ExperienceId.IsValid())
	{
		/*if (TryDedicatedServerLogin())
		{
			// This will start to host as a dedicated server
			return;
		}*/

		//@TODO: Pull this from a config setting or something
		ExperienceId = FPrimaryAssetId(FPrimaryAssetType("MGExperienceDefinition"), FName("B_Experience0"));
		ExperienceIdSource = TEXT("Default");
	}

	OnMatchAssignmentGiven(ExperienceId, ExperienceIdSource);
}

void AMGGameModeBase::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer)
{
	// 延迟生成玩家，直到 Experience 加载完成（加载完后 OnExperienceLoaded 会 RestartPlayer）
	if (IsExperienceLoaded())
	{
		Super::HandleStartingNewPlayer_Implementation(NewPlayer);
	}
}
