// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "MGAIController.generated.h"

class UBehaviorTreeComponent;

UCLASS()
class MUNDUSGRANUM_API AMGAIController : public AAIController
{
	GENERATED_BODY()

public:
	AMGAIController();
	
protected:
	UPROPERTY()
	TObjectPtr<UBehaviorTreeComponent> BTComp;
	
private:
	
};
