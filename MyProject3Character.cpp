// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "MyProject3Character.h"
#include "MyProject3Projectile.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/InputSettings.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "MotionControllerComponent.h"
#include "DrawDebugHelpers.h"
#include "BuildSystem.h"

#include "XRMotionControllerBase.h" // for FXRMotionControllerBase::RightHandSourceId

#include "wall.h"
#include "floor.h"
#include "roof.h"
#include "DoorFrame.h"
#include "door.h"

DEFINE_LOG_CATEGORY_STATIC(LogFPChar, Warning, All);

//////////////////////////////////////////////////////////////////////////
// AMyProject3Character

AMyProject3Character::AMyProject3Character()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->RelativeLocation = FVector(-39.56f, 1.75f, 64.f); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	Mesh1P->SetOnlyOwnerSee(true);
	Mesh1P->SetupAttachment(FirstPersonCameraComponent);
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->CastShadow = false;
	Mesh1P->RelativeRotation = FRotator(1.9f, -19.19f, 5.2f);
	Mesh1P->RelativeLocation = FVector(-0.5f, -4.4f, -155.7f);

	// Create a gun mesh component
	FP_Gun = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FP_Gun"));
	FP_Gun->SetOnlyOwnerSee(true);			// only the owning player will see this mesh
	FP_Gun->bCastDynamicShadow = false;
	FP_Gun->CastShadow = false;
	// FP_Gun->SetupAttachment(Mesh1P, TEXT("GripPoint"));
	FP_Gun->SetupAttachment(RootComponent);

	FP_MuzzleLocation = CreateDefaultSubobject<USceneComponent>(TEXT("MuzzleLocation"));
	FP_MuzzleLocation->SetupAttachment(FP_Gun);
	FP_MuzzleLocation->SetRelativeLocation(FVector(0.2f, 48.4f, -10.6f));

	// Default offset from the character location for projectiles to spawn
	GunOffset = FVector(100.0f, 0.0f, 10.0f);


	// Note: The ProjectileClass and the skeletal mesh/anim blueprints for Mesh1P, FP_Gun, and VR_Gun 
	// are set in the derived blueprint asset named MyCharacter to avoid direct content references in C++.

	// Create VR Controllers.
	R_MotionController = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("R_MotionController"));
	R_MotionController->MotionSource = FXRMotionControllerBase::RightHandSourceId;
	R_MotionController->SetupAttachment(RootComponent);
	L_MotionController = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("L_MotionController"));
	L_MotionController->SetupAttachment(RootComponent);

	// Create a gun and attach it to the right-hand VR controller.
	// Create a gun mesh component
	VR_Gun = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("VR_Gun"));
	VR_Gun->SetOnlyOwnerSee(true);			// only the owning player will see this mesh
	VR_Gun->bCastDynamicShadow = false;
	VR_Gun->CastShadow = false;
	VR_Gun->SetupAttachment(R_MotionController);
	VR_Gun->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));

	VR_MuzzleLocation = CreateDefaultSubobject<USceneComponent>(TEXT("VR_MuzzleLocation"));
	VR_MuzzleLocation->SetupAttachment(VR_Gun);
	VR_MuzzleLocation->SetRelativeLocation(FVector(0.000004, 53.999992, 10.000000));
	VR_MuzzleLocation->SetRelativeRotation(FRotator(0.0f, 90.0f, 0.0f));		// Counteract the rotation of the VR gun model.
	// Uncomment the following line to turn motion controllers on by default:
	//bUsingMotionControllers = true;
}

