// Fill out your copyright notice in the Description page of Project Settings.


#include "AIBotController.h"
#include "GameFramework/Actor.h"

void AAIBotController::BeginPlay()
{
	Super::BeginPlay();

	FVector TargetLocation = FVector(500.0f, 500.0f, 100.0f);
	MoveToLocation(TargetLocation);
}