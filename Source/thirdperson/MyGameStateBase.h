// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "Math/IntVector.h"
#include "MyGameStateBase.generated.h"

/**
 * 
 */
UCLASS()
class THIRDPERSON_API AMyGameStateBase : public AGameStateBase
{
	GENERATED_BODY()

public:
	AMyGameStateBase();
	~AMyGameStateBase();

	FVector vec;

	UFUNCTION(BlueprintCallable)
	void TestFunction();

	UFUNCTION(BlueprintCallable)
	void LoadGameChunk(FIntVector chunkAddress);
	
};
