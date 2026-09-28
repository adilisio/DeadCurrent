using UnrealBuildTool;
using System.Collections.Generic;

public class DeadCurrentEditorTarget : TargetRules
{
	public DeadCurrentEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V7;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_8;
		ExtraModuleNames.Add("DeadCurrent");
	}
}
