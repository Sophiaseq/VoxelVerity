// Fill out your copyright notice in the Description page of Project Settings.


#include "MGHUD.h"

#include "Blueprint/UserWidget.h"
#include "UI/Widget/MGUserWidget.h"
#include "UI/WidgetController/AttributeMenuWidgetController.h"
#include "UI/WidgetController/OverlayWidgetController.h"


UOverlayWidgetController* AMGHUD::GetOverlayWidgetController(const FWidgetControllerParams& WCParams)
{
	if (OverlayWidgetController == nullptr)
	{
		OverlayWidgetController = NewObject<UOverlayWidgetController>(this, OverlayWidgetControllerClass);
		OverlayWidgetController->SetWidgetControllerParams(WCParams);
		OverlayWidgetController->BindCallbackToDependencies();
		return OverlayWidgetController;
	}
	return OverlayWidgetController;
}

UAttributeMenuWidgetController* AMGHUD::GetAttributeMenuWidgetController(const FWidgetControllerParams& WCParams)
{
	if (AttributeMenuWidgetController == nullptr)
	{
		AttributeMenuWidgetController = NewObject<UAttributeMenuWidgetController>(this, AttributeMenuWidgetControllerClass);
		AttributeMenuWidgetController->SetWidgetControllerParams(WCParams);
		AttributeMenuWidgetController->BindCallbackToDependencies();
		return AttributeMenuWidgetController;
	}
	return AttributeMenuWidgetController;
}

void AMGHUD::InitOverlay(APlayerController* PC, APlayerState* PS, UAbilitySystemComponent* ASC, FMGPlayerAttributeSet Attributes)
{
	if (OverlayWidget) return;
	checkf(OverlayWidgetClass, TEXT("Overlay Widget Class uninitialized"))

	UUserWidget* Widget = CreateWidget<UUserWidget>(GetWorld(), OverlayWidgetClass);
	OverlayWidget = Cast<UMGUserWidget>(Widget);
	
	const FWidgetControllerParams WidgetControllerParams(PC, PS, ASC, Attributes);
	UOverlayWidgetController* WidgetController = GetOverlayWidgetController(WidgetControllerParams);
	
	OverlayWidget->SetWidgetController(WidgetController);
	WidgetController->BroadcastInitialValues();
	Widget->AddToViewport();
	
}

void AMGHUD::BeginPlay()
{
	Super::BeginPlay();
	
	//可以在HUD中InitOverlay一遍来兜一次底
	/*APlayerController* PC = GetOwningPlayerController();
	AMGPlayerState* PS = PC ? PC->GetPlayerState<AMGPlayerState>() : nullptr;
	if (PC && PS)
	{
		InitOverlay(PC, PS, PS->GetMGAbilitySystemComponent(), PS->GetHealth());
	}*/
}


