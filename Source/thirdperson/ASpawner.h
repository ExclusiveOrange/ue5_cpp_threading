// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Spawnable.h"

#include "ASpawner.generated.h"

UCLASS()
class THIRDPERSON_API AASpawner : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AASpawner();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	void Tick(float DeltaTime) override;

	UPROPERTY(EditInstanceOnly)
	TSubclassOf<ASpawnable> spawnable;

	UPROPERTY(EditInstanceOnly)
	UMaterialInterface *material;
};
