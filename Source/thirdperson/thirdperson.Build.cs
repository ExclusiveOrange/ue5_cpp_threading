// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class thirdperson : ModuleRules
{
	public thirdperson(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		CppStandard = CppStandardVersion.Cpp17; // works
		// CppStandard = CppStandardVersion.Cpp20;
		//CppStandard = CppStandardVersion.Latest; // doesn't work: get many compile errors in engine source files
		PublicDependencyModuleNames.AddRange(
			new string[]
				{"Core", "CoreUObject", "Engine", "InputCore", "HeadMountedDisplay", "ProceduralMeshComponent"});
	}
}