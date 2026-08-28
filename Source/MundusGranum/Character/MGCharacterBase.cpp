// Fill out your copyright notice in the Description page of Project Settings.


#include "MGCharacterBase.h"

#include "Components/SphereComponent.h"
#include "MGCharacterDefinition.h"
#include "Components/CapsuleComponent.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "Engine/SkeletalMesh.h"
#include "HUD/HealthBarComponent.h"
#include "Kismet/KismetSystemLibrary.h"

// Sets default values
AMGCharacterBase::AMGCharacterBase()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// CapsuleComponent 和 SkeletalMeshComponent 由 ACharacter 自动创建
	Sphere = CreateDefaultSubobject<USphereComponent>(FName("Sphere"));
	Sphere->SetSphereRadius(50.f);
	Sphere->SetupAttachment(GetRootComponent());
	
	HealthBarWidget = CreateDefaultSubobject<UHealthBarComponent>(FName("HealthBarWidget"));
	HealthBarWidget->SetupAttachment(GetRootComponent());

}

// Called when the game starts or when spawned
void AMGCharacterBase::BeginPlay()
{
	Super::BeginPlay();
	
	Sphere->OnComponentBeginOverlap.AddDynamic(this,&AMGCharacterBase::OnSphereOverlap);
	Sphere->OnComponentEndOverlap.AddDynamic(this,&AMGCharacterBase::SphereOverlapEnd);
	
	InitializeAnimal(CharacterDef);
	
	if (HealthBarWidget)
	{
		HealthBarWidget->SetHealthPercent(.1f);
	}
}

void AMGCharacterBase::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	FString OtherActorName = OtherActor->GetName();
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(2,30.f,FColor::Blue,OtherActorName);
	}
}

void AMGCharacterBase::SphereOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(2,30.f,FColor::Blue,FString("Ending Overlap with") + OtherActor->GetName());
	}
}

// Called every frame
void AMGCharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AMGCharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void AMGCharacterBase::PlayHitReactMontage(const FName& SectionName) const
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && HitMontage)
	{
		AnimInstance->Montage_Play(HitMontage);
		AnimInstance->Montage_JumpToSection(SectionName, HitMontage);
	}
}

FName AMGCharacterBase::GetHitMontageSection(const FVector& HitPoint) const
{
	FString SectionName = TEXT("Hit");
	const FVector Forward = GetActorForwardVector();
	FVector ToHit = HitPoint - GetActorLocation();
	if (ToHit.Z > 5)
	{
		SectionName += TEXT("High");
	}
	else if (ToHit.Z < -5)
	{
		SectionName += TEXT("Low");
	}
	else 
		SectionName += TEXT("Mid");
	ToHit.Z = GetActorLocation().Z;
	ToHit.Normalize();
	
	const double CosTheta = FVector::DotProduct(Forward, ToHit);
	double Theta = FMath::Acos(CosTheta);
	Theta = FMath::RadiansToDegrees(Theta);
	const FVector CrossProduct = FVector::CrossProduct(Forward, ToHit);
	if (CrossProduct.Z < 0)
	{
		Theta *= -1.f;
	}
	if (Theta >= -45.f && Theta < 45.f)
	{
		SectionName += TEXT("Forward");
	}
	else if (Theta >= 45.f && Theta < 135.f)
	{
		SectionName += TEXT("Right");
	}
	else if (Theta >= -135.f && Theta < -45.f)
	{
		SectionName += TEXT("Left");
	}
	else
	{
		SectionName += TEXT("Back");
	}
	
	UKismetSystemLibrary::DrawDebugArrow(this, GetActorLocation(), GetActorLocation() + CrossProduct * 60.f, 5.f, FColor::Blue, 5.f);
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(1,30.f,FColor::Red,SectionName);
	}
	return FName(SectionName);
}

void AMGCharacterBase::GetHit_Implementation(const FVector& HitPoint)
{
	const FName SectionName = GetHitMontageSection(HitPoint);
	PlayHitReactMontage(SectionName);
}

float AMGCharacterBase::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
	class AController* EventInstigator, AActor* DamageCauser)
{
	return Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
}

void AMGCharacterBase::InitializeAnimal(UMGCharacterDefinition* InDef)
{
	if (!InDef || !HasAuthority()) return; // 服务器决定刷什么

	CharacterDef = InDef;
    
	// 如果已经加载过，直接应用；否则发起异步加载
	if (CharacterDef->SkeletalMesh.Get() && CharacterDef->AnimBlueprintClass.Get())
	{
		ApplyAnimalAssets();
	}
	else
	{
		// 收集需要加载的资产路径
		TArray<FSoftObjectPath> AssetsToLoad;
		AssetsToLoad.Add(CharacterDef->SkeletalMesh.ToSoftObjectPath());
		AssetsToLoad.Add(CharacterDef->AnimBlueprintClass.ToSoftObjectPath());

		// 发起异步加载（可在加载期间显示一个占位球体）
		FStreamableManager& Manager = UAssetManager::GetStreamableManager();
		Manager.RequestAsyncLoad(AssetsToLoad, FStreamableDelegate::CreateUObject(this, &AMGCharacterBase::OnAnimalAssetsLoaded));
	}
}

void AMGCharacterBase::OnAnimalAssetsLoaded()
{
	ApplyAnimalAssets();
}

void AMGCharacterBase::ApplyAnimalAssets()
{
	if (!CharacterDef) return;

	// 应用骨骼网格体
	if (USkeletalMesh* AnimalMesh = CharacterDef->SkeletalMesh.Get())
	{
		GetMesh()->SetSkeletalMesh(AnimalMesh);
		GetMesh()->SetRelativeTransform(CharacterDef->HitCapsule.RelativeTransform);
	}

	// 应用动画蓝图
	if (UClass* AnimClass = CharacterDef->AnimBlueprintClass.Get())
	{
		GetMesh()->SetAnimInstanceClass(AnimClass);
	}
	
	FCapsuleCollisionConfig AnimalCapsule = CharacterDef->HitCapsule;
	GetCapsuleComponent()->SetCapsuleSize(
		AnimalCapsule.CapsuleRadius,
		AnimalCapsule.CapsuleHalfHeight,
		true
	);
	
	

	// 应用数值
	// 注意：这些属性最好也用同步机制，但为了演示直接赋值（如果是服务器，自动会复制给客户端）
	// 如果需要在客户端也生效，建议在 OnRep_AnimalDef 中调用此函数
}

#if WITH_EDITOR

void AMGCharacterBase::SaveCapsuleToDataAsset()
{
	if (!CharacterDef)
	{
		UE_LOG(LogTemp, Warning, TEXT("请先指定 CharacterDef 资产"));
		return;
	}

	const UCapsuleComponent* CapsuleComp = GetCapsuleComponent();
	if (!CapsuleComp)
	{
		UE_LOG(LogTemp, Warning, TEXT("胶囊体组件不存在，无法保存"));
		return;
	}

	// 读取当前胶囊体的无缩放尺寸，写入 DataAsset，数值更精准
	FCapsuleCollisionConfig& CapsuleConfig = CharacterDef->HitCapsule;
	CapsuleConfig.CapsuleRadius = CapsuleComp->GetUnscaledCapsuleRadius();
	CapsuleConfig.CapsuleHalfHeight = CapsuleComp->GetUnscaledCapsuleHalfHeight();
	
	UE_LOG(LogTemp, Log, TEXT("胶囊体配置已保存到 %s"), *CharacterDef->GetName());
}

#endif