// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MGGameplayAbility.h"
#include "Items/Weapons/MeleeCombos.h"
#include "MGGameplayAbility_MeleeAttack.generated.h"

class UAbilityTask_PlayMontageAndWait;
class UMGWeaponItemDefinition;

/**
 * 近战连招能力
 *
 * 流程：
 *   1. 攻击输入触发本技能（输入标签挂在 Spec 的 DynamicSpecSourceTags 上）。
 *   2. ActivateAbility 读取初始输入标签 -> 用 MeleeCombos 查第一段 -> 播放蒙太奇并跳到该段。
 *   3. 播放期间再次按下攻击输入 -> InputPressed 追加输入标签 -> 查下一段 -> MontageJumpToSection。
 *   4. 蒙太奇 blend out / 完成 -> EndAbility。
 *
 * 僵直（CharacterState.Rigidity）由 AnimNotifyState_AddGameplayTag 在蒙太奇指定范围内
 * 添加/移除，本技能不再手动添加，避免僵直无法解除。
 */
UCLASS()
class MUNDUSGRANUM_API UMGGameplayAbility_MeleeAttack : public UMGGameplayAbility
{
	GENERATED_BODY()

public:
	UMGGameplayAbility_MeleeAttack();

	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void InputPressed(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
	// 播放指定片段（启动/跳转蒙太奇 + 结算该段命中）
	void PlaySection(const FMeleeComboSection& Section);

	// 用当前输入序列推进连招：成功返回 true 并跳转下一段
	bool TryAdvanceCombo();

	// 从 ASC 的本帧输入缓存中，取“与本技能输入相关”的标签追加到连招序列；成功返回 true
	bool AppendPressedInputTag();

	// 对前方做扫描并结算伤害（简化版：进入片段即结算，精确时机应改用命中窗口 Notify）
	void PerformMeleeTrace(const FMeleeComboSection& Section);
	
	// 连招输入序列（每次攻击输入追加一个 InputTag，保序可重复，用于匹配连招分支）
	TArray<FGameplayTag> MeleeInputTags;

	// 当前武器连招数据
	UPROPERTY(Transient)
	TObjectPtr<UMeleeCombos> MeleeCombos;

	// 本次连招要播放的蒙太奇（武器定义上取）
	UPROPERTY(Transient)
	TObjectPtr<UAnimMontage> AttackMontage;

	// 蒙太奇播放任务
	UPROPERTY(Transient)
	TObjectPtr<UAbilityTask_PlayMontageAndWait> MontageTask;

	// 当前片段（用于命中检测取参数）
	FMeleeComboSection CurrentSection;

	UFUNCTION()
	void HandleMontageBlendOut();

	UFUNCTION()
	void HandleMontageCompleted();

	UFUNCTION()
	void HandleMontageInterrupted();

	UFUNCTION()
	void HandleMontageCancelled();
};
