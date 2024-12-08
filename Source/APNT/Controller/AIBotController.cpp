// Fill out your copyright notice in the Description page of Project Settings.


#include "AIBotController.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"

void AAIBotController::BeginPlay()
{
	Super::BeginPlay();

	// FVector TargetLocation = FVector(500.0f, 500.0f, 100.0f);
	//FVector TargetLocation = UGameplayStatics::GetPlayerPawn(GetWorld(), 0)->GetActorLocation();
	//MoveToLocation(TargetLocation);
}

void AAIBotController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);

	if (!PlayerPawn) 
		return;


	FVector PlayerLocation = PlayerPawn->GetActorLocation();
	FVector AILocation = GetPawn()->GetActorLocation();
	float DistanceToPlayer = FVector::Dist(PlayerLocation, AILocation);

	if (DistanceToPlayer > FollowDistanceMax)
	{
		MoveToLocation(PlayerLocation);
		// UE_LOG(LogTemp, Warning, TEXT("AI Moving to Player: %s"), *PlayerLocation.ToString());
	}
	else if (DistanceToPlayer < FollowDistanceMin) {
		StopMovement();
		// UE_LOG(LogTemp, Warning, TEXT("AI Stopping"));
	}
}
