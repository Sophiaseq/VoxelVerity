#include "MGAbilitySystemComponent.h"



UMGAbilitySystemComponent::UMGAbilitySystemComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}


// Called when the game starts
void UMGAbilitySystemComponent::BeginPlay()
{
	Super::BeginPlay();
}


// Called every frame
void UMGAbilitySystemComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                              FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

