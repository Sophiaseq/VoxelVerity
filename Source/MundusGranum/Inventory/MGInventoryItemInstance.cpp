// Fill out your copyright notice in the Description page of Project Settings.


#include "MGInventoryItemInstance.h"

#include "Net/UnrealNetwork.h"

void UMGInventoryItemInstance::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, StatTags);
	DOREPLIFETIME(ThisClass, ItemDef);
}

void UMGInventoryItemInstance::AddStatTagStack(FGameplayTag Tag, int32 StackCount)
{
	StatTags.AddStack(Tag, StackCount);
}

void UMGInventoryItemInstance::SetItemDef(const TSubclassOf<UMGInventoryItemDefinition> InDef)
{
	ItemDef = InDef;
}
