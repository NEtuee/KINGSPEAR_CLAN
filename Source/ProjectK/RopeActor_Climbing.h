// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RopeActor_Climbing.generated.h"

UCLASS()
class PROJECTK_API ARopeActor_Climbing : public AActor
{
	GENERATED_BODY()
	
public:	
	ARopeActor_Climbing();

	virtual void Tick(float DeltaTime) override;

	void UpdateRope(float length);

	void ClimbingRope(float direction, float speed);

	void AddSwingForce(FVector force, float multiple);

	FVector GetStartPoint() const;

	UStaticMeshComponent* GetMiddlePoint() const;

	FVector GetRopeVelocity() const;

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "StartPoint", meta = (AllowPrivateAccess = "true"))
    UStaticMeshComponent* mStartPoint;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PhysicsConstraint", meta = (AllowPrivateAccess = "true"))
	class UPhysicsConstraintComponent* mPhysicsConstraintComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MidlePoint", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* mMiddlePoint;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cable", meta = (AllowPrivateAccess = "true"))
	class UCableComponent* mEndCable;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BoxCollision", meta = (AllowPrivateAccess = "true"))
	class UBoxComponent* mBoxCollision;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cable", meta = (AllowPrivateAccess = "true"))
	class UCableComponent* mMiddleCable;

	float mCableLength = 500.0f;
	float mCurrentCableLength;
	float mLengthLimit = 50.0f;
};
