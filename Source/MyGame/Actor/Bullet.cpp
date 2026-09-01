// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/Bullet.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"

// Sets default values
ABullet::ABullet()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// 충돌체 컴포넌트
	collisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComp"));
	collisionComp->SetCollisionProfileName(TEXT("BlockAll"));
	collisionComp->SetSphereRadius(13);
	RootComponent = collisionComp;

	// 외관 컴포넌트
	bodyMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BodyMeshComp"));
	bodyMeshComp->SetupAttachment(collisionComp);
	bodyMeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	bodyMeshComp->SetRelativeScale3D(FVector(0.25f));

	// 발사체 컴포넌트
	movementComp = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("MovementComp"));
	movementComp->SetUpdatedComponent(collisionComp);
	movementComp->InitialSpeed = 5000;
	movementComp->MaxSpeed = 5000;
	movementComp->bShouldBounce = true;
	movementComp->Bounciness = 0.3f;

	InitialLifeSpan = 2.0f;
}

// Called when the game starts or when spawned
void ABullet::BeginPlay()
{
	Super::BeginPlay();
	
	FTimerHandle deathTimer;
	//GetWorld()->GetTimerManager().SetTimer(deathTimer, this, &ABullet::Die, 2.0f, false);

	// 람다 사용시, 포인터를 캡처할때는 주의해야한다.
	// 타이머가 울려서 람다가 실행될때, 포인터가 소멸할수도 있다.
	// TObjectPtr<> // 일반적인 포인터
	// 
	// 수정방법 1)
	TWeakObjectPtr<ABullet> WeakBullet = this;

	GetWorld()->GetTimerManager().SetTimer(deathTimer,
		FTimerDelegate::CreateLambda([this]()->void
			{
				Destroy();		
			}), 2.0f, false);

	// 수정방법 2)
	// this 캡처 그대로 사용하고 싶다면, Bullet 삭제될때 TimeManager 에 타이머도 함께 제거
}

// Called every frame
void ABullet::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ABullet::Die()
{
	Destroy();
}

