using UnrealBuildTool;

public class TheLastSignal : ModuleRules
{
	public TheLastSignal(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"UMG",
			"Slate",
			"SlateCore",
			"AIModule",
			"NavigationSystem",
			"GameplayTasks",
			"Niagara",
			"AudioMixer"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });
	}
}
