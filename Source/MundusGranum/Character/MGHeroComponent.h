#pragma once

#include "CoreMinimal.h"
#include "Components/PawnComponent.h"
#include "MGHeroComponent.generated.h"


struct FInputActionValue;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MUNDUSGRANUM_API UMGHeroComponent : public UPawnComponent
{
	GENERATED_BODY()

public:
	UMGHeroComponent(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void BeginPlay() override;
	
	void Input_Move(const FInputActionValue& Value) const;
	void Input_LookMouse(const FInputActionValue& Value);
	void Input_Jump() const;
	void Input_SprintPressed();
	void Input_SprintReleased();
	void InputTag_UseLeftHandItem();
	void InputTag_UseRightHandItem();
	void InputTag_SelectItem(const FInputActionValue& Value);
	void InputTag_SlowWalk();

private:
	
};
