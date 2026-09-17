// Fill out your copyright notice in the Description page of Project Settings.


#include "MGGameplayAbility.h"

#include "AbilitySystem/MGAbilitySystemComponent.h"
#include "Character/MGCharacter.h"
#include "Character/MGHeroComponent.h"
#include "Player/MGPlayerController.h"

UMGGameplayAbility::UMGGameplayAbility(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
	ReplicationPolicy = EGameplayAbilityReplicationPolicy::ReplicateNo;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
	NetSecurityPolicy = EGameplayAbilityNetSecurityPolicy::ClientOrServer;

	ActivationPolicy = EMGAbilityActivationPolicy::OnInputTriggered;
}

UMGAbilitySystemComponent* UMGGameplayAbility::GetMGAbilitySystemComponentFromActorInfo() const
{
	return CurrentActorInfo ? Cast<UMGAbilitySystemComponent>(CurrentActorInfo->AbilitySystemComponent.Get()) : nullptr;
}

AMGPlayerController* UMGGameplayAbility::GetMGPlayerControllerFromActorInfo() const
{
	return (CurrentActorInfo ? Cast<AMGPlayerController>(CurrentActorInfo->PlayerController.Get()) : nullptr);
}

AController* UMGGameplayAbility::GetControllerFromActorInfo() const
{
	if (CurrentActorInfo)
	{
		if (AController* PC = CurrentActorInfo->PlayerController.Get())
		{
			return PC;
		}

		// Look for a player controller or pawn in the owner chain.
		AActor* TestActor = CurrentActorInfo->OwnerActor.Get();
		while (TestActor)
		{
			if (AController* C = Cast<AController>(TestActor))
			{
				return C;
			}

			if (APawn* Pawn = Cast<APawn>(TestActor))
			{
				return Pawn->GetController();
			}

			TestActor = TestActor->GetOwner();
		}
	}

	return nullptr;
}

UMGHeroComponent* UMGGameplayAbility::GetHeroComponentFromActorInfo() const
{
	return (CurrentActorInfo ? UMGHeroComponent::FindHeroComponent(CurrentActorInfo->AvatarActor.Get()) : nullptr);
}

AMGCharacter* UMGGameplayAbility::GetMGCharacterFromActorInfo() const
{
	return (CurrentActorInfo ? Cast<AMGCharacter>(CurrentActorInfo->AvatarActor.Get()) : nullptr);
}
