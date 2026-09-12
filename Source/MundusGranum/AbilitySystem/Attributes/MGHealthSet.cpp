// Fill out your copyright notice in the Description page of Project Settings.


#include "MGHealthSet.h"

#include "GameplayEffectExtension.h"
#include "GameFramework/Character.h"
#include "Net/UnrealNetwork.h"

UMGHealthSet::UMGHealthSet()
{
}

void UMGHealthSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UMGHealthSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMGHealthSet, MaxHealth, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMGHealthSet, Mana, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMGHealthSet, MaxMana, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMGHealthSet, Stamina, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMGHealthSet, MaxStamina, COND_None, REPNOTIFY_Always);
}

void UMGHealthSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
    Super::PreAttributeChange(Attribute, NewValue);

    // 钳制主要属性：保证当前值不超过最大值且不低于0
    // 例如：如果MaxHealth=100，NewValue=150，则自动修正为100
    if (Attribute == GetHealthAttribute())
    {
        ClampVitalAttribute(Attribute, NewValue, MaxHealth);
    }
    else if (Attribute == GetManaAttribute())
    {
        ClampVitalAttribute(Attribute, NewValue, MaxMana);
    }
    else if (Attribute == GetStaminaAttribute())
    {
        ClampVitalAttribute(Attribute, NewValue, MaxStamina);
    }
    // 注意：这里只做钳制，不做业务逻辑
    // ❌ 错误：在这里处理死亡逻辑
    // ✅ 正确：在PostGameplayEffectExecute中处理死亡逻辑
}

// ====================================================================
// PreAttributeBaseChange — BaseValue被修改前调用
// ====================================================================
void UMGHealthSet::PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const
{
    Super::PreAttributeBaseChange(Attribute, NewValue);

    // BaseValue通常表示永久性变化（如升级增加的属性）
    // 同样需要钳制到合理范围
    if (Attribute == GetHealthAttribute())
    {
        ClampVitalAttribute(Attribute, NewValue, MaxHealth);
    }
    else if (Attribute == GetManaAttribute())
    {
        ClampVitalAttribute(Attribute, NewValue, MaxMana);
    }
    else if (Attribute == GetStaminaAttribute())
    {
        ClampVitalAttribute(Attribute, NewValue, MaxStamina);
    }
}

void UMGHealthSet::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
    Super::PostAttributeChange(Attribute, OldValue, NewValue);
}

// ====================================================================
// PostGameplayEffectExecute — GE执行完毕后的处理（最核心的函数！）
// ====================================================================
void UMGHealthSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
    Super::PostGameplayEffectExecute(Data);

    // --- 第一步：获取上下文信息 ---
    // Data.EvaluatedData.Attribute 告诉你哪个属性被修改了
    // Data.EvaluatedData.Magnitude 告诉你修改量是多少
    // Data.EffectSpec 包含GE的完整信息（等级、标签、源对象等）

    const FGameplayEffectContextHandle ContextHandle = Data.EffectSpec.GetContext();
    const FGameplayAttribute& ModifiedAttribute = Data.EvaluatedData.Attribute;

    // --- 第二步：处理临时属性（IncomingDamage）---
    if (ModifiedAttribute == GetIncomingDamageAttribute())
    {
        // 读取GE计算后写入到IncomingDamage的值
        const float LocalIncomingDamage = GetIncomingDamage();

        // 防御检查：伤害必须大于0才有意义
        if (LocalIncomingDamage > 0.0f)
        {
            // 计算实际生命值减少
            // 注意：IncomingDamage已经在MMC中完成了防御力减免等计算
            // 这里直接应用即可
            const float NewHealth = GetHealth() - LocalIncomingDamage;

            // 设置新生命值（PreAttributeChange会自动钳制到0~MaxHealth范围）
            SetHealth(FMath::Clamp(NewHealth, 0.0f, GetMaxHealth()));

            // === 在这里实现死亡判定 ===
            // ✅ 正确位置：PostGameplayEffectExecute
            if (GetHealth() <= 0.0f && LocalIncomingDamage > 0.0f)
            {
                // 获取受伤的角色
                AActor* TargetActor = Data.Target.GetOwner();
                if (ACharacter* TargetCharacter = Cast<ACharacter>(TargetActor))
                {
                    // 通过ASC广播死亡事件
                    // 实战中可以通过GameplayEvent或委托通知
                    UE_LOG(LogTemp, Warning, TEXT("RPGAttributeSet: Character Died! Health reached 0"));

                    // 可选：发送GameplayEvent来触发死亡技能
                    // FGameplayEventData EventData;
                    // EventData.Instigator = Data.EffectSpec.GetContext().GetInstigator();
                    // UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
                    //     TargetCharacter,
                    //     FGameplayTag::RequestGameplayTag("Event.Death"),
                    //     EventData
                    // );
                }
            }
        }

        // 重置临时属性，为下一个GE做准备
        SetIncomingDamage(0.0f);
    }

    // --- 第三步：处理临时属性（IncomingHealing）---
    else if (ModifiedAttribute == GetIncomingHealingAttribute())
    {
        const float LocalIncomingHealing = GetIncomingHealing();

        if (LocalIncomingHealing > 0.0f)
        {
            // 治疗不能超过最大生命值
            const float NewHealth = GetHealth() + LocalIncomingHealing;
            SetHealth(FMath::Clamp(NewHealth, 0.0f, GetMaxHealth()));
        }

        // 重置临时属性
        SetIncomingHealing(0.0f);
    }
}

// ====================================================================
// 内部辅助函数实现
// ====================================================================

void UMGHealthSet::ResetMetaAttributes()
{
    // 将所有临时属性重置为0
    // 在每次GE应用前调用，确保数据干净
    SetIncomingDamage(0.0f);
    SetIncomingHealing(0.0f);
}

void UMGHealthSet::ClampVitalAttribute(const FGameplayAttribute& Attribute, float& NewValue,
    const FGameplayAttributeData& MaxValueAttribute)
{
    // 钳制逻辑：
    // 1. 当前值不能超过最大值
    // 2. 当前值不能低于0

    const float MaxValue = MaxValueAttribute.GetCurrentValue();

    if (NewValue > MaxValue)
    {
        UE_LOG(LogTemp, Verbose, TEXT("ClampVitalAttribute: Clamped %s from %.1f to %.1f (Max)"),
            *Attribute.GetName(), NewValue, MaxValue);
        NewValue = MaxValue;
    }

    if (NewValue < 0.0f)
    {
        UE_LOG(LogTemp, Verbose, TEXT("ClampVitalAttribute: Clamped %s from %.1f to 0 (Min)"),
            *Attribute.GetName(), NewValue);
        NewValue = 0.0f;
    }
}

void UMGHealthSet::OnRep_Health(const FGameplayAttributeData& OldValue)
{
	// 通知GAS系统Health属性已变化（内部处理UI绑定等）
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMGHealthSet, Health, OldValue);
}

void UMGHealthSet::OnRep_MaxHealth(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMGHealthSet, MaxHealth, OldValue);
}

void UMGHealthSet::OnRep_Mana(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMGHealthSet, Mana, OldValue);
}

void UMGHealthSet::OnRep_MaxMana(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMGHealthSet, MaxMana, OldValue);
}

void UMGHealthSet::OnRep_Stamina(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMGHealthSet, Stamina, OldValue);
}

void UMGHealthSet::OnRep_MaxStamina(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMGHealthSet, MaxStamina, OldValue);
}

