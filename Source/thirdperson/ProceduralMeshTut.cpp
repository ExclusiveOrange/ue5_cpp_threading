// Fill out your copyright notice in the Description page of Project Settings.


#include "ProceduralMeshTut.h"

////////////////////////////////////////////////////////////////////////////////

// Sets default values
AProceduralMeshTut::AProceduralMeshTut()
{
  UE_LOG(LogTemp, Warning, TEXT(__FUNCTION__));
  
  // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
  //PrimaryActorTick.bCanEverTick = true;

  mesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("GeneratedMesh"));
  RootComponent = mesh;
  mesh->bUseAsyncCooking = true; // multi-threaded physx cooking
  mesh->SetMaterial(0, material);
  CreateTriangle();
}

// Called every frame
void AProceduralMeshTut::Tick(float DeltaTime)
{
  Super::Tick(DeltaTime);
  UE_LOG(LogTemp, Warning, TEXT("AProceduralMeshTut::Tick()"));
}

////////////////////////////////////////////////////////////////////////////////

// Called when the game starts or when spawned
void AProceduralMeshTut::BeginPlay()
{
  UE_LOG(LogTemp, Warning, TEXT(__FUNCTION__));
  Super::BeginPlay();
}

////////////////////////////////////////////////////////////////////////////////

void AProceduralMeshTut::CreateTriangle() const
{
  const TArray<FVector> vertices{
    {0, 0, 0},
    {0, 100, 0},
    {0, 0, 100}};

  const TArray<int32> triangles{ 0, 1, 2 };

  const TArray<FVector> normals{
    {-1, 0, 0},
    {-1, 0, 0},
    {-1, 0, 0} };

  const TArray<FVector2D> uv0{
    {0, 0},
    {10, 0},
    {0, 10} };

  const TArray<FLinearColor> colors{
    {0.75, 0.75, 0.75, 1.0},
    {0.75, 0.75, 0.75, 1.0},
    {0.75, 0.75, 0.75, 1.0}};

  const TArray<FProcMeshTangent> tangents{
    {0, 1, 0},
    {0, 1, 0},
    {0, 1, 0} };

  mesh->CreateMeshSection_LinearColor( 0, vertices, triangles, normals, uv0, colors, tangents, true );
}
