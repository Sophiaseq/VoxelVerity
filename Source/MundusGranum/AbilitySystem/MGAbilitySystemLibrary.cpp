// Fill out your copyright notice in the Description page of Project Settings.


#include "MGAbilitySystemLibrary.h"

#include "MGAbilitySystemComponent.h"
#include "MundusGranum.h"
#include "Attributes/MGHealthSet.h"
#include "Engine/OverlapResult.h"
#include "GameModes/MGExperienceManagerComponent.h"
#include "GameModes/MGGameMode.h"
#include "GameModes/MGGameState.h"
#include "Kismet/GameplayStatics.h"
#include "Player/MGPlayerState.h"
#include "UI/HUD/MGHUD.h"
#include "UI/WidgetController/MGWidgetController.h"

UAttributeMenuWidgetController* UMGAbilitySystemLibrary::GetAttributeMenuWidgetController(const UObject* WorldContextObject)
{
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(WorldContextObject, 0))
	{
		if (AMGHUD* MGHUD = Cast<AMGHUD>(PC->GetHUD()))
		{
			AMGPlayerState* PS = PC->GetPlayerState<AMGPlayerState>();
			UMGAbilitySystemComponent* ASC = PS->GetMGAbilitySystemComponent();
			const TArray<UAttributeSet*> Attributes= ASC->GetSpawnedAttributes();
			const FWidgetControllerParams Params(PC, PS, ASC, Attributes);
			return MGHUD->GetAttributeMenuWidgetController(Params);
		}
	}
	return nullptr;
}

const UMGPawnData* UMGAbilitySystemLibrary::GetDefaultPawnData(const UObject* WorldContextObject)
{
	const AMGGameState* MGGameState = Cast<AMGGameState>(UGameplayStatics::GetGameMode(WorldContextObject)->GameState);
	if (!MGGameState) return nullptr;

	const UMGExperienceManagerComponent* ExperienceManagerComponent = MGGameState->FindComponentByClass<UMGExperienceManagerComponent>();
	if (!ExperienceManagerComponent) return nullptr;
	
	return ExperienceManagerComponent->GetCurrentExperienceChecked()->DefaultPawnData;
}

bool UMGAbilitySystemLibrary::IsBlockedHit(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if (const FMGGameplayEffectContext* MGEffectContext = static_cast<const FMGGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		return MGEffectContext->IsBlockedHit();
	}
	return false;
}

bool UMGAbilitySystemLibrary::IsCriticalHit(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if (const FMGGameplayEffectContext* MGEffectContext = static_cast<const FMGGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		return MGEffectContext->IsCriticalHit();
	}
	return false;
}

void UMGAbilitySystemLibrary::SetIsBlockedHit(FGameplayEffectContextHandle& EffectContextHandle, bool bInIsBlockedHit)
{
	if (FMGGameplayEffectContext* MGEffectContext = static_cast<FMGGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		MGEffectContext->SetIsBlockedHit(bInIsBlockedHit);
	}
}

void UMGAbilitySystemLibrary::SetIsCriticalHit(FGameplayEffectContextHandle& EffectContextHandle, bool bInIsCriticalHit)
{
	if (FMGGameplayEffectContext* MGEffectContext = static_cast<FMGGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		MGEffectContext->SetIsCriticalHit(bInIsCriticalHit);
	}
}

void UMGAbilitySystemLibrary::GetLivePlayersWithinRadius(const UObject* WorldContextObject,
	TArray<AActor*>& OutOverlappingActors, const TArray<AActor*>& ActorsToIgnore, float Radius,
	const FVector& SphereLocation)
{
	FCollisionQueryParams SphereParams;
	SphereParams.AddIgnoredActors(ActorsToIgnore);
	
	TArray<FOverlapResult> Overlaps;
	if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		World->OverlapMultiByObjectType(Overlaps, SphereLocation, FQuat::Identity, FCollisionObjectQueryParams(FCollisionObjectQueryParams::InitType::AllDynamicObjects), FCollisionShape::MakeSphere(Radius), SphereParams);
		for (const FOverlapResult& Overlap : Overlaps)
		{
			if (Overlap.GetActor()->Implements<UCombatInterface>() && !ICombatInterface::Execute_IsDead(Overlap.GetActor()))
			{
				OutOverlappingActors.AddUnique(Overlap.GetActor());
			}
		}
	}
}

