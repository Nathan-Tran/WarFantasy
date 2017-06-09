// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "WarFantasy.h"
#include "WarFantasyCharacter.h"
#include "WarFantasyProjectile.h" //TODO remove
#include "Animation/AnimInstance.h"
#include "GameFramework/InputSettings.h"

DEFINE_LOG_CATEGORY_STATIC(LogFPChar, Warning, All);

//////////////////////////////////////////////////////////////////////////
// AWarFantasyCharacter

AWarFantasyCharacter::AWarFantasyCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(40.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;				//Appears to be for controller support
	BaseLookUpRate = 45.f;				//Appears to be for controller support

	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->RelativeLocation = FVector(0.f, 0.f, 64.f); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	Mesh1P->SetOnlyOwnerSee(true);
	Mesh1P->SetupAttachment(FirstPersonCameraComponent);
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->CastShadow = false;
	Mesh1P->RelativeRotation = FRotator(0.f, -90.f, 0.f);  //TODO shrink and reposition gun
	Mesh1P->RelativeLocation = FVector(0.f, 0.f, -165.5f);

	// Create a gun mesh component
	FP_Gun = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FP_Gun"));
	FP_Gun->SetOnlyOwnerSee(true);			// only the owning player will see this mesh
	FP_Gun->bCastDynamicShadow = false;
	FP_Gun->CastShadow = false;
	FP_Gun->SetupAttachment(Mesh1P);
	//FP_Gun->SetupAttachment(RootComponent);

	FP_MuzzleLocation = CreateDefaultSubobject<USceneComponent>(TEXT("MuzzleLocation"));
	FP_MuzzleLocation->SetupAttachment(FP_Gun);
	FP_MuzzleLocation->SetRelativeLocation(FVector(0.2f, 48.4f, -10.6f));

	// Default offset from the character location for projectiles to spawn
	GunOffset = FVector(100.0f, 0.0f, 10.0f);

	//TODO wtf does text below mean

	// Note: The ProjectileClass and the skeletal mesh/anim blueprints for Mesh1P, FP_Gun, and VR_Gun 
	// are set in the derived blueprint asset named MyCharacter to avoid direct content references in C++.

}

void AWarFantasyCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	//Attach gun mesh component to Skeleton, doing it here because the skeleton is not yet created in the constructor
	FP_Gun->AttachToComponent(Mesh1P, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), TEXT("WeaponSocket"));

	Mesh1P->SetHiddenInGame(false, true);
	
}

//////////////////////////////////////////////////////////////////////////
// Input

void AWarFantasyCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// set up gameplay key bindings
	check(PlayerInputComponent);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &AWarFantasyCharacter::StartSprint);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &AWarFantasyCharacter::StopSprint);

	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &AWarFantasyCharacter::OnFire);
	PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &AWarFantasyCharacter::OnReload);

	PlayerInputComponent->BindAction("Aim", IE_Pressed, this, &AWarFantasyCharacter::OnLookDownSights);
	PlayerInputComponent->BindAction("Aim", IE_Released, this, &AWarFantasyCharacter::OnLookAwayFromSights);

	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &AWarFantasyCharacter::OnCrouchDown);
	PlayerInputComponent->BindAction("Crouch", IE_Released, this, &AWarFantasyCharacter::OnStandUp);

	PlayerInputComponent->BindAxis("MoveForward", this, &AWarFantasyCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AWarFantasyCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &AWarFantasyCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AWarFantasyCharacter::LookUpAtRate);

	// Set default walk speed. For some reason sprint is automatically called when starting the game. This issue might be solved when deriving a fresh blueprint from this class
	GetCharacterMovement()->MaxWalkSpeed = walkSpeed;
}

void AWarFantasyCharacter::OnFire()
{
	// try and fire a projectile
	if (ProjectileClass != NULL)
	{
		UWorld* const World = GetWorld();
		if (World != NULL)
		{
			const FRotator SpawnRotation = GetControlRotation();
			// MuzzleOffset is in camera space, so transform it to world space before offsetting from the character location to find the final muzzle position
			const FVector SpawnLocation = ((FP_MuzzleLocation != nullptr) ? FP_MuzzleLocation->GetComponentLocation() : GetActorLocation()) + SpawnRotation.RotateVector(GunOffset);

			//Set Spawn Collision Handling Override
			FActorSpawnParameters ActorSpawnParams;
			ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;

			// spawn the projectile at the muzzle
			World->SpawnActor<AWarFantasyProjectile>(ProjectileClass, SpawnLocation, SpawnRotation, ActorSpawnParams);
		}
	}

	// try and play the sound if specified
	if (FireSound != NULL)
	{
		UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation());
	}

	// try and play a firing animation if specified
	if (FireAnimation != NULL)
	{
		// Get the animation object for the arms mesh
		UAnimInstance* AnimInstance = Mesh1P->GetAnimInstance();
		if (AnimInstance != NULL)
		{
			AnimInstance->Montage_Play(FireAnimation, 1.f);
		}
	}
}

void AWarFantasyCharacter::StartSprint()
{
	bSprinting = true;

	GetCharacterMovement()->MaxWalkSpeed = sprintSpeed;

}

void AWarFantasyCharacter::StopSprint()
{
	bSprinting = false;

	GetCharacterMovement()->MaxWalkSpeed = walkSpeed;
}

void AWarFantasyCharacter::OnReload()
{

	// try and play a firing animation if specified
	if (HandsReloadAnimation != NULL/* && ReloadAnimation != NULL*/)
	{
		bReloading = true;

		// Get the animation object for the arms mesh
		UAnimInstance* HandsAnimInstance = Mesh1P->GetAnimInstance();
		UAnimInstance* GunAnimInstance = FP_Gun->GetAnimInstance();
		if (HandsAnimInstance != NULL && GunAnimInstance != NULL)
		{
			//Play the reload animations for the gun and the hand
			HandsAnimInstance->Montage_Play(HandsReloadAnimation, 1.f);
			GunAnimInstance->Montage_Play(ReloadAnimation, 1.f);

			//TODO delay code (I'm starting to think all animations should be done through blueprints)
		}
	}

}

void AWarFantasyCharacter::OnLookDownSights()
{
	if (!bSprinting)
		bAiming = true;

}

void AWarFantasyCharacter::OnLookAwayFromSights()
{
	bAiming = false;
}



/** Cease ADS. */
void AWarFantasyCharacter::OnCrouchDown()
{
	UE_LOG(LogTemp, Warning, TEXT("CROUCH PRESSED"));
	bCrouched = true;

	FirstPersonCameraComponent->RelativeLocation = FVector(0.f, 0.f, 0.f); // Position the camera
}

/** Cease ADS. */
void AWarFantasyCharacter::OnStandUp()
{
	bCrouched = true;

	FirstPersonCameraComponent->RelativeLocation = FVector(0.f, 0.f, 64.f); // Position the camera
}

void AWarFantasyCharacter::MoveForward(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorForwardVector(), Value);
	}
}

void AWarFantasyCharacter::MoveRight(float Value)
{
	if (Value != 0.0f)
	{
		FVector movementVector = GetActorRightVector();
		//if (bSprinting) movementVector *= sprintRate;

		// add movement in that direction
		AddMovementInput(movementVector, Value);
	}
}

void AWarFantasyCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AWarFantasyCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

