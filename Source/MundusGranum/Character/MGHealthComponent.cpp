// Fill out your copyright notice in the Description page of Project Settings.


#include "MGHealthComponent.h"

#include "MGLogChannels.h"
#include "MundusGranumGameplayTags.h"
#include "AbilitySystem/MGAbilitySystemComponent.h"
#include "AbilitySystem/Attributes/MGHealthSet.h"
#include "Net/UnrealNetwork.h"


UMGHealthComponent::UMGHealthComponent(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.bCanEverTick = false;

	SetIsReplicatedByDefault(true);
	
	DeathState = EMGDeathState::NotDead;
}

void UMGHealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UMGHealthComponent, DeathState);
}

void UMGHealthComponent::ClearGameplayTags()
{
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->SetLooseGameplayTagCount(MundusGranumGameplayTags::CharacterState_Death_Dying, 0);
		AbilitySystemComponent->SetLooseGameplayTagCount(MundusGranumGameplayTags::CharacterState_Death_Dead, 0);
	}
}

void UMGHealthComponent::InitializeWithAbilitySystem(UMGAbilitySystemComponent* InASC)
{
	AActor* Owner = GetOwner();
	check(Owner);

	if (AbilitySystemComponent)
	{
		UE_LOG(LogMG, Error, TEXT("MGHealthComponent: Health component for owner [%s] has already been initialized with an ability system."), *GetNameSafe(Owner));
		return;
	}

	AbilitySystemComponent = InASC;
	if (!AbilitySystemComponent)
	{
		UE_LOG(LogMG, Error, TEXT("MGHealthComponent: Cannot initialize health component for owner [%s] with NULL ability system."), *GetNameSafe(Owner));
		return;
	}

	HealthSet = AbilitySystemComponent->GetSet<UMGHealthSet>();
	if (!HealthSet)
	{
		UE_LOG(LogMG, Error, TEXT("MGHealthComponent: Cannot initialize health component for owner [%s] with NULL health set on the ability system."), *GetNameSafe(Owner));
		return;
	}

	// Register to listen for attribute changes.
	HealthSet->OnHealthChanged.AddUObject(this, &ThisClass::HandleHealthChanged);
	HealthSet->OnMaxHealthChanged.AddUObject(this, &ThisClass::HandleMaxHealthChanged);
	HealthSet->OnOutOfHealth.AddUObject(this, &ThisClass::HandleOutOfHealth);
	HealthSet->OnStaminaChanged.AddUObject(this, &ThisClass::HandleStaminaChanged);
	HealthSet->OnMaxStaminaChanged.AddUObject(this, &ThisClass::HandleMaxStaminaChanged);
	HealthSet->OnOutOfStamina.AddUObject(this, &ThisClass::HandleOutOfStamina);
	
	ClearGameplayTags();

	OnHealthChanged.Broadcast(this, HealthSet->GetHealth(), HealthSet->GetHealth(), nullptr);
	OnMaxHealthChanged.Broadcast(this, HealthSet->GetMaxHealth(), HealthSet->GetMaxHealth(), nullptr);
	OnStaminaChanged.Broadcast(this, HealthSet->GetStamina(), HealthSet->GetStamina(), nullptr);
	OnMaxStaminaChanged.Broadcast(this, HealthSet->GetMaxStamina(), HealthSet->GetMaxStamina(), nullptr);
}

float UMGHealthComponent::GetHealth() const
{
	return HealthSet ? HealthSet->GetHealth() : 0.0f;
}

float UMGHealthComponent::GetMaxHealth() const
{
	return HealthSet ? HealthSet->GetMaxHealth() : 0.0f;
}

void UMGHealthComponent::UninitializeFromAbilitySystem()
{
	ClearGameplayTags();

	if (HealthSet)
	{
		HealthSet->OnHealthChanged.RemoveAll(this);
		HealthSet->OnMaxHealthChanged.RemoveAll(this);
		HealthSet->OnOutOfHealth.RemoveAll(this);
		HealthSet->OnStaminaChanged.RemoveAll(this);
		HealthSet->OnMaxStaminaChanged.RemoveAll(this);
		HealthSet->OnOutOfStamina.RemoveAll(this);
	}

	HealthSet = nullptr;
	AbilitySystemComponent = nullptr;
}

void UMGHealthComponent::StartDeath()
{
	if (DeathState != EMGDeathState::NotDead)
	{
		return;
	}

	DeathState = EMGDeathState::DeathStarted;

	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->SetLooseGameplayTagCount(MundusGranumGameplayTags::CharacterState_Death_Dying, 1);
	}

	AActor* Owner = GetOwner();
	check(Owner);

	OnDeathStarted.Broadcast(Owner);
	
	Owner->ForceNetUpdate();
}

void UMGHealthComponent::FinishDeath()
{
	if (DeathState != EMGDeathState::DeathStarted)
	{
		return;
	}

	DeathState = EMGDeathState::DeathFinished;

	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->SetLooseGameplayTagCount(MundusGranumGameplayTags::CharacterState_Death_Dead, 1);
	}

	AActor* Owner = GetOwner();
	check(Owner);

	OnDeathFinished.Broadcast(Owner);

	Owner->ForceNetUpdate();
}

void UMGHealthComponent::OnUnregister()
{
	UninitializeFromAbilitySystem();
	
	Super::OnUnregister();
}

