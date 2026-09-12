#pragma once

#include "MGPlayerAttributeSet.generated.h"

class UAttributeSet;
/*
 * 本放在MGPlayerState中，单独拿出来只是为了防止依赖整个MGPlayerState
 */
USTRUCT(Blueprintable)
struct FMGPlayerAttributeSet
{
	GENERATED_BODY()
	
	UPROPERTY()
	TObjectPtr<const UAttributeSet> PrimarySet;
	
	// Health attribute set used by this actor.
	UPROPERTY()
	TObjectPtr<const UAttributeSet> HealthSet;

	// Combat attribute set used by this actor.
	UPROPERTY()
	TObjectPtr<const UAttributeSet> CombatSet;
};
