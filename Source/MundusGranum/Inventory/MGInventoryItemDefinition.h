// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "MGInventoryItemDefinition.generated.h"

class UMGInventoryItemInstance;
/**
 * 
 */
UCLASS(MinimalAPI, DefaultToInstanced, EditInlineNew, Abstract)
class UMGInventoryItemFragment : public UObject
{
	GENERATED_BODY()
	
public:
	virtual void OnInstanceCreated(UMGInventoryItemInstance* Instance) const {}
};

UCLASS()
class MUNDUSGRANUM_API UMGInventoryItemDefinition : public UObject
{
	GENERATED_BODY()
	
public:
	UMGInventoryItemDefinition(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
	const UMGInventoryItemFragment* FindFragmentByClass(TSubclassOf<UMGInventoryItemFragment> FragmentClass) const;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Display)
	FText DisplayName;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Display, Instanced)
	TArray<TObjectPtr<UMGInventoryItemFragment>> Fragments;
};

//@TODO: 如果需要添加成员变量可做成Subsystem
UCLASS()
class UMGInventoryFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	UFUNCTION(BlueprintCallable, meta=(DeterminesOutputType=FragmentClass))
	static const UMGInventoryItemFragment* FindItemDefinitionFragment(TSubclassOf<UMGInventoryItemDefinition> ItemDef, TSubclassOf<UMGInventoryItemFragment> FragmentClass);
};