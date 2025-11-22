// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/InstancedSkinnedMeshComponent.h"
#include "MyInstancedSkinnedMeshComponent.generated.h"

/**
 * 
 */
UCLASS()
class VAT_API UMyInstancedSkinnedMeshComponent : public UInstancedSkinnedMeshComponent
{
	GENERATED_BODY()
	
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;
};
