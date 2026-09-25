// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ModularGameMode.h"
#include "Character/MGPawnData.h"
#include "MGGameMode.generated.h"

class UMGExperienceDefinition;
/**
 * 
 */
UCLASS(Config=Game)
class MUNDUSGRANUM_API AMGGameMode : public AModularGameModeBase
{
	GENERATED_BODY()
	
public:
	AMGGameMode();
	
	UFUNCTION(BlueprintCallable, Category = "MundusGranum|Pawn")
	const UMGPawnData* GetPawnDataForController(const AController* InController) const;
	
	//~AGameModeBase interface
	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
	virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;
	virtual UClass* GetDefaultPawnClassForController_Implementation(AController* InController) override;
	virtual APawn* SpawnDefaultPawnAtTransform_Implementation(AController* NewPlayer, const FTransform& SpawnTransform) override;
	virtual void InitGameState() override;
	//~End of AGameModeBase interface
	
	// 让所有的TargetPoint根据它们携带的PawnData生成pawn
	UFUNCTION(BlueprintCallable, Category = "MundusGranum|Pawn")
	void SpawnAllCharactersFromSpawnPoints();
protected:
	void OnExperienceLoaded(const UMGExperienceDefinition* CurrentExperience);
	bool IsExperienceLoaded() const;
	
	void OnMatchAssignmentGiven(FPrimaryAssetId ExperienceId, const FString& ExperienceIdSource);
	void HandleMatchAssignmentIfNotExpectingOne();
};
