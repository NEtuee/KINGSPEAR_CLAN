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
#include "RopeActor.h"


AKCharacter::AKCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationYaw = false;

	//ĳ���� �����Ʈ ������Ʈ ����
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f,540.0f,0.0f);
	GetCharacterMovement()->JumpZVelocity = 600.0f;
	GetCharacterMovement()->AirControl = 0.2f;
	GetCharacterMovement()->MaxWalkSpeed = 200.0f;

	//ī�޶� �������� ������Ʈ ����
    mCameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	mCameraBoom->SetupAttachment(RootComponent);
	mCameraBoom->SetRelativeLocation(FVector(200.0f, 0.0f, 90.0f));
	mCameraBoom->TargetArmLength = 450.0f;
	mCameraBoom->bUsePawnControlRotation = true;

	//ī�޶� ������Ʈ ����
	mFollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FllowCamera"));
	mFollowCamera->SetupAttachment(mCameraBoom,USpringArmComponent::SocketName);
	mFollowCamera->bUsePawnControlRotation = false;

	//�� IK ������Ʈ 
	mHandIKComponent = CreateDefaultSubobject<UHandIKComponent>(TEXT("HandIK"));

	//�� ������ ĸ��
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
	else
	{
		//���� �յ� �̵�
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
			//ī�޶� ���ͷ� ����
			swingForce = UKismetMathLibrary::GetForwardVector(mFollowCamera->GetComponentRotation()) * mInputVerticalDelta + UKismetMathLibrary::GetRightVector(mFollowCamera->GetComponentRotation()) * mInputHorizonDelta;
			mCurrentOverlappedRope->AddSwingForce(swingForce, 500.0f);
		}
	}
	else
	{
		//���� �¿� �̵�
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
		
		FRotator currentRotation = GetActorRotation();
		currentRotation.Pitch = 0.0f;
		currentRotation.Roll = 0.0f;
		SetActorRotation(currentRotation);

		//mFollowCamera->bUsePawnControlRotation = false;
		//bUseControllerRotationYaw = false;
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

		FRotator currentRotation = GetActorRotation();
		currentRotation.Pitch = 0.0f;
		currentRotation.Roll = 0.0f;
		SetActorRotation(currentRotation);

		//mFollowCamera->bUsePawnControlRotation = false;
		//bUseControllerRotationYaw = false;
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

void AKCharacter::OnCharaterOverlapBegin(class UPrimitiveComponent* overlappedComp, class AActor* otherActor, class UPrimitiveComponent* otherComp, int32 otherBodyIndex, bool bFromSweep, const FHitResult& sweepResult)
{
	ARopeActor* overlapRope = Cast<ARopeActor>(otherActor);
	if (overlapRope != nullptr)
	{
		//UE_LOG(LogTemp,Warning,TEXT("Begin Overlap rope"));
		//ĳ���Ϳ� ���� ���Ϳ��� �浹�̺�Ʈ�� �߻��ϸ� �ش� ������ �������ͷ� �����صӴϴ�.
		//�׸��� bool �� ������ Hanging�� �������� �����մϴ�.
		mbIsCanRopeHanging = true;
		mCurrentOverlappedRope = overlapRope;
	}
}

void AKCharacter::OnCharaterEndOverlap(UPrimitiveComponent* overlappedComponent, AActor* otherActor, UPrimitiveComponent* otherComp, int32 otherBodyIndex)
{
	ARopeActor* overlapRope = Cast<ARopeActor>(otherActor);
	if (overlapRope != nullptr)
	{
	    //ĳ���Ϳ� ������ �浹�̺�Ʈ�� ����ȴٸ� ������ �������͸� null���·� ��ȯ.
	    mbIsCanRopeHanging = false;
	    mCurrentOverlappedRope.Reset();
	}
}

void AKCharacter::GrabRope()
{
	if (mCurrentOverlappedRope.IsValid() && mbIsHanging == false)
	{
	    //mFollowCamera->bUsePawnControlRotation = true;
		//bUseControllerRotationYaw = true;
		mbIsHanging = true;
		mMovementState = MovementState::Hanging;
		FVector currentVelocity = GetCapsuleComponent()->GetPhysicsLinearVelocity();
		GetCapsuleComponent()->AttachToComponent(mCurrentOverlappedRope->GetEndPoint(),mRopeAttachRules);
		SetActorLocation(mCurrentOverlappedRope->GetEndPoint()->GetComponentLocation());
		mCurrentOverlappedRope->AddSwingForce(currentVelocity,200.0f);
	}
}