void AMyProject3Character::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	//Attach gun mesh component to Skeleton, doing it here because the skeleton is not yet created in the constructor
	FP_Gun->AttachToComponent(Mesh1P, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), TEXT("GripPoint"));

	// Show or hide the two versions of the gun based on whether or not we're using motion controllers.
	if (bUsingMotionControllers)
	{
		VR_Gun->SetHiddenInGame(false, true);
		Mesh1P->SetHiddenInGame(true, true);
	}
	else
	{
		VR_Gun->SetHiddenInGame(true, true);
		Mesh1P->SetHiddenInGame(false, true);
	}

	whatBuild = 0;
	
}
void AMyProject3Character::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	FVector loc1;
	FRotator rot1;
	FHitResult Hit;
	GetController()->GetPlayerViewPoint(loc1, rot1);
	FVector start1 = loc1;
	FVector end1 = start1 + (rot1.Vector() * 700);
	// FHitResult will hold all data returned by our line collision query


	// We set up a line trace from our current location to a point 1000cm ahead of us

	// You can use FCollisionQueryParams to further configure the query
	// Here we add ourselves to the ignored list so we won't block the trace
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);
	
	// To run the query, you need a pointer to the current level, which you can get from an Actor with GetWorld()
	// UWorld()->LineTraceSingleByChannel runs a line trace and returns the first actor hit over the provided collision channel.
	GetWorld()->LineTraceSingleByChannel(Hit, start1, end1, TraceChannelProperty, QueryParams);

	

	// If the trace hit something, bBlockingHit will be true,
	// and its fields will be filled with detailed info about what was hit
	if (Hit.bBlockingHit && IsValid(Hit.GetActor()))
	{
		traceHit = Hit.GetActor();
		WhereBuild = traceHit->GetActorLocation() - Hit.Location;
		//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, (traceHit->GetActorLocation()- GetActorLocation()).ToString());
		//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Purple, (traceHit ->GetActorRotation()).ToString());
		//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Purple, WhereBuild.ToString());

	}
	else {
		traceHit = nullptr;
	}
	
}


//////////////////////////////////////////////////////////////////////////
// Input

void AMyProject3Character::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// set up gameplay key bindings
	check(PlayerInputComponent);

	// Bind jump events
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	// Bind fire event
	PlayerInputComponent->BindAction("Increment", IE_Pressed, this, &AMyProject3Character::Increment);
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &AMyProject3Character::Build);

	PlayerInputComponent->BindAction("OnBuildSystem", IE_Pressed, this, &AMyProject3Character::OnStartSystem);
	PlayerInputComponent->BindAction("OnBuildSystem", IE_Released, this, &AMyProject3Character::OnEndSystem);

	PlayerInputComponent->BindAction("OpenDoor", IE_Pressed, this, &AMyProject3Character::OpenDoor);



	// Enable touchscreen input
	EnableTouchscreenMovement(PlayerInputComponent);

	PlayerInputComponent->BindAction("ResetVR", IE_Pressed, this, &AMyProject3Character::OnResetVR);

	// Bind movement events
	PlayerInputComponent->BindAxis("MoveForward", this, &AMyProject3Character::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AMyProject3Character::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &AMyProject3Character::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AMyProject3Character::LookUpAtRate);
	
}




void AMyProject3Character::Increment()
{
	whatBuild++;
	if (whatBuild == 5){
		whatBuild = 0;
	}
}




void AMyProject3Character::OpenDoor()
{
	if (Cast<Adoor>(traceHit)) {
		if (!Cast<Adoor>(traceHit)->IsOpen()) {
			Cast<Adoor>(traceHit)->OpenedDoor();
		}
		else {
			Cast<Adoor>(traceHit)->CloseDoor();
		}
		
	}
}











void AMyProject3Character::SpawnRoof(FVector location, FRotator rotation, bool ShouldOverlap, bool ShouldDestroy, bool ShouldRespawn)
{
	if (ShouldRespawn) {
		RoofSpawnDestroy = GetWorld()->SpawnActor<Aroof>(location, rotation);
	}
	else {
		RoofSpawn = GetWorld()->SpawnActor<Aroof>(location, rotation);
	}

	if (ShouldDestroy) {
		if (!RoofSpawn->CanSpawn()) {
			RoofSpawn->Destr();
		}
	}

	if (!ShouldOverlap) {
		RoofSpawnDestroy->StopOverlap();

	}
	if (ShouldDestroy) {
		RoofSpawn = nullptr;
	}
	if (ShouldRespawn) {
		GetWorld()->GetTimerManager().SetTimer(timerToShot, this, &AMyProject3Character::OnDestroyRoof, 0.1f, false);
	}
}

