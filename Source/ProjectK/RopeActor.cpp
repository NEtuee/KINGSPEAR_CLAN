// Fill out your copyright notice in the Description page of Project Settings.


#include "RopeActor.h"
#include "Components/CapsuleComponent.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"
#include "CableComponent.h"

// Sets default values
ARopeActor::ARopeActor()
{
	PrimaryActorTick.bCanEverTick = true;

	//로프 액터의 스테틱 메쉬들 설정
	mStartPoint = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StartPoint"));
	RootComponent = mStartPoint;
	static ConstructorHelpers::FObjectFinder<UStaticMesh>SphereMeshAsset(TEXT("StaticMesh'/Engine/BasicShapes/Sphere.Sphere'"));
	if (SphereMeshAsset.Succeeded())
	{
		mStartPoint->SetStaticMesh(SphereMeshAsset.Object);
	}

	//케이블 컴포넌트 설정
	mCable = CreateDefaultSubobject<UCableComponent>(TEXT("Cable"));
	mCable->SetupAttachment(RootComponent);
	mCable->bAttachStart = true;
	mCable->bAttachEnd = true;
	mCable->SetAttachEndTo(nullptr, "mEndPoint");
	mCable->EndLocation = FVector::ZeroVector;
	mCable->CableLength = 358.0f;
	mCable->NumSegments = 10;

	mEndPoint = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("EndPoint"));
	mEndPoint->SetupAttachment(RootComponent);
	if (SphereMeshAsset.Succeeded())
	{
		mEndPoint->SetStaticMesh(SphereMeshAsset.Object);
	}
	mEndPoint->SetRelativeLocation(FVector(0.0f,0.0f,-370.0f));
	mEndPoint->SetRelativeScale3D(FVector(0.3f,0.3f,0.3f));

	//케이블 엔드 콜리전 설정
	mEndPointCollision = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Collision"));
	mEndPointCollision->SetupAttachment(mEndPoint);
	mEndPointCollision->SetCapsuleHalfHeight(480.0f);
	mEndPointCollision->SetCapsuleRadius(350.0f);
	mEndPointCollision->SetGenerateOverlapEvents(true);

	//피직스 컨스트레인트 컴포넌트 설정
	mPhysicsConstraintComponent = CreateDefaultSubobject<UPhysicsConstraintComponent>(TEXT("PhysicsConstraint"));
	mPhysicsConstraintComponent->SetupAttachment(RootComponent);
	FConstrainComponentPropName comp1;
	comp1.ComponentName = "mStartPoint";
	FConstrainComponentPropName comp2;
	comp2.ComponentName = "mEndPoint";
	mPhysicsConstraintComponent->ComponentName1 = comp1;
	mPhysicsConstraintComponent->ComponentName2 = comp2;
	mPhysicsConstraintComponent->SetDisableCollision(true);
	mPhysicsConstraintComponent->SetAngularSwing1Limit(ACM_Free,45.0f);
	mPhysicsConstraintComponent->SetAngularSwing2Limit(ACM_Free,45.0f);
	mPhysicsConstraintComponent->SetAngularTwistLimit(ACM_Free,45.0f);
	mPhysicsConstraintComponent->SetConstrainedComponents(mStartPoint, NAME_None, mEndPoint, NAME_None);

	
}

// Called when the game starts or when spawned
void ARopeActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ARopeActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ARopeActor::AddSwingForce(FVector force, float multiple)
{
    
    mEndPoint->AddForce(force*multiple,NAME_None,true);
	//UE_LOG(LogTemp, Warning, TEXT("%f , %f , %f"),force.X, force.Y, force.Z);
	//UE_LOG(LogTemp,Warning,TEXT("AddSwingForce"));
}

void ARopeActor::DestroyRope()
{
    this->Destroy();
}

FVector ARopeActor::GetRopeVelocity() const
{
    return mEndPoint->GetPhysicsLinearVelocity();
}

void ARopeActor::SetEndPointLocation(FVector location)
{
    mEndPoint->SetWorldLocation(location);
}

void ARopeActor::SetCableLength(float length)
{
    mCable->CableLength = length;
}

void ARopeActor::ReSetConstrained()
{
	mPhysicsConstraintComponent->SetConstrainedComponents(mStartPoint, NAME_None, mEndPoint, NAME_None);
}

