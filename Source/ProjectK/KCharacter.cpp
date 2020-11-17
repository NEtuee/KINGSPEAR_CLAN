// Fill out your copyright notice in the Description page of Project Settings.


#include "KCharacter.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "KCharacterAnimInstance.h"
#include "HandIKComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "DrawDebugHelpers.h"
#include "kismet/GameplayStatics.h"
#include "RopeActor.h"
#include "Components/ArrowComponent.h"
#include "SpearActor.h"
#include "RopeActor_Climbing.h"


AKCharacter::AKCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationYaw = false;

	//캐릭터 무브먼트 컴포넌트 설정
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f,540.0f,0.0f);
	GetCharacterMovement()->JumpZVelocity = 600.0f;
	GetCharacterMovement()->AirControl = 0.2f;
	GetCharacterMovement()->MaxWalkSpeed = 200.0f;

	//카메라 스프링암 컴포넌트 설정
    mCameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	mCameraBoom->SetupAttachment(RootComponent);
	mCameraBoom->SetRelativeLocation(FVector(200.0f, 0.0f, 90.0f));
	mCameraBoom->TargetArmLength = 450.0f;
	mCameraBoom->bUsePawnControlRotation = true;

	//카메라 컴포넌트 설정
	mFollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FllowCamera"));
	mFollowCamera->SetupAttachment(mCameraBoom,USpringArmComponent::SocketName);
	mFollowCamera->bUsePawnControlRotation = false;

	// 발사 지점 설정
	mLaunchPos = CreateDefaultSubobject<UArrowComponent>(TEXT("LaunchPos"));
	mLaunchPos->SetupAttachment(mFollowCamera);
	mLaunchPos->SetRelativeLocation(FVector(360.0f,0.0f,0.0f));

	//손 IK 컴포넌트 
	mHandIKComponent = CreateDefaultSubobject<UHandIKComponent>(TEXT("HandIK"));

	//턱 감지용 캡슐
	mLedgeDetectCapsule = CreateDefaultSubobject<UCapsuleComponent>(TEXT("LedgeDetect"));
	mLedgeDetectCapsule->SetCapsuleRadius(GetCapsuleComponent()->GetScaledCapsuleRadius());
	mLedgeDetectCapsule->SetCapsuleHalfHeight(GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight());
	mLedgeDetectCapsule->SetupAttachment(RootComponent);
	mLedgeDetectCapsule->SetRelativeLocation(FVector(40.0f, 0.0f, 205.0f));
	mLedgeDetectCapsule->SetCollisionProfileName(TEXT("DetectLedge"));
	mLedgeDetectCapsule->SetGenerateOverlapEvents(true);

	mClimbingCheckRange = 30.0f;
}

void AKCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	mbIsMustClimbing = false;

	mMovementState = MovementState::Ground;

	GetCapsuleComponent()->OnComponentBeginOverlap.AddDynamic(this, &AKCharacter::OnCharaterOverlapBegin);
	GetCapsuleComponent()->OnComponentEndOverlap.AddDynamic(this, &AKCharacter::OnCharaterEndOverlap);

	FName path = TEXT("/Game/Blueprints/BP_RopeActor.BP_RopeActor_C");
	mGenerateShotRope = Cast<UClass>(StaticLoadObject(UClass::StaticClass(),NULL,*path.ToString()));

	path = TEXT("/Game/Blueprints/SpearActor.SpearActor_C");
	mGenerateSpear = Cast<UClass>(StaticLoadObject(UClass::StaticClass(),NULL,*path.ToString()));
}

void AKCharacter::PostInitializeComponents()
{
   Super::PostInitializeComponents();
   animInstance = Cast<UKCharacterAnimInstance>(GetMesh()->GetAnimInstance());
   //check(animInstance != nullptr);
}

void AKCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	ClimbingMovement(DeltaTime);

	DetectLedge();

	if (mbIsHanging == true)
	{
		if (mCurrentOverlappedRope.IsValid())
		{
		    SetActorRotation(mCurrentOverlappedRope->GetEndPointRotation());
		}
	}

	if (mbIsAim == true)
	{
	    mCameraBoom->SocketOffset = UKismetMathLibrary::VLerp(mCameraBoom->SocketOffset,FVector(250.0f,100.0f,0.0f),0.2f);
	}
	else
	{
		mCameraBoom->SocketOffset = UKismetMathLibrary::VLerp(mCameraBoom->SocketOffset, FVector(0.0f, 0.0f, 0.0f), 0.2f);
	}
}

void AKCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released,this,&ACharacter::StopJumping);

	PlayerInputComponent->BindAxis("MoveForward",this,&AKCharacter::moveForward);
	PlayerInputComponent->BindAxis("MoveRight",this,&AKCharacter::moveRight);

	PlayerInputComponent->BindAxis("Turn",this,&APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("LookUp",this, &APawn::AddControllerPitchInput);

	PlayerInputComponent->BindAction("Climbing",IE_Pressed,this,&AKCharacter::startClimbing);
	PlayerInputComponent->BindAction("Climbing", IE_Released, this, &AKCharacter::releaseClimbing);

	PlayerInputComponent->BindAction("Jump",IE_Pressed,this,&ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump",IE_Released,this,&ACharacter::StopJumping);
	PlayerInputComponent->BindAction("Jump",IE_Pressed,this,&AKCharacter::fromWallJump);

	PlayerInputComponent->BindAction("Aim",IE_Pressed,this,&AKCharacter::AimStart);
	PlayerInputComponent->BindAction("Aim",IE_Released,this,&AKCharacter::AimRelease);

	PlayerInputComponent->BindAction("RopeShot",IE_Pressed,this,&AKCharacter::RopeShot);

	PlayerInputComponent->BindAction("Shot",IE_Pressed,this,&AKCharacter::SpearShot);
}

void AKCharacter::ClimbingMovement(float deltaTime)
{
	if (mbIsMustClimbing == true)
	{
		FHitResult hitResult;
		FVector climbingDir;

		TArray<AActor*> ignore;
		ignore.Add(GetOwner());

		FVector start = GetActorLocation();
		FVector target = GetActorLocation() + GetActorForwardVector() * 200.0f;
		bool bResult = UKismetSystemLibrary::LineTraceSingle(GetWorld(), start, target, UEngineTypes::ConvertToTraceType(ECC_Visibility), true, ignore, EDrawDebugTrace::None, hitResult, true);

		if (bResult == true)
		{
			float value = (mMovementState == MovementState::Climbing) ? 1.0f : -1.0f;

			FVector hitNormal = hitResult.ImpactNormal;
			//UE_LOG(LogTemp, Warning, TEXT("%f , %f , %f"), hitNormal.X, hitNormal.Y, hitNormal.Z);
			FVector verticalClimbingVector = FRotationMatrix(hitNormal.Rotation()).GetUnitAxis(EAxis::Z) * mInputVerticalValue;
			FVector horizonClimbingVector = FRotationMatrix(hitNormal.Rotation()).GetUnitAxis(EAxis::Y) * mInputHorizonValue * -1.0f;
			climbingDir = (verticalClimbingVector + horizonClimbingVector);
			climbingDir.Normalize();
			FRotator currentRotation = GetActorRotation();
			FRotator slopeRotation = hitNormal.Rotation();
			float pitch = currentRotation.Pitch;
			pitch = UKismetMathLibrary::Lerp(pitch, -slopeRotation.Pitch, 10.0f * deltaTime);
			currentRotation.Pitch = pitch;
			//currentRotation.Yaw = -slopeRotation.Yaw;
			SetActorRotation(currentRotation);

			/*FVector hitPoint = hitResult.ImpactPoint;
			SetActorLocation(hitPoint + (FRotationMatrix(hitNormal.Rotation()).GetUnitAxis(
			EAxis::X)*25.0f));*/
			//UE_LOG(LogTemp,Warning,TEXT("Dist From Wall : %f"),FVector::Distance(hitResult.ImpactPoint,GetActorLocation()));
		}

		FVector currentLocation = GetActorLocation();
		currentLocation += climbingDir * 50.0f * deltaTime;
		SetActorLocation(currentLocation);


		DrawDebugCapsule(GetWorld(),
			mLedgeDetectCapsule->GetComponentLocation(),
			mLedgeDetectCapsule->GetScaledCapsuleHalfHeight(),
			mLedgeDetectCapsule->GetScaledCapsuleRadius(),
			mLedgeDetectCapsule->GetComponentRotation().Quaternion(),
			FColor::Red,
			false,
			0.01f);
	}
}

void AKCharacter::DetectLedge()
{
	if (mMovementState == MovementState::Climbing)
	{
		TArray<AActor*> overlappedActors;
		TSubclassOf<AActor> filter;
		mLedgeDetectCapsule->GetOverlappingActors(overlappedActors,filter);
		if (overlappedActors.Num() > 0)
		{
			animInstance->SetIsCanClimbingLedge(false);
		}
		else
		{
		    animInstance->SetIsCanClimbingLedge(true);
		}
	}

	mLedgeDetectCapsule->SetWorldRotation(FRotator::ZeroRotator);
}


void AKCharacter::moveForward(float value)
{
    const FRotator rotation = Controller->GetControlRotation();
	const FRotator yawRotation(0,rotation.Yaw,0);
	FVector direction;

	mInputVerticalDelta = value;

	if (mbIsGrap == true)
	{
		if (mbIsMustClimbing == false)
		{
			if (value == 0.0f)
			{
			    mInputVerticalValue = 0.0f;
				animInstance->StopClimbing();
				animInstance->StopReverseClimbing();
			}
			else if (value >= 0.0f)
			{
			    mInputVerticalValue = 1.0f;
				animInstance->StartClimbing();
				animInstance->StopReverseClimbing();
				mMovementState = MovementState::Climbing;
			}
			else
			{
			    mInputVerticalValue = -1.0f;
				animInstance->StopClimbing();
				animInstance->StartClimbing();
				mMovementState = MovementState::ReverseClimbing;
			}
		}
	}
	else if (mbIsHanging)
	{
		if (mCurrentOverlappedRope.IsValid())
		{   
		    FVector swingForce;
			FRotator targetRot = FRotator(0.0f, GetActorRotation().Yaw, 0.0f);
			/*swingForce = UKismetMathLibrary::GetForwardVector(targetRot)*mInputVerticalDelta + UKismetMathLibrary::GetRightVector(targetRot)*mInputHorizonDelta;*/
			swingForce = UKismetMathLibrary::GetForwardVector(mFollowCamera->GetComponentRotation())*mInputVerticalDelta + UKismetMathLibrary::GetRightVector(mFollowCamera->GetComponentRotation())*mInputHorizonDelta;
			mCurrentOverlappedRope->AddSwingForce(swingForce,500.0f);
		}
	}
	else if(mbIsHangingToClimbingRope == true)
	{

		if (mCurrentClimbingRope.IsValid())
		{
		   //UE_LOG(LogTemp,Warning,TEXT("ddd"));
		   mCurrentClimbingRope->ClimbingRope(value, 200.0f);
		}
	}
	else
	{
		//지상 앞뒤 이동
		if ((Controller != nullptr) && (value != 0.0f))
		{
			direction = FRotationMatrix(yawRotation).GetUnitAxis(EAxis::X);
			AddMovementInput(direction, value);
		}
	}
}

void AKCharacter::moveRight(float value)
{
    mInputHorizonDelta = value;

	if (mbIsGrap == true)
	{
		if (mInputVerticalValue == 0.0f && value != 0.0f)
		{
			animInstance->StartClimbing();
			animInstance->StopReverseClimbing();
			mMovementState=MovementState::Climbing;
		}

		if (mbIsMustClimbing == false)
		{
			if (value > 0.0f)
			{
			    mInputHorizonValue = UKismetMathLibrary::FCeil(value);
			}
			else
			{
			    mInputHorizonValue = UKismetMathLibrary::FFloor(value);
			}
		}
	}
	else if (mbIsHanging == true)
	{
		if (mCurrentOverlappedRope.IsValid())
		{
			FVector swingForce;
			FRotator targetRot = FRotator(0.0f, GetActorRotation().Yaw, 0.0f);
			/*swingForce = UKismetMathLibrary::GetForwardVector(targetRot) * mInputVerticalDelta + UKismetMathLibrary::GetRightVector(targetRot) * mInputHorizonDelta;*/
			//카메라 벡터로 스윙
			swingForce = UKismetMathLibrary::GetForwardVector(mFollowCamera->GetComponentRotation()) * mInputVerticalDelta + UKismetMathLibrary::GetRightVector(mFollowCamera->GetComponentRotation()) * mInputHorizonDelta;
			mCurrentOverlappedRope->AddSwingForce(swingForce, 500.0f);
		}
	}
	else
	{
		//지상 좌우 이동
		if ((Controller != NULL) && (value != 0.0f))
		{
			const FRotator rotation = Controller->GetControlRotation();
			const FRotator yawRotation(0, rotation.Yaw, 0);

			const FVector direction = FRotationMatrix(yawRotation).GetUnitAxis(EAxis::Y);
			AddMovementInput(direction, value);
		}
	}
}

void AKCharacter::startClimbing()
{
	if (mbIsCanRopeHanging == true)
	{
		GrabRope();
		return;
	}

	FHitResult hitResult;
	FCollisionQueryParams params(NAME_None, false, this);
	bool bResult = GetWorld()->SweepSingleByChannel(
		hitResult,
		GetActorLocation(),
		GetActorLocation() + GetActorForwardVector() * mClimbingCheckRange,
		FQuat::Identity,
		ECollisionChannel::ECC_GameTraceChannel2,
		FCollisionShape::MakeSphere(50.0f),
		params);

	if (bResult == true)
	{
	    animInstance->StartGrap();
		setMoveMode(EMovementMode::MOVE_Flying);

		GetRootComponent()->AttachToComponent(hitResult.Actor->GetRootComponent(),mAttachRules);

		FVector hitPoint = hitResult.ImpactPoint;
		FVector hitNormal = hitResult.ImpactNormal;
        FRotator currentRotation = GetActorRotation();
		currentRotation.Yaw = hitNormal.Rotation().Yaw + 180.0f;
		currentRotation.Pitch = -hitNormal.Rotation().Pitch;
		SetActorRotation(currentRotation);
		SetActorLocation(hitPoint + (FRotationMatrix(hitNormal.Rotation()).GetUnitAxis(
			EAxis::X) * 25.0f));

		mbIsGrap = true;
		mMovementState = MovementState::Climbing;
	}
}

void AKCharacter::releaseClimbing()
{
    if (mbIsHangingToShotRope == true)
    {
	    return;
    }

	if (mbIsGrap == true)
	{
	    GetRootComponent()->DetachFromComponent(mDettachRules);

		animInstance->StopGrap();
		animInstance->StopClimbing();
		mbIsGrap = false;
		setMoveMode(EMovementMode::MOVE_Walking);

		FRotator currentRotation = GetActorRotation();
		if (currentRotation.Pitch != 0.0f)
		{
			currentRotation.Pitch = 0.0f;
			SetActorRotation(currentRotation);
		}
	}

	if (mbIsHanging == true)
	{
		GetRootComponent()->DetachFromComponent(mRopeDettachRules);
		mbIsHanging = false;
		//UE_LOG(LogTemp, Warning, TEXT("releaseClimbing"));

		FRotator currentRotation = GetActorRotation();
		currentRotation.Pitch = 0.0f;
		currentRotation.Roll = 0.0f;
		SetActorRotation(currentRotation);

		//mFollowCamera->bUsePawnControlRotation = false;
		//bUseControllerRotationYaw = false;
	}

	if (mbIsHangingToClimbingRope == true)
	{
		return;
	}

	

	mMovementState = MovementState::Ground;
}

void AKCharacter::fromWallJump()
{
	if (mbIsGrap)
	{
		setMoveMode(MOVE_Walking);

		FRotator currentRotation = GetActorRotation();
		currentRotation.Pitch = 0.0f;
		currentRotation.Yaw = currentRotation.Yaw + 180.0f;
		SetActorRotation(currentRotation);
		GetCharacterMovement()->Velocity = FVector::ZeroVector;

		FVector forward = FRotationMatrix(GetActorRotation()).GetUnitAxis(EAxis::X);
		FVector up = FRotationMatrix(GetActorRotation()).GetUnitAxis(EAxis::Z);
		FVector finalDirection = (forward + up) * 500.0f;
		LaunchCharacter(finalDirection,false,false);

		mbIsMustClimbing = false;
		mbIsGrap = false;
		animInstance->StopClimbing();
		animInstance->StopReverseClimbing();
		animInstance->StopGrap();

		mMovementState = MovementState::Ground;
	}
	else if (mbIsHanging == true)
	{
		GetRootComponent()->DetachFromComponent(mRopeDettachRules);
		FRotator targetRot = UKismetMathLibrary::MakeRotFromX(GetVelocity());
		SetActorRotation(targetRot);
		FVector launchForce;
		if (mCurrentOverlappedRope.IsValid())
		{
			launchForce = mCurrentOverlappedRope->GetRopeVelocity();
		}
		else
		{
			return;
		}

		launchForce.Z = 1000.0f;
		LaunchCharacter(launchForce,true,true);

		mbIsHanging = false;
		mMovementState = MovementState::Ground;
		//UE_LOG(LogTemp, Warning, TEXT("Jump"));


		FRotator currentRotation = GetActorRotation();
		currentRotation.Pitch = 0.0f;
		currentRotation.Roll = 0.0f;
		SetActorRotation(currentRotation);

		if (mbIsHangingToShotRope == true)
		{
		   if (mCurrentOverlappedRope.IsValid())
		   {
		       mCurrentOverlappedRope->DestroyRope();
		   }
			mbIsHangingToShotRope = false;
		}

		//mFollowCamera->bUsePawnControlRotation = false;
		//bUseControllerRotationYaw = false;
	}
	else if (mbIsHangingToClimbingRope == true)
	{
		GetRootComponent()->DetachFromComponent(mRopeDettachRules);
		FRotator targetRot = UKismetMathLibrary::MakeRotFromX(GetVelocity());
		SetActorRotation(targetRot);
		FVector launchForce;
		/*if (mCurrentClimbingRope.IsValid())
		{
			launchForce = mCurrentClimbingRope->GetRopeVelocity();
		}
		else
		{
			return;
		}*/

		launchForce.Z = 1000.0f;
		LaunchCharacter(launchForce, true, true);

		mbIsHangingToClimbingRope = false;
		mMovementState = MovementState::Ground;
		UE_LOG(LogTemp, Warning, TEXT("Jump"));


		FRotator currentRotation = GetActorRotation();
		currentRotation.Pitch = 0.0f;
		currentRotation.Roll = 0.0f;
		SetActorRotation(currentRotation);
	}

}

void AKCharacter::setMoveMode(EMovementMode movementMode)
{
	switch (movementMode)
	{
	case MOVE_Walking:
		GetCharacterMovement()->MovementMode = EMovementMode::MOVE_Walking;
		GetCharacterMovement()->GravityScale = 1.0f;
		break;
	case MOVE_Flying:
		GetCharacterMovement()->MovementMode = EMovementMode::MOVE_Flying;
		GetCharacterMovement()->GravityScale = 0.0f;
		GetCharacterMovement()->BrakingDecelerationFlying = GetCharacterMovement()->MaxFlySpeed;
		break;
		default:
			GetCharacterMovement()->MovementMode = EMovementMode::MOVE_Walking;
			GetCharacterMovement()->GravityScale = 1.0f;
		break;
	}
}

void AKCharacter::AimStart()
{
     mbIsAim = true;

	 UGameplayStatics::SetGlobalTimeDilation(GetWorld(),0.2f);
}
	 
void AKCharacter::AimRelease()
{
     mbIsAim = false;
	 UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 1.0f);

}

