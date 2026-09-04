// Fill out your copyright notice in the Description page of Project Settings.


#include "MGWeaponSpawner.h"

#include "Character/MGCharacter.h"
#include "Components/SphereComponent.h"

// Sets default values
AMGWeaponSpawner::AMGWeaponSpawner()
{
	PrimaryActorTick.bCanEverTick = true;
	
	WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>("WeaponMesh");
	RootComponent = WeaponMesh;
	Sphere = CreateDefaultSubobject<USphereComponent>(FName("Sphere"));
	Sphere->SetSphereRadius(50.f);
	Sphere->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void AMGWeaponSpawner::BeginPlay()
{
	Super::BeginPlay();
	Sphere->OnComponentBeginOverlap.AddDynamic(this,&AMGWeaponSpawner::OnSphereOverlap);
	Sphere->OnComponentEndOverlap.AddDynamic(this,&AMGWeaponSpawner::SphereOverlapEnd);
}

void AMGWeaponSpawner::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	FString OtherActorName = OtherActor->GetName();
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(1,30.f,FColor::Blue,OtherActorName);
	}
	if (AMGCharacter* CharacterBase =  Cast<AMGCharacter>(OtherActor))
	{
		FAttachmentTransformRules AttachmentTransformRules(EAttachmentRule::SnapToTarget, true);
		WeaponMesh->AttachToComponent(CharacterBase->GetMesh(), AttachmentTransformRules, FName("hand_r"));
		SetActorEnableCollision(false);
		DisableComponentsSimulatePhysics();
	}
}

void AMGWeaponSpawner::SphereOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(1,30.f,FColor::Blue,FString("Ending Overlap with") + OtherActor->GetName());
	}
}

// Called every frame
void AMGWeaponSpawner::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

