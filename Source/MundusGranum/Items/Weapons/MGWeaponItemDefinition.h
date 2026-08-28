#pragma once
#include "Items/MGItemDefinition.h"
#include "MGWeaponItemDefinition.generated.h"

USTRUCT(BlueprintType)
struct FBoxCollisionInfo
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere)
	FVector BoxExtent = { 100.0f, 100.0f, 100.0f };
	
	UPROPERTY(EditAnywhere)
	FVector Offset = { 0.0f, 0.0f, 0.0f };
	
	UPROPERTY(EditAnywhere)
	FVector TraceStartOffset = { 0.0f, 0.0f, 0.0f };
	
	UPROPERTY(EditAnywhere)
	FVector TraceEndOffset = { 0.0f, 0.0f, 0.0f };
};

UCLASS(BlueprintType)
class MUNDUSGRANUM_API UMGWeaponItemDefinition : public UMGItemDefinition
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly)
	FBoxCollisionInfo CollisionTransform;
	
	// 武器攻击力等属性...
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float BaseDamage = 10.0f;
};