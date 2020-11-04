// Fill out your copyright notice in the Description page of Project Settings.


#include "TestAgent.h"

// Sets default values
ATestAgent::ATestAgent()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ATestAgent::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ATestAgent::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ATestAgent::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

