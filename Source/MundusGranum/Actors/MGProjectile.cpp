// Fill out your copyright notice in the Description page of Project Settings.


#include "MGProjectile.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "MundusGranum.h"
#include "AbilitySystem/MGAbilitySystemLibrary.h"
#include "AbilitySystem/Abilities/MGGameplayAbility_Damage.h"
#include "Components/SphereComponent.h"



AMGProjectile::AMGProjectile()
{
	PrimaryActorTick.bCanEverTick = true;
	
	Sphere = CreateDefaultSubobject<USphereComponent>(FName("Sphere"));
	SetRootComponent(Sphere);
	Sphere->SetCollisionObjectType(ECC_Projectile);
	Sphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Sphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	Sphere->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
	Sphere->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Overlap);
	Sphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(FName("Projectile"));
	ProjectileMovement->MaxSpeed = 550.f;
	ProjectileMovement->ProjectileGravityScale = 0.f;
}

void AMGProjectile::BeginPlay()
{
	Super::BeginPlay();
	SetLifeSpan(LifeSpan);
	Sphere->OnComponentBeginOverlap.AddDynamic(this, &AMGProjectile::OnSphereOverlap);
	//UAudioComponent* AudioComponent = UGameplayStatics::SpawnSoundAttached(LoopingSound, GetRootComponent());
}

void AMGProjectile::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
                                    bool bFromSweep, const FHitResult& SweepResult)
{
	//UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation());
	//UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, ImpactEffect, GetActorLocation());
	//LoopingSoundComponent->Stop();
	if (OtherActor == nullptr || OtherActor == GetInstigator() || UMGAbilitySystemLibrary::IsSameTeamByActorTags(OtherActor, GetInstigator()) || OtherActor == this)
	{
		return;
	}
	if (HasAuthority())
	{
		if (DamagedActors.Contains(OtherActor))
		{
			return;
		}
		
		FGameplayEffectSpec* EffectSpec = DamageEffectSpecHandle.Data.Get();
		if (EffectSpec)
		{
			FGameplayEffectContextHandle ContextHandle = EffectSpec->GetContext();
			if (ContextHandle.IsValid())
			{
				if (const UMGGameplayAbility_Damage* DamageAbility = Cast<UMGGameplayAbility_Damage>(ContextHandle.GetAbilityInstance_NotReplicated()))
				{
					DamageAbility->CauseDamage(OtherActor);
					DamagedActors.Add(OtherActor);
				}
			}
		}
		//Destroy();
	}
	else
	{
		bHit = true;
	}
}

void AMGProjectile::Destroyed()
{
	if (!bHit && !HasAuthority())
	{
		//UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation());
		//UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, ImpactEffect, GetActorLocation());
		//LoopingSoundComponent->Stop();
	}
	Super::Destroyed();
}



