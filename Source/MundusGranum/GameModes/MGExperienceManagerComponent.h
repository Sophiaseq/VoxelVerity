#pragma once

#include "GameFeaturePluginOperationResult.h"
#include "Components/GameStateComponent.h"
#include "MGExperienceDefinition.h"
#include "MGExperienceManagerComponent.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnMGExperienceLoaded, const UMGExperienceDefinition* /*Experience*/);

enum class EMGExperienceLoadState
{
	Unloaded,
	Loading,
	LoadingGameFeatures,
	LoadingChaosTestingDelay,
	ExecutingActions,
	Loaded,
	Deactivating
};

UCLASS()
class UMGExperienceManagerComponent : public UGameStateComponent
{
	GENERATED_BODY()

public:
	UMGExperienceManagerComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// 设置当前体验（传入 PrimaryAssetId）
	void SetCurrentExperience(FPrimaryAssetId ExperienceId);

	// Ensures the delegate is called once the experience has been loaded,
	// before others are called.
	// However, if the experience has already loaded, calls the delegate immediately.
	void CallOrRegister_OnExperienceLoaded_HighPriority(FOnMGExperienceLoaded::FDelegate&& Delegate);

	// Ensures the delegate is called once the experience has been loaded
	// If the experience has already loaded, calls the delegate immediately
	void CallOrRegister_OnExperienceLoaded(FOnMGExperienceLoaded::FDelegate&& Delegate);

	// Ensures the delegate is called once the experience has been loaded
	// If the experience has already loaded, calls the delegate immediately
	void CallOrRegister_OnExperienceLoaded_LowPriority(FOnMGExperienceLoaded::FDelegate&& Delegate);

	// 检查是否加载完成
	bool IsExperienceLoaded() const;

	// 获取当前体验（仅在加载完成后有效）
	const UMGExperienceDefinition* GetCurrentExperienceChecked() const;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	void StartExperienceLoad();
	void OnExperienceLoadComplete();
	void OnGameFeaturePluginLoadComplete(const UE::GameFeatures::FResult& Result);
	void OnExperienceFullLoadCompleted();

	EMGExperienceLoadState LoadState = EMGExperienceLoadState::Unloaded;
	
	// 当前体验资产（非复制，仅本地使用，若需网络可加 Replicated）
	UPROPERTY()
	TObjectPtr<const UMGExperienceDefinition> CurrentExperience;

	// 正在加载的插件计数
	int32 NumGameFeaturePluginsLoading = 0;

	// 已激活的插件 URL 列表（用于清理）
	TArray<FString> GameFeaturePluginURLs;
	
	/**
 * Delegate called when the experience has finished loading just before others
 * (e.g., subsystems that set up for regular gameplay)
 */
	FOnMGExperienceLoaded OnExperienceLoaded_HighPriority;

	/** Delegate called when the experience has finished loading */
	FOnMGExperienceLoaded OnExperienceLoaded;

	/** Delegate called when the experience has finished loading */
	FOnMGExperienceLoaded OnExperienceLoaded_LowPriority;
};