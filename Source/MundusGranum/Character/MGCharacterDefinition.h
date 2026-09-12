// AnimalDefinition.h
#pragma once
#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "MGCharacterDefinition.generated.h"

class UMGAbilityTagRelationshipMapping;
class UMGInputConfig;
class UMGItemDefinition;

USTRUCT(BlueprintType)
struct FCapsuleCollisionConfig
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, Category = "Collision")
	float CapsuleRadius = 20.f;
	
	UPROPERTY(EditAnywhere, Category = "Collision")
	float CapsuleHalfHeight = 50.f;

	UPROPERTY(EditAnywhere, Category = "Collision")
	FTransform RelativeTransform = FTransform::Identity;
};

UCLASS(BlueprintType)
class MUNDUSGRANUM_API UMGCharacterDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FText CharacterName;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MundusGranum|Pawn")
	TSubclassOf<APawn> PawnClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MundusGranum|Input")
	TObjectPtr<UMGInputConfig> InputConfig;
	
	/*UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MundusGranum|Abilities")
	TArray<TObjectPtr<UMGAbilitySet>> AbilitySets;*/
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MundusGranum|Abilities")
	TObjectPtr<UMGAbilityTagRelationshipMapping> TagRelationshipMapping;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<TSoftObjectPtr<UMGItemDefinition>> DropItemOnDeath;
	
	virtual FPrimaryAssetId GetPrimaryAssetId() const override
	{
		return FPrimaryAssetId("CharacterDefinition", GetFName());
	}
};