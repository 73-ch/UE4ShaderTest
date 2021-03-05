// Fill out your copyright notice in the Description page of Project Settings.


#include "SketchComponent.h"


#include "SketchShader.h"
#include "Engine/TextureRenderTarget2D.h"

#include "RHIStaticStates.h"
#include "PipelineStateCache.h"
#include "GlobalShader.h"

#include "SceneUtils.h"
#include "ShaderParameterStruct.h"

// #include "LensDistortionAPI.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/World.h"
#include "GlobalShader.h"
#include "PipelineStateCache.h"
#include "RHIStaticStates.h"
#include "SceneUtils.h"
#include "SceneInterface.h"
#include "ShaderParameterUtils.h"

// #define GETSAFERHISHADER_PIXEL(Shader) ((Shader) ? (Shader)->GetPixelShader() : nullptr)
// #define GETSAFERHISHADER_VERTEX(Shader) ((Shader) ? (Shader)->GetVertexShader() : nullptr)

// Sets default values for this component's properties
USketchComponent::USketchComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void USketchComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
}


// Called every frame
void USketchComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                     FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	auto This = this;
	auto RenderTargetResource = this->RenderTexture->GameThread_GetRenderTargetResource();
	ENQUEUE_RENDER_COMMAND(FRaymarchingPostProcess)(
		[This, RenderTargetResource](FRHICommandListImmediate& RHICmdList)
		{
			This->ExecuteInRenderThread(RHICmdList, RenderTargetResource);
		}
	);
}

void USketchComponent::ExecuteInRenderThread(FRHICommandListImmediate& RHICmdList,
                                             FTextureRenderTargetResource* OutputRenderTargetResource)
{
	check(IsInRenderingThread());

#if WANTS_DRAW_MESH_EVENTS
	FString EventName;
	this->RenderTexture->GetFName().ToString(EventName);
	SCOPED_DRAW_EVENTF(RHICmdList, SceneCapture, TEXT("SketchShader %s"), *EventName);
#else
	SCOPED_DRAW_EVENT(RHICmdList, DrawUVDisplacementToRenderTarget_RenderThread);
#endif

	// SetRenderTarget
	FRHIRenderPassInfo rpInfo(OutputRenderTargetResource->GetRenderTargetTexture(),
	                          ERenderTargetActions::DontLoad_DontStore);
	RHICmdList.BeginRenderPass(rpInfo, TEXT("Sketch"));

	// Shader setup
	const auto ShaderMap = GetGlobalShaderMap(ERHIFeatureLevel::SM5);
	
	TShaderMapRef<FSketchShaderVS> ShaderVS(ShaderMap);
	TShaderMapRef<FSketchShaderPS> ShaderPS(ShaderMap);

	FSketchVertexDeclaration VertexDec;
	VertexDec.InitRHI();

	// Declare a pipeline state object that holds all the rendering state
	FGraphicsPipelineStateInitializer PSOInitializer;
	RHICmdList.ApplyCachedRenderTargets(PSOInitializer);

	// RHICreateBoundShaderState(VertexDec.VertexDeclarationRHI, ShaderVS, nullptr, nullptr, ShaderPS, nullptr);
	
	PSOInitializer.PrimitiveType = PT_TriangleList;

	FRHIVertexShader;
	FBoundShaderStateInput;

	
	PSOInitializer.BoundShaderState.VertexDeclarationRHI = VertexDec.VertexDeclarationRHI;
	PSOInitializer.BoundShaderState.VertexShaderRHI = ShaderVS.GetVertexShader();
	PSOInitializer.BoundShaderState.PixelShaderRHI = ShaderPS.GetPixelShader();
	PSOInitializer.RasterizerState = TStaticRasterizerState<FM_Solid, CM_None>::GetRHI();
	PSOInitializer.BlendState = TStaticBlendState<>::GetRHI();
	PSOInitializer.DepthStencilState = TStaticDepthStencilState<false, CF_Always>::GetRHI();
	SetGraphicsPipelineState(RHICmdList, PSOInitializer);

	static const FSketchVertex Vertices[4] = {
		{FVector4(-1.0f, 1.0f, 0.0f, 1.0f), FVector2D(0.0f, 0.0f)},
		{FVector4(1.0f, 1.0f, 0.0f, 1.0f), FVector2D(1.0f, 0.0f)},
		{FVector4(-1.0f, -1.0f, 0.0f, 1.0f), FVector2D(0.0f, 1.0f)},
		{FVector4(1.0f, -1.0f, 0.0f, 1.0f), FVector2D(1.0f, 1.0f)},
	};

	static const uint16 Indices[6] = {
		0, 1, 2,
		2, 1, 3
	};

	DrawIndexedPrimitiveUP(RHICmdList, PT_TriangleList, 0, ARRAY_COUNT(Vertices), 2, Indices, sizeof(Indices[0]),
	                       Vertices, sizeof(Vertices[0]));

	// Resolve render target.
	RHICmdList.CopyToResolveTarget(
		OutputRenderTargetResource->GetRenderTargetTexture(),
		OutputRenderTargetResource->TextureRHI,
		FResolveParams()
	);

	RHICmdList.EndRenderPass();
}

void USketchComponent::DrawIndexedPrimitiveUP(FRHICommandList& RHICmdList, uint32 PrimitiveType, uint32 MinVertexIndex,
                                              uint32 NumVertices, uint32 NumPrimitives, const void* IndexData,
                                              uint32 IndexDataStride, const void* VertexData,
                                              uint32 VertexDataStride) const
{
	const uint32 NumIndices = GetVertexCountForPrimitiveCount(NumPrimitives, PrimitiveType);

	FRHIResourceCreateInfo CreateInfo;
	FVertexBufferRHIRef VertexBufferRHI = RHICreateVertexBuffer(VertexDataStride * NumVertices, BUF_Volatile, CreateInfo);
	void* VoidPtr = RHILockVertexBuffer(VertexBufferRHI, 0, VertexDataStride * NumVertices, RLM_WriteOnly);
	FPlatformMemory::Memcpy(VoidPtr, VertexData, VertexDataStride * NumVertices);
	RHIUnlockVertexBuffer(VertexBufferRHI);

	FIndexBufferRHIRef IndexBufferRHI =RHICreateIndexBuffer(IndexDataStride, IndexDataStride * NumIndices, BUF_Volatile, CreateInfo);
	void* VoidPtr2 = RHILockIndexBuffer(IndexBufferRHI, 0, IndexDataStride * NumIndices, RLM_WriteOnly);
	FPlatformMemory::Memcpy(VoidPtr2, IndexData, IndexDataStride * NumIndices);
	RHIUnlockIndexBuffer(IndexBufferRHI);

	RHICmdList.SetStreamSource(0, VertexBufferRHI,0);
	RHICmdList.DrawIndexedPrimitive(IndexBufferRHI, MinVertexIndex, 0, NumVertices, 0, NumPrimitives, 1);

	IndexBufferRHI.SafeRelease();
	VertexBufferRHI.SafeRelease();
}
