// Fill out your copyright notice in the Description page of Project Settings.


#include "ASpawner.h"

#include "Kismet/GameplayStatics.h"

// Sets default values
AASpawner::AASpawner()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AASpawner::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogTemp, Warning, TEXT("AASpawner::BeginPlay()"));
	UE_LOG(LogTemp, Warning, TEXT("AASpawner spawning one %s"), *spawnable->GetFName().ToString());

	//GetWorld()->SpawnActor(spawnable, &GetTransform());
	ASpawnable *toSpawn = GetWorld()->SpawnActorDeferred<ASpawnable>(spawnable, GetTransform());

//  AActor *newActor = GetWorld()->SpawnActor(this->actorToSpawn.Get(), GetActorTransform())
	toSpawn->material = material;

	UGameplayStatics::FinishSpawningActor(toSpawn, GetTransform());

}

// Called every frame
void AASpawner::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

