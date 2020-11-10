// Fill out your copyright notice in the Description page of Project Settings.


#include "KCharacterAnimInstance.h"

UKCharacterAnimInstance::UKCharacterAnimInstance()
{

}

void UKCharacterAnimInstance::StopGrap()
{
    mbIsGrap = false;
    mbEnableLeftHandIK = false;
    mbEnableRightHandIK = false;
}

void UKCharacterAnimInstance::SetIsCanClimbingLedge(bool result)
{
    mbIsCanClimbingLedge = result;
}

void UKCharacterAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();

    APawn* owner = TryGetPawnOwner();
    if (owner != nullptr)
    {
        UActorComponent* actorComp = owner->GetComponentByClass(UHandIKComponent::StaticClass());
        check(actorComp != nullptr);
        mHandIKComp = Cast<UHandIKComponent>(actorComp);
        check(mHandIKComp != nullptr);
    }

    mbEnableLeftHandIK = false;
    mbEnableRightHandIK = false;
}

void UKCharacterAnimInstance::NativeUpdateAnimation(float deltaSeconds)
{
     Super::NativeUpdateAnimation(deltaSeconds);

     //매 프레임마다 손IK 컴포넌트한테 받아서 갱신
     if(mHandIKComp !=nullptr)
     { 
     mHandIKValue = mHandIKComp->GetHandIKValue();
     }
}
