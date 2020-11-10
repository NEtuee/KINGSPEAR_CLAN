// Fill out your copyright notice in the Description page of Project Settings.


#include "HandIKComponent.h"
#include "KCharacter.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"

UHandIKComponent::UHandIKComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	mTraceSphereRaduis = 40.0f;
	mTraceLineDist = 200.0f;
}


void UHandIKComponent::BeginPlay()
{
	Super::BeginPlay();

	mOwnerCharacter = Cast<AKCharacter>(GetOwner());
	check(mOwnerCharacter != nullptr);
}


void UHandIKComponent::leftHandTraceBySphere()
{
    FVector socketLocation = mOwnerCharacter->GetMesh()->GetSocketLocation("LeftHandSocket");
	FVector targetLocation = mOwnerCharacter->GetMesh()->GetBoneLocation("LeftHand");

	FHitResult hitResult;
	TArray<AActor*> ignore;
	ignore.Add(GetOwner());

	bool bResult = UKismetSystemLibrary::SphereTraceSingle(GetWorld(),socketLocation,targetLocation,mTraceSphereRaduis,ETraceTypeQuery::TraceTypeQuery3,false,ignore,EDrawDebugTrace::ForDuration,hitResult,true);

	if (bResult == true)
	{
	    //UE_LOG();
		mHandIKValue.leftHand_Effector = hitResult.ImpactPoint;
		mLeftHandGap = mHandIKValue.leftHand_Effector - mOwnerCharacter->GetActorLocation();
		mbDoTraceLeft = false;
	}
}

void UHandIKComponent::rightHandTraceBySphere()
{
	FVector socketLocation = mOwnerCharacter->GetMesh()->GetSocketLocation("RightHandSocket");
	FVector targetLocation = mOwnerCharacter->GetMesh()->GetBoneLocation("RightHand");

	FHitResult hitResult;
	TArray<AActor*> ignore;
	ignore.Add(GetOwner());

	bool bResult = UKismetSystemLibrary::SphereTraceSingle(GetWorld(), socketLocation, targetLocation, mTraceSphereRaduis, ETraceTypeQuery::TraceTypeQuery3, false, ignore, EDrawDebugTrace::ForDuration, hitResult, true);

	if (bResult == true)
	{
		//UE_LOG();
		mHandIKValue.rightHand_Effector = hitResult.ImpactPoint;
		mRightHandGap = mHandIKValue.rightHand_Effector - mOwnerCharacter->GetActorLocation();
		mbDoTraceRight = false;
	}
}

void UHandIKComponent::leftHandTraceByLine()
{
    FVector socketLocation = mOwnerCharacter->GetMesh()->GetSocketLocation("LeftHandSocket");
	FVector targetLocation = socketLocation + UKismetMathLibrary::GetForwardVector(mOwnerCharacter->GetMesh()->GetSocketRotation("LeftHandSocket")) * mTraceLineDist;
	
	FHitResult hitResult;
	TArray<AActor*> ignore;
	ignore.Add(GetOwner());

	bool bResult = UKismetSystemLibrary::LineTraceSingle(GetWorld(), socketLocation, targetLocation, UEngineTypes::ConvertToTraceType(ECC_Visibility), true, ignore,EDrawDebugTrace::ForOneFrame, hitResult, true);

	if (bResult == true)
	{
		mHandIKValue.leftHand_Effector = hitResult.ImpactPoint;
		mLeftHandGap = mHandIKValue.leftHand_Effector - mOwnerCharacter->GetActorLocation();
	}
}

void UHandIKComponent::rightHandTraceByLine()
{
	FVector socketLocation = mOwnerCharacter->GetMesh()->GetSocketLocation("RightHandSocket");
	FVector targetLocation = socketLocation + UKismetMathLibrary::GetForwardVector(mOwnerCharacter->GetMesh()->GetSocketRotation("RightHandSocket")) * mTraceLineDist;

	FHitResult hitResult;
	TArray<AActor*> ignore;
	ignore.Add(GetOwner());

	bool bResult = UKismetSystemLibrary::LineTraceSingle(GetWorld(), socketLocation, targetLocation, UEngineTypes::ConvertToTraceType(ECC_Visibility), true, ignore, EDrawDebugTrace::ForOneFrame, hitResult, true);

	if (bResult == true)
	{
		mHandIKValue.rightHand_Effector = hitResult.ImpactPoint;
		mRightHandGap = mHandIKValue.rightHand_Effector - mOwnerCharacter->GetActorLocation();
	}
}

void UHandIKComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	mHandIKValue.leftHand_Effector = mOwnerCharacter->GetActorLocation() + mLeftHandGap;
	mHandIKValue.rightHand_Effector = mOwnerCharacter->GetActorLocation() + mRightHandGap;

	if (mbDoTraceLeft == true)
	{
		leftHandTraceBySphere();
	}

	if (mbDoTraceRight == true)
	{
		rightHandTraceBySphere();
	}
#if ENABLE_DRAW_DEBUG
    //디버그용 코드
#endif
}

void UHandIKComponent::SetDoTraceLeft(bool result)
{
     mbDoTraceLeft = result;
}

void UHandIKComponent::SetDoTraceRight(bool result)
{
     mbDoTraceRight = result;
}

