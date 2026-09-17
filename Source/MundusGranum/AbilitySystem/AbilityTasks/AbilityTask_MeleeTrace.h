// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "AbilityTask_MeleeTrace.generated.h"

/**
 * 
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMeleeTraceHitDelegate, const TArray<FHitResult>&, Hits);

UCLASS()
class MUNDUSGRANUM_API UAbilityTask_MeleeTrace : public UAbilityTask
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, meta = (DisplayName ="MeleeTrace", HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "true"), Category = "Ability|Tasks")
	static UAbilityTask_MeleeTrace* MeleeTrace(UGameplayAbility* OwningAbility, FGameplayTag ActivationTag, FVector InBoxHalfExtent);
	
	virtual void TickTask(float DeltaTime) override;
	virtual void OnDestroy(bool bInOwnerFinished) override;
	
	UPROPERTY(BlueprintAssignable)
	FMeleeTraceHitDelegate OnHit;

	UPROPERTY(BlueprintAssignable)
	FMeleeTraceHitDelegate OnAttackComplete;
	
protected:
	virtual void Activate() override;
	
	void OnTagChanged(FGameplayTag tag, int32 Count);
	void PerformTrace();
	
	FGameplayTag OnActivatedTag;
	
	FVector BoxHalfExtent;
	
	FVector LastStart;
	FVector LastEnd;
	
	bool bHasLast = true;
	bool bWindowOpen = false;
	
	TSet<TWeakObjectPtr<AActor>> HitActors;
	
	FDelegateHandle TagEventHandle;
};
