// Fill out your copyright notice in the Description page of Project Settings.


#include "ProceduralLandscape.h"
#include "Chunk.h"

#include <cmath>
#include <chrono>

namespace
{
  using clock_t = std::chrono::high_resolution_clock;
  
  void
  enumerateChunksInRadius(
    TArray<FIntVector>& chunksInRadius,
    const FVector2D center,
    const float radius,
    const float chunkSize)
  {
    const auto startTime = clock_t::now();

    // clear and reserve space in chunksInRadius
    {
      const int32 chunksPerSide = int32(radius * 2.f / chunkSize);
      chunksInRadius.Empty(chunksPerSide * chunksPerSide);
    }
    
    auto emplaceIfInRadius = [=,&chunksInRadius](const int32 x, const int32 y)
    {
      return
        (FVector2D{x*chunkSize,y*chunkSize}-center).SizeSquared() <= radius * radius
          ? (chunksInRadius.Emplace(x, y, 0), true)
          : false;
    };

    // coordinates of center of chunk nearest 'center' in whole terms of chunkSize from world origin
    const int32 xCenter = int32(std::floor((center.X + 0.5f * chunkSize) / chunkSize));
    const int32 yCenter = int32(std::floor((center.Y + 0.5f * chunkSize) / chunkSize));

    // check center chunk
    if (!emplaceIfInRadius(xCenter, yCenter))
      return;

    // check chunks in an expanding square around the center chunk
    for (int32 d = 1;; ++d)
    {
      const int32 numChunksAtStart = chunksInRadius.Num();

      // Demonstration of possible registration order with a small radius:
      //  
      //             26 25 27
      //          12 10  9 11 20
      //       36 23  2  1  6 18 32
      //       34 21  7  0  5 17 31
      // +x    35 22  8  3  4 19 33
      //  |       24 15 13 14 16 
      //  |          30 28 29
      //  |
      //  +-------- +y

      for(int32 o = 0; o < d;)
      {
        int any = emplaceIfInRadius(xCenter + d, yCenter + o); // +x
        any += emplaceIfInRadius(xCenter - d, yCenter - o);    // -x
        any += emplaceIfInRadius(xCenter - o, yCenter + d);    // +y
        any += emplaceIfInRadius(xCenter + o, yCenter - d);    // +z

        ++o;

        any += emplaceIfInRadius(xCenter + d, yCenter - o);    // +x
        any += emplaceIfInRadius(xCenter - d, yCenter + o);    // -x
        any += emplaceIfInRadius(xCenter + o, yCenter + d);    // +y
        any += emplaceIfInRadius(xCenter - o, yCenter - d);    // +z

        if(!any)
          break;
      }

      // check if any chunks were found in radius; if not then stop looking
      if (chunksInRadius.Num() == numChunksAtStart)
        break;
    }

    const auto endTime = clock_t::now();
    const auto nanos = std::chrono::duration_cast<std::chrono::nanoseconds>(endTime-startTime).count();

    UE_LOG(LogTemp, Warning, TEXT("ProceduralLandscape: numChunks(%d), xCenter(%d), yCenter(%d), ns(%lld)"), chunksInRadius.Num(), xCenter, yCenter, nanos);
  }
} // namespace

//==============================================================================

struct AProceduralLandscape::Private
{
  TArray<FIntVector> chunksInRadius;
};

//==============================================================================

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

  const FVector playerLocation = GetWorld()->GetFirstPlayerController()->GetPawn()->GetActorLocation();
  // UE_LOG(LogTemp, Warning, TEXT("PlayerLocation: (%f, %f, %f)"), playerLocation.X, playerLocation.Y, playerLocation.Z);
  enumerateChunksInRadius(p->chunksInRadius, {playerLocation.X, playerLocation.Y}, LoadRadius, ChunkSize);

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


