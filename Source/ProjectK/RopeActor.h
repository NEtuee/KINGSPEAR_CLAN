// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RopeActor.generated.h"

UCLASS()
class PROJECTK_API ARopeActor : public AActor
{
	GENERATED_BODY()
	
public:	
	ARopeActor();

	virtual void Tick(float DeltaTime) override;

	void AddSwingForce(FVector force,float multiple);

	void DestroyRope();

    UStaticMeshComponent* GetEndPoint() const { return mEndPoint;}

	FVector GetRopeVelocity() const;

    FRotator GetEndPointRotation() const { return mEndPoint->GetComponentRotation(); }

	void SetEndPointLocation(FVector location);

	void SetCableLength(float length);

	void ReSetConstrained();

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "StartPoint", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* mStartPoint;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cable", meta = (AllowPrivateAccess = "true"))
	class UCableComponent* mCable;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "EndPoint", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* mEndPoint;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "EndPoint", meta = (AllowPrivateAccess = "true"))
	class UCapsuleComponent* mEndPointCollision;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PhysicsConstraint", meta = (AllowPrivateAccess = "true"))
	class UPhysicsConstraintComponent* mPhysicsConstraintComponent;

};
