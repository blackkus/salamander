using UnrealBuildTool;

public class Salamander : ModuleRules
{
	public Salamander(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"NetCore",
			"GameplayTasks",
			"GameplayAbilities",
			"GameplayTags",
			"UMG",
			"Slate",
			"SlateCore"
		});
	}
}
