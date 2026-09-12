// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Player/MGPlayerAttributeSet.h"
#include "UObject/Object.h"
#include "MGWidgetController.generated.h"

class UAbilitySystemComponent;
class AMGPlayerController;
/**
 * 
 */
USTRUCT(BlueprintType)
struct FWidgetControllerParams
{
	GENERATED_BODY()
	
	FWidgetControllerParams(){}
	FWidgetControllerParams(APlayerController* PC, APlayerState* PS, UAbilitySystemComponent* ASC,const FMGPlayerAttributeSet& Attributes)
	: PlayerController(PC), PlayerState(PS), AbilitySystemComponent(ASC), PlayerAttributes(Attributes) {}
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<APlayerController> PlayerController;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<APlayerState> PlayerState;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FMGPlayerAttributeSet PlayerAttributes;
};

UCLASS()
class MUNDUSGRANUM_API UMGWidgetController : public UObject
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable)
	void SetWidgetControllerParams(const FWidgetControllerParams& WCParams);
	virtual void BroadcastInitialValues();
	virtual void BindCallbackToDependencies();
	
protected:
	UPROPERTY(BlueprintReadOnly, Category=WidgetController)
	TObjectPtr<APlayerController> PlayerController;
	
	UPROPERTY(BlueprintReadOnly, Category=WidgetController)
	TObjectPtr<APlayerState> PlayerState;
	
	UPROPERTY(BlueprintReadOnly, Category=WidgetController)
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;
	
	UPROPERTY(BlueprintReadOnly, Category=WidgetController)
	FMGPlayerAttributeSet Attributes;
	
};
