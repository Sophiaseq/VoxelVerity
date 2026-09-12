// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "AbilitySystemComponent.h"
#include "MGAttributeSet.h"
#include "MGPrimarySet.generated.h"

/**
 * 
 */
UCLASS()
class MUNDUSGRANUM_API UMGPrimarySet : public UMGAttributeSet
{
	GENERATED_BODY()
	
public:
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
	ATTRIBUTE_ACCESSORS_BASIC(UMGPrimarySet, Strength)        // 力量
	ATTRIBUTE_ACCESSORS_BASIC(UMGPrimarySet, Dexterity)       // 敏捷
	ATTRIBUTE_ACCESSORS_BASIC(UMGPrimarySet, Constitution)    // 体质
	ATTRIBUTE_ACCESSORS_BASIC(UMGPrimarySet, Intelligence)    // 智力
	ATTRIBUTE_ACCESSORS_BASIC(UMGPrimarySet, Perception)      // 感知
	ATTRIBUTE_ACCESSORS_BASIC(UMGPrimarySet, Luck)            // 运气
	
protected:
	UFUNCTION()
	virtual void OnRep_Strength(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	virtual void OnRep_Dexterity(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	virtual void OnRep_Constitution(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	virtual void OnRep_Intelligence(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	virtual void OnRep_Perception(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	virtual void OnRep_Luck(const FGameplayAttributeData& OldValue);

private:
	UPROPERTY(BlueprintReadOnly, Category = "Primary|Strength", ReplicatedUsing = OnRep_Strength, meta = (AllowPrivateAccess))
	FGameplayAttributeData Strength;

	UPROPERTY(BlueprintReadOnly, Category = "Primary|Dexterity", ReplicatedUsing = OnRep_Dexterity, meta = (AllowPrivateAccess))
	FGameplayAttributeData Dexterity;

	UPROPERTY(BlueprintReadOnly, Category = "Primary|Constitution", ReplicatedUsing = OnRep_Constitution, meta = (AllowPrivateAccess))
	FGameplayAttributeData Constitution;

	UPROPERTY(BlueprintReadOnly, Category = "Primary|Intelligence", ReplicatedUsing = OnRep_Intelligence, meta = (AllowPrivateAccess))
	FGameplayAttributeData Intelligence;

	UPROPERTY(BlueprintReadOnly, Category = "Primary|Perception", ReplicatedUsing = OnRep_Perception, meta = (AllowPrivateAccess))
	FGameplayAttributeData Perception;

	UPROPERTY(BlueprintReadOnly, Category = "Primary|Luck", ReplicatedUsing = OnRep_Luck, meta = (AllowPrivateAccess))
	FGameplayAttributeData Luck;
};
