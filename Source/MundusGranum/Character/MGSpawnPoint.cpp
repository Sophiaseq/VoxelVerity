// Fill out your copyright notice in the Description page of Project Settings.


#include "MGSpawnPoint.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "MGCharacterData.h"
#include "MGCharacterWithAbilities.h"
#include "MGPawnData.h"
#include "MGPawnExtensionComponent.h"
#include "AbilitySystem/MGAbilitySystemComponent.h"
#include "GameFramework/Character.h"
#include "Net/UnrealNetwork.h"

AMGSpawnPoint::AMGSpawnPoint()
{
}

void AMGSpawnPoint::SpawnCharacter()
{
	SpawnedPawn = PerformSpawn();
	if (ACharacter* Character = Cast<ACharacter>(SpawnedPawn))
	{
		SpawnItem(Character);
	}
}

// Called when the game starts or when spawned
void AMGSpawnPoint::BeginPlay()
{
	if (SpawnDelay > 0.0f)
	{
		FTimerHandle TimerHandle;
		GetWorldTimerManager().SetTimer(TimerHandle, [this]() { SpawnedPawn = PerformSpawn(); }, SpawnDelay, false);
	}
	else
	{
		SpawnedPawn = PerformSpawn();
	}
}

APawn* AMGSpawnPoint::PerformSpawn()
{
	if (!PawnData)
	{
		UE_LOG(LogTemp, Error, TEXT("SpawnPoint [%s]: PawnData is null!"), *GetNameSafe(this));
		return nullptr;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}
	
	if (!bAutoSpawnOnBeginPlay || !HasAuthority()) return nullptr;
	
	if (SpawnedPawn) return SpawnedPawn;
	
	const FTransform SpawnTransform = GetActorTransform();

	// 1. 创建 AIController
	FActorSpawnParameters ControllerSpawnInfo;
	ControllerSpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AMGAIController* MGAIController = World->SpawnActor<AMGAIController>(
		AMGAIController::StaticClass(), SpawnTransform, ControllerSpawnInfo);

	if (!IsValid(MGAIController))
	{
		UE_LOG(LogTemp, Error, TEXT("SpawnPoint: Failed to spawn AIController"));
		return nullptr;
	}
	
	FActorSpawnParameters SpawnInfo;
	SpawnInfo.Owner = MGAIController;
	SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	SpawnInfo.ObjectFlags |= RF_Transient;  // 不保存到地图
	SpawnInfo.bDeferConstruction = true;    // 关键：延迟构建

	if (UClass* PawnClass = PawnData->PawnClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("GameMode::SpawnDefaultPawnAtTransform → SpawnActor(%s)"), *GetNameSafe(PawnClass));

		if (APawn* Pawn = GetWorld()->SpawnActor<APawn>(PawnClass, SpawnTransform, SpawnInfo))
		{
			if (UMGPawnExtensionComponent* PawnExtComp = UMGPawnExtensionComponent::FindPawnExtensionComponent(Pawn))
			{
				PawnExtComp->SetPawnData(PawnData);
				if (UMGAbilitySystemComponent* ASC = Cast<UMGAbilitySystemComponent>(UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Pawn)))
				{
					PawnExtComp->InitializeAbilitySystem(ASC, Pawn);
				}
			}

			Pawn->FinishSpawning(SpawnTransform);
			
			if (IsValid(MGAIController) && IsValid(Pawn))
			{
				MGAIController->Possess(Pawn);

				// 验证 Possess 是否成功
				if (Pawn->GetController() != MGAIController)
				{
					UE_LOG(LogTemp, Warning, TEXT("SpawnPoint: Possess failed! Character controller: %s"),
						*GetNameSafe(Pawn->GetController()));
				}
			}

			return Pawn;
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Game mode was unable to spawn Pawn of class [%s] at [%s]."), *GetNameSafe(PawnClass), *SpawnTransform.ToHumanReadableString());
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Game mode was unable to spawn Pawn due to NULL pawn class."));
	}

	return nullptr;
}

void AMGSpawnPoint::SpawnItem(ACharacter* CharacterToAttach) const
{
	if (!WeaponDef || !CharacterToAttach) return;
	const FEquipDisplayData& EquipDisplayData = WeaponDef->EquipDisplayData;
	if (EquipDisplayData.SkeletalMesh)
	{
		USkeletalMeshComponent* WeaponSkeletalMesh = NewObject<USkeletalMeshComponent>(CharacterToAttach);
		WeaponSkeletalMesh->SetupAttachment(CharacterToAttach->GetMesh(), FName("WeaponSocket"));
		WeaponSkeletalMesh->RegisterComponent();
		WeaponSkeletalMesh->SetSkeletalMesh(EquipDisplayData.SkeletalMesh);
		WeaponSkeletalMesh->SetRelativeTransform(EquipDisplayData.EquippedTransform);
		WeaponSkeletalMesh->ComponentTags.Add(FName("Component.Mesh.Weapon"));
		WeaponSkeletalMesh->SetCollisionResponseToAllChannels(ECR_Overlap);
		WeaponSkeletalMesh->SetAnimClass(EquipDisplayData.EquipAnimClass);
	}
	else if (EquipDisplayData.StaticMesh)
	{
		UStaticMeshComponent* WeaponStaticMesh = NewObject<UStaticMeshComponent>(CharacterToAttach);
		WeaponStaticMesh->SetupAttachment(CharacterToAttach->GetMesh(), FName("WeaponSocket"));
		WeaponStaticMesh->RegisterComponent();
		WeaponStaticMesh->SetStaticMesh(EquipDisplayData.StaticMesh);
		WeaponStaticMesh->SetRelativeTransform(EquipDisplayData.EquippedTransform);
		WeaponStaticMesh->ComponentTags.Add(FName("Component.Mesh.Weapon"));
		WeaponStaticMesh->SetCollisionResponseToAllChannels(ECR_Overlap);
	}
	else
	{
		return;
	}
	if (AMGCharacterWithAbilities* NPC = Cast<AMGCharacterWithAbilities>(CharacterToAttach))
	{
		NPC->SetWeaponDef(WeaponDef);
	}
}




