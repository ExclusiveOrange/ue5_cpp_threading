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

    chunksInRadius.Reset();
    
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

    // start with center chunk
    if (emplaceIfInRadius(xCenter, yCenter))
      // check chunks in an expanding square around the center chunk
      for (int32 r = 1;; ++r) // radial distance from center
      {
        const int32 numChunksAtStart = chunksInRadius.Num();

        for(int32 o = 0; o < r;) // orthogonal distance from axial radial
        {
          
          int any = emplaceIfInRadius(xCenter + r, yCenter + o); // +x +o
          any += emplaceIfInRadius(xCenter - r, yCenter - o);    // -x -o
          any += emplaceIfInRadius(xCenter - o, yCenter + r);    // +y -o
          any += emplaceIfInRadius(xCenter + o, yCenter - r);    // +z +o

          ++o;

          any += emplaceIfInRadius(xCenter + r, yCenter - o);    // +x -o
          any += emplaceIfInRadius(xCenter - r, yCenter + o);    // -x +o
          any += emplaceIfInRadius(xCenter + o, yCenter + r);    // +y +o
          any += emplaceIfInRadius(xCenter - o, yCenter - r);    // +z -o

          if(!any)
            break;
        }

        // check if any chunks were found in radius; if not then stop looking
        if (chunksInRadius.Num() == numChunksAtStart)
          break;
      }

    const auto endTime = clock_t::now();
    const auto nanos = std::chrono::duration_cast<std::chrono::nanoseconds>(endTime-startTime).count();

    //UE_LOG(LogTemp, Warning, TEXT("enumerateChunksInRadius: (%d), xCenter(%d), yCenter(%d), ns(%lld)"), chunksInRadius.Num(), xCenter, yCenter, nanos);
  }

  void
  moveChunksOutsideRadiusToArray(
    TMap<FIntVector, AChunk*> &chunksLoaded, // will be removed from this map
    TArray<AChunk*> &chunksOutside,          // will be put in this array
    const FVector2D center,
    const float radius,
    const float chunkSize)
  {
    const auto startTime = clock_t::now();

    chunksOutside.Reset();

    auto chunkIsOutside = [=](const FIntVector chunk)
    {
      return (FVector2D{chunk.X*chunkSize, chunk.Y*chunkSize}-center).SizeSquared() > radius*radius;
    };

    for( auto it = chunksLoaded.CreateIterator(); it; ++it )
      if( chunkIsOutside(it.Key()))
      {
        chunksOutside.Push(it.Value());
        it.RemoveCurrent();
      }
    
    const auto endTime = clock_t::now();
    const auto nanos = std::chrono::duration_cast<std::chrono::nanoseconds>(endTime-startTime).count();

    if( const auto numUnloaded = chunksOutside.Num())
      UE_LOG(LogTemp, Warning, TEXT("unloadChunksBeyondRadius: numChunksUnloaded(%d), numChunksStillLoaded(%d), ns(%lld)"), numUnloaded, chunksLoaded.Num(), nanos);
  }
} // namespace

//==============================================================================

struct AProceduralLandscape::Private
{
  TArray<FIntVector> chunksInRadius_array; // order matters
  TArray<FIntVector> chunksToLoad_array;   // order matters
  TSet<FIntVector> chunksToLoad_set;       // presence matters
  TMap<FIntVector, AChunk*> chunksLoaded;  // presence matters
  TArray<AChunk*> chunksToUnload_array;
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

  // TODO: get just-created chunks from workers
  // TODO: discard any beyond unload radius, before creating actors
  // TODO: don't create actors until after the chunksToLoad has been sorted, at the bottom of this function

  const FVector2D playerLocation2D{ GetWorld()->GetFirstPlayerController()->GetPawn()->GetActorLocation() };
  
  moveChunksOutsideRadiusToArray(p->chunksLoaded, p->chunksToUnload_array, playerLocation2D, UnloadRadius, ChunkSize);

  enumerateChunksInRadius(p->chunksInRadius_array, playerLocation2D, LoadRadius, ChunkSize);

  for( auto chunkInRadius : p->chunksInRadius_array )
    if( !p->chunksLoaded.Contains(chunkInRadius) && !p->chunksToLoad_set.Contains(chunkInRadius) )
    {
      p->chunksToLoad_array.Push(chunkInRadius);
      p->chunksToLoad_set.Add(chunkInRadius);
    }

  if(!p->chunksToLoad_array.IsEmpty())
    UE_LOG(LogTemp, Warning, TEXT("loading %d new chunks"), p->chunksToLoad_array.Num());

  for( auto chunkToLoad : p->chunksToLoad_array)
    if( !p->chunksLoaded.Contains(chunkToLoad))
      p->chunksLoaded.Add(chunkToLoad, nullptr);

  // TODO: submit work to workers

  // TODO: create actors from just-created chunks collected at the top of this function

  // DELETE (just for testing)
  p->chunksToLoad_array.Reset();
  p->chunksToLoad_set.Reset();
}

// Called when the game starts or when spawned
void AProceduralLandscape::BeginPlay()
{
	Super::BeginPlay();
	
}


