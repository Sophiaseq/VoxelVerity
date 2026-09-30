// Fill out your copyright notice in the Description page of Project Settings.


#include "UGC_Damage.h"

#include "UI/Widget/DamageTextComponent.h"

bool UUGC_Damage::OnExecute_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const
{
	if (!MyTarget) return false;
	const float Damage = Parameters.RawMagnitude;
	const FVector SpawnLocation = MyTarget->GetActorLocation() + FVector(0.f, 40.f, 70.f);
	SpawnDamageNumber(MyTarget, SpawnLocation, Damage);
	return true;
}

void UUGC_Damage::SpawnDamageNumber(AActor* Target, const FVector& Location, const float Damage)
{
	UDamageTextComponent* TextComponent = NewObject<UDamageTextComponent>(Target, UDamageTextComponent::StaticClass());
	TextComponent->RegisterComponent();
	TextComponent->SetWorldLocation(Location);
	TextComponent->SetDrawSize(FVector2D(200.f, 80.f));
	TextComponent->SetDamageText(Damage);
}
