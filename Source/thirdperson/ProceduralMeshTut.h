// Fill out your copyright notice in the Description page of Project Settings.
//
// Written (probably close to a copy) by following the tutorial at:
// https://nerivec.github.io/old-ue4-wiki/pages/procedural-mesh-component-in-cgetting-started.html

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProceduralMeshComponent.h"

#include "ProceduralMeshTut.generated.h"

UCLASS()
class THIRDPERSON_API AProceduralMeshTut : public AActor
{
  GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly);
	UMaterialInterface *material;
  
  AProceduralMeshTut();

  virtual void Tick(float DeltaTime) override;

protected:
  virtual void BeginPlay() override;

private:
  UPROPERTY(VisibleAnywhere)
  UProceduralMeshComponent* mesh;

  void CreateTriangle() const;
};
