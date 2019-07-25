using UnrealBuildTool;
using System.Collections.Generic;

public class DroidSenseDemoEditorTarget : TargetRules
{
	public DroidSenseDemoEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;

		//bUseUnityBuild = false;
		//bUsePCHFiles = false;

		ExtraModuleNames.AddRange( new string[] { "DroidSenseDemo" } );
	}
}
