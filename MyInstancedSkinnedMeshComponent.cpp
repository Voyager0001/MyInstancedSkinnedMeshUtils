// Fill out your copyright notice in the Description page of Project Settings.


#include "MyInstancedSkinnedMeshComponent.h"
#include "Components/MeshComponent.h"
#include "Engine/World.h"

void UMyInstancedSkinnedMeshComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{
	SCOPED_NAMED_EVENT(UMyInstancedSkinnedMeshComponent_TickComponent, FColor::Yellow);
	// SCOPE_CYCLE_COUNTER(STAT_SkinnedMeshCompTick);

	// We cant run the blueprint events (ReceiveTick, latent actions) on worker threads, so skip them if we are running as such
	if(!PrimaryComponentTick.bRunOnAnyThread)
	{
		// Tick ActorComponent first.
		// Call UMeshComponent::TickComponent to bypass USkinnedMeshComponent::TickComponent logic
		UMeshComponent::TickComponent(DeltaTime, TickType, ThisTickFunction);
	}

	// See if this mesh was rendered recently. This has to happen first because other data will rely on this
#if WITH_STATE_STREAM_ACTOR
	bRecentlyRendered = true;
#else
	bRecentlyRendered = ((bUseScreenRenderStateForUpdate ? GetLastRenderTimeOnScreen() : GetLastRenderTime()) > GetWorld()->TimeSeconds - 1.0f);
#endif

	// Update component's LOD settings
	// This must be done BEFORE animation Update and Evaluate (TickPose and RefreshBoneTransforms respectively)
	const bool bLODHasChanged = UpdateLODStatus();

	// Skip the rest of the work if we are running on worker threads as none of it is safe
	if(!PrimaryComponentTick.bRunOnAnyThread)
	{

		DispatchParallelTickPose( ThisTickFunction );
		
		auto constexpr bAllowPoseRefresh = false;

		// Tick Pose first
		if ( bAllowPoseRefresh && ShouldTickPose())//never called in editor
		{
			TickPose(DeltaTime, false);
		}

		// If we have been recently rendered, and bForceRefPose has been on for at least a frame, or the LOD changed, update bone matrices.
		if( bAllowPoseRefresh && ShouldUpdateTransform(bLODHasChanged) )//never called in editor
		{
			// Do not update bones if we are taking bone transforms from another SkelMeshComp
			if( LeaderPoseComponent.IsValid() )
			{
				UpdateFollowerComponent();
			}
			else 
			{
				RefreshBoneTransforms(ThisTickFunction);
			}
		}
		else if(VisibilityBasedAnimTickOption == EVisibilityBasedAnimTickOption::AlwaysTickPose)
		{
			// We are not refreshing bone transforms, but we do want to tick pose. We may need to kick off a parallel task
			DispatchParallelTickPose(ThisTickFunction);
		}
#if WITH_EDITOR
		else 
		{
			// only do this for level viewport actors
			UWorld* World = GetWorld();
			if (World && World->WorldType == EWorldType::Editor)
			{
				RefreshMorphTargets();
			}
		}
#endif // WITH_EDITOR
	}
}

