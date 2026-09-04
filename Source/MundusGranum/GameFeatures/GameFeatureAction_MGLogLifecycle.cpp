// Fill out your copyright notice in the Description page of Project Settings.


#include "GameFeatureAction_MGLogLifecycle.h"

#include "GameFeaturesSubsystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GameFeatureAction_MGLogLifecycle)

#define LOCTEXT_NAMESPACE "UGameFeatures"

void UGameFeatureAction_MGLogLifecycle::OnGameFeatureRegistering()
{
	Super::OnGameFeatureRegistering();
	UE_LOG(LogGameFeatures, Log, TEXT("[MGLogLifecycle] 1. OnGameFeatureRegistering"));
}

void UGameFeatureAction_MGLogLifecycle::OnGameFeatureLoading()
{
	Super::OnGameFeatureLoading();
	UE_LOG(LogGameFeatures, Log, TEXT("[MGLogLifecycle] 2. OnGameFeatureLoading"));
}

void UGameFeatureAction_MGLogLifecycle::OnGameFeatureActivating(FGameFeatureActivatingContext& Context)
{
	Super::OnGameFeatureActivating(Context);
	UE_LOG(LogGameFeatures, Log, TEXT("[MGLogLifecycle] 3. OnGameFeatureActivating"));
}

void UGameFeatureAction_MGLogLifecycle::OnGameFeatureDeactivating(FGameFeatureDeactivatingContext& Context)
{
	Super::OnGameFeatureDeactivating(Context);
	UE_LOG(LogGameFeatures, Log, TEXT("[MGLogLifecycle] 4. OnGameFeatureDeactivating"));
}

void UGameFeatureAction_MGLogLifecycle::OnGameFeatureUnregistering()
{
	Super::OnGameFeatureUnregistering();
	UE_LOG(LogGameFeatures, Log, TEXT("[MGLogLifecycle] 5. OnGameFeatureUnregistering"));
}

#undef LOCTEXT_NAMESPACE