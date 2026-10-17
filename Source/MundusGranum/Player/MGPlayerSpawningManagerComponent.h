// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/GameStateComponent.h"
#include "MGPlayerSpawningManagerComponent.generated.h"


class UMGHealthComponent;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MUNDUSGRANUM_API UMGPlayerSpawningManagerComponent : public UGameStateComponent
{
	GENERATED_BODY()

public:
	UMGPlayerSpawningManagerComponent(const FObjectInitializer& InitializerInitializer);
	
	//~Begin UActorComponent
	virtual void InitializeComponent() override;
	//~End UActorComponent
	
	void RecordDeathTransform(AController* Controller, const FTransform& Transform);
	
protected:
	static APlayerStart* GetFirstRandomPlayerStart(AController* Controller, const TArray<APlayerStart*>& StartPoints);
	
	//根据不同的Experience重写为不同的策略
	virtual AActor* OnChoosePlayerStart(AController* Player, TArray<APlayerStart*>& PlayerStarts) { return nullptr; }
	virtual void OnFinishRestartPlayer(AController* Player, const FRotator& StartRotation);
	
	UFUNCTION(BlueprintImplementableEvent, meta=(DisplayName=OnFinishRestartPlayer))
	void K2_OnFinishRestartPlayer(AController* Player, const FRotator& StartRotation);

private:
	//将AMGGameMode将这些调用代理到此组件，以便每个Experience能更容易地自定义其所需的重生系统。
	AActor* ChoosePlayerStart(AController* Player);
	bool ControllerCanRestart(AController* Player);
	void FinishRestartPlayer(AController* NewPlayer, const FRotator& StartRotation);
	friend class AMGGameMode;
	/** ~AMGGameMode */
	
	void OnLevelAdded(ULevel* InLevel, UWorld* InWorld);
	void HandleOnActorSpawned(AActor* SpawnedActor);

	UFUNCTION()
	void HandlePawnDeathStarted(AActor* OwningActor);
	void HandleNewPawn(APawn* NewPawn);
	
	UPROPERTY(Transient)
	TArray<TWeakObjectPtr<APlayerStart>> CachedPlayerStarts;
	
	TMap<TWeakObjectPtr<AController>, FTransform> DeathTransforms;
	
	UPROPERTY(Transient)
	TArray<TObjectPtr<APlayerStart>> TempPlayerStarts;
	
#if WITH_EDITOR
	APlayerStart* FindPlayFromHereStart(AController* Player);
#endif

};
