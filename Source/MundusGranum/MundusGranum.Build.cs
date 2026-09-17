// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class MundusGranum : ModuleRules
{
	public MundusGranum(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { 
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore", 
			"EnhancedInput",
			"GameplayTasks",
			"GameplayAbilities",
			"ModularGameplay",
			"ProceduralMeshComponent",
			"GeometryCollectionEngine",
			"UMG",
			"NetCore",
			"ModularGameplayActors",
			"GameFeatures"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { "GameplayTags", "GameplayTasks", "NavigationSystem", "Niagara", "ModelViewViewModel" });

		PrivateIncludePaths.AddRange(new string[] { "MundusGranum" });

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
