#pragma once

#include "GameplayEffectTypes.h"
#include "MGGameplayEffectContext.generated.h"

USTRUCT(BlueprintType)
struct FMGGameplayEffectContext : public FGameplayEffectContext
{
	GENERATED_BODY()

public:
	FMGGameplayEffectContext()
	:bIsBlockedHit(false)
	, bIsCriticalHit(false)
	{
	}
	
	bool IsBlockedHit() const{return bIsBlockedHit;}
	void SetIsBlockedHit(bool IsBlockedHit){bIsBlockedHit = IsBlockedHit;}
	bool IsCriticalHit() const{return bIsCriticalHit;}
	void SetIsCriticalHit(bool IsCriticalHit){bIsCriticalHit = IsCriticalHit;}
	
	/** Creates a copy of this context, used to duplicate for later modifications */
	virtual FMGGameplayEffectContext* Duplicate() const override
	{
		FMGGameplayEffectContext* NewContext = new FMGGameplayEffectContext();
		*NewContext = *this;
		if (GetHitResult())
		{
			// Does a deep copy of the hit result
			NewContext->AddHitResult(*GetHitResult(), true);
		}
		return NewContext;
	}
	
	/** Returns the actual struct used for serialization, subclasses must override this! */
	virtual UScriptStruct* GetScriptStruct() const override
	{
		return FMGGameplayEffectContext::StaticStruct();
	}
	
	/** Custom serialization, subclasses must override this */
	virtual bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess) override;
	
protected:
	UPROPERTY()
	uint8 bIsBlockedHit:1;
	
	UPROPERTY()
	uint8 bIsCriticalHit:1;
	
private:
	
};

template<>
struct TStructOpsTypeTraits<FMGGameplayEffectContext> : public TStructOpsTypeTraitsBase2<FMGGameplayEffectContext>
{
	enum
	{
		WithNetSerializer = true,
		WithCopy = true
	};
};
