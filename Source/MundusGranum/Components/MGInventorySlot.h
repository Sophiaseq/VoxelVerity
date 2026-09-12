#pragma once

#include "CoreMinimal.h"
#include "Items/MGItemDefinition.h"
#include "MGInventorySlot.generated.h"

USTRUCT(BlueprintType)
struct FMGInventorySlot
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UMGItemDefinition> Item = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Count = 0;

	bool IsEmpty() const { return Item == nullptr || Count <= 0; }
	int32 GetRemainingSpace() const { return Item ? Item->BaseData.MaxStackSize - Count : 0; }
};