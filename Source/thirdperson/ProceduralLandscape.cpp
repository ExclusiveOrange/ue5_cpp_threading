// Fill out your copyright notice in the Description page of Project Settings.


#include "ProceduralLandscape.h"
#include "Chunk.h"
#include "ProceduralMeshComponent.h"
#include "HAL/RunnableThread.h"
#include "Kismet/GameplayStatics.h"

#include <chrono>
#include <cmath>
#include <condition_variable>
#include <deque>
#include <memory>
#include <mutex>
#include <stack>

namespace
{
  using clock_t = std::chrono::high_resolution_clock;
  
  struct MeshData
  {
    TArray<FVector> vertices;
    TArray<int32> triangles;
    TArray<FVector> normals;
    TArray<FVector2D> uv0;
    TArray<FLinearColor> colors;
    TArray<FProcMeshTangent> tangents;
  };

  struct GenerationWorkUnit
  {
    MeshData meshData{}; // local coordinates always from (0,0) to (size,size)
    FIntVector chunkLocation{}; // world coordinates of center are (chunkLocation - 0.5) * size
    int32 resolution{1};
    float size{1.f};
  };

  //==============================================================================
  
  void
  createProceduralMeshSection(
    UProceduralMeshComponent* mesh,
    const int32 sectionIndex,
    const MeshData& meshData)
  {
    const auto& [vertices, triangles, normals, uv0, colors, tangents] = meshData;
    mesh->CreateMeshSection_LinearColor(sectionIndex, vertices, triangles, normals, uv0, colors, tangents, true);
  }
  
  //==============================================================================
  
  void
  enumerateChunksInRadius(
    TArray<FIntVector>& chunksInRadius,
    const FVector2D center,
    const float radius,
    const float chunkSize)
  {
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

    /*
    Example chunk registration order as might be generated in the following loop:

      r = 0
    
                  0

      r = 1
    
               5  1  7
               4  0  3
               8  2  6
    
      r = 2
    
           21 13  9 17 23
           20  5  1  7 15
           12  4  0  3 11
           16  8  2  6 19
           24 18 10 14 22

    
      r = 3 (suppose radius is small enough that corners are outside and thus not registered)

              29 25 33
           21 13  9 17 23
        36 20  5  1  7 15 31
        28 12  4  0  3 11 27
        32 16  8  2  6 19 35
           24 18 10 14 22
              34 26 30
    */

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
  }
  
  //------------------------------------------------------------------------------

  void generateMesh(MeshData &meshData, const int32 resolution, const float size)
  {
    // UE_LOG(LogTemp, Warning, TEXT("generateMesh(): xSteps(%d), ySteps(%d)"), meshParameters.xSteps, meshParameters.ySteps);
    
    // make arrays big enough to hold all vertices
    [totalNumVertices=(resolution + 1) * (resolution + 1)]
    (auto& ... object) { (object.Reset(totalNumVertices), ...); }
      (meshData.vertices, meshData.normals, meshData.uv0, meshData.colors, meshData.tangents);

    // set vertex values
    for (int32 y = 0; y <= resolution; ++y)
    {
      const float yFrac = float(y) / resolution;
      const float yPos = yFrac * size;

      for (int32 x = 0; x <= resolution; ++x)
      {
        const float xFrac = float(x) / resolution;
        const float xPos = xFrac * size;

        meshData.vertices.Emplace(xPos, yPos, 0.f);
        meshData.normals.Emplace(0, 0, 1.f);
        meshData.uv0.Emplace(xPos * 0.1f, yPos * 0.1f);
        meshData.colors.Emplace(1.f, 1.f, 1.f, 1.f);
        meshData.tangents.Emplace(1.f, 0.f, 0.f);
      }
    }

    // make triangles array big enough to hold all triangles
    meshData.triangles.Reset(resolution * resolution * 2 * 3);

    int32 count = 0;

    // set triangle indices
    for( int32 y = 0, index = 0; y < resolution; ++y, ++index )
      for( int32 x = 0; x < resolution; ++x, ++index )
      {
        meshData.triangles.Append({index, index + resolution + 1, index + 1});
        meshData.triangles.Append({index + 1, index + resolution + 1, index + resolution + 2});
        ++count;
      }
  }

  //------------------------------------------------------------------------------

