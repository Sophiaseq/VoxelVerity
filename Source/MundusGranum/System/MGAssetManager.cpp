// Fill out your copyright notice in the Description page of Project Settings.


#include "MGAssetManager.h"

#include "AbilitySystemGlobals.h"
#include "MGLogChannels.h"

UMGAssetManager::UMGAssetManager()
{
}

UMGAssetManager& UMGAssetManager::Get()
{
	check(GEngine);

	if (UMGAssetManager* Singleton = Cast<UMGAssetManager>(GEngine->AssetManager))
	{
		return *Singleton;
	}

	UE_LOG(LogMG, Fatal, TEXT("Invalid AssetManagerClassName in DefaultEngine.ini.  It must be set to MGAssetManager!"));

	// Fatal error above prevents this from being called.
	return *NewObject<UMGAssetManager>();
}

void UMGAssetManager::StartInitialLoading()
{
	Super::StartInitialLoading();
	
	//TODO 早期版本的重要步骤，可能需要，需确认
	UAbilitySystemGlobals::Get().InitGlobalData();
}

const UMGPawnData* UMGAssetManager::GetDefaultPawnData() const
{
	return GetAsset(DefaultPawnData);
}

UObject* UMGAssetManager::SynchronousLoadAsset(const FSoftObjectPath& AssetPath)
{
	if (AssetPath.IsValid())
	{
		TUniquePtr<FScopeLogTime> LogTimePtr;

		if (ShouldLogAssetLoads())
		{
			LogTimePtr = MakeUnique<FScopeLogTime>(*FString::Printf(TEXT("Synchronously loaded asset [%s]"), *AssetPath.ToString()), nullptr, FScopeLogTime::ScopeLog_Seconds);
		}

		if (UAssetManager::IsInitialized())
		{
			return UAssetManager::GetStreamableManager().LoadSynchronous(AssetPath, false);
		}

		// Use LoadObject if asset manager isn't ready yet.
		return AssetPath.TryLoad();
	}

	return nullptr;
}

bool UMGAssetManager::ShouldLogAssetLoads()
{
	static bool bLogAssetLoads = FParse::Param(FCommandLine::Get(), TEXT("LogAssetLoads"));
	return bLogAssetLoads;
}

void UMGAssetManager::AddLoadedAsset(const UObject* Asset)
{
	if (ensureAlways(Asset))
	{
		FScopeLock LoadedAssetsLock(&LoadedAssetsCritical);
		LoadedAssets.Add(Asset);
	}
}
