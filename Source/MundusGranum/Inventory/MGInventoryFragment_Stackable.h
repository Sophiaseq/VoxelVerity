// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MGInventoryItemDefinition.h"
#include "MGInventoryFragment_Stackable.generated.h"

/**
 * 
 */
UCLASS()
class MUNDUSGRANUM_API UMGInventoryFragment_Stackable : public UMGInventoryItemFragment
{
	GENERATED_BODY()
	
public:
	virtual int32 GetMaxStackCount() const { return MaxStackCount; }
	
	virtual bool CanStackWith(int32 CurrentStackCount) const
	{
		return CurrentStackCount < GetMaxStackCount();
	}

protected:
	UPROPERTY(EditDefaultsOnly, meta=(ClampMin = 1))
	int32 MaxStackCount = 1;
};
