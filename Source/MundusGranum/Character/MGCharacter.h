// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "MGCharacter.generated.h"

class UMGPawnExtensionComponent;

UCLASS(Config = Game)
class MUNDUSGRANUM_API AMGCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	
	AMGCharacter();
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
protected:
	virtual void BeginPlay() override;
	
private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MundusGranum|Character", Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UMGPawnExtensionComponent> PawnExtComponent;
	
};
