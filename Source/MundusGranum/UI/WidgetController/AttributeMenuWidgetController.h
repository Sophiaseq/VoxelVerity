// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "MGWidgetController.h"
#include "AttributeMenuWidgetController.generated.h"

class UAttributeInfo;


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAttributeInfoDelegate, const FMGAttributeInfo&, Info);
/**
 * 
 */
UCLASS(Blueprintable, BlueprintType)
class MUNDUSGRANUM_API UAttributeMenuWidgetController : public UMGWidgetController
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable)
	virtual void BroadcastInitialValues() override;
	virtual void BindCallbackToDependencies() override;
	
	UPROPERTY(BlueprintAssignable, Category = "GAS|Attributes")
	FAttributeInfoDelegate AttributeInfoDelegate;
	
private:
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UAttributeInfo> AttributeInfo;
};
