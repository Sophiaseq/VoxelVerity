// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ModularPlayerController.h"
#include "GameFramework/PlayerController.h"
#include "UI/Widget/DamageTextComponent.h"
#include "MGPlayerController.generated.h"

class UMGAbilitySystemComponent;
class AMGPlayerState;
struct FInputActionValue;
class UMGInputConfig;
class UInputMappingContext;
/**
 * 
 */
UCLASS(Config = Game, Meta = (ShortTooltip = "The base player controller class used by this project."))
class MUNDUSGRANUM_API AMGPlayerController : public AModularPlayerController
{
	GENERATED_BODY()
	
public:
	AMGPlayerController();
	
	UFUNCTION(BlueprintCallable, Category = "MundusGranum|PlayerController")
	AMGPlayerState* GetMGPlayerState() const;
	
	UFUNCTION(BlueprintCallable, Category = "MundusGranum|PlayerController")
	UMGAbilitySystemComponent* GetMGAbilitySystemComponent() const;
	
	//~AActor interface
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	//~End of AActor interface
	
	//~AController interface
	virtual void InitPlayerState() override;
	virtual void CleanupPlayerState() override;
	virtual void OnRep_PlayerState() override;
	//~End of AController interface

	//~APlayerController interface
	virtual void SetPlayer(UPlayer* InPlayer) override;
	virtual void PreProcessInput(const float DeltaTime, const bool bGamePaused) override;
	virtual void PostProcessInput(const float DeltaTime, const bool bGamePaused) override;
	//~End of APlayerController interface

	UFUNCTION(Client, Unreliable, BlueprintCallable)
	void ShowDamageNumber(float DamageAmount, const FVector& WidgetSpawnLocation);
	
protected:
	virtual void BeginPlay() override;
	
	// Called when the player state is set or cleared
	virtual void OnPlayerStateChanged();
	
	void InitHUD();

private:
	void BroadcastOnPlayerStateChanged();
	
	UPROPERTY()
	TObjectPtr<APlayerState> LastSeenPlayerState;
	
	UPROPERTY(EditDefaultsOnly, Category = "MundusGranum|PlayerController")
	TSubclassOf<UDamageTextComponent> DamageTextComponentClass;

};
