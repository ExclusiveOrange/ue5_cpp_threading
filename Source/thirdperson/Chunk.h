// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProceduralMeshComponent.h"
#include "Chunk.generated.h"

UCLASS(Transient)
class THIRDPERSON_API AChunk : public AActor
{
	GENERATED_BODY()
	
public:	
	AChunk();

  void OnConstruction(const FTransform& Transform) override;

  // UPROPERTY()
  // UProceduralMeshComponent *mesh;
  
  UPROPERTY(EditAnywhere)
  UStaticMeshComponent *mesh;
  
  UPROPERTY()
  UMaterialInterface* Material;
  
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
};
