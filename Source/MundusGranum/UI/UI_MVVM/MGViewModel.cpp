// Fill out your copyright notice in the Description page of Project Settings.


#include "MGViewModel.h"

#include "AbilitySystem/Attributes/MGHealthSet.h"

void UMGViewModel::Initialize(AActor* Owner)
{
	if (!Owner) return;

	ASC = Owner->FindComponentByClass<UAbilitySystemComponent>();
	if (!ASC) return;
	AttributeSet = Cast<UMGHealthSet>(ASC->GetAttributeSet(UMGHealthSet::StaticClass()));
	if (!AttributeSet) return;

	// 绑定 Attribute 变化委托
	ASC->GetGameplayAttributeValueChangeDelegate(AttributeSet->GetHealthAttribute()).AddUObject(this, &UMGViewModel::OnHealthChanged);
	ASC->GetGameplayAttributeValueChangeDelegate(AttributeSet->GetMaxHealthAttribute()).AddUObject(this, &UMGViewModel::OnMaxHealthChanged);
	ASC->GetGameplayAttributeValueChangeDelegate(AttributeSet->GetStaminaAttribute()).AddUObject(this, &UMGViewModel::OnStaminaChanged);
	ASC->GetGameplayAttributeValueChangeDelegate(AttributeSet->GetMaxStaminaAttribute()).AddUObject(this, &UMGViewModel::OnMaxStaminaChanged);

	// 关键：主动拉取一次初始值，解决错过初始广播的问题
	UE_MVVM_SET_PROPERTY_VALUE(Health, AttributeSet->GetHealth());
	UE_MVVM_SET_PROPERTY_VALUE(MaxHealth, AttributeSet->GetMaxHealth());
	UE_MVVM_SET_PROPERTY_VALUE(Stamina, AttributeSet->GetStamina());
	UE_MVVM_SET_PROPERTY_VALUE(MaxStamina, AttributeSet->GetMaxStamina());
	// 初始拉取后，也需要广播派生值
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetHealthPercent);
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetStaminaPercent);
}

void UMGViewModel::OnHealthChanged(const FOnAttributeChangeData& Data)
{
	// 使用宏更新 Health，会自动广播 Health 的变更
	if (UE_MVVM_SET_PROPERTY_VALUE(Health, Data.NewValue))
	{
		// 如果 Health 确实变了，手动广播依赖它的 GetHealthPercent
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetHealthPercent);
	}
}

void UMGViewModel::OnMaxHealthChanged(const FOnAttributeChangeData& Data)
{
	if (UE_MVVM_SET_PROPERTY_VALUE(MaxHealth, Data.NewValue))
	{
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetHealthPercent);
	}
}

void UMGViewModel::OnStaminaChanged(const struct FOnAttributeChangeData& Data)
{
	if (UE_MVVM_SET_PROPERTY_VALUE(Stamina, Data.NewValue))
	{
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetStaminaPercent);
	}
}

void UMGViewModel::OnMaxStaminaChanged(const struct FOnAttributeChangeData& Data)
{
	if (UE_MVVM_SET_PROPERTY_VALUE(MaxStamina, Data.NewValue))
	{
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetStaminaPercent);
	}
}

float UMGViewModel::GetHealthPercent() const
{
	return MaxHealth > 0.f ? Health / MaxHealth : 0.f;
}

float UMGViewModel::GetStaminaPercent() const
{
	return MaxStamina > 0.f ? Stamina / MaxStamina : 0.f;
}

void UMGViewModel::SetHealth(float InHealth)
{
	UE_MVVM_SET_PROPERTY_VALUE(Health, InHealth);
}

void UMGViewModel::SetMaxHealth(float InMaxHealth)
{
	UE_MVVM_SET_PROPERTY_VALUE(MaxHealth, InMaxHealth);
}

void UMGViewModel::SetStamina(float InStamina)
{
	UE_MVVM_SET_PROPERTY_VALUE(Stamina, InStamina);
}

void UMGViewModel::SetMaxStamina(float InMaxStamina)
{
	UE_MVVM_SET_PROPERTY_VALUE(MaxStamina, InMaxStamina);
}
