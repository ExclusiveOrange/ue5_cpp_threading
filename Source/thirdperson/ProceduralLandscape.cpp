// Fill out your copyright notice in the Description page of Project Settings.


#include "ProceduralLandscape.h"
#include "Chunk.h"

namespace
{
  struct DeadActorCache
  {
    
  };
} // namespace

struct AProceduralLandscape::Private
{
  
};

// Sets default values
AProceduralLandscape::AProceduralLandscape()
  : p{ new Private }
{
  // not sure if I need all of these but it seems Unreal changes how ticks work from version to version and this combo works
  PrimaryActorTick.bCanEverTick = true;
  PrimaryActorTick.bRunOnAnyThread = false; // only run on main thread because we need to create world objects
  PrimaryActorTick.SetTickFunctionEnable(true); // this is necessary to get ticks to work without a Blueprint as of 2021.10.08
}

AProceduralLandscape::~AProceduralLandscape()
{
  delete p;
}

// Called every frame
void AProceduralLandscape::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

  // TODO
  //   get local player world location
  //   enque chunks in LoadRadius that aren't already loaded
  //   unload chunks beyond UnloadRadius
}

// Called when the game starts or when spawned
void AProceduralLandscape::BeginPlay()
{
	Super::BeginPlay();
	
}


