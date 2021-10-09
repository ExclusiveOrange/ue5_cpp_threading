// Fill out your copyright notice in the Description page of Project Settings.


#include "PrimesThreadTest.h"

namespace
{
	class PrimeSearchTask : public FNonAbandonableTask
	{
		int32 numPrimesToFind;
		TSharedPtr< std::atomic_bool > busyFindingPrimes;

	public:
		PrimeSearchTask(int32 numPrimesToFind, TSharedPtr< std::atomic_bool > busyFindingPrimes = {})
			: numPrimesToFind{ numPrimesToFind }
			, busyFindingPrimes{ std::move( busyFindingPrimes )}
		{}

		~PrimeSearchTask()
		{
			UE_LOG(LogTemp, Warning, TEXT("Finished searching for primes!"));
			if( busyFindingPrimes )
				*busyFindingPrimes = false;
		}


		// required by UE, ***must have***
		FORCEINLINE TStatId GetStatId() const
		{
			RETURN_QUICK_DECLARE_CYCLE_STAT(PrimeSearchTask, STATGROUP_ThreadPoolAsyncTasks);
		}

		void DoWork()
		{
			int32 numPrimesFound = 0;
			int32 currentTestNumber = 2;

			while (numPrimesFound < numPrimesToFind)
			{
				bool isPrime = true;

				for (int32 i = 2; i < currentTestNumber / 2; ++i)
					if (currentTestNumber % i == 0)
					{
						isPrime = false;
						break;
					}

				if (isPrime)
				{
					numPrimesFound += isPrime;

					if (numPrimesFound % 1000 == 0)
						UE_LOG(LogTemp, Warning, TEXT("Primes found: %i"), numPrimesFound);
				}

				++currentTestNumber;
			}
		}
	};
} // namespace

// Sets default values
APrimesThreadTest::APrimesThreadTest()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void APrimesThreadTest::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void APrimesThreadTest::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bLastBusyFindingPrimes && !*bBusyFindingPrimes)
	{
		bLastBusyFindingPrimes = false;
		*bBusyFindingPrimes = false;
		OnDoneFindingPrimes();
	}
}

void APrimesThreadTest::RunPrimeTaskOnBackgroundThread(int32 numPrimesToFind)
{
	*bBusyFindingPrimes = true;
	bLastBusyFindingPrimes = true;
	(new FAutoDeleteAsyncTask<PrimeSearchTask>(numPrimesToFind, bBusyFindingPrimes))->StartBackgroundTask();
}

void APrimesThreadTest::RunPrimeTaskOnMainThread(int32 numPrimesToFind)
{
	PrimeSearchTask pst(numPrimesToFind);
	pst.DoWork();
	OnDoneFindingPrimes();
}