void AKCharacter::RopeShot()
{
	if (mbIsAim == true && mbIsHanging == false)
	{
	    FVector startPos = mFollowCamera->GetComponentLocation();
		FVector endPos = startPos + UKismetMathLibrary::GetForwardVector(mFollowCamera->GetComponentRotation())*10000.0f;

		FHitResult hitResult;
		TArray<AActor*> ignore;
		ignore.Add(GetOwner());

		bool bResult = UKismetSystemLibrary::LineTraceSingle(GetWorld(),
		startPos,
		endPos, UEngineTypes::ConvertToTraceType(ECC_Visibility),
		false,ignore,EDrawDebugTrace::ForDuration,
		hitResult,true);

		if (bResult == true)
		{
		    FActorSpawnParameters spawnParams;
			//spawnParams.Owner = this;
			mCurrentOverlappedRope = GetWorld()->SpawnActor<ARopeActor>(mGenerateShotRope,hitResult.ImpactPoint,FRotator::ZeroRotator);

			if (mCurrentOverlappedRope.IsValid())
			{
				mCurrentOverlappedRope->SetEndPointLocation(GetActorLocation());
				float distance = FVector::Distance(hitResult.ImpactPoint,GetActorLocation());
				mCurrentOverlappedRope->SetCableLength(distance*0.5f);
				mbIsHangingToShotRope = true;
				GrabRope();
			}
		}
	}
}