AActor* UMGAbilitySystemLibrary::FindTargetByCameraTrace(const UObject* WorldContextObject, float MaxDistance)
{
	APlayerController* PC = UGameplayStatics::GetPlayerController(WorldContextObject, 0);
	if (!PC) return nullptr;

	FVector CameraLocation;
	FRotator CameraRotation;
	PC->GetPlayerViewPoint(CameraLocation, CameraRotation);

	const FVector TraceEnd = CameraLocation + CameraRotation.Vector() * MaxDistance;

	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(PC->GetPawn());
	Params.bTraceComplex = false;

	if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
    	bool bHit = World->LineTraceSingleByChannel(
    		Hit,
    		CameraLocation,
    		TraceEnd,
    		ECC_CameraTrace,
    		Params
    	);
		
#if ENABLE_DRAW_DEBUG
	    {
            const float DrawTime = 2.0f;  // 显示 2 秒
    
            // 射线起点（摄像机位置）- 蓝色球
            DrawDebugSphere(
                World,
                CameraLocation,
                10.f,          // 半径
                12,            // 分段
                FColor::Blue,
                false,         // 不持久
                DrawTime,
                0,             // 深度优先级
                2.f            // 线宽
            );
    
            // 射线终点 - 红色球
            DrawDebugSphere(
                World,
                TraceEnd,
                10.f,
                12,
                FColor::Red,
                false,
                DrawTime,
                0,
                2.f
            );
    
            // 射线本体
            // 命中：绿色；未命中：黄色
            FColor LineColor = bHit ? FColor::Green : FColor::Yellow;
            DrawDebugLine(
                World,
                CameraLocation,
                TraceEnd,
                LineColor,
                false,
                DrawTime,
                0,
                2.f            // 线宽
            );
    
            // 命中点 - 橙色球 + 法线
            if (bHit)
            {
                DrawDebugSphere(
                    World,
                    Hit.ImpactPoint,
                    15.f,
                    12,
                    FColor::Orange,
                    false,
                    DrawTime,
                    0,
                    3.f
                );
    
                // 命中法线
                DrawDebugDirectionalArrow(
                    World,
                    Hit.ImpactPoint,
                    Hit.ImpactPoint + Hit.ImpactNormal * 50.f,
                    20.f,
                    FColor::Cyan,
                    false,
                    DrawTime,
                    0,
                    2.f
                );
    
                // 命中 Actor 名字
                DrawDebugString(
                    World,
                    Hit.ImpactPoint + FVector(0, 0, 50.f),
                    FString::Printf(TEXT("Hit: %s"), *GetNameSafe(Hit.GetActor())),
                    nullptr,
                    FColor::White,
                    DrawTime,
                    true,      // 显示阴影
                    1.5f       // 字体缩放
                );
            }
            else
            {
                // 未命中提示
                DrawDebugString(
                    World,
                    TraceEnd,
                    TEXT("No Hit"),
                    nullptr,
                    FColor::Yellow,
                    DrawTime,
                    true,
                    1.5f
                );
            }
    
            // 摄像机朝向箭头
            DrawDebugDirectionalArrow(
                World,
                CameraLocation,
                CameraLocation + CameraRotation.Vector() * 100.f,
                30.f,
                FColor::Magenta,
                false,
                DrawTime,
                0,
                3.f
            );
        }	
#endif
    
    	if (bHit)
    	{
    		return Hit.GetActor();
    	}	
	}
	return nullptr;
}

AActor* UMGAbilitySystemLibrary::GetClosestActor(AActor* Observer, const TArray<AActor*>& Actors)
{
	AActor* ClosestActor = nullptr;
	float MinDistance = MAX_FLT;

	if (!Observer) return nullptr;

	for (AActor* Actor : Actors)
	{
		if (Actor && Actor != Observer)
		{
			float CurrentDistance = Observer->GetDistanceTo(Actor);
			if (CurrentDistance < MinDistance)
			{
				MinDistance = CurrentDistance;
				ClosestActor = Actor;
			}
		}
	}
	return ClosestActor;
}
