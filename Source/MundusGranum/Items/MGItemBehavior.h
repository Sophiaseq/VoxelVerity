// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "MGItemBehavior.generated.h"

/**
 * 
 */
UCLASS(Blueprintable)
class MUNDUSGRANUM_API AMGItemBehavior : public AActor
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintImplementableEvent)
	void CreateFields(const FVector& FieldLocation);
};
