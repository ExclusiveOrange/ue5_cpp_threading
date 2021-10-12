// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ProceduralLandscape.generated.h"

UCLASS()
class THIRDPERSON_API AProceduralLandscape : public AActor
{
	GENERATED_BODY()
	
public:
  
  UPROPERTY(EditAnywhere, meta=(ClampMin="1000.0", ClampMax="10000000.0"))
  float LoadRadius = 1000.f;
  
  UPROPERTY(EditAnywhere, meta=(ClampMin="1", ClampMax="255"))
  int32 StepsPerChunk = 1;

  UPROPERTY(EditAnywhere, meta=(ClampMin="1.0", ClampMax="100000.0"))
  float UnitsPerChunk = 100.f;
  
  UPROPERTY(EditAnywhere, meta=(ClampMin="1000.0", ClampMax="10000000.0"))
  float UnloadRadius = 1333.f;
  
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
  Private *p;
};
