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

  /** Chunk grid resolution. */
  UPROPERTY(EditAnywhere, meta=(ClampMin="1", ClampMax="255"))
  int32 StepsPerChunk = 1;

  /** Chunk size along one side. */
  UPROPERTY(EditAnywhere, meta=(ClampMin="1.0", ClampMax="100000.0"))
  float ChunkSize = 1000.f;

  /** Chunks with centers outside this radius of the first local player will be unloaded automatically. */
  UPROPERTY(EditAnywhere, meta=(ClampMin="1000.0", ClampMax="10000000.0"))
  float UnloadRadius = 1333.f;

  /** Applied to every chunk. */
  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  UMaterialInterface* LandscapeMaterial;

  //------------------------------------------------------------------------------

  AProceduralLandscape();
  ~AProceduralLandscape() override;

  void Tick(float DeltaTime) override;

protected:
  void BeginPlay() override;

private:
  struct Private;
  Private* p;
};
