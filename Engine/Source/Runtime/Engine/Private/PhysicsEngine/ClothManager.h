// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

/*=============================================================================
	ClothManager.h
	In charge of passing data between skeletal meshes and cloth solver
=============================================================================*/

#pragma once

class FPhysScene;
class USkeletalMeshComponent;

/** Determines when prepare cloth work will be done in relation the rigid body simulation */
enum class PrepareClothSchedule
{
	IgnorePhysics = 0,	//Do not wait for rigid body sim
	WaitOnPhysics,	//Wait for rigid body sim (e.g. ragdolls with cloth)
	MAX
};

struct FPrepareClothTickFunction : public FTickFunction
{
	class FClothManagerData* Target;

	// FTickFunction interface
	virtual void ExecuteTick(float DeltaTime, enum ELevelTick TickType, ENamedThreads::Type CurrentThread, const FGraphEventRef& MyCompletionGraphEvent) override;
	virtual FString DiagnosticMessage() override;
	// End of FTickFunction interface
};

/** Break the data into class since we have multiple passes for different scheduling purposes */
class FClothManagerData
{
public:
	TArray<USkeletalMeshComponent*> SkeletalMeshComponents;
	FPrepareClothTickFunction TickFunction;
	FThreadSafeBool IsPreparingCloth;

	/** Go through all SkeletalMeshComponents registered for work this frame and call the solver as needed*/
	void PrepareCloth(float DeltaTime);
};


class FClothManager
{
public:

	FClothManager(UWorld* AssociatedWorld);

	/** Register the SkeletalMeshComponent so that it does cloth solving work this frame */
	void RegisterForPrepareCloth(USkeletalMeshComponent* SkeletalMeshComponent, PrepareClothSchedule PrepSchedule);

	/** Whether the cloth manager is currently doing work in an async thread */
	bool IsPreparingClothAsync() const
	{
		bool bIsPreparingCloth = false;
		for(const FClothManagerData& PrepareCloth : PrepareClothDataArray)
		{
			return bIsPreparingCloth |= PrepareCloth.IsPreparingCloth;
		}

		return bIsPreparingCloth;
	}
	
private:
	
	FClothManagerData PrepareClothDataArray[(int32)PrepareClothSchedule::MAX];
};