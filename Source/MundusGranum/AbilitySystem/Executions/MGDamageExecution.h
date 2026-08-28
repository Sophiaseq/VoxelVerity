// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectExecutionCalculation.h"
#include "MGDamageExecution.generated.h"

/**
 * 
 */
UCLASS()
class MUNDUSGRANUM_API UMGDamageExecution : public UGameplayEffectExecutionCalculation
{
	GENERATED_BODY()
	
public:
	UMGDamageExecution();
	virtual float CalculateBaseMagnitude_Implementation(
		const FGameplayEffectSpec& Spec,
		const FGameplayTagContainer* SourceTags,
		const FGameplayTagContainer* TargetTags,
		const FAggregatorEvaluateParameters& SourceAttributes,
		const FAggregatorEvaluateParameters& TargetAttributes
	) const override;

protected:
	// ================================================================
	// 属性捕获定义 — 告诉GAS我们需要读取哪些属性
	// ================================================================
	// 这些成员变量在编辑器中配置（或在构造函数中硬编码）
	// FGameplayEffectAttributeCaptureDefinition 包含：
	//   - AttributeToCapture：要捕获的属性
	//   - EGameplayEffectAttributeCaptureSource：从谁身上捕获（Source还是Target）
	//   - bSnapshot：是否快照（true=使用GE应用时的值，false=使用计算时的值）
	// ================================================================

	/** 捕获：攻击者的攻击力属性 */
	UPROPERTY(EditDefaultsOnly, Category = "Calculation|Capture")
	FGameplayEffectAttributeCaptureDefinition AttackPowerCapture;
 
	/** 捕获：防御者的防御力属性 */
	UPROPERTY(EditDefaultsOnly, Category = "Calculation|Capture")
	FGameplayEffectAttributeCaptureDefinition DefensePowerCapture;

	/** 捕获：攻击者的暴击率属性 */
	UPROPERTY(EditDefaultsOnly, Category = "Calculation|Capture")
	FGameplayEffectAttributeCaptureDefinition CriticalRateCapture;

	/** 捕获：攻击者的暴击伤害倍率属性 */
	UPROPERTY(EditDefaultsOnly, Category = "Calculation|Capture")
	FGameplayEffectAttributeCaptureDefinition CriticalDamageCapture;

	// ================================================================
	// 可配置参数
	// ================================================================

	/**
	 * 防御减免系数
	 * 公式：减免率 = Defense / (Defense + DefenseCoefficient)
	 * 默认100：当防御力=100时，减免率为50%
	 * 降低此值会使防御力的效果更明显
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Calculation|Formula")
	float DefenseCoefficient;
};