void AMyProject3Character::SpawnWall(FVector location, FRotator rotation, bool ShouldOverlap, bool ShouldDestroy, bool ShouldRespawn)
{
	if (ShouldRespawn) {
		WallSpawnDestroy = GetWorld()->SpawnActor<Awall>(location, rotation);
	}
	else {
		WallSpawn = GetWorld()->SpawnActor<Awall>(location, rotation);
	}

	if (ShouldDestroy) {
		if (!WallSpawn->CanSpawn()) {
			WallSpawn->Destr();
		}
	}

	if (!ShouldOverlap) {
		WallSpawnDestroy->StopOverlap();

	}
	if (ShouldDestroy) {
		
	}
	if (ShouldRespawn) {
		GetWorld()->GetTimerManager().SetTimer(timerToShot, this, &AMyProject3Character::OnDestroyWall, 0.1f, false);
	}
}

void AMyProject3Character::SpawnDoorFrame(FVector location, FRotator rotation, bool ShouldOverlap, bool ShouldDestroy, bool ShouldRespawn)
{
	if (ShouldRespawn) {
		DoorFrameSpawnDestroy = GetWorld()->SpawnActor<ADoorFrame>(location, rotation);
	}
	else {
		DoorFrameSpawn = GetWorld()->SpawnActor<ADoorFrame>(location, rotation);
		}

	if (ShouldDestroy) {
		if (!DoorFrameSpawn->CanSpawn()) {
			DoorFrameSpawn->Destr();
		}
	}

	if (!ShouldOverlap) {
		DoorFrameSpawnDestroy->StopOverlap();

	}
	if (ShouldDestroy) {
		DoorFrameSpawn = nullptr;
	}
	if (ShouldRespawn) {
		GetWorld()->GetTimerManager().SetTimer(timerToShot, this, &AMyProject3Character::OnDestroyDoorFrame, 0.1f, false);
	}
}

void AMyProject3Character::SpawnDoor(FVector location, FRotator rotation, bool ShouldOverlap, bool ShouldDestroy, bool ShouldRespawn)
{
	if (ShouldRespawn) {
		DoorSpawnDestroy = GetWorld()->SpawnActor<Adoor>(location, rotation);

	}
	else {
		DoorSpawn = GetWorld()->SpawnActor<Adoor>(location, rotation);

	}

	if (ShouldDestroy) {
		if (!DoorSpawn->CanSpawn()) {
			DoorSpawn->Destr();
		}
	}

	if (!ShouldOverlap) {
		DoorSpawnDestroy->StopOverlap();

	}
	if (ShouldDestroy) {
		DoorSpawn = nullptr;
	}
	if (ShouldRespawn) {
		GetWorld()->GetTimerManager().SetTimer(timerToShot, this, &AMyProject3Character::OnDestroyDoor, 0.1f, false);
	}
}

void AMyProject3Character::SpawnFloor(FVector location, FRotator rotation, bool ShouldOverlap, bool ShouldDestroy, bool ShouldRespawn)
{
	if (ShouldRespawn) {
		FloorSpawnDestroy = GetWorld()->SpawnActor<Afloor>(location, rotation);
	}
	else {
		FloorSpawn = GetWorld()->SpawnActor<Afloor>(location, rotation);
	}
	
	if (ShouldDestroy) {
		if (!FloorSpawn->CanSpawn()) {
			FloorSpawn->Destr();
		}
	}
	
	if (!ShouldOverlap) {
		FloorSpawnDestroy->StopOverlap();

	}
	if (ShouldDestroy) {
		FloorSpawn = nullptr;
	}
	if (ShouldRespawn) {
		GetWorld()->GetTimerManager().SetTimer(timerToShot, this, &AMyProject3Character::OnDestroyFloor, 0.1f, false);
	}
}

