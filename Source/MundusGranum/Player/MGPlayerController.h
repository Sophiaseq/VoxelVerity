// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ModularPlayerController.h"
#include "GameFramework/PlayerController.h"
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
	
	//~AController interface
	virtual void InitPlayerState() override;
	virtual void CleanupPlayerState() override;
	virtual void OnRep_PlayerState() override;
	//~End of AController interface

	//~APlayerController interface
	virtual void PostProcessInput(const float DeltaTime, const bool bGamePaused) override;
	//~End of APlayerController interface
	
protected:
	virtual void BeginPlay() override;
	
	// Called when the player state is set or cleared
	virtual void OnPlayerStateChanged();

private:
	void BroadcastOnPlayerStateChanged();
	
	UPROPERTY()
	TObjectPtr<APlayerState> LastSeenPlayerState;

};