  void
  destroyChunksOutsideRadius(
    TMap<FIntVector, AChunk*> &chunksLoaded, // will be removed from this map
    const FVector2D center,
    const float radius,
    const float chunkSize)
  {
    auto chunkIsOutside = [=](const FIntVector chunk)
    {
      return (FVector2D{chunk.X*chunkSize, chunk.Y*chunkSize}-center).SizeSquared() > radius*radius;
    };

    for( auto it = chunksLoaded.CreateIterator(); it; ++it )
      if( chunkIsOutside(it.Key()))
      {
        // it.Value()->RemoveFromRoot(); // not sure if I need to do this
        it.Value()->Destroy();
        it.RemoveCurrent();
      }
  }

  //==============================================================================

  class MeshGenerator : public FRunnable
  {
    std::mutex workMutex;
    std::condition_variable workConditionVariable; // main notifies workers
    std::deque<std::unique_ptr<GenerationWorkUnit>> workQueue; // lock before access
    bool shouldStop{}; // set true then notify workConditionVariable then wait for worker to finish

    std::mutex doneMutex;
    TArray<std::unique_ptr<GenerationWorkUnit>> doneWork; // lock before access

    FRunnableThread* thread;

  public:
    MeshGenerator()
      : thread{FRunnableThread::Create(this, TEXT("MeshGeneratorThread"))}
    {}

    ~MeshGenerator() override
    {
      if (thread)
      {
        thread->Kill(true);
        delete thread;
      }
    }

    //------------------------------------------------------------------------------
    // FRunnable

    // bool Init() override {}

    uint32 Run() override
    {
      UE_LOG(LogTemp, Warning, TEXT("MeshGenerator::Run() starting"));

      for (;;)
      {
        std::unique_ptr<GenerationWorkUnit> workUnit;

        {
          std::unique_lock lock(workMutex);
          workConditionVariable.wait(lock, [this] { return shouldStop || !workQueue.empty(); });

          if (shouldStop)
            break;

          workUnit = std::move(workQueue.front());
          workQueue.pop_front();
        }

        // UE_LOG(LogTemp, Warning, TEXT("MeshGenerator::Run() generating mesh"));
        generateMesh(workUnit->meshData, workUnit->resolution, workUnit->size);

        {
          std::lock_guard lock(doneMutex);
          doneWork.Push(std::move(workUnit));
        }
      }

      UE_LOG(LogTemp, Warning, TEXT("MeshGenerator::Run() stopping"));

      return 0;
    }

    void Stop() override
    {
      shouldStop = true;
      workConditionVariable.notify_all();
    }

    // void Exit() override;

    //------------------------------------------------------------------------------

    TArray<std::unique_ptr<GenerationWorkUnit>>
    getCompletedWork(TArray<std::unique_ptr<GenerationWorkUnit>> emptyArray)
    {
      emptyArray.Reset(); // make sure it's empty
      std::lock_guard lock(doneMutex);
      return std::exchange(doneWork, std::move(emptyArray));
    }

    TArray<std::unique_ptr<GenerationWorkUnit>> // the same array but now empty
    submitWorkToDo(TArray<std::unique_ptr<GenerationWorkUnit>> workUnits)
    {
      if(!workUnits.IsEmpty())
      {
        {
          std::lock_guard lock(workMutex);
          for(auto &workUnit : workUnits)
            workQueue.push_back(std::move(workUnit));
        }
        workConditionVariable.notify_all();
        workUnits.Reset();
      }
      
      return std::move(workUnits);
    }
  };

  //==============================================================================

  struct UnusedWorkUnits
  {
    TArray<std::unique_ptr<GenerationWorkUnit>> unusedWorkUnits;

    std::unique_ptr<GenerationWorkUnit>
    getUnusedWorkUnit()
    {
      if (unusedWorkUnits.IsEmpty())
        return std::make_unique<GenerationWorkUnit>();

      return unusedWorkUnits.Pop(false);
    };

    void
    putUnusedWorkUnit(std::unique_ptr<GenerationWorkUnit> workUnit)
    {
      unusedWorkUnits.Push(std::move(workUnit));
    }
  };
} // namespace

//==============================================================================

struct AProceduralLandscape::Private : UnusedWorkUnits
{
  TArray<FIntVector> chunksInRadius_array; // order matters
  
