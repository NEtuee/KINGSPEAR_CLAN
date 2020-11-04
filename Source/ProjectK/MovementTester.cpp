// Fill out your copyright notice in the Description page of Project Settings.


#include "MovementTester.h"
#include "Camera/CameraComponent.h"

// Sets default values
AMovementTester::AMovementTester()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	AutoPossessPlayer = EAutoReceiveInput::Player0;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

	UCameraComponent* ourCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	visibleComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("visibleComponent"));

	ourCamera->SetupAttachment(RootComponent);
	ourCamera->SetRelativeLocation(FVector(-250.f, 0.f, 250.f));
	ourCamera->SetRelativeRotation(FRotator(-45.f, 0.f, 0.f));
	visibleComponent->SetupAttachment(RootComponent);

}

// Called when the game starts or when spawned
void AMovementTester::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AMovementTester::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!currVelocity.IsZero())
	{
		FVector newLocation = GetActorLocation() + currVelocity * DeltaTime;
		SetActorLocation(newLocation);
	}
}

// Called to bind functionality to input
void AMovementTester::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	InputComponent->BindAxis("MoveForward", this, &AMovementTester::MoveXAxis);
	InputComponent->BindAxis("MoveRight", this, &AMovementTester::MoveYAxis);
}

void AMovementTester::MoveXAxis(float value)
{
	currVelocity.X = FMath::Clamp(value, -1.f, 1.f) * 100.f;
}

void AMovementTester::MoveYAxis(float value)
{
	currVelocity.Y = FMath::Clamp(value, -1.f, 1.f) * 100.f;
}

