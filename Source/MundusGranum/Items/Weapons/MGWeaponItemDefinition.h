// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Items/MGItemDefinition.h"
#include "MGWeaponItemDefinition.generated.h"

class UMeleeCombos;
class UMGAbilitySet;

USTRUCT(BlueprintType)
struct FBoxCollisionInfo
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere)
	FVector BoxExtent = { 100.0f, 100.0f, 100.0f };
	
	UPROPERTY(EditAnywhere)
	FVector Offset = { 0.0f, 0.0f, 0.0f };
	
	UPROPERTY(EditAnywhere)
	FVector TraceStartOffset = { 0.0f, 0.0f, 0.0f };
	
	UPROPERTY(EditAnywhere)
	FVector TraceEndOffset = { 0.0f, 0.0f, 0.0f };
};

USTRUCT(BlueprintType)
struct FWeaponAttributes
{
	GENERATED_BODY()
	
	//锐利度(降低目标防御，最终伤害与目标防御和来源力量有关)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float Sharpness = 10.0f;
	
	//质量(攻击造成的僵直)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float Quality = 1.0f;
};

UCLASS(BlueprintType)
class MUNDUSGRANUM_API UMGWeaponItemDefinition : public UMGItemDefinition
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly)
	FBoxCollisionInfo CollisionTransform;
	
	UPROPERTY(EditDefaultsOnly)
	FWeaponAttributes WeaponAttributes;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<UAnimMontage> Montage;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<UMeleeCombos> MeleeCombos;
};