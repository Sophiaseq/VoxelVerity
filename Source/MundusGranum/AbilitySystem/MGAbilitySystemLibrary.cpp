// Fill out your copyright notice in the Description page of Project Settings.


#include "MGAbilitySystemLibrary.h"

#include "MGAbilitySystemComponent.h"
#include "Attributes/MGHealthSet.h"
#include "GameModes/MGExperienceManagerComponent.h"
#include "GameModes/MGGameMode.h"
#include "GameModes/MGGameState.h"
#include "Kismet/GameplayStatics.h"
#include "Player/MGPlayerState.h"
#include "UI/HUD/MGHUD.h"
#include "UI/WidgetController/MGWidgetController.h"

UAttributeMenuWidgetController* UMGAbilitySystemLibrary::GetAttributeMenuWidgetController(const UObject* WorldContextObject)
{
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(WorldContextObject, 0))
	{
		if (AMGHUD* MGHUD = Cast<AMGHUD>(PC->GetHUD()))
		{
			AMGPlayerState* PS = PC->GetPlayerState<AMGPlayerState>();
			UMGAbilitySystemComponent* ASC = PS->GetMGAbilitySystemComponent();
			const TArray<UAttributeSet*> Attributes= ASC->GetSpawnedAttributes();
			const FWidgetControllerParams Params(PC, PS, ASC, Attributes);
			return MGHUD->GetAttributeMenuWidgetController(Params);
		}
	}
	return nullptr;
}

const UMGPawnData* UMGAbilitySystemLibrary::GetDefaultPawnData(const UObject* WorldContextObject)
{
	const AMGGameState* MGGameState = Cast<AMGGameState>(UGameplayStatics::GetGameMode(WorldContextObject)->GameState);
	if (!MGGameState) return nullptr;

	const UMGExperienceManagerComponent* ExperienceManagerComponent = MGGameState->FindComponentByClass<UMGExperienceManagerComponent>();
	if (!ExperienceManagerComponent) return nullptr;
	
	return ExperienceManagerComponent->GetCurrentExperienceChecked()->DefaultPawnData;
}

bool UMGAbilitySystemLibrary::IsBlockedHit(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if (const FMGGameplayEffectContext* MGEffectContext = static_cast<const FMGGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		return MGEffectContext->IsBlockedHit();
	}
	return false;
}

bool UMGAbilitySystemLibrary::IsCriticalHit(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if (const FMGGameplayEffectContext* MGEffectContext = static_cast<const FMGGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		return MGEffectContext->IsCriticalHit();
	}
	return false;
}

void UMGAbilitySystemLibrary::SetIsBlockedHit(FGameplayEffectContextHandle& EffectContextHandle, bool bInIsBlockedHit)
{
	if (FMGGameplayEffectContext* MGEffectContext = static_cast<FMGGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		MGEffectContext->SetIsBlockedHit(bInIsBlockedHit);
	}
}

void UMGAbilitySystemLibrary::SetIsCriticalHit(FGameplayEffectContextHandle& EffectContextHandle, bool bInIsCriticalHit)
{
	if (FMGGameplayEffectContext* MGEffectContext = static_cast<FMGGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		MGEffectContext->SetIsCriticalHit(bInIsCriticalHit);
	}
}