void AMyProject3Character::Build()
{
	if (isBuilding) {

		FVector loc;
		FRotator rot;
		FHitResult Hit;

		GetController()->GetPlayerViewPoint(loc, rot);

		FVector start = loc;
		FVector end = start + (rot.Vector() * 700);
		if (whatBuild == 0) {
			if (Cast<Afloor>(traceHit)) {
				if (WhereBuild.X > 0 && WhereBuild.Y > 0) {

					SpawnFloor(traceHit->GetActorLocation() + FVector(0.f, -300.f, 0.f), FRotator(0.f), true, true, false);

				}
				else if (WhereBuild.X < 0 && WhereBuild.Y > 0) {
					SpawnFloor(traceHit->GetActorLocation() + FVector(300.f, 0.f, 0.f), FRotator(0.f), true, true, false);

				}
				else if (WhereBuild.X < 0 && WhereBuild.Y < 0) {
					SpawnFloor(traceHit->GetActorLocation() + FVector(0.f, 300.f, 0.f), FRotator(0.f), true, true, false);

				}
				else {
					SpawnFloor(traceHit->GetActorLocation() + FVector(-300.f, 0.f, 0.f), FRotator(0.f), true, true, false);

				}

			}
			else {
				SpawnFloor(end, FRotator(0.f), true, true, false);
			}

		}
		else if (whatBuild == 1) {
			if (Cast<Afloor>(traceHit)) {
				if (WhereBuild.X > 0 && WhereBuild.Y > 0) {
					SpawnWall(traceHit->GetActorLocation() + FVector(0.f, -148.f, 157.f), FRotator(0.f, -90.f, 0.f), true, true, false);
				}
				else if (WhereBuild.X < 0 && WhereBuild.Y > 0) {

					SpawnWall(traceHit->GetActorLocation() + FVector(148.f, 0.f, 157.f), FRotator(0.f), true, true, false);
				}
				else if (WhereBuild.X < 0 && WhereBuild.Y < 0) {
					SpawnWall(traceHit->GetActorLocation() + FVector(0.f, 148.f, 157.f), FRotator(0.f, 90.f, 0.f), true, true, false);
				}
				else {
					SpawnWall(traceHit->GetActorLocation() + FVector(-148.f, 0.f, 157.f), FRotator(0.f, 180.f, 0.f), true, true, false);
				}
			}
			else {
				SpawnWall(end, FRotator(0.f), true, true, false);
			}
		}
		else if (whatBuild == 2) {
			if (Cast<Awall>(traceHit) || Cast<ADoorFrame>(traceHit)) {
				if ((traceHit->GetActorRotation() == FRotator(90.f, 90.f, 0.f) || traceHit->GetActorRotation() == FRotator(90.f, 90.f, 180.f)) || (traceHit->GetActorRotation() == Wall1 || traceHit->GetActorRotation() == Wall3)) {
					if ((traceHit->GetActorLocation() - GetActorLocation()).Y > 0) {
						SpawnRoof(traceHit->GetActorLocation() + FVector(0.f, -150.f, 150.f), FRotator(0.f), true, true, false);
					}
					else {
						SpawnRoof(traceHit->GetActorLocation() + FVector(0.f, 150.f, 150.f), FRotator(0.f), true, true, false);
					}
				}
				else {
					if ((traceHit->GetActorLocation() - GetActorLocation()).X > 0) {
						SpawnRoof(traceHit->GetActorLocation() + FVector(-150.f, 0.f, 150.f), FRotator(0.f), true, true, false);
					}
					else {
						SpawnRoof(traceHit->GetActorLocation() + FVector(150.f, 0.f, 150.f), FRotator(0.f), true, true, false);
					}

				}
			}


			else if (Cast<Aroof>(traceHit)) {
				if (WhereBuild.X > 0 && WhereBuild.Y > 0) {
					SpawnRoof(traceHit->GetActorLocation() + FVector(0.f, -300.f, 0.f), FRotator(0.f), true, true, false);
				}
				else if (WhereBuild.X < 0 && WhereBuild.Y > 0) {
					SpawnRoof(traceHit->GetActorLocation() + FVector(300.f, 0.f, 0.f), FRotator(0.f), true, true, false);
				}
				else if (WhereBuild.X < 0 && WhereBuild.Y < 0) {
					SpawnRoof(traceHit->GetActorLocation() + FVector(0.f, 300.f, 0.f), FRotator(0.f), true, true, false);
				}
				else {
					SpawnRoof(traceHit->GetActorLocation() + FVector(-300.f, 0.f, 0.f), FRotator(0.f), true, true, false);
				}
			}
			else {
				GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Cant Build here!!!!!!!!!!!"));
			}
		}

		else if (whatBuild == 3) {
			if (Cast<Afloor>(traceHit)) {
				if (WhereBuild.X > 0 && WhereBuild.Y > 0) {
					SpawnDoorFrame(traceHit->GetActorLocation() + FVector(0.f, -148.f, 157.f), FRotator(0.f, 0.f, 0.f), true, true, false);

				}
				else if (WhereBuild.X < 0 && WhereBuild.Y > 0) {

					SpawnDoorFrame(traceHit->GetActorLocation() + FVector(148.f, 0.f, 157.f), FRotator(0.f, 90.f, 0.f), true, true, false);

				}
				else if (WhereBuild.X < 0 && WhereBuild.Y < 0) {
					SpawnDoorFrame(traceHit->GetActorLocation() + FVector(0.f, 148.f, 157.f), FRotator(0.f, 180.f, 0.f), true, true, false);

				}
				else {
					SpawnDoorFrame(traceHit->GetActorLocation() + FVector(-148.f, 0.f, 157.f), FRotator(0.f, 270.f, 0.f), true, true, false);

				}
			}
			else {
				SpawnDoorFrame(end, FRotator(0.f), true, true, false);
			}
		}
		else if (whatBuild == 4) {
			if (Cast<ADoorFrame>(traceHit)) {
				if (traceHit->GetActorRotation() == Wall2 || traceHit->GetActorRotation() == Wall4) {
					SpawnDoor(traceHit->GetActorLocation(), FRotator(0.f, 90.f, 0.f), true, true, false);
				}
				else{
					SpawnDoor(traceHit->GetActorLocation(), FRotator(0.f, 0.f, 0.f), true, true, false);
				}

			}
			else {
					GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Cant Build here!!!!!!!!!!!"));
			}
		}


	}







}




