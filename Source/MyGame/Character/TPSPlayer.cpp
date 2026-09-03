// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/TPSPlayer.h"
#include <GameFramework/SpringArmComponent.h>
#include <Camera/CameraComponent.h>
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"
#include "Actor/Bullet.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "System/EnemyFSM.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Anim/PlayerAnim.h"

#define WEAPON_TRACE  ECC_GameTraceChannel2

// Sets default values
ATPSPlayer::ATPSPlayer()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	ConstructorHelpers::FObjectFinder<USkeletalMesh> TempMesh(TEXT("/Script/Engine.SkeletalMesh'/Game/Assets/Characters/Mannequins/Meshes/SKM_Manny_Simple.SKM_Manny_Simple'"));
	if (TempMesh.Succeeded())
	{
		GetMesh()->SetSkeletalMesh(TempMesh.Object);
		GetMesh()->SetRelativeLocationAndRotation(FVector(0, 0, -90), FRotator(0, 0, -90));
	}

	springArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComp"));
	springArmComp->SetupAttachment(RootComponent);
	springArmComp->SetRelativeLocation(FVector(0, 70, 90));
	//springArmComp->SocketOffset(FVector(0, 70, 90));
	springArmComp->TargetArmLength = 400;
	springArmComp->bUsePawnControlRotation = true;

	tpsCamComp = CreateDefaultSubobject<UCameraComponent>(TEXT("TpsCamComp"));
	tpsCamComp->SetupAttachment(springArmComp);
	tpsCamComp->bUsePawnControlRotation = false;

	bUseControllerRotationYaw = true;

	JumpMaxCount = 2;

	gunMeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("GunMeshComp"));
	gunMeshComp->SetupAttachment(GetMesh(), TEXT("hand_rSocket"));
	ConstructorHelpers::FObjectFinder<USkeletalMesh> TempGunMesh(TEXT("/Script/Engine.SkeletalMesh'/Game/Assets/MilitaryWeapSilver/Weapons/Assault_Rifle_A.Assault_Rifle_A'"));

	if (TempGunMesh.Succeeded())
	{
		gunMeshComp->SetSkeletalMesh(TempGunMesh.Object);
		//gunMeshComp->SetRelativeLocation(FVector(-14, 11, 138));
	}

	sniperGunComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SniperGunComp"));
	sniperGunComp->SetupAttachment(GetMesh(), TEXT("hand_rSocket"));
	ConstructorHelpers::FObjectFinder<USkeletalMesh> TempSniperMesh(TEXT("/Script/Engine.SkeletalMesh'/Game/Assets/MilitaryWeapSilver/Weapons/Sniper_Rifle_A.Sniper_Rifle_A'"));
	
	if (TempSniperMesh.Succeeded())
	{
		sniperGunComp->SetSkeletalMesh(TempSniperMesh.Object);
		//sniperGunComp->SetRelativeLocation(FVector(-22, 31, 128));
	}

	ConstructorHelpers::FObjectFinder<USoundBase> tempSound(TEXT("/Script/Engine.SoundCue'/Game/Assets/MilitaryWeapSilver/Sound/Rifle/Cues/RifleA_Fire_Cue.RifleA_Fire_Cue'"));
	
	if (tempSound.Succeeded())
	{
		bulletSound = tempSound.Object;
	}
}

// Called when the game starts or when spawned
void ATPSPlayer::BeginPlay()
{
	Super::BeginPlay();

	GetCharacterMovement()->MaxWalkSpeed = walkSpeed;

	auto pc = Cast<APlayerController>(Controller);
	if (pc)
	{
		auto subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(pc->GetLocalPlayer());
		if (subsystem)
		{
			subsystem->AddMappingContext(imc_TPS, 0);
		}
	}

	_sniperUI = CreateWidget(GetWorld(), sniperUIFactory);

	sniperGunComp->SetVisibility(false);
}

// Called every frame
void ATPSPlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 카메라 거리, 캐릭터 거리 비교
}

// Called to bind functionality to input
void ATPSPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	auto PlayerInput = CastChecked<UEnhancedInputComponent>(PlayerInputComponent);
	if (PlayerInput)
	{		
		PlayerInput->BindAction(ia_LookUp, ETriggerEvent::Triggered, this, &ATPSPlayer::Input_Look);
		PlayerInput->BindAction(ia_Move, ETriggerEvent::Triggered, this, &ATPSPlayer::Move);
		PlayerInput->BindAction(ia_Jump, ETriggerEvent::Started, this, &ATPSPlayer::InputJump);
		PlayerInput->BindAction(ia_Run, ETriggerEvent::Started, this, &ATPSPlayer::InputRun);
		PlayerInput->BindAction(ia_Run, ETriggerEvent::Completed, this, &ATPSPlayer::InputRun);
		PlayerInput->BindAction(ia_Fire, ETriggerEvent::Started, this, &ATPSPlayer::InputFire);
		PlayerInput->BindAction(ia_Change, ETriggerEvent::Started, this, &ATPSPlayer::ChangeToGun);
		PlayerInput->BindAction(ia_Sniper, ETriggerEvent::Started, this, &ATPSPlayer::SniperAim);
		PlayerInput->BindAction(ia_Sniper, ETriggerEvent::Completed, this, &ATPSPlayer::SniperAim);
	}

}

