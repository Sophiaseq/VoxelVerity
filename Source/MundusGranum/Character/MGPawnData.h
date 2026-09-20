// AnimalDefinition.h
#pragma once
#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "MGPawnData.generated.h"

class UMGAbilitySet;
class UMGAbilityTagRelationshipMapping;
class UMGInputConfig;
class UMGItemDefinition;

UCLASS(BlueprintType)
class MUNDUSGRANUM_API UMGPawnData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MundusGranum|Pawn")
	TSubclassOf<APawn> PawnClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MundusGranum|Input")
	TObjectPtr<UMGInputConfig> InputConfig;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MundusGranum|Abilities")
	TArray<TObjectPtr<UMGAbilitySet>> AbilitySets;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MundusGranum|Abilities")
	TObjectPtr<UMGAbilityTagRelationshipMapping> TagRelationshipMapping;
	
	//很可能需要换个位置
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MundusGranum|Anim")
	TObjectPtr<UAnimMontage> HitReactMontage;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MundusGranum|Damage")
	TObjectPtr<UCurveTable> DamageCalculationCoefficients;
	
	virtual FPrimaryAssetId GetPrimaryAssetId() const override
	{
		return FPrimaryAssetId("PawnData", GetFName());
	}
};