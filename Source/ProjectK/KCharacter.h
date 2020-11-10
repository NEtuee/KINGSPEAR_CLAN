// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "KCharacter.generated.h"

UENUM(BlueprintType)
enum class MovementState : uint8
{
	Ground = 0,
	Climbing,
	ReverseClimbing,
	JumpFromWall
};

UCLASS()
class PROJECTK_API AKCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AKCharacter();

public:	
	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void ClimbingMovement(float deltaTime);

	void DetectLedge();

protected:
	virtual void BeginPlay() override;
	virtual void PostInitializeComponents() override;

	void moveForward(float value);
	
	void moveRight(float value);
	
	void startClimbing();
	void releaseClimbing();

	void fromWallJump();

	void setMoveMode(EMovementMode movementMode);

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* mCameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* mFollowCamera;


	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "IK_Hand", meta = (AllowPrivateAccess = "true"))
	class UHandIKComponent* mHandIKComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Detect", meta = (AllowPrivateAccess = "true"))
	UCapsuleComponent* mLedgeDetectCapsule;

	UPROPERTY()
	class UKCharacterAnimInstance* animInstance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State", meta = (AllowPrivateAccess = "true"))
	MovementState mMovementState;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State", meta = (AllowPrivateAccess = "true"))
	bool mbIsGrap;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State", meta = (AllowPrivateAccess = "true"))
	bool mbIsMustClimbing;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WallCheck", meta = (AllowPrivateAccess = "true"))
	float mClimbingCheckRange;

	FAttachmentTransformRules mAttachRules = FAttachmentTransformRules(EAttachmentRule::KeepWorld, EAttachmentRule::KeepWorld, EAttachmentRule::KeepWorld, true);
	FDetachmentTransformRules mDettachRules = FDetachmentTransformRules(EDetachmentRule::KeepWorld, EDetachmentRule::KeepWorld, EDetachmentRule::KeepWorld, false);

	float mInputVerticalValue;//W,S 키 입력값
	float mInputHorizonValue;//A,D 키 입력값
};
