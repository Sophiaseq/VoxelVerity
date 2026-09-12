

#include "MGCharacterWithAbilities.h"

#include "AbilitySystem/MGAbilitySystemComponent.h"
#include "AbilitySystem/Attributes/MGCombatSet.h"
#include "AbilitySystem/Attributes/MGHealthSet.h"


AMGCharacterWithAbilities::AMGCharacterWithAbilities()
{
	AbilitySystemComponent = CreateDefaultSubobject<UMGAbilitySystemComponent>("AbilitySystemComponent");
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);

	// These attribute sets will be detected by AbilitySystemComponent::InitializeComponent. Keeping a reference so that the sets don't get garbage collected before that.
	HealthSet = CreateDefaultSubobject<UMGHealthSet>(TEXT("HealthSet"));
	CombatSet = CreateDefaultSubobject<UMGCombatSet>(TEXT("CombatSet"));

	// AbilitySystemComponent needs to be updated at a high frequency.
	SetNetUpdateFrequency(100.0f);
}

void AMGCharacterWithAbilities::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	check(AbilitySystemComponent);
	AbilitySystemComponent->InitAbilityActorInfo(this, this);
}

UAbilitySystemComponent* AMGCharacterWithAbilities::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

float AMGCharacterWithAbilities::GetCharacterLevel()
{
	return GetNonPlayerLevel();
}

