// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "AnimNotifyState_AddGameplayTag.generated.h"

class UAbilitySystemComponent;
/**
 * 
 */
UCLASS(meta=(DisplayName="Add Gameplay Tag"))
class MUNDUSGRANUM_API UAnimNotifyState_AddGameplayTag : public UAnimNotifyState
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, Category="GameplayTag")
	FGameplayTag Tag;
	
	UPROPERTY(EditAnywhere, Category="GameplayTag", meta=(ClampMin="1"))
	int32 StackCount = 1;

	/** 只在服务器生效（避免客户端本地标签和服务端不一致） */
	UPROPERTY(EditAnywhere, Category="GameplayTag")
	bool bServerOnly = false;

	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
		float TotalDuration, const FAnimNotifyEventReference& EventReference) override;

	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference) override;

	virtual FString GetNotifyName_Implementation() const override;

private:
	static UAbilitySystemComponent* GetASC(const USkeletalMeshComponent* MeshComp);
	static bool ShouldApply(const UAbilitySystemComponent* ASC, bool bServerOnly);
};
