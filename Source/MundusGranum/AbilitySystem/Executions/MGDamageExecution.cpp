// Fill out your copyright notice in the Description page of Project Settings.


#include "MGDamageExecution.h"
#include "AbilitySystem/Attributes/MGCombatSet.h"

UMGDamageExecution::UMGDamageExecution()
{
	// --- 捕获：攻击者的攻击力 ---
	AttackPowerCapture.AttributeToCapture = UMGCombatSet::GetAttackPowerAttribute();
	AttackPowerCapture.AttributeSource = EGameplayEffectAttributeCaptureSource::Source;
	// bSnapshot = false：使用计算时刻的实时值（而非GE应用时刻的快照值）
	// 为什么不用快照：如果攻击者在GE执行过程中被Buff影响，我们希望使用最新值
	AttackPowerCapture.bSnapshot = false;

	// --- 捕获：防御者的防御力 ---
	DefensePowerCapture.AttributeToCapture = UMGCombatSet::GetDefensePowerAttribute();
	DefensePowerCapture.AttributeSource = EGameplayEffectAttributeCaptureSource::Target;
	DefensePowerCapture.bSnapshot = false;

	// --- 捕获：攻击者的暴击率 ---
	CriticalRateCapture.AttributeToCapture = UMGCombatSet::GetCriticalRateAttribute();
	CriticalRateCapture.AttributeSource = EGameplayEffectAttributeCaptureSource::Source;
	CriticalRateCapture.bSnapshot = false;

	// --- 捕获：攻击者的暴击伤害倍率 ---
	CriticalDamageCapture.AttributeToCapture = UMGCombatSet::GetCriticalDamageAttribute();
	CriticalDamageCapture.AttributeSource = EGameplayEffectAttributeCaptureSource::Source;
	CriticalDamageCapture.bSnapshot = false;

	// ================================================================
	// ⚠️ 关键步骤：必须将捕获定义注册到RelevantAttributesToCapture
	// ================================================================
	// 如果不注册，GAS将不会捕获这些属性值，计算结果永远为0
	RelevantAttributesToCapture.Add(AttackPowerCapture);
	RelevantAttributesToCapture.Add(DefensePowerCapture);
	RelevantAttributesToCapture.Add(CriticalRateCapture);
	RelevantAttributesToCapture.Add(CriticalDamageCapture);
	
	DefenseCoefficient = 100.0f;
}


