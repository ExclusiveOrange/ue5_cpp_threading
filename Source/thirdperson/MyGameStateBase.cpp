// Fill out your copyright notice in the Description page of Project Settings.


#include "MyGameStateBase.h"

AMyGameStateBase::AMyGameStateBase()
{
  UE_LOG(LogTemp, Warning, TEXT("AMyGameStateBase::AMyGameStateBase()"));
}

AMyGameStateBase::~AMyGameStateBase()
{
  UE_LOG(LogTemp, Warning, TEXT("AMyGameStateBase::~AMyGameStateBase()"));
  // TODO
}

void
AMyGameStateBase::TestFunction()
{
  UE_LOG(LogTemp, Warning, TEXT("AMyGameStateBase::TestFunction()"));
}

void
AMyGameStateBase::LoadGameChunk(FIntVector chunkAddress)
{
  UE_LOG(LogTemp, Warning, TEXT("AMyGameStateBase::LoadGameChunk(..): {%d, %d, %d}"), chunkAddress.X, chunkAddress.Y, chunkAddress.Z);
}