void AMyProject3Character::OnStartSystem()
{
	isBuilding=1;
	OnBuildSystem();
}

void AMyProject3Character::OnEndSystem()
{
	isBuilding = 0;
}

void AMyProject3Character::OnBuildSystem()
{
	if (isBuilding) {


		FVector loc;
		FRotator rot;
		FHitResult Hit;

		GetController()->GetPlayerViewPoint(loc, rot);
		FVector start = loc;
		FVector end = start + (rot.Vector() * 700);

		if (whatBuild == 0) {
			if (Cast<Afloor>(traceHit)) {
				if (WhereBuild.X > 0 && WhereBuild.Y > 0) {

					SpawnFloor(traceHit->GetActorLocation() + FVector(0.f, -300.f, 0.f), FRotator(0.f), false, false, true);

				}
				else if (WhereBuild.X < 0 && WhereBuild.Y > 0) {
					SpawnFloor(traceHit->GetActorLocation() + FVector(300.f, 0.f, 0.f), FRotator(0.f), false, false, true);

				}
				else if (WhereBuild.X < 0 && WhereBuild.Y < 0) {
					SpawnFloor(traceHit->GetActorLocation() + FVector(0.f, 300.f, 0.f), FRotator(0.f), false, false, true);

				}
				else {
					SpawnFloor(traceHit->GetActorLocation() + FVector(-300.f, 0.f, 0.f), FRotator(0.f), false, false, true);

				}

			}
			else {
				SpawnFloor(end, FRotator(0.f), false, false, true);
			}

		}
		else if (whatBuild == 1) {
			if (Cast<Afloor>(traceHit)) {
				if (WhereBuild.X > 0 && WhereBuild.Y > 0) {
					SpawnWall(traceHit->GetActorLocation() + FVector(0.f, -148.f, 157.f), FRotator(0.f, -90.f, 0.f), false, false, true);
				}
				else if (WhereBuild.X < 0 && WhereBuild.Y > 0) {

					SpawnWall(traceHit->GetActorLocation() + FVector(148.f, 0.f, 157.f), FRotator(0.f), false, false, true);
				}
				else if (WhereBuild.X < 0 && WhereBuild.Y < 0) {
					SpawnWall(traceHit->GetActorLocation() + FVector(0.f, 148.f, 157.f), FRotator(0.f, 90.f, 0.f), false, false, true);
				}
				else {
					SpawnWall(traceHit->GetActorLocation() + FVector(-148.f, 0.f, 157.f), FRotator(0.f, 180.f, 0.f), false, false, true);
				}
			}
			else {
				SpawnWall(end, FRotator(0.f), false, false, true);
			}
		}
		else if (whatBuild == 2) {
			if (Cast<Awall>(traceHit) || Cast<ADoorFrame>(traceHit)) {
				if ((traceHit->GetActorRotation() == FRotator(90.f, 90.f, 0.f) || traceHit->GetActorRotation() == FRotator(90.f, 90.f, 180.f)) || (traceHit->GetActorRotation() == Wall1 || traceHit->GetActorRotation() == Wall3)) {
					if ((traceHit->GetActorLocation() - GetActorLocation()).Y > 0) {
						SpawnRoof(traceHit->GetActorLocation() + FVector(0.f, -150.f, 150.f), FRotator(0.f), false, false, true);
					}
					else {
						SpawnRoof(traceHit->GetActorLocation() + FVector(0.f, 150.f, 150.f), FRotator(0.f), false, false, true);
					}
				}
				else {
					if ((traceHit->GetActorLocation() - GetActorLocation()).X > 0) {
						SpawnRoof(traceHit->GetActorLocation() + FVector(-150.f, 0.f, 150.f), FRotator(0.f), false, false, true);
					}
					else {
						SpawnRoof(traceHit->GetActorLocation() + FVector(150.f, 0.f, 150.f), FRotator(0.f), false, false, true);
					}

				}
			}


			else if (Cast<Aroof>(traceHit)) {
				if (WhereBuild.X > 0 && WhereBuild.Y > 0) {
					SpawnRoof(traceHit->GetActorLocation() + FVector(0.f, -300.f, 0.f), FRotator(0.f), false, false, true);
				}
				else if (WhereBuild.X < 0 && WhereBuild.Y > 0) {
					SpawnRoof(traceHit->GetActorLocation() + FVector(300.f, 0.f, 0.f), FRotator(0.f), false, false, true);
				}
				else if (WhereBuild.X < 0 && WhereBuild.Y < 0) {
					SpawnRoof(traceHit->GetActorLocation() + FVector(0.f, 300.f, 0.f), FRotator(0.f), false, false, true);
				}
				else {
					SpawnRoof(traceHit->GetActorLocation() + FVector(-300.f, 0.f, 0.f), FRotator(0.f), false, false, true);
				}
			}

			else {
				SpawnRoof(end, FRotator(0.f), false, false, true);
			}
		}

		else if (whatBuild == 3) {
			if (Cast<Afloor>(traceHit)) {
				if (WhereBuild.X > 0 && WhereBuild.Y > 0) {
					SpawnDoorFrame(traceHit->GetActorLocation() + FVector(0.f, -148.f, 157.f), FRotator(0.f, 0.f, 0.f), false, false, true);
					Wall1 = DoorFrameSpawnDestroy->GetActorRotation();
				}
				else if (WhereBuild.X < 0 && WhereBuild.Y > 0) {

					SpawnDoorFrame(traceHit->GetActorLocation() + FVector(148.f, 0.f, 157.f), FRotator(0.f, 90.f, 0.f), false, false, true);
					Wall2 = DoorFrameSpawnDestroy->GetActorRotation();
				}
				else if (WhereBuild.X < 0 && WhereBuild.Y < 0) {
					SpawnDoorFrame(traceHit->GetActorLocation() + FVector(0.f, 148.f, 157.f), FRotator(0.f, 180.f, 0.f), false, false, true);
					Wall3 = DoorFrameSpawnDestroy->GetActorRotation();
				}
				else {
					SpawnDoorFrame(traceHit->GetActorLocation() + FVector(-148.f, 0.f, 157.f), FRotator(0.f, 270.f, 0.f), false, false, true);
					Wall4 = DoorFrameSpawnDestroy->GetActorRotation();
				}
			}
			else {
				SpawnDoorFrame(end, FRotator(0.f), false, false, true);
			}
		}
		else if (whatBuild == 4) {
			if (Cast<ADoorFrame>(traceHit)) {
				if (traceHit->GetActorRotation() == Wall2 || traceHit->GetActorRotation() == Wall4) {
					SpawnDoor(traceHit->GetActorLocation(), FRotator(0.f, 90.f, 0.f), false, false, true);
				}
				else {
					SpawnDoor(traceHit->GetActorLocation(), FRotator(0.f, 0.f, 0.f), false, false, true);
				}
			}
			else {
				SpawnDoor(end, FRotator(0.f), false, false, true);
			}
		}
	}
}

