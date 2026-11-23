using UnrealBuildTool;
using System.Collections.Generic;

public class TheLastSignalTarget : TargetRules
{
	public TheLastSignalTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.Add("TheLastSignal");
	}
}