void ATPSPlayer::Input_Look(const FInputActionValue& inputValue)
{
	const FVector2D Value = inputValue.Get<FVector2D>();

	if (Value.X != 0.0f)
	{
		AddControllerYawInput(Value.X);
	}

	if (Value.Y != 0.0f)
	{
		AddControllerPitchInput(Value.Y);
	}
}

void ATPSPlayer::Move(const FInputActionValue& inputValue)
{
	if (Controller)
	{
		const FVector2D Value = inputValue.Get<FVector2D>();
		const FRotator MovementRotation(0.0f, Controller->GetControlRotation().Yaw, 0.0f);

		if (Value.X != 0.0f)
		{
			const FVector MovementDirection = MovementRotation.RotateVector(FVector::RightVector);
			AddMovementInput(MovementDirection, Value.X);
		}

		if (Value.Y != 0.0f)
		{
			const FVector MovementDirection = MovementRotation.RotateVector(FVector::ForwardVector);
			AddMovementInput(MovementDirection, Value.Y);
		}
	}
}

void ATPSPlayer::InputJump(const FInputActionValue& inputValue)
{
	Jump();
}

void ATPSPlayer::InputRun(const FInputActionValue& inputValue)
{
	auto movement = GetCharacterMovement();

	if (movement->MaxWalkSpeed > walkSpeed)
	{
		movement->MaxWalkSpeed = walkSpeed;
	}
	else
	{
		movement->MaxWalkSpeed = runSpeed;
	}
}

void ATPSPlayer::InputFire(const FInputActionValue& inputValue)
{
	// TSubclassOf : 에디터에서 생성한 블루프린트 클래스와 대응하는 C++ 클래스 타입이 없다.
	// 블루프린트 클래스를 담는 용도로 사용.
	// CDO ( 어떤 클래스인지 틀만 정의 )
	// UClass는 모든 CDO를 보여줘서 불편하고 위험하다. 안전하게 Bullet으로 파생된 클래스만 할당하려면 TSubclassOf
	UGameplayStatics::PlaySound2D(GetWorld(), bulletSound);

	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (PlayerController)
	{
		PlayerController->PlayerCameraManager->StartCameraShake(cameraShake);
	}
	
	auto anim = Cast<UPlayerAnim>(GetMesh()->GetAnimInstance());
	anim->PlayAttackAnim();
	if (GetCharacterMovement()->MaxWalkSpeed > 300)
	{
		anim->SetbShootAndRun(GetCharacterMovement()->MaxWalkSpeed > walkSpeed);
	}
	else
	{
		anim->SetbShootAndRun(false);
	}
	
	
	if (!bUsingSniperGun)
	{
		FTransform firePosition = gunMeshComp->GetSocketTransform(TEXT("FirePosition"));
		// UClass* 는 기본객체 = CDO
		GetWorld()->SpawnActor<ABullet>(bulletFactory, firePosition);
	}
	else
	{
		FVector startPos = tpsCamComp->GetComponentLocation();
		FVector endPos = tpsCamComp->GetComponentLocation() + tpsCamComp->GetForwardVector() * 5000;

		FHitResult hitInfo;
		FCollisionQueryParams params;
		params.AddIgnoredActor(this);

		bool bHit = GetWorld()->LineTraceSingleByChannel(hitInfo, startPos, endPos, WEAPON_TRACE, params);

		if (bHit)
		{
			FTransform bulletTrans;
			bulletTrans.SetLocation(hitInfo.ImpactPoint);
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(
				this,
				BulletEffectFactory,
				hitInfo.ImpactPoint);

			auto hitComp = hitInfo.GetComponent();

			if (hitComp && hitComp->IsSimulatingPhysics())
			{
				FVector dir = (endPos - startPos).GetSafeNormal();
				FVector force = dir * hitComp->GetMass() * 500000;
				hitComp->AddForceAtLocation(force, hitInfo.ImpactPoint);
			}

			auto enemy = hitInfo.GetActor()->GetDefaultSubobjectByName(TEXT("FSM"));
			if (enemy)
			{
				auto enemyFSM = Cast<UEnemyFSM>(enemy);
				enemyFSM->OnDamageProcess();
			}
		}
	}
}

void ATPSPlayer::ChangeToGun(const FInputActionValue& inputValue)
{
	if (!bUsingSniperGun)
	{
		bUsingSniperGun = true;
		sniperGunComp->SetVisibility(true);
		gunMeshComp->SetVisibility(false);
	}
	else
	{
		bUsingSniperGun = false;
		sniperGunComp->SetVisibility(false);
		gunMeshComp->SetVisibility(true);
	}
}

void ATPSPlayer::SniperAim(const FInputActionValue& inputValue)
{
	if (!bUsingSniperGun)
		return;

	if (bSniperAim == false)
	{
		bSniperAim = true;
		_sniperUI->AddToViewport();
		tpsCamComp->SetFieldOfView(30.0f);
		
	}
	else
	{
		bSniperAim = false;
		_sniperUI->RemoveFromParent();
		tpsCamComp->SetFieldOfView(90.0f);
	}
}



