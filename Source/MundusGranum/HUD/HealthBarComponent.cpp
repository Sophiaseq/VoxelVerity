// Fill out your copyright notice in the Description page of Project Settings.


#include "HealthBarComponent.h"

#include "Components/ProgressBar.h"
#include "HUD/HealthBar.h"


void UHealthBarComponent::SetHealthPercent(float Percent)
{
	if (!HealthBarWidget)
	{
		HealthBarWidget = Cast<UHealthBar>(GetUserWidgetObject());
	}
	if (HealthBarWidget && HealthBarWidget->HealthBar)
	{
		HealthBarWidget->HealthBar->SetPercent(Percent);
	}
}
