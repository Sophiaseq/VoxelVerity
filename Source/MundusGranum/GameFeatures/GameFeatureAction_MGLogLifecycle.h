// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFeatureAction.h"
#include "GameFeatureAction_MGLogLifecycle.generated.h"

/**
 * 
 */
UCLASS()
class MUNDUSGRANUM_API UGameFeatureAction_MGLogLifecycle final : public UGameFeatureAction
{
	GENERATED_BODY()
	
public:
	virtual void OnGameFeatureRegistering() override;
	virtual void OnGameFeatureUnregistering() override;
	virtual void OnGameFeatureLoading() override;
	virtual void OnGameFeatureActivating(FGameFeatureActivatingContext& Context) override;
	virtual void OnGameFeatureDeactivating(FGameFeatureDeactivatingContext& Context) override;
};
