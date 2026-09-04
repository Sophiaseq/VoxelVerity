// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Character/MGCharacterDefinition.h"
#include "MGGameModeBase.generated.h"

class UMGExperienceDefinition;
/**
 * 
 */
UCLASS(Config=Game)
class MUNDUSGRANUM_API AMGGameModeBase : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	AMGGameModeBase();
	
	UFUNCTION(BlueprintCallable, Category = "MundusGranum|Pawn")
	const UMGCharacterDefinition* GetPawnDataForController(const AController* InController) const;
	
	//~AGameModeBase interface
	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
	virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;
	virtual UClass* GetDefaultPawnClassForController_Implementation(AController* InController) override;
	virtual APawn* SpawnDefaultPawnAtTransform_Implementation(AController* NewPlayer, const FTransform& SpawnTransform) override;
	virtual void InitGameState() override;
	//~End of AGameModeBase interface
	
protected:
	void OnExperienceLoaded(const UMGExperienceDefinition* CurrentExperience);
	bool IsExperienceLoaded() const;
	
	void OnMatchAssignmentGiven(FPrimaryAssetId ExperienceId, const FString& ExperienceIdSource);
	void HandleMatchAssignmentIfNotExpectingOne();
};
