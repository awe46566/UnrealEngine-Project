// Fill out your copyright notice in the Description page of Project Settings.


#include "Anim/PlayerAnim.h"
#include "Character/TPSPlayer.h"
#include "GameFramework/CharacterMovementComponent.h"

void UPlayerAnim::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	// 초기화 될때 캐싱해두기
	Character = Cast<ATPSPlayer>(TryGetPawnOwner());
	if (Character != nullptr)
	{
		MovementComponent = Character->GetCharacterMovement();
	}
}

void UPlayerAnim::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	
	if (Character && MovementComponent)
	{
		// 속도 계산
		Velocity = Character->GetVelocity();
		GroundSpeed = Velocity.Length();
		bShouldMove = (MovementComponent->GetCurrentAcceleration().Size() > 0) && (GroundSpeed >= 0.01f);
		bIsFalling = MovementComponent->IsFalling();

		// 방향 계산
		Direction = CalculateDirection(Velocity, Character->GetActorRotation());
	}
}

void UPlayerAnim::PlayAttackAnim()
{
	Montage_Play(attackAnimMontage);
}

void UPlayerAnim::SetbShootAndRun(bool bNewValue)
{
	bShouldShootAndRun = bNewValue;
}
