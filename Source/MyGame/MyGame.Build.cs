// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class MyGame : ModuleRules
{
	public MyGame(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput", "UMG", "Niagara", "AnimGraphRuntime" });

        PublicIncludePaths.AddRange(
            new string[] {
                "MyGame" // 여러분 프로젝트 이름
            }
        );

        PrivateIncludePaths.AddRange(
            new string[] {

            }
        );
    }
}
