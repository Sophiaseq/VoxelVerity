// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "AttributeInfo.generated.h"

/**
 * 
 */
USTRUCT(Blueprintable)
struct FMGAttributeInfo
{
	GENERATED_BODY()
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag AttributeTag = FGameplayTag();
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FText AttributeName = FText();
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FText AttributeDescription = FText();
	
	UPROPERTY(BlueprintReadOnly)
	float AttributeValue = 0.0f;
};

UCLASS()
class MUNDUSGRANUM_API UAttributeInfo : public UDataAsset
{
	GENERATED_BODY()
	
public:
	FMGAttributeInfo FindAttributeInfoForTag(const FGameplayTag& AttributeTag, bool bLogNotFound = false);
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<FMGAttributeInfo> AttributeInformation;
};
