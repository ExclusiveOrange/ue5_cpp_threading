// Fill out your copyright notice in the Description page of Project Settings.


#include "ThreadedProceduralMesh.h"
#include "ProceduralMeshComponent.h"
#include "Chunk.h"
#include "HAL/RunnableThread.h"

#include <condition_variable>
#include <deque>
#include <memory>
#include <mutex>
#include <stack>

#include "Kismet/GameplayStatics.h"

namespace
{
  struct MeshParameters
  {
    int32 xSteps, ySteps;
    float xTotalUnits, yTotalUnits;
  };

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
    MeshData meshData{};
    MeshParameters meshParameters{};
  };

  //------------------------------------------------------------------------------

  void generateMesh(MeshData &meshData, const MeshParameters meshParameters)
  {
    UE_LOG(LogTemp, Warning, TEXT("generateMesh(): xSteps(%d), ySteps(%d)"), meshParameters.xSteps, meshParameters.ySteps);
    
    // vertices
    // make arrays big enough to hold all vertices
    [totalNumVertices=(meshParameters.xSteps + 1) * (meshParameters.ySteps + 1)]
    (auto& ... object) { (object.Reset(totalNumVertices), ...); }
      (meshData.vertices, meshData.normals, meshData.uv0, meshData.colors, meshData.tangents);

    // set vertex values
    for (int32 y = 0; y <= meshParameters.ySteps; ++y)
    {
      const float yFrac = float(y) / meshParameters.ySteps;
      const float yPos = yFrac * meshParameters.yTotalUnits;

      for (int32 x = 0; x <= meshParameters.xSteps; ++x)
      {
        const float xFrac = float(x) / meshParameters.xSteps;
        const float xPos = xFrac * meshParameters.xTotalUnits;

        meshData.vertices.Emplace(xPos, yPos, 0.f);
        meshData.normals.Emplace(0, 0, 1.f);
        meshData.uv0.Emplace(xPos * 0.1f, yPos * 0.1f);
        meshData.colors.Emplace(1.f, 1.f, 1.f, 1.f);
        meshData.tangents.Emplace(1.f, 0.f, 0.f);
      }
    }

    // triangles
    // make triangles array big enough to hold all triangles
    meshData.triangles.Reset(meshParameters.xSteps * meshParameters.ySteps * 2 * 3);

    int32 count = 0;

    // set triangle indices
    for( int32 y = 0, index = 0; y < meshParameters.ySteps; ++y, ++index )
      for( int32 x = 0; x < meshParameters.xSteps; ++x, ++index )
      {
        const int32 rowSize = meshParameters.xSteps + 1;
        meshData.triangles.Append({index, index + rowSize, index + 1});
        meshData.triangles.Append({index + 1, index + rowSize, index + rowSize + 1});
        ++count;
      }
    
    UE_LOG(LogTemp, Warning, TEXT("generateMesh(): count(%d)"), count);
  }

  void generateMesh(GenerationWorkUnit &workUnit)
  {
    generateMesh(workUnit.meshData, workUnit.meshParameters);
  }

  MeshParameters
  getMeshParameters(const AThreadedProceduralMesh &aMesh)
  {
    MeshParameters p;

    p.xSteps = aMesh.xSteps;
    p.ySteps = aMesh.ySteps;
    p.xTotalUnits = aMesh.xTotalUnits;
    p.yTotalUnits = aMesh.yTotalUnits;

    return p;
  }

  void createProceduralMeshSection(
    UProceduralMeshComponent* mesh,
    const int32 sectionIndex,
    const MeshData& meshData)
  {
    const auto& [vertices, triangles, normals, uv0, colors, tangents] = meshData;
    mesh->CreateMeshSection_LinearColor(sectionIndex, vertices, triangles, normals, uv0, colors, tangents, true);
  }
} // namespace

//==============================================================================

class MeshGenerator : public FRunnable
{
  std::mutex workMutex;
  std::condition_variable workConditionVariable; // main notifies workers
  std::deque<std::unique_ptr<GenerationWorkUnit>> workQueue; // lock before access
  bool shouldStop{}; // set true then notify workConditionVariable then wait for worker to finish

  std::mutex doneMutex;
  std::deque<std::unique_ptr<GenerationWorkUnit>> doneQueue; // lock before access
  
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

      UE_LOG(LogTemp, Warning, TEXT("MeshGenerator::Run() generating mesh"));
      ::generateMesh(*workUnit);

      {
        std::lock_guard lock(doneMutex);
        doneQueue.push_back(std::move(workUnit));
      }
    }

    UE_LOG(LogTemp, Warning, TEXT("MeshGenerator::Run() returning"));

    return 0;
  }

  void Stop() override
  {
    shouldStop = true;
    workConditionVariable.notify_all();
  }
  
  // void Exit() override;

  //------------------------------------------------------------------------------

  void generateMesh(std::unique_ptr<GenerationWorkUnit> workUnit)
  {
    {
      std::lock_guard lock(workMutex);
      workQueue.push_back(std::move( workUnit ));
    }
    workConditionVariable.notify_one();
  }

  std::unique_ptr<GenerationWorkUnit>
  getCompletedMesh()
  {
    std::lock_guard lock(doneMutex);

    if (!doneQueue.empty())
    {
      std::unique_ptr workUnit = std::move(doneQueue.front());
      doneQueue.pop_front();
      return workUnit;
    }

    return nullptr;
  }
};

