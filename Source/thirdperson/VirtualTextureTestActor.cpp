// Fill out your copyright notice in the Description page of Project Settings.


#include "VirtualTextureTestActor.h"

// Sets default values
AVirtualTextureTestActor::AVirtualTextureTestActor(const FObjectInitializer &ObjectInitializer)
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

  StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Cube"));
  UStaticMesh *cubeMesh = ConstructorHelpers::FObjectFinder<UStaticMesh>(TEXT("StaticMesh'/Engine/BasicShapes/Cube.Cube'")).Object;
  if(!cubeMesh)
  {
    UE_LOG(LogTemp, Warning, TEXT("ERROR: couldn't find Cube.Cube"));
    return;
  }

  // cubeMesh->SetMaterial(0, Material);

  StaticMeshComponent->SetStaticMesh(cubeMesh);
  
  RootComponent = StaticMeshComponent;
}

// Called when the game starts or when spawned
void AVirtualTextureTestActor::BeginPlay()
{
	Super::BeginPlay();
}

void AVirtualTextureTestActor::OnConstruction(const FTransform& transform)
{
  Super::OnConstruction(transform);
	
  StaticMeshComponent->SetMaterial(0, Material);
  StaticMeshComponent->RuntimeVirtualTextures = RuntimeVirtualTextures;
  StaticMeshComponent->VirtualTextureLodBias = VirtualTextureLodBias;
  StaticMeshComponent->VirtualTextureCullMips = VirtualTextureCullMips;
  StaticMeshComponent->VirtualTextureMinCoverage = VirtualTextureMinCoverage;
  StaticMeshComponent->VirtualTextureRenderPassType = VirtualTextureRenderPassType;
}

// Called every frame
void AVirtualTextureTestActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

