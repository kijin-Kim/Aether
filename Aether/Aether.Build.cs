// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Aether : ModuleRules
{
	public Aether(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput", "GameplayAbilities", "GameplayTags", "GameplayTasks", "UMG", "CommonUI", "CommonInput" });
		PrivateDependencyModuleNames.AddRange(new string[] { "AIModule", "GameplayStateTreeModule", "StateTreeModule" });

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
	}
}
