// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Interaction/HitInterface.h"
#include "MGCharacterBase.generated.h"

class UHealthBarComponent;
class UMGCharacterDefinition;
class USphereComponent;

UCLASS()
class MUNDUSGRANUM_API AMGCharacterBase : public ACharacter, public IHitInterface
{
	GENERATED_BODY()

public:
	AMGCharacterBase();
	// 初始化函数，由生成器调用（例如：刷怪笼）
	void InitializeAnimal(UMGCharacterDefinition* InDef);
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	void PlayHitReactMontage(const FName& SectionName) const;
	FName GetHitMontageSection(const FVector& HitPoint) const;

	virtual void GetHit_Implementation(const FVector& HitPoint) override;
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override; 

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void SphereOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

#if WITH_EDITOR
	// 将当前胶囊体参数一键保存到 DataAsset
	UFUNCTION(CallInEditor, Category = "Collision Tool")
	void SaveCapsuleToDataAsset();
#endif
	
	// 异步加载完成后的回调
	void OnAnimalAssetsLoaded();
	void ApplyAnimalAssets();
	
	UPROPERTY(EditAnywhere)
	UHealthBarComponent* HealthBarWidget;
	
	UPROPERTY(EditAnywhere)
	USphereComponent* Sphere;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UMGCharacterDefinition> CharacterDef;
	
	UPROPERTY(EditAnywhere, Category =Action)
	UAnimMontage* HitMontage; 

	// 用于防止重复加载
	bool bAssetsLoaded = false;

};
