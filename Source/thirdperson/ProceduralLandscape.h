// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ProceduralLandscape.generated.h"

UCLASS()
class THIRDPERSON_API AProceduralLandscape : public AActor
{
  GENERATED_BODY()

public:
  /** Chunks with centers within this radius of the first local player will be loaded automatically. */
  UPROPERTY(EditAnywhere, meta=(ClampMin="1000.0", ClampMax="10000000.0"))
  float LoadRadius = 1000.f;
  
  /** Chunks with centers outside this radius of the first local player will be unloaded automatically. */
  UPROPERTY(EditAnywhere, meta=(ClampMin="1000.0", ClampMax="10000000.0"))
  float UnloadRadius = 1333.f;

  /** Chunk grid resolution. */
  UPROPERTY(EditAnywhere, meta=(ClampMin="1", ClampMax="255"))
  int32 StepsPerChunk = 1;

  /** Chunk size along one side. */
  UPROPERTY(EditAnywhere, meta=(ClampMin="1.0", ClampMax="100000.0"))
  float ChunkSize = 1000.f;

  /** Procedural noise sampling function input x,y are divided by this. */
  UPROPERTY(EditAnywhere, meta=(ClampMin="100.0", ClampMax="1000000.0"))
  float HorizontalNoiseScale = 1000.f;

  /** Landscape Z-values will vary between 0.0 and this scale value. */
  UPROPERTY(EditAnywhere, meta=(ClampMin="1.0", ClampMax="10000.0"))
  float VerticalScale = 10.f;

  /** Applied to every chunk. UV scale is 1.0 per 100.0 world units. */
  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  UMaterialInterface* LandscapeMaterial;

  //==============================================================================
  // Runtime Virtual Texture support
  
  // SEE: PrimitiveComponent.h for more details about virtual texture rendering
  // D:\Epic Games\UE_5.0EA\Engine\Source\Runtime\Engine\Classes\Components\PrimitiveComponent.h

	/** 
	 * Array of runtime virtual textures into which we draw the mesh for this actor. 
	 * The material also needs to be set up to output to a virtual texture. 
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = VirtualTexture, meta = (DisplayName = "Draw in Virtual Textures"))
	TArray<TObjectPtr<URuntimeVirtualTexture>> RuntimeVirtualTextures;

	/** Bias to the LOD selected for rendering to runtime virtual textures. */
	UPROPERTY(EditAnywhere, AdvancedDisplay, Category = VirtualTexture, meta = (DisplayName = "Virtual Texture LOD Bias", UIMin = "-7", UIMax = "8"))
	int8 VirtualTextureLodBias = 0;

	/**
	 * Number of lower mips in the runtime virtual texture to skip for rendering this primitive.
	 * Larger values reduce the effective draw distance in the runtime virtual texture.
	 * This culling method doesn't take into account primitive size or virtual texture size.
	 */
	UPROPERTY(EditAnywhere, AdvancedDisplay, Category = VirtualTexture, meta = (DisplayName = "Virtual Texture Skip Mips", UIMin = "0", UIMax = "7"))
	int8 VirtualTextureCullMips = 0;

	/**
	 * Set the minimum pixel coverage before culling from the runtime virtual texture.
	 * Larger values reduce the effective draw distance in the runtime virtual texture.
	 */
	UPROPERTY(EditAnywhere, AdvancedDisplay, Category = VirtualTexture, meta = (UIMin = "0", UIMax = "7"))
	int8 VirtualTextureMinCoverage = 0;

	/** Controls if this component draws in the main pass as well as in the virtual texture. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = VirtualTexture, meta = (DisplayName = "Draw in Main Pass"))
	ERuntimeVirtualTextureMainPassType VirtualTextureRenderPassType = ERuntimeVirtualTextureMainPassType::Exclusive;

  //==============================================================================

  AProceduralLandscape();
  ~AProceduralLandscape() override;

  // bool ShouldTickIfViewportsOnly() const override { return true; }
  void Tick(float DeltaTime) override;

protected:
  void BeginPlay() override;

private:
  struct Private;
  Private* p{};
};
