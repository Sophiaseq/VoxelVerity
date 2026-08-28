// Fill out your copyright notice in the Description page of Project Settings.


#include "MGAnimInstance.h"

#include "Character/MGPlayer.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

void UMGAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	MGCharacterBase = Cast<AMGPlayer>(TryGetPawnOwner());
	if (MGCharacterBase)
	{
		MGCharacterMovement = MGCharacterBase->GetCharacterMovement();
	}
}

void UMGAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);
	if (MGCharacterMovement)
	{
		GroudSpeed = UKismetMathLibrary::VSizeXY(MGCharacterMovement->Velocity);
		IsFalling = MGCharacterMovement->IsFalling();
		CharacterState = MGCharacterBase->GetCharacterState();
		ActionState = MGCharacterBase->GetActionState();
		CharacterDirection = MGCharacterBase->GetMovementDirection();
	}
}

