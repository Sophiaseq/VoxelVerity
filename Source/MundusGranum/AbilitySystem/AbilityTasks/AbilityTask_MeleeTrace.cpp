// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilityTask_MeleeTrace.h"

#include "AbilitySystemComponent.h"
#include "Interaction/CombatInterface.h"

UAbilityTask_MeleeTrace* UAbilityTask_MeleeTrace::MeleeTrace(UGameplayAbility* OwningAbility, FGameplayTag ActivationTag, FVector InBoxHalfExtent)
{
	UAbilityTask_MeleeTrace* MyObj = NewAbilityTask<UAbilityTask_MeleeTrace>(OwningAbility);
	MyObj->OnActivatedTag = ActivationTag;
	MyObj->BoxHalfExtent = InBoxHalfExtent;
	return MyObj;
}

void UAbilityTask_MeleeTrace::Activate()
{
	Super::Activate();
	
	UAbilitySystemComponent* ASC = AbilitySystemComponent.Get();
	if (!ASC)
	{
		EndTask();
		return;
	}
	
	TagEventHandle = ASC->RegisterGameplayTagEvent(
		OnActivatedTag,
		EGameplayTagEventType::AnyCountChange)
		.AddUObject(this, &UAbilityTask_MeleeTrace::OnTagChanged);
	
	const int32 CurrentCount = ASC->GetTagCount(OnActivatedTag);
	OnTagChanged(OnActivatedTag, CurrentCount);

	bTickingTask = true;
}

void UAbilityTask_MeleeTrace::TickTask(float DeltaTime)
{
	Super::TickTask(DeltaTime);
	
	if (!bWindowOpen)
	{
		return;
	}

	PerformTrace();
}

void UAbilityTask_MeleeTrace::OnDestroy(bool bInOwnerFinished)
{
	if (UAbilitySystemComponent* ASC = AbilitySystemComponent.Get())
	{
		ASC->RegisterGameplayTagEvent(OnActivatedTag, EGameplayTagEventType::AnyCountChange)
			.Remove(TagEventHandle);
	}

	Super::OnDestroy(bInOwnerFinished);
}

void UAbilityTask_MeleeTrace::OnTagChanged(FGameplayTag tag, int32 Count)
{
	const bool bWasOpen = Count > 0;
	bWindowOpen = bWasOpen;
	if (bWasOpen)
	{
		// 窗口刚打开，重置状态
		bHasLast = false;
		HitActors.Reset();
	}
	else if (!bWasOpen)
	{
		// 窗口刚关闭
		OnAttackComplete.Broadcast(TArray<FHitResult>());
	}
}

void UAbilityTask_MeleeTrace::PerformTrace()
{
	AActor* Avatar = Ability->GetAvatarActorFromActorInfo();
	if (!Avatar) return;

	ICombatInterface* CombatInterface = Cast<ICombatInterface>(Avatar);
	if (!CombatInterface) return;
	
	FVector CurrentStart = CombatInterface->GetSocketLocation("Component.Mesh.Weapon", "TraceStart");
	FVector CurrentEnd = CombatInterface->GetSocketLocation("Component.Mesh.Weapon","TraceEnd");
	
	if (!bHasLast)
	{
		LastStart = CurrentStart;
		LastEnd = CurrentEnd;
		bHasLast = true;
		return;
	}

	// 用上一帧到这一帧的位移做扫掠
	FQuat Orientation = (CurrentEnd - CurrentStart).GetSafeNormal().Rotation().Quaternion();

	TArray<FHitResult> Hits;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Avatar);
	Params.bTraceComplex = false;

	GetWorld()->SweepMultiByChannel(
		Hits,
		LastStart,
		CurrentStart,
		Orientation,
		ECC_Pawn,
		FCollisionShape::MakeBox(BoxHalfExtent),
		Params);

	TArray<FHitResult> NewHits;
	for (const FHitResult& Hit : Hits)
	{
		AActor* HitActor = Hit.GetActor();
		if (!HitActor) continue;

		TWeakObjectPtr<AActor> Key(HitActor);
		if (HitActors.Contains(Key)) continue;

		HitActors.Add(Key);
		NewHits.Add(Hit);
	}

	if (NewHits.Num() > 0)
	{
		OnHit.Broadcast(NewHits);
	}

	LastStart = CurrentStart;
	LastEnd = CurrentEnd;
}
