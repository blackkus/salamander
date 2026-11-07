using UnrealBuildTool;

public class SalamanderTarget : TargetRules
{
	public SalamanderTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.Add("Salamander");
	}
}
