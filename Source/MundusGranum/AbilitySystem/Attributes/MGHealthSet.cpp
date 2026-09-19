// Fill out your copyright notice in the Description page of Project Settings.


#include "MGHealthSet.h"

#include "GameplayEffectExtension.h"
#include "MGLogChannels.h"
#include "AbilitySystem/MGAbilitySystemComponent.h"
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
}

bool UMGHealthSet::PreGameplayEffectExecute(struct FGameplayEffectModCallbackData& Data)
{
    if (!Super::PreGameplayEffectExecute(Data))
    {
        return false;
    }
    
    //TODO: 加一些GameplayTag和无敌机制
    
    // Save the current health and Stamina
    StaminaBeforeAttributeChange = GetStamina();
    MaxStaminaBeforeAttributeChange= GetMaxStamina();
    HealthBeforeAttributeChange = GetHealth();
    MaxHealthBeforeAttributeChange = GetMaxHealth();

    return true;
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
    
    if (Attribute == GetMaxHealthAttribute())
    {
        // Make sure current health is not greater than the new max health.
        if (GetHealth() > NewValue)
        {
            UMGAbilitySystemComponent* MGASC = GetMGAbilitySystemComponent();
            check(MGASC);

            MGASC->ApplyModToAttribute(GetHealthAttribute(), EGameplayModOp::Override, NewValue);
        }
    }

    if (bOutOfHealth && GetHealth() > 0.0f)
    {
        bOutOfHealth = false;
    }
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
    AActor* Instigator = ContextHandle.GetOriginalInstigator();
    AActor* Causer = ContextHandle.GetEffectCauser();

    // --- 第二步：处理临时属性（IncomingDamage）---
    if (ModifiedAttribute == GetIncomingDamageAttribute())
    {
        // 读取GE计算后写入到IncomingDamage的值
        const float LocalIncomingDamage = GetIncomingDamage();

        // Damage数值不用设置为负数
        if (LocalIncomingDamage > 0.0f)
        {
            const float NewHealth = GetHealth() - LocalIncomingDamage;
            
            SetHealth(FMath::Clamp(NewHealth, 0.0f, GetMaxHealth()));
            SetIncomingDamage(0.0f);
            UE_LOG(LogMGAbilitySystem, Warning, TEXT("Changed Health on %s, Health: %f"), *Data.Target.GetAvatarActor()->GetName(), GetHealth())
        }
    }
    else if (Data.EvaluatedData.Attribute == GetIncomingHealingAttribute())
    {
        // Convert into +Health and then clamp
        SetHealth(FMath::Clamp(GetHealth() + GetIncomingHealing(), 0.0f, GetMaxHealth()));
        SetIncomingHealing(0.0f);
    }
    else if (Data.EvaluatedData.Attribute == GetHealthAttribute())
    {
        // Clamp and fall into out of health handling below
        SetHealth(FMath::Clamp(GetHealth(), 0.0f, GetMaxHealth()));
    }
    else if (Data.EvaluatedData.Attribute == GetMaxHealthAttribute())
    {
        // TODO clamp current health?

        // Notify on any requested max health changes
        OnMaxHealthChanged.Broadcast(Instigator, Causer, &Data.EffectSpec, Data.EvaluatedData.Magnitude, MaxHealthBeforeAttributeChange, GetMaxHealth());
    }

    // If health has actually changed activate callbacks
    if (GetHealth() != HealthBeforeAttributeChange)
    {
        OnHealthChanged.Broadcast(Instigator, Causer, &Data.EffectSpec, Data.EvaluatedData.Magnitude, HealthBeforeAttributeChange, GetHealth());
    }

    if ((GetHealth() <= 0.0f) && !bOutOfHealth)
    {
        OnOutOfHealth.Broadcast(Instigator, Causer, &Data.EffectSpec, Data.EvaluatedData.Magnitude, HealthBeforeAttributeChange, GetHealth());
    }

    // Check health again in case an event above changed it.
    bOutOfHealth = (GetHealth() <= 0.0f);
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
    // 钳制逻辑：当前值不能超过最大值,当前值不能低于0
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
    
    const float CurrentHealth = GetHealth();
    const float EstimatedMagnitude = CurrentHealth - OldValue.GetCurrentValue();
    
    // Call the change callback, but without an instigator
    // This could be changed to an explicit RPC in the future
    // These events on the client should not be changing attributes
    OnHealthChanged.Broadcast(nullptr, nullptr, nullptr, EstimatedMagnitude, OldValue.GetCurrentValue(), CurrentHealth);

    if (!bOutOfHealth && CurrentHealth <= 0.0f)
    {
        OnOutOfHealth.Broadcast(nullptr, nullptr, nullptr, EstimatedMagnitude, OldValue.GetCurrentValue(), CurrentHealth);
    }

    bOutOfHealth = (CurrentHealth <= 0.0f);
}

void UMGHealthSet::OnRep_MaxHealth(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMGHealthSet, MaxHealth, OldValue);
    
    OnMaxHealthChanged.Broadcast(nullptr, nullptr, nullptr, GetMaxHealth() - OldValue.GetCurrentValue(), OldValue.GetCurrentValue(), GetMaxHealth());
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
    
    const float CurrentStamina = GetStamina();
    const float EstimatedMagnitude = CurrentStamina - OldValue.GetCurrentValue();
    OnHealthChanged.Broadcast(nullptr, nullptr, nullptr, EstimatedMagnitude, OldValue.GetCurrentValue(), CurrentStamina);

    if (!bOutOfStamina && CurrentStamina <= 0.0f)
    {
        OnOutOfStamina.Broadcast(nullptr, nullptr, nullptr, EstimatedMagnitude, OldValue.GetCurrentValue(), CurrentStamina);
    }

    bOutOfStamina = (CurrentStamina <= 0.0f);
}

void UMGHealthSet::OnRep_MaxStamina(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMGHealthSet, MaxStamina, OldValue);
    
    OnMaxHealthChanged.Broadcast(nullptr, nullptr, nullptr, GetMaxStamina() - OldValue.GetCurrentValue(), OldValue.GetCurrentValue(), GetMaxStamina());
}

