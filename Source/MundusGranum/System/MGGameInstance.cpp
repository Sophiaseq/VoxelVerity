// Copyright Epic Games, Inc. All Rights Reserved.

#include "MGGameInstance.h"

#include "Components/GameFrameworkComponentManager.h"
#include "MGLogChannels.h"
#include "MundusGranumGameplayTags.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(MGGameInstance)

void UMGGameInstance::Init()
{
	Super::Init();

	UE_LOG(LogMG, Warning, TEXT("[Init] GameInstance::Init — 注册 InitState 四态顺序"));

	// 关键：注册 InitState 四态的先后顺序。
	// UGameFrameworkComponentManager::IsInitStateAfterOrEqual 靠这个顺序判断"谁在前谁在后"，
	// 不注册的话（顺序列表为空），只有"状态相等"才返回 true，跨状态的比较一律 false，
	// 会导致 Hero 的 DataAvailable→DataInitialized 门控（HasFeatureReachedInitState）永远过不去。
	UGameFrameworkComponentManager* ComponentManager = GetSubsystem<UGameFrameworkComponentManager>(this);

	if (ensure(ComponentManager))
	{
		ComponentManager->RegisterInitState(MundusGranumGameplayTags::InitState_Spawned,        false, FGameplayTag());
		ComponentManager->RegisterInitState(MundusGranumGameplayTags::InitState_DataAvailable,   false, MundusGranumGameplayTags::InitState_Spawned);
		ComponentManager->RegisterInitState(MundusGranumGameplayTags::InitState_DataInitialized, false, MundusGranumGameplayTags::InitState_DataAvailable);
		ComponentManager->RegisterInitState(MundusGranumGameplayTags::InitState_GameplayReady,  false, MundusGranumGameplayTags::InitState_DataInitialized);
	}
}
