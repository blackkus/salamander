using UnrealBuildTool;

public class SalamanderEditorTarget : TargetRules
{
	public SalamanderEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.Add("Salamander");
	}
}