void AKCharacter::SpearShot()
{
	if (mbIsAim == true)
	{
	    GetWorld()->SpawnActor<ASpearActor>(mGenerateSpear,mLaunchPos->GetComponentLocation(),mLaunchPos->GetComponentRotation());

		UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 1.0f);
	}
}

void AKCharacter::OnCharaterOverlapBegin(class UPrimitiveComponent* overlappedComp, class AActor* otherActor, class UPrimitiveComponent* otherComp, int32 otherBodyIndex, bool bFromSweep, const FHitResult& sweepResult)
{
	ARopeActor* overlapRope = Cast<ARopeActor>(otherActor);
	if (overlapRope != nullptr)
	{
		//UE_LOG(LogTemp,Warning,TEXT("Begin Overlap rope"));
		//캐릭터에 로프 액터와의 충돌이벤트가 발생하면 해당 로프를 약포인터로 저장해둡니다.
		//그리고 bool 형 변수로 Hanging이 가능함을 저장합니다.
		mbIsCanRopeHanging = true;
		mCurrentOverlappedRope = overlapRope;
	}

	ARopeActor_Climbing* overlapClimbingRope = Cast<ARopeActor_Climbing>(otherActor);
	if (overlapClimbingRope != nullptr)
	{
		mbIsCanRopeHanging = true;
		mCurrentClimbingRope = overlapClimbingRope;
		//UE_LOG(LogTemp, Warning, TEXT("Begine"));
	}
	
}

