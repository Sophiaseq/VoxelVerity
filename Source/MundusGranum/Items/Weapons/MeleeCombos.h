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

	bool FindMatchingCombos(const TArray<FGameplayTag>& InputTags, TArray<FMeleeCombo>& OutCombos) const;
	
	// 给定输入标签序列，返回“最后一次输入”对应的连招片段（序列长度=1 时返回第一个片段）
	bool GetSectionForSequence(const TArray<FGameplayTag>& InputSequence, FMeleeComboSection& OutSection) const;
};
