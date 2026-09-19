

#include "MGCharacterWithAbilities.h"

#include "MGPawnData.h"
#include "MGHealthComponent.h"
#include "MGPawnExtensionComponent.h"
#include "AbilitySystem/MGAbilitySet.h"
#include "AbilitySystem/MGAbilitySystemComponent.h"
#include "AbilitySystem/Attributes/MGCombatSet.h"
#include "UI/Widget/MGUserWidget.h"


AMGCharacterWithAbilities::AMGCharacterWithAbilities()
{
	AbilitySystemComponent = CreateDefaultSubobject<UMGAbilitySystemComponent>("AbilitySystemComponent");
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);

	// These attribute sets will be detected by AbilitySystemComponent::InitializeComponent. Keeping a reference so that the sets don't get garbage collected before that.
	/*HealthSet = CreateDefaultSubobject<UMGHealthSet>(TEXT("HealthSet"));
	CombatSet = CreateDefaultSubobject<UMGCombatSet>(TEXT("CombatSet"));*/
	
	HealthBarComponent = CreateDefaultSubobject<UHealthBarComponent>("HealthBarComponent");
	HealthBarComponent->SetupAttachment(GetRootComponent());
	
	// AbilitySystemComponent needs to be updated at a high frequency.
	SetNetUpdateFrequency(100.0f);
}

void AMGCharacterWithAbilities::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	check(AbilitySystemComponent);
	AbilitySystemComponent->InitAbilityActorInfo(this, this);

	SetPawnData();

	// SetPawnData 会通过 AbilitySet 生成 HealthSet 并应用默认 GE，所以必须在它之后初始化 HealthComponent
	HealthComponent->InitializeWithAbilitySystem(AbilitySystemComponent);
}

void AMGCharacterWithAbilities::BeginPlay()
{
	Super::BeginPlay();
	
	if (UMGUserWidget* MGWidget = Cast<UMGUserWidget>(HealthBarComponent->GetUserWidgetObject()))
	{
		MGWidget->SetWidgetController(HealthComponent);
	}
}

UAbilitySystemComponent* AMGCharacterWithAbilities::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

float AMGCharacterWithAbilities::GetCharacterLevel()
{
	return GetNonPlayerLevel();
}

void AMGCharacterWithAbilities::SetPawnData() const
{
	if (GetLocalRole() != ROLE_Authority)
	{
		return;
	}
	
	const UMGPawnData* CharacterDef = PawnExtComponent->GetPawnData<UMGPawnData>();
	
	if (!CharacterDef)
	{
		UE_LOG(LogTemp, Warning, TEXT("Fail to set PawnData [%s] on CharacterWithAbility [%s]"), *GetNameSafe(CharacterDef), *GetNameSafe(this));
		return;
	}

	MARK_PROPERTY_DIRTY_FROM_NAME(UMGPawnExtensionComponent, PawnData, this);
     
	for (const UMGAbilitySet* AbilitySet : CharacterDef->AbilitySets)
	{
		if (AbilitySet)
		{
			AbilitySet->GiveToAbilitySystem(AbilitySystemComponent, nullptr);
		}
	}
	
	//不确定
	//ForceNetUpdate();
}

