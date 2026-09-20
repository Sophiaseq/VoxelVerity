// Fill out your copyright notice in the Description page of Project Settings.


#include "MGPlayerController.h"

#include <ThirdParty/ShaderConductor/ShaderConductor/External/DirectXShaderCompiler/include/dxc/DXIL/DxilConstants.h>

#include "MGLogChannels.h"
#include "MGPlayerState.h"
#include "AbilitySystem/MGAbilitySystemComponent.h"
#include "AbilitySystem/Attributes/MGHealthSet.h"
#include "Net/UnrealNetwork.h"
#include "UI/HUD/MGHUD.h"

AMGPlayerController::AMGPlayerController()
{
	bReplicates = true;
}

void AMGPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Disable replicating the PC target view as it doesn't work well for replays or client-side spectating.
	// The engine TargetViewRotation is only set in APlayerController::TickActor if the server knows ahead of time that 
	// a specific pawn is being spectated and it only replicates down for COND_OwnerOnly.
	// In client-saved replays, COND_OwnerOnly is never true and the target pawn is not always known at the time of recording.
	// To support client-saved replays, the replication of this was moved to ReplicatedViewRotation and updated in PlayerTick.
	DISABLE_REPLICATED_PROPERTY(APlayerController, TargetViewRotation);
}

AMGPlayerState* AMGPlayerController::GetMGPlayerState() const
{
	return CastChecked<AMGPlayerState>(PlayerState, ECastCheckedType::NullAllowed);
}

UMGAbilitySystemComponent* AMGPlayerController::GetMGAbilitySystemComponent() const
{
	const AMGPlayerState* MGPS = GetMGPlayerState();
	return (MGPS ? MGPS->GetMGAbilitySystemComponent() : nullptr);
}


void AMGPlayerController::BeginPlay()
{
	Super::BeginPlay();
	
	InitHUD();
}

void AMGPlayerController::OnPlayerStateChanged()
{
	// Empty, place for derived classes to implement without having to hook all the other events
}

void AMGPlayerController::InitHUD()
{
	AMGPlayerState* PS = GetPlayerState<AMGPlayerState>();
	AMGHUD* HUD = Cast<AMGHUD>(GetHUD());
	UMGAbilitySystemComponent* MGASC = GetMGAbilitySystemComponent();
	if (MGASC == nullptr) return;
	const TArray<UAttributeSet*> Attributes = MGASC->GetSpawnedAttributes();
	if (PS && HUD)
	{
		HUD->InitOverlay(this, PS, MGASC, Attributes);
	}
}

void AMGPlayerController::BroadcastOnPlayerStateChanged()
{
	OnPlayerStateChanged();
	LastSeenPlayerState = PlayerState;
}

void AMGPlayerController::InitPlayerState()
{
	Super::InitPlayerState();
	BroadcastOnPlayerStateChanged();
}

void AMGPlayerController::CleanupPlayerState()
{
	Super::CleanupPlayerState();
	BroadcastOnPlayerStateChanged();
}

void AMGPlayerController::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	BroadcastOnPlayerStateChanged();

	UE_LOG(LogMG, Warning, TEXT("[Init] PlayerController::OnRep_PlayerState → PlayerState 复制到达"));

	// When we're a client connected to a remote server, the player controller may replicate later than the PlayerState and AbilitySystemComponent.
	// However, TryActivateAbilitiesOnSpawn depends on the player controller being replicated in order to check whether on-spawn abilities should
	// execute locally. Therefore once the PlayerController exists and has resolved the PlayerState, try once again to activate on-spawn abilities.
	// On other net modes the PlayerController will never replicate late, so MGASC's own TryActivateAbilitiesOnSpawn calls will succeed. The handling 
	// here is only for when the PlayerState and ASC replicated before the PC and incorrectly thought the abilities were not for the local player.
	if (GetWorld()->IsNetMode(NM_Client))
	{
		if (UMGAbilitySystemComponent* MGASC = GetMGAbilitySystemComponent())
		{
			MGASC->RefreshAbilityActorInfo();
			MGASC->TryActivateAbilitiesOnSpawn();
		}
	}
	
	InitHUD();
}

void AMGPlayerController::SetPlayer(UPlayer* InPlayer)
{
	Super::SetPlayer(InPlayer);
}

void AMGPlayerController::PreProcessInput(const float DeltaTime, const bool bGamePaused)
{
	Super::PreProcessInput(DeltaTime, bGamePaused);
}

void AMGPlayerController::PostProcessInput(const float DeltaTime, const bool bGamePaused)
{
	if (UMGAbilitySystemComponent* MGASC = GetMGAbilitySystemComponent())
	{
		MGASC->ProcessAbilityInput(DeltaTime, bGamePaused);
	}
	
	Super::PostProcessInput(DeltaTime, bGamePaused);
}

void AMGPlayerController::ShowDamageNumber_Implementation(float DamageAmount, const FVector& WidgetSpawnLocation)
{
	if (DamageTextComponentClass)
	{
		UDamageTextComponent* TextComponent = NewObject<UDamageTextComponent>(this, DamageTextComponentClass);
		TextComponent->RegisterComponent();
		TextComponent->SetWorldLocation(WidgetSpawnLocation);
		TextComponent->SetDamageText(DamageAmount);
	}
}

