// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameplayEffect.h"
#include "ModularCharacter.h"
#include "Interaction/CombatInterface.h"
#include "MGCharacter.generated.h"

class UMGAbilitySystemComponent;
class UMGPawnExtensionComponent;

UCLASS(Blueprintable, Config = Game)
class MUNDUSGRANUM_API AMGCharacter : public AModularCharacter, public IAbilitySystemInterface, public ICombatInterface
{
	GENERATED_BODY()

public:
	
	AMGCharacter();
	virtual void Tick(float DeltaTime) override;
	
	UFUNCTION(BlueprintCallable, Category = "MundusGranum|PlayerState")
	UMGAbilitySystemComponent* GetMGAbilitySystemComponent() const;
	
	//~Begin IAbilitySystemInterface
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	//~End IAbilitySystemInterface
	
	//~Begin ICombatInterface
	virtual float GetCharacterLevel() override;
	virtual UMGWeaponItemDefinition* GetCurrentWeapon() const override;//TODO 或许可以改成通过GameplayTag
	virtual FVector GetSocketLocation() const override;
	//~End ICombatInterface
	
protected:
	virtual void OnAbilitySystemInitialized();
	virtual void OnAbilitySystemUninitialized();
	
	virtual void PostInitializeComponents() override;
	virtual void BeginPlay() override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void UnPossessed() override;
	virtual void OnRep_PlayerState() override;
	virtual void OnRep_Controller() override;
;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	
	//TODO 由Experience来做
	void SetupInitialAttribute() const;
	
	//TODO 移交给Experience，将HealthSet交给HealthComponent
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Attribute", meta = (ToolTip = "将PrimaryAttribute放在最开始"))
	TArray<TSubclassOf<UGameplayEffect>> DefaultAttributes;

private:
	void SetupAttributeByLevel(TSubclassOf<UGameplayEffect> AttributeEffect, float Level) const;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MundusGranum|Character", Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UMGPawnExtensionComponent> PawnExtComponent;
	
};
