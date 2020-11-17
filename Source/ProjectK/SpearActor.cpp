// Fill out your copyright notice in the Description page of Project Settings.


#include "SpearActor.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/BoxComponent.h"


// Sets default values
ASpearActor::ASpearActor()
{
	PrimaryActorTick.bCanEverTick = true;

	mProjectile = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("Projectile"));
	mProjectile->InitialSpeed = 3000.0f;
	mProjectile->MaxSpeed = 3000.0f;
	mProjectile->bRotationFollowsVelocity = true;
	mProjectile->bInitialVelocityInLocalSpace = true;
	mProjectile->bShouldBounce = true;
	mProjectile->Bounciness = 0.1f;
	mProjectile->Friction = 0.5f;
	mProjectile->Velocity = FVector(3000.0f,0.0f,0.0f);
	
	mBoxCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("Box"));
	RootComponent = mBoxCollision;
	mBoxCollision->SetBoxExtent(FVector(32.0f, 32.0f, 32.0f));
	mBoxCollision->SetSimulatePhysics(true);
	mBoxCollision->SetGenerateOverlapEvents(true);

}

void ASpearActor::BeginPlay()
{
	Super::BeginPlay();

	mBoxCollision->OnComponentHit.AddDynamic(this,&ASpearActor::OnSpearHit);
	
}

void ASpearActor::OnSpearBeginOverlap(class UPrimitiveComponent* overlappedComp, class AActor* otherActor, class UPrimitiveComponent* otherComp, int32 otherBodyIndex, bool bFromSweep, const FHitResult& sweepResult)
{

}

void ASpearActor::OnSpearHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
   UE_LOG(LogTemp,Warning,TEXT("Spear coll"));

   mProjectile->ProjectileGravityScale = 0.0f;
   RootComponent->AttachToComponent(OtherComp,FAttachmentTransformRules(EAttachmentRule::KeepWorld,EAttachmentRule::KeepWorld,EAttachmentRule::KeepWorld,true));
}

// Called every frame
void ASpearActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

