// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "MeleeCombos.generated.h"

struct FMeleeAttackDefinition;
class UGameplayEffect;
/**
 * 
 */
USTRUCT(BlueprintType)
struct FMeleeComboSection
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag InputTag;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName SectionName;
	
	// 伤害 GE
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<UGameplayEffect> DamageEffectClass;

	// 消耗 GE
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<UGameplayEffect> CostEffectClass;
	
	// 检测参数
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float TraceRadius = 20.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float TraceLength = 150.f;
};

USTRUCT(BlueprintType)
struct FMeleeCombo
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FMeleeComboSection> Combo;
};

UCLASS(BlueprintType)
class MUNDUSGRANUM_API UMeleeCombos : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FMeleeCombo> Combos;

	bool FindMatchingCombos(const FGameplayTagContainer& InputTags, TArray<FMeleeCombo>& OutCombos) const;
	
	// 获取指定连招在指定索引处的片段
	UFUNCTION(BlueprintPure, Category = "Melee")
	bool GetComboSection(const FGameplayTagContainer& InputTags, TArray<FMeleeCombo>& InCombos, FMeleeComboSection& OutSection) const;
};
