

#include "MGCharacterWithAbilities.h"

#include "MGCharacterData.h"
#include "MGPawnData.h"
#include "MGHealthComponent.h"
#include "MGPawnExtensionComponent.h"
#include "AbilitySystem/MGAbilitySet.h"
#include "AbilitySystem/MGAbilitySystemComponent.h"
#include "AbilitySystem/Attributes/MGCombatSet.h"
#include "AbilitySystem/Attributes/MGHealthSet.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Items/Weapons/MGWeaponItemDefinition.h"
#include "Net/UnrealNetwork.h"
#include "UI/Widget/MGUserWidget.h"


AMGCharacterWithAbilities::AMGCharacterWithAbilities()
{
	AbilitySystemComponent = CreateDefaultSubobject<UMGAbilitySystemComponent>("AbilitySystemComponent");
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);

	// These attribute sets will be detected by AbilitySystemComponent::InitializeComponent. Keeping a reference so that the sets don't get garbage collected before that.
	HealthSet = CreateDefaultSubobject<UMGHealthSet>(TEXT("HealthSet"));
	CombatSet = CreateDefaultSubobject<UMGCombatSet>(TEXT("CombatSet"));
	
	HealthBarComponent = CreateDefaultSubobject<UHealthBarComponent>("HealthBarComponent");
	HealthBarComponent->SetupAttachment(GetRootComponent());
	
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;
	GetCharacterMovement()->bUseControllerDesiredRotation = true;
	
	PawnExtComponent->PostReplicatePawnDataDelegate.AddUObject(this, &AMGCharacterWithAbilities::PostReplicatedPawnData);
	
	// AbilitySystemComponent needs to be updated at a high frequency.
	SetNetUpdateFrequency(100.0f);
}

void AMGCharacterWithAbilities::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMGCharacterWithAbilities, WeaponDef);
}

void AMGCharacterWithAbilities::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	check(AbilitySystemComponent);
	AbilitySystemComponent->InitAbilityActorInfo(this, this);

	SetPawnData();

	// SetPawnData 会通过 AbilitySet 生成 HealthSet 并应用默认 GE，所以必须在它之后初始化 HealthComponent
	//HealthComponent->InitializeWithAbilitySystem(AbilitySystemComponent);
}

void AMGCharacterWithAbilities::BeginPlay()
{
	Super::BeginPlay();
}

UAbilitySystemComponent* AMGCharacterWithAbilities::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

float AMGCharacterWithAbilities::GetCharacterLevel()
{
	return GetNonPlayerLevel();
}

const UMGWeaponItemDefinition* AMGCharacterWithAbilities::GetCurrentWeapon() const
{
	return WeaponDef;
}

void AMGCharacterWithAbilities::SetPawnData()
{
	/*if (GetLocalRole() != ROLE_Authority)
	{
		return;
	}*/
	
	const UMGCharacterData* CharacterDef = Cast<UMGCharacterData>(PawnExtComponent->GetPawnData<UMGPawnData>());
	
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
	
	Tags.AddUnique(CharacterDef->CharacterName);
	GetMesh()->SetSkeletalMesh(CharacterDef->CharacterMesh);
	GetMesh()->SetAnimInstanceClass(CharacterDef->Anim);
	
	//ForceNetUpdate();
}

void AMGCharacterWithAbilities::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	
	if (!HasAuthority()) return;
	MGAIController = Cast<AMGAIController>(NewController);
	MGAIController->GetBlackboardComponent()->InitializeBlackboard(*BehaviorTree->BlackboardAsset);
	MGAIController->RunBehaviorTree(BehaviorTree);
	MGAIController->GetBlackboardComponent()->SetValueAsBool(FName("HitReacting"), false);
	MGAIController->GetBlackboardComponent()->SetValueAsBool(FName("RangedAttacker"), ActorHasTag(FName("Enemy.Ranged")));
}

void AMGCharacterWithAbilities::OnRep_WeaponDef()
{
	if (!WeaponDef) return;
	const FEquipDisplayData& EquipDisplayData = WeaponDef->EquipDisplayData;
	if (EquipDisplayData.SkeletalMesh)
	{
		USkeletalMeshComponent* WeaponSkeletalMesh = NewObject<USkeletalMeshComponent>(this);
		WeaponSkeletalMesh->SetupAttachment(GetMesh(), FName("hand_r"));
		WeaponSkeletalMesh->RegisterComponent();
		WeaponSkeletalMesh->SetSkeletalMesh(EquipDisplayData.SkeletalMesh);
		WeaponSkeletalMesh->SetRelativeTransform(EquipDisplayData.EquippedTransform);
		WeaponSkeletalMesh->ComponentTags.Add(FName("Component.Mesh.Weapon"));
		WeaponSkeletalMesh->SetCollisionResponseToAllChannels(ECR_Overlap);
	}
	else if (EquipDisplayData.StaticMesh)
	{
		UStaticMeshComponent* WeaponStaticMesh = NewObject<UStaticMeshComponent>(this);
		WeaponStaticMesh->SetupAttachment(GetMesh(), FName("hand_r"));
		WeaponStaticMesh->RegisterComponent();
		WeaponStaticMesh->SetStaticMesh(EquipDisplayData.StaticMesh);
		WeaponStaticMesh->SetRelativeTransform(EquipDisplayData.EquippedTransform);
		WeaponStaticMesh->ComponentTags.Add(FName("Component.Mesh.Weapon"));
		WeaponStaticMesh->SetCollisionResponseToAllChannels(ECR_Overlap);
	}
}

void AMGCharacterWithAbilities::HitReactTagChanged(const FGameplayTag CallbackTag, int32 NewCount)
{
	Super::HitReactTagChanged(CallbackTag, NewCount);
	
	MGAIController->GetBlackboardComponent()->SetValueAsBool(FName("HitReacting"), true);
}

void AMGCharacterWithAbilities::PostReplicatedPawnData()
{
	SetPawnData();

	// SetPawnData 会通过 AbilitySet 生成 HealthSet 并应用默认 GE，所以必须在它之后初始化 HealthComponent
	HealthComponent->InitializeWithAbilitySystem(AbilitySystemComponent);
	
	if (UMGUserWidget* MGWidget = Cast<UMGUserWidget>(HealthBarComponent->GetUserWidgetObject()))
	{
		MGWidget->SetWidgetController(HealthComponent);
	}
}

