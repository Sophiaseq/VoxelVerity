// Fill out your copyright notice in the Description page of Project Settings.

//调试绘制
#undef ENABLE_DRAW_DEBUG
#define ENABLE_DRAW_DEBUG 0

#include "AbilityTask_MeleeTrace.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "MundusGranumGameplayTags.h"
#include "Interaction/CombatInterface.h"

UAbilityTask_MeleeTrace* UAbilityTask_MeleeTrace::MeleeTrace(UGameplayAbility* OwningAbility, const FGameplayTag ActivationTag, const FVector& InBoxHalfExtent)
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

	const ICombatInterface* CombatInterface = Cast<ICombatInterface>(Avatar);
	if (!CombatInterface) return;

	const FVector CurrentStart = CombatInterface->GetSocketLocation("Component.Mesh.Weapon", "CenterTrace");
	
	if (!bHasLast)
	{
		LastStart = CurrentStart;
		bHasLast = true;
		return;
	}

	// 用上一帧到这一帧的位移做扫掠
	FQuat Orientation = (CurrentStart - LastStart).GetSafeNormal().Rotation().Quaternion();

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
	
#if ENABLE_DRAW_DEBUG
	DrawDebugSweptBox(
		GetWorld(),
		LastStart,
		CurrentStart,
		Orientation.Rotator(),
		BoxHalfExtent,
		Hits.Num() > 0 ? FColor::Green : FColor::Red,
		false, 1.f, 0);
	
	for (const FHitResult& Hit : Hits)
	{
		DrawDebugPoint(GetWorld(), Hit.ImpactPoint, 8.f, FColor::Yellow, false, -1.f, 0);
	}
#endif
	
	TArray<FHitResult> NewHits;
	for (const FHitResult& Hit : Hits)
	{
		AActor* HitActor = Hit.GetActor();
		if (!HitActor) continue;

		TWeakObjectPtr<AActor> Key(HitActor);
		if (HitActors.Contains(Key)) continue;

		SendHitSection(Hit);
		
		HitActors.Add(Key);
		NewHits.Add(Hit);
	}

	if (NewHits.Num() > 0)
	{
		OnHit.Broadcast(NewHits);
	}

	LastStart = CurrentStart;
}


void UAbilityTask_MeleeTrace::SendHitSection(const FHitResult& Hit)
{
	AActor* Avatar = GetAvatarActor();
	if (!Avatar)
	{
		return;
	}

	AActor* HitActor = Hit.GetActor();
	if (!HitActor || HitActor == Avatar)
	{
		return;
	}

	UAbilitySystemComponent* VictimASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(HitActor);
	if (!VictimASC)
	{
		return;
	}

	// 计算受击方向：攻击者相对受击者的水平方位，映射为前/后/左/右
	const FVector ToAttacker = (Avatar->GetActorLocation() - HitActor->GetActorLocation()).GetSafeNormal2D();
	const float ForwardDot = FVector::DotProduct(HitActor->GetActorForwardVector().GetSafeNormal2D(), ToAttacker);
	const float RightDot = FVector::DotProduct(HitActor->GetActorRightVector().GetSafeNormal2D(), ToAttacker);

	FGameplayTag DirectionTag;
	if (FMath::Abs(ForwardDot) >= FMath::Abs(RightDot))
	{
		DirectionTag = ForwardDot >= 0.f
			? MundusGranumGameplayTags::HitReact_Direction_Front
			: MundusGranumGameplayTags::HitReact_Direction_Back;
	}
	else
	{
		DirectionTag = RightDot >= 0.f
			? MundusGranumGameplayTags::HitReact_Direction_Right
			: MundusGranumGameplayTags::HitReact_Direction_Left;
	}

	// 构造事件，把受击方向作为目标标签随 Payload 一起发给受击者 ASC
	FGameplayEventData Payload;
	Payload.EventTag = MundusGranumGameplayTags::GameplayEvent_HitReact;
	Payload.Instigator = Avatar;
	Payload.Target = HitActor;
	Payload.TargetTags.AddTag(DirectionTag);

	FScopedPredictionWindow NewScopedWindow(VictimASC, true);
	VictimASC->HandleGameplayEvent(Payload.EventTag, &Payload);
}