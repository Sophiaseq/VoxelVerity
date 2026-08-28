// Fill out your copyright notice in the Description page of Project Settings.


#include "MGPawnExtensionComponent.h"


UMGPawnExtensionComponent::UMGPawnExtensionComponent(const FObjectInitializer& ObjectInitializer) 
    : Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
}


// Called when the game starts
void UMGPawnExtensionComponent::BeginPlay()
{
	Super::BeginPlay();
}


// Called every frame
void UMGPawnExtensionComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                              FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

