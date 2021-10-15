// Fill out your copyright notice in the Description page of Project Settings.


#include "Chunk.h"

AChunk::AChunk()
{
  mesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("AChunk mesh"));
  RootComponent = mesh;
	PrimaryActorTick.bCanEverTick = false;
}

void AChunk::OnConstruction(const FTransform& transform)
{
  Super::OnConstruction(transform);
  
  mesh->SetMaterial(0, Material);
  mesh->RuntimeVirtualTextures = RuntimeVirtualTextures;
  mesh->VirtualTextureLodBias = VirtualTextureLodBias;
  mesh->VirtualTextureCullMips = VirtualTextureCullMips;
  mesh->VirtualTextureMinCoverage = VirtualTextureMinCoverage;
  mesh->VirtualTextureRenderPassType = VirtualTextureRenderPassType;
}
