// Fill out your copyright notice in the Description page of Project Settings.


#include "RopeActor_Climbing.h"
#include "Components/BoxComponent.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"
#include "CableComponent.h"
#include "Kismet/KismetMathLibrary.h"

ARopeActor_Climbing::ARopeActor_Climbing()
{
	PrimaryActorTick.bCanEverTick = true;

	//스타트 포인트 설정
	mStartPoint = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StartPoint"));
    RootComponent = mStartPoint;
	static ConstructorHelpers::FObjectFinder<UStaticMesh>SphereMeshAsset(TEXT("StaticMesh'/Engine/BasicShapes/Sphere.Sphere'"));
	if (SphereMeshAsset.Succeeded())
	{
		mStartPoint->SetStaticMesh(SphereMeshAsset.Object);
	}


	//피직스 컨스트레인트 설정
	mPhysicsConstraintComponent = CreateDefaultSubobject<UPhysicsConstraintComponent>(TEXT("PhysicsConstraint"));
	mPhysicsConstraintComponent->SetupAttachment(RootComponent);

	//미들 포인트 설정
	mMiddlePoint = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MiddlePoint"));
	mMiddlePoint->SetupAttachment(RootComponent);
	mMiddlePoint->SetRelativeLocation(FVector(0.0f,0.0f,-380.0f));
	if (SphereMeshAsset.Succeeded())
	{
		mMiddlePoint->SetStaticMesh(SphereMeshAsset.Object);
	}
	mMiddlePoint->SetSimulatePhysics(true);
	mMiddlePoint->SetLinearDamping(0.4f);
	mMiddlePoint->SetAngularDamping(1.0f);

	//끝 쪽 케이블 설정
	mEndCable = CreateDefaultSubobject<UCableComponent>(TEXT("EndCable"));
	mEndCable->SetupAttachment(mMiddlePoint);
	mEndCable->SetRelativeLocation(FVector(0.0f,0.0f,-10.0f));
	mEndCable->bAttachStart = true;
	mEndCable->bAttachEnd = false;
	mEndCable->EndLocation = FVector::ZeroVector;
	mEndCable->CableLength = 161.0f;
	mEndCable->NumSegments = 13;
	mEndCable->SolverIterations = 5;
	mEndCable->CableGravityScale = 6.25f;
	mEndCable->SetGenerateOverlapEvents(false);
	mEndCable->SetCollisionProfileName("PhysicsActor");

	//박스 콜리전 설정
	mBoxCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxCollision"));
	mBoxCollision->SetupAttachment(mMiddlePoint);
    mBoxCollision->SetRelativeLocation(FVector(0.0f,0.0f,190.0f));
	mBoxCollision->SetBoxExtent(FVector(64.0f,64.0f,135.2f));
	mBoxCollision->bHiddenInGame = false;
	mBoxCollision->SetGenerateOverlapEvents(true);
	mBoxCollision->SetCollisionProfileName("OverlapAllDynamic");

	//중간 케이블 설정
	mMiddleCable = CreateDefaultSubobject<UCableComponent>(TEXT("MiddleCable"));
	mMiddleCable->SetupAttachment(RootComponent);
	mMiddleCable->bAttachStart=true;
	mMiddleCable->bAttachEnd = true;
	mMiddleCable->SetAttachEndTo(nullptr,"mMiddlePoint");
	mMiddleCable->EndLocation = FVector::ZeroVector;
	mMiddleCable->CableLength = 276.0f;
	mMiddleCable->NumSegments = 10;
	mMiddleCable->SolverIterations = 2;
	mMiddleCable->SetGenerateOverlapEvents(true);
	mMiddleCable->SetCollisionProfileName("PhysicsActor");

	FConstrainComponentPropName comp1;
	comp1.ComponentName = "mStartPoint";
	FConstrainComponentPropName comp2;
	comp2.ComponentName = "mMiddlePoint";
	mPhysicsConstraintComponent->ComponentName1 = comp1;
	mPhysicsConstraintComponent->ComponentName2 = comp2;
	mPhysicsConstraintComponent->SetDisableCollision(true);
	mPhysicsConstraintComponent->SetAngularSwing1Limit(ACM_Free, 45.0f);
	mPhysicsConstraintComponent->SetAngularSwing2Limit(ACM_Free, 45.0f);
	mPhysicsConstraintComponent->SetAngularTwistLimit(ACM_Free, 45.0f);
	mPhysicsConstraintComponent->SetConstrainedComponents(mStartPoint, NAME_None, mMiddlePoint, NAME_None);

}

void ARopeActor_Climbing::BeginPlay()
{
	Super::BeginPlay();
	
}

void ARopeActor_Climbing::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ARopeActor_Climbing::UpdateRope(float length)
{
	float updateLength = length;

	//UE_LOG(LogTemp,Warning,TEXT("UpdateLength : %f , mCableL : %f "),updateLength,mCableLength);

	FVector middlePos = mMiddlePoint->GetComponentLocation();
	FVector startPos = mStartPoint->GetComponentLocation();

	if (updateLength >= mCableLength)
	{
		mCurrentCableLength = mCableLength;
	}
	else
	{
		mCurrentCableLength = updateLength;
	}
	
	FVector updataPos = (UKismetMathLibrary::Normal(middlePos - startPos) * mCurrentCableLength) + startPos;

	mMiddlePoint->SetWorldLocation(updataPos);

	mPhysicsConstraintComponent->SetConstrainedComponents(mStartPoint, NAME_None, mMiddlePoint, NAME_None);
	
	mBoxCollision->SetRelativeLocation(FVector(0.0f,0.0f,mCurrentCableLength*0.5f));
	FVector boxExtent = mBoxCollision->GetScaledBoxExtent();
	boxExtent.Z = mCurrentCableLength*0.5f;
	mBoxCollision->SetBoxExtent(boxExtent,true);

	mEndCable->CableLength = mCableLength-mCurrentCableLength;
	mMiddleCable->CableLength = mCurrentCableLength*0.7f;
	//UE_LOG(LogTemp, Warning, TEXT("middleCabel : %f "),mMiddleCable->CableLength);
}

void ARopeActor_Climbing::ClimbingRope(float direction, float speed)
{
	//UE_LOG(LogTemp, Warning, TEXT("direction : %f "),direction);
    float final = direction *-1.0f* speed * GetWorld()->GetDeltaSeconds() + mCurrentCableLength;

	if (final < mCableLength)
	{
		if (final <= mLengthLimit)
		{
			final = mLengthLimit;
		}
	}
	else
	{
		final = mCableLength;
	}

	UpdateRope(final);
}

void ARopeActor_Climbing::AddSwingForce(FVector force, float multiple)
{
    mMiddlePoint->AddForce(force * multiple, NAME_None, true);
}

FVector ARopeActor_Climbing::GetStartPoint() const
{
    return mStartPoint->GetComponentLocation();
}

UStaticMeshComponent* ARopeActor_Climbing::GetMiddlePoint() const
{
    return mMiddlePoint;
}

FVector ARopeActor_Climbing::GetRopeVelocity() const
{
    return mMiddlePoint->GetPhysicsLinearVelocity();
}

