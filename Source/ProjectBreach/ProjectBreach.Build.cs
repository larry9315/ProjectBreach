// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class ProjectBreach : ModuleRules
{
	public ProjectBreach(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"AIModule",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"UMG",
			"Slate"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"ProjectBreach",
			"ProjectBreach/Variant_Platforming",
			"ProjectBreach/Variant_Platforming/Animation",
			"ProjectBreach/Variant_Combat",
			"ProjectBreach/Variant_Combat/AI",
			"ProjectBreach/Variant_Combat/Animation",
			"ProjectBreach/Variant_Combat/Gameplay",
			"ProjectBreach/Variant_Combat/Interfaces",
			"ProjectBreach/Variant_Combat/UI",
			"ProjectBreach/Variant_SideScrolling",
			"ProjectBreach/Variant_SideScrolling/AI",
			"ProjectBreach/Variant_SideScrolling/Gameplay",
			"ProjectBreach/Variant_SideScrolling/Interfaces",
			"ProjectBreach/Variant_SideScrolling/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
