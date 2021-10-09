// Fill out your copyright notice in the Description page of Project Settings.


#include "Chunk.h"

AChunk::AChunk()
{
  mesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("AChunk mesh"));
  RootComponent = mesh;
	PrimaryActorTick.bCanEverTick = false;
}

