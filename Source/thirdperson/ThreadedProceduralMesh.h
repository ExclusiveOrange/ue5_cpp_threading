// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ThreadedProceduralMesh.generated.h"

UCLASS()
class THIRDPERSON_API AThreadedProceduralMesh : public AActor
{
  GENERATED_BODY()

public:
  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  UMaterialInterface* Material;

  UPROPERTY(EditAnywhere, meta = (ClampMin = "1", ClampMax = "255"))
  int32 xSteps = 1;

  UPROPERTY(EditAnywhere, meta = (ClampMin = "1", ClampMax = "255"))
  int32 ySteps = 1;

  UPROPERTY(EditAnywhere, meta = (ClampMin = "1.0", ClampMax = "10000.0"))
  float xTotalUnits = 100.f;

  UPROPERTY(EditAnywhere, meta = (ClampMin = "1.0", ClampMax = "10000.0"))
  float yTotalUnits = 100.f;

  AThreadedProceduralMesh();
  ~AThreadedProceduralMesh() override;

  void Tick(float DeltaTime) override;

protected:
  // Called when the game starts or when spawned
  void BeginPlay() override;

private:
  struct Private;
  Private* p;
};
