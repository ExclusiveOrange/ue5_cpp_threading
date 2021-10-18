// Fill out your copyright notice in the Description page of Project Settings.


#include "Chunk.h"

AChunk::AChunk()
{
  StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("AChunk static mesh"), true);
  
  StaticMeshComponent->bRenderInMainPass = false; // not needed so long as a virtual heightfield mesh is overlain
  StaticMeshComponent->bRenderInDepthPass = true; // needed for proper shadows evidently

  // TESTING
  // StaticMeshComponent->bUseAsOccluder = false;
  
  RootComponent = StaticMeshComponent;
  PrimaryActorTick.bCanEverTick = false;

}

void AChunk::OnConstruction(const FTransform& transform)
{
  Super::OnConstruction(transform);
  
  StaticMeshComponent->SetMaterial(0, Material);

  // set virtual textures (output)
  StaticMeshComponent->RuntimeVirtualTextures = RuntimeVirtualTextures;
  StaticMeshComponent->VirtualTextureLodBias = VirtualTextureLodBias;
  StaticMeshComponent->VirtualTextureCullMips = VirtualTextureCullMips;
  StaticMeshComponent->VirtualTextureMinCoverage = VirtualTextureMinCoverage;
  StaticMeshComponent->VirtualTextureRenderPassType = VirtualTextureRenderPassType;

  StaticMeshComponent->SetMobility(EComponentMobility::Static);
  // StaticMeshComponent->bUseDefaultCollision = true;
  // StaticMeshComponent->bDrawMeshCollisionIfComplex = true; // actually draws mesh collision wireframe at least in PIE
  // StaticMeshComponent->bTraceComplexOnMove = true; // doesn't seem to affect pawn
}
