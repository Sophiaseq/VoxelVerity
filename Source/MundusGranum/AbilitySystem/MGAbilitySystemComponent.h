// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "NativeGameplayTags.h"
#include "MGAbilitySystemComponent.generated.h"

class UMGAbilityTagRelationshipMapping;

DECLARE_MULTICAST_DELEGATE_OneParam(FEffectAssetTags, FGameplayTagContainer& /*AssetTags*/);

MUNDUSGRANUM_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Gameplay_AbilityInputBlocked);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MUNDUSGRANUM_API UMGAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:
	FEffectAssetTags EffectAssetTags;
	
	UMGAbilitySystemComponent();
	
	//绑定AbilitySystemComponent中的一些委托
	void AbilityActorInfoSet();
	
	void SetTagRelationshipMapping(UMGAbilityTagRelationshipMapping* NewMapping);
	
	void ProcessAbilityInput(float DeltaTime, bool bGamePaused);
	void ClearAbilityInput();
	
	void AbilityInputTagPressed(const FGameplayTag& InputTag);
	void AbilityInputTagReleased(const FGameplayTag& InputTag);

	/** 本帧被按下的输入标签（保序、可重复），供激活中的技能读取（如近战连招分支） */
	const TArray<FGameplayTag>& GetPressedInputTags() const { return PressedInputTags; }

protected:
	void EffectApplied(UAbilitySystemComponent* AbilitySystemComponent, const FGameplayEffectSpec& EffectSpec, FActiveGameplayEffectHandle ActiveEffectHandle);

	// If set, this table is used to look up tag relationships for activate and cancel
	UPROPERTY()
	TObjectPtr<UMGAbilityTagRelationshipMapping> TagRelationshipMapping;

	// Handles to abilities that had their input pressed this frame.
	TArray<FGameplayAbilitySpecHandle> InputPressedSpecHandles;

	// Handles to abilities that had their input released this frame.
	TArray<FGameplayAbilitySpecHandle> InputReleasedSpecHandles;

	// Handles to abilities that have their input held.
	TArray<FGameplayAbilitySpecHandle> InputHeldSpecHandles;

	// Number of abilities running in each activation group.
	//TODO 为激活的技能分类并写成数组
	int32 ActivationGroupCounts;

	// 本帧按下的输入标签缓存（保序、可重复；ProcessAbilityInput 结束时清空）
	TArray<FGameplayTag> PressedInputTags;

};
