// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "HandIKComponent.h"
#include "KCharacterAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTK_API UKCharacterAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
	
public:
    UKCharacterAnimInstance();

	FORCEINLINE void StartGrap(){mbIsGrap = true;}
	void StopGrap();
	FORCEINLINE void StartClimbing(){mbIsClimbing = true;}
	FORCEINLINE void StopClimbing(){mbIsClimbing = false;}
	FORCEINLINE void StartReverseClimbing(){mbIsReverseClimbing = true;}
	FORCEINLINE void StopReverseClimbing(){mbIsReverseClimbing = false;}
	
	void SetIsCanClimbingLedge(bool result);

protected:
    virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float deltaSeconds) override;

private:
    UPROPERTY()
	UHandIKComponent* mHandIKComp;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "IKFoot Value", meta = (AllowPrivateAccess = true))
	FHandIK_Value mHandIKValue;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "HandIK", meta = (AllowPrivateAccess = true))
	bool mbEnableLeftHandIK;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "HandIK", meta = (AllowPrivateAccess = true))
	bool mbEnableRightHandIK;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation", meta = (AllowPrivateAccess = true))
	bool mbIsGrap;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation", meta = (AllowPrivateAccess = true))
	bool mbIsClimbing;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation", meta = (AllowPrivateAccess = true))
	bool mbIsReverseClimbing;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation", meta = (AllowPrivateAccess = true))
	bool mbIsCanClimbingLedge;

};
