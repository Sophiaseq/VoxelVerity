// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameplayEffect.h"
#include "ModularCharacter.h"
#include "Interaction/CombatInterface.h"
#include "MGCharacter.generated.h"

class UMGHealthComponent;
class UMGAbilitySystemComponent;
class UMGPawnExtensionComponent;

UCLASS(Blueprintable, Config = Game)
class MUNDUSGRANUM_API AMGCharacter : public AModularCharacter, public IAbilitySystemInterface, public ICombatInterface
{
	GENERATED_BODY()

public:
	
	AMGCharacter();
	virtual void Tick(float DeltaTime) override;

	/** 切换冲刺状态：本地立即生效，客户端会发 RPC 给服务器做权威修改 */
	void SetSprinting(bool bSprinting);

	UFUNCTION(Server, Reliable)
	void ServerSetSprinting(bool bSprinting);

	UFUNCTION(BlueprintCallable, Category = "MundusGranum|PlayerState")
	UMGAbilitySystemComponent* GetMGAbilitySystemComponent() const;
	
	//~Begin IAbilitySystemInterface
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	//~End IAbilitySystemInterface
	
	//~Begin ICombatInterface
	virtual float GetCharacterLevel() override;
	virtual UMGWeaponItemDefinition* GetCurrentWeapon() const override;//TODO 或许可以改成通过GameplayTag
	virtual FVector GetSocketLocation(FName TagName, FName SocketName) const override;
	virtual UAnimMontage* GetHitReactMontage() const override;
	//~End ICombatInterface
	
	// Begins the death sequence for the character (disables collision, disables movement, etc...)
	UFUNCTION()
	virtual void OnDeathStarted(AActor* OwningActor);

	// Ends the death sequence for the character (detaches controller, destroys pawn, etc...)
	UFUNCTION()
	virtual void OnDeathFinished(AActor* OwningActor);
	
protected:
	void InitializeGameplayTags();
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
	
	void DisableMovementAndCollision();
	void DestroyDueToDeath();
	void UninitAndDestroy();
	
	// Called when the death sequence for the character has completed
	UFUNCTION(BlueprintImplementableEvent, meta=(DisplayName="OnDeathFinished"))
	void K2_OnDeathFinished();

	//暂时没用
	void SetupAttributeByLevel(TSubclassOf<UGameplayEffect> AttributeEffect, float Level) const;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MundusGranum|Movement")
	float SprintSpeed = 600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MundusGranum|Movement")
	float WalkSpeed = 230.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MundusGranum|Character", Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UMGHealthComponent> HealthComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MundusGranum|Character", Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UMGPawnExtensionComponent> PawnExtComponent;
};
