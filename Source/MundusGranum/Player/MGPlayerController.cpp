// Fill out your copyright notice in the Description page of Project Settings.


#include "MGPlayerController.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "MGPlayerState.h"
#include "MundusGranumGameplayTags.h"
#include "AbilitySystem/MGAbilitySystemComponent.h"
#include "Character/MGCharacter.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Input/MGInputComponent.h"

AMGPlayerController::AMGPlayerController()
{
	bReplicates = true;
}



void AMGPlayerController::BeginPlay()
{
	Super::BeginPlay();

}

void AMGPlayerController::OnPlayerStateChanged()
{
	// Empty, place for derived classes to implement without having to hook all the other events
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

	// When we're a client connected to a remote server, the player controller may replicate later than the PlayerState and AbilitySystemComponent.
	// However, TryActivateAbilitiesOnSpawn depends on the player controller being replicated in order to check whether on-spawn abilities should
	// execute locally. Therefore once the PlayerController exists and has resolved the PlayerState, try once again to activate on-spawn abilities.
	// On other net modes the PlayerController will never replicate late, so LyraASC's own TryActivateAbilitiesOnSpawn calls will succeed. The handling 
	// here is only for when the PlayerState and ASC replicated before the PC and incorrectly thought the abilities were not for the local player.
	if (GetWorld()->IsNetMode(NM_Client))
	{
		if (AMGPlayerState* LyraPS = GetPlayerState<AMGPlayerState>())
		{
			if (UMGAbilitySystemComponent* MGASC = LyraPS->GetMGAbilitySystemComponent())
			{
				MGASC->RefreshAbilityActorInfo();
				//TODO MGASC->TryActivateAbilitiesOnSpawn();
			}
		}
	}
}

