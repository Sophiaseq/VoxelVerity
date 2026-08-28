#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "HitInterface.generated.h"


UINTERFACE()
class UHitInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class MUNDUSGRANUM_API IHitInterface
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintNativeEvent)
	void GetHit(const FVector& HitPoint);
};
