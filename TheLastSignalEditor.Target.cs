using UnrealBuildTool;
using System.Collections.Generic;

public class TheLastSignalEditorTarget : TargetRules
{
	public TheLastSignalEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.Add("TheLastSignal");
	}
}
