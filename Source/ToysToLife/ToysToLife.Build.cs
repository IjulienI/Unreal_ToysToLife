// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class ToysToLife : ModuleRules
{
	public ToysToLife(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicIncludePaths.AddRange(new string[]
		{
			
		});
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
			}
			);
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Slate",
				"SlateCore",
			}
			);
		
		if (Target.Platform == UnrealTargetPlatform.Win64)
		{
			PublicSystemLibraries.Add("winscard.lib");
			PublicDefinitions.Add("NFC_PCSC_SUPPORTED=1");
		}
		else
		{
			PublicDefinitions.Add("NFC_PCSC_SUPPORTED=0");
		}
	}
}
