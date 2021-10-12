// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include <atomic>

#include "PrimesThreadTest.generated.h"

UCLASS()
class THIRDPERSON_API APrimesThreadTest : public AActor
{
	GENERATED_BODY()

	TSharedPtr< std::atomic_bool > bBusyFindingPrimes{ new std::atomic_bool( false ) };
	bool bLastBusyFindingPrimes{ false };
	
public:	
	// Sets default values for this actor's properties
	APrimesThreadTest();

protected:
	// Called when the game starts or when spawned
  void BeginPlay() override;

public:	
	// Called every frame
	void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable)
	void RunPrimeTaskOnBackgroundThread(int32 numPrimesToFind);

	UFUNCTION(BlueprintCallable)
	void RunPrimeTaskOnMainThread(int32 numPrimesToFind);

	UFUNCTION(BlueprintImplementableEvent)
	void OnDoneFindingPrimes();
};
