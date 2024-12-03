// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "AIBotController.generated.h"

/**
 * 
 */
UCLASS()
class APNT_API AAIBotController : public AAIController
{
	GENERATED_BODY()
	
protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

private:
	float FollowDistanceMin = 200.0f;
	float FollowDistanceMax = 500.0f;
};
