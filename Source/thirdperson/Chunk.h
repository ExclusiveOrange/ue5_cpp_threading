// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProceduralMeshComponent.h"
#include "Chunk.generated.h"

UCLASS()
class THIRDPERSON_API AChunk : public AActor
{
	GENERATED_BODY()
	
public:	
	AChunk();

  UProceduralMeshComponent *mesh;
};
