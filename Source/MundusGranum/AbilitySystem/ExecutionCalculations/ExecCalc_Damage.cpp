// Fill out your copyright notice in the Description page of Project Settings.


#include "ExecCalc_Damage.h"

#include "MundusGranumGameplayTags.h"
#include "AbilitySystem/MGAbilitySystemComponent.h"
#include "AbilitySystem/MGAbilitySystemLibrary.h"
#include "AbilitySystem/Attributes/MGCombatSet.h"
#include "AbilitySystem/Attributes/MGHealthSet.h"
#include "Interaction/CombatInterface.h"

//通过设置为UMGCombatSet的友元来访问成员变量，需要注意。
struct MGDamageStatics
{
	DECLARE_ATTRIBUTE_CAPTUREDEF(AttackPower)
	DECLARE_ATTRIBUTE_CAPTUREDEF(DefensePower)
	DECLARE_ATTRIBUTE_CAPTUREDEF(CriticalRate)
	DECLARE_ATTRIBUTE_CAPTUREDEF(CriticalDamage)
	
	MGDamageStatics()
	{
		DEFINE_ATTRIBUTE_CAPTUREDEF(UMGCombatSet, AttackPower, Target, true);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UMGCombatSet, DefensePower, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UMGCombatSet, CriticalRate, Target, true);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UMGCombatSet, CriticalDamage, Target, true);
	}
};

static const MGDamageStatics& DamageStatic()
{
	static MGDamageStatics DStatics;
	return DStatics;
}
	
	
UExecCalc_Damage::UExecCalc_Damage()
{
	RelevantAttributesToCapture.Add(DamageStatic().AttackPowerDef);
	RelevantAttributesToCapture.Add(DamageStatic().DefensePowerDef);
	RelevantAttributesToCapture.Add(DamageStatic().CriticalRateDef);
	RelevantAttributesToCapture.Add(DamageStatic().CriticalDamageDef);
}

void UExecCalc_Damage::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams,
	FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	const UAbilitySystemComponent* SourceASC = ExecutionParams.GetSourceAbilitySystemComponent();
	const UAbilitySystemComponent* TargetASC = ExecutionParams.GetTargetAbilitySystemComponent();
	
	AActor* SourceAvatar = SourceASC ? SourceASC->GetAvatarActor() : nullptr;
	AActor* TargetAvatar = TargetASC ? TargetASC->GetAvatarActor() : nullptr;
	ICombatInterface* SourceCombatInterface = Cast<ICombatInterface>(SourceAvatar);
	ICombatInterface* TargetCombatInterface = Cast<ICombatInterface>(TargetAvatar);
	float SourceLevel = SourceCombatInterface->GetCharacterLevel();
	float TargetLevel = TargetCombatInterface->GetCharacterLevel();
	
	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();
	
	const FGameplayTagContainer* SourceTag = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTag = Spec.CapturedTargetTags.GetAggregatedTags();
	
	FAggregatorEvaluateParameters EvaluateParameters;
	EvaluateParameters.SourceTags = SourceTag;
	EvaluateParameters.TargetTags = TargetTag;
	
	const float Sharpness =  Spec.GetSetByCallerMagnitude("Damage.Melee.Sharpness");
	const float Quality =  Spec.GetSetByCallerMagnitude("Damage.Melee.Quality");
	
	float AttackPower = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatic().AttackPowerDef, EvaluateParameters, AttackPower);
	AttackPower = FMath::Max<float>(0.f, AttackPower);
	
	float DefensePower = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatic().DefensePowerDef, EvaluateParameters, DefensePower);
	DefensePower = FMath::Max<float>(0.f, DefensePower);
	
	float CriticalRateDef = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatic().CriticalRateDef, EvaluateParameters, CriticalRateDef);
	CriticalRateDef = FMath::Max<float>(0.f, CriticalRateDef);
	
	float CriticalDamageDef = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatic().CriticalDamageDef, EvaluateParameters, CriticalDamageDef);
	CriticalDamageDef = FMath::Max<float>(0.f, CriticalDamageDef);
	
	float BlockCoefficient = 0.f;
	if (TargetASC->HasMatchingGameplayTag(MundusGranumGameplayTags::CharacterState_Block))
	{
		BlockCoefficient = 0.65;
	}
	if (TargetASC->HasMatchingGameplayTag(MundusGranumGameplayTags::CharacterState_Block_Parry))
	{
		BlockCoefficient = 0.99;
	}
	
	FGameplayEffectContextHandle EffectContextHandle = Spec.GetEffectContext();
	UMGAbilitySystemLibrary::SetIsBlockedHit(EffectContextHandle, BlockCoefficient>0.9);
	
	const UMGPawnData* PawnData = UMGAbilitySystemLibrary::GetDefaultPawnData(SourceASC);
	FRealCurve* PenetrationCurve = PawnData->DamageCalculationCoefficients->FindCurve(FName("Penetration"), FString());
	FRealCurve* EffectiveCurve = PawnData->DamageCalculationCoefficients->FindCurve(FName("Effective"), FString());
	const float PenetrationCoefficient = PenetrationCurve->Eval(SourceLevel);
	const float EffectiveCoefficient = EffectiveCurve->Eval(TargetLevel);

	float DamageReduction = (100 - EffectiveCoefficient * (DefensePower * (100 - Sharpness*PenetrationCoefficient) / 100.f)) / 100.f;
	
	float FinalDamage = AttackPower * Quality * DamageReduction*(1-BlockCoefficient);
	const bool bCriticalHit = FMath::RandRange(0, 100) < CriticalRateDef;
	FinalDamage = bCriticalHit ? FinalDamage*CriticalDamageDef : FinalDamage;
	
	UMGAbilitySystemLibrary::SetIsCriticalHit(EffectContextHandle, bCriticalHit);

	const FGameplayModifierEvaluatedData EvaluatedData(UMGHealthSet::GetIncomingDamageAttribute(), EGameplayModOp::Additive, FinalDamage);
	OutExecutionOutput.AddOutputModifier(EvaluatedData);
}