void UMGHealthComponent::OnRep_DeathState(EMGDeathState OldDeathState)
{
	const EMGDeathState NewDeathState = DeathState;

	// Revert the death state for now since we rely on StartDeath and FinishDeath to change it.
	DeathState = OldDeathState;

	if (OldDeathState > NewDeathState)
	{
		// The server is trying to set us back but we've already predicted past the server state.
		UE_LOG(LogMG, Warning, TEXT("MGHealthComponent: Predicted past server death state [%d] -> [%d] for owner [%s]."), (uint8)OldDeathState, (uint8)NewDeathState, *GetNameSafe(GetOwner()));
		return;
	}

	if (OldDeathState == EMGDeathState::NotDead)
	{
		if (NewDeathState == EMGDeathState::DeathStarted)
		{
			StartDeath();
		}
		else if (NewDeathState == EMGDeathState::DeathFinished)
		{
			StartDeath();
			FinishDeath();
		}
		else
		{
			UE_LOG(LogMG, Error, TEXT("MGHealthComponent: Invalid death transition [%d] -> [%d] for owner [%s]."), (uint8)OldDeathState, (uint8)NewDeathState, *GetNameSafe(GetOwner()));
		}
	}
	else if (OldDeathState == EMGDeathState::DeathStarted)
	{
		if (NewDeathState == EMGDeathState::DeathFinished)
		{
			FinishDeath();
		}
		else
		{
			UE_LOG(LogMG, Error, TEXT("MGHealthComponent: Invalid death transition [%d] -> [%d] for owner [%s]."), (uint8)OldDeathState, (uint8)NewDeathState, *GetNameSafe(GetOwner()));
		}
	}

	ensureMsgf((DeathState == NewDeathState), TEXT("MGHealthComponent: Death transition failed [%d] -> [%d] for owner [%s]."), (uint8)OldDeathState, (uint8)NewDeathState, *GetNameSafe(GetOwner()));
}

void UMGHealthComponent::HandleHealthChanged(AActor* DamageInstigator, AActor* DamageCauser,
                                             const FGameplayEffectSpec* DamageEffectSpec, float DamageMagnitude, float OldValue, float NewValue)
{
	OnHealthChanged.Broadcast(this, OldValue, NewValue, DamageInstigator);
}

void UMGHealthComponent::HandleMaxHealthChanged(AActor* DamageInstigator, AActor* DamageCauser,
	const FGameplayEffectSpec* DamageEffectSpec, float DamageMagnitude, float OldValue, float NewValue)
{
	OnMaxHealthChanged.Broadcast(this, OldValue, NewValue, DamageInstigator);
}

void UMGHealthComponent::HandleOutOfHealth(AActor* DamageInstigator, AActor* DamageCauser,
	const FGameplayEffectSpec* DamageEffectSpec, float DamageMagnitude, float OldValue, float NewValue)
{
#if WITH_SERVER_CODE
	if (AbilitySystemComponent && DamageEffectSpec)
	{
		// Send the "GameplayEvent.Death" gameplay event through the owner's ability system.  This can be used to trigger a death gameplay ability.
		{
			FGameplayEventData Payload;
			Payload.EventTag = MundusGranumGameplayTags::GameplayEvent_Death;
			Payload.Instigator = DamageInstigator;
			Payload.Target = AbilitySystemComponent->GetAvatarActor();
			Payload.OptionalObject = DamageEffectSpec->Def;
			Payload.ContextHandle = DamageEffectSpec->GetEffectContext();
			Payload.InstigatorTags = *DamageEffectSpec->CapturedSourceTags.GetAggregatedTags();
			Payload.TargetTags = *DamageEffectSpec->CapturedTargetTags.GetAggregatedTags();
			Payload.EventMagnitude = DamageMagnitude;

			FScopedPredictionWindow NewScopedWindow(AbilitySystemComponent, true);
			AbilitySystemComponent->HandleGameplayEvent(Payload.EventTag, &Payload);
		}

		// Send a standardized verb message that other systems can observe
		/*{
			FMGVerbMessage Message;
			Message.Verb = TAG_MG_Elimination_Message;
			Message.Instigator = DamageInstigator;
			Message.InstigatorTags = *DamageEffectSpec->CapturedSourceTags.GetAggregatedTags();
			Message.Target = UMGVerbMessageHelpers::GetPlayerStateFromObject(AbilitySystemComponent->GetAvatarActor());
			Message.TargetTags = *DamageEffectSpec->CapturedTargetTags.GetAggregatedTags();
			//@TODO: Fill out context tags, and any non-ability-system source/instigator tags
			//@TODO: Determine if it's an opposing team kill, self-own, team kill, etc...

			UGameplayMessageSubsystem& MessageSystem = UGameplayMessageSubsystem::Get(GetWorld());
			MessageSystem.BroadcastMessage(Message.Verb, Message);
		}*/

		//@TODO: assist messages (could compute from damage dealt elsewhere)?
	}

#endif // #if WITH_SERVER_CODE
}

void UMGHealthComponent::HandleStaminaChanged(AActor* DamageInstigator, AActor* DamageCauser,
	const FGameplayEffectSpec* DamageEffectSpec, float DamageMagnitude, float OldValue, float NewValue)
{
	OnStaminaChanged.Broadcast(this, OldValue, NewValue, DamageInstigator);
}

void UMGHealthComponent::HandleMaxStaminaChanged(AActor* DamageInstigator, AActor* DamageCauser,
	const FGameplayEffectSpec* DamageEffectSpec, float DamageMagnitude, float OldValue, float NewValue)
{
	OnMaxStaminaChanged.Broadcast(this, OldValue, NewValue, DamageInstigator);
}

void UMGHealthComponent::HandleOutOfStamina(AActor* DamageInstigator, AActor* DamageCauser,
	const FGameplayEffectSpec* DamageEffectSpec, float DamageMagnitude, float OldValue, float NewValue)
{
	//TODO: Stamina消耗完时触发强硬的僵直
}



