// Fill out your copyright notice in the Description page of Project Settings.


#include "MGUserWidget.h"

void UMGUserWidget::SetWidgetController(UObject* InWidgetController)
{
	WidgetController = InWidgetController;
	WidgetControllerSet();
}
