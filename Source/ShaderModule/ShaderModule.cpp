#include "ShaderModule.h"
#include "Modules/ModuleManager.h"

void FShaderModule::StartupModule()
{
#if (ENGINE_MINOR_VERSION >= 21)    
	const FString ShaderDirectory = FPaths::Combine(FPaths::ProjectDir(), TEXT("/Shader"));
	if(!AllShaderSourceDirectoryMappings().Contains("/Project"))
		AddShaderSourceDirectoryMapping("/Project", ShaderDirectory);
#endif
}

void FShaderModule::ShutdownModule()
{
}

IMPLEMENT_GAME_MODULE( FShaderModule, ShaderModule );