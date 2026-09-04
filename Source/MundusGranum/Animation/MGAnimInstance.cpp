// Fill out your copyright notice in the Description page of Project Settings.


#include "MGAnimInstance.h"

#include "Character/MGCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

void UMGAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	MGCharacter = Cast<AMGCharacter>(TryGetPawnOwner());
	if (MGCharacter)
	{
		MGCharacterMovement = MGCharacter->GetCharacterMovement();
	}
}

void UMGAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);
	if (MGCharacterMovement)
	{
		GroudSpeed = UKismetMathLibrary::VSizeXY(MGCharacterMovement->Velocity);
		IsFalling = MGCharacterMovement->IsFalling();
		//CharacterState = MGCharacter->GetCharacterState();
		//ActionState = MGCharacter->GetActionState();
		//CharacterDirection = MGCharacter->GetMovementDirection();
	}
}

