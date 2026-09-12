// Fill out your copyright notice in the Description page of Project Settings.


#include "MeleeCombos.h"

bool UMeleeCombos::FindMatchingCombos(const FGameplayTagContainer& InputTags, TArray<FMeleeCombo>& OutCombos) const
{
	if (!InputTags.IsValid()) return false;

	for (int32 i = 0; i < InputTags.Num(); ++i)
	{
		const FGameplayTag InputTag = InputTags.GetByIndex(i);

		for (int32 j = OutCombos.Num() - 1; j >= 0; --j)
		{
			if (!OutCombos[j].Combo.IsValidIndex(i) || OutCombos[j].Combo[i].InputTag != InputTag)
			{
				OutCombos.RemoveAtSwap(j, 1, EAllowShrinking::No);
			}
		}
	}

	return !OutCombos.IsEmpty();
}

bool UMeleeCombos::GetComboSection(const FGameplayTagContainer& InputTags, TArray<FMeleeCombo>& InCombos, FMeleeComboSection& OutSection) const
{
	TArray<FMeleeCombo> MeleeCombos = Combos;
	if (FindMatchingCombos(InputTags, MeleeCombos))
	{
		OutSection = MeleeCombos[0].Combo[InputTags.Num()];
		return true;
	}
	return false;
}
