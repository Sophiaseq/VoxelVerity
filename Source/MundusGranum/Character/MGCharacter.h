// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/Character.h"
#include "MGCharacter.generated.h"

class UMGAbilitySystemComponent;
class UMGPawnExtensionComponent;

UCLASS(Config = Game)
class MUNDUSGRANUM_API AMGCharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	
	AMGCharacter();
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	UFUNCTION(BlueprintCallable, Category = "MundusGranum|PlayerState")
	UMGAbilitySystemComponent* GetMGAbilitySystemComponent() const;
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	
protected:
	virtual void BeginPlay() override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;
	
	//Likely to be modified
	void InitAbilityActorInfo();
	
private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MundusGranum|Character", Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UMGPawnExtensionComponent> PawnExtComponent;
	
};
