// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SketchComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class SHADERMODULE_API USketchComponent final : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	USketchComponent();

	UPROPERTY(EditAnywhere, AdvancedDisplay, Category=Sketch)
	class UTextureRenderTarget2D* RenderTexture;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

private:
	void ExecuteInRenderThread(FRHICommandListImmediate& RHICmdList, FTextureRenderTargetResource* OutputRenderTargetResource);
	void DrawIndexedPrimitiveUP(
		FRHICommandList& RHICmdList,
		uint32 PrimitiveType,
		uint32 MinVertexIndex,
		uint32 NumVertices,
		uint32 NumPrimitives,
		const void* IndexData,
		uint32 IndexDataStride,
		const void* VertexData,
		uint32 VertexDataStride
	) const;
};