//==============================================================================

struct AThreadedProceduralMesh::Private
{
  std::unique_ptr<MeshGenerator> meshGenerator;

  UProceduralMeshComponent* mesh;

  std::stack<std::unique_ptr<GenerationWorkUnit>> unusedWorkUnits;

  bool generated{};
  bool generating{};

  std::unique_ptr<GenerationWorkUnit>
  getUnusedWorkUnit()
  {
    if (unusedWorkUnits.empty())
      return std::make_unique<GenerationWorkUnit>();

    std::unique_ptr workUnit = std::move(unusedWorkUnits.top());
    unusedWorkUnits.pop();
    return workUnit;
  }

  void
  putUnusedWorkUnit( std::unique_ptr<GenerationWorkUnit> workUnit)
  {
    unusedWorkUnits.push(std::move(workUnit));
  }
};

//==============================================================================

// Sets default values
AThreadedProceduralMesh::AThreadedProceduralMesh()
  : p{new Private}
{
  // Create a default sub-object so we can see (and move) something in the Editor.
  {
    p->mesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("GeneratedMesh"));

    {
      MeshParameters meshParameters = getMeshParameters(*this);
      meshParameters.xSteps = 1;
      meshParameters.ySteps = 1;

      {
        std::unique_ptr workUnit = p->getUnusedWorkUnit();

        workUnit->meshParameters = meshParameters;
        generateMesh(*workUnit);
        createProceduralMeshSection(p->mesh, 0, workUnit->meshData);

        p->putUnusedWorkUnit(std::move(workUnit));
      }
    }

    RootComponent = p->mesh;
  }
  
  p->meshGenerator = std::make_unique<MeshGenerator>();

  // not sure if I need all of these but it seems Unreal changes how ticks work from version to version and this combo works
  PrimaryActorTick.bCanEverTick = true;
  PrimaryActorTick.bRunOnAnyThread = false; // only run on main thread because we need to create world objects
  PrimaryActorTick.SetTickFunctionEnable(true); // this is necessary to get ticks to work without a Blueprint as of 2021.10.08
}

AThreadedProceduralMesh::~AThreadedProceduralMesh()
{
  delete p;
}

void AThreadedProceduralMesh::BeginPlay()
{
  Super::BeginPlay();
  UE_LOG(LogTemp, Warning, TEXT("AThreadedProceduralMesh::BeginPlay()"));
  SetActorHiddenInGame(true);
}

void AThreadedProceduralMesh::OnConstruction(const FTransform& Transform)
{
  Super::OnConstruction(Transform);
  UE_LOG(LogTemp, Warning, TEXT("AThreadedProceduralMesh::OnConstruction()"));
  p ->mesh->SetMaterial(0, Material);
}

void AThreadedProceduralMesh::Tick(float DeltaTime)
{
  Super::Tick(DeltaTime);

  if (!p->generated)
  {
    if (!p->generating)
    {
      UE_LOG(LogTemp, Warning, TEXT("AThreadedProceduralMesh::Tick(): enqueing mesh generation"));
      std::unique_ptr workUnit = p->getUnusedWorkUnit();
      workUnit->meshParameters = getMeshParameters(*this);
      p->generating = true;
      p->meshGenerator->generateMesh(std::move(workUnit));
    }
    else
    {
      UE_LOG(LogTemp, Warning, TEXT("AThreadedProceduralMesh::Tick(): checking if generation done"));

      if (std::unique_ptr workUnit = p->meshGenerator->getCompletedMesh())
      {
        UE_LOG(LogTemp, Warning, TEXT("AThreadedProceduralMesh::Tick(): got generated mesh; setting mesh section"));

        const auto location = GetTransform().GetLocation();
        UE_LOG(LogTemp, Warning, TEXT("AThreadedProceduralMesh::Tick(): creating AChunk at (%f,%f,%f)"), location.X,
               location.Y, location.Z);

        AChunk* chunk = GetWorld()->SpawnActorDeferred<AChunk>(AChunk::StaticClass(), FTransform());

        // createProceduralMeshSection(chunk->mesh, 0, workUnit->meshData);
        p->putUnusedWorkUnit(std::move(workUnit));
        chunk->mesh->SetMaterial(0, Material);
        chunk->SetFolderPath("/Chunks");

        // chunk->SetActorTransform(GetTransform());
        UGameplayStatics::FinishSpawningActor(chunk, GetTransform());

        p->generating = false;
        p->generated = true;
      }
    }
  }
}

