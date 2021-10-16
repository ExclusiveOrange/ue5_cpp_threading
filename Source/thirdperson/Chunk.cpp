// Fill out your copyright notice in the Description page of Project Settings.


#include "Chunk.h"

AChunk::AChunk()
{
  StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("AChunk static mesh"), true);
  RootComponent = StaticMeshComponent;
	PrimaryActorTick.bCanEverTick = false;
}

void AChunk::OnConstruction(const FTransform& transform)
{
  Super::OnConstruction(transform);
  
  StaticMeshComponent->SetMaterial(0, Material);
  StaticMeshComponent->RuntimeVirtualTextures = RuntimeVirtualTextures;
  StaticMeshComponent->VirtualTextureLodBias = VirtualTextureLodBias;
  StaticMeshComponent->VirtualTextureCullMips = VirtualTextureCullMips;
  StaticMeshComponent->VirtualTextureMinCoverage = VirtualTextureMinCoverage;
  StaticMeshComponent->VirtualTextureRenderPassType = VirtualTextureRenderPassType;

  // StaticMeshComponent->bUseDefaultCollision = true;
  StaticMeshComponent->collision
  StaticMeshComponent->UpdateCollisionFromStaticMesh();
  StaticMeshComponent->SetMobility(EComponentMobility::Static);
}
