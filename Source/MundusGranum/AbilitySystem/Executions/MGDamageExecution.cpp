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

float UMGDamageExecution::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec,
                                                                const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags,
                                                                const FAggregatorEvaluateParameters& SourceAttributes, const FAggregatorEvaluateParameters& TargetAttributes) const
{
	// ================================================================
    // 第一步：从Source获取攻击者的属性值
    // ================================================================

    float AttackPower = 0.0f;
    // AttemptCalculateCapturedAttributeMagnitude 尝试从捕获中读取属性值
    // 参数1：捕获定义（哪个属性、从哪里捕获）
    // 参数2：Source的评估参数（包含Source的Aggregator数据）
    // 参数3：输出参数，接收捕获到的属性值
    // 返回值：true=成功捕获，false=捕获失败（例如Source没有这个AttributeSet）
    if (!AttackPowerCapture.AttemptCalculateCapturedAttributeMagnitude(
            Spec, SourceAttributes, AttackPower))
    {
        // 捕获失败时的默认值（攻击力为0）
        AttackPower = 0.0f;
        UE_LOG(LogTemp, Warning, TEXT("DamageCalculation: Failed to capture AttackPower from Source"));
    }

    // ================================================================
    // 第二步：从Target获取防御者的属性值
    // ================================================================

    float DefensePower = 0.0f;
    if (!DefensePowerCapture.AttemptCalculateCapturedAttributeMagnitude(
            Spec, TargetAttributes, DefensePower))
    {
        DefensePower = 0.0f;
    }

    // ================================================================
    // 第三步：读取SetByCaller传递的技能倍率
    // ================================================================
    // SetByCaller是GE中传递动态参数的机制
    // 技能在激活时通过FGameplayAbilitySpecHandle → SetByCallerTagMagnitude传入倍率
    // 例如：火球术倍率=1.5，普通攻击倍率=1.0

    const FGameplayTag SkillMultiplierTag = FGameplayTag::RequestGameplayTag(
        FName("Data.SkillMultiplier"),  // 标签名称（必须与GA中设置的标签一致）
        false                           // false = 如果标签不存在不报错，返回空标签
    );

    float SkillMultiplier = 1.0f;  // 默认技能倍率1.0（100%攻击力伤害）
    if (SkillMultiplierTag.IsValid())
    {
        // 尝试从GE Spec中读取SetByCaller的值
        // GetSetByCallerMagnitude：读取指定标签关联的数值
        // 如果该标签没有被设置，返回false，SafeMultiplyValue保持为1.0f
        SkillMultiplier = Spec.GetSetByCallerMagnitude(SkillMultiplierTag, true, 1.0f);
    }

    // ================================================================
    // 第四步：计算基础伤害（攻击力 × 技能倍率）
    // ================================================================

    const float BaseDamage = AttackPower * SkillMultiplier;

    UE_LOG(LogTemp, Verbose, TEXT("DamageCalculation: AttackPower=%.1f, Multiplier=%.2f, BaseDamage=%.1f"),
        AttackPower, SkillMultiplier, BaseDamage);

    // ================================================================
    // 第五步：计算防御减免（使用边际递减公式）
    // ================================================================

    // 减免率 = Defense / (Defense + DefenseCoefficient)
    // 注意：如果DefenseCoefficient为0会导致除零错误
    const float DefenseReductionRate = (DefenseCoefficient > 0.0f)
        ? DefensePower / (DefensePower + DefenseCoefficient)
        : 0.0f;

    // 有效伤害 = 基础伤害 × (1 - 减免率)
    const float EffectiveDamage = BaseDamage * (1.0f - DefenseReductionRate);

    // ================================================================
    // 第六步：暴击判定
    // ================================================================

    // 获取暴击率
    float CriticalRate = 0.0f;
    if (!CriticalRateCapture.AttemptCalculateCapturedAttributeMagnitude(
            Spec, SourceAttributes, CriticalRate))
    {
        CriticalRate = 0.0f;
    }

    // 获取暴击伤害倍率
    float CriticalDamageMultiplier = 1.5f;  // 默认150%
    if (!CriticalDamageCapture.AttemptCalculateCapturedAttributeMagnitude(
            Spec, SourceAttributes, CriticalDamageMultiplier))
    {
        CriticalDamageMultiplier = 1.5f;
    }

    // 生成0~1之间的随机数，判断是否暴击
    const float RandomRoll = FMath::FRand();
    const bool bIsCritical = (RandomRoll < CriticalRate);

    // 计算最终伤害
    float FinalDamage = EffectiveDamage;

    if (bIsCritical)
    {
        // 暴击伤害 = 有效伤害 × 暴击倍率
        FinalDamage = EffectiveDamage * CriticalDamageMultiplier;

        UE_LOG(LogTemp, Log, TEXT("DamageCalculation: CRIT! Roll=%.3f, Rate=%.3f, Final=%.1f (x%.1f)"),
            RandomRoll, CriticalRate, FinalDamage, CriticalDamageMultiplier);

        // ================================================================
        // 注意：这里只是计算属性修改量的数值
        // 为了让UI显示"暴击"文字/效果，应该通过GameplayCue或GameplayTag传递暴击信息
        // 这里通过动态标签标记此次伤害为暴击
        // ================================================================

        // 在实际项目中，可以通过以下方式传递暴击信息：
        // Spec.DynamicGrantedTags.AddTag(FGameplayTag::RequestGameplayTag("Combat.Critical"));
        // 但由于Spec是const引用，需要在GA创建GE时设置
    }
    else
    {
        UE_LOG(LogTemp, Verbose, TEXT("DamageCalculation: Normal hit, Final=%.1f"), FinalDamage);
    }

    // ================================================================
    // 第七步：确保伤害不为负数（防御力再高也不会反弹伤害）
    // ================================================================

    FinalDamage = FMath::Max(FinalDamage, 0.0f);

    // 返回计算结果 → GAS会将此值应用到GE的目标属性上（通常是IncomingDamage）
    return FinalDamage;
}
