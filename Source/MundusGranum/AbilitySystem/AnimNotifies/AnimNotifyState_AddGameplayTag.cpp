// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNotifyState_AddGameplayTag.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"

UAbilitySystemComponent* UAnimNotifyState_AddGameplayTag::GetASC(const USkeletalMeshComponent* MeshComp)
{
	if (!MeshComp) return nullptr;
	AActor* Owner = MeshComp->GetOwner();
	if (!Owner) return nullptr;
	// IAbilitySystemInterface 优先，再退回 FindComponent
	return UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Owner);
}

bool UAnimNotifyState_AddGameplayTag::ShouldApply(const UAbilitySystemComponent* ASC, bool bServerOnly)
{
	if (!ASC) return false;
	if (bServerOnly && !ASC->GetOwner()->HasAuthority()) return false;
	return true;
}

void UAnimNotifyState_AddGameplayTag::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (!Tag.IsValid()) return;

	UAbilitySystemComponent* ASC = GetASC(MeshComp);
	if (!ShouldApply(ASC, bServerOnly)) return;

	// 用 Count 版本，支持嵌套/叠加，避免多次添加后一次移除就清零
	ASC->AddLooseGameplayTag(Tag, StackCount);
}

void UAnimNotifyState_AddGameplayTag::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	if (!Tag.IsValid()) return;

	UAbilitySystemComponent* ASC = GetASC(MeshComp);
	if (!ShouldApply(ASC, bServerOnly)) return;

	ASC->RemoveLooseGameplayTag(Tag, StackCount);
}

FString UAnimNotifyState_AddGameplayTag::GetNotifyName_Implementation() const
{
	return Tag.IsValid() ? FString::Printf(TEXT("Tag: %s"), *Tag.ToString()) : TEXT("AddGameplayTag");
}