#pragma once

#include "CoreMinimal.h"
#include "Components/PawnComponent.h"
#include "MGPawnExtensionComponent.generated.h"


class UMGAbilitySystemComponent;
class UMGCharacterDefinition;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MUNDUSGRANUM_API UMGPawnExtensionComponent : public UPawnComponent
{
	GENERATED_BODY()

public:
	UMGPawnExtensionComponent(const FObjectInitializer& ObjectInitializer);
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;
	
protected:
	virtual void BeginPlay() override;
	
private:
	UPROPERTY(EditAnywhere, Category = "MundusGranum|Pawn")
	TObjectPtr<const UMGCharacterDefinition> PawnData;
	
	UPROPERTY(Transient)
	TObjectPtr<UMGAbilitySystemComponent> AbilitySystemComponent;
	
};
