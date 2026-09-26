// Fill out your copyright notice in the Description page of Project Settings.


#include "MGGameplayAbility_MeleeAttack.h"

#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "AbilitySystem/MGAbilitySystemComponent.h"
#include "Interaction/CombatInterface.h"
#include "Items/Weapons/MGWeaponItemDefinition.h"
#include "MundusGranumGameplayTags.h"
#include "CollisionQueryParams.h"
#include "CollisionShape.h"
#include "AbilitySystem/AbilityTasks/AbilityTask_MeleeTrace.h"
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
	
	if (!ActorInfo || !ActorInfo->AbilitySystemComponent.IsValid()) return false;
	
	const UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();
	
	if (!ASC->HasMatchingGameplayTag(MundusGranumGameplayTags::ItemCategory_Weapon)) return false;
	
	// 僵直状态下不能发起攻击
	if (ASC->HasMatchingGameplayTag(MundusGranumGameplayTags::CharacterState_Rigidity_SelfAction.GetTag().RequestDirectParent()))
	{
		return false;
	}
	
	ICombatInterface* CombatInterface = Cast<ICombatInterface>(GetAvatarActorFromActorInfo());
	const UMGWeaponItemDefinition* WeaponDef = CombatInterface ? CombatInterface->GetCurrentWeapon() : nullptr;
	if (!WeaponDef->MeleeCombos) return false;
	
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	return true;
}

void UMGGameplayAbility_MeleeAttack::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	if (UMGAbilitySystemComponent* MGASC = Cast<UMGAbilitySystemComponent>(GetAbilitySystemComponentFromActorInfo()))
	{
		for (auto& InputTag : MGASC->GetCachedInputTag())
		{
			MeleeInputTags.Add(InputTag);
			MGASC->RemoveTagFromCachedInputTag(InputTag);
		}
	}
	
	ICombatInterface* CombatInterface = Cast<ICombatInterface>(GetAvatarActorFromActorInfo());
	const UMGWeaponItemDefinition* WeaponDef = CombatInterface ? CombatInterface->GetCurrentWeapon() : nullptr;
	
	MeleeCombos = WeaponDef->MeleeCombos;
	
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	/*UAbilityTask_MeleeTrace* MeleeTask = UAbilityTask_MeleeTrace::MeleeTrace(this, MundusGranumGameplayTags::CharacterState_Rigidity_SelfAction, FVector(5,5,0));
	MeleeTask->ReadyForActivation();
	
	if (!TryAdvanceCombo())
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
	}*/
}

void UMGGameplayAbility_MeleeAttack::InputPressed(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
	if (!MeleeCombos || !AttackMontage)
    {
		return;
	}

	// 僵直状态下不能继续连招（不接受 MontageJumpToSection），只能被受击打断
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
	{
		if (ASC->HasMatchingGameplayTag(MundusGranumGameplayTags::CharacterState_Rigidity_SelfAction.GetTag().RequestDirectParent()))
		{
			return;
		}
		if (UMGAbilitySystemComponent* MGASC = Cast<UMGAbilitySystemComponent>(ASC))
		{
			FGameplayTagContainer Tags = MGASC->GetCachedInputTag();
			for (auto& InputTag : Tags)
			{
				MeleeInputTags.Add(InputTag);
				MGASC->RemoveTagFromCachedInputTag(InputTag);
			}
		}
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

bool UMGGameplayAbility_MeleeAttack::TryAdvanceCombo()
{
	FMeleeComboSection NextSection;
	if (!MeleeCombos || !MeleeCombos->GetSectionForSequence(MeleeInputTags, NextSection))
	{
		return false;
	}

	/*if (!MontageTask)
	{
		// 首次：直接从该段开始播放
		PlaySection(NextSection);
	}*/
	else
	{
		MontageJumpToSection(NextSection.SectionName);
	}

	// 记录下一段为“当前”，并结算该段命中（简化：排队时即结算）
	CurrentSection = NextSection;

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
