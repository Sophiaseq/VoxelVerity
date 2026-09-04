// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Animation/AnimInstance.h"
#include "MGAnimInstance.generated.h"

class AMGCharacter;
class UCharacterMovementComponent;
/**
 * 
 */
UCLASS()
class MUNDUSGRANUM_API UMGAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaTime) override;
	
	UPROPERTY(BlueprintReadOnly)
	AMGCharacter* MGCharacter;
	
	UPROPERTY(BlueprintReadOnly, Category="Movement")
	UCharacterMovementComponent* MGCharacterMovement;
	
	UPROPERTY(BlueprintReadOnly, Category="Movement")
	float CharacterDirection;
	
	UPROPERTY(BlueprintReadOnly, Category="Movement")
	float GroudSpeed;
	
	UPROPERTY(BlueprintReadOnly, Category="Movement")
	bool IsFalling;
	
	UPROPERTY(BlueprintReadOnly, Category="Movement | CharacterState")
	FGameplayTag CharacterState;
	
	UPROPERTY(BlueprintReadOnly, Category="Movement |ActionState")
	FGameplayTag ActionState;
};
