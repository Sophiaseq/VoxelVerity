// Fill out your copyright notice in the Description page of Project Settings.


#include "MGGameplayAbility_MeleeAttack.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "AbilitySystem/MGAbilitySystemComponent.h"
#include "Character/MGCharacter.h"
#include "Interaction/CombatInterface.h"
#include "Items/Weapons/MGWeaponItemDefinition.h"
#include "MGLogChannels.h"
#include "MundusGranumGameplayTags.h"
#include "CollisionQueryParams.h"
#include "CollisionShape.h"
#include "Engine/World.h"

UMGGameplayAbility_MeleeAttack::UMGGameplayAbility_MeleeAttack()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

bool UMGGameplayAbility_MeleeAttack::CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags,
	const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	// 僵直状态下不能发起攻击（只能被受击打断）
	if (ActorInfo && ActorInfo->AbilitySystemComponent.IsValid() &&
		ActorInfo->AbilitySystemComponent->HasMatchingGameplayTag(MundusGranumGameplayTags::CharacterState_Rigidity))
	{
		return false;
	}

	return true;
}

void UMGGameplayAbility_MeleeAttack::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	// 1. 拿到当前武器定义（蒙太奇 + 连招数据）
	ICombatInterface* CombatInterface = Cast<ICombatInterface>(GetAvatarActorFromActorInfo());
	UMGWeaponItemDefinition* WeaponDef = CombatInterface ? CombatInterface->GetCurrentWeapon() : nullptr;
	if (!WeaponDef || !WeaponDef->Montage || !WeaponDef->MeleeCombos)
	{
		UE_LOG(LogMG, Warning, TEXT("[MeleeAttack] 无武器 / 无蒙太奇 / 无连招数据，无法激活"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	MeleeCombos = WeaponDef->MeleeCombos;
	AttackMontage = WeaponDef->Montage;

	// 2. 读取本次激活所按下的输入标签（左右键 -> UseLeftHandItem / UseRightHandItem）
	MeleeInputTags.Reset();
	AppendPressedInputTag();

	// 3. 用初始输入序列播放第一段（失败则直接结束）
	if (!TryAdvanceCombo())
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
	}
}

void UMGGameplayAbility_MeleeAttack::InputPressed(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
	if (!MeleeCombos || !AttackMontage)
	{
		return;
	}

	// 僵直状态下不能继续连招（不接受 MontageJumpToSection），只能被受击打断
	if (const UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
	{
		if (ASC->HasMatchingGameplayTag(MundusGranumGameplayTags::CharacterState_Rigidity))
		{
			return;
		}
	}

	// 追加本帧按下的、与本技能输入相关的标签（左右键 -> UseLeftHandItem / UseRightHandItem）
	if (!AppendPressedInputTag())
	{
		return;
	}

	// 用新的输入序列推进连招（成功则跳到下一段）
	TryAdvanceCombo();
}

void UMGGameplayAbility_MeleeAttack::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	MontageTask = nullptr;
	AttackMontage = nullptr;
	MeleeCombos = nullptr;
	MeleeInputTags.Reset();

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

bool UMGGameplayAbility_MeleeAttack::AppendPressedInputTag()
{
	const FGameplayAbilitySpec* Spec = GetCurrentAbilitySpec();
	const UMGAbilitySystemComponent* MGASC = Cast<UMGAbilitySystemComponent>(GetAbilitySystemComponentFromActorInfo());
	if (!Spec || !MGASC)
	{
		return false;
	}

	// 本技能允许的输入标签集合（AbilitySet 里授权的 InputTags）
	const FGameplayTagContainer& AbilityInputTags = Spec->GetDynamicSpecSourceTags();
	bool bAppended = false;
	for (const FGameplayTag& PressedTag : MGASC->GetPressedInputTags())
	{
		if (AbilityInputTags.HasTagExact(PressedTag))
		{
			MeleeInputTags.Add(PressedTag);
			bAppended = true;
		}
	}

	return bAppended;
}

bool UMGGameplayAbility_MeleeAttack::TryAdvanceCombo()
{
	FMeleeComboSection NextSection;
	if (!MeleeCombos || !MeleeCombos->GetSectionForSequence(MeleeInputTags, NextSection))
	{
		return false;
	}

	if (!MontageTask)
	{
		// 首次：直接从该段开始播放
		PlaySection(NextSection);
	}
	else
	{
		// 后续：链接“当前段结束 -> 下一段”，让蒙太奇在当前段播完后自动衔接（预输入缓冲）
		MontageSetNextSectionName(CurrentSection.SectionName, NextSection.SectionName);
	}

	// 记录下一段为“当前”，并结算该段命中（简化：排队时即结算）
	CurrentSection = NextSection;
	PerformMeleeTrace(NextSection);

	return true;
}

void UMGGameplayAbility_MeleeAttack::PlaySection(const FMeleeComboSection& Section)
{
	// 首次：创建蒙太奇任务并从该段开始播放
	MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this, TEXT("MeleeMontage"), AttackMontage, 1.f, Section.SectionName);

	MontageTask->OnBlendOut.AddDynamic(this, &UMGGameplayAbility_MeleeAttack::HandleMontageBlendOut);
	MontageTask->OnCompleted.AddDynamic(this, &UMGGameplayAbility_MeleeAttack::HandleMontageCompleted);
	MontageTask->OnInterrupted.AddDynamic(this, &UMGGameplayAbility_MeleeAttack::HandleMontageInterrupted);
	MontageTask->OnCancelled.AddDynamic(this, &UMGGameplayAbility_MeleeAttack::HandleMontageCancelled);

	MontageTask->ReadyForActivation();
}

void UMGGameplayAbility_MeleeAttack::PerformMeleeTrace(const FMeleeComboSection& Section)
{
	AMGCharacter* Character = GetMGCharacterFromActorInfo();
	if (!Character || !Section.DamageEffectClass)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo();
	if (!SourceASC)
	{
		return;
	}

	const FVector Start = Character->GetActorLocation();
	const FVector End = Start + Character->GetActorForwardVector() * Section.TraceLength;

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Character);

	TArray<FHitResult> Hits;
	World->SweepMultiByChannel(Hits, Start, End, FQuat::Identity, ECC_Pawn,
		FCollisionShape::MakeSphere(Section.TraceRadius), Params);

	for (const FHitResult& Hit : Hits)
	{
		AActor* Victim = Hit.GetActor();
		if (!Victim || Victim == Character)
		{
			continue;
		}

		UAbilitySystemComponent* TargetASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Victim);
		if (!TargetASC)
		{
			continue;
		}

		FGameplayEffectContextHandle Context = SourceASC->MakeEffectContext();
		Context.AddSourceObject(Character);
		const FGameplayEffectSpecHandle Spec = SourceASC->MakeOutgoingSpec(Section.DamageEffectClass, GetAbilityLevel(), Context);
		SourceASC->ApplyGameplayEffectSpecToTarget(*Spec.Data.Get(), TargetASC);
	}
}

void UMGGameplayAbility_MeleeAttack::HandleMontageBlendOut()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UMGGameplayAbility_MeleeAttack::HandleMontageCompleted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UMGGameplayAbility_MeleeAttack::HandleMontageInterrupted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UMGGameplayAbility_MeleeAttack::HandleMontageCancelled()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}