void AKCharacter::OnCharaterEndOverlap(UPrimitiveComponent* overlappedComponent, AActor* otherActor, UPrimitiveComponent* otherComp, int32 otherBodyIndex)
{
	ARopeActor* overlapRope = Cast<ARopeActor>(otherActor);
	if (overlapRope != nullptr)
	{
	    //캐릭터와 로프의 충돌이벤트가 종료된다면 로프의 약포인터를 null상태로 전환.
	    mbIsCanRopeHanging = false;
	    mCurrentOverlappedRope.Reset();
	}

	ARopeActor_Climbing* overlapClimbingRope = Cast<ARopeActor_Climbing>(otherActor);
	if (overlapClimbingRope != nullptr)
	{
		mbIsCanRopeHanging = false;
		mCurrentClimbingRope.Reset();
		//UE_LOG(LogTemp, Warning, TEXT("End"));
	}
}

void AKCharacter::GrabRope()
{
	if (mCurrentOverlappedRope.IsValid() && mbIsHanging == false)
	{
	    UE_LOG(LogTemp,Warning,TEXT("GrapRope"));
	    //mFollowCamera->bUsePawnControlRotation = true;
		//bUseControllerRotationYaw = true;
		mbIsHanging = true;
		mMovementState = MovementState::Hanging;
		FVector currentVelocity = GetCapsuleComponent()->GetPhysicsLinearVelocity();
		GetCapsuleComponent()->AttachToComponent(mCurrentOverlappedRope->GetEndPoint(),mRopeAttachRules);
		SetActorLocation(mCurrentOverlappedRope->GetEndPoint()->GetComponentLocation());
		mCurrentOverlappedRope->AddSwingForce(currentVelocity,200.0f);
	}

	if (mCurrentClimbingRope.IsValid() && mbIsHangingToClimbingRope == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("ClimbingGrapRope"));
		TWeakObjectPtr<ARopeActor_Climbing> tempPtr = mCurrentClimbingRope;

		mbIsHangingToClimbingRope = true;
		mMovementState = MovementState::Hanging;
		FVector ropeStartLocation = tempPtr->GetStartPoint();
		tempPtr->UpdateRope(FVector::Distance(ropeStartLocation, GetActorLocation()));
		FVector currentVelocity = GetCapsuleComponent()->GetPhysicsLinearVelocity();
		UStaticMeshComponent* temp = tempPtr->GetMiddlePoint();
		GetCapsuleComponent()->AttachToComponent(tempPtr->GetMiddlePoint(), FAttachmentTransformRules(EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::KeepWorld, true));
		mCurrentClimbingRope->AddSwingForce(currentVelocity, 100.0f);
		UE_LOG(LogTemp, Warning, TEXT("%f"), FVector::Distance(ropeStartLocation, GetActorLocation()));
		
	}
}