void AMyProject3Character::OnDestroy()
{
	

	


	OnBuildSystem();
}

void AMyProject3Character::OnDestroyWall()
{
	if (WallSpawnDestroy != nullptr) {
		WallSpawnDestroy->Destr();
	}
	OnBuildSystem();
}

void AMyProject3Character::OnDestroyRoof()
{
	if (RoofSpawnDestroy != nullptr) {
		RoofSpawnDestroy->Destr();

	}
	OnBuildSystem();
}

void AMyProject3Character::OnDestroyFloor()
{
	if (FloorSpawnDestroy != nullptr) {
		FloorSpawnDestroy->Destr();
	}
	OnBuildSystem();
}

void AMyProject3Character::OnDestroyDoor()
{
	if (DoorSpawnDestroy != nullptr) {
		DoorSpawnDestroy->Destr();
	}
	OnBuildSystem();
}

void AMyProject3Character::OnDestroyDoorFrame()
{
	if (DoorFrameSpawnDestroy != nullptr) {
		DoorFrameSpawnDestroy->Destr();
	}
	OnBuildSystem();
}





void AMyProject3Character::OnResetVR()
{
	UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition();
}

void AMyProject3Character::BeginTouch(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	if (TouchItem.bIsPressed == true)
	{
		return;
	}
	if ((FingerIndex == TouchItem.FingerIndex) && (TouchItem.bMoved == false))
	{
		Build();
	}
	TouchItem.bIsPressed = true;
	TouchItem.FingerIndex = FingerIndex;
	TouchItem.Location = Location;
	TouchItem.bMoved = false;
}

