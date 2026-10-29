// Fill out your copyright notice in the Description page of Project Settings.


#include "MGInventoryItemDefinition.h"

UMGInventoryItemDefinition::UMGInventoryItemDefinition(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

const UMGInventoryItemFragment* UMGInventoryItemDefinition::FindFragmentByClass(
	TSubclassOf<UMGInventoryItemFragment> FragmentClass) const
{
	if (FragmentClass != nullptr)
	{
		for (UMGInventoryItemFragment* Fragment : Fragments)
		{
			if (Fragment && Fragment->IsA(FragmentClass))
			{
				return Fragment;
			}
		}
	}

	return nullptr;
}

const UMGInventoryItemFragment* UMGInventoryFunctionLibrary::FindItemDefinitionFragment(
	TSubclassOf<UMGInventoryItemDefinition> ItemDef, TSubclassOf<UMGInventoryItemFragment> FragmentClass)
{
	if ((ItemDef != nullptr) && (FragmentClass != nullptr))
	{
		return GetDefault<UMGInventoryItemDefinition>(ItemDef)->FindFragmentByClass(FragmentClass);
	}
	return nullptr;
}
