using UnrealBuildTool;

public class DeadCurrent : ModuleRules
{
	public DeadCurrent(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"GameplayTags",
			"UMG",
			"Slate",
			"AIModule",
			"NavigationSystem",
			"GameplayTasks"
		});

		PrivateDependencyModuleNames.AddRange(new string[] {
			"AssetRegistry",
			"Json",
			"ImageWrapper",
			"RHI"   // the review capture reads the RHI's per-frame draw-call counts (DCReviewCaptureTest.cpp)
		});

		// Module root is public so systems include each other as "Folder/File.h".
		PublicIncludePaths.Add(ModuleDirectory);
	}
}
