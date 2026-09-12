// Fill out your copyright notice in the Description page of Project Settings.


#include "MGAbilitySystemLibrary.h"

#include "MGAbilitySystemComponent.h"
#include "Attributes/MGHealthSet.h"
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
			const FWidgetControllerParams Params(PC, PS, ASC, PS->GetPlayerAttributes());
			return MGHUD->GetAttributeMenuWidgetController(Params);
		}
	}
	return nullptr;
}