  TArray<std::unique_ptr<GenerationWorkUnit>> chunksToGenerate;            // order matters
  TArray<std::unique_ptr<GenerationWorkUnit>> chunksGenerated;             // order matters
  TArray<std::unique_ptr<GenerationWorkUnit>> chunksGeneratedAndInRadius;  // order matters
  
  TSet<FIntVector> chunksLoading;       // presence matters
  TMap<FIntVector, AChunk*> chunksLoaded;  // presence matters
  TArray<AChunk*> chunksToUnload;

  std::unique_ptr<MeshGenerator> meshGenerator = std::make_unique<MeshGenerator>();
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

  const FVector2D playerLocation2D{ GetWorld()->GetFirstPlayerController()->GetPawn()->GetActorLocation() };
  
  //- - - - - - - - - - - - - - - - - - - - 

  // check if old chunks need to be unloaded
  destroyChunksOutsideRadius(p->chunksLoaded, playerLocation2D, UnloadRadius, ChunkSize);
  
  //- - - - - - - - - - - - - - - - - - - - 
  
  // get list of chunks which might need to be loaded
  enumerateChunksInRadius(p->chunksInRadius_array, playerLocation2D, LoadRadius, ChunkSize);

  // refine list to chunks which do need to be loaded
  for( auto chunkInRadius : p->chunksInRadius_array )
    if( !p->chunksLoaded.Contains(chunkInRadius) && !p->chunksLoading.Contains(chunkInRadius) )
    {
      std::unique_ptr<GenerationWorkUnit> workUnit = p->getUnusedWorkUnit();
      workUnit->chunkLocation = chunkInRadius;
      workUnit->resolution = StepsPerChunk;
      workUnit->size = ChunkSize;
      p->chunksToGenerate.Emplace(std::move(workUnit));
    }
  
  //- - - - - - - - - - - - - - - - - - - - 

  // get fresh chunks
  p->chunksGenerated = p->meshGenerator->getCompletedWork(std::move(p->chunksGenerated));
  
  // update set of chunksLoading AND discard fresh chunks that are now outside of UnloadRadius
  for( auto &workUnit : p->chunksGenerated )
  {
    p->chunksLoading.Remove(workUnit->chunkLocation);
    
    if(const auto [x,y,z] = workUnit->chunkLocation;
      (FVector2D{x*ChunkSize,y*ChunkSize}-playerLocation2D).SizeSquared() <= UnloadRadius*UnloadRadius)
        p->chunksGeneratedAndInRadius.Push(std::move(workUnit));
    else
      p->putUnusedWorkUnit(std::move(workUnit));
  }
  p->chunksGenerated.Reset();
  
  //- - - - - - - - - - - - - - - - - - - - 

  // start generating meshes (loading) chunks asynchronously
  for( const auto &workUnit : p->chunksToGenerate )
    p->chunksLoading.Add(workUnit->chunkLocation);
  p->chunksToGenerate = p->meshGenerator->submitWorkToDo(std::move(p->chunksToGenerate));
  
  //- - - - - - - - - - - - - - - - - - - - 

  // create actors from just-created chunks collected at the top of this function
  for( auto &workUnit : p->chunksGeneratedAndInRadius )
  {
    AChunk* chunkActor = GetWorld()->SpawnActorDeferred<AChunk>(AChunk::StaticClass(), FTransform());

    createProceduralMeshSection(chunkActor->mesh, 0, workUnit->meshData);
    chunkActor->mesh->SetMaterial(0, LandscapeMaterial);
    chunkActor->SetFolderPath("/Chunks");

    const FVector chunkTranslation = (FVector{workUnit->chunkLocation} - 0.5f) * ChunkSize;
    UGameplayStatics::FinishSpawningActor(chunkActor, FTransform{chunkTranslation});

    if(p->chunksLoaded.Contains(workUnit->chunkLocation))
      UE_LOG(LogTemp, Warning, TEXT("ERROR: trying to add loaded chunk that is already loaded"));
      
    p->chunksLoaded.Add(workUnit->chunkLocation, chunkActor);
    
    p->putUnusedWorkUnit(std::move(workUnit));
  }
  p->chunksGeneratedAndInRadius.Reset();
}

// Called when the game starts or when spawned
void AProceduralLandscape::BeginPlay()
{
	Super::BeginPlay();
	
}


