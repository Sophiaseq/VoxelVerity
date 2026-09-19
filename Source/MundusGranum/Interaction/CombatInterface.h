// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/MGPawnData.h"
#include "UObject/Interface.h"
#include "CombatInterface.generated.h"

class UMGWeaponItemDefinition;
// This class does not need to be modified.
UINTERFACE()
class UCombatInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class MUNDUSGRANUM_API ICombatInterface
{
	GENERATED_BODY()
	
public:
	virtual float GetCharacterLevel() = 0;
	
	virtual UMGWeaponItemDefinition* GetCurrentWeapon() const {return nullptr;}
	
	virtual FVector GetSocketLocation(FName TagName, FName SocketName) const {return FVector();}
	
	virtual UAnimMontage* GetHitReactMontage() const {return nullptr;}

};