void AMyProject3Character::EndTouch(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	if (TouchItem.bIsPressed == false)
	{
		return;
	}
	TouchItem.bIsPressed = false;
}

//Commenting this section out to be consistent with FPS BP template.
//This allows the user to turn without using the right virtual joystick

//void AMyProject3Character::TouchUpdate(const ETouchIndex::Type FingerIndex, const FVector Location)
//{
//	if ((TouchItem.bIsPressed == true) && (TouchItem.FingerIndex == FingerIndex))
//	{
//		if (TouchItem.bIsPressed)
//		{
//			if (GetWorld() != nullptr)
//			{
//				UGameViewportClient* ViewportClient = GetWorld()->GetGameViewport();
//				if (ViewportClient != nullptr)
//				{
//					FVector MoveDelta = Location - TouchItem.Location;
//					FVector2D ScreenSize;
//					ViewportClient->GetViewportSize(ScreenSize);
//					FVector2D ScaledDelta = FVector2D(MoveDelta.X, MoveDelta.Y) / ScreenSize;
//					if (FMath::Abs(ScaledDelta.X) >= 4.0 / ScreenSize.X)
//					{
//						TouchItem.bMoved = true;
//						float Value = ScaledDelta.X * BaseTurnRate;
//						AddControllerYawInput(Value);
//					}
//					if (FMath::Abs(ScaledDelta.Y) >= 4.0 / ScreenSize.Y)
//					{
//						TouchItem.bMoved = true;
//						float Value = ScaledDelta.Y * BaseTurnRate;
//						AddControllerPitchInput(Value);
//					}
//					TouchItem.Location = Location;
//				}
//				TouchItem.Location = Location;
//			}
//		}
//	}
//}

void AMyProject3Character::MoveForward(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorForwardVector(), Value);
	}
}

void AMyProject3Character::MoveRight(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorRightVector(), Value);
	}
}

void AMyProject3Character::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AMyProject3Character::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

bool AMyProject3Character::EnableTouchscreenMovement(class UInputComponent* PlayerInputComponent)
{
	if (FPlatformMisc::SupportsTouchInput() || GetDefault<UInputSettings>()->bUseMouseForTouch)
	{
		PlayerInputComponent->BindTouch(EInputEvent::IE_Pressed, this, &AMyProject3Character::BeginTouch);
		PlayerInputComponent->BindTouch(EInputEvent::IE_Released, this, &AMyProject3Character::EndTouch);

		//Commenting this out to be more consistent with FPS BP template.
		//PlayerInputComponent->BindTouch(EInputEvent::IE_Repeat, this, &AMyProject3Character::TouchUpdate);
		return true;
	}
	
	return false;
}
