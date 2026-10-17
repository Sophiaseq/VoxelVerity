// Fill out your copyright notice in the Description page of Project Settings.


#include "MGPlayerSpawningManagerComponent.h"

#include "EngineUtils.h"
#include "Character/MGHealthComponent.h"
#include "GameFramework/PlayerStart.h"
#include "GameFramework/PlayerState.h"
#include "Engine/PlayerStartPIE.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(MGPlayerSpawningManagerComponent)

UMGPlayerSpawningManagerComponent::UMGPlayerSpawningManagerComponent(const FObjectInitializer& InitializerInitializer)
	: Super(InitializerInitializer)
{
	SetIsReplicatedByDefault(false);
	bAutoRegister = true;
	bAutoActivate = true;
	bWantsInitializeComponent = true;
	PrimaryComponentTick.TickGroup = TG_PrePhysics;
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bAllowTickOnDedicatedServer = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

AActor* UMGPlayerSpawningManagerComponent::ChoosePlayerStart(AController* Player)
{
	if (Player)
	{
		if (const FTransform* DeathTransform = DeathTransforms.Find(Player))
		{
			if (UWorld* World = GetWorld())
			{
				FActorSpawnParameters SpawnParams;
				SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

				APlayerStart* TempStart = World->SpawnActor<APlayerStart>(
					APlayerStart::StaticClass(), *DeathTransform, SpawnParams);

				if (TempStart)
				{
					TempPlayerStarts.Add(TempStart);
					return TempStart;
				}
			}
		}
#if WITH_EDITOR
		if (APlayerStart* PlayerStart = FindPlayFromHereStart(Player))
		{
			return PlayerStart;
		}
#endif
		
		TArray<APlayerStart*> StarterPoints;
		for (auto StartIt = CachedPlayerStarts.CreateIterator(); StartIt; ++StartIt)
		{
			if (APlayerStart* Start = (*StartIt).Get())
			{
				StarterPoints.Add(Start);
			}
			else
			{
				StartIt.RemoveCurrent();
			}
		}

		if (APlayerState* PlayerState = Player->GetPlayerState<APlayerState>())
		{
			// start dedicated spectators at any random starting location, but they do not claim it
			if (PlayerState->IsOnlyASpectator())
			{
				if (!StarterPoints.IsEmpty())
				{
					return StarterPoints[FMath::RandRange(0, StarterPoints.Num() - 1)];
				}

				return nullptr;
			}
		}

		AActor* PlayerStart = OnChoosePlayerStart(Player, StarterPoints);

		if (!PlayerStart)
		{
			PlayerStart = GetFirstRandomPlayerStart(Player, StarterPoints);
		}

		/*if (AMGPlayerStart* MGStart = Cast<AMGPlayerStart>(PlayerStart))
		{
			MGStart->TryClaim(Player);
		}*/

		return PlayerStart;
	}
	return nullptr;
}

#if WITH_EDITOR
APlayerStart* UMGPlayerSpawningManagerComponent::FindPlayFromHereStart(AController* Player)
{
	// Only 'Play From Here' for a player controller, bots etc. should all spawn from normal spawn points.
	if (Player->IsA<APlayerController>())
	{
		if (UWorld* World = GetWorld())
		{
			for (TActorIterator<APlayerStart> It(World); It; ++It)
			{
				if (APlayerStart* PlayerStart = *It)
				{
					if (PlayerStart->IsA<APlayerStartPIE>())
					{
						// Always prefer the first "Play from Here" PlayerStart, if we find one while in PIE mode
						return PlayerStart;
					}
				}
			}
		}
	}

	return nullptr;
}
#endif
	
void UMGPlayerSpawningManagerComponent::InitializeComponent()
{
	Super::InitializeComponent();
	
	UWorld* World = GetWorld();
	
	World->AddOnActorSpawnedHandler(FOnActorSpawned::FDelegate::CreateUObject(this, &ThisClass::HandleOnActorSpawned));

	for (TActorIterator<APlayerStart> It(World); It; ++It)
	{
		if (APlayerStart* PlayerStart = *It)
		{
			CachedPlayerStarts.Add(PlayerStart);
		}
	}
	
	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		if (APlayerController* PC = It->Get())
		{
			PC->GetOnNewPawnNotifier().AddUObject(this, &ThisClass::HandleNewPawn);
		}
	}
	
	FWorldDelegates::LevelAddedToWorld.AddUObject(this, &ThisClass::OnLevelAdded);
}

void UMGPlayerSpawningManagerComponent::RecordDeathTransform(AController* Controller, const FTransform& Transform)
{
	if (Controller)
	{
		DeathTransforms.Add(Controller, Transform);
	}
}

void UMGPlayerSpawningManagerComponent::OnLevelAdded(ULevel* InLevel, UWorld* InWorld)
{
	if (InWorld == GetWorld())
	{
		for (AActor* Actor : InLevel->Actors)
		{
			if (APlayerStart* PlayerStart = Cast<APlayerStart>(Actor))
			{
				ensure(!CachedPlayerStarts.Contains(PlayerStart));
				CachedPlayerStarts.Add(PlayerStart);
			}
		}
	}
}

void UMGPlayerSpawningManagerComponent::HandleOnActorSpawned(AActor* SpawnedActor)
{
	if (APlayerStart* PlayerStart = Cast<APlayerStart>(SpawnedActor))
	{
		CachedPlayerStarts.Add(PlayerStart);
	}
}

void UMGPlayerSpawningManagerComponent::HandleNewPawn(APawn* NewPawn)
{
	if (!NewPawn)
	{
		return;
	}

	if (UMGHealthComponent* HealthComp = NewPawn->FindComponentByClass<UMGHealthComponent>())
	{
		HealthComp->OnDeathStarted.RemoveAll(this);
		HealthComp->OnDeathStarted.AddDynamic(this, &ThisClass::HandlePawnDeathStarted);
	}
}

void UMGPlayerSpawningManagerComponent::HandlePawnDeathStarted(AActor* OwningActor)
{
	const APawn* Pawn = Cast<APawn>(OwningActor);
	if (!Pawn)
	{
		return;
	}

	if (AController* Controller = Pawn->GetController())
	{
		DeathTransforms.Add(Controller, Pawn->GetActorTransform());
	}
}

bool UMGPlayerSpawningManagerComponent::ControllerCanRestart(AController* Player)
{
	bool bCanRestart = true;

	// TODO Can they restart?

	return bCanRestart;
}

void UMGPlayerSpawningManagerComponent::FinishRestartPlayer(AController* NewPlayer, const FRotator& StartRotation)
{
	OnFinishRestartPlayer(NewPlayer, StartRotation);
	K2_OnFinishRestartPlayer(NewPlayer, StartRotation);
}

APlayerStart* UMGPlayerSpawningManagerComponent::GetFirstRandomPlayerStart(AController* Controller,
                                                                           const TArray<APlayerStart*>& StartPoints)
{
	if (Controller)
	{
		if (StartPoints.Num() > 0)
		{
			return StartPoints[FMath::RandRange(0, StartPoints.Num() - 1)];
		}
	}
	return nullptr;
}

void UMGPlayerSpawningManagerComponent::OnFinishRestartPlayer(AController* Player, const FRotator& StartRotation)
{
	for (APlayerStart* Temp : TempPlayerStarts)
	{
		if (IsValid(Temp))
		{
			Temp->Destroy();
		}
	}
	TempPlayerStarts.Empty();

	if (Player)
	{
		DeathTransforms.Remove(Player);
	}
}


