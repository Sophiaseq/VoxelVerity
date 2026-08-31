// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "MGInputConfig.generated.h"

class UInputAction;

/**
 * 
 */
USTRUCT(BlueprintType)
struct FMGInputAction
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere)
	FGameplayTag InputTag;
	
	UPROPERTY(EditAnywhere)
	UInputAction* InputAction = nullptr;
};

UCLASS()
class MUNDUSGRANUM_API UMGInputConfig : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UMGInputConfig(const FObjectInitializer& ObjectInitializer);
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Meta = (TitleProperty = "InputAction"))
	TArray<FMGInputAction> TaggedInputActions;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Meta = (TitleProperty = "InputAction"))
	TArray<FMGInputAction> AbilityInputActions;
	
	UFUNCTION(BlueprintCallable, Category = "MundusGranum|Pawn")
	const UInputAction* FindAbilityInputActionForTag(const FGameplayTag& InputTag, bool bLogNotFound = true) const;
	
	UFUNCTION(BlueprintCallable, Category = "MundusGranum|Pawn")
	const UInputAction* FindActionByTag(const FGameplayTag& InputTag, bool bLogMissing = false) const;
};
