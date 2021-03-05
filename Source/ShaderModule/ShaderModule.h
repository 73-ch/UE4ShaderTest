#pragma once

#include "CoreMinimal.h"

class FShaderModule final: public IModuleInterface
{
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
    
    // static inline FShaderModule& Get() 
    // { 
    //     return FModuleManager::LoadModuleChecked< FShaderModule >( "FShaderModule" ); 
    // } 

    // static inline bool IsAvailable() 
    // { 
    //     return FModuleManager::Get().IsModuleLoaded( "FShaderModule" ); 
    // } 
};
