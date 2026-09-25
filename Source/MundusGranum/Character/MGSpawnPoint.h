// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/TargetPoint.h"
#include "Items/Weapons/MGWeaponItemDefinition.h"
#include "MGSpawnPoint.generated.h"

class UMGPawnData;

UCLASS()
class MUNDUSGRANUM_API AMGSpawnPoint : public ATargetPoint
{
	GENERATED_BODY()

public:
	void SpawnCharacter();

protected:
	virtual void BeginPlay() override;
	
	APawn* PerformSpawn();
	void SpawnItem(ACharacter* CharacterToAttach) const;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn")
	TObjectPtr<const UMGPawnData> PawnData;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn")
	TObjectPtr<const UMGWeaponItemDefinition> WeaponDef;
	
	//如果要生成多个可以改成数组
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Spawn")
	TObjectPtr<APawn> SpawnedPawn;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn")
	bool bAutoSpawnOnBeginPlay = true;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn", meta = (ClampMin = "0.0"))
	float SpawnDelay = 0.0f;
	
};
