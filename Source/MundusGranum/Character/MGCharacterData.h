// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MGPawnData.h"
#include "MGCharacterData.generated.h"

/**
 * 
 */
UCLASS(BlueprintType)
class MUNDUSGRANUM_API UMGCharacterData : public UMGPawnData
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FText CharacterName;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<USkeletalMesh> CharacterMesh;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UAnimInstance> Anim;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<TSoftObjectPtr<UMGItemDefinition>> DropsAfterDeath;
};
