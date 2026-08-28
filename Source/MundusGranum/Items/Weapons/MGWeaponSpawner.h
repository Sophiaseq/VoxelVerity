// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MGWeaponSpawner.generated.h"

class USphereComponent;
class UStaticMeshComponent;

// DEPRECATED：旧的"直接 attach 到右手、不进背包"拾取路径，已被 AMGDroppedItemActor + 背包系统取代，请勿再使用
UCLASS(Blueprintable)
class MUNDUSGRANUM_API AMGWeaponSpawner : public AActor
{
	GENERATED_BODY()

public:
	AMGWeaponSpawner();
	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;
	
	UFUNCTION(BlueprintCallable)
	void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
	UFUNCTION(BlueprintCallable)
	void SphereOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UStaticMeshComponent* WeaponMesh;
	
	UPROPERTY(EditAnywhere)
	USphereComponent* Sphere;
};
