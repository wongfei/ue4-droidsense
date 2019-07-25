using UnrealBuildTool;
using System.Collections.Generic;

public class DroidSenseDemoTarget : TargetRules
{
	public DroidSenseDemoTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;

		//bUseUnityBuild = false;
		//bUsePCHFiles = false;

		ExtraModuleNames.AddRange( new string[] { "DroidSenseDemo" } );
	}
}
