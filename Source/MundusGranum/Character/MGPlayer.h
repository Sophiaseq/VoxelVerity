// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h" //后续加入能力标签后或许可替换成组件
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "Interaction/MGInteractionReceiver.h"
#include "MundusGranumGameplayTags.h"
#include "MundusGranum/Input/MGInputConfig.h"
#include "MundusGranum/Input/MGInputComponent.h"

#include "MGPlayer.generated.h"

class UMGHandEquipComponent;
class UMGInventoryComponent;
class AMGDroppedItemActor;
class USpringArmComponent;
class USkeletalMeshComponent;
class UCameraComponent;

UCLASS()
class MUNDUSGRANUM_API AMGPlayer : public ACharacter, public IMGInteractionReceiver
{
	GENERATED_BODY()

public:
	AMGPlayer(const FObjectInitializer& ObjectInitializer);
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void BeginPlay() override;
	
	void Input_Move(const FInputActionValue& Value);
	void Input_LookMouse(const FInputActionValue& Value);
	void Input_Jump();
	void Input_SprintPressed();
	void Input_SprintReleased();
	void Input_Pickup();
	void InputTag_UseLeftHandItem();
	void InputTag_UseRightHandItem();
	void InputTag_SelectItem(const FInputActionValue& Value);
	void InputTag_SlowWalk();

	virtual int32 AttemptPickup(UMGItemDefinition* Item, int32 Count) override;
	virtual void AddNearbyDrop(AMGDroppedItemActor* Drop, AActor* OtherActor) override;
	virtual void RemoveNearbyDrop(AMGDroppedItemActor* Drop, AActor* OtherActor) override;
	void PlayAttackMontage() const;
	
	UFUNCTION(BlueprintCallable)
	void SetWeaponCollisionEnabled(ECollisionEnabled::Type CollisionEnabled);
	
	UFUNCTION(BlueprintCallable)
	float GetMovementDirection() const;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	float TurnRateGamepad;
	
protected:
	void TurnAtRate(float Rate);
	void LookUpAtRate(float Rate);

	// 根据当前选中槽刷新手持显示和角色状态
	void RefreshEquippedItemFromSelectedSlot();

	// 委托回调：背包槽变化 / 选中槽切换
	UFUNCTION()
	void HandleInventoryChanged(int32 SlotIndex);

	UFUNCTION()
	void HandleSelectedSlotChanged(int32 NewIndex);
	
	UPROPERTY()
	bool bIsSprinting = false;

	UPROPERTY(EditAnywhere)
	float WanderingSpeed = 200.f;
	UPROPERTY(EditAnywhere)
	float WalkSpeed = 500.f;
	UPROPERTY(EditAnywhere)
	float SprintSpeed = 1000.f;
	
	UPROPERTY()
	TArray<AMGDroppedItemActor*> NearbyDrops;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Items)
	UMGInventoryComponent* InventoryComp;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Items)
	TObjectPtr<UMGHandEquipComponent> HandEquipComp;
	
	UPROPERTY(EditAnywhere, Category = "Input")
	UMGInputConfig* GlobalInputConfig = nullptr;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Input")
	UMGInputComponent* InputComp = nullptr;
	
private:
	UPROPERTY(VisibleInstanceOnly)
	UMGItemDefinition* OverlappingItemDef;
	
	UPROPERTY(VisibleAnywhere, Category = Camera)
	UCameraComponent* CameraComponent;
	
	UPROPERTY(VisibleAnywhere, Category = Camera)
	USpringArmComponent* ArmComponent;
	
	//后续可能会移至能力组件
	UPROPERTY(VisibleInstanceOnly ,Category="Movement | CharacterState")
	FGameplayTag CharacterState = MundusGranumGameplayTags::CharacterState_Unequipped;
	
	UPROPERTY(VisibleInstanceOnly ,Category=Action)
	FGameplayTag ActionState = MundusGranumGameplayTags::ActionState_Unoccupied;
	
	UPROPERTY(EditAnywhere, Category =Action)
	UAnimMontage* AttackMontage; 

public:
	FORCEINLINE void SetOverlappingItem(UMGItemDefinition* Item){OverlappingItemDef = Item;};
	FORCEINLINE [[nodiscard]] UMGInventoryComponent* GetInventoryComp() const{return InventoryComp;}
	FORCEINLINE FGameplayTag GetActionState() const{return ActionState;}
	UFUNCTION(BlueprintCallable)
	FORCEINLINE void SetActionState(const FGameplayTag State){ActionState = State;}
	UFUNCTION(BlueprintCallable)
	FORCEINLINE FGameplayTag GetCharacterState() const{return CharacterState;}
};
