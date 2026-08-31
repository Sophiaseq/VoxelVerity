// Fill out your copyright notice in the Description page of Project Settings.


#include "MGWidgetController.h"

void UMGWidgetController::SetWidgetControllerParams(const FWidgetControllerParams& WCParams)
{
	PlayerController = WCParams.PlayerController;
	PlayerState = WCParams.PlayerState;
	AbilitySystemComponent = WCParams.AbilitySystemComponent;
	AttributeSet = WCParams.AttributeSet;
}

void UMGWidgetController::BroadcastInitialValues()
{
	
}

void UMGWidgetController::BindCallbackToDependencies()
{
}
