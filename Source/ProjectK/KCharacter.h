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
	JumpFromWall,
	Hanging
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

	void AimStart();
	void AimRelease();

	void RopeShot();

	void SpearShot();

private:
	UFUNCTION()
	void OnCharaterOverlapBegin(class UPrimitiveComponent* overlappedComp, class AActor* otherActor, class UPrimitiveComponent* otherComp, int32 otherBodyIndex, bool bFromSweep, const FHitResult& sweepResult);

	UFUNCTION()
	void OnCharaterEndOverlap(UPrimitiveComponent* overlappedComponent, AActor* otherActor, UPrimitiveComponent* otherComp, int32 otherBodyIndex);

	void GrabRope();

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* mCameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* mFollowCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UArrowComponent* mLaunchPos;

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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State", meta = (AllowPrivateAccess = "true"))
	bool mbIsAim;

	FAttachmentTransformRules mAttachRules = FAttachmentTransformRules(EAttachmentRule::KeepWorld, EAttachmentRule::KeepWorld, EAttachmentRule::KeepWorld, true);
	FDetachmentTransformRules mDettachRules = FDetachmentTransformRules(EDetachmentRule::KeepWorld, EDetachmentRule::KeepWorld, EDetachmentRule::KeepWorld, false);
	FAttachmentTransformRules mRopeAttachRules = FAttachmentTransformRules(EAttachmentRule::KeepWorld,EAttachmentRule::KeepWorld,EAttachmentRule::KeepWorld,true);
	FDetachmentTransformRules mRopeDettachRules = FDetachmentTransformRules(EDetachmentRule::KeepWorld, EDetachmentRule::KeepWorld, EDetachmentRule::KeepWorld, true);


	float mInputVerticalValue;//W,S Ű �Է°�
	float mInputHorizonValue;//A,D Ű �Է°�
	float mInputVerticalDelta;//W,S Ű �Է� ������ �� ��
	float mInputHorizonDelta;//A,D Ű �Է� ������ �� ��

	bool mbIsCanRopeHanging;//hanging ���� ����
	TWeakObjectPtr<class ARopeActor> mCurrentOverlappedRope;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State", meta = (AllowPrivateAccess = "true"))
	bool mbIsHanging;//hanging ����

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State", meta = (AllowPrivateAccess = "true"))
	bool mbIsHangingToClimbingRope;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State", meta = (AllowPrivateAccess = "true"))
	bool mbIsHangingToShotRope;//ĳ���Ͱ� �� ������ �Ŵ޷��ִ��� ����

	UClass* mGenerateShotRope;

	UClass* mGenerateSpear;

	TWeakObjectPtr<class ARopeActor_Climbing> mCurrentClimbingRope;
	//class ARopeActor_Climbing* mCurrentClimbingRope;
};
