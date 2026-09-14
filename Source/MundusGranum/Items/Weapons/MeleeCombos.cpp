// Fill out your copyright notice in the Description page of Project Settings.


#include "MeleeCombos.h"

bool UMeleeCombos::FindMatchingCombos(const TArray<FGameplayTag>& InputTags, TArray<FMeleeCombo>& OutCombos) const
{
	if (InputTags.Num() == 0) return false;

	for (int32 i = 0; i < InputTags.Num(); ++i)
	{
		const FGameplayTag InputTag = InputTags[i];

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

bool UMeleeCombos::GetSectionForSequence(const TArray<FGameplayTag>& InputSequence, FMeleeComboSection& OutSection) const
{
	const int32 SequenceLen = InputSequence.Num();
	if (SequenceLen <= 0)
	{
		return false;
	}

	// 过滤出“前 SequenceLen 个输入标签全部匹配”的连招
	TArray<FMeleeCombo> Matching = Combos;
	if (!FindMatchingCombos(InputSequence, Matching))
	{
		return false;
	}

	// 返回“最后一次输入”对应的片段（索引 = 序列长度 - 1）
	const int32 SectionIndex = SequenceLen - 1;
	for (const FMeleeCombo& Combo : Matching)
	{
		if (Combo.Combo.IsValidIndex(SectionIndex))
		{
			OutSection = Combo.Combo[SectionIndex];
			return true;
		}
	}

	return false;
}
