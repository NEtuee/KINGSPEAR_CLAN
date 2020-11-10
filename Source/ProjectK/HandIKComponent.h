// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HandIKComponent.generated.h"


USTRUCT(Atomic, BlueprintType)
struct FHandIK_Value
{
    GENERATED_BODY()
	public:
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	FVector leftHand_Effector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector rightHand_Effector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FRotator leftHand_Rotation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FRotator rightHand_Rotation;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECTK_API UHandIKComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UHandIKComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void SetDoTraceLeft(bool result);
	void SetDoTraceRight(bool result);

	FORCEINLINE FHandIK_Value GetHandIKValue() const { return mHandIKValue; }

protected:
	virtual void BeginPlay() override;

private:
    void leftHandTraceBySphere();

	void rightHandTraceBySphere();

	void leftHandTraceByLine();

	void rightHandTraceByLine();

private:
    UPROPERTY()
	class AKCharacter* mOwnerCharacter;
    
	FHandIK_Value mHandIKValue;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "HandIK", meta = (AllowPrivateAccess = "true"))
	float mTraceSphereRaduis;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "HandIK", meta = (AllowPrivateAccess = "true"))
	float mTraceLineDist;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "HandIK", meta = (AllowPrivateAccess = "true"))
	bool mbDoTraceLeft;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "HandIK", meta = (AllowPrivateAccess = "true"))
	bool mbDoTraceRight;

	FVector mLeftHandGap;//왼손 IK 위치와 캐릭터 위치의 차이 (매 프레임 갱신)
	FVector mRightHandGap;//오른손 IK 위치와 캐릭터 위치의 차이 (매 프레임 갱신)
		
};
